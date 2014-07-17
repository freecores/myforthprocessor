#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)safepoint.cpp	1.247 03/04/25 10:46:49 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

//

# include "incls/_precompiled.incl"
# include "incls/_safepoint.cpp.incl"

// --------------------------------------------------------------------------------------------------
// Implementation of Safepoint begin/end

SafepointSynchronize::SynchronizeState volatile SafepointSynchronize::_state = SafepointSynchronize::_not_synchronized;
volatile int  SafepointSynchronize::_waiting_to_block = 0;
long SafepointSynchronize::_last_safepoint = 0;

// Roll all threads forward to a safepoint and suspend them all
void SafepointSynchronize::begin() {   
  Thread* myThread = Thread::current();

  assert(myThread->is_VM_thread() ||
    (UseConcMarkSweepGC && myThread->is_ConcurrentMarkSweep_thread()),
    "Only VM thread (or CMS_thread) may initiate a safepoint synchronization");

  if (UseConcMarkSweepGC) {
    ConcurrentMarkSweepThread::synchronize(
      myThread->is_ConcurrentMarkSweep_thread());
  }

  // By getting the Threads_lock, we assure that no threads are about to start or 
  // exit. It is released again in SafepointSynchronize::end().  
  Threads_lock->lock();
  
  assert( _state == _not_synchronized, "trying to safepoint synchronize with wrong state");     
  int nof_threads = Threads::number_of_threads();

  if (TraceSafepoint) {
    tty->print_cr("Safepoint synchronization initiated. (%d)", nof_threads);
  }

  if (PrintSafepointStatistics) {
    begin_statistics(nof_threads);
  }    

  {
  MutexLocker mu(Safepoint_lock);

  // Set number of threads to wait for, before we initiate the callbacks 
  _waiting_to_block = nof_threads;    
  int still_running = nof_threads;    

  // At this point, threads will start doing callback into the safepoint code. If it is in
  // native or blocked, the thread will simply block on the barrier. If it is the vm, it will
  // decrement the _waiting_to_block counter and then block.
  _state            = _synchronizing;
  if( os::is_MP() ) atomic::membar();

  // Make interpreter safepoint aware
  AbstractInterpreter::notice_safepoints(); 

#ifdef ASSERT
  for (JavaThread *cur = Threads::first(); cur != NULL; cur = cur->next()) {
    assert(cur->safepoint_state()->is_running(), "Illegal initial state");
  }
#endif
  // Iterate through all threads until it have been determined how to stop them all at a safepoint  
  unsigned int iterations = 0;
  while(still_running > 0) {
    for (JavaThread *cur = Threads::first(); cur != NULL; cur = cur->next()) {
      assert(!cur->is_ConcurrentMarkSweep_thread(), "The cms thread is unexpectly being suspended");
      ThreadSafepointState *cur_state = cur->safepoint_state();
      if (cur_state->is_running()) {          
        cur_state->examine_state_of_thread(iterations);                              
        if (!cur_state->is_running()) still_running--;
        if (TraceSafepoint && Verbose) cur_state->print();
      }        
    } 
    if (still_running > 0) {
      if (iterations < (unsigned int)DeferThrSuspendLoopCount) {
        os::yield();
      } else {
        os::yield_all(iterations); // Yield to all (including low-priority) threads
      }
      iterations += 1;
    }
    assert(iterations < max_jint, "We have been iterating in the safepoint loop too long");
  }     
  assert(still_running == 0, "sanity check");      

  // wait until all threads are stopped    
  while (_waiting_to_block > 0) {
    if (TraceSafepoint) tty->print_cr("Waiting for %d thread(s) to block", _waiting_to_block);
    if (!SafepointTimeout) {
      Safepoint_lock->wait(true);  // true, means with no safepoint checks
    } else {
      // Wait for at most delay milliseconds to reach a safepoint
      bool timed_out = Safepoint_lock->wait(true, SafepointTimeoutDelay);
      static bool timeout_error_printed = false;
      if (timed_out && !timeout_error_printed) {
        timeout_error_printed = true;
        // Print out the thread IDs which didn't reach the safepoint
        // for debugging purposes (useful when there are lots of
        // threads in the debugger)
        tty->print_cr("# SafepointSynchronize::begin: Fatal error:");
        tty->print_cr("# SafepointSynchronize::begin: Timed out while attempting to reach a safepoint.");
        tty->print_cr("# SafepointSynchronize::begin: Threads which did not reach the safepoint:");
        for(JavaThread *cur = Threads::first(); cur; cur = cur->next()) {
          ThreadSafepointState *cur_state = cur->safepoint_state();
          if (cur_state->type() == ThreadSafepointState::_call_back) {
            if (!cur->has_called_back()) {
              tty->print("# ");
              cur_state->print();
              cur->osthread()->print();
              tty->print_cr("");
            }
          } else if (cur_state->type() == ThreadSafepointState::_compiled_safepoint) {
            tty->print("# ");
            cur_state->print();
            cur->osthread()->print();
            tty->print_cr("");
          }
        }
        tty->print_cr("# SafepointSynchronize::begin: (End of list)");
        if (DieOnSafepointTimeout) {
          ShouldNotReachHere();
        }
      }
    }
  }               
  assert(_waiting_to_block == 0, "sanity check");
    
  // Record state
  _state = _synchronized;
  if( os::is_MP() ) atomic::membar();

  if (TraceSafepoint) {
    VM_Operation *op = VMThread::vm_operation();     
    tty->print_cr("Entering safepoint region: %s", (op != NULL) ? op->name() : "no vm operation");
  }
 
  if (PrintSafepointStatistics) {    
    end_statistics();
  }    

  // Call stuff that needs to be run when a safepoint is just about to be completed
  do_cleanup_tasks();  
  }
}

// Wake up all threads, so they are ready to resume execution after the safepoint
// operation has been carried out
void SafepointSynchronize::end() {        

  Thread* myThread = Thread::current();
  assert(myThread->is_VM_thread() ||
   (UseConcMarkSweepGC && myThread->is_ConcurrentMarkSweep_thread()),
   "Only VM thread (or CMS_thread) can end a safepoint synchronization");

#ifndef CORE
  // If any exception got thrown on a compiled thread in a compiled safepoint, then
  // deoptimize the frame.
  for(JavaThread *cur = Threads::first(); cur; cur = cur->next()) {      

#ifndef PRODUCT
    // Force a deopt_suspend for native threads to test the code
    if (DeoptimizeALot && needs_deopt_suspend(cur) && cur->has_last_Java_frame()) {
      ResourceMark rm; // vframe code allocates ResourceObjs's
      HandleMark hm; // Can allocate handles reading scope info

      frame stub_fr = cur->last_frame();      // Should be native transition frame
      RegisterMap map(cur, false);            
      frame caller_fr = stub_fr.sender(&map); 
      Deoptimization::deoptimize_frame(cur, caller_fr.id());     
    }
#endif /* PRODUCT */
    if (cur->has_pending_exception() && cur->safepoint_state()->type() == ThreadSafepointState::_compiled_safepoint) {    
#ifdef COMPILER1
      if (DeoptC1 && DeoptOnAsyncException) {
#endif
      ResourceMark rm; // vframe code allocates ResourceObjs's
      HandleMark hm; // Can allocate handles reading scope info
      frame stub_fr = cur->last_frame();      
      RegisterMap map(cur, false);            
      frame caller_fr = stub_fr.sender_for_raw_compiled_frame(&map);         
      Deoptimization::deoptimize_frame(cur, caller_fr.id());     
#ifdef COMPILER1
      }
#endif
    }    
  }
#endif // !CORE

  {
    MutexLocker mu(Safepoint_lock);

    assert(_state == _synchronized, "must be synchronized before ending safepoint synchronization");

    // Set to not synchronized, so the threads will not go into the signal_thread_blocked method
    // when they get restarted.
    _state = _not_synchronized;  
    if( os::is_MP() ) atomic::membar();
  
    if (TraceSafepoint) {
       tty->print_cr("Leaving safepoint region");
    }

    // Remove safepoint check from interpreter
    AbstractInterpreter::ignore_safepoints();
  
    // Start suspended threads
    for(JavaThread *current = Threads::first(); current; current = current->next()) {      
      // A problem occuring on Solaris is when attempting to restart threads
      // the first #cpus - 1 go well, but then the VMThread is preempted when we get
      // to the next one (since it has been running the longest).  We then have
      // to wait for a cpu to become available before we can continue restarting
      // threads.
      // FIXME: This causes the performance of the VM to degrade when active and with
      // large numbers of threads.  Apparently this is due to the synchronous nature
      // of suspending threads.
      if (VMThreadHintNoPreempt) {
        os::hint_no_preempt();
      }
      Thread::unboost_priority(current);
      ThreadSafepointState* cur_state = current->safepoint_state();
      assert(cur_state->type() != ThreadSafepointState::_running, "Thread not suspended at safepoint");
      cur_state->restart();                  
      assert(cur_state->is_running(), "safepoint state has not been reset");
    }

    // Release threads lock, so threads can be created/destroyed again. It will also starts all threads
    // blocked in signal_thread_blocked
    Threads_lock->unlock();  

    _last_safepoint = os::javaTimeMillis();

  }
  if (UseConcMarkSweepGC) {
    ConcurrentMarkSweepThread::desynchronize(
      myThread->is_ConcurrentMarkSweep_thread());
  }
}

bool SafepointSynchronize::is_cleanup_needed() {
#ifndef CORE
  // Need a safepoint if some inline cache buffers is non-empty
  if (!InlineCacheBuffer::is_empty()) return true;
  // do not trigger safepoints for running the CounterDecayTask it causes too
  // many safepoints resulting in a degradation in performance
  //if (UseCounterDecay && CounterDecay::is_decay_needed()) return true;
#endif // !CORE
  return false;
}

// Various cleaning tasks that should be done periodically at safepoints
void SafepointSynchronize::do_cleanup_tasks() {  
  // Update fat-monitor pool, since this is a safepoint.
  ObjectSynchronizer::deflate_idle_monitors();
#ifndef CORE  
  InlineCacheBuffer::update_inline_caches();  
  if( UseCounterDecay ) {
    CounterDecay::decay();
  }
  NMethodSweeper::sweep(); 
#endif // !CORE
}


// -------------------------------------------------------------------------------------------------------
// Implementation of Safepoint callback point

void SafepointSynchronize::block(JavaThread *thread) {
  assert(thread != NULL, "thread must be set");
  assert(thread->is_Java_thread(), "not a Java thread");

  // Threads shouldn't block if they are in the middle of printing, but...
  ttyLocker::break_tty_lock_for_safepoint(os::current_thread_id());

  // Only bail from the block() call if the thread is gone from the
  // thread list; starting to exit should still block.
  if (thread->is_terminated()) {
     // block current thread if we come here from native code when VM is gone
     thread->block_if_vm_exited();

     // otherwise do nothing
     return;
  }

  JavaThreadState state = thread->thread_state();
  JavaFrameAnchor orig_state;
  thread->save_state_before_block(&orig_state);
  
  // Check that we have a valid thread_state at this point
  switch(state) {
    case _thread_in_vm_trans:
    case _thread_in_Java:        // From compiled code            

      // We are highly likely to block on the Safepoint_lock. In order to avoid blocking in this case,
      // we pretend we are still in the VM.
      thread->set_thread_state(_thread_in_vm);

      // We will always be holding the Safepoint_lock when we are examine the state
      // of a thread. Hence, the instructions between the Safepoint_lock->lock() and 
      // Safepoint_lock->unlock() are happening atomic with regards to the safepoint code      
      Safepoint_lock->lock_without_safepoint_check();
      if (is_synchronizing()) {
        // Decrement the number of threads to wait for and signal vm thread      
        assert(_waiting_to_block > 0, "sanity check");
        _waiting_to_block--;                       
        thread->set_has_called_back(true);
        Safepoint_lock->notify_all();
      }

      // We transition the thread to state _thread_blocked here, but
      // we can't do our usual check for external suspension and then
      // self-suspend after the lock_without_safepoint_check() call
      // below because we are often called during transitions while
      // we hold different locks. That would leave us suspended while
      // holding a resource which results in deadlocks.
      thread->set_thread_state(_thread_blocked);
      Safepoint_lock->unlock();            
            
      // We now try to acquire the threads lock. Since this lock is hold by the VM thread during
      // the entire safepoint, the threads will all line up here during the safepoint.
      Threads_lock->lock_without_safepoint_check();
      // restore original state. This is important if the thread comes from compiled code, so it
      // will continue to execute with the _thread_in_Java state. 
      thread->set_thread_state(state);
      Threads_lock->unlock(); 
      break;                  
       
    case _thread_in_native_trans:
    case _thread_blocked_trans:
    case _thread_new_trans:
      if (thread->safepoint_state()->type() == ThreadSafepointState::_call_back) {
        address stop_pc = thread->safepoint_state()->_stop_pc;
        InterpreterCodelet* icd = Interpreter::codelet_containing(stop_pc);
        if (icd != NULL) {
          icd->print();
          fatal("Wrong safepoint info in interpreter");
        } else {
          fatal1("Deadlock in safepoint code. stopped at " INTPTR_FORMAT, stop_pc);
        }
      }
          
      // It is not safe to suspend a thread if we discover it is in _thread_in_native_trans. Hence,
      // the safepoint code might still be waiting for it to block. We need to change the state here,
      // so it can see that it is at a safepoint. 
      // Furthermore, JVM_ThreadStop needs to be able to tell if a thread is in native, so it can figure
      // out how to deliver an async. exception to a thread. It uses the thread_state(), so we have a special
      // thread_state, so we can see we are here from native code.
      if (state == _thread_in_native_trans) thread->set_thread_state(_thread_in_native_blocked);

      // Block until the safepoint operation is completed. 
      Threads_lock->lock_without_safepoint_check();

      // Restore state
      thread->set_thread_state(state);

      Threads_lock->unlock();      
#ifndef CORE
      if (thread->has_deopt_suspension()) {
	// Make sure top most java/native transition frame has correct state
	SafepointSynchronize::do_deopt_suspend(thread);
      }
#endif
      break;

    default:
     fatal1("Illegal threadstate encountered: %d", state);
  }

  thread->restore_state_after_block(&orig_state);

  // Check for pending. async. exceptions or suspends - except if the
  // thread was blocked inside the VM. has_special_runtime_exit_condition()
  // is called last since it grabs a lock and we only want to do that when
  // we must.
  if (state != _thread_blocked_trans && state != _thread_in_vm_trans &&
  thread->has_special_runtime_exit_condition()) {
    thread->handle_special_runtime_exit_condition();  
  }
}

// ------------------------------------------------------------------------------------------------------
// Exception handlers

#ifndef PRODUCT
#ifdef _LP64
#define PTR_PAD ""
#else
#define PTR_PAD "        "
#endif

static void print_ptrs(intptr_t oldptr, intptr_t newptr, bool wasoop) {
  bool is_oop = newptr ? ((oop)newptr)->is_oop() : false;
  tty->print_cr(PTR_FORMAT PTR_PAD " %s %c " PTR_FORMAT PTR_PAD " %s %s",
                oldptr, wasoop?"oop":"   ", oldptr == newptr ? ' ' : '!',
                newptr, is_oop?"oop":"   ", (wasoop && !is_oop) ? "STALE" : ((wasoop==false&&is_oop==false&&oldptr !=newptr)?"STOMP":"     "));
}

static void print_longs(jlong oldptr, jlong newptr, bool wasoop) {
  bool is_oop = newptr ? ((oop)newptr)->is_oop() : false;
  tty->print_cr(PTR64_FORMAT " %s %c " PTR64_FORMAT " %s %s",
                oldptr, wasoop?"oop":"   ", oldptr == newptr ? ' ' : '!',
                newptr, is_oop?"oop":"   ", (wasoop && !is_oop) ? "STALE" : ((wasoop==false&&is_oop==false&&oldptr !=newptr)?"STOMP":"     "));
}

#ifdef SPARC
static void print_me(intptr_t *new_sp, intptr_t *old_sp, bool *was_oops) {
#ifdef _LP64
  tty->print_cr("--------+------address-----+------before-----------+-------after----------+");
  const int incr = 1;           // Increment to skip a long, in units of intptr_t
#else
  tty->print_cr("--------+--address-+------before-----------+-------after----------+");
  const int incr = 2;           // Increment to skip a long, in units of intptr_t
#endif
  tty->print_cr("---SP---|");
  for( int i=0; i<16; i++ ) {
    tty->print("blob %c%d |"PTR_FORMAT" ","LO"[i>>3],i&7,new_sp); print_ptrs(*old_sp++,*new_sp++,*was_oops++); }
  tty->print_cr("--------|");
  for( int i1=0; i1<frame::memory_parameter_word_sp_offset-16; i1++ ) {
    tty->print("argv pad|"PTR_FORMAT" ",new_sp); print_ptrs(*old_sp++,*new_sp++,*was_oops++); }
  tty->print("     pad|"PTR_FORMAT" ",new_sp); print_ptrs(*old_sp++,*new_sp++,*was_oops++);
  tty->print_cr("--------|");
  tty->print(" G1     |"PTR_FORMAT" ",new_sp); print_longs(*(jlong*)old_sp,*(jlong*)new_sp,was_oops[incr-1]); old_sp += incr; new_sp += incr; was_oops += incr;
  tty->print(" G3     |"PTR_FORMAT" ",new_sp); print_longs(*(jlong*)old_sp,*(jlong*)new_sp,was_oops[incr-1]); old_sp += incr; new_sp += incr; was_oops += incr;
  tty->print(" G4     |"PTR_FORMAT" ",new_sp); print_longs(*(jlong*)old_sp,*(jlong*)new_sp,was_oops[incr-1]); old_sp += incr; new_sp += incr; was_oops += incr;
  tty->print(" G5     |"PTR_FORMAT" ",new_sp); print_longs(*(jlong*)old_sp,*(jlong*)new_sp,was_oops[incr-1]); old_sp += incr; new_sp += incr; was_oops += incr;
  tty->print_cr(" FSR    |"PTR_FORMAT" "PTR64_FORMAT"       "PTR64_FORMAT,new_sp,*(jlong*)old_sp,*(jlong*)new_sp);
  old_sp += incr; new_sp += incr; was_oops += incr;
  // Skip the floats
  tty->print_cr("--Float-|"PTR_FORMAT,new_sp);
  tty->print_cr("---FP---|");
  old_sp += incr*32;  new_sp += incr*32;  was_oops += incr*32;
  for( int i2=0; i2<16; i2++ ) {
    tty->print("call %c%d |"PTR_FORMAT" ","LI"[i2>>3],i2&7,new_sp); print_ptrs(*old_sp++,*new_sp++,*was_oops++); }
  tty->print_cr("");
}
#endif
#endif

int SafepointSynchronize::handle_illegal_instruction_exception(JavaThread *thread) { 
  assert(thread->is_Java_thread(), "illegal instruction encountered by VM thread");
  assert(thread->thread_state() == _thread_in_Java, "should come from Java code");
  assert(SafepointSynchronize::is_synchronizing(), "illegal instruction encountered outside safepoint synchronization");  

  // Uncomment this to get some serious before/after printing of the
  // Sparc safepoint-blob frame structure.
  /*
  intptr_t* sp = thread->last_Java_sp();
  intptr_t stack_copy[150];
  for( int i=0; i<150; i++ ) stack_copy[i] = sp[i];
  bool was_oops[150];
  for( int i=0; i<150; i++ ) 
    was_oops[i] = stack_copy[i] ? ((oop)stack_copy[i])->is_oop() : false;
  */

  if (ShowSafepointMsgs) {
    tty->print("handle_illegal_instruction_exception: ");  
  }  

  ThreadSafepointState* state = thread->safepoint_state();
  assert(state->type() == ThreadSafepointState::_compiled_safepoint, "called on thread without installing handler");
  assert(state->handle() != NULL, "handler not set");  

  int result = state->handle()->handle_illegal_instruction_exception();  
  // print_me(sp,stack_copy,was_oops);
  return result;
}


// -------------------------------------------------------------------------------------------------------
// Implementation of ThreadSafepointState 

ThreadSafepointState::ThreadSafepointState(JavaThread *thread) {
  _thread = thread;
  _type   = _running;
  NOT_CORE(_handle = new CompiledCodeSafepointHandler(thread);)
  NOT_CORE(_code_buffer = NULL;)
}

ThreadSafepointState::~ThreadSafepointState() {
#ifndef CORE
  assert(_handle != NULL, "must be created");
  delete _handle;
  _handle = NULL;
  assert(_code_buffer == NULL, "should already been removed");
#endif // !CORE
}

void ThreadSafepointState::create(JavaThread *thread) {
  ThreadSafepointState *state = new ThreadSafepointState(thread);
  thread->set_safepoint_state(state);
}

void ThreadSafepointState::destroy(JavaThread *thread) {
  if (thread->safepoint_state()) {
    delete(thread->safepoint_state());  
    thread->set_safepoint_state(NULL);
  }
}

void ThreadSafepointState::examine_state_of_thread(int iterations) {  
  assert(is_running(), "better be running or this should not be called");
  _stop_pc    = 0; // temp. debugging
  
  // Quick check for common cases which can be done without suspending the thread  
  JavaThreadState state = _thread->thread_state();  
  _stop_state = state;
  
  switch(state) {

   case _thread_in_native:
    if (SafepointSynchronize::can_be_at_safepoint_before_suspend(_thread, state)) {
      roll_forward(_at_safepoint);       
      return;
    }
    break;

   case _thread_in_native_blocked:
   case _thread_blocked:
    if (SafepointSynchronize::can_be_at_safepoint_before_suspend(_thread, state)) {      
      roll_forward(_at_safepoint);
      return;
    }
    break;

   case _thread_new:   
     // Let new thread run until it is done getting local storage
     // (may require a malloc). Note that we have not suspended the
     // thread here, so we do not resume it.
     roll_forward(_running, NULL, true);
     return;

   case _thread_in_vm:
     roll_forward(_call_back);     
     return;
  }

  // Check for a thread that is suspended. Note that thread resume tries
  // to grab the Threads_lock which we own here, so a thread cannot be
  // resumed during safepoint synchronization. 

  // We check with locking because another thread that has not yet
  // synchronized may be trying to suspend this one.
  bool is_suspended = _thread->is_any_suspended_with_lock();
  if (is_suspended) {
    // The other suspension can be either internal (non-safepoint) or
    // external so we can see non-cooperative suspension of both native
    // threads and Java threads. We used to assert() specific thread
    // states here:
    //
    // _thread_in_Java         - suspended while in Java code
    // _thread_new_trans       - suspended while just started
    // _thread_in_native       - suspended while in native code
    // _thread_in_native_trans - suspended while in ~ThreadInVMfromNative
    //
    // A race between JNI DestroyJavaVM(), safepointing and a daemon
    // JavaThread would have added the following to the list:
    //
    // _thread_blocked_trans   - suspended coming out of ObjectMonitor.wait()
    //
    // Code inspection revealed that _thread_in_vm_trans was also
    // possible in the last scenario. The list seems endless...
    //
    // The assertion has been removed since we have few remaining
    // JavaThreadState values and no good theories about why they
    // cannot occur here.
    // 
    roll_forward(_at_safepoint); 
    return;
  }

  // Iterate through the safepoint loop without suspending threads in the
  // hope that threads would block or suspend themselves
  if (iterations < DeferThrSuspendLoopCount) {
    return;
  }

  // At this point it was either _thread_in_Java or one of the transition state
  // Suspend thread and examine state
#ifndef PRODUCT
  if (PrintSafepointStatistics) {
    SafepointSynchronize::log_suspension();
  }
#endif // !PRODUCT

  long ret = _thread->vm_suspend();
  if (ret>1) {
    // Thread already suspended. Should only could happen if profiler is active
    SafepointSynchronize::safepoint_msg("Looking at suspended thread (is profiler active?)");    
  }

  // Reread thread_state for thread now it is blocked
  state = _thread->thread_state();  
  _stop_state = state; // temp. debugging
  
  // Normalize all xx_trans states to normal states  
  switch(state) {      
   case _thread_in_native_trans:   
     // The _last_sp might not be setup correctly, if we are just about to enter the
     // VM again, hence, we avoid counting this transition as being a safepoint.
     roll_forward(_running); 
     return;

   case _thread_in_vm_trans:
     if (SafepointSynchronize::can_be_at_safepoint_after_suspend(_thread, state)) {
       roll_forward(_at_safepoint_suspended); 
       return;
     }
     // we can't stop here because some important thread state. The last_java_frame
     // might not work, since the return address has been popped of the stack on return
     // from the native method
     roll_forward(_running);
     return;

   case _thread_blocked_trans:
   case _thread_new_trans:         
     roll_forward(_at_safepoint_suspended); 
     return;

   case _thread_in_native:
   case _thread_blocked:
   case _thread_new:   
   case _thread_in_native_blocked:
     // It is critical that we not keep a thread in native code suspended,
     // since it could hold a lock in a C library (such as malloc) and thereby
     // deadlocking the vm

     // If possible, this call will make the stack walkable
     SafepointSynchronize::save_thread_state_at_safepoint(thread());

     // We can only be at a safepoint if the stack is walkable
     if (! _thread->has_last_Java_frame() || _thread->frame_anchor()->walkable()) {
       roll_forward(_at_safepoint);
       _thread->vm_resume(false);  // don't yield
     } else {
       roll_forward(_running);
     }
     return;

   case _thread_in_vm:
     roll_forward(_call_back);
     _thread->vm_resume(false);  // don't yield
     return; 
  }

  // Examine state of thread
  // get current frame for stopped thread  
  frame fr;
  if (!_thread->get_top_frame(&fr, &_addr)) {
    // get_top_frame can fail on Solaris due to a bug in the Solaris signal code. It also
    // returns false on Sparc if the current instruction is in a delay slot
    // Simply printing the following message changes the I/O profile of the VM thread's lwp
    // and on Solaris may render priority boosting useless (I/O increases lwp priority)
    // SafepointSynchronize::safepoint_msg("Thread is in unknown code. [id: %2x]", _thread->osthread()->thread_id());

    // get_top_frame() can fail on Linux when we have a race between a
    // pending external suspend and the current safepoint on a thread
    // that is being attached. The next time we retry, we will catch
    // the thread in the is_any_suspended_with_lock() check above.

    // help the thread make progress
    Thread::boost_priority(_thread, Thread::get_priority(VMThread::vm_thread()));
    roll_forward(_running);  
    return;
  }

  // contained_pc seem to be the pc where execution will next occur.
  // On x86 this is just pc, but on sparc this in npc. This has
  // implications when looking at whether it seems we are executing in
  // a zombie method further below.
  //
  address pc = _addr.contained_pc();
  _stop_pc = pc;  

  guarantee(state == _thread_in_Java, "must be in Java code at this point");  

  // We handle VM stub routines by restarting the thread again, and then stop it later.
  if (StubRoutines::contains(pc)) {    
    SafepointSynchronize::safepoint_msg("Thread in VM stub code: %s. pc: " INTPTR_FORMAT "  [id: %2x]",
					StubCodeDesc::name_for(pc), pc, _thread->osthread()->thread_id()); 
    // help the thread make progress    
    roll_forward(_running);
    return;
  } 
    
#ifndef CORE
  // Compiled code or native method stub
  // NOTE: we can not use find_blob here because there is a very small
  // window in which we may be in the inline cache check for a
  // previously zombied method (that is, before the zombie trap at the
  // verified entry point which will cause re-resolution of the call
  // site). In this situation the pc will be in the zombied method and
  // the guarantee in find_blob will fail. To solve this (very
  // uncommon) problem we temporarily resume a thread found in this
  // state.
  CodeBlob* cb = CodeCache::find_blob_unsafe(pc);
  if (cb != NULL) {    
    if (cb->is_java_method()) {            
      nmethod* nm = (nmethod*) cb;
      // if thread is stopped before or at verified entry point, resume 
      // and try again later - 4832542
      // On sparc contained pc is npc and can be a single instruction passed 
      // the verified_entry_point, which is now an instruction preventing entry
      if ((uintptr_t)_addr.pc() <= (uintptr_t)nm->verified_entry_point()) {
        roll_forward(_running);
        return;
      } else {
        assert(!cb->is_zombie(), "should not be running a zombie method beyond verified entry point");
        roll_forward(_compiled_safepoint, nm);
        return;
      }
    } else {
      // Thread is in stub code. We run the thread for a little while again...            
      SafepointSynchronize::safepoint_msg("Thread in native stub.");
      // help the thread make progress      
      roll_forward(_running);
      return;
    }       
  }
#endif // !CORE

  // In interpreter
#ifdef CC_INTERP
   if (fr.is_interpreted_frame()) {
      // help the thread make progress      
      roll_forward(_running);
      return;
   }
#else
   // QQQ this seems to really ought to use is_interpreted_frame but is_interpreted_frame
   // on SPARC need to check npc which we don't have. Hmm.
   // 
   if (Interpreter::contains(pc)) {
    InterpreterCodelet* clet = Interpreter::codelet_containing(pc);
    assert(clet != NULL, "code in interpreter must be in a codelet");
    if (UseCallBackInInterpreter && clet->is_safepoint_safe()) {
      roll_forward(_call_back);
      // Do we really want to do a yield for it to run again?
      _thread->vm_resume(true);  // yield
      return;
    } else {
      // help the thread make progress      
      roll_forward(_running);
      return;
    }    
  }
#endif
   
  // Thread is in an unknown (or unhandled) region of the code
  // On Solaris, merely printing a message (from within the examine state loop) will boost
  // VM thread's priority.
  // SafepointSynchronize::safepoint_msg("Thread is in unknown code. pc: " INTPTR_FORMAT " [id: %2x]",
  //					 pc, _thread->osthread()->thread_id());
  
  // help the thread make progress  
  roll_forward(_running);  
}

// Returns true is thread could not be rolled forward at present position.
void ThreadSafepointState::roll_forward(suspend_type type, nmethod *nm, bool disable_resume_for_running_thread) {
  _type = type;  

  switch(_type) {
    case _at_safepoint:
    case _at_safepoint_suspended:
      SafepointSynchronize::save_thread_state_at_safepoint(thread());
      SafepointSynchronize::signal_thread_at_safepoint();
      break;

    case _running:            
      if (!disable_resume_for_running_thread) {
	Thread::boost_priority(thread(), Thread::get_priority(VMThread::vm_thread()));
	thread()->vm_resume(false);  // don't yield
      }
      break;
    
    case _call_back:
      thread()->set_has_called_back(false);
      break;
         
    case _compiled_safepoint:      
      assert(nm != NULL, "handler must be set");
      assert(_stop_pc != NULL, "must be set");      
      assert(handle() != NULL, "must be set");
      handle()->setup(this, nm);
      break;

    default:
      ShouldNotReachHere();
  }      
}  

void ThreadSafepointState::restart() {  
  switch(type()) {        
    case _at_safepoint:               
    case _call_back:
      break;    
    
    case _at_safepoint_suspended:
      thread()->vm_resume(false);  // don't yield
      break;
        
    case _compiled_safepoint:
      assert(handle() != NULL, "handler must be set (it's preallocated)");
      _handle->release(this);      
      break;    

    default:
      ShouldNotReachHere();
  }    
  _type = _running;  
}


bool ThreadSafepointState::caller_must_gc_arguments() const {
  assert(_handle != NULL, "does not make sense without a handler");
  return _handle->caller_must_gc_arguments();
}

void ThreadSafepointState::print() {
  const char *s;

  switch(_type) {    
    case _running                : s = "_running";              break;    
    case _at_safepoint           : s = "_at_safepoint";         break;
    case _at_safepoint_suspended : s = "_at_safepoint_suspend"; break;
    case _call_back              : s = "_call_back";            break;    
    case _compiled_safepoint     : s = "_compiled_handler";     break;
    default:
      ShouldNotReachHere();
  }
  
  tty->print_cr("Thread: " INTPTR_FORMAT "  [0x%2x] State: %s  pc: " INTPTR_FORMAT, 
		_thread, _thread->osthread()->thread_id(), s, _stop_pc);

  if (_type == _compiled_safepoint) {
    assert(handle() != NULL, "handler must be set");
    handle()->print();
  }
}

#ifndef CORE
ThreadCodeBuffer* ThreadSafepointState::allocate_code_buffer(int size_in_bytes, nmethod* nm, address real_pc) {
  assert(_code_buffer == NULL, "buffer must be destroyed before allocating");
  _code_buffer_is_enabled =false;
  _code_buffer = new ThreadCodeBuffer(size_in_bytes, nm, real_pc);
  return _code_buffer;
}

void ThreadSafepointState::destroy_code_buffer() {
  assert(_code_buffer != NULL, "cannot destroy non-existing buffer");
  _code_buffer_is_enabled = false;
  delete _code_buffer;
  _code_buffer = NULL;
}

// On Windows and Linux, this is called from VM thread; on Solaris, this is
// called from signal handler of the thread we are trying to bring to a
// compiled safepoint. Target thread is still suspended.
void ThreadSafepointState::notify_set_thread_pc_result(bool success) {
  // 4720694 If we can't restart thread in thread code buffer, we must disable 
  // its code buffer while the thread is still suspended. Don't free memory
  // here as we might get called from a signal handler. Code buffer will be
  // deallocated later in VM thread.
  if (success) {
     enable_code_buffer();
  } else {
     disable_code_buffer();
  }
}
#endif // !CORE


// ---------------------------------------------------------------------------------------------------------------------
// Implementation of general SafepointHandler

void SafepointHandler::set_state_to_running() {    
  thread()->safepoint_state()->roll_forward(ThreadSafepointState::_running);  
}


// ---------------------------------------------------------------------------------------------------------------------
// Implementation of SafepointHandler for compiled code (not in CORE Version)
#ifndef CORE  

void CompiledCodeSafepointHandler::setup(ThreadSafepointState *state, nmethod* nm) { 
  ResourceMark rm;
    
  // Bail out if compiler safepoints are disabled
  if (!UseCompilerSafepoints) {       
    set_state_to_running();
    return;
  } 
  
  _nm = nm;
  JavaThread* thread = state->thread();

  // This routine handles both the case where a thread is stopped in:
  // - an nmethod, or
  // - a leaf call from an nmethod
  //
  // Most parts of the code is the same. The difference is how we find which
  // nmethod to copy andpatch, and how to restart the thread.
  //
  address pc = NULL;
  frame caller_fr;
  assert(thread->thread_state() == _thread_in_Java, "wrong thread state");
  pc = state->current_address().contained_pc();
  safepoint_msg("### Compiled code: %s. pc: " INTPTR_FORMAT, nm->method()->name()->as_C_string(), pc);
    
  // It it in body, exception, or stub
  if (nm->stub_contains(pc) || nm->exception_contains(pc)) {      
    // we restart thread in these cases for now
    safepoint_msg("exception/stub part");      
    set_state_to_running();
    return;
  }  
  assert(nm->code_contains(pc), "must be in body part");
  
  // Allocate code buffer for roll-forward.  We currently copy entire code-buffer.
  int size_in_bytes = pd_thread_code_buffer_size(nm);
  ThreadCodeBuffer *cb = state->allocate_code_buffer(size_in_bytes, nm, nm->instructions_begin() );        
  if(ShowSafepointMsgs) {
    safepoint_msg("############# Original nmethod ################");
    nm->print();
  }

  // Step 1: Make a copy of code.  Code is handled specially since it must be relocated.
  cb->copy_code(0, nm->instructions_begin(), nm->instructions_size());            

  if (ShowSafepointMsgs) {
    tty->print_cr("Original code: [" INTPTR_FORMAT " - " INTPTR_FORMAT "]", nm->instructions_begin(), nm->instructions_end());
    tty->print_cr("Code buffer:   [" INTPTR_FORMAT " - " INTPTR_FORMAT "]", cb->code_begin(), cb->code_end());
    tty->print_cr("Orignal pc:     " INTPTR_FORMAT, pc);
  }
  
  // Step 2: Patch all calls and backwards jumps out of the code
  RelocIterator iter(nm);
  while (iter.next()) {
    address reloc_pc = iter.addr();
    if (reloc_pc >= nm->code_end())
    break;
            
    address cb_pc = cb->code_begin() + (reloc_pc - nm->instructions_begin());
    assert(cb->contains(cb_pc), "pc must be in temp. buffer");            

    switch(iter.type()) {	
      // Handle all patches of calls
	        
      case relocInfo::virtual_call_type:     // virtual call
      case relocInfo::opt_virtual_call_type: // optimized virtual call
      case relocInfo::static_call_type:      // static call             
        if (ShowSafepointMsgs && Verbose) { 
          tty->print_cr("Illegal instruction at " INTPTR_FORMAT " for invoke (org. pc: " INTPTR_FORMAT ")", cb_pc, reloc_pc); 
        }
        assert(((NativeInstruction*)cb_pc)->is_call(), "must be a call");
	NativeIllegalInstruction::insert(cb_pc);
        break;

      case relocInfo::return_type:
        if (ShowSafepointMsgs && Verbose) { 
          tty->print_cr("Illegal instruction at " INTPTR_FORMAT " for return (org. pc: " INTPTR_FORMAT ")", cb_pc, reloc_pc); 
        }	
        SafepointSynchronize::patch_return_instruction_md(cb_pc);
        break;

      case relocInfo::runtime_call_type:
        if (ShowSafepointMsgs && Verbose) { 
          tty->print_cr("ignoring runtime call at " INTPTR_FORMAT " (org. pc: " INTPTR_FORMAT ")", cb_pc, reloc_pc); 
        }
        // Nothing to do - just for debugging
        break;
            
      case relocInfo::safepoint_type:  
        // Could be any instruction for C1, except a conditional jump
        NOT_COMPILER1(assert(((NativeInstruction*)cb_pc)->is_nop(), "must be a nop");)
        NOT_COMPILER2(assert(!((NativeInstruction*)cb_pc)->is_cond_jump(), "must not be a conditional jump");)
      	NativeIllegalInstruction::insert(cb_pc);
        break;      

      default:
	// skib
	break;
    } // switch
  } // while

  pd_patch_runtime_calls_with_trampolines(cb, nm->instructions_size());

  if (ShowSafepointMsgs && Verbose) {
    tty->print_cr("Repositioning thread found in nmethod");
  }
  // If thread was stopped in the nmethod, we simply reposition it into the new code
  // This may fail in some rare cases under Solaris - back off if it does
  if(!cb->reposition_and_resume_thread(thread, state->current_address())) {
    // Validate that this failure happened in a manner we can handle
    assert(thread->osthread()->valid_reposition_failure(), "Unexpected reposition failure");
    state->destroy_code_buffer();
    
    safepoint_msg("failed to reposition thread, resetting state to running");
    
    // No need to resume it 
    thread->safepoint_state()->roll_forward(ThreadSafepointState::_running, NULL, true);  
  }
}  

void CompiledCodeSafepointHandler::check_has_escaped(CodeBlob* stub_cb) {
  if (stub_cb != NULL &&
    (stub_cb->is_safepoint_stub() || stub_cb->is_runtime_stub() ||
    stub_cb->is_exception_stub() || stub_cb->is_uncommon_trap_stub() ||
    COMPILER1_ONLY(false)
    COMPILER2_ONLY(stub_cb->instructions_begin() == OptoRuntime::rethrow_stub())
)) {
  } else {
    // Print the name of the method that has escaped in the fatal error.
    ResourceMark rm;
    fatal1("method escaped CompiledCodeSafepointHandler %s",
           get_nmethod()->method()->name_and_sig_as_C_string());
  }
}


void CompiledCodeSafepointHandler::release(ThreadSafepointState *state) {  
  assert(state->thread()->thread_state() == _thread_blocked, "thread_state should have changed, since it is blocked");

  if (!UseCompilerSafepoints) {      
    return;
  }  

  ThreadCodeBuffer* cbuf = thread()->safepoint_state()->code_buffer();
  if (cbuf != NULL) {            
    frame stub_fr = thread()->last_frame();
    CodeBlob* stub_cb = CodeCache::find_blob(stub_fr.pc());  
    // Guarantee there was no escape from the compiled method.
    check_has_escaped(stub_cb);

    // If the thread stopped in a runtime operation, rather than at a compiled safepoint, change the runtime stub's
    // return address from a location in the ThreadCodeBuffer to the corresponding location in the original nmethod.
#ifdef COMPILER1
    bool stub_is_jvmpi_method_exit = stub_cb->instructions_begin() == Runtime1::entry_for(Runtime1::jvmpi_method_exit_id);
    if (stub_is_jvmpi_method_exit) {
      // This stub is frameless (uses the nmethod's frame) to avoid deoptimization,
      // so its return address is not in the conventional location.
      address return_addr = stub_fr.frameless_stub_return_addr();
      assert(cbuf->contains(return_addr), "CompiledCodeSafepointHandler::release: wrong return address from jvmpi stub");
      safepoint_msg("adjusting return address");
      return_addr  = cbuf->compute_adjusted_pc(return_addr);
      stub_fr.patch_frameless_stub_return_addr(thread(), return_addr);
    } else 
#endif
    {
      // Adjust the return address only when returning to the ThreadCodeBuffer.
      RegisterMap map(thread(), false);    
      frame caller_fr = stub_fr.sender_for_raw_compiled_frame(&map);
      address return_addr = caller_fr.pc();
      if (cbuf->contains(return_addr)) {      
        safepoint_msg("adjusting return address");
        return_addr  = cbuf->compute_adjusted_pc(return_addr);
        caller_fr.patch_pc(thread(), return_addr);	
      }
    }
    state->destroy_code_buffer();
  }
}


// Return non zero-positive if we are stopped at a return that must pop input
// arguments (UseNewCallingConvention = false && ret x, with x > 0)
// Return negative if we have deoptimized at a blocked call.  In that case
// we must continue at the destination instead of continuing in the blocked 
// method. 
int CompiledCodeSafepointHandler::handle_illegal_instruction_exception() {    

  // Step 0: preserve the thread frame state
  JavaFrameAnchor orig_state(thread()->frame_anchor());

  // Step 1: find references to SafepointState and ThreadCodeBuffer
  ThreadSafepointState *state = thread()->safepoint_state();
  ThreadCodeBuffer     *cbuf   = state->code_buffer();  
  assert(cbuf != NULL, "for pc to be in code buffer the codebuffer must exist");
  nmethod* nm = get_nmethod();

  // Step 2: Find frame of caller  
  frame stub_fr = thread()->last_frame();
  CodeBlob* stub_cb = CodeCache::find_blob(stub_fr.pc());
  assert(stub_cb->is_safepoint_stub(), "must be a safepoint stub");      
  RegisterMap map(thread(), false);  
  frame caller_fr = stub_fr.sender_for_raw_compiled_frame(&map);   
  
  // Step 3: Calculate real return address     
  if (ShowSafepointMsgs && Verbose) {
    tty->print_cr("illegal instruction exception at " INTPTR_FORMAT, thread()->saved_exception_pc());
  }
  address real_return_addr = cbuf->compute_adjusted_pc(thread()->saved_exception_pc());  
  thread()->set_saved_exception_pc(NULL); // clear exception pc

  // Find relocInfo::relocType for the address where we get called, so we can figure out how to
  // preserve the oops.  

  relocInfo::relocType type = nm->reloc_type_for_address(real_return_addr);

  // Default case. We we are blocked at a call, then the caller must GC args
  _caller_must_gc_args = true;

  // set to non zero if using old calling convention and the safepoint
  // happened at a return where the return must remove its arguments
  int arg_size_in_bytes = 0; 
  
  switch(type) {	
    // Handle all patches of calls

    case relocInfo::virtual_call_type:	        // inline cache call
      safepoint_msg("ic_call_type");               
      { HandleMark hm;
        Handle receiver(NULL);
        Handle cached_klass(NULL);
#ifdef COMPILER1
        receiver = caller_fr.saved_receiver();
        oop saved_oop = caller_fr.saved_oop_result(); 
        if (Universe::is_non_oop(saved_oop)) {
          // cached klass remains NULL
        } else {
          cached_klass = saved_oop;
        }
#endif // COMPILER1
        // Move return address after call, so exact oopmap lookup will work      
        caller_fr.patch_pc(thread(), real_return_addr + NativeCall::instruction_size);
        // Block
        SafepointSynchronize::block(thread()); 
        // Adjust return address so mov in inline cache is reexecuted      
      
        real_return_addr -= NativeMovConstReg::instruction_size;
#ifdef COMPILER1
        caller_fr.set_saved_receiver(receiver());
        if (cached_klass.not_null()) {
          // an oop has been preserved
          caller_fr.set_saved_oop_result(cached_klass());
        }
#endif // COMPILER1
      }
      break;

    case relocInfo::static_call_type:       // static call  	        
      safepoint_msg("static_call_type");
      // Move return address after call, so exact oopmap lookup will work     
      caller_fr.patch_pc(thread(), real_return_addr + NativeCall::instruction_size);
      // Block
      SafepointSynchronize::block(thread());       
      break;

    case relocInfo::opt_virtual_call_type:  // optimized virtual call
      safepoint_msg("static_call_type");
      { HandleMark hm;
        Handle receiver(NULL);
        COMPILER1_ONLY(receiver = caller_fr.saved_receiver();)
        // Move return address after call, so exact oopmap lookup will work     
        caller_fr.patch_pc(thread(), real_return_addr + NativeCall::instruction_size);
        // Block
        SafepointSynchronize::block(thread());       
        COMPILER1_ONLY(caller_fr.set_saved_receiver(receiver());)
      }
      break;

    case relocInfo::return_type:            
      { safepoint_msg("return_type");        
        HandleMark hm;

        // All arguments are dead at the return. Caller must ignore arguments
        _caller_must_gc_args = false;

        // See if return type is an oop.
        bool return_oop = nm->method()->is_returning_oop();
        Handle return_value;
        if (return_oop) {
          // The oop result has been saved on the stack together with all
          // the other registers. In order to preserve it over GCs we need
          // to keep it in a handle.
          return_value = caller_fr.saved_oop_result();  
          assert(Universe::heap()->is_in_or_null(return_value()), "must be heap pointer");          
        }

        // compute the number of arguments to pop
	// Do this before blocking as nmethod could be zombie by the time
	// we unblock.
        nmethod* nm = CodeCache::find_nmethod(real_return_addr);
        assert(nm != NULL, "cannot find nmethod");
        arg_size_in_bytes = (UseC2CallingConventions || UseFixedFrameSize) ? 0 : 
                              nm->method()->size_of_parameters() * wordSize;
        
        SafepointSynchronize::block(thread());
        
        // restore oop result, if any
        if (return_oop) {
          caller_fr.set_saved_oop_result(return_value());        
        }       

        real_return_addr = caller_fr.pc();    // No change        

        // If the popframe_condition field is exactly popframe_pending_bit
        // (which is why has_pending_popframe() is not used), then we add
        // the popframe_compiled_return_bit to indicate that we passed
        // through this portion of the handler.
        if (thread()->popframe_condition() == JavaThread::popframe_pending_bit) {
          thread()->set_popframe_condition_bit(
            JavaThread::popframe_compiled_return_bit);
        }
      }
      break;

    case relocInfo::safepoint_type:
       safepoint_msg("safepoint_type");
       // We are not at a call
       _caller_must_gc_args = false; 
       thread()->set_pc_not_at_call_for_frame(caller_fr.id());

       // Setup return address in stub, so stack traversal works
       caller_fr.patch_pc(thread(), real_return_addr);
       SafepointSynchronize::block(thread());              
       break;
    
    default:
      safepoint_msg("Unexpected relocType: %d", type);
      ShouldNotReachHere();
  }  

  // Deoptimize frame if exception has been thrown. 
  if (thread()->has_pending_exception()) {    
    safepoint_msg("Exception installed during safepoint");   
#ifdef COMPILER1
    if (type == relocInfo::safepoint_type) {
      thread()->set_is_handling_implicit_exception(true); // will be reset in Runtime1::exception_handler_for_pc
    }
#else
    // Deoptimize frame. The compiler might not have any safepoint state at this point, so the frame
    // must have been deoptimized. The deoptimization happended in the SafepointSynchronize::end method     
#endif // COMPILER1
  } else {
    // patch return address to point to code in nmethod: it might have been changed above;
    // e.g., for an inline cache the real_return_addr has been moved back to also include
    // the move instruction for the inline cache.
    //
    // There is one situation where patching the real return address is wrong:
    // The calling activation might have been deoptimized while being blocked.
    // If the calling activation has been deoptimized we must leave the
    // return address pointing to the deoptimization stub. In all other
    // cases the real_return_address will point into the caller frame.
    frame stub_fr = thread()->last_frame();
    RegisterMap map(thread(), false);  
    frame caller_fr = stub_fr.sender_for_raw_compiled_frame(&map);   
    CodeBlob* caller_cb = CodeCache::find_blob(caller_fr.pc());
    // For the safepoint type "return_type", we've already popped the
    // frame of the calling compiled method. If the caller to the
    // compiled method which took the safepoint was itself compiled
    // code, this patch is a no-op; see the return_type arm of the
    // switch statement above. However, if the caller to the
    // safepointed method was interpreted, then the caller frame's PC
    // will be the interpreter's return entry point and the above
    // find_blob call will fail. This is actually okay.
    if ((caller_cb != NULL) && (!caller_cb->is_deoptimization_stub())) {
      assert(caller_cb->contains(real_return_addr), "current caller frame must contain return address");
      caller_fr.patch_pc(thread(), real_return_addr);
      if (ShowSafepointMsgs && Verbose) {
        tty->print_cr("patch the caller frame pc to " INTPTR_FORMAT, real_return_addr);
      }
    } else {
      if (ShowSafepointMsgs && Verbose) tty->print_cr("Deoptimizing at a safepoint type %d", type);
      if (   type == relocInfo::virtual_call_type 
          || type == relocInfo::static_call_type
          || type == relocInfo::opt_virtual_call_type ) {
        // if there is no pending exception, the call may be skipped as the 
        // deoptimization at call will cause the interpreter to execute the next instruction
        if (!thread()->has_pending_exception()) {
          if (ShowSafepointMsgs && Verbose) warning("deoptimizing at a call at a compiler safepoint, will skip call -> problem known");
#ifdef ASSERT
          COMPILER1_ONLY(DeoptimizationBlob* deopt_blob = (DeoptimizationBlob*) Runtime1::blob_for(Runtime1::deoptimization_handler_id););
	  COMPILER2_ONLY(DeoptimizationBlob* deopt_blob = OptoRuntime::deoptimization_blob());
          assert(deopt_blob != NULL, "deoptimization blob must have been created");
          assert(caller_fr.pc() == deopt_blob->unpack(), "cannot handle this address");
#endif // ASSERT
          assert(arg_size_in_bytes == 0, "only return_type sets it to non-zero");
          // find the destination address
          // as we planned to reexecute the call, real_return_addr points to the start 
          // of the call instruction

          // compute the address of the call (real_return_addr may point at the IC move)
          address at_call = type == relocInfo::virtual_call_type ?
                                      real_return_addr + NativeMovConstReg::instruction_size
                                    : real_return_addr;

          NativeCall* call = nativeCall_at(at_call);
          // replace return address with the destination of the call,
          // so that the stub can use it; the stub will replace the
          // return address with the entry into the deoptimization blob.
          caller_fr.patch_pc(thread(), call->destination());

          arg_size_in_bytes = -1; // notify that compilation must be reexecuted
        } else {
          if (ShowSafepointMsgs) {
            tty->print_cr("Has pending exception");
          }
        }
      }
    }
  }
      
  // flush any inadvertently cached frame state by restoring the entry state
  thread()->frame_anchor()->copy(&orig_state);

  return arg_size_in_bytes;
}


void CompiledCodeSafepointHandler::print() {
  ResourceMark rm;
  tty->print_cr("%s: %s", name(), _nm->method()->name_and_sig_as_C_string());
}


#endif // !CORE

// ------------------------------------------------------------------------------------------------
// Non-product code

#ifndef PRODUCT
//
// Statistics
// 

long      SafepointSynchronize::_total_safepoints = 0;
long      SafepointSynchronize::_total_threads    = 0;
long      SafepointSynchronize::_max_threads      = 0;
double    SafepointSynchronize::_total_secs       = 0;
double    SafepointSynchronize::_max_secs         = 0;
double    SafepointSynchronize::_min_secs         = 3600; // One hour (should be high enough)
TimeStamp SafepointSynchronize::_timer;
long      SafepointSynchronize::_num_of_suspends = 0;
long      SafepointSynchronize::_cumm_num_of_suspends = 0;

void SafepointSynchronize::begin_statistics(int nof_threads) {
  _total_safepoints += 1;
  _total_threads    += nof_threads;
  if (nof_threads>_max_threads)
    _max_threads = nof_threads;
  _num_of_suspends = 0;

  _timer.update();
}

void SafepointSynchronize::end_statistics() {
  double secs = _timer.seconds();
  
  _total_secs += secs;
  if (_min_secs>secs) _min_secs = secs;
  if (_max_secs<secs) _max_secs = secs;
  _cumm_num_of_suspends += _num_of_suspends;

  print_statistics();
}

void SafepointSynchronize::print_statistics() {
  tty->print_cr("Safepoint#%Ld: Threads:(avg: %3.1f, max: %3Ld), Time: (avg: %2.5f, min: %2.5f, max:%2.5f), Suspends: (current: %Ld, cummulative: %Ld)",      
    _total_safepoints,
    (double) _total_threads/_total_safepoints,
    _max_threads,
    _total_secs/_total_safepoints,
    _min_secs,
    _max_secs,
    _num_of_suspends,
    _cumm_num_of_suspends);
}

void SafepointSynchronize::print_state() {
  if (_state == _not_synchronized) {
    tty->print_cr("not synchronized");
  } else if (_state == _synchronizing || _state == _synchronized) {
    tty->print_cr("State: %s", (_state == _synchronizing) ? "synchronizing" : "synchronized");

    for(JavaThread *cur = Threads::first(); cur; cur = cur->next()) {       
       cur->safepoint_state()->print();
    }  
  } 
}

void SafepointSynchronize::safepoint_msg(const char* format, ...) {
  if (ShowSafepointMsgs) {
    va_list ap;
    va_start(ap, format);
    tty->vprint_cr(format, ap);
    va_end(ap);
  }
}

#endif // !PRODUCT

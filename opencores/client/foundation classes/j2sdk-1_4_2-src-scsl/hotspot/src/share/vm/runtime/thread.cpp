#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)thread.cpp	1.659 03/05/08 14:38:56 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_thread.cpp.incl"

// Class hierarchy
// - Thread
//   - VMThread
//   - WatcherThread
//   - ConcurrentMarkSweepThread
//   - SuspendCheckerThread
//   - JavaThread
//     - CompilerThread

// ======= Thread ========

// Base class for all threads: VMThread, WatcherThread, ConcurrentMarkSweepThread,
// SuspendCheckerThread, JavaThread

Thread::Thread() {
  // stack  
  _stack_base   = NULL;
  _stack_size   = 0;
  _self_raw_id  = 0;
  _osthread     = NULL;

  // allocated data structures
  set_resource_area(new ResourceArea());
  set_handle_area(new HandleArea(NULL));
  set_active_handles(NULL); 
  set_free_handle_block(NULL);
  set_last_handle_mark(NULL);
  set_osthread(NULL);

  // This initial value ==> never claimed.
  _oops_do_parity = 0;

  // the handle mark links itself to last_handle_mark
  new HandleMark(this);

  // plain initialization  
  debug_only(_owned_locks = NULL;) 
  debug_only(_allow_allocation_count = 0;)
  NOT_PRODUCT(_allow_safepoint_count = 0;)  
  _highest_lock = NULL;
  _vm_operation_started_count = 0;
  _vm_operation_completed_count = 0;
  set_unboosted_priority(NoPriority);
  _pending_exception = NULL;
  _rawmonitor_list = NULL;
  _current_pending_monitor = NULL;
  _current_pending_monitor_is_from_java = true;
  _current_waiting_monitor = NULL;

  _SR_lock = new Monitor(Mutex::suspend_resume, "SR_lock", true);
  _suspend_flags = _is_baby_thread;
  _vm_suspend_depth = 0;
}

void Thread::initialize_thread_local_storage() {
  // Note: Make sure this method only calls 
  // non-blocking operations. Otherwise, it might not work
  // with the thread-startup/safepoint interaction.

  // During Java thread startup, safepoint code should allow this
  // method to complete because it may need to allocate memory to
  // store information for the new thread.

  // initialize structure dependent on thread local storage
  ThreadLocalStorage::set_thread(this);
  
  // set up any platform-specific state.
  os::initialize_thread();

}

void Thread::delete_thread_and_TLS_current() {
  bool mythread = (Thread::current() == this); 
  delete this;
  // clear thread local storage
  if (mythread) {
    ThreadLocalStorage::set_thread(NULL);
  }
}

void Thread::record_stack_base_and_size() {
  set_stack_base(os::current_stack_base());
  set_stack_size(os::current_stack_size());
}


Thread::~Thread() {
  // deallocate data structures
  delete resource_area();
  // since the handle marks are using the handle area, we have to deallocated the root
  // handle mark before deallocating the thread's handle area,
  assert(last_handle_mark() != NULL, "check we have an element");
  delete last_handle_mark();
  assert(last_handle_mark() == NULL, "check we have reached the end");

  delete handle_area();

  // osthread() can be NULL, if creation of thread failed.
  if (osthread() != NULL) os::free_thread(osthread());
  
  // clear thread local storage - moved to after delete calls
  // so heap tracing would work

  delete _SR_lock;
}

// NOTE: dummy function for assertion purpose.
void Thread::run() {
  ShouldNotReachHere();
}

#ifdef ASSERT
// Private method to check for dangling thread pointer
void check_for_dangling_thread_pointer(Thread *thread) {
 assert(!thread->is_Java_thread() || Thread::current() == thread || Threads_lock->owned_by_self(),
         "possibility of dangling Thread pointer");
}
#endif


#ifndef PRODUCT
// Tracing method for basic thread operations
void Thread::trace(const char* msg, Thread* thread) {    
  if (!TraceThreadEvents) return;
  ResourceMark rm;
  ThreadCritical tc;
  const char *name = "non-Java thread";
  int prio = -1;
  if (thread->is_Java_thread() 
#ifndef CORE
      && !thread->is_Compiler_thread()
#endif
      ) {
    // The Threads_lock must be held to get information about
    // this thread but may not be in some situations when
    // tracing  thread events.
    bool release_Threads_lock = false;
    if (!Threads_lock->owned_by_self()) {
      Threads_lock->lock();
      release_Threads_lock = true;
    }
    JavaThread* jt = (JavaThread *)thread;
    name = (char *)jt->get_thread_name();
    oop thread_oop = jt->threadObj();
    if (thread_oop != NULL) {
      prio = java_lang_Thread::priority(thread_oop); 
    }
    if (release_Threads_lock) {
      Threads_lock->unlock();
    }
  }    
  tty->print_cr("Thread::%s " INTPTR_FORMAT " [%lx] %s (prio: %d)", msg, thread, thread->osthread()->thread_id(), name, prio);
}
#endif


ThreadPriority Thread::get_priority(Thread* thread) {  
  trace("get priority", thread);  
  ThreadPriority priority;
  // Can return an error!
  (void)os::get_priority(thread, priority);  
  assert(MinPriority <= priority && priority <= MaxPriority, "non-Java priority found");
  return priority;
}

void Thread::set_priority(Thread* thread, ThreadPriority priority) {
  trace("set priority", thread);
  debug_only(check_for_dangling_thread_pointer(thread);)
  // Can return an error!
  (void)os::set_priority(thread, priority);
}

void Thread::boost_priority(Thread* thread, ThreadPriority priority) {    
  trace("boost priority", thread);
  debug_only(check_for_dangling_thread_pointer(thread);)
  
  thread->set_unboosted_priority(Thread::get_priority(thread));
  Thread::set_priority(thread, priority);
}

void Thread::unboost_priority(Thread* thread) {      
  debug_only(check_for_dangling_thread_pointer(thread);)
  if (thread->unboosted_priority() != NoPriority) {
    trace("unboost priority", thread);
    Thread::set_priority(thread, thread->unboosted_priority());
    thread->set_unboosted_priority(NoPriority);
  }
}

void Thread::start(Thread* thread) {
  trace("start", thread);
  // Start is different from resume in that its safety is guaranteed by context or
  // being called from a Java method synchronized on the Thread object.
  if (!DisableStartThread) {
    os::start_thread(thread);
  }
}

// Enqueue a VM_Operation to do the job for us - sometime later
void Thread::send_async_exception(oop java_thread, oop java_throwable) {    
  VM_ThreadStop* vm_stop = new VM_ThreadStop(java_thread, java_throwable);
  VMThread::execute(vm_stop);
}

// Suspend another thread. Returns the current suspend count.
// Used for safepoint synchronization by the VMThread, flat profiling
// by the WatcherThread, and by both JVM/DI and JVM/PI for suspending
// threads in state _thread_in_native.

int Thread::do_vm_suspend() {
  assert(Thread::current() != this, "vm_suspend() cannot suspend self");
  assert(SR_lock()->owned_by_self(), "must own SR_lock()");

  if (this->is_SuspendChecker_thread()) {
    return 0;  // ignore SuspendCheckerThread
  }

  int depth = this->vm_suspend_depth() + 1;

  if (depth > 1 ) {
    // nesting vm_suspend request, update depth count
    set_vm_suspend_depth(depth);

  } else {
    // this is the first suspend
    assert(depth == 1, "wrong vm_suspend_depth");

    if (this->is_baby_thread()) {
      // Baby thread is unstarted thread. Just set a special flag so we 
      // know how to start it when in vm_resume().
      this->set_suspended_as_baby();

    } else {
      // Normal thread, call OS level suspend function
      int ret = os::pd_suspend_thread(this, true);

      if (ret != 0) {
         assert(false, "os::pd_suspend_thread failed!");
         // Limited recovery in production mode - just return original depth
         // and effectively drop the suspend request.
         return depth - 1;
      }
    }

    set_vm_suspend_depth(depth);
    set_vm_suspended();
  }

  return depth;
}

int Thread::vm_suspend() {
  trace("vm_suspend", this);  
  MutexLockerEx ml(SR_lock(), Mutex::_no_safepoint_check_flag);
  return do_vm_suspend();
}

// Resume a vm-suspended thread by one level. Return the new vm-suspend
// count. The thread will start to run again when vm_suspend_depth reaches 0.
// It can only be used to resume vm-suspended thread.
// Called by the VMThread for safepoint synchronization or the WatcherThread
// for flat profiling or by JavaThread::java_resume() when target thread
// is suspended in _thread_in_native state (is_suspended_in_native()).

int Thread::do_vm_resume(bool do_yield) {
  assert(!Thread::current()->is_Java_thread() || Threads_lock->owned_by_self(),
    "JavaThread calling vm_resume must hold Threads_lock");
  assert(SR_lock()->owned_by_self(), "must own SR_lock()");
  assert(this->is_vm_suspended(), "must be vm_suspended");

  int depth = vm_suspend_depth() - 1;

  if (depth > 0) {
    set_vm_suspend_depth(depth);

  } else {
    assert(depth == 0, "wrong vm_suspend_depth");

    set_vm_suspend_depth(depth);
    clear_vm_suspended();

    if (this->is_baby_thread()) {
      // baby thread is unstarted thread, just clear the flag
      assert(is_suspended_as_baby(), "just checking");
      clear_suspended_as_baby();

    } else {
      // target thread has started, two possibilities here:
      // + common case, the suspension occurred after Thread::start(), or,
      // + special case, target was suspended before Thread::start()
      if (!this->is_suspended_as_baby()) {
        // call OS level resume function
        int ret = os::pd_resume_thread(this);

        if (ret != 0) {
          assert(false, "os::pd_resume_thread failed");
          // Limited recovery in production mode - just reset the thread's
          // state and effectively drop the resume request.
          // XXX - can we do better?
          set_vm_suspend_depth(depth + 1);
          set_vm_suspended();
          
          return depth + 1;
        }
      } else {
        // This is the special case where target was suspended before
        // Thread::start(). We need to start the thread here.
        clear_suspended_as_baby();
        os::pd_start_thread(this);                // We already own SR_lock
      }
    }
  }

  if (do_yield) {
    os::yield_all(0);  // give a resumed thread a chance to be dispatched
  }

  return depth;
}

int Thread::vm_resume(bool do_yield) {
  trace("vm_resume", this);
  MutexLockerEx ml(SR_lock(), Mutex::_no_safepoint_check_flag);
  return do_vm_resume(do_yield);
}

//
// Wait for an external suspend request to complete (or be cancelled).
// Returns true if the thread is externally suspended and false otherwise.
//
bool JavaThread::wait_for_ext_suspend_completion(int retries) {
  // local flags copies to minimize SR_lock hold time
  bool is_suspended;
  bool pending;


  {
    MutexLockerEx ml(SR_lock(), Mutex::_no_safepoint_check_flag);
    pending = is_external_suspend();
    is_suspended = this->is_ext_suspended();
  }
  // must release SR_lock to allow suspension to complete

  for (int i = 0; pending && !is_suspended && i < retries; i++) {

    if (is_exiting()) {
      // thread is in the process of exiting so the wait is done
      return false;
    }

    if (thread_state() == _thread_blocked && is_suspend_equivalent()) {
      // If the thread's state is _thread_blocked and this blocking
      // condition is known to be equivalent to a suspend, then we can
      // consider the thread to be externally suspended. This means that
      // the code that sets _thread_blocked has been modified to do
      // self-suspension if the blocking condition releases. We also
      // used to check for CONDVAR_WAIT here, but that is now covered by
      // the _thread_blocked with self-suspension check.
      //
      // Return true since we wouldn't be here unless there was still an
      // external suspend request.
      return true;
    } else if (suspend_critical() == _native_suspend_critical) {
      // If the thread is in a native SuspendCritical region, then the
      // code that sets _native_suspend_critical has been modified to
      // do self-suspension if the SuspendCritical region is left.
      return true;
    } else if (!UseForcedSuspension && 
                thread_state() == _thread_in_native && frame_anchor()->walkable()) {
      // When UseForcedSuspension is false, threads running native code
      // will self-suspend on native==>VM/Java transitions. If its stack 
      // is walkable (should always be the case unless this function is 
      // called before the actual java_suspend() call), then the wait is done.
      return true;
    }

    // If the VMThread is trying to synchronize all the threads for a
    // safepoint, then we block ourself on the Threads_lock. Our grab
    // of the SR_lock will block us only if it is contended.
    if (SafepointSynchronize::is_synchronizing()) {
      Threads_lock->lock();
      Threads_lock->unlock();
    }
    os::yield_all(i);  // increase yield with each retry
    {
      MutexLockerEx ml(SR_lock(), Mutex::_no_safepoint_check_flag);
      is_suspended = this->is_ext_suspended();

      // It is possible for the external suspend request to be cancelled
      // (by a resume) before the actual suspend operation is completed.
      // Refresh our local copy to see if we still need to wait.
      pending = is_external_suspend();
    }
  }

  // Even if the external suspend request is cancelled, we still might
  // be suspended at this point due to an internal suspend request. But
  // we don't want to tell the caller that because an internal resume
  // will occur on its own which may affect the caller's assumptions.
  // We also don't want to return true if the thread was terminated.
  return (pending && is_suspended && !is_exiting());
}

void Thread::interrupt(Thread* thread) {
  trace("interrupt", thread);
  debug_only(check_for_dangling_thread_pointer(thread);)
  os::interrupt(thread);
}

bool Thread::is_interrupted(Thread* thread, bool clear_interrupted) {
  trace("is_interrupted", thread);  
  debug_only(check_for_dangling_thread_pointer(thread);)
  // Note:  If clear_interrupted==false, this simply fetches and
  // returns the value of the field osthread()->interrupted().
  return os::is_interrupted(thread, clear_interrupted);
}


// Exception handling

void Thread::set_pending_exception(oop exception, const char* file, int line) {
  assert(exception != NULL && exception->is_oop(), "invalid exception oop");
  _pending_exception = exception;
  _exception_file    = file;
  _exception_line    = line;
}


void Thread::clear_pending_exception() {
  if (TraceClearedExceptions) {
    if (_pending_exception != NULL) {
      tty->print_cr("Thread::clear_pending_exception: cleared exception:");
      _pending_exception->print();
    }
  }

  _pending_exception = NULL;
  _exception_file    = NULL;
  _exception_line    = NULL;
}


// GC Support
bool Thread::claim_oops_do_par_case(int strong_roots_parity) {
  jint thread_parity = _oops_do_parity;
  if (thread_parity != strong_roots_parity) {
    jint res = atomic::compare_and_exchange(strong_roots_parity,
					    &_oops_do_parity,
					    thread_parity);
    if (res == thread_parity) return true;
    else {
      guarantee(res == strong_roots_parity, "Or else what?");
      assert(SharedHeap::heap()->n_par_threads() > 0,
	     "Should only fail when parallel.");
      return false;
    }      
  }
  assert(SharedHeap::heap()->n_par_threads() > 0,
	 "Should only fail when parallel.");
  return false;
}

void Thread::oops_do(OopClosure* f) {
  active_handles()->oops_do(f);
  f->do_oop((oop*)&_pending_exception);
  handle_area()->oops_do(f);
}

void Thread::nmethods_do() {
}

void Thread::print() {
  // get_priority assumes osthread initialized
  if (osthread() != NULL) {
    tty->print("prio=%d tid=" INTPTR_FORMAT " ", get_priority(this), this);
    osthread()->print();
  }
  debug_only(if (WizardMode) print_owned_locks();)
}

#ifdef ASSERT
void Thread::print_owned_locks() {  
  Mutex *cur = _owned_locks;
  if (cur == NULL) {
    tty->print(" (no locks) ");
  } else {
    tty->print_cr(" Locks owned:");
    while(cur) {
      cur->print();
      cur = cur->next();
    }
  }
}

static int ref_use_count  = 0;

bool Thread::owns_locks_but_compiled_lock() const {
  for(Mutex *cur = _owned_locks; cur; cur = cur->next()) {
    if (cur != Compile_lock) return true;
  }
  return false;
}


#endif

#ifndef PRODUCT

// The flag: potential_vm_operation notifies if this particular safepoint state could potential
// invoke the vm-thread (i.e., and oop allocation). In that case, we also have to make sure that
// no threads which allow_vm_block's are held
void Thread::check_for_valid_safepoint_state(bool potential_vm_operation) {  
    // Check if current thread is allowed to block at a safepoint
    if (!(_allow_safepoint_count == 0))
      fatal("Possible safepoint reached by thread that does not allow it");
    if (is_Java_thread() && ((JavaThread*)this)->thread_state() != _thread_in_vm) {
      fatal("LEAF method calling lock?");
    }

#ifdef ASSERT    
    if (potential_vm_operation && is_Java_thread() 
        && !Universe::is_bootstrapping()) {
      // Make sure we do not hold any locks that the VM thread also uses.
      // This could potentially lead to deadlocks      
      for(Mutex *cur = _owned_locks; cur; cur = cur->next()) {
        // Threads_lock is special, since the safepoint synchronization will not start before this is
        // acquired. Hence, a JavaThread cannot be holding it at a safepoint. So is VMOperationRequest_lock,
        // since it is used to transfer control between JavaThreads and the VMThread
        // Do not *exclude* any locks unless you are absolutly sure it is correct. Ask someone else first!
        if ( (cur->allow_vm_block() && 
              cur != Threads_lock && 
              cur != Compile_lock &&               // Temporary: should not be necessary when we get spearate compilation
              cur != VMOperationRequest_lock &&               
              cur != VMOperationQueue_lock) ||
              cur->rank() == Mutex::special) { 
          warning("Thread holding lock at safepoint that vm can block on: %s", cur->name());          
        }
      }
    }

    if (GCALotAtAllSafepoints) {
      // We could enter a safepoint here and thus have a gc
      InterfaceSupport::check_gc_alot();
    }

#endif
}
#endif

bool Thread::is_in_stack(address adr) const {
  assert(Thread::current() == this, "is_in_stack can only be called from current thread");
  // High limit: highest_lock is set during thread execution
  // Low  limit: address of the local variable dummy, rounded to 4K boundary.
  // (The rounding helps finding threads in unsafe mode, even if the particular stack
  // frame has been popped already.  Correct as long as stacks are at least 4K long and aligned.)
  address end = os::current_stack_pointer();
  if (_highest_lock >= adr && adr >= end) return true;
  
  return false;
}

// We had to move these methods here, because vm threads get into ObjectSynchronizer::enter
// However, there is a note in JavaThread::is_lock_owned() about the VM threads not being
// used for compilation in the future. If that change is made, the need for these methods
// should be revisited, and they should be removed if possible.

bool Thread::is_lock_owned(address adr) const {
  if (is_in_stack(adr) ) return true;
  return false;
}

void Thread::set_as_starting_thread() {
 // NOTE: this must be called inside the main thread.
  os::create_main_thread(this);
}

static void initialize_class(symbolHandle class_name, TRAPS) {
  klassOop klass = SystemDictionary::resolve_or_fail(class_name, true, CHECK);
  instanceKlass::cast(klass)->initialize(CHECK);
}


// Creates the initial ThreadGroup
static Handle create_initial_thread_group(TRAPS) {
  klassOop k = SystemDictionary::resolve_or_fail(vmSymbolHandles::java_lang_ThreadGroup(), true, CHECK_NH);
  instanceKlassHandle klass (THREAD, k);
  
  Handle system_instance = klass->allocate_instance_handle(CHECK_NH);
  {
    JavaValue result(T_VOID);
    JavaCalls::call_special(&result, 
                            system_instance, 
                            klass, 
                            vmSymbolHandles::object_initializer_name(), 
                            vmSymbolHandles::void_method_signature(), 
                            CHECK_NH);
  }
  Universe::set_system_thread_group(system_instance());

  Handle main_instance = klass->allocate_instance_handle(CHECK_NH);
  {
    JavaValue result(T_VOID);
    Handle string = java_lang_String::create_from_str("main", CHECK_NH);
    JavaCalls::call_special(&result,
                            main_instance,
                            klass,
                            vmSymbolHandles::object_initializer_name(),
                            vmSymbolHandles::threadgroup_string_void_signature(),
                            system_instance,
                            string,
                            CHECK_NH);
  }
  return main_instance;
}

// Creates the initial Thread
static oop create_initial_thread(Handle thread_group, JavaThread* thread, TRAPS) {  
  klassOop k = SystemDictionary::resolve_or_fail(vmSymbolHandles::java_lang_Thread(), true, CHECK_0);
  instanceKlassHandle klass (THREAD, k);
  instanceHandle thread_oop = klass->allocate_instance_handle(CHECK_0);
  
  java_lang_Thread::set_thread(thread_oop(), thread);
  java_lang_Thread::set_priority(thread_oop(), NormPriority);
  thread->set_threadObj(thread_oop());
    
  Handle string = java_lang_String::create_from_str("main", CHECK_0);
  
  JavaValue result(T_VOID);
  JavaCalls::call_special(&result, thread_oop, 
                                   klass, 
                                   vmSymbolHandles::object_initializer_name(), 
                                   vmSymbolHandles::threadgroup_string_void_signature(), 
                                   thread_group, 
                                   string, 
                                   CHECK_0);  
  return thread_oop();
}

static void call_initializeSystemClass(TRAPS) {
  klassOop k =  SystemDictionary::resolve_or_fail(vmSymbolHandles::java_lang_System(), true, CHECK);
  instanceKlassHandle klass (THREAD, k);
  
  JavaValue result(T_VOID);
  JavaCalls::call_static(&result, klass, vmSymbolHandles::initializeSystemClass_name(), 
                                         vmSymbolHandles::void_method_signature(), CHECK);
}

static void reset_vm_info_property(TRAPS) {
  // the vm info string
  ResourceMark rm(THREAD);
  const char *vm_info = VM_Version::vm_info_string();
  
  // java.lang.System class
  klassOop k =  SystemDictionary::resolve_or_fail(vmSymbolHandles::java_lang_System(), true, CHECK);
  instanceKlassHandle klass (THREAD, k);
  
  // setProperty arguments
  Handle key_str    = java_lang_String::create_from_str("java.vm.info", CHECK);
  Handle value_str  = java_lang_String::create_from_str(vm_info, CHECK);
  
  // return value
  JavaValue r(T_OBJECT); 

  // public static String setProperty(String key, String value);
  JavaCalls::call_static(&r,
                         klass,
                         vmSymbolHandles::setProperty_name(), 
                         vmSymbolHandles::string_string_string_signature(), 
                         key_str, 
                         value_str, 
                         CHECK);  
}


void JavaThread::allocate_threadObj(Handle thread_group, char* thread_name, bool daemon, TRAPS) {
  assert(thread_group.not_null(), "thread group should be specified");
  assert(threadObj() == NULL, "should only create Java thread object once");

  klassOop k = SystemDictionary::resolve_or_fail(vmSymbolHandles::java_lang_Thread(), true, CHECK);
  instanceKlassHandle klass (THREAD, k);
  instanceHandle thread_oop = klass->allocate_instance_handle(CHECK);

  java_lang_Thread::set_thread(thread_oop(), this);
  java_lang_Thread::set_priority(thread_oop(), NormPriority);
  set_threadObj(thread_oop());

  JavaValue result(T_VOID);
  if (thread_name != NULL) {
    Handle name = java_lang_String::create_from_str(thread_name, CHECK);
    // Thread gets assigned specified name and null target
    JavaCalls::call_special(&result,
                            thread_oop,
                            klass, 
                            vmSymbolHandles::object_initializer_name(), 
                            vmSymbolHandles::threadgroup_string_void_signature(),
                            thread_group, // Argument 1                        
                            name,         // Argument 2
                            THREAD);
  } else {
    // Thread gets assigned name "Thread-nnn" and null target
    // (java.lang.Thread doesn't have a constructor taking only a ThreadGroup argument)
    JavaCalls::call_special(&result,
                            thread_oop,
                            klass, 
                            vmSymbolHandles::object_initializer_name(), 
                            vmSymbolHandles::threadgroup_runnable_void_signature(),
                            thread_group, // Argument 1                        
                            Handle(),     // Argument 2
                            THREAD);
  }

  if (daemon) {
      java_lang_Thread::set_daemon(thread_oop());
  }
}


// ======= WatcherThread ========

// The watcher thread exists to simulate timer interrupts.  It should
// be replaced by an abstraction over whatever native support for
// timer interrupts exists on the platform.

WatcherThread* WatcherThread::_watcher_thread   = NULL;
bool           WatcherThread::_should_terminate = false;

WatcherThread::WatcherThread() : Thread() {
  assert(watcher_thread() == NULL, "we can only allocate one WatcherThread");
  if (os::create_thread(this, os::watcher_thread)) {
    _watcher_thread = this;
    
    // Set the watcher thread to the highest OS priority which should not be 
    // used, unless a Java thread with priority java.lang.Thread.MAX_PRIORITY 
    // is created. The only normal thread using this priority is the reference 
    // handler thread, which runs for very short intervals only.
    // If the VMThread's priority is not lower than the WatcherThread profiling
    // will be inaccurate.
    os::set_priority(this, MaxPriority);
    if (!DisableStartThread) {
      os::start_thread(this);
    }
  }
}

void WatcherThread::run() {
  assert(this == watcher_thread(), "just checking");

  this->record_stack_base_and_size();
  this->initialize_thread_local_storage();
  this->set_active_handles(JNIHandleBlock::allocate_block());
#ifdef ASSERT
  // All PeriodicTasks should be created before the WatcherThread
  // is started.
  const int num_periodic_tasks = PeriodicTask::num_tasks();
#endif
  while(!_should_terminate) {    
    assert(watcher_thread() == Thread::current(),  "thread consistency check");
    assert(watcher_thread() == this,  "thread consistency check");
    // To relax this assert, we'd need to make sure PeriodicTasks can
    // join the system gracefully -- we might have a long sleep value
    // but are adding a PeriodicTask with a small interval, so it'll appear
    // to take a long time for the initial event to fire.
    // One solution would be to interrupt the current sleep and cause
    // a PeriodicTask::real_time_tick to fire, or we could do a timed wait 
    // on a condvar (or ObjectMonitor::wait).
    // Currently, we don't need or allow PeriodicTasks to dynamically enter
    // or leave the system.
    assert(PeriodicTask::num_tasks() == num_periodic_tasks, "late joiner?");
    // Calculate how long it'll be until the next PeriodicTask work
    // should be done, and sleep that amount of time.
    const size_t time_to_wait = PeriodicTask::time_to_wait();
    os::sleep(this, time_to_wait, false);
    PeriodicTask::real_time_tick(time_to_wait);
  }
  
  // Signal that it is terminated
  {
    MutexLockerEx mu(Terminator_lock, Mutex::_no_safepoint_check_flag);
    _watcher_thread = NULL;
    Terminator_lock->notify();
  }
  
  // Thread destructor usually does this..
  ThreadLocalStorage::set_thread(NULL);
}

void WatcherThread::start() {
  if (watcher_thread() == NULL) {
    _should_terminate = false;
    // Create the single instance of WatcherThread
    new WatcherThread();   
  }
}

void WatcherThread::stop() {
  // it is ok to take late safepoints here, if needed
  MutexLocker mu(Terminator_lock);
  _should_terminate = true;  
  while(watcher_thread() != NULL) {
    Terminator_lock->wait();
  }  
}

void WatcherThread::print() {
  tty->print("\"VM Periodic Task Thread\" ");
  Thread::print();
  tty->cr();
}



// ======= SuspendCheckerThread ========

// The SuspendCheckerThread is used by the VMThread to see if
// the current suspend operation will cause a VMThread deadlock.

SuspendCheckerThread* SuspendCheckerThread::_suspend_checker_thread = NULL;
bool           SuspendCheckerThread::_should_terminate              = false;

SuspendCheckerThread::SuspendCheckerThread() : Thread() {
  assert(suspend_checker_thread() == NULL, "we can only allocate one SuspendCheckerThread");
  if (os::create_thread(this, os::suspend_checker_thread)) {
    _suspend_checker_thread = this;
    // Set the suspend checker thread to the highest OS priority. Since
    // this thread is doing work for the VMThread, it should have a
    // higher priority.
    os::set_priority(this, MaxPriority);
    if (!DisableStartThread) {
      os::start_thread(this);
    }
  }
}

void SuspendCheckerThread::run() {
  this->record_stack_base_and_size();
  this->initialize_thread_local_storage();
  this->set_active_handles(JNIHandleBlock::allocate_block());
  {
    MutexLockerEx ml(SuspendChecker_lock, Mutex::_no_safepoint_check_flag);

    while(!_should_terminate) {    
      assert(suspend_checker_thread() == Thread::current(),  "thread consistency check");
      assert(suspend_checker_thread() == this,  "thread consistency check");

      SuspendChecker_lock->wait(true, 0);  // wait for VMThread to need us
      { // temporarily drop the lock in case we get stuck
	MutexUnlocker mul(SuspendChecker_lock);

        // The malloc lock is only one operating system resource that causes
        // deadlock when held by a suspended thread.  Should there be a more
        // general model of operating system resources to fence off?

        char *b = (char *)malloc(1);         // see if we can allocate a byte
        if (b != NULL) free(b);              // give it back
      }
      SuspendChecker_lock->notify();       // notify VMThread we are not stuck
    }
  }
  
  // Signal that it is terminated
  {
    MutexLockerEx mu(Terminator_lock, Mutex::_no_safepoint_check_flag);
    _suspend_checker_thread = NULL;
    Terminator_lock->notify();
  }
  
  // Thread destructor usually does this..
  ThreadLocalStorage::set_thread(NULL);
}

void SuspendCheckerThread::start() {
  if (suspend_checker_thread() == NULL) {
    _should_terminate = false;
    // Create the single instance of SuspendCheckerThread
    new SuspendCheckerThread();   
  }
}

void SuspendCheckerThread::stop() {
  // Since the VMThread depends on us for deadlock detection, we
  // shouldn't block for safepoints.
  MutexLockerEx mu(Terminator_lock, Mutex::_no_safepoint_check_flag);
  _should_terminate = true;
  {
    // wake up SuspendCheckerThread so it can see that it needs to exit
    MutexLockerEx ml(SuspendChecker_lock, Mutex::_no_safepoint_check_flag);
    SuspendChecker_lock->notify();
  }
  while (suspend_checker_thread() != NULL) {
    Terminator_lock->wait(true);  // don't block for safepoints
  }  
}

void SuspendCheckerThread::print() {
  tty->print("\"Suspend Checker Thread\" ");
  Thread::print();
  tty->cr();
}


// ======= JavaThread ========

// A JavaThread is a normal Java thread

const struct JNINativeInterface_* jni_functions();

void JavaThread::initialize() {
  // Initialize fields
  set_saved_exception_pc(NULL);
  set_threadObj(NULL);  
  _anchor.clear();
  set_entry_point(NULL);
  _jni_environment.functions = jni_functions();
  set_vm_result(NULL);
  set_vm_result_2(NULL);
  set_vframe_array_head(NULL);
  set_vframe_array_last(NULL);
  set_monitor_chunks(NULL);
  set_monitor_arrays(NULL);  
  set_next(NULL);  
  set_thread_state(_thread_new);  
  _terminated = _not_terminated;
  _privileged_stack_top = NULL;
  _array_for_gc = NULL;
  _suspend_critical = _not_suspend_critical;
  _suspend_equivalent = false;
  _in_jvm_wait = false;
  _doing_unsafe_access = false;
  _stack_guard_state = stack_guard_unused;
  _exception_oop = NULL;
  _exception_pc  = 0;
  _exception_handler_pc = 0;
  _exception_stack_size = 0;
  _last_exception_pc = NULL;
  _jvmdi_thread_state= NULL;
  _jvmdi_get_loaded_classes_closure = NULL;
  _special_runtime_exit_condition = _no_async_condition;
  _is_handling_implicit_exception = false;
  _is_throwing_null_ptr_exception = false;
  _pending_async_exception = NULL;
  _is_compiling = false;
#ifdef COMPILER2
  _blocked_on_compilation = false;
#endif
#ifndef CORE
  _log = NULL;  // %%% anticipate action in CompilerThread; see ciEnv()
#endif
  _jvmpi_data = NULL;
  _last_sum = 0;
  _deferred_obj_alloc_events = NULL;
  _jni_active_critical = 0;
  _do_not_unlock_if_synchronized = false;

  if (UseTLAB) {
    tlab().clear();  // initialize thread-local alloc buffer related fields
  }

  set_thread_profiler(NULL);
  if (FlatProfiler::is_active()) {
    // This is where we would decide to either give each thread it's own profiler
    // or use one global one from FlatProfiler,
    // or up to some count of the number of profiled threads, etc.
    ThreadProfiler* pp = new ThreadProfiler();
    pp->engage();
    set_thread_profiler(pp);
  }

  // Setup safepoint state info for this thread
  ThreadSafepointState::create(this);  

  debug_only(_java_call_counter = 0);

  // Give JVMDI a chance to setup any per thread data structures it
  // might need.
  if (jvmdi::enabled()) {
    jvmdi::initialize_thread(this);
  }
#ifdef HOTSWAP
  _popframe_condition = popframe_inactive;
#ifndef CORE
  _popframe_preserved_args = NULL;
  _popframe_preserved_args_size = 0;
#endif
#endif HOTSWAP

  pd_initialize();
}

JavaThread::JavaThread() : Thread() {
  initialize();      
}

bool JavaThread::reguard_stack(address cur_sp) {
  if (_stack_guard_state != stack_guard_yellow_disabled) {
    return true; // Stack already guarded or guard pages not needed.
  }
  if (cur_sp > stack_yellow_zone_base() + StackReguardSlack) {
    // For those architectures which have separate register and
    // memory stacks, we must check the register stack to see if 
    // it has overflowed.
    if ( register_stack_overflow() )
      return false;

    enable_stack_yellow_zone(); // Enough space reprotect; do so.
    return true;
  }
  return false;	  // Caller must continue unwinding frames.
}

bool JavaThread::reguard_stack(void) {
  return reguard_stack(os::current_stack_pointer());
}


void JavaThread::block_if_vm_exited() {
  if (_terminated == _vm_exited) {
    // _vm_exited is set at safepoint, and Threads_lock is never released
    // we will block here forever
    Threads_lock->lock_without_safepoint_check();
    ShouldNotReachHere();
  }
}


// Remove this ifdef when C1 is ported to the compiler interface.
#ifndef CORE
static void compiler_thread_entry(JavaThread* thread, TRAPS);
#endif

JavaThread::JavaThread(ThreadFunction entry_point, size_t stack_sz) : Thread() {
  if (TraceThreadEvents) {
    tty->print_cr("creating thread %p", this);
  }
  initialize();
  set_entry_point(entry_point);
  // Create the native thread itself.
  // %note runtime_23
  os::ThreadType thr_type = os::java_thread;
  NOT_CORE(thr_type = entry_point == &compiler_thread_entry ?
      os::compiler_thread : os::java_thread;)
  os::create_thread(this, thr_type, stack_sz);

  // The _osthread may be NULL here because we ran out of memory (too many threads active).
  // We need to throw and OutOfMemoryError - however we cannot do this here because the caller
  // may hold a lock and all locks must be unlocked before throwing the exception (throwing
  // the exception consists of creating the exception object & initializing it, initialization
  // will leave the VM via a JavaCall and then all locks must be unlocked).
  //
  // The thread is still suspended when we reach here. Thread must be explicit started
  // by creator! Furthermore, the thread must also explicitly be added to the Threads list
  // by calling Threads:add. The reason why this is not done here, is because the thread 
  // object must be fully initialized (take a look at JVM_Start)  
}

JavaThread::~JavaThread() {
  if (TraceThreadEvents) {
      tty->print_cr("terminate thread %p", this);
  }

#ifndef CORE
  // Free any remaining  previous UnrollBlock
  vframeArray* old_array = vframe_array_last();

  if (old_array != NULL) {
    Deoptimization::UnrollBlock* old_info = old_array->unroll_block();
    old_array->set_unroll_block(NULL);
    delete old_info;
    delete old_array;
  }
#endif /* CORE */
  
  // All Java related clean up happens in exit  
  ThreadSafepointState::destroy(this);
  if (_thread_profiler != NULL) delete _thread_profiler;

  if (jvmdi::enabled()) {
    jvmdi::cleanup_thread(this);
  }
}


// The first routine called by a new Java thread
void JavaThread::run() {
  // used to test validitity of stack trace backs
  this->record_base_of_stack_pointer();
  
  // Record real stack base and size.
  this->record_stack_base_and_size();

  // Initialize thread local storage; set before calling MutexLocker
  this->initialize_thread_local_storage();  

  this->create_stack_guard_pages();
  
  // Thread is now sufficient initialized to be handled by the safepoint code as being
  // in the VM. Change thread state from _thread_new to _thread_in_vm  
  ThreadStateTransition::transition(this, _thread_new, _thread_in_vm);  
  
  assert(JavaThread::current() == this, "sanity check");
  assert(!Thread::current()->owns_locks(), "sanity check");
  
  // This operation might block. We call that after all safepoint checks for a new thread has
  // been completed.
  this->set_active_handles(JNIHandleBlock::allocate_block());
  
  if (jvmdi::enabled()) {
    jvmdi::post_thread_start_event(this);
  }
  if (jvmpi::is_event_enabled(JVMPI_EVENT_THREAD_START)) {
    jvmpi::post_thread_start_event(this);
  }

  // We call another function to do the rest so we are sure that the stack addresses used
  // from there will be lower than the stack base just computed
  thread_main_inner();

  // Note, thread is no longer valid at this point!
}


void JavaThread::thread_main_inner() {  
  assert(JavaThread::current() == this, "sanity check");
  assert(this->threadObj() != NULL, "just checking");

  // Execute thread entry point. If this thread is being asked to restart, 
  // or has been stopped before starting, do not reexecute entry point.
  // Note: Due to JVM_StopThread we can have pending exceptions already!
  if (!this->has_pending_exception() && !java_lang_Thread::is_stillborn(this->threadObj())) {
    // enter the thread's entry point only if we have no pending exceptions
    HandleMark hm(this);    
    this->entry_point()(this, this);
  }
  
  this->exit(false);
  this->delete_thread_and_TLS_current();

}

  
static void ensure_join(JavaThread* thread) {
  // We do not need to grap the Threads_lock, since we are operating on ourself.
  Handle threadObj(thread, thread->threadObj());
  assert(threadObj.not_null(), "java thread object must exist");
  ObjectLocker lock(threadObj, thread);
  // Ignore pending exception (ThreadDeath), since we are exiting anyway
  thread->clear_pending_exception();
  // It is of profound importance that we set the stillborn bit and reset the thread object, 
  // before we do the notify. Since, changing these two variable will make JVM_IsAlive return
  // false. So in case another thread is doing a join on this thread , it will detect that the thread 
  // is dead when it gets notified.
  java_lang_Thread::set_stillborn(threadObj());
  java_lang_Thread::set_thread(threadObj(), NULL);
  lock.notify_all(thread);
  // Ignore pending exception (ThreadDeath), since we are exiting anyway
  thread->clear_pending_exception();
}


void JavaThread::exit(bool destroy_vm) {
  assert(this == JavaThread::current(),  "thread consistency check");
  if (!InitializeJavaLangSystem) return;

  HandleMark hm(this);
  Handle uncaught_exception(this, this->pending_exception());
  this->clear_pending_exception();
  Handle threadObj(this, this->threadObj());
  assert(threadObj.not_null(), "Java thread object should be created");
  
  if (get_thread_profiler() != NULL) {
    get_thread_profiler()->disengage();
    ResourceMark rm;
    get_thread_profiler()->print(get_thread_name());
  }


  // FIXIT: This code should be moved into else part, when reliable 1.2/1.3 check is in place
  instanceKlassHandle shutdown_klass;
  {
    EXCEPTION_MARK;
    
    CLEAR_PENDING_EXCEPTION;
  }
  // FIXIT: The is_null check is only so it works better on JDK1.2 VM's. This
  // has to be fixed by a runtime query method
  if (!destroy_vm || Universe::is_jdk12x_version()) {
    // Handle uncaught_exception if any, java.lang.ThreadGroup.uncaughtException must be called 
    if (uncaught_exception.not_null()) {
      Handle group(this, java_lang_Thread::threadGroup(threadObj()));
      Events::log("uncaught exception %#x %p %#x", uncaught_exception(), threadObj(), group());
      { 
        EXCEPTION_MARK;
        KlassHandle thread_group(THREAD, SystemDictionary::threadGroup_klass());
        JavaValue result(T_VOID);
        JavaCalls::call_virtual(&result, 
                                group, thread_group,
                                vmSymbolHandles::uncaughtException_name(), 
                                vmSymbolHandles::thread_throwable_void_signature(), 
                                threadObj,          // Arg 1
                                uncaught_exception, // Arg 2
                                THREAD);
        CLEAR_PENDING_EXCEPTION;
      }
    }
 
    // Call Thread.exit(). We try 3 times in case we got another Thread.stop during
    // the execution of the method. If that is not enough, then we don't really care. Thread.stop
    // is deprecated anyhow.
    { int count = 3;
      while (java_lang_Thread::threadGroup(threadObj()) != NULL && (count-- > 0)) {
        EXCEPTION_MARK;
        JavaValue result(T_VOID);
        KlassHandle thread_klass(THREAD, SystemDictionary::thread_klass());
        JavaCalls::call_virtual(&result, 
                              threadObj, thread_klass,
                              vmSymbolHandles::exit_method_name(), 
                              vmSymbolHandles::void_method_signature(), 
                              THREAD);  
        CLEAR_PENDING_EXCEPTION;
      }
    }      

    // notify jvmdi
    if (jvmdi::enabled()) {
      jvmdi::post_thread_end_event(this);
    }

    // notify jvmpi
    if (jvmpi::is_event_enabled(JVMPI_EVENT_THREAD_END)) {
      jvmpi::post_thread_end_event(this);
    }

    // We have notified the agents that we are exiting, before we go on,
    // we must check for a pending external suspend request and honor it
    // in order to not surprise the thread that made the suspend request.
    while (true) {
      {
        MutexLockerEx ml(SR_lock(), Mutex::_no_safepoint_check_flag);
        if (!is_external_suspend()) {
          set_terminated(_thread_exiting);
          break;
        }
        // Implied else:
        // Things get a little tricky here. We have a pending external
        // suspend request, but we are holding the SR_lock so we
        // can't just self-suspend. So we temporarily drop the lock
        // and then self-suspend.
      }

      StateSaver sv(this);  // prepare for possible stack walks, etc
      ThreadBlockInVM tbivm(this);
      java_suspend_self();

      // We're done with this suspend request, but we have to loop around
      // and check again. Eventually we will get SR_lock without a pending
      // external suspend request and will be able to mark ourselves as
      // exiting.
    }
    // no more external suspends are allowed at this point
  } else {
    // Last thread running. It calls java.lang.Shutdown.shutdown()
    EXCEPTION_MARK;
    klassOop k = SystemDictionary::resolve_or_null(vmSymbolHandles::java_lang_Shutdown(), CHECK);    
    instanceKlassHandle shutdown_klass (THREAD, k);
    assert(shutdown_klass.not_null(), "sanity check");
    JavaValue result(T_VOID);
    JavaCalls::call_static(&result,                      
                           shutdown_klass,
                           vmSymbolHandles::shutdown_method_name(),
                           vmSymbolHandles::void_method_signature(),
                           THREAD);  
    CLEAR_PENDING_EXCEPTION;

    // before_exit() has already posted JVMDI/PI THREAD_END events
  }
  
  // Notify waiters on thread object. This has to be done after exit() is called
  // on the thread (if the thread is the last thread in a daemon ThreadGroup the
  // group should have the destroyed bit set before waiters are notified).
  ensure_join(this);

  // These things needs to be done while we are still a Java Thread. Make sure that thread
  // is in a consistent state, in case GC happens
  assert(_privileged_stack_top == NULL, "must be NULL when we get here");
    
  if (active_handles() != NULL) {
    JNIHandleBlock* block = active_handles();
    set_active_handles(NULL);
    JNIHandleBlock::release_block(block);
  }

  if (free_handle_block() != NULL) {
    JNIHandleBlock* block = free_handle_block();
    set_free_handle_block(NULL);
    JNIHandleBlock::release_block(block);
  }

  // These have to be removed while this is still a valid thread.
  remove_stack_guard_pages();

  if (UseTLAB) {
    tlab().reset();  
  }

  // Remove from list of active threads list, and notify VM thread if we are the last non-daemon thread 
  Threads::remove(this);  
}


JavaThread* JavaThread::active() {
  Thread* thread = ThreadLocalStorage::thread();
  assert(thread != NULL, "just checking");
  if (thread->is_Java_thread()) {
    return (JavaThread*) thread;
  } else {
    assert(thread->is_VM_thread(), "this must be a vm thread");
    VM_Operation* op = ((VMThread*) thread)->vm_operation();
    JavaThread *ret=op == NULL ? NULL : (JavaThread *)op->calling_thread();
    assert(ret->is_Java_thread(), "must be a Java thread");
    return ret;
  }
}

bool JavaThread::is_lock_owned(address adr) const {  
  if (is_in_stack(adr)) return true;
    
  for (MonitorChunk* chunk = monitor_chunks(); chunk != NULL; chunk = chunk->next()) {
    if (chunk->contains(adr)) return true;
  }

  return false;
}


void JavaThread::add_monitor_chunk(MonitorChunk* chunk) {
  chunk->set_next(monitor_chunks());
  set_monitor_chunks(chunk);
}

void JavaThread::remove_monitor_chunk(MonitorChunk* chunk) {
  guarantee(monitor_chunks() != NULL, "must be non empty");
  if (monitor_chunks() == chunk) {
    set_monitor_chunks(chunk->next());
  } else {
    MonitorChunk* prev = monitor_chunks();
    while (prev->next() != chunk) prev = prev->next();
    prev->set_next(chunk->next()); 
  }
}


void JavaThread::add_monitor_array(MonitorArray* array) {
#ifndef CORE
  array->set_next(monitor_arrays());
  set_monitor_arrays(array);
#endif
}

void JavaThread::remove_monitor_array(MonitorArray* array) {
#ifndef CORE
  guarantee(monitor_arrays() != NULL, "link must be present");
  if (monitor_arrays() == array) {
    set_monitor_arrays(array->next());
  } else {
    MonitorArray* prev = monitor_arrays();
    while (prev->next() != array) prev = prev->next();
    prev->set_next(array->next()); 
  }
#endif
}

// JVM support.

void JavaThread::handle_special_runtime_exit_condition(bool check_asyncs) {
  // Check for pending external suspend. Internal suspend requests do
  // not use handle_special_runtime_exit_condition(). Also, don't
  // self-suspend if the target thread is not current thread. jdbx threads
  // call into the VM with another thread's JNIEnv so we can be here
  // operating on behalf of a suspended thread (4432884).
  bool do_self_suspend = is_external_suspend_with_lock();
  if (do_self_suspend && this == JavaThread::current()) {
    StateSaver sv(this);  // prepare for possible stack walks, etc
    java_suspend_self();

    // We might be here in addition to the self-suspend request
    // so check for other async requests.
  }   

  if (!check_asyncs)
    return;

  JavaThread::AsyncRequests condition = clear_special_runtime_exit_condition();
  if (condition == _no_async_condition) {
    // Conditions have changed since has_special_runtime_exit_condition()
    // was called:
    // - if we were here only because of an external suspend request,
    //   then that was taken care of above (or cancelled) so we are done
    // - if we were here because of another async request, then it has
    //   been cleared between the has_special_runtime_exit_condition()
    //   and now so again we are done
    return;
  }

  // Check for pending async. exception
  if (_pending_async_exception != NULL) {
    // Only overwrite an already pending exception, if it is not a threadDeath.
    if (!has_pending_exception() || !pending_exception()->is_a(SystemDictionary::threaddeath_klass())) {
      // We cannot call Exceptions::_throw(...) here because we cannot block      
      set_pending_exception(_pending_async_exception, __FILE__, __LINE__);
      
      if (TraceExceptions) {
        ResourceMark rm;
        tty->print_cr("Async. exception installed at runtime exit of type: %s", instanceKlass::cast(_pending_async_exception->klass())->external_name());
      }
      _pending_async_exception = NULL;
    }        
  }

  if (condition == _async_unsafe_access_error && !has_pending_exception()) {
    condition = _no_async_condition;  // done
    switch (thread_state()) {
    case _thread_in_native:
      {
	ThreadInVMfromNative tiv(this);
	JavaThread* THREAD = this;
	THROW_MSG(vmSymbols::java_lang_InternalError(), "a fault occurred in an unsafe memory access operation");
      }
    case _thread_in_Java:
      {
	ThreadInVMfromJava tiv(this);
	JavaThread* THREAD = this;
	THROW_MSG(vmSymbols::java_lang_InternalError(), "a fault occurred in a recent unsafe memory access operation in compiled Java code");
      }
    default:
      ShouldNotReachHere();
    }
  }

  assert(condition == _no_async_condition || has_pending_exception(),
	 "must have handled the async condition, if no exception");

}

void JavaThread::send_thread_stop(oop java_throwable)  {
  assert(Thread::current()->is_VM_thread(), "should be in the vm thread");
  assert(Threads_lock->is_locked(), "Threads_lock should be locked by safepoint code");
  assert(SafepointSynchronize::is_at_safepoint(), "all threads are stopped");  

#ifndef CORE
  // Do not throw asynchronous exceptions against the compiler thread
  // (the compiler thread should not be a Java thread -- fix in 1.4.2)
  if (is_Compiler_thread()) return;
#endif

  // This is a change from JDK 1.1, but JDK 1.2 will also do it:
  if (java_throwable->is_a(SystemDictionary::threaddeath_klass())) {
    java_lang_Thread::set_stillborn(threadObj());
  }

  if (thread_state() == _thread_in_native) {
    // If the thread is in native code, it just gets the exception
    if (!has_pending_exception() || !pending_exception()->is_a(SystemDictionary::threaddeath_klass())) {
      set_pending_exception(java_throwable, __FILE__, __LINE__);

      if (TraceExceptions) {
       ResourceMark rm;
       tty->print_cr("Pending exception installed of type: %s", instanceKlass::cast(pending_exception()->klass())->external_name());
      }
    }
  } else {
    // Actually throw the Throwable against the target Thread - however
    // only if there is no thread death exception installed already.
    if (_pending_async_exception == NULL || !_pending_async_exception->is_a(SystemDictionary::threaddeath_klass())) {
#ifdef COMPILER2
      // If the topmost frame is a runtime stub, then we are calling into
      // OptoRuntime from compiled code. Some runtime stubs (new, monitor_exit..)
      // must deoptimize the caller before continuing, as the compiled  exception handler table
      // may not be valid
      if (has_last_Java_frame()) {
        frame f = last_frame();
        if (f.is_c2runtime_frame() || f.is_safepoint_blob_frame()) {          
          RegisterMap reg_map(this, false);
          frame compiled_frame = f.sender(&reg_map);
          if (compiled_frame.is_compiled_frame()) {
            Deoptimization::deoptimize_frame(this, compiled_frame.id());
          }          
        }
      }
#endif
    
      // Set async. pending exception in thread.
      set_pending_async_exception(java_throwable);    

      if (TraceExceptions) {
       ResourceMark rm;
       tty->print_cr("Pending Async. exception installed of type: %s", instanceKlass::cast(_pending_async_exception->klass())->external_name());
      }
    }
  }

  
  // Interrupt thread so it will wake up from a potential wait()
  Thread::interrupt(this);  
}

// External suspension mechanism.
//
// Tell the VM to suspend a thread when ever it knows that it does not hold on 
// to any VM_locks and it is at a transition
// Self-suspension will happen on the transition out of the vm.
// Catch "this" coming in from JNIEnv pointers when the thread has been freed
//
// Guarantees on return:
//   + Target thread will not execute any new bytecode (that's why we need to 
//     force a safepoint)
//   + Target thread will not enter any new monitors
//
void JavaThread::java_suspend(bool native_now) {
  { MutexLocker mu(Threads_lock);
    if (!Threads::includes(this) || is_exiting() || this->threadObj() == NULL) {
       return;
    }
  }

  // a racing resume has cancelled us; bail out now
  if (!this->is_external_suspend_with_lock()) {
    return;
  }

  // 4587669 Don't allocate vm_suspend on heap - if the thread is suspended
  // while it is holding malloc lock, the attempt to delete vm_suspend
  // will deadlock VM.
  VM_ThreadSuspend vm_suspend(this->threadObj(), native_now);
  VMThread::execute(&vm_suspend);
}

void JavaThread::send_java_suspend(bool native_now) {
  if (native_now && thread_state() == _thread_in_native &&
  suspend_critical() != _native_suspend_critical) {
    // Make an immediate external suspend request:
    // - caller has said that it is okay to immediately suspend threads
    //   in state _thread_in_native
    // - the thread is in state _thread_in_native
    // - the thread has not marked itself as being in a native
    //   SuspendCritical region.
    // We don't retry if the thread is in a native SuspendCritical
    // region because it will eventually self-suspend.
    MutexLockerEx ml(SR_lock(), Mutex::_no_safepoint_check_flag);
    this->do_vm_suspend();
    this->set_suspended_in_native();
  }
}

// Part II of external suspension.
// A JavaThread self suspends when it detects a pending external suspend
// request. This is usually on transitions. It is also done in places
// where continuing to the next transition would surprise the caller,
// e.g., monitor entry.
//
// Returns the number of times that the thread self-suspended.
//
// Note: DO NOT call java_suspend_self() when you just want to block current
//       thread. java_suspend_self() is the second stage of cooperative
//       suspension for external suspend requests and should only be used
//       to complete an external suspend request.
//
int JavaThread::java_suspend_self() {
  int ret = 0;

  // we are in the process of exiting so don't suspend
  if (is_exiting()) {
     clear_external_suspend();
     return ret;
  }

  assert(_anchor.walkable() ||
    (is_Java_thread() && !((JavaThread*)this)->has_last_Java_frame()),
    "must have created a StateSaver");

  MutexLockerEx ml(SR_lock(), Mutex::_no_safepoint_check_flag);

  assert(!this->is_any_suspended(),
    "a thread trying to self-suspend should not already be suspended");
  assert(!this->is_baby_thread(), "baby thread can not self-suspend");

  if (this->is_suspend_equivalent()) {
    // If we are self-suspending as a result of the lifting of a
    // suspend equivalent condition, then the suspend_equivalent
    // flag is not cleared until we set the self_suspended flag so
    // that wait_for_ext_suspend_completion() returns consistent
    // results.
    this->clear_suspend_equivalent();
  }

  // A racing resume may have cancelled us before we grabbed SR_lock
  // above. Or another external suspend request could be waiting for us
  // by the time we return from pd_self_suspend_thread(). The thread
  // that requested the suspension may already be trying to walk our
  // stack and if we return now, we will destroy destroy the StateSaver
  // setup by the caller. This may cause the stack walker to crash. We
  // stay self-suspended until there are no more pending external
  // suspend requests.
  while (is_external_suspend()) {
    ret++;
    this->set_self_suspended();

    // _self_suspended flag is cleared by java_resume()
    while (is_self_suspended()) {
      os::pd_self_suspend_thread(this);
    }
  }

  return ret;
}

// Slow path when the native==>VM/Java barriers detect a safepoint is in
// progress or when there is a pending suspend request. Current thread needs
// to self-suspend if there is a suspend request and/or block if a safepoint
// is in progress.
// Note only the native==>VM/Java barriers can call this function and when
// thread state is _thread_in_native_trans
void JavaThread::check_safepoint_and_suspend_for_native_trans(JavaThread *thread) {

  assert(thread->thread_state() == _thread_in_native_trans, "wrong state");

  JavaThread *curJT = JavaThread::current();
  bool do_self_suspend = thread->is_external_suspend();

  // Don't self-suspend if the caller is not the target thread. jdbx
  // threads can call into the VM with another thread's JNIEnv so we
  // can be here operating on behalf of a suspended thread (4432884).
  if (do_self_suspend && curJT == thread) {
    JavaThreadState state = thread->thread_state();

    // If the safepoint code sees the _thread_in_native_trans state, it will
    // wait until the thread changes to other thread state. There is no
    // guarantee on how soon we can obtain the SR_lock and complete the
    // self-suspend request. It would be a bad idea to let safepoint wait for
    // too long. Temporarily change the state to _thread_in_native_blocked to 
    // let the VM thread know that this thread is ready for GC. The problem
    // of changing thread state is that safepoint could happen just after
    // java_suspend_self() returns after being resumed, and VM thread will 
    // see the _thread_in_native_blocked state. We must check for safepoint 
    // after restoring the state and make sure we won't leave while a safepoint
    // is in progress.
    StateSaver sv(thread);  // prepare for possible stack walks, etc
    thread->set_thread_state(_thread_in_native_blocked);
    thread->java_suspend_self();
    thread->set_thread_state(state);
  }

  if (SafepointSynchronize::do_call_back()) {
    // If we are safepointing, then block the caller which may not be
    // the same as the target thread (see above).
    SafepointSynchronize::block(curJT);
  }

#ifndef CORE
  // Did we get here because we might have a stale java/native transition frame?
  // Since we are in thread_in_native_trans a new safepoint can not start
  // that we might be racing against. So we can't miss a new deopt_suspend
  // and if it is present now it came from a safepoint while we were in native.
  // So there should be no race here checking this deopt suspend flag.
  if (curJT == thread && thread->has_deopt_suspension()) {
    // Make sure top most java/native transition frame has correct state
    SafepointSynchronize::do_deopt_suspend(thread);
  }
#endif
}

// We need to guarantee the Threads_lock here, since resumes are not 
// allowed during safepoint synchronization
// Can only resume from an external suspension
void JavaThread::java_resume() {
  assert_locked_or_safepoint(Threads_lock);  
  
  // Sanity check: thread is gone, has started exiting or the thread
  // was not externally suspended.
  if (!Threads::includes(this) || is_exiting() || !is_external_suspend()) {
    return;
  }

  MutexLockerEx ml(SR_lock(), Mutex::_no_safepoint_check_flag);

  clear_external_suspend();

  if (is_suspended_in_native()) {
    // thread is suspended in native code by a vm_suspend() call, resume it
    // by vm_resume()
    clear_suspended_in_native();
    do_vm_resume(true);
  }

  if (is_self_suspended()) {
    clear_self_suspended();
    SR_lock()->notify_all();
  }
}

void JavaThread::create_stack_guard_pages() {
  if (! os::uses_stack_guard_pages() || _stack_guard_state != stack_guard_unused) return;
  address low_addr = stack_base() - stack_size();
  size_t len = (StackYellowPages + StackRedPages) * os::vm_page_size();

  int allocate = os::allocate_stack_guard_pages();
  // warning("Guarding at " PTR_FORMAT " for len " SIZE_FORMAT "\n", low_addr, len);

  if (allocate && !os::commit_memory((char *) low_addr, len)) {
    warning("Attempt to allocate stack guard pages failed.");
    return;
  }

  if (os::guard_memory((char *) low_addr, len)) {
    _stack_guard_state = stack_guard_enabled;
  } else {
    warning("Attempt to protect stack guard pages failed.");
    if (os::uncommit_memory((char *) low_addr, len)) {
      warning("Attempt to deallocate stack guard pages failed.");
    }
  }
}

void JavaThread::remove_stack_guard_pages() {
  if (_stack_guard_state == stack_guard_unused) return;
  address low_addr = stack_base() - stack_size();
  size_t len = (StackYellowPages + StackRedPages) * os::vm_page_size();

  if (os::allocate_stack_guard_pages()) {
    if (os::uncommit_memory((char *) low_addr, len)) {
      _stack_guard_state = stack_guard_unused;
    } else {
      warning("Attempt to deallocate stack guard pages failed.");
    }
  } else {
    if (_stack_guard_state == stack_guard_unused) return;
    if (os::unguard_memory((char *) low_addr, len)) {
      _stack_guard_state = stack_guard_unused;
    } else {
	warning("Attempt to unprotect stack guard pages failed.");
    }
  }
}

void JavaThread::enable_stack_yellow_zone() {
  assert(_stack_guard_state != stack_guard_unused, "must be using guard pages.");
  assert(_stack_guard_state != stack_guard_enabled, "already enabled");

  // The base notation is from the stacks point of view, growing downward.
  // We need to adjust it to work correctly with guard_memory()
  address base = stack_yellow_zone_base() - stack_yellow_zone_size();
  
  guarantee(base < stack_base(),"Error calculating stack yellow zone");
  guarantee(base < os::current_stack_pointer(),"Error calculating stack yellow zone");

  if (os::guard_memory((char *) base, stack_yellow_zone_size())) {
    _stack_guard_state = stack_guard_enabled;
  } else {
    warning("Attempt to guard stack yellow zone failed.");
  }
  enable_register_stack_guard();
}

void JavaThread::disable_stack_yellow_zone() {
  assert(_stack_guard_state != stack_guard_unused, "must be using guard pages.");
  assert(_stack_guard_state != stack_guard_yellow_disabled, "already disabled");

  // Simply return if called for a thread that does not use guard pages.
  if (_stack_guard_state == stack_guard_unused) return;

  // The base notation is from the stacks point of view, growing downward.
  // We need to adjust it to work correctly with guard_memory()
  address base = stack_yellow_zone_base() - stack_yellow_zone_size();
  
  if (os::unguard_memory((char *)base, stack_yellow_zone_size())) {
    _stack_guard_state = stack_guard_yellow_disabled;
  } else {
    warning("Attempt to unguard stack yellow zone failed.");
  }
  disable_register_stack_guard();
}

void JavaThread::enable_stack_red_zone() {
  // The base notation is from the stacks point of view, growing downward.
  // We need to adjust it to work correctly with guard_memory()
  assert(_stack_guard_state != stack_guard_unused, "must be using guard pages.");
  address base = stack_red_zone_base() - stack_red_zone_size();
  
  guarantee(base < stack_base(),"Error calculating stack red zone");
  guarantee(base < os::current_stack_pointer(),"Error calculating stack red zone");

  if(!os::guard_memory((char *) base, stack_red_zone_size())) {
    warning("Attempt to guard stack red zone failed.");
  }
}

void JavaThread::disable_stack_red_zone() {
  // The base notation is from the stacks point of view, growing downward.
  // We need to adjust it to work correctly with guard_memory()
  assert(_stack_guard_state != stack_guard_unused, "must be using guard pages.");
  address base = stack_red_zone_base() - stack_red_zone_size();
  if (!os::unguard_memory((char *)base, stack_red_zone_size())) {
    warning("Attempt to unguard stack red zone failed.");
  }
}

void JavaThread::insert_vframe_array(vframeArray* value) {
#ifdef CORE
  ShouldNotReachHere();
#else
  vframeArray* temp = vframe_array_head();
  if (   (vframe_array_head() == NULL)
      || vframe_array_head()->original().is_older(value->original().id())) {
    set_vframe_array_head(value);
    value->set_next(temp);
  } else {
    // Find the right place to insert the vframe array
    while (   (temp->next() != NULL)
           && temp->next()->original().is_younger(value->original().id())) {
      temp = temp->next();
    }
    assert( temp->original().id() != value->original().id(), "Double deopt!");
    value->set_next(temp->next());    
    temp->set_next(value);
  }
#endif
}

vframeArray* JavaThread::vframe_array_for(const frame* f) {
#ifdef CORE
  ShouldNotReachHere();
#else
  for (vframeArray* vfa = vframe_array_head(); vfa != NULL; vfa = vfa->next()) {
#ifdef IA64
      // IA64 can have zero sized frame. id is unique but runs opposite
      // direction from sp
    if (vfa->original().id() == f->id()) return vfa;
#else
    // Account for changes to SP by an interpreted callee which may extend its
    // caller's frame to allocate space for its non-argument local variables.
    // It suffices to simply compare the original, unextended SPs of the key
    // frame and the frame from which the vframeArray was constructed.
    if (vfa->unextended_sp() == f->unextended_sp()) return vfa;
#endif /* IA64 */
  }
#endif
  return NULL;
}

void JavaThread::frames_do(void f(frame*, const RegisterMap* map)) {
  // ignore is there is no stack
  if (!has_last_Java_frame()) return;
  // traverse the stack frames. Starts from top frame.  
  for(StackFrameStream fst(this); !fst.is_done(); fst.next()) {
    frame* fr = fst.current();    
    f(fr, fst.register_map());    
  }  
}


// Deoptimization
// Function for testing deoptimization
void JavaThread::deoptimize() {
#ifndef CORE
  COMPILER1_ONLY(assert(DeoptC1, "no debug information is emitted");)
  StackFrameStream fst(this);
  frame callee_fr = *fst.current();;
  bool deopt = false;           // Dump stack only if a deopt actually happens.
  // Iterate over all frames in the thread and deoptimize
  for(; !fst.is_done(); fst.next()) {
    if(fst.current()->is_deoptimized_frame() ) break;
    if(fst.current()->can_be_deoptimized()) {
      if (DebugDeoptimization && !deopt) {
        deopt = true; // One-time only print before deopt
        tty->print_cr("[BEFORE Deoptimization]");
        trace_frames();
        trace_stack();
      }
      // Make a private register-map for deoptimize to trash
      RegisterMap reg_map(fst.register_map());
      Deoptimization::deoptimize(this, *fst.current(), callee_fr, &reg_map);
    }
    callee_fr = *fst.current();
  }

  if (DebugDeoptimization && deopt) {
    tty->print_cr("[AFTER Deoptimization]");
    trace_frames();
  }
#endif
}


#ifndef PRODUCT
// Make zombies
void JavaThread::make_zombies() {
#ifndef CORE
  for(StackFrameStream fst(this); !fst.is_done(); fst.next()) {
    if (fst.current()->can_be_deoptimized()) {
      // it is a Java nmethod
      nmethod* nm = CodeCache::find_nmethod(fst.current()->pc());
      nm->make_not_entrant();
    }
  }
#endif
}
#endif


bool JavaThread::deoptimized_one_wrt_marked_nmethods() {
#ifndef CORE
  StackFrameStream fst(this);
  frame callee_fr = *fst.current();
  for(; !fst.is_done(); fst.next()) {
    if (fst.current()->should_be_deoptimized()) {
      Deoptimization::deoptimize(this, *fst.current(), callee_fr, fst.register_map());
      return true;
    }
    callee_fr = *fst.current();
  }
#endif
  return false;
}


void JavaThread::deoptimized_wrt_marked_nmethods() {
#ifndef CORE
  if (!has_last_Java_frame()) return;
   
  // We only deoptimize one frame in each traversal to avoid 
  // manipulating a frame and the continue traversal.
  int cont = true;
  while (cont) {
    cont = deoptimized_one_wrt_marked_nmethods();
  }
#endif
}


// GC support
static void frame_gc_epilogue(frame* f, const RegisterMap* map) { f->gc_epilogue(); }

void JavaThread::gc_epilogue() {
  frames_do(frame_gc_epilogue);
}


static void frame_gc_prologue(frame* f, const RegisterMap* map) { f->gc_prologue(); }

void JavaThread::gc_prologue() {
  frames_do(frame_gc_prologue);
}


void JavaThread::oops_do(OopClosure* f) {  
  // The ThreadProfiler oops_do is done from FlatProfiler::oops_do
  // since there may be more than one thread using each ThreadProfiler.

  // Traverse the GCHandles
  Thread::oops_do(f);

  assert( (!has_last_Java_frame() && java_call_counter() == 0) ||
          (has_last_Java_frame() && java_call_counter() > 0), "wrong java_sp info!");

  if (has_last_Java_frame()) {

    // Traverse the privileged stack
    if (_privileged_stack_top != NULL) {
      _privileged_stack_top->oops_do(f);
    }

    // traverse the registered growable array
    if (_array_for_gc != NULL) {
      for (int index = 0; index < _array_for_gc->length(); index++) {
        f->do_oop(_array_for_gc->adr_at(index));
      }
    }

    // Traverse the monitor chunks
    for (MonitorChunk* chunk = monitor_chunks(); chunk != NULL; chunk = chunk->next()) {
      chunk->oops_do(f);
    }

#ifndef CORE
    // Traverse the monitor arrays
    for (MonitorArray* array = monitor_arrays(); array != NULL; array = array->next()) {
      array->oops_do(f);
    }
#endif

    // Traverse the execution stack    
    for(StackFrameStream fst(this); !fst.is_done(); fst.next()) {
      fst.current()->oops_do(f, fst.register_map());
    }
  } 

#ifndef CORE
  // Traverse the vframeArray
  vframeArray* current = vframe_array_head();
  while (current) {
    current->oops_do(f);
    current = current->next();
  }
#endif

  // Traverse instance variables at the end since the GC may be moving things
  // around using this function
  f->do_oop((oop*) &_threadObj);
  f->do_oop((oop*) &_vm_result);
  f->do_oop((oop*) &_vm_result_2);
  f->do_oop((oop*) &_exception_oop);
  f->do_oop((oop*) &_pending_async_exception);
}

void JavaThread::nmethods_do() {  
  // Traverse the GCHandles
  Thread::nmethods_do();

  assert( (!has_last_Java_frame() && java_call_counter() == 0) ||
          (has_last_Java_frame() && java_call_counter() > 0), "wrong java_sp info!");

  if (has_last_Java_frame()) {
    // Traverse the execution stack    
    for(StackFrameStream fst(this); !fst.is_done(); fst.next()) {
      fst.current()->nmethods_do();
    }
  } 
}

// Printing
#ifndef PRODUCT
void JavaThread::print_thread_state() {
  const char *s;
  switch (_thread_state) {
  case _thread_uninitialized:     s ="_thread_uninitialized";     break;
  case _thread_new:               s ="_thread_new";               break;
  case _thread_new_trans:         s ="_thread_new_trans";         break;
  case _thread_in_native:         s ="_thread_in_native";         break;
  case _thread_in_native_trans:   s ="_thread_in_native_trans";   break;
  case _thread_in_vm:             s ="_thread_in_vm";             break;
  case _thread_in_vm_trans:       s ="_thread_in_vm_trans";       break;
  case _thread_in_Java:           s ="_thread_in_Java";           break;
  case _thread_in_Java_trans:     s ="_thread_in_Java_trans";     break;
  case _thread_blocked:           s ="_thread_blocked";           break;
  case _thread_blocked_trans:     s ="_thread_blocked_trans";     break;
  case _thread_in_native_blocked: s ="_thread_in_native_blocked"; break;
  default:
    ShouldNotReachHere();
  }
  tty->print_cr("Thread state: %s", s);
};
#endif // PRODUCT


void JavaThread::print() {
  tty->print("\"%s\" ", get_thread_name());
  if (threadObj() != NULL && java_lang_Thread::is_daemon(threadObj()))  tty->print("daemon ");
  Thread::print();
  // print guess for valid stack memory region (assume 4K pages); helps lock debugging
  tty->print_cr("[%lx..%lx]", (intptr_t)last_Java_sp() & ~right_n_bits(12), highest_lock());
#ifndef PRODUCT
  print_thread_state();
  _safepoint_state->print();
#endif // PRODUCT
}

// Verification

#ifndef PRODUCT
static void frame_verify(frame* f, const RegisterMap *map) { f->verify(map); }

void JavaThread::verify() {
  // Verify oops in the thread.
  oops_do(&VerifyOopClosure::verify_oop);

  // Verify the stack frames.
  frames_do(frame_verify);
}

#ifndef CORE
void JavaThread::verify_vframes() {
 // Verify the vframeArray list in the thread matches the frames.
  vframeArray* current = vframe_array_head();

  if (has_last_Java_frame()) {
    for(StackFrameStream fst(this); !fst.is_done(); fst.next()) {
      frame* fr = fst.current();
      if (fr->is_deoptimized_frame()) {
        guarantee(current != NULL, "must have vframeArray for deoptimized frame");
#ifdef IA64
	// IA64 can have zero sized frame. id is unique 
        guarantee(fr->id() == current->original().id(), "must find matching frame in vframeArray");
#else
        guarantee(fr->unextended_sp() == current->unextended_sp(), "must find matching frame in vframeArray");
#endif /* IA64 */
        current = current->next();
      }
    }
  }
  guarantee(current == NULL, "should not have any vframeArrays left");
}
#endif // CORE
#endif // PRODUCT

const char* JavaThread::get_thread_name() const {
  // Early safepoints hit while JavaThread::current does not yet have TLS
  debug_only(if (!(SafepointSynchronize::is_at_safepoint()
    || JavaThread::current() == this)) assert_locked_or_safepoint(Threads_lock);)
  oop thread_obj = threadObj();
  if (thread_obj != NULL) {
    typeArrayOop name = java_lang_Thread::name(thread_obj);
    const char* str = UNICODE::as_utf8((jchar*) name->base(T_CHAR), name->length());
    return str;
  } else {
    return NULL;
  }
  return NULL;
}

const char* JavaThread::get_threadgroup_name() const {
  debug_only(if (JavaThread::current() != this) assert_locked_or_safepoint(Threads_lock);)
  oop thread_obj = threadObj();
  if (thread_obj != NULL) {
    oop thread_group = java_lang_Thread::threadGroup(thread_obj);
    if (thread_group != NULL) {
      typeArrayOop name = java_lang_ThreadGroup::name(thread_group);
      const char* str = UNICODE::as_utf8((jchar*) name->base(T_CHAR), name->length());
      return str;
    }
  }
  return NULL;
}

const char* JavaThread::get_parent_name() const {
  debug_only(if (JavaThread::current() != this) assert_locked_or_safepoint(Threads_lock);)
  oop thread_obj = threadObj();
  if (thread_obj != NULL) {
    oop thread_group = java_lang_Thread::threadGroup(thread_obj);
    if (thread_group != NULL) {
      oop parent = java_lang_ThreadGroup::parent(thread_group);
      if (parent != NULL) {
        typeArrayOop name = java_lang_ThreadGroup::name(parent);
        const char* str = UNICODE::as_utf8((jchar*) name->base(T_CHAR), name->length());
        return str;
      }
    }
  }
  return NULL;
}

ThreadPriority JavaThread::java_priority() const {
  oop thr_oop = threadObj();
  if (thr_oop == NULL) return NormPriority; // Bootstrapping
  ThreadPriority priority = java_lang_Thread::priority(thr_oop);
  assert(MinPriority <= priority && priority <= MaxPriority, "sanity check");
  return priority;
}

void JavaThread::prepare(jthread jni_thread, ThreadPriority prio) {

  assert(Threads_lock->owner() == Thread::current(), "must have threads lock");
  // Link Java Thread object <-> C++ Thread

  // Get the C++ thread object (an oop) from the JNI handle (a jthread)
  // and put it into a new Handle.  The Handle "thread_oop" can then
  // be used to pass the C++ thread object to other methods.

  // Set the Java level thread object (jthread) field of the
  // new thread (a JavaThread *) to C++ thread object using the
  // "thread_oop" handle.
  
  // Set the thread field (a JavaThread *) of the
  // oop representing the java_lang_Thread to the new thread (a JavaThread *).

  Handle thread_oop(Thread::current(),
                    JNIHandles::resolve_non_null(jni_thread));
  assert(instanceKlass::cast(thread_oop->klass())->is_linked(), 
    "must be initialized");
  set_threadObj(thread_oop());
  java_lang_Thread::set_thread(thread_oop(), this);

  if (prio == NoPriority) {
    prio = java_lang_Thread::priority(thread_oop());
    assert(prio != NoPriority, "A valid priority should be present");
  }

  // Push the Java priority down to the native thread; needs Threads_lock
  Thread::set_priority(this, prio);

  // Add the new thread to the Threads list and set it in motion. 
  // We must have threads lock in order to call Threads::add. 
  // It is crucial that we do not block before the thread is 
  // added to the Threads list for if a GC happens, then the java_thread oop
  // will not be visited by GC.
  Threads::add(this);
}

frame JavaThread::cook_last_frame(frame fr) {
  
  address pc = fr.pc();
  assert(pc != NULL, "cannot be null");

// QQQ a temp hack until ia64 can get pc's
#if !defined(CORE) && !defined(IA64)
  // Check if frame pointer has been patched and/or pointing into temp. buffer.
  address adjusted_pc = safepoint_state()->compute_adjusted_pc(pc);
  // This should really be an assert - but to easy debugging it is currently a guarantee. 
  // QQQ for C++ interpreter this assert is too weak (AbstractInterpreter::contains())
  assert(Interpreter::contains(adjusted_pc) || CodeCache::contains(adjusted_pc) || StubRoutines::contains(adjusted_pc), "invalid pc");
#else
  // No adjustment needed for interpreter-only system
  address adjusted_pc = pc;  
#endif    
  fr.set_pc(adjusted_pc);
  return fr;
}


//
// Fabricate heavyweight monitor information for each lightweight monitor
// found in this Java thread.
//
void JavaThread::jvmpi_fab_heavy_monitors(bool query, int* index, GrowableArray<ObjectMonitor*>* fab_list) {
  assert(SafepointSynchronize::is_at_safepoint(), "must be at safepoint");

  if (!has_last_Java_frame()) return;
  ResourceMark rm;
  HandleMark   hm;
  RegisterMap reg_map(this);
  vframe* start_vf = last_java_vframe(&reg_map);
  int count = 0;
  for (vframe* f = start_vf; f != NULL; f = f->sender()) {
    if (count++ >= MaxJavaStackTraceDepth) return;  // stack too deep

    if (!f->is_java_frame()) continue;
    javaVFrame* jvf = javaVFrame::cast(f);
    jvf->jvmpi_fab_heavy_monitors(query, index, count, fab_list);
  }
}


//
// Count the number of entries for a lightweight monitor. The hobj
// parameter is object that owns the monitor so this routine will
// count the number of times the same object was locked by this
// JavaThread by looking through its javaVframes.
//
jint JavaThread::count_lock_entries(Handle hobj) {
  if (!has_last_Java_frame()) {
    return 0;  // no Java frames so no monitors
  }

  ResourceMark rm;
  HandleMark   hm;
  RegisterMap  reg_map(this);

  int depth = 0;
  jint ret = 0;
  for (javaVFrame *jvf = last_java_vframe(&reg_map); jvf != NULL;
    jvf = jvf->java_sender()) {

    if (depth++ >= MaxJavaStackTraceDepth) {
      return ret;  // stack too deep
    }
    ret += jvf->count_lock_entries(hobj);
  }

  return ret;
}


// Save global JNI handles for any objects that this thread owns.
void JavaThread::save_lock_entries(GrowableArray<jobject>* handle_list) {
  if (!has_last_Java_frame()) {
    return;  // no Java frames so no monitors
  }

  ResourceMark rm;
  HandleMark   hm;
  RegisterMap  reg_map(this);

  int depth = 0;
  for (javaVFrame *jvf = last_java_vframe(&reg_map); jvf != NULL;
    jvf = jvf->java_sender()) {

    if (depth++ >= MaxJavaStackTraceDepth) {
      return;  // stack too deep
    }
    jvf->save_lock_entries(handle_list);
  }
}


void JavaThread::print_stack() {  
  if (!has_last_Java_frame()) return;
  ResourceMark rm;
  HandleMark   hm;
  RegisterMap reg_map(this);
  vframe* start_vf = last_java_vframe(&reg_map);
  int count = 0;
  for (vframe* f = start_vf; f; f = f->sender() ) {
    if (f->is_java_frame()) {
      javaVFrame* jvf = javaVFrame::cast(f);
      java_lang_Throwable::print_stack_element(Handle(), jvf->method(), jvf->bci());
      
      // Print out lock information
      if (JavaMonitorsInStackTrace) {
        jvf->print_lock_info(count);
      }
    } else {
      // Ignore non-Java frames
    }

    // Bail-out case for too deep stacks
    count++;
    if (MaxJavaStackTraceDepth == count) return;    
  }
}


#ifdef HOTSWAP
#ifndef CORE
void JavaThread::popframe_preserve_args(ByteSize size_in_bytes, void* start) {
  assert(_popframe_preserved_args == NULL, "should not wipe out old PopFrame preserved arguments");
  if (in_bytes(size_in_bytes) != 0) {
    _popframe_preserved_args = NEW_C_HEAP_ARRAY(char, in_bytes(size_in_bytes));
    _popframe_preserved_args_size = in_bytes(size_in_bytes);
    Memory::copy_bytes_overlapping(start, _popframe_preserved_args, _popframe_preserved_args_size);
  }
}

void* JavaThread::popframe_preserved_args() {
  return _popframe_preserved_args;
}

ByteSize JavaThread::popframe_preserved_args_size() {
  return in_ByteSize(_popframe_preserved_args_size);
}

WordSize JavaThread::popframe_preserved_args_size_in_words() {
  int sz = in_bytes(popframe_preserved_args_size());
  assert(sz % wordSize == 0, "argument size must be multiple of wordSize");
  return in_WordSize(sz / wordSize);
}

void JavaThread::popframe_free_preserved_args() {
  assert(_popframe_preserved_args != NULL, "should not free PopFrame preserved arguments twice");
  FREE_C_HEAP_ARRAY(char, (char*) _popframe_preserved_args);
  _popframe_preserved_args = NULL;
  _popframe_preserved_args_size = 0;
}
#endif /* !CORE */
#endif HOTSWAP


#ifndef PRODUCT

void JavaThread::trace_frames() {
  tty->print_cr("[Describe stack]");
  int frame_no = 1;
  for(StackFrameStream fst(this); !fst.is_done(); fst.next()) {
    tty->print("  %d. ", frame_no++);
    fst.current()->print_value_on(tty,this);
    tty->cr();
  }
}


void JavaThread::trace_stack_from(vframe* start_vf) {
  ResourceMark rm;
  int vframe_no = 1;
  for (vframe* f = start_vf; f; f = f->sender() ) {
    if (f->is_java_frame()) {
      javaVFrame::cast(f)->print_activation(vframe_no++);
    } else {
      f->print();
    }
    if (vframe_no > StackPrintLimit) {
      tty->print_cr("...<more frames>...");
      return;
    }
  }
}


void JavaThread::trace_stack() {
  if (!has_last_Java_frame()) return;
  ResourceMark rm;
  HandleMark   hm;
  RegisterMap reg_map(this);
  trace_stack_from(last_java_vframe(&reg_map));
}


#endif // PRODUCT


javaVFrame* JavaThread::last_java_vframe(RegisterMap *reg_map) {
  assert(reg_map != NULL, "a map must be given");
  frame f = last_frame();
  for (vframe* vf = vframe::new_vframe(&f, reg_map, this); vf; vf = vf->sender() ) {
    if (vf->is_java_frame()) return javaVFrame::cast(vf);
  }
  return NULL;
}


klassOop JavaThread::security_get_caller_class(int depth) {  
  vframeStream vfst(this);
  vfst.security_get_caller_frame(depth);
  if (!vfst.at_end()) {
    return vfst.method()->method_holder();
  }    
  return NULL;
}

#ifndef CORE
static void compiler_thread_entry(JavaThread* thread, TRAPS) {
  assert(thread->is_Compiler_thread(), "must be compiler thread");
  CompileBroker::compiler_thread_loop();
}

// Create a CompilerThread
CompilerThread::CompilerThread(CompileQueue* queue, CompilerCounters* counters)
: JavaThread(&compiler_thread_entry) {
  _env    = NULL;
  _compile_data = NULL;
  _log   = NULL;
  _task  = NULL;
  _queue = queue;
  _counters = counters;
}
#endif


// ======= Threads ========

// The Threads class links together all active threads, and provides
// operations over all threads.  It is protected by its own Mutex
// lock, which is also used in other contexts to protect thread
// operations from having the thread being operated on from exiting
// and going away unexpectedly (e.g., safepoint synchronization)

JavaThread* Threads::_thread_list = NULL;
int         Threads::_number_of_threads = 0;
int         Threads::_number_of_non_daemon_threads = 0;
int         Threads::_return_code = 0;
size_t      JavaThread::_stack_size_at_create = 0;


#define ALL_THREADS(X) for (JavaThread* X = _thread_list; X; X = X->next())

void os_stream();



jint Threads::create_vm(JavaVMInitArgs* args) {
  
  // Check version
  if (!is_supported_jni_version(args->version)) return JNI_EVERSION;

  // Initialize the output stream module
  ostream_init();

  // Initialize the os module before using TLS
  os::init();

  // Parse arguments
  jint parse_result = Arguments::parse(args);
  if (parse_result != JNI_OK) return parse_result;

  // Timing (must come after argument parsing)
  TraceTime timer("Create VM", TraceStartupTime);

  // Initialize the os module after parsing the args
  jint os_init_2_result = os::init_2();
  if (os_init_2_result != JNI_OK) return os_init_2_result;

  // Initialize output stream logging
  ostream_init_log();

  // Initialize Threads state
  _thread_list = NULL;
  _number_of_threads = 0;
  _number_of_non_daemon_threads = 0;

  // Initialize TLS
  ThreadLocalStorage::init();

  // Initialize global data structures and create system classes in heap
  vm_init_globals();

  // Attach the main thread to this os thread
  JavaThread* main_thread = new JavaThread();
  main_thread->set_thread_state(_thread_in_vm);
  // must do this before set_active_handles and initialize_thread_local_storage
  // Note: on solaris initialize_thread_local_storage() will (indirectly)
  // change the stack size recorded here to one based on the java thread 
  // stacksize. This adjusted size is what is used to figure the placement
  // of the guard pages.
  main_thread->record_stack_base_and_size(); 
  main_thread->initialize_thread_local_storage();

  main_thread->set_active_handles(JNIHandleBlock::allocate_block());

  main_thread->create_stack_guard_pages();

  main_thread->set_as_starting_thread();  
  HandleMark hm;

  // Initialize global modules
  init_globals();

  { MutexLocker mu(Threads_lock);
    Threads::add(main_thread);
  }

  if (VerifyBeforeGC &&
      Universe::heap()->total_collections() >= VerifyGCStartAt) {
    Universe::heap()->prepare_for_verify();
    Universe::verify();   // make sure we're starting with a clean slate
  }

  // Create the VMThread  
  { TraceTime timer("Start VMThread", TraceStartupTime);
    VMThread::create();
    Thread* vmthread = VMThread::vm_thread();
    os::create_thread(vmthread, os::vm_thread);

    // Wait for the VM thread to become ready, and VMThread::run to initialize
    // Monitors can have spurious returns, must always check another state flag
    {
      MutexLocker ml(Notify_lock);
      os::start_thread(vmthread);
      while (vmthread->active_handles() == NULL) {
        Notify_lock->wait();
      }
    }
  }
  
  assert (Universe::is_fully_initialized(), "not initialized");
  EXCEPTION_MARK;
  {
    TraceTime timer("Initialize java.lang classes", TraceStartupTime);

    if (EagerXrunInit && Arguments::init_libraries_at_startup()) {
      create_vm_init_libraries();
    }

    if (InitializeJavaLangString) {
      initialize_class(vmSymbolHandles::java_lang_String(), CHECK_0);
    } else {
      warning("java.lang.String not initialized");
    }

    // Initialize java_lang.System (needed before creating the thread)
    if (InitializeJavaLangSystem) {      
      initialize_class(vmSymbolHandles::java_lang_System(), CHECK_0);
      initialize_class(vmSymbolHandles::java_lang_ThreadGroup(), CHECK_0);
      Handle thread_group = create_initial_thread_group(CHECK_0);
      Universe::set_main_thread_group(thread_group());
      initialize_class(vmSymbolHandles::java_lang_Thread(), CHECK_0);
      oop thread_object = create_initial_thread(thread_group, main_thread, CHECK_0);
      main_thread->set_threadObj(thread_object);
      // The VM preresolve methods to these classes. Make sure that get initialized
      initialize_class(vmSymbolHandles::java_lang_reflect_Method(), CHECK_0);
      initialize_class(vmSymbolHandles::java_lang_ref_Finalizer(),  CHECK_0);
      call_initializeSystemClass(CHECK_0);
    } else {
      warning("java.lang.System not initialized");
    }

    // an instance of OutOfMemory exception has been allocated earlier
    if (InitializeJavaLangExceptionsErrors) {
      initialize_class(vmSymbolHandles::java_lang_OutOfMemoryError(), CHECK_0);
      initialize_class(vmSymbolHandles::java_lang_NullPointerException(), CHECK_0);
      initialize_class(vmSymbolHandles::java_lang_ClassCastException(), CHECK_0);
      initialize_class(vmSymbolHandles::java_lang_ArrayStoreException(), CHECK_0);
      initialize_class(vmSymbolHandles::java_lang_ArithmeticException(), CHECK_0);
      initialize_class(vmSymbolHandles::java_lang_StackOverflowError(), CHECK_0);      
    } else {
      warning("java.lang.OutOfMemoryError has not been initialized");
      warning("java.lang.NullPointerException has not been initialized");
      warning("java.lang.ClassCastException has not been initialized");
      warning("java.lang.ArrayStoreException has not been initialized");
      warning("java.lang.ArithmeticException has not been initialized");
      warning("java.lang.StackOverflowError has not been initialized");
    }
  }
  
  // See        : bugid 4211085.  
  // Background : the static initializer of java.lang.Compiler tries to read 
  //              property"java.compiler" and read & write property "java.vm.info".
  //              When a security manager is installed through the command line
  //              option "-Djava.security.manager", the above properties are not
  //              readable and the static initializer for java.lang.Compiler fails
  //              resulting in a NoClassDefFoundError.  This can happen in any
  //              user code which calls methods in java.lang.Compiler.  
  // Hack :       the hack is to pre-load and initialize this class, so that only
  //              system domains are on the stack when the properties are read.
  //              Currently even the AWT code has calls to methods in java.lang.Compiler.
  //              On the classic VM, java.lang.Compiler is loaded very early to load the JIT.
  // Future Fix : the best fix is to grant everyone permissions to read "java.compiler" and 
  //              read and write"java.vm.info" in the default policy file. See bugid 4211383
  //              Once that is done, we should remove this hack.
  initialize_class(vmSymbolHandles::java_lang_Compiler(), CHECK_0);
  
  // More hackery - the static initializer of java.lang.Compiler adds the string "nojit" to
  // the java.vm.info property if no jit gets loaded through java.lang.Compiler (the hotspot
  // compiler does not get loaded through java.lang.Compiler).  "java -version" with the 
  // hotspot vm says "nojit" all the time which is confusing.  So, we reset it here.
  // This should also be taken out as soon as 4211383 gets fixed.
  reset_vm_info_property(CHECK_0);
  
  // Set flag that basic initialization has completed. Used by exceptions and various 
  // debug stuff, that does not work until all basic classes have been initialized.
  set_init_completed();

  // Compute system loader. Note that this has to occur after set_init_completed, since
  // valid exceptions may be thrown in the process. 
  // Note that we do not use CHECK_0 here since we are inside an EXCEPTION_MARK and 
  // set_init_completed has just been called, causing exceptions not to be shortcut
  // anymore. We call vm_exit_during_initialization directly instead.
  SystemDictionary::compute_java_system_loader(THREAD);
  if (HAS_PENDING_EXCEPTION) {
    vm_exit_during_initialization(Handle(THREAD, PENDING_EXCEPTION));
  }
  
  if (!EagerXrunInit && Arguments::init_libraries_at_startup()) {
    create_vm_init_libraries();
  }

  // Support for ConcurrentMarkSweep. This should be cleaned up
  // and better encapsulated. XXX YSR
  if (UseConcMarkSweepGC) {
    ConcurrentMarkSweepThread::makeSurrogateLockerThread(THREAD);
    if (HAS_PENDING_EXCEPTION) {
      vm_exit_during_initialization(Handle(THREAD, PENDING_EXCEPTION));
    }
  }

  // Notify tool interfaces that VM initialization is complete.
  if (jvmdi::enabled()) {
    jvmdi::post_vm_initialized_event();
  }
  if (jvmpi::enabled() && UseTrainGC) {
    vm_exit_during_initialization("JVMPI not supported with option -Xincgc");
  }
  if (jvmpi::enabled()) {
    // Issue class load, thread start and object allocation events for all
    // preloaded classes
    jvmpi::post_vm_initialization_events();
  }
  if (jvmpi::is_event_enabled(JVMPI_EVENT_JVM_INIT_DONE)) {
    // Tell the agent that the VM is ready and events requested (e.g. heap dump)
    // from this point onwards will contains IDs that are all known to the agent
    jvmpi::post_vm_initialized_event();
  }

  SuspendCheckerThread::start();

  Chunk::start_chunk_pool_cleaner_task();
#ifndef CORE
  RecompilationMonitor::start_recompilation_monitor_task();  
#endif
  os::signal_init();

  // initialize compiler
  COMPILER1_ONLY(CompileBroker::compilation_init(new Compiler());)
  COMPILER2_ONLY(CompileBroker::compilation_init(new C2Compiler());)

  if (Arguments::has_profile())       FlatProfiler::engage(main_thread, true);
  if (Arguments::has_alloc_profile()) AllocationProfiler::engage();
  if (MemProfiling)                   MemProfiler::engage();    
  StatSampler::engage();    

  // Start up the WatcherThread if there are any periodic tasks
  // NOTE:  All PeriodicTasks should be registered by now. If they
  //   aren't, late joiners might appear to start slowly (we might
  //   take a while to process their first tick).
  if (PeriodicTask::num_tasks() > 0) {
    WatcherThread::start();
  }

  return JNI_OK;
}

extern "C" {
  typedef jint (JNICALL *JVM_OnLoad_t)(JavaVM *, char *, void *);
}
void Threads::create_vm_init_libraries() {
  const char *on_load_symbols[] = JVM_ONLOAD_SYMBOLS;
  extern struct JavaVM_ main_vm; // Should this be fetched, instead?

  for(int i=0; i<Arguments::library_count(); i++) {
    char buffer[JVM_MAXPATHLEN];
    JVM_OnLoad_t JVM_OnLoad = NULL;
    void *library;
    char ebuf[1024];

    // Try to load the library from the standard dll directory
    hpi::dll_build_name(buffer, sizeof(buffer), os::get_system_properties()->dll_dir, Arguments::library_name(i));
    library = hpi::dll_load(buffer, ebuf, sizeof ebuf);
    if(library == NULL) { // Try the local directory
      char ns[1] = {0};
      hpi::dll_build_name(buffer, sizeof(buffer), ns, Arguments::library_name(i));
      library = hpi::dll_load(buffer, ebuf, sizeof ebuf);
      if (library == NULL) {
        // If we can't find the library, exit.
        vm_exit_during_initialization("Could not find -Xrun library", buffer);
      }
    }

    // Find the JVM_OnLoad function.
    for (int symbol_index = 0; symbol_index<sizeof(on_load_symbols) / sizeof(char*); symbol_index++) {
      JVM_OnLoad = CAST_TO_FN_PTR(JVM_OnLoad_t, hpi::dll_lookup(library, on_load_symbols[symbol_index]));
      if(JVM_OnLoad != NULL) break;
    }

    // Invoke the JVM_OnLoad function
    if (JVM_OnLoad != NULL) {
      JavaThread* thread = JavaThread::current();
      StateSaver nsv(thread); // we must have post Java state set while in native
      ThreadToNativeFromVM ttn(thread);
      HandleMark hm(thread);
      jint err = (*JVM_OnLoad)(&main_vm, Arguments::library_options(i), NULL);
      if (err != 0) {
        vm_exit_during_initialization("-Xrun library failed to init", Arguments::library_name(i));
      }
    } else {
      vm_exit_during_initialization("Could not find JVM_OnLoad function in -Xrun library", Arguments::library_name(i));
    }
  }
}

// Threads::destroy_vm() is normally called from jni_DestroyJavaVM() when 
// the program falls off the end of main(). Another VM exit path is through 
// vm_exit() when the program calls System.exit() to return a value or when 
// there is a serious error in VM. The two shutdown paths are not exactly
// the same, but they share Shutdown.shutdown() at Java level and before_exit()
// and VM_Exit op at VM level.
//
// Shutdown sequence:
//   + Wait until we are the last non-daemon thread to execute
//     <-- every thing is still working at this moment -->
//   + Call before_exit(), prepare for VM exit
//      > run VM level shutdown hooks (they are registered through JVM_OnExit(),
//        currently the only user of this mechanism is File.deleteOnExit())
//      > stop flat profiler, StatSampler, watcher thread, CMS threads,
//        post thread end and vm death events to JVMDI/PI, disable JVMPI,
//        stop signal thread
//   + Call JavaThread::exit(), it will:
//      > call java.lang.Shutdown.shutdown(), which will invoke Java level 
//        shutdown hooks, run finalizers if finalization-on-exit
//      > release JNI handle blocks, remove stack guard pages
//      > remove this thread from Threads list
//     <-- no more Java code from this thread after this point -->
//   + Stop SuspendCheckerThread
//   + Stop VM thread, it will bring the remaining VM to a safepoint and stop
//     the compiler threads at safepoint
//     <-- do not use anything that could get blocked by Safepoint -->
//   + Disable tracing at JNI/JVM/JVMDI/JVMPI barriers
//   + Set _vm_exited flag for threads that are still running native code
//   + Delete this thread
//   + Call exit_globals()
//      > deletes tty
//      > deletes PerfMemory resources
//   + Return to caller

bool Threads::destroy_vm() {
  JavaThread* thread = JavaThread::current();

  // Wait until we are the last non-daemon thread to execute
  { MutexLocker nu(Threads_lock);
    while (Threads::number_of_non_daemon_threads() > 1 )
      Threads_lock->wait();
  }

  if (Universe::is_jdk12x_version()) {
    // We are the last thread running, so check if finalizers should be run.
    // For 1.3 or later this is done in thread->exit(true)
    HandleMark rm(thread);
    Universe::run_finalizers_on_exit();
  }

  before_exit(thread);

  // this will call Java level shutdown hooks
  thread->exit(true);

  // Terminate suspend checker thread.
  SuspendCheckerThread::stop();

  // Stop VM thread.
  VMThread::wait_for_vm_thread_exit();
  assert(SafepointSynchronize::is_at_safepoint(), "VM thread should exit at Safepoint");
  VMThread::destroy();

  // Now, all Java threads are gone except daemon threads. Daemon threads
  // running Java code or in VM are stopped by the Safepoint. However,
  // daemon threads executing native code are still running.  But they
  // will be stopped at native=>Java/VM barriers. Note that we can't 
  // simply kill or suspend them, as it is inherently deadlock-prone.

#ifndef PRODUCT
  // disable function tracing at JNI/JVM/JVMPI/JVMDI barriers
  TraceHPI = false;
  TraceJNICalls = false;
  TraceJVMCalls = false;
  TraceRuntimeCalls = false;
  TraceJVMPI = false;
  TraceJVMDICalls = false;
#endif

  VM_Exit::set_vm_exited();

  thread->delete_thread_and_TLS_current();

  // exit_globals() will delete tty
  exit_globals();

  return true;
}


jboolean Threads::is_supported_jni_version_including_1_1(jint version) {
  if (version == JNI_VERSION_1_1) return JNI_TRUE;
  return is_supported_jni_version(version);
}


jboolean Threads::is_supported_jni_version(jint version) {
  if (version == JNI_VERSION_1_2) return JNI_TRUE;
  if (version == JNI_VERSION_1_4) return JNI_TRUE;
  return JNI_FALSE;
}


void Threads::add(JavaThread* p, bool force_daemon) {
  // The threads lock must be owned at this point
  assert_locked_or_safepoint(Threads_lock);
  p->set_next(_thread_list);  
  _thread_list = p;
  _number_of_threads++;
  oop threadObj = p->threadObj();
  // Bootstrapping problem: threadObj can be null for initial
  // JavaThread (or for threads attached via JNI)
  if ((!force_daemon) && (threadObj == NULL || !java_lang_Thread::is_daemon(threadObj))) {
    _number_of_non_daemon_threads++;
  }

  // Possible GC point.
  Events::log("Thread added: " INTPTR_FORMAT, p);
}

void Threads::remove(JavaThread* p) {  
  // Extra scope needed for Thread_lock, so we can check
  // that we do not remove thread without safepoint code notice
  { MutexLocker ml(Threads_lock);
  
    assert(includes(p), "p must be present");

    JavaThread* current = _thread_list; 
    JavaThread* prev    = NULL;

    while (current != p) {
      prev    = current;
      current = current->next();
    }

    if (prev) {
      prev->set_next(current->next());
    } else {
      _thread_list = p->next();
    }  
    _number_of_threads--;
    oop threadObj = p->threadObj();
    if (threadObj == NULL || !java_lang_Thread::is_daemon(threadObj)) {
      _number_of_non_daemon_threads--;
      // Only one thread left, do a notify on the Threads_lock so a thread waiting
      // on destroy_vm will wake up.
      if (number_of_non_daemon_threads() == 1) 
        Threads_lock->notify_all();
    }    

    // Make sure that safepoint code disregard this thread. This is needed since
    // the thread might mess around with locks after this point. This can cause it
    // to do callbacks into the safepoint code. However, the safepoint code is not aware
    // of this thread since it is removed from the queue.
    p->set_terminated_value();        
  } // unlock Threads_lock 

  // Since Events::log uses a lock, we grab it outside the Threads_lock
  Events::log("Thread exited: " INTPTR_FORMAT, p);
}

// Threads_lock must be held when this is called (or must be called during a safepoint)
bool Threads::includes(JavaThread* p) {  
  assert(Threads_lock->is_locked(), "sanity check");
  ALL_THREADS(q) {
    if (q == p ) {
      return true;
    }
  }
  return false;
}

// Operations on the Threads list for GC.  These are not explicitly locked,
// but the garbage collector must provide a safe context for them to run.
// In particular, these things should never be called when the Threads_lock
// is held by some other thread. (Note: the Safepoint abstraction also
// uses the Threads_lock to gurantee this property. It also makes sure that  
// all threads gets blocked when exiting or starting).

void Threads::oops_do(OopClosure* f) {
  ALL_THREADS(p) {
    p->oops_do(f);
  }
  VMThread::vm_thread()->oops_do(f);
}

void Threads::possibly_parallel_oops_do(OopClosure* f) {
  // Introduce a mechanism allowing parallel threads to claim threads as
  // root groups.  Overhead should be small enough to use all the time,
  // even in sequential code.
  SharedHeap* sh = SharedHeap::heap();
  bool is_par = (sh->n_par_threads() > 0);
  int cp = SharedHeap::heap()->strong_roots_parity();
  ALL_THREADS(p) {
    if (p->claim_oops_do(is_par, cp)) {
      p->oops_do(f);
    }
  }
  VMThread* vmt = VMThread::vm_thread();
  if (vmt->claim_oops_do(is_par, cp))
    vmt->oops_do(f);
}

// Used by ParallelScavenge
void Threads::create_thread_roots_tasks(GCTaskQueue* q) {
  ALL_THREADS(p) {
    q->enqueue(new ThreadRootsTask(p));
  }
  q->enqueue(new ThreadRootsTask(VMThread::vm_thread()));
}

void Threads::nmethods_do() {
  ALL_THREADS(p) {
    p->nmethods_do();
  }
  VMThread::vm_thread()->nmethods_do();
}

void Threads::gc_epilogue() {
  ALL_THREADS(p) {
    p->gc_epilogue();
  }
}

void Threads::gc_prologue() {
  ALL_THREADS(p) {    
    p->gc_prologue();
  }
}

void Threads::deoptimized_wrt_marked_nmethods() {
  ALL_THREADS(p) {
    p->deoptimized_wrt_marked_nmethods();
  }
}


//
// Fabricate heavyweight monitor information for each lightweight monitor
// found in every Java thread.
//
GrowableArray<ObjectMonitor*>* Threads::jvmpi_fab_heavy_monitors() {
  assert(SafepointSynchronize::is_at_safepoint(), "must be at safepoint");

  // In the first pass we have to count the number of monitors needed
  int index = 0;
  ALL_THREADS(p) {
    NOT_CORE(if (p->is_Compiler_thread()) continue;)

    p->jvmpi_fab_heavy_monitors(true, &index, NULL);
  }

  GrowableArray<ObjectMonitor*>* result = new GrowableArray<ObjectMonitor*>(index, index, NULL);
  //
  // In the second pass we actually get the data. We can't call append()
  // down in javaVFrame::jvmpi_fab_heavy_monitors() because there are other
  // uses of ResourceMark between here and there.
  //
  index = 0;
  ALL_THREADS(q) {
    NOT_CORE(if (q->is_Compiler_thread()) continue;)

    q->jvmpi_fab_heavy_monitors(false, &index, result);
  }
  return result;
}


// Get count Java threads that are waiting to enter the specified monitor.
GrowableArray<JavaThread*>* Threads::get_pending_threads(int count,
  address monitor, bool doLock) {
  assert(doLock || SafepointSynchronize::is_at_safepoint(),
    "must grab Threads_lock or be at safepoint");
  GrowableArray<JavaThread*>* result = new GrowableArray<JavaThread*>(count);

  int i = 0;
  {
    MutexLockerEx ml(doLock ? Threads_lock : NULL);
    ALL_THREADS(p) {
      NOT_CORE(if (p->is_Compiler_thread()) continue;)

      address pending = (address)p->current_pending_monitor();
      if (pending == monitor) {             // found a match
        if (i < count) result->append(p);   // save the first count matches
        i++;
      }
    }
  }
  CORE_ONLY(assert(count == i, "Did not find the expected number of pending Java threads");)
  return result;
}


JavaThread *Threads::owning_thread_from_monitor_owner(address owner, bool doLock) {
  assert(doLock || SafepointSynchronize::is_at_safepoint(),
    "must grab Threads_lock or be at safepoint");

  // NULL owner means not locked so we can skip the search
  if (owner == NULL) return NULL;

  {
    MutexLockerEx ml(doLock ? Threads_lock : NULL);
    ALL_THREADS(p) {
      // first, see if owner is the address of a Java thread
      if (owner == (address)p) return p;
    }
  }
  CORE_ONLY(assert(UseHeavyMonitors == false, "Did not find owning Java thread with UseHeavyMonitors enabled");)
  if (UseHeavyMonitors) return NULL;

  //
  // If we didn't find a matching Java thread and we didn't force use of
  // heavyweight monitors, then the owner is the stack address of the
  // Lock Word in the owning Java thread's stack.
  //
  // We can't use Thread::is_lock_owned() or Thread::is_in_stack() because
  // those routines rely on the "current" stack pointer. That would be our
  // stack pointer which is not relevant to the question. Instead we use the
  // highest lock ever entered by the thread and find the thread that is
  // higher than and closest to our target stack address.
  //
  address    least_diff = 0;
  bool       least_diff_initialized = false;
  JavaThread* the_owner = NULL;
  {
    MutexLockerEx ml(doLock ? Threads_lock : NULL);
    ALL_THREADS(q) {
      address addr = q->highest_lock();
      if (addr == NULL || addr < owner) continue;  // thread has entered no monitors or is too low
      address diff = (address)(addr - owner);
      if (!least_diff_initialized || diff < least_diff) {
        least_diff_initialized = true;
        least_diff = diff;
        the_owner = q;
      }
    }
  }
  assert(the_owner != NULL, "Did not find owning Java thread for lock word address");
  return the_owner;
}


void Threads::print(bool print_stacks, bool internal_format) {
  tty->print_cr("Full thread dump %s (%s %s):",
                 Abstract_VM_Version::vm_name(),
                 Abstract_VM_Version::vm_release(),
                 Abstract_VM_Version::vm_info_string()
                );
  tty->cr();
  ALL_THREADS(p) {
    NOT_CORE(if (p->is_Compiler_thread()) continue;)

    ResourceMark rm;
    p->print();
    if (print_stacks) {
      if (internal_format) {
        p->trace_stack();
      } else {
        p->print_stack();
      }
    }
    tty->cr();
  }
  VMThread::vm_thread()->print();
  tty->cr();
  WatcherThread::watcher_thread()->print();
  SuspendCheckerThread::suspend_checker_thread()->print();
  NOT_CORE(CompileBroker::print_compiler_threads();)
}

// We know thread is involved in a deadlock. Print the deadlock cycle.
static void print_one_deadlock(JavaThread* thread) {
  tty->cr();
  tty->print_cr("Found one Java-level deadlock:");
  tty->print("=============================");
  ObjectMonitor* waitingToLock;
  JavaThread* currentThread = thread;
  do {
    tty->cr();
    tty->print_cr("\"%s\":", currentThread->get_thread_name());
    waitingToLock = (ObjectMonitor*)currentThread->current_pending_monitor();
    tty->print("  waiting to lock monitor " INTPTR_FORMAT, waitingToLock);
    oop obj = (oop)waitingToLock->object();
    if (obj != NULL) {
      tty->print(" (object "INTPTR_FORMAT ", a %s)", obj, 
                 (instanceKlass::cast(obj->klass()))->external_name());
      if (currentThread->current_pending_monitor_is_from_java()) {
        tty->print(",\n  which is held by");
      } else {
        tty->print("\n  in JNI, which is held by");
      }
    } else {
      // No Java object associated - a JVMDI/JVMPI raw monitor
      tty->print(" (JVMDI/JVMPI raw monitor),\n  which is held by");
    }
    currentThread = Threads::owning_thread_from_monitor_owner(
      (address)waitingToLock->owner(), false /* no locking needed */);
    tty->print(" \"%s\"", currentThread->get_thread_name());
  } while (currentThread != thread);
  tty->cr();
  tty->cr();

  bool oldJavaMonitorsInStackTrace = JavaMonitorsInStackTrace;
  JavaMonitorsInStackTrace = true;
  tty->print_cr("Java stack information for the threads listed above:");
  tty->print_cr("===================================================");
  currentThread = thread;
  do {
    tty->print_cr("\"%s\":", currentThread->get_thread_name());
    currentThread->print_stack();
    waitingToLock = (ObjectMonitor*)currentThread->current_pending_monitor();
    currentThread = Threads::owning_thread_from_monitor_owner(
      (address)waitingToLock->owner(), false /* no locking needed */);
  } while (currentThread != thread);
  JavaMonitorsInStackTrace = oldJavaMonitorsInStackTrace;
}

// Sketch of the Deadlock Finding Algorithm (from EVM):
// We use a "depth-first" traversal of the waits-for graph, looking for
// cycles. The waits-for graph is constructed on the fly. The cycle detection
// is simplified since our graph has the property that the out-degree of each
// node (thread) is at most 1.

void Threads::find_deadlocks() {
  int globalDfn = 0, thisDfn;
  ObjectMonitor* waitingToLock = NULL;
  JavaThread *currentThread, *previousThread;
  int number_of_deadlocks = 0;

  ALL_THREADS(p) {
    // Initialize the depth-first-number
    p->set_depth_first_number(-1);
  }

  ALL_THREADS(q) {
    if (q->depth_first_number() >= 0) {
      // this thread was already visited
      continue;
    }

    thisDfn = globalDfn;
    q->set_depth_first_number(globalDfn++);
    previousThread = q;

    // When there is a deadlock, all the monitors involved in the dependency
    // cycle must be contended and heavyweight. So we only care about the
    // heavyweight monitor a thread is waiting to lock.
    waitingToLock = (ObjectMonitor*)q->current_pending_monitor();
    while (waitingToLock != NULL) {
      currentThread = Threads::owning_thread_from_monitor_owner(
        (address)waitingToLock->owner(), false /* no locking needed */);
      if (currentThread == NULL) {
        // No dependency on another thread
        break;
      }
      if (currentThread->depth_first_number() < 0) {
        // First visit to this thread
        currentThread->set_depth_first_number(globalDfn++);
      } else if (currentThread->depth_first_number() < thisDfn) {
        // Thread already visited, and not on a (new) cycle
        break;
      } else if (currentThread == previousThread) {
        // Self-loop, ignore
        break;
      } else {
        // We have a (new) cycle
        number_of_deadlocks ++;
        print_one_deadlock(currentThread);
        break;
      }
      previousThread = currentThread;
      waitingToLock = (ObjectMonitor*)currentThread->current_pending_monitor();
    }
  }

  if (number_of_deadlocks == 1) {
    tty->print_cr("\nFound 1 deadlock.\n");
  } else if (number_of_deadlocks > 1) {
    tty->print_cr("\nFound %d deadlocks.\n", number_of_deadlocks);
  }
}

//------------------------------------------------------------------------------------------------------------------
// Debugging code. Not included in PRODUCT version

#ifndef PRODUCT

void Threads::verify() {  
  ALL_THREADS(p) {
    p->verify();
  }
  VMThread* thread = VMThread::vm_thread();
  if (thread != NULL) thread->verify();
}

void Threads::print_on_error(JavaThread* current) {
  tty->print_cr("Java Threads:");
  ALL_THREADS(p) {
    ResourceMark rm;
    if (current == p) {
      tty->print("[CURRENT] ");
    }
    p->print();
    p->trace_stack();
  }
}

#endif

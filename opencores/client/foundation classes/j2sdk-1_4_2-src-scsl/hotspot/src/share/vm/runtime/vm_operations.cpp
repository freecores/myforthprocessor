#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)vm_operations.cpp	1.137 03/01/23 12:27:12 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_vm_operations.cpp.incl"


void VM_Operation::set_calling_thread(Thread* thread, ThreadPriority priority) { 
  _calling_thread = thread; 
  assert(MinPriority <= priority && priority <= MaxPriority, "sanity check");
  _priority = priority;
}  


void VM_Operation::evaluate() {
  ResourceMark rm;
  if (TraceVMOperation) { 
    tty->print("["); 
    NOT_PRODUCT(print();)
  }
  doit();
  if (TraceVMOperation) { 
    tty->print_cr("]"); 
  }
}


void VM_Operation::report_error(const char* title, char* message) {  
  tty->print_cr("# Error happened during: %s", name());
  tty->print_cr("#");
}


#ifndef PRODUCT
void VM_Operation::print() { 
  const char* mode;
  switch(evaluation_mode()) {
    case _safepoint      : mode = "safepoint";       break;
    case _no_safepoint   : mode = "no safepoint";    break;
    case _concurrent     : mode = "concurrent";      break;
    case _async_safepoint: mode = "async safepoint"; break;
    default              : ShouldNotReachHere();
  }
  tty->print("VM_Operation %s Mode: %s", name(), mode); 
}
#endif // !PRODUCT


void VM_GC_Operation::acquire_pending_list_lock() {
  // we may enter this with pending exception set
  _notify_ref_lock = false;

  instanceRefKlass::acquire_pending_list_lock(&_pending_list_basic_lock);
}


void VM_GC_Operation::release_and_notify_pending_list_lock() {

  instanceRefKlass::release_and_notify_pending_list_lock(_notify_ref_lock,
							 &_pending_list_basic_lock);
}

// Allocations may fail in several threads at about the same time,
// resulting in multiple gc requests.  We only want to do one of them.

bool VM_GC_Operation::gc_count_changed() const { 
  return (_gc_count_before != Universe::heap()->total_collections());
}

bool VM_GC_Operation::doit_prologue() {
  assert(Thread::current()->is_Java_thread(), "just checking");
  _gc_count_before = Universe::heap()->total_collections();
  acquire_pending_list_lock();
  // If the GC count has changed someone beat us to the collection
  // Get the Heap_lock after the pending_list_lock.
  Heap_lock->lock();
  // Check invocations
  if (gc_count_changed()) {
    // skip collection
    Heap_lock->unlock();
    release_and_notify_pending_list_lock();
    _prologue_succeeded = false;
  } else {
    _prologue_succeeded = true;
  }
  return _prologue_succeeded;
}


void VM_GC_Operation::doit_epilogue() {
  assert(Thread::current()->is_Java_thread(), "just checking");
  // Release the Heap_lock first.
  Heap_lock->unlock();
  release_and_notify_pending_list_lock();
}

void VM_GenCollectForAllocation::doit() {
  GenCollectedHeap* gch = GenCollectedHeap::heap();
  _res = gch->satisfy_failed_allocation(_size, _large_noref, _tlab, _notify_ref_lock);
  assert(gch->is_in_or_null(_res), "result not in heap");
} 			

void VM_GenCollectFull::doit() {
  GenCollectedHeap* gch = GenCollectedHeap::heap();
  gch->do_full_collection(false /* clear_all_soft_refs */,
			  _max_level,
                          _notify_ref_lock);
} 			

// The following methods are used by the parallel scavenge collector
VM_ParallelGCFailedAllocation::VM_ParallelGCFailedAllocation(size_t size, bool is_noref, bool is_tlab) :
  VM_GC_Operation(Universe::heap()->total_collections()),
  _size(size),
  _is_noref(is_noref),
  _is_tlab(is_tlab),
  _result(NULL)
{
}

void VM_ParallelGCFailedAllocation::doit() {
  ParallelScavengeHeap* heap = (ParallelScavengeHeap*)Universe::heap();
  assert(heap->kind() == CollectedHeap::ParallelScavengeHeap, "must be a ParallelScavengeHeap");

  _result = heap->failed_mem_allocate(_notify_ref_lock, _size, _is_noref, _is_tlab);
}

VM_ParallelGCFailedPermanentAllocation::VM_ParallelGCFailedPermanentAllocation(size_t size) :
  VM_GC_Operation(Universe::heap()->total_collections()),
  _size(size),
  _result(NULL)
{
}

void VM_ParallelGCFailedPermanentAllocation::doit() {
  ParallelScavengeHeap* heap = (ParallelScavengeHeap*)Universe::heap();
  assert(heap->kind() == CollectedHeap::ParallelScavengeHeap, "must be a ParallelScavengeHeap");

  _result = heap->failed_permanent_mem_allocate(_notify_ref_lock, _size);
}

// Only used for System.gc() calls
VM_ParallelGCSystemGC::VM_ParallelGCSystemGC() :
  VM_GC_Operation(Universe::heap()->total_collections())
{
}

void VM_ParallelGCSystemGC::doit() {
  ParallelScavengeHeap* heap = (ParallelScavengeHeap*)Universe::heap();
  assert(heap->kind() == CollectedHeap::ParallelScavengeHeap, "must be a ParallelScavengeHeap");

  PSMarkSweep::invoke(_notify_ref_lock, false);
}

void VM_ThreadStop::doit() {
  assert(SafepointSynchronize::is_at_safepoint(), "must be at a safepoint");
  JavaThread* target = java_lang_Thread::thread(target_thread());  
  // Note that this now allows multiple ThreadDeath exceptions to be
  // thrown at a thread.
  if (target != NULL) {
    // the thread has run and is not already in the process of exiting
    target->send_thread_stop(throwable());
  }  
}


void VM_ThreadSuspend::doit() {
  assert(SafepointSynchronize::is_at_safepoint(), "must be at a safepoint");
  JavaThread* target = java_lang_Thread::thread(target_thread());  
  if (target != NULL) {
    // The thread has run and is not in the process of exiting.
    // Since we are at a safepoint, we don't have to refresh target
    // at every retry attempt.
    while (true) {
      // request that the thread be suspended
      target->send_java_suspend(is_native_now());

      if (!target->is_suspended_in_native()) break;  // no immediate suspend so we are done

      //
      // See if the VMThread is likely to deadlock due to this suspend:
      // - tell SuspendCheckerThread to check for deadlock
      // - briefly wait for it to tell us it is still okay
      // - if we time out, then we want to undo this suspend
      //
      MutexLockerEx scl(SuspendChecker_lock, Mutex::_no_safepoint_check_flag);
      // If the SuspendCheckerThread is not ready for work yet or has
      // exited during VM shutdown, then we skip checking this suspend
      // operation.
      if (SuspendCheckerThread::suspend_checker_thread() == NULL) break;
      SuspendChecker_lock->notify();
      bool timedOut = SuspendChecker_lock->wait(true, 1000);
      if (!timedOut) break;

      //
      // The SuspendCheckerThread is stuck. The thread we just suspended
      // may be holding some critical resource which means the VMThread
      // will deadlock if we leave the thread suspended. It is also
      // possible that some other running thread temporarily has the
      // resource and for some reason has held it longer than our timeout
      // above. The latter is the false positive case and it shouldn't
      // hurt to temporarily resume the thread. The temporary resume
      // is done without clearing the _external_suspend flag.
      //
      { MutexLockerEx ml(target->SR_lock(), Mutex::_no_safepoint_check_flag);
        target->do_vm_resume(true);  // yield
        target->clear_suspended_in_native();
      }
    }

    // It would be nice to have the following assertion in all the
    // time, but it is possible for a racing resume request to have
    // resumed this thread right after we suspended it. Temporarily
    // enable this assertion if you are chasing a different kind of
    // bug.
    //
    // assert(java_lang_Thread::thread(target->threadObj()) == NULL ||
    //   target->is_being_ext_suspended(), "thread is not suspended");
  }
}


#ifndef CORE

void VM_Deoptimize::doit() {
  // We do not want any GCs to happen while we are in the middle of this VM operation
  ResourceMark rm;
  DeoptimizationMarker dm;

  // Deoptimize all activations depending on marked nmethods  
  Deoptimization::deoptimize_dependents();

  // Make the dependent methods zombies
  CodeCache::make_marked_nmethods_zombies();
}


VM_DeoptimizeFrame::VM_DeoptimizeFrame(JavaThread* thread, intptr_t* id) {
  _thread = thread;
  _id     = id;
}


void VM_DeoptimizeFrame::doit() {
  Deoptimization::deoptimize_frame(_thread, _id);
}


#ifndef PRODUCT

void VM_DeoptimizeAll::doit() {
  DeoptimizationMarker dm;
  // deoptimize all java threads in the system
  for (JavaThread* thread = Threads::first(); thread != NULL; thread = thread->next()) {
    if (thread->has_last_Java_frame()) {
      thread->deoptimize();
    }
  }
}


void VM_ZombieAll::doit() {
  JavaThread *thread = (JavaThread *)calling_thread();
  assert(thread->is_Java_thread(), "must be a Java thread");
  thread->make_zombies();
}

#endif // !PRODUCT
#endif // !CORE


#ifndef PRODUCT
void VM_Verify::doit() {
  Universe::verify();
}
#endif // !PRODUCT


void VM_PrintThreads::doit() {
  Threads::print(true, false);
}

void VM_FindDeadlocks::doit() {
  Threads::find_deadlocks();
}

VM_GC_HeapInspection::VM_GC_HeapInspection() :
  VM_GC_Operation(Universe::heap()->total_collections()) {
}

void VM_GC_HeapInspection::doit() {
  if (Universe::heap()->kind() == CollectedHeap::GenCollectedHeap) {
    HandleMark hm;
    GenCollectedHeap* gch = GenCollectedHeap::heap();
    gch->do_full_collection(false /* clear_all_soft_refs */,
                            gch->n_gens() - 1,
                            _notify_ref_lock);
    HeapInspection::heap_inspection();
  }
}

volatile bool VM_Exit::_vm_exited = false;
Thread * VM_Exit::_shutdown_thread = NULL;

int VM_Exit::set_vm_exited() {
  Thread * thr_cur = ThreadLocalStorage::get_thread_slow();

  assert(SafepointSynchronize::is_at_safepoint(), "must be at safepoint already");

  int num_active = 0;

  _shutdown_thread = thr_cur;
  _vm_exited = true;                                // global flag
  for(JavaThread *thr = Threads::first(); thr != NULL; thr = thr->next())
    if (thr!=thr_cur && thr->thread_state() == _thread_in_native) {
      ++num_active;
      thr->set_terminated(JavaThread::_vm_exited);  // per-thread flag
    }

  return num_active;
}

int VM_Exit::wait_for_threads_in_native_to_block(int timeout) {
  Thread * thr_cur = ThreadLocalStorage::get_thread_slow();
  // We don't need an accurate timer
  int max_attempts = timeout/10;

  assert(SafepointSynchronize::is_at_safepoint(), "must be at safepoint already");

  int attempts = 0;
  while (true) {
    int num_active = 0;
    for(JavaThread *thr = Threads::first(); thr != NULL; thr = thr->next()) {
      if (thr!=thr_cur && thr->thread_state() == _thread_in_native)
        ++num_active;
    }

    if (num_active == 0) return 0;
    if (attempts++ >= max_attempts) return num_active;

    // this will trigger assertion failure on Windows if used by Java thread
    os::sleep(thr_cur, 10, false);
  }
}

void VM_Exit::doit() {
  // Stop compiler threads
  NOT_CORE(CompileBroker::wait_for_compile_threads_to_block());

  if (_exit_vm) {
    // Wait for a short period for threads in native to block. Any thread
    // still executing native code after the wait will be stopped at
    // native==>Java/VM barriers.
    // Among 16276 JCK tests, 94% of them come here without any threads still
    // running in native; the other 6% are quiescent within 250ms (Ultra 80).
    wait_for_threads_in_native_to_block(300);

    set_vm_exited();

    // cleanup globals resources before exiting. exit_globals() currently
    // cleans up outputStream resources and PerfMemory resources.
    exit_globals();

    // Check for exit hook
    exit_hook_t exit_hook = Arguments::exit_hook();
    if (exit_hook != NULL) {
      exit_hook(_exit_code);
    } else {
      ::exit(_exit_code);
    }
  }
}


void VM_Exit::wait_if_vm_exited() {
  if (_vm_exited && 
      ThreadLocalStorage::get_thread_slow() != _shutdown_thread) {
    // _vm_exited is set at safepoint, and the Threads_lock is never released
    // we will block here until the process dies
    Threads_lock->lock_without_safepoint_check();
    ShouldNotReachHere();
  }
}

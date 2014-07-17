#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)objectMonitor_win32.cpp	1.47 03/05/09 11:31:40 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_objectMonitor_win32.cpp.incl"

class ObjectWaiter : public StackObj {
 public:
  ObjectWaiter* _next;
  Thread*       _thread;
  HANDLE        _event;
  intptr_t      _notified;
 public:
  ObjectWaiter(Thread* thread, HANDLE event) {
    _next     = NULL;
    _thread   = thread;
    _notified = 0;
    _event    = event;
  }
};


ObjectWaiter* ObjectMonitor::first_waiter() {
  return (ObjectWaiter*)_queue;
}

ObjectWaiter* ObjectMonitor::next_waiter(ObjectWaiter* o) {
  return o->_next;
}

Thread* ObjectMonitor::thread_of_waiter(ObjectWaiter* o) {
  return o->_thread;
}

// initialize the monitor, exception the semaphore, all other fields
// are simple integers or pointers
ObjectMonitor::ObjectMonitor() {
  _header     = NULL;
  _count      = 0;
  _waiters    = 0,
  _recursions = 0;
  _object     = NULL;
  _owner      = NULL;
  _queue      = NULL;
  _semaphore  = ::CreateSemaphore(0, 0, 1, 0);
  assert(_semaphore != NULL, "CreateSemaphore() failed!");
}

ObjectMonitor::~ObjectMonitor() {
  ::CloseHandle(_semaphore);
}

inline void ObjectMonitor::enqueue(void* argwaiter) {
  ObjectWaiter* waiter = (ObjectWaiter*)argwaiter;
  waiter->_next = NULL;
  // put waiter to the end of queue (single linked list)
  if (_queue == NULL) {
    _queue = waiter;
  } else {
    ObjectWaiter* iterator = (ObjectWaiter*)_queue;
    while (iterator->_next) {
      iterator = iterator->_next;
    }
    iterator->_next = waiter;
  }
}

inline ObjectWaiter* ObjectMonitor::dequeue() {
  // dequeue the very first waiter
  ObjectWaiter* waiter = (ObjectWaiter*)_queue;
  if (waiter) {
    _queue = waiter->_next;
    waiter->_next = NULL;
  }
  return waiter;
}

inline void ObjectMonitor::dequeue2(void* argwaiter) {
  ObjectWaiter* waiter = (ObjectWaiter*) argwaiter;
  // when the waiter has waked up because of interrupt,
  // timeout or other suprious wake-up, dequeue the 
  // waiter from waiting list (use linear search)
  if (_queue == waiter) {
    _queue = waiter->_next;
    waiter->_next = NULL;
  } else {
    ObjectWaiter* iterator = (ObjectWaiter*) _queue;
    while (iterator->_next != waiter) {
      // the waiter must be in the queue, iterator could not be null
      assert(iterator->_next != NULL, "ObjectMonitor::dequeue2");
      iterator = iterator->_next;
    }
    iterator->_next = waiter->_next;
    waiter->_next = NULL;
  }
}

void ObjectMonitor::enter(TRAPS) {
  if (atomic::compare_and_exchange_ptr(1, (intptr_t *)&_count, 0) == 0) {
    // fast path, the monitor is not owned yet
    atomic::membar();
    _owner = THREAD;
  } else if (_owner == THREAD) {
    // normal recursion case
    _recursions++;
  } else if (THREAD->is_lock_owned((address) _owner)) {
    assert(_recursions == 0, "check");
    _owner = THREAD;
    _recursions = 1;
  } else {
    if (UseSpinning) {
      for(int j = 0; j < PreBlockSpin; j++) {
        if (PreSpinYield) {
          os::yield();
        }
        // Spinning: retry the fast path again
        for (int i = 0; i < ReadSpinIterations; i++) {
          if (_count == 0) {
            if (atomic::compare_and_exchange_ptr(1, (intptr_t *)&_count, 0) == 0) {
              // fast path, the monitor is not owned yet
              atomic::membar();
              _owner = THREAD;
              return;
            }
          }
        }
        if (PostSpinYield) {
          os::yield();  
        }
      }
    }

    // increment the lock count
    if (atomic::add_ptr(1, (intptr_t *)&_count) > 1) {
      // the monitor is locked by some other thread
      assert(THREAD->is_Java_thread(), "Must be Java thread!");
      {
        if (jvmpi::is_event_enabled(JVMPI_EVENT_MONITOR_CONTENDED_ENTER)) {
          jvmpi::post_monitor_contended_enter_event(object());
        } 
        OSThreadContendState osts(THREAD->osthread());
        JavaThread *jt = (JavaThread *)THREAD;
        StateSaver sv(jt);
        ThreadBlockInVM tbivm(jt);

        bool threadIsSuspended;
        THREAD->set_current_pending_monitor(this);
        do {
          jt->set_suspend_equivalent();
          // cleared by handle_special_suspend_equivalent_condition() or java_suspend_self()
          DWORD ret = ::WaitForSingleObject(_semaphore, INFINITE);
          assert(ret == WAIT_OBJECT_0, "WaitForSingleObject() failed!");

          //
          // We have been granted the contended monitor, but while we were
          // waiting another thread externally suspended us. We don't want
          // to enter the monitor while suspended because that would surprise
          // the thread that suspended us.
          //
          threadIsSuspended = jt->handle_special_suspend_equivalent_condition();
          if (threadIsSuspended) {
            BOOL ret = ::ReleaseSemaphore(_semaphore, 1, NULL);
            assert(ret, "ReleaseSemaphore() failed!");
            // StateSaver is constructed above
            jt->java_suspend_self();
          }
        } while (threadIsSuspended);
        THREAD->set_current_pending_monitor(NULL);
      }

      if (jvmpi::is_event_enabled(JVMPI_EVENT_MONITOR_CONTENDED_ENTERED)) {
        jvmpi::post_monitor_contended_entered_event(object());
      }
    }
    // there should be no ownership
    assert(_owner == NULL, "Logic error with monitor owner!");
    assert(_recursions == 0, "Logic error with monitor recursions!");
    // use membar to enforce write to write order
    atomic::membar();
    _owner = THREAD;
  }
}

void ObjectMonitor::exit(TRAPS) {
  if (THREAD != _owner) {
    if (THREAD->is_lock_owned((address) _owner)) {
      _owner = THREAD;  // regain ownership of inflated monitor
    } else {
      // NOTE: we need to handle unbalanced monitor enter/exit
      // in native code by throwing an exception.
      assert(false, "Non-balanced monitor enter/exit!");
      return;
    }
  }
  if (_recursions == 0) {
    _owner = NULL;        // clear the owner ship
    atomic::membar();     // memory barrier to prevent out-of-order write
    // atomic decrement count, if it still more than zero
    // there must be someone else blocking on monitor enter
    if (atomic::add_ptr(-1, (intptr_t *)&_count) > 0) {
      BOOL ret = ::ReleaseSemaphore(_semaphore, 1, NULL);
      assert(ret, "ReleaseSemaphore() failed!");
      if (jvmpi::is_event_enabled(JVMPI_EVENT_MONITOR_CONTENDED_EXIT)) {
        jvmpi::post_monitor_contended_exit_event(object());
      }
    }
  } else {
    _recursions--;        // this is simple recursive enter
  }
}

void ObjectMonitor::wait(jlong millis, bool interruptable, TRAPS) {
  assert(THREAD->is_Java_thread(), "Must be Java thread!");
  JavaThread *jt = (JavaThread *)THREAD;

  check(CHECK);
  // check interrupt event
  if (interruptable && Thread::is_interrupted(THREAD, true) && !HAS_PENDING_EXCEPTION) {
    THROW(vmSymbols::java_lang_InterruptedException());
  }

  jt->set_current_waiting_monitor(this);

  // create a node to be put into the queue
  ObjectWaiter node(THREAD, THREAD->osthread()->interrupt_event());
  // enter the waiting queue, which is single linked list in this case
  // how it could be a priority queue, or any data structure
  enqueue(&node);
  if (jvmpi::is_event_enabled(JVMPI_EVENT_MONITOR_WAIT)) {
    jvmpi::post_monitor_wait_event((oop)object(), millis);
  }
  DWORD start_wait;
  if (jvmpi::is_event_enabled(JVMPI_EVENT_MONITOR_WAITED)) {
    start_wait = GetTickCount();
  }
  int save = _recursions; // record the old recursion count
  _waiters++;             // increment the number of waiters
  _recursions = 0;        // set the recursion level to be 1
  exit(THREAD);           // exit the monitor
  {
    // external suspend retry for this ThreadBlockInVM is in raw_enter()
    StateSaver sv(jt);
    ThreadBlockInVM tbivm(jt);
    OSThreadWaitState osts(THREAD->osthread(), true /* is Object.wait() */);
    jt->set_suspend_equivalent();
    // cleared by handle_special_suspend_equivalent_condition() or java_suspend_self()
    EventWait(node._event, millis);

    // were we externally suspended while we were waiting?
    if (jt->handle_special_suspend_equivalent_condition()) {
      //
      // If we were suspended while we were waiting in EventWait() above,
      // then we don't want to re-enter the monitor while suspended
      // because that would surprise the thread that suspended us.
      // The raw_enter() call below handles the contended monitor
      // case. However, if the thread that notified us has already
      // released the monitor, then we may complete the raw_enter()
      // without contention and without self-suspending.
      //
      // StateSaver is contructed above.
      jt->java_suspend_self();
    }

    // retain OBJECT_WAIT state until re-enter successfully completes
    raw_enter(THREAD, false);          // re-enter the monitor
  }

  jt->set_current_waiting_monitor(NULL);
 
  if (jvmpi::is_event_enabled(JVMPI_EVENT_MONITOR_WAITED)) {
    DWORD end_wait = GetTickCount();
    // the following code is to handle the GetTickCount()
    // wrap as documented by MSDN
    long millis_wait = (end_wait >= start_wait) ? (end_wait - start_wait) :
      (ULONG_MAX - start_wait + end_wait);
    jvmpi::post_monitor_waited_event((oop)object(), millis_wait);
  }
  _recursions = save;     // restore the old recursion count
  _waiters--;             // decrement the number of waiters

  // NOTE: this line must be here, because a notification
  // may be right after the timeout, but before re-enter
  // so the event may still be in signed state
  BOOL ret = ::ResetEvent(node._event);
  assert(ret, "ResetEvent() failed!");
  // check if the notification happened
  if (node._notified == 0) {
    // no, it could be timeout or Thread.interrupt() or both
    // dequeue the the node from the single linked list
    dequeue2(&node);
    // check for interrupt event, otherwise it is timeout
    if (interruptable && Thread::is_interrupted(THREAD, true) && !HAS_PENDING_EXCEPTION) {
      THROW(vmSymbols::java_lang_InterruptedException());
    }
  }
  // NOTES: Suprious wake up will be consider as timeout.
  // Monitor notify has precedence over thread interrupt.
}

void ObjectMonitor::notify(TRAPS) {
  check(CHECK);
  ObjectWaiter* iterator;
  if ((iterator = dequeue()) != 0) {
    iterator->_notified = 1;
    BOOL ret = ::SetEvent(iterator->_event);
    assert(ret, "SetEvent() failed!");
  }
}

void ObjectMonitor::notifyAll(TRAPS) {
  check(CHECK);
  ObjectWaiter* iterator;
  while ((iterator = dequeue()) != 0) {
    iterator->_notified = 1;
    BOOL ret = ::SetEvent(iterator->_event);
    assert(ret, "SetEvent() failed!");
  }
}

// used for JVMDI/JVMPI raw monitor implementation
// Also used by ObjectMonitor::wait().
int ObjectMonitor::raw_enter(TRAPS, bool isRawMonitor) {
  bool uncontended;

  // don't enter raw monitor if thread is being externally suspended, it will
  // surprise the suspender if a "suspended" thread can still enter monitor
  if (THREAD->is_Java_thread()) {
    JavaThread * jt = (JavaThread *)THREAD;
    jt->SR_lock()->lock_without_safepoint_check();
    while (jt->is_external_suspend()) {
      jt->SR_lock()->unlock();

      if (jt->frame_anchor()->walkable()) {
        // StateSaver already done by caller
        jt->java_suspend_self();
      } else {
        StateSaver sv((JavaThread*)THREAD);
        jt->java_suspend_self();
      }

      jt->SR_lock()->lock_without_safepoint_check();
    }

    // guarded by SR_lock to avoid racing with new external suspend requests.
    uncontended = (atomic::compare_and_exchange_ptr(1, (intptr_t *)&_count, 0) == 0);

    jt->SR_lock()->unlock();
  } else {
    uncontended = (atomic::compare_and_exchange_ptr(1, (intptr_t *)&_count, 0) == 0);
  }

  if (uncontended) {
    atomic::membar();
    _owner = THREAD;
  } else if (_owner == THREAD) {
    _recursions++;
  } else {
    if (isRawMonitor) {
      if (jvmpi::is_event_enabled(JVMPI_EVENT_RAW_MONITOR_CONTENDED_ENTER)) {
        jvmpi::post_raw_monitor_contended_enter_event((RawMonitor *)this);
      }
    }
    if (atomic::add_ptr(1, (intptr_t *)&_count) > 1) {
      bool threadIsSuspended;
      THREAD->set_current_pending_monitor(this);
      if (!THREAD->is_Java_thread()) {
        // No other non-Java threads besides VM thread should acquire
        // a raw monitor.
        assert(THREAD->is_VM_thread(), "must be VM thread");
        DWORD ret = ::WaitForSingleObject(_semaphore, INFINITE);
        assert(ret == WAIT_OBJECT_0, "WaitForSingleObject() failed!");
      } else {
        JavaThread * jt = (JavaThread *)THREAD;
        do {
          jt->set_suspend_equivalent();
          // cleared by handle_special_suspend_equivalent_condition() or
          // java_suspend_self()
          DWORD ret = ::WaitForSingleObject(_semaphore, INFINITE);
          assert(ret == WAIT_OBJECT_0, "WaitForSingleObject() failed!");

          // were we externally suspended while we were waiting?
          threadIsSuspended = jt->handle_special_suspend_equivalent_condition();
          if (threadIsSuspended) {
            //
            // This logic isn't needed for JVM/DI or JVM/PI raw monitors,
            // but doesn't hurt just in case the suspend rules change. This
            // logic is needed for the ObjectMonitor.wait() reentry phase.
            // We have reentered the contended monitor, but while we were
            // waiting another thread suspended us. We don't want to reenter
            // the monitor while suspended because that would surprise the
            // thread that suspended us.
            //
            BOOL ret = ::ReleaseSemaphore(_semaphore, 1, NULL);
            assert(ret, "ReleaseSemaphore() failed!");
            if (jt->frame_anchor()->walkable()) {
              // StateSaver already done by caller
              jt->java_suspend_self();
            } else {
              StateSaver sv(jt);
              jt->java_suspend_self();
            }
          }
        } while (threadIsSuspended);
      }
      THREAD->set_current_pending_monitor(NULL);
    }
    // there should be no ownership
    assert(_owner == NULL, "Fatal error with monitor owner!");
    assert(_recursions == 0, "Fatal error with monitor recursions!");
    // prevent out of order write
    atomic::membar();
    _owner = THREAD;
    if (isRawMonitor) {
      if (jvmpi::is_event_enabled(JVMPI_EVENT_RAW_MONITOR_CONTENDED_ENTERED)) {
        jvmpi::post_raw_monitor_contended_entered_event((RawMonitor *)this);
      }
    }
  }
  return OM_OK;
}

// Used for JVMDI, JVMPI raw monitor implementation.
int ObjectMonitor::raw_exit(TRAPS, bool isRawMonitor) {
  if (THREAD != _owner) {
    return OM_ILLEGAL_MONITOR_STATE;
  }
  if (_recursions == 0) {
    if (isRawMonitor) {
      ((RawMonitor*)this)->remove_from_locked_list();
    }
    _owner = NULL;      // clear the owner ship
    atomic::membar();   // memory barrier to prevent out-of-order write
    // atomic decrement count, if it still more than zero
    // there must be someone else blocking on monitor enter
    if (atomic::add_ptr(-1, (intptr_t *)&_count) > 0) {
      BOOL ret = ::ReleaseSemaphore(_semaphore, 1, NULL);
      assert(ret, "ReleaseSemaphore() failed!");
      if (isRawMonitor) {
        if (jvmpi::is_event_enabled(JVMPI_EVENT_RAW_MONITOR_CONTENDED_EXIT)) {
          jvmpi::post_raw_monitor_contended_exit_event((RawMonitor *)this);
        }
      }
    }
  } else {
    _recursions--; // this is simple recursive enter
  }
  return OM_OK;
}

// Used for JVMDI, JVMPI raw monitor implementation.
// assumes that the current thread is in native state
int ObjectMonitor::raw_wait(jlong millis, bool interruptable, TRAPS) {
  if (THREAD != _owner) {
    return OM_ILLEGAL_MONITOR_STATE;
  }
  // check interrupt event
  if (interruptable && Thread::is_interrupted(THREAD, true)) {
    return OM_INTERRUPTED;
  }
  // create a node to be put into the queue
  ObjectWaiter node(THREAD, THREAD->osthread()->interrupt_event());
  // enter the waiting queue, which is single linked list in this case
  // how it could be a priority queue, or any data structure
  enqueue(&node);

  int save = _recursions; // record the old recursion count
  _waiters++;             // increment the number of waiters
  _recursions = 0;        // set the recursion level to be 1
  {
    // On this platform, ObjectMonitor has no parts with an owner
    // so it does not need a SuspendCritical helper object to be
    // created.
    raw_exit(THREAD, false);// exit the monitor

    EventWait(node._event, millis);
  }

  // were we externally suspended while we were waiting?
  bool threadIsSuspended =
    ((JavaThread *)THREAD)->is_external_suspend_with_lock();
  if (threadIsSuspended) {
    //
    // If we were suspended while we were waiting in EventWait() above,
    // then we don't want to re-enter the monitor while suspended
    // because that would surprise the thread that suspended us.
    // The raw_enter() call below handles the contended monitor
    // case. However, if the thread that notified us has already
    // released the monitor, then we may complete the raw_enter()
    // without contention and without self-suspending.
    //
    StateSaver sv((JavaThread*)THREAD);
    ((JavaThread *)THREAD)->java_suspend_self();
  }

  raw_enter(THREAD, false);      // re-enter the monitor
  _recursions = save;     // restore the old recursion count
  _waiters--;             // decrement the number of waiters

  // NOTE: this line must be here, because a notification
  // may be right after the timeout, but before re-enter
  // so the event may still be in signed state
  BOOL ret = ::ResetEvent(node._event);
  assert(ret, "ResetEvent() failed!");
  // check if the notification happened
  if (node._notified == 0) {
    // no, it could be timeout or Thread.interrupt() or both
    // dequeue the the node from the single linked list
    dequeue2(&node);
    // check for interrupt event, otherwise it is timeout
    if (interruptable && Thread::is_interrupted(THREAD, true)) {
      return OM_INTERRUPTED;
    }
  }
  // NOTES: Suprious wake up will be consider as timeout.
  // Monitor notify has precedence over thread interrupt.
  return OM_OK;
}

int ObjectMonitor::raw_notify(TRAPS) {
  if (THREAD != _owner) {
    return OM_ILLEGAL_MONITOR_STATE;
  }
  ObjectWaiter* iterator;
  if ((iterator = dequeue()) != 0) {
    iterator->_notified = 1;
    BOOL ret = ::SetEvent(iterator->_event);
    assert(ret, "SetEvent() failed!");
  }
  return OM_OK;
}

int ObjectMonitor::raw_notifyAll(TRAPS) {
  if (THREAD != _owner) {
    return OM_ILLEGAL_MONITOR_STATE;
  }
  ObjectWaiter* iterator;
  while ((iterator = dequeue()) != 0) {
    iterator->_notified = 1;
    BOOL ret = ::SetEvent(iterator->_event);
    assert(ret, "SetEvent() failed!");
  }
  return OM_OK;
}

// %%% should move this to objectMonitor.cpp
void ObjectMonitor::check_slow(TRAPS) {
  // called only from check()
  assert(THREAD != _owner && !THREAD->is_lock_owned((address) _owner), "must not be owner");
  THROW_MSG(vmSymbols::java_lang_IllegalMonitorStateException(), "current thread not owner");
}

#ifndef PRODUCT
void ObjectMonitor::verify() {
}

void ObjectMonitor::print() {
}
#endif

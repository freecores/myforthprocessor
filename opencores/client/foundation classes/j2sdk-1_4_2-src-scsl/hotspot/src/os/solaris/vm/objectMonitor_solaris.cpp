#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)objectMonitor_solaris.cpp	1.40 03/05/09 11:45:18 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_objectMonitor_solaris.cpp.incl"

// NOTE:
//   This file is 99% based on win32 heavy weight monitor.
// The only significant change is related to owner transfer, because
// the underlying OS mutex has an owner. The Win32 implementation uses
// semaphores, which don't have an owner. In addition, the waiters
// queue has been modified to use a doubly linked list for efficiency.

typedef os::Solaris::Event* event_t;

// A macro is used below because there may already be a pending
// exception which should not abort the execution of the routines
// which use this (which is why we don't put this into check_slow and
// call it with a CHECK argument).
#define CHECK_OWNER() \
  do { \
    if (THREAD != _owner) { \
      if (THREAD->is_lock_owned((address) _owner)) { \
        enter(THREAD); /* regain ownership of inflated monitor */ \
        _recursions = 0; \
      } else { \
        THROW(vmSymbols::java_lang_IllegalMonitorStateException()); \
      } \
    } \
  } while (false)

class ObjectWaiter : public StackObj {
public:
  ObjectWaiter* _next;
  ObjectWaiter* _prev;
  Thread*       _thread;
  intptr_t      _notified;
  event_t       _event;
public:
  ObjectWaiter(Thread* thread, event_t event) {
    _next     = NULL;
    _prev     = NULL;
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
}

ObjectMonitor::~ObjectMonitor() {
}

intptr_t ObjectMonitor::is_busy() const {
  return (_count|_waiters|(intptr_t)_owner);
}

inline void ObjectMonitor::enqueue(ObjectWaiter* node) {
  assert(node != NULL, "should not dequeue NULL node");
  assert(node->_prev == NULL, "node already in list");
  assert(node->_next == NULL, "node already in list");
  // put node at end of queue (circular doubly linked list)
  if (_queue == NULL) {
    _queue = node;
    node->_prev = node;
    node->_next = node;
  } else {
    ObjectWaiter* head = (ObjectWaiter*)_queue;
    ObjectWaiter* tail = head->_prev;
    assert(tail->_next == head, "invariant check");
    tail->_next = node;
    head->_prev = node;
    node->_next = head;
    node->_prev = tail;
  }
}

inline ObjectWaiter* ObjectMonitor::dequeue() {
  // dequeue the very first waiter
  ObjectWaiter* waiter = (ObjectWaiter*)_queue;
  if (waiter) {
    dequeue2(waiter);
  }
  return waiter;
}

inline void ObjectMonitor::dequeue2(ObjectWaiter* node) {
  assert(node != NULL, "should not dequeue NULL node");
  assert(node->_prev != NULL, "node already removed from list");
  assert(node->_next != NULL, "node already removed from list");
  // when the waiter has woken up because of interrupt,
  // timeout or other suprious wake-up, dequeue the 
  // waiter from waiting list
  ObjectWaiter* next = node->_next;
  if (next == node) {
    assert(node->_prev == node, "invariant check");
    _queue = NULL;
  } else {
    ObjectWaiter* prev = node->_prev;
    assert(prev->_next == node, "invariant check");
    assert(next->_prev == node, "invariant check");
    next->_prev = prev;
    prev->_next = next;
    if (_queue == node) {
      _queue = next;
    }
  }
  node->_next = NULL;
  node->_prev = NULL;
}

void ObjectMonitor::enter2(TRAPS) {
  if (UseSpinning) {
    bool status;
    for(int j = 0; j < PreBlockSpin; j++) {
      if (PreSpinYield) {
        os::yield();
      }
      // Spinning: retry the fast path again
      for (int i = 0; i < ReadSpinIterations; i++) {
        if (_count == 0) {
          status = _mutex.trylock();
          if (status && (_owner == NULL)) {
            _owner = THREAD;
            return;
          } else if (status) {
            // release the mutex since we don't own the monitor.
            _mutex.unlock();
          }
        }
      }
      if (PostSpinYield) {
        os::yield();  
      }
    }
  }
  
  // HACK: do not use AtomicPtrCounter because of gcc bug.
  atomic::increment_ptr((intptr_t *)&this->_count);
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
      _mutex.lock();
    
      if (_owner != NULL) {
        // We were granted ownership of the mutex. However, the
        // ObjectMonitor is still owned by a thread that entered
        // it while it was lightweight.

        // Regaining ownership should be done by enter(), not here.
        assert(!THREAD->is_lock_owned((address)_owner), "fatal logic error");

        // COMMENT: this is the only difference from win32 impl.
        // Wait here until the owner exits the ObjectMonitor.
        while (_owner != NULL) {
          ObjectWaiter node(THREAD, THREAD->osthread()->interrupt_event());
          enqueue(&node);
          _mutex.unlock();
          node._event->down();
          _mutex.lock();
          if (node._notified == 0) {
            dequeue2(&node);
          }
        }
      }

      // were we externally suspended while we were waiting?
      threadIsSuspended = jt->handle_special_suspend_equivalent_condition();
      if (threadIsSuspended) {
        //
        // We have been granted the contended monitor, but while we were
        // waiting another thread suspended us. We don't want to enter
        // the monitor while suspended because that would surprise the
        // thread that suspended us.
        //
        _mutex.unlock();
        // StateSaver is constructed above
        jt->java_suspend_self();
      }
    } while (threadIsSuspended);
    THREAD->set_current_pending_monitor(NULL);

    _owner = THREAD;
    if (jvmpi::is_event_enabled(JVMPI_EVENT_MONITOR_CONTENDED_ENTERED)) {
      jvmpi::post_monitor_contended_entered_event(object());
    }
  }
  atomic::decrement_ptr((intptr_t *)&this->_count);

}

void ObjectMonitor::enter(TRAPS) {
  if (THREAD != _owner) {
    bool status = _mutex.trylock();
    if (status && (_owner == NULL)) {
      _owner = THREAD;
    } else if (THREAD->is_lock_owned((address)_owner)) {
      // NOTE: be careful since we can not take safepoint while we are
      // regaining the monitor ownership during monitor exit.
      assert(_recursions == 0, "internal state error");
      if (!status) {
        _mutex.lock();
        assert(THREAD->is_lock_owned((address)_owner), "owner should not change");
      }
      _owner = THREAD;
      _recursions = 1;
      int res = raw_notifyAll(THREAD);
      assert(res == OM_OK, "notifyAll must succeed");
      assert(_queue == NULL, "queue must be null");
    } else {
      // this is the real contended case.
      if (status) {
        // release the mutex since we don't own the monitor.
        _mutex.unlock();
      }
      enter2(THREAD);
    }
  } else {
    _recursions++;
  }
}

void ObjectMonitor::exit(TRAPS) {
  if (THREAD != _owner) {
    if (THREAD->is_lock_owned((address) _owner)) {
      enter(THREAD);    // regain ownership of inflated monitor
      _recursions = 0;  
    } else {
      // NOTE: we need to handle unbalanced monitor enter/exit
      // in native code by throwing an exception.
      assert(false, "Non-balanced monitor enter/exit!");
      return;
    }
  }
  debug_only(_mutex.verify_locked();)
  if (_recursions == 0) {
   intptr_t savedcount; 
    _owner = NULL;        // clear the owner ship
    atomic::membar();     // memory barrier to prevent out-of-order write
    savedcount = _count;
    _mutex.unlock();

    // Exit is contended only if count had monitor enter requestors
    if (savedcount > 0) {
      if (jvmpi::is_event_enabled(JVMPI_EVENT_MONITOR_CONTENDED_EXIT)) {
        jvmpi::post_monitor_contended_exit_event(object());
      }
    }
  } else {
    _recursions--;        // this is simple recursive enter
  }
}

void ObjectMonitor::wait(jlong millis, bool interruptible, TRAPS) {
  assert(THREAD->is_Java_thread(), "Must be Java thread!");
  JavaThread *jt = (JavaThread *)THREAD;

  CHECK_OWNER();
  // check interrupt event
  if (interruptible && Thread::is_interrupted(THREAD, true) && !HAS_PENDING_EXCEPTION) {
    THROW(vmSymbols::java_lang_InterruptedException());
  }

  jt->set_current_waiting_monitor(this);

  // create a node to be put into the queue
  ObjectWaiter node(THREAD, THREAD->osthread()->interrupt_event());
  // enter the waiting queue, which is a circular doubly linked list in this case
  // how it could be a priority queue, or any data structure
  enqueue(&node);
  if (jvmpi::is_event_enabled(JVMPI_EVENT_MONITOR_WAIT)) {
    jvmpi::post_monitor_wait_event((oop)object(), millis);
  }
  hrtime_t start_wait;
  if (jvmpi::is_event_enabled(JVMPI_EVENT_MONITOR_WAITED)) {
    start_wait = gethrtime();
  }

  int save = _recursions; // record the old recursion count
  _waiters++;             // increment the number of waiters
  _recursions = 0;        // set the recursion level to be 1
  int result = raw_exit(THREAD, false);     // exit the monitor
  assert(result == OM_OK, "raw_exit should have succeeded");
  {
    StateSaver sv(jt);
    // external suspend retry for this ThreadBlockInVM is in raw_enter()
    ThreadBlockInVM tbivm(jt);
    OSThread* osthread = THREAD->osthread();
    OSThreadWaitState osts(osthread, true /* is Object.wait() */);
    jt->set_suspend_equivalent();
    // cleared by handle_special_suspend_equivalent_condition() or java_suspend_self()
    if (millis <= 0) {
      node._event->down();
    } else {
      node._event->down(millis);
    }

    // were we externally suspended while we were waiting?
    if (jt->handle_special_suspend_equivalent_condition()) {
      //
      // If we were suspended while we were waiting in down() above,
      // then we don't want to re-enter the monitor while suspended
      // because that would surprise the thread that suspended us.
      // The raw_enter() call below handles the contended monitor
      // case. However, if the thread that notified us has already
      // released the monitor, then we may complete the raw_enter()
      // without contention and without self-suspending.
      //
      // StateSaver is constructed above.
      jt->java_suspend_self();
    }

    // retain OBJECT_WAIT state until re-enter successfully completes
    result = raw_enter(THREAD, false);    // re-enter the monitor
    assert(result == OM_OK, "raw_enter should have succeeded");
  }

  jt->set_current_waiting_monitor(NULL);

  if (jvmpi::is_event_enabled(JVMPI_EVENT_MONITOR_WAITED)) {
    hrtime_t end_wait = gethrtime();
    jvmpi::post_monitor_waited_event((oop)object(), 
	(end_wait - start_wait)/1000000L);
  }
  _recursions = save;     // restore the old recursion count
  _waiters--;             // decrement the number of waiters

  // NOTE: this line must be here, because a notification
  // may be right after the timeout, but before re-enter
  // so the event may still be in signed state
  node._event->reset();

  // check if the notification happened
  if (node._notified == 0) {
    // no, it could be timeout or Thread.interrupt() or both
    // dequeue the node from the waiters queue
    dequeue2(&node);

    // check for interrupt event, otherwise it is timeout
    if (interruptible && Thread::is_interrupted(THREAD, true) && !HAS_PENDING_EXCEPTION) {
      THROW(vmSymbols::java_lang_InterruptedException());
    }
  }
  // NOTES: Suprious wake up will be consider as timeout.
  // Monitor notify has precedence over thread interrupt.
}

void ObjectMonitor::notify(TRAPS) {
  CHECK_OWNER();
  ObjectWaiter* iterator;
  if ((iterator = dequeue()) != NULL) {
    iterator->_notified = 1;
    iterator->_event->set();
  }
}

void ObjectMonitor::notifyAll(TRAPS) {
  CHECK_OWNER();
  ObjectWaiter* iterator;
  while ((iterator = dequeue()) != NULL) {
    iterator->_notified = 1;
    iterator->_event->set();
  }
}

// Used mainly for JVMDI, JVMPI raw monitor implementation
// Also used for ObjectMonitor::wait().
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
    uncontended = _mutex.trylock();

    jt->SR_lock()->unlock();
  } else {
    uncontended = _mutex.trylock();
  }

  if (uncontended) {
    // @@@: this membar seems unnecessary on Solaris and Linux
    atomic::membar();
    _owner = THREAD;
  } else if (_owner == THREAD) {
    _recursions++;
  } else {
    if (isRawMonitor) {
      // HACK: do not use AtomicPtrCounter because of gcc bug.
      atomic::increment_ptr((intptr_t *)&this->_count);
      if (jvmpi::is_event_enabled(JVMPI_EVENT_RAW_MONITOR_CONTENDED_ENTER)) {
        jvmpi::post_raw_monitor_contended_enter_event((RawMonitor *)this);
      }
    }

    bool threadIsSuspended;
    THREAD->set_current_pending_monitor(this);
    if (!THREAD->is_Java_thread()) {
      // No other non-Java threads besides VM thread would acquire 
      // a raw monitor.
      assert(THREAD->is_VM_thread(), "must be VM thread");
      _mutex.lock();
    } else {
      JavaThread * jt = (JavaThread *)THREAD;
      do {
        jt->set_suspend_equivalent();
        // cleared by handle_special_suspend_equivalent_condition() or
        // java_suspend_self()
        _mutex.lock();

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
          _mutex.unlock();
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

    // there should be no ownership
    assert(_owner == NULL, "Fatal error with monitor owner!");
    assert(_recursions == 0, "Fatal error with monitor recursions!");
    // prevent out of order write
    // @@@: this membar seems unnecessary on Solaris and Linux
    atomic::membar();
    _owner = THREAD;
    if (isRawMonitor) {
      if (jvmpi::is_event_enabled(JVMPI_EVENT_RAW_MONITOR_CONTENDED_ENTERED)) {
        jvmpi::post_raw_monitor_contended_entered_event((RawMonitor *)this);
      }
      atomic::decrement_ptr((intptr_t *)&this->_count);
    }
  }
  if (isRawMonitor) {
    if (_recursions == 0) {
      ((RawMonitor *)this)->add_to_locked_list();
    }
  }
  return OM_OK;
}

// Used mainly for JVMDI, JVMPI raw monitor implementation
// Also used for ObjectMonitor::wait().
int ObjectMonitor::raw_exit(TRAPS, bool isRawMonitor) {
  if (THREAD != _owner) {
    return OM_ILLEGAL_MONITOR_STATE;
  }
  if (_recursions == 0) {
    intptr_t savedcount;
    if (isRawMonitor) {
      ((RawMonitor *)this)->remove_from_locked_list();
    }
    _owner = NULL;      // clear the owner ship
    atomic::membar();   // memory barrier to prevent out-of-order write
    savedcount = _count;
    _mutex.unlock();
    if (isRawMonitor) {
      if (savedcount > 0) {
        if (jvmpi::is_event_enabled(JVMPI_EVENT_RAW_MONITOR_CONTENDED_EXIT)) {
          jvmpi::post_raw_monitor_contended_exit_event((RawMonitor*)this);
        }
      }
    }
  } else {
    _recursions--; // this is simple recursive exit
  }
  return OM_OK;
}

// Used for JVMDI, JVMPI raw monitor implementation.
// assumes that the current thread is in native state
int ObjectMonitor::raw_wait(jlong millis, bool interruptible, TRAPS) {
  if (THREAD != _owner) {
    return OM_ILLEGAL_MONITOR_STATE;
  }
  // check interrupt event
  if (interruptible && Thread::is_interrupted(THREAD, true)) {
    return OM_INTERRUPTED;
  }
  // create a node to be put into the queue
  ObjectWaiter node(THREAD, THREAD->osthread()->interrupt_event());
  // enter the waiting queue, which is a circular doubly linked list in this case
  // how it could be a priority queue, or any data structure
  enqueue(&node);

  int save = _recursions; // record the old recursion count
  _waiters++;             // increment the number of waiters
  _recursions = 0;        // set the recursion level to be 1
  {
    //
    // On this platform, ObjectMonitor has two parts with an owner:
    //
    // 1) the ObjectMonitor's _mutex field
    // 2) the underlying event mutex
    //
    // A JavaThread should not be suspended between the time that it
    // releases ownership of the ObjectMonitor mutex in the raw_exit()
    // call and releases ownership of the underlying event mutex in the
    // down() call. Fortunately, this non-cooperative suspension can
    // only occur when the thread is in state _thread_in_native. If we
    // figure out a way to stop doing non-cooperative suspension of
    // threads in state _thread_in_native, then we can get rid of this
    // mechanism.
    //
    // If a JavaThread is suspended after it releases ownership of the
    // ObjectMonitor mutex and while it still owns the underlying event
    // mutex, then deadlock can occur. Another thread can gain
    // ownership of the ObjectMonitor mutex part, but will be unable to
    // gain ownership of the underlying event mutex. If that thread
    // happens to be the one that is supposed to notify the JavaThread,
    // then deadlock!
    //
    // We used to clear the _native_suspend_critical value when we
    // returned from the down() call below. However, this allowed
    // is_ext_suspended_equivalent_with_lock() to return different
    // values depending on where we are in this code path. In the
    // down() call and after self-suspension is complete, true was
    // returned. Between those two points, false was returned. This
    // oscillation of values caused problems in code paths that made
    // more than one call to is_ext_suspended_equivalent_with_lock().
    //
    SuspendCritical(THREAD, Thread::_native_suspend_critical);

    raw_exit(THREAD, false);   // exit the monitor

    if (millis <= 0) {
      node._event->down();
    } else {
      node._event->down(millis);
    }

    // were we externally suspended while we were waiting?
    bool threadIsSuspended =
      ((JavaThread *)THREAD)->is_external_suspend_with_lock();
    if (threadIsSuspended) {
      //
      // If we were suspended while we were waiting in down() above,
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
  } // SuspendCritical deleted here

  raw_enter(THREAD, false);      // re-enter the monitor
  _recursions = save;     // restore the old recursion count
  _waiters--;             // decrement the number of waiters

  // NOTE: this line must be here, because a notification
  // may be right after the timeout, but before re-enter
  // so the event may still be in signed state
  node._event->reset();
  // check if the notification happened
  if (node._notified == 0) {
    // no, it could be timeout or Thread.interrupt() or both
    // dequeue the node from the waiters queue
    dequeue2(&node);

    // check for interrupt event, otherwise it is timeout
    if (interruptible && Thread::is_interrupted(THREAD, true)) {
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
    iterator->_event->set();
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
    iterator->_event->set();
  }
  return OM_OK;
}

// %%% should move this to objectMonitor.cpp
void ObjectMonitor::check_slow(TRAPS) {
  // called only from check() (NOTE: unused in this implementation)
  assert(THREAD != _owner && !THREAD->is_lock_owned((address) _owner), "must not be owner");
  THROW_MSG(vmSymbols::java_lang_IllegalMonitorStateException(), "current thread not owner");
}

#ifndef PRODUCT
void ObjectMonitor::verify() {
}

void ObjectMonitor::print() {
}
#endif

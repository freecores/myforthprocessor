#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)threadLocalStorage.cpp	1.35 03/01/23 12:26:17 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_threadLocalStorage.cpp.incl"

// static member initialization
int ThreadLocalStorage::_thread_index = -1;
Thread* ThreadLocalStorage::_get_thread_cache[ThreadLocalStorage::_pd_cache_size] = {NULL};

Thread* ThreadLocalStorage::get_thread_slow() {
  return (Thread*) os::thread_local_storage_at(ThreadLocalStorage::thread_index());
}
#ifndef PRODUCT
#define	_PCT(n,d)	((100.0*(double)(n))/(double)(d))

int ThreadLocalStorage::_tcacheHit = 0;
int ThreadLocalStorage::_tcacheMiss = 0;

void ThreadLocalStorage::print_statistics() {
  int total = _tcacheMiss+_tcacheHit;
  tty->print_cr("Thread cache hits %d misses %d total %d percent %f\n",
                _tcacheHit, _tcacheMiss, total, _PCT(_tcacheHit, total));
}
#undef _PCT
#endif // PRODUCT

Thread* ThreadLocalStorage::get_thread_via_cache_slowly(uintptr_t raw_id,
                                                        int index) {
  Thread *thread = get_thread_slow();
  if (thread != NULL) {
    address sp = pd_sp_address();
    guarantee(thread->_stack_base == NULL ||
              (sp <= thread->_stack_base && 
                 sp >= thread->_stack_base - thread->_stack_size) || 
               is_error_reported(),
              "sp must be inside of selected thread stack");

    thread->_self_raw_id = raw_id;  // mark for quick retrieval
    _get_thread_cache[ index ] = thread;
  }
  return thread;
}


static const double all_zero[ sizeof(Thread) / sizeof(double) + 1 ] = {0};
#define NO_CACHED_THREAD ((Thread*)all_zero)

void ThreadLocalStorage::set_thread(Thread* thread) {
  Thread* old_thread = get_thread_slow();
  if (old_thread != NULL) {
    // Remove it from the cache:
    for (int i = 0; i < _pd_cache_size; i++) {
      if (_get_thread_cache[i] == old_thread)
	_get_thread_cache[i] = NO_CACHED_THREAD;
    }
  }
  os::thread_local_storage_at_put(ThreadLocalStorage::thread_index(), thread);
  // The following ensure that any optimization tricks we have tried
  // did not backfire on us:
  guarantee(get_thread()      == thread, "must be the same thread, quickly");
  guarantee(get_thread_slow() == thread, "must be the same thread, slowly");
}

void ThreadLocalStorage::init() {
  assert(ThreadLocalStorage::thread_index() == -1, "More than one attempt to initialize threadLocalStorage");
  for (int i = 0; i < _pd_cache_size; i++) {
    _get_thread_cache[i] = NO_CACHED_THREAD;
  }
  set_thread_index(os::allocate_thread_local_storage());
  generate_code_for_get_thread();
}


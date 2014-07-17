#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)threadLocalStorage.hpp	1.35 03/01/23 12:26:19 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Interface for thread local storage

// Fast variant of ThreadLocalStorage::get_thread_slow
extern "C" Thread* get_thread();

class ThreadLocalStorage : AllStatic {
 public:
  static void    set_thread(Thread* thread);
  static Thread* get_thread_slow();

  // Machine dependent stuff
  #include "incls/_threadLS_pd.hpp.incl"

  // Accessor
  static inline int  thread_index() { return _thread_index; }
  static inline void set_thread_index(int index) { _thread_index = index; }

  // Initialization
  // Called explicitly from VMThread::activate_system instead of init_globals.
  static void init();

  static void print_statistics() PRODUCT_RETURN;

 private:
  static void generate_code_for_get_thread();
  static int _thread_index;

  static Thread* _get_thread_cache[]; // index by [pd_sp_hash % _pd_cache_size]
  static Thread* ThreadLocalStorage::get_thread_via_cache_slowly(
                                                 uintptr_t raw_id, int index);

  NOT_PRODUCT(static int _tcacheHit;)
  NOT_PRODUCT(static int _tcacheMiss;)
};


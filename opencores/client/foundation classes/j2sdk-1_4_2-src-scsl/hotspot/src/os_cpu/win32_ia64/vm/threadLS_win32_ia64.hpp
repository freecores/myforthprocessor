#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)threadLS_win32_ia64.hpp	1.6 03/01/23 11:11:39 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Processor dependent parts of ThreadLocalStorage
public:
  enum Constants {
    _pd_cache_size         =   8*2, // projected max # of processors * 2
    _pd_min_page_size      =   4*K, // smallest possible thread-stack page size
    _pd_typical_stack_size = 512*K, // typical "small" thread stack size
  };

  static address pd_sp_address() { return os::current_stack_pointer(); }

  static inline intptr_t thread_offset() {return thread_index() * sizeof(intptr_t);}

  // Java Thread
  static inline Thread *ThreadLocalStorage::get_thread() { 
    return (Thread*)TlsGetValue(thread_index());
  }

  static inline Thread* thread() { return get_thread(); }


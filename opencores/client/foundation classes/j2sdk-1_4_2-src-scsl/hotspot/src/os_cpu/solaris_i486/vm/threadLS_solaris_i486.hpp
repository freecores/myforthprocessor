#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)threadLS_solaris_i486.hpp	1.9 03/01/23 11:10:07 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Processor dependent parts of ThreadLocalStorage

public:

  enum Constants {
    _pd_cache_size         =  8*2,   // projected max # of processors * 2
    _pd_min_page_size      =   4*K, // smallest possible thread-stack page size
    _pd_typical_stack_size = 512*K // typical "small" thread stack size
  };

  static address pd_sp_address() {
    int junk;
    return (address)&junk;
  }

  // Note:  Two sps from different threads must __not__ have the same page!
  // On i486, create the index in the tls cache using the stack page.
  static uintptr_t pd_raw_thread_id() {
    address sp = pd_sp_address();
    return (unsigned int)sp / _pd_min_page_size;
  }

  // On solaris x86, use the page as thread cache index because I can't
  // seem to get .il files working.
  static int pd_cache_index(uintptr_t sp_page) {
    return ((sp_page / 2)  /* pages tend to come in pairs */
         ^ (sp_page / (_pd_typical_stack_size/_pd_min_page_size)))
         % _pd_cache_size;
  }

  // Java Thread
  static inline Thread* thread();

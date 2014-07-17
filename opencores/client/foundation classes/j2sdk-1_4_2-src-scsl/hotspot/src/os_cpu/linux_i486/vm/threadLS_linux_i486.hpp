#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)threadLS_linux_i486.hpp	1.6 03/01/23 11:09:09 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Processor dependent parts of ThreadLocalStorage

public:

  enum Constants {
    _pd_cache_size         =  8*2,  // projected max # of processors * 2
    _pd_min_page_size      =  4*K,  // smallest possible thread-stack page size
    _pd_typical_stack_size = 512*K  // typical "small" thread stack size
  };

  static address pd_sp_address() {
    int junk;
    return (address)&junk;
  }

  static Thread* thread();

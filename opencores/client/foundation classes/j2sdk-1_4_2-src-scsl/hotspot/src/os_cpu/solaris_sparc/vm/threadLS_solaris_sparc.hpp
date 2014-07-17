#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)threadLS_solaris_sparc.hpp	1.17 03/01/23 11:10:36 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

public:
  // Java Thread  - force inlining
  static inline Thread* thread() ; 

public:
  enum Constants {
    _pd_cache_size         =  64*2, // projected max # of processors * 2
    _pd_min_page_size      =   4*K, // smallest possible thread-stack page size
    _pd_typical_stack_size = 512*K  // typical "small" thread stack size
  };

  static address pd_sp_address() {
    int junk;
    return (address)&junk;
  }

  static uintptr_t pd_raw_thread_id() {
    return atomic::raw_thread_id();
  }

  static int pd_cache_index(uintptr_t raw_id) {
    // Hash function: From email from Dave Dice:
    // The hash function deserves an explanation.  %g7 points to libthread's
    // "thread" structure.  On T1 the thread structure is allocated on the
    // user's stack (yes, really!) so the ">>20" handles T1 where the JVM's
    // stack size is usually >= 1Mb.  The ">>9" is for T2 where Roger allocates
    // globs of thread blocks contiguously.  The "9" has to do with the
    // expected size of the T2 thread structure.  If these constants are wrong
    // the worst thing that'll happen is that the hit rate for heavily threaded
    // apps won't be as good as it could be.  If you want to burn another
    // shift+xor you could mix together _all of the %g7 bits to form the hash,
    // but I think that's excessive.  Making the change above changed the
    // T$ miss rate on SpecJBB (on a 16X system) from about 3% to imperceptible.
    uintptr_t ix = (int) (((raw_id >> 9) ^ (raw_id >> 20)) % _pd_cache_size);
    return ix;
  }

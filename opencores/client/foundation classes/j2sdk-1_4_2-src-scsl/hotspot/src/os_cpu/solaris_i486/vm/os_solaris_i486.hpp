#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)os_solaris_i486.hpp	1.10 03/01/23 11:10:00 JVM"
#endif
// 
// Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
//

  //
  // NOTE: we are back in class os here, not Solaris
  //

  static jint (*atomic_exchange_func)              (jint, jint*);
  static jint (*atomic_compare_and_exchange_func)  (jint, jint*, jint);
  static jlong (*atomic_compare_and_exchange_long_func)  (jlong, jlong*, jlong);
  static jint (*atomic_increment_func)             (jint, jint*);
  static void (*atomic_membar_func)                ();

  static jint atomic_exchange_bootstrap            (jint, jint*);
  static jint atomic_compare_and_exchange_bootstrap(jint, jint*, jint);
  static jlong atomic_compare_and_exchange_long_bootstrap(jlong, jlong*, jlong);
  static jint atomic_increment_bootstrap           (jint, jint*);
  static void atomic_membar_bootstrap              ();

  static bool supports_sse() { return false; }

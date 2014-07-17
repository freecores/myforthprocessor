#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)threadLS_solaris_sparc.cpp	1.8 03/01/23 11:10:34 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Provides an entry point we can link against and
// a buffer we can emit code into. The buffer is
// filled by ThreadLocalStorage::generate_code_for_get_thread
// and called from ThreadLocalStorage::thread()

#include "incls/_precompiled.incl"
#include "incls/_threadLS_solaris_sparc.cpp.incl"
#include <sys/systeminfo.h>

// The portable TLS mechanism (get_thread_via_cache) is enough on SPARC.
// There is no need for hand-assembling a special function.
void ThreadLocalStorage::generate_code_for_get_thread() {
}

extern "C" Thread* get_thread() {
  return ThreadLocalStorage::thread();
}

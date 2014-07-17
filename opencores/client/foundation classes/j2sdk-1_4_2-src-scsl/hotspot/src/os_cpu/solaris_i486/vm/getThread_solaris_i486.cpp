#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)getThread_solaris_i486.cpp	1.7 03/01/23 11:09:52 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Provides an entry point we can link against and
// a buffer we can emit code into. The buffer is
// filled by ThreadLocalStorage::generate_code_for_get_thread
// and called from ThreadLocalStorage::thread()

// do not include precompiled header file
#include "incls/_getThread_solaris_i486.cpp.incl"


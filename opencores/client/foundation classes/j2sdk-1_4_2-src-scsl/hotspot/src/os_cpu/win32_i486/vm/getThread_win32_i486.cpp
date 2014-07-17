#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)getThread_win32_i486.cpp	1.13 03/01/23 11:10:53 JVM"
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
#include "incls/_getThread_win32_i486.cpp.incl"

const int get_thread_size = 20; // a copy of this is used in ThreadLocalStorage::generate_code_for_get_thread
extern "C" char get_thread[get_thread_size];
char get_thread[get_thread_size];


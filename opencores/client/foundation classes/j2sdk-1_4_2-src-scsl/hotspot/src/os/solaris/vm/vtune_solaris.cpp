#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)vtune_solaris.cpp	1.15 03/01/23 11:06:05 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_vtune_solaris.cpp.incl"

// put OS-includes here


// This file may say "vtune", but it supports the Forte(TM)
// Peformance Tools collector. The method prototype is derived
// from libcollector.h. For more information, please see the
// libcollect man page.

extern "C" {

// Method to let libcollector know about a dynamically loaded function.
// Because it is weakly bound, the calls become NOP's when the library
// isn't present.
void    collector_func_load(char* name,
                            void* null_argument_1,
                            void* null_argument_2,
                            void *vaddr,
                            int size,
                            int zero_argument,
                            void* null_argument_3);
#pragma weak collector_func_load
#define collector_func_load(x0,x1,x2,x3,x4,x5,x6) \
        ( collector_func_load ? collector_func_load(x0,x1,x2,x3,x4,x5,x6),0 : 0 )
}

void VTune::start_GC() {}
void VTune::end_GC() {}
void VTune::start_class_load() {}
void VTune::end_class_load() {}
void VTune::exit() {}

// I'm co-opting this to pass info to the Forte profiler...
void VTune::register_stub(const char* name, address start, address end) {
  assert(pointer_delta(end, start, sizeof(jbyte)) < INT_MAX, "Code size exceeds maximum range")
  collector_func_load((char*)name, NULL, NULL, start, pointer_delta(end, start, sizeof(jbyte)), 0, NULL);
}

#ifndef CORE
void VTune::create_nmethod(nmethod* nm) {}
void VTune::delete_nmethod(nmethod* nm) {}
#endif

void vtune_init() {}


//Reconciliation History
// 1.7 98/05/15 09:52:12 vtune_win32.cpp
// 1.8 98/11/11 13:22:55 vtune_win32.cpp
// 1.9 98/12/04 17:37:54 vtune_win32.cpp
// 1.12 99/06/28 11:01:49 vtune_win32.cpp
//End


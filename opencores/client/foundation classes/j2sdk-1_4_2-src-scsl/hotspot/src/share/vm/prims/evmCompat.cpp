#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)evmCompat.cpp	1.5 03/01/23 12:19:31 JVM"
#endif
// 
// Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

// This file contains definitions for functions that exist
// in the ExactVM, but not in HotSpot. They are stubbed out
// here to prevent linker errors when attempting to use HotSpot
// with the ExactVM jdk.

# include "incls/_precompiled.incl"
# include "incls/_evmCompat.cpp.incl"

extern "C" void JVM_Process_DestroyProcess(void);
extern "C" void JVM_Process_ForkAndExec(void);
extern "C" void JVM_Process_WaitForProcessExit(void);
extern "C" void gc(void);

void JVM_Process_DestroyProcess(void) {
  ShouldNotReachHere();
}

void JVM_Process_ForkAndExec(void) {
  ShouldNotReachHere();
}

void JVM_Process_WaitForProcessExit(void) {
  ShouldNotReachHere();
}

void gc(void) {
  ShouldNotReachHere();
}

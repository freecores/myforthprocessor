#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)shared.cpp	1.23 03/01/23 12:05:48 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_shared.cpp.incl"

// First register number that refers to a stack slot
OptoReg::Name SharedInfo::stack0;

// Register names
const char *SharedInfo::regName[REGNAME_SIZE];

// Stack pointer register
OptoReg::Name SharedInfo::c_frame_pointer;



void SharedInfo::set_stack0(int n) {
  COMPILER2_ONLY(ShouldNotReachHere();)           // temporary solution
  SharedInfo::stack0 = OptoReg::Name(n);
  assert(RegisterImpl::number_of_registers <= n, "too may registers");
  set_regName();
}


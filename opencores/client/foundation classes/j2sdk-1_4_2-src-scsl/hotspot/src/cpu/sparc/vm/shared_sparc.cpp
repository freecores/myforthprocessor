#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)shared_sparc.cpp	1.9 03/01/23 11:02:17 JVM"
#endif
// 
// Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

#include "incls/_precompiled.incl"
#include "incls/_shared_sparc.cpp.incl"



void SharedInfo::set_regName() {
  Register reg = G0;
  for (uint i = 0;
       i < RegisterImpl::number_of_registers;
       ++i, reg = reg->successor()) {
    regName[i] = reg->name();
  }

#ifdef COMPILER1
  uint fbase = RegisterImpl::number_of_registers;
  FloatRegister freg = F0;
  for (uint i = 0;
       i < FloatRegisterImpl::number_of_registers;
       ++i, freg = freg->successor()) {
    regName[fbase+i] = freg->name();
  }
#endif
}

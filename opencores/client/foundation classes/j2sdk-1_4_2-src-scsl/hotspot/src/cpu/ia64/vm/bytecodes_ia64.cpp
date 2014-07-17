#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)bytecodes_ia64.cpp	1.3 03/01/23 10:56:36 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_bytecodes_ia64.cpp.incl"


void Bytecodes::pd_initialize() {
  // No ia64 specific initialization
}


Bytecodes::Code Bytecodes::pd_base_code_for(Code code) {
  // No ia64 specific bytecodes
  return code;
}


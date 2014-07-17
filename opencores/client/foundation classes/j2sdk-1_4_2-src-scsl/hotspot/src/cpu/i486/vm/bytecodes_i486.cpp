#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)bytecodes_i486.cpp	1.11 03/01/23 10:52:16 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_bytecodes_i486.cpp.incl"


void Bytecodes::pd_initialize() {
  // No i486 specific initialization
}


Bytecodes::Code Bytecodes::pd_base_code_for(Code code) {
  // No i486 specific bytecodes
  return code;
}


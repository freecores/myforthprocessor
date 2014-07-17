#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)assembler_linux_ia64.cpp	1.6 03/01/23 11:09:17 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_assembler_linux_ia64.cpp.incl"

// We don't have implicit null check support yet so we have
// to do explicit checks for nulls.
bool MacroAssembler::needs_explicit_null_check(intptr_t offset) {
#if 0
  // The kernel is at low addresses on x86 Linux so we can
  // avoid checking for NULL in it's range.
  // I'm not sure that this is true for IA64
  return (offset < 0 || offset >= 0x100000);
#else
  return true;
#endif
}


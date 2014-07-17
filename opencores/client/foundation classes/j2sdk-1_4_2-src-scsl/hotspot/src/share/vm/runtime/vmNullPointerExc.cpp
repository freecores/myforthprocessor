#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)vmNullPointerExc.cpp	1.18 03/01/23 12:26:55 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_vmNullPointerExc.cpp.incl"

void VmNullPointerException::stop_vm(char* message) {
  MutexLocker m(Event_lock);
  tty->print_cr("Unexpected Null pointer exception (%s)", message);
  if (Universe::heap()->is_gc_active()) {
    tty->print_cr("- garbage collection");
  }
  warning("null pointer exception for VM thread");
  ShouldNotReachHere();
}

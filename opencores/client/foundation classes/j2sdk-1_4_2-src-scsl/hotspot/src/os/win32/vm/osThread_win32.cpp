#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)osThread_win32.cpp	1.9 03/01/23 11:08:17 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_osThread_win32.cpp.incl"

void OSThread::pd_initialize() {
  set_thread_handle(NULL);
  set_thread_id(NULL);
  set_interrupt_event(NULL);
}

// TODO: this is not well encapsulated; creation and deletion of the
// interrupt_event are done in os_win32.cpp, create_thread and
// free_thread. Should follow pattern of Linux/Solaris code here.
void OSThread::pd_destroy() {
}

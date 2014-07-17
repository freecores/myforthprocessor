#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)osThread_linux.cpp	1.13 03/01/23 11:03:48 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// do not include  precompiled  header file
# include "incls/_osThread_linux.cpp.incl"

void OSThread::pd_initialize() {
  assert(this != NULL, "check");
  _thread_id        = 0;
  _pthread_id       = 0;
  _siginfo = NULL;
  _ucontext = NULL;
  _expanding_stack = 0;
  _alt_sig_stack = NULL;


  _interrupt_event  = new os::Linux::Event();
  assert(_interrupt_event != NULL, "check");

  _startThread_event = new os::Linux::Event();
  assert(_startThread_event !=NULL, "check");
}

void OSThread::pd_destroy() {
  delete _interrupt_event;
  delete _startThread_event;
}

// Reconciliation History
// osThread_solaris.cpp	1.18 99/07/15 14:33:21
// End

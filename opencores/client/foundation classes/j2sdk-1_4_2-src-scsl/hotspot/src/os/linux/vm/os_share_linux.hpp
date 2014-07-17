#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)os_share_linux.hpp	1.3 03/01/23 11:04:02 JVM"
#endif
//
// Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

// misc
void signalHandler(int, siginfo_t*, ucontext_t*);
void handle_unexpected_exception(Thread* thread, int sig, siginfo_t* info, address pc, address adjusted_pc);
#ifndef PRODUCT
void continue_with_dump(void);
#endif

#define PROCFILE_LENGTH 128

// Reconciliation History
// os_share_solaris.hpp	1.2 99/08/06 10:14:40
// End

#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)safepoint_solaris.cpp	1.18 03/01/23 11:05:55 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_safepoint_solaris.cpp.incl"


bool SafepointSynchronize::can_be_at_safepoint_before_suspend(JavaThread *thread, JavaThreadState state) {
  switch(state) {
   case _thread_in_native:
     // threads in native code have not yet saved state
     return false;

   // New thread may have resources during creation. (ie. malloc lock)
   case _thread_new:
     return false;

   // blocked threads have saved state
   // new threads have no state to save
   case _thread_blocked:
   case _thread_in_native_blocked:
     return true;

   default:
    ShouldNotReachHere();
  }
}


bool SafepointSynchronize::can_be_at_safepoint_after_suspend(JavaThread *thread, JavaThreadState state) {
  switch(state) {
   case _thread_in_vm_trans:
     // threads in vm code have not yet saved state
     return false;

   default:
    ShouldNotReachHere();
  }
}

/*
 * @(#)WindbgThread.java	1.2 03/01/23 11:35:49
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.debugger.windbg;

import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.debugger.x86.*;

class WindbgThread implements ThreadProxy {
  private WindbgDebugger debugger;
  private long           sysId;
  private boolean        gotID;
  private long           id;

  /** The address argument must be the address of the HANDLE of the
      desired thread in the target process. */
  WindbgThread(WindbgDebugger debugger, Address addr) {
    this.debugger = debugger;
    // FIXME: size of data fetched here should be configurable.
    // However, making it so would produce a dependency on the "types"
    // package from the debugger package, which is not desired.

    // another hack here is that we use sys thread id instead of handle.
    // windbg can't get details based on handles it seems.
    // I assume that osThread_win32 thread struct has _thread_id (which
    // sys thread id) just after handle field.

    this.sysId   = (int) addr.addOffsetTo(4).getCIntegerAt(0, 4, true);
    gotID = false;
  }

  WindbgThread(WindbgDebugger debugger, long sysId) {
    this.debugger = debugger;
    this.sysId    = sysId;
    gotID         = false;
  }

  public ThreadContext getContext() throws IllegalThreadStateException {
    long[] data = debugger.getThreadIntegerRegisterSet(getThreadID());
    WindbgThreadContext context = new WindbgThreadContext(debugger);
    for (int i = 0; i < data.length; i++) {
      context.setRegister(i, data[i]);
    }
    return context;
  }

  public boolean canSetContext() throws DebuggerException {
    return false;
  }

  public void setContext(ThreadContext thrCtx)
    throws IllegalThreadStateException, DebuggerException {
    throw new DebuggerException("Unimplemented");
  }

  public boolean equals(Object obj) {
    if ((obj == null) || !(obj instanceof WindbgThread)) {
      return false;
    }

    return (((WindbgThread) obj).getThreadID() == getThreadID());
  }

  public int hashCode() {
    return (int) getThreadID();
  }

  public String toString() {
    return Long.toString(getThreadID());
  }

  /** Retrieves the thread ID of this thread by examining the Thread
      Information Block. */
  private long getThreadID() {
    if (!gotID) {
       id = debugger.getThreadIdFromSysId(sysId);
    }
    
    return id;
  }
}

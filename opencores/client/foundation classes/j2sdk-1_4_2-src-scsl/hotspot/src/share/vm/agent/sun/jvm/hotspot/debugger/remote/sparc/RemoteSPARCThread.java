/*
 * @(#)RemoteSPARCThread.java	1.3 03/01/23 11:31:21
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.debugger.remote.sparc;

import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.debugger.sparc.*;
import sun.jvm.hotspot.debugger.remote.*;
import sun.jvm.hotspot.utilities.*;

public class RemoteSPARCThread extends RemoteThread {
  public RemoteSPARCThread(RemoteDebuggerClient debugger, Address addr) {
     super(debugger, addr);
  }

  public RemoteSPARCThread(RemoteDebuggerClient debugger, long id) {
     super(debugger, id);
  }

  public ThreadContext getContext() throws IllegalThreadStateException {
    RemoteSPARCThreadContext context = new RemoteSPARCThreadContext(debugger);
    long[] regs = (addr != null)? debugger.getThreadIntegerRegisterSet(addr) :
                                  debugger.getThreadIntegerRegisterSet(id);

    if (Assert.ASSERTS_ENABLED) {
      Assert.that(regs.length == SPARCThreadContext.NPRGREG, "size of register set must match");
    }
    for (int i = 0; i < regs.length; i++) {
      context.setRegister(i, regs[i]);
    }
    return context;
  }
}

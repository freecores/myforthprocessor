/*
 * @(#)RemoteX86Thread.java	1.3 03/01/23 11:31:28
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.debugger.remote.x86;

import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.debugger.x86.*;
import sun.jvm.hotspot.debugger.remote.*;
import sun.jvm.hotspot.utilities.*;

public class RemoteX86Thread extends RemoteThread  {
  public RemoteX86Thread(RemoteDebuggerClient debugger, Address addr) {
     super(debugger, addr);
  }

  public RemoteX86Thread(RemoteDebuggerClient debugger, long id) {
     super(debugger, id);
  }

  public ThreadContext getContext() throws IllegalThreadStateException {
    RemoteX86ThreadContext context = new RemoteX86ThreadContext(debugger);
    long[] regs = (addr != null)? debugger.getThreadIntegerRegisterSet(addr) :
                                  debugger.getThreadIntegerRegisterSet(id);
    if (Assert.ASSERTS_ENABLED) {
      Assert.that(regs.length == X86ThreadContext.NPRGREG, "size of register set must match");
    }
    for (int i = 0; i < regs.length; i++) {
      context.setRegister(i, regs[i]);
    }
    return context;
  }
}

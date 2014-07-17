/*
 * @(#)WindbgCDebugger.java	1.2 03/01/23 11:35:39
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.debugger.windbg;

import java.io.*;
import java.util.*;
import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.debugger.cdbg.*;
import sun.jvm.hotspot.debugger.cdbg.basic.x86.*;
import sun.jvm.hotspot.debugger.x86.*;
import sun.jvm.hotspot.utilities.AddressOps;

class WindbgCDebugger implements CDebugger {
  // FIXME: think about how to make this work in a remote debugging
  // scenario; who should keep open DLLs? Need local copies of these
  // DLLs on the debugging machine?
  private WindbgDebugger dbg;

  WindbgCDebugger(WindbgDebugger dbg) {
    this.dbg = dbg;
  }

  public List getThreadList() throws DebuggerException {
    return dbg.getThreadList();
  }

  public List/*<LoadObject>*/ getLoadObjectList() throws DebuggerException{
    return dbg.getLoadObjectList();
  }

  public LoadObject loadObjectContainingPC(Address pc) throws DebuggerException {
    // FIXME: could keep sorted list of these to be able to do binary
    // searches, for better scalability
    if (pc == null) {
      return null;
    }
    List objs = getLoadObjectList();
    for (Iterator iter = objs.iterator(); iter.hasNext(); ) {
      LoadObject obj = (LoadObject) iter.next();
      if (AddressOps.lte(obj.getBase(), pc) && (pc.minus(obj.getBase()) < obj.getSize())) {
        return obj;
      }
    }
    return null;
  }

  public CFrame topFrameForThread(ThreadProxy thread) throws DebuggerException {
    X86ThreadContext context = (X86ThreadContext) thread.getContext();
    Address ebp = context.getRegisterAsAddress(X86ThreadContext.EBP);
    if (ebp == null) return null;
    Address pc  = context.getRegisterAsAddress(X86ThreadContext.EIP);
    if (pc == null) return null;
    return new X86CFrame(this, ebp, pc);
  }

  public String getNameOfFile(String fileName) {
    return new File(fileName).getName();
  }

  public ProcessControl getProcessControl() throws DebuggerException {
    return null;
  }
}

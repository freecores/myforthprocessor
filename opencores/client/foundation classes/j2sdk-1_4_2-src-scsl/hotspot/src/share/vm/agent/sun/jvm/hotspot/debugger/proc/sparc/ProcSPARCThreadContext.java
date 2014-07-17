/*
 * @(#)ProcSPARCThreadContext.java	1.3 03/01/23 11:30:50
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.debugger.proc.sparc;

import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.debugger.sparc.*;
import sun.jvm.hotspot.debugger.proc.*;

public class ProcSPARCThreadContext extends SPARCThreadContext {
  private ProcDebugger debugger;

  public ProcSPARCThreadContext(ProcDebugger debugger) {
    super();
    this.debugger = debugger;
  }

  /** This can't be implemented in this class since we would have to
      tie the implementation to, for example, the debugging system */
  public void setRegisterAsAddress(int index, Address value) {
    setRegister(index, debugger.getAddressValue(value));
  }

  /** This can't be implemented in this class since we would have to
      tie the implementation to, for example, the debugging system */
  public Address getRegisterAsAddress(int index) {
    return debugger.newAddress(getRegister(index));
  }
}

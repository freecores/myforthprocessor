/*
 * @(#)X86CFrame.java	1.3 03/01/23 11:29:12
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.debugger.cdbg.basic.x86;

import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.debugger.cdbg.*;
import sun.jvm.hotspot.debugger.cdbg.basic.*;

/** Basic X86 frame functionality providing sender() functionality. */

public class X86CFrame extends BasicCFrame {
  private Address ebp;
  private Address pc;

  private static final int ADDRESS_SIZE = 4;

  /** Constructor for topmost frame */
  public X86CFrame(CDebugger dbg, Address ebp, Address pc) {
    super(dbg);
    this.ebp = ebp;
    this.pc  = pc;
  }

  public CFrame sender() {
    if (ebp == null) {
      return null;
    }

    Address nextEBP = ebp.getAddressAt( 0 * ADDRESS_SIZE);
    if (nextEBP == null) {
      return null;
    }
    Address nextPC  = ebp.getAddressAt( 1 * ADDRESS_SIZE);
    if (nextPC == null) {
      return null;
    }
    return new X86CFrame(dbg(), nextEBP, nextPC);
  }

  public Address pc() {
    return pc;
  }

  public Address localVariableBase() {
    return ebp;
  }
}

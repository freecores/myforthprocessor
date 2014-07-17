/*
 * @(#)X86Register.java	1.4 03/01/23 11:22:32
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.asm.x86;

import sun.jvm.hotspot.asm.*;

public class X86Register extends Register {
  public X86Register(int num) {
    super(num);
  }

  public int getNumberOfRegisters() {
    return X86Registers.getNumberOfRegisters();
  }

  public boolean isStackPointer() {
    return number == 4; // is ESP?
  }

  public boolean isFramePointer() {
    return number == 5; // is EBP?
  }

  public boolean isFloat() {
    return false;
  }
}

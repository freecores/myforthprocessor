/*
 * @(#)CPUHelper.java	1.2 03/01/23 11:14:37
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.asm;

public interface CPUHelper {
   public Disassembler createDisassembler(long startPc, byte[] code);
   public Register getIntegerRegister(int num);
   public Register getFloatRegister(int num);
   public Register getStackPointer();
   public Register getFramePointer();
}

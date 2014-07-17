/*
 * @(#)SPARCHelper.java	1.2 03/01/23 11:17:24
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.asm.sparc;

import sun.jvm.hotspot.asm.*;

public class SPARCHelper implements CPUHelper {
   public Disassembler createDisassembler(long startPc, byte[] code) {
      return new SPARCV9Disassembler(startPc, code);
   }

   public Register getIntegerRegister(int num) {
      return SPARCRegisters.getRegister(num);
   }

   public Register getFloatRegister(int num) {
      return SPARCFloatRegisters.getRegister(num);
   }

   public Register getStackPointer() {
      return SPARCRegisters.O7;
   }

   public Register getFramePointer() {
      return SPARCRegisters.I7;
   }
}

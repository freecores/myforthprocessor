/*
 * @(#)X86Helper.java	1.2 03/01/23 11:22:29
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.asm.x86;

import sun.jvm.hotspot.asm.*;

// FIXME: placeholder class - change it when X86 disassembler is
// ready.

public class X86Helper implements CPUHelper {
   public Disassembler createDisassembler(long startPc, byte[] code) {
      return null;
   }

   public Register getIntegerRegister(int num) {
      return null;
   }

   public Register getFloatRegister(int num) {
      return null;
   }

   public Register getStackPointer() {
      return null;
   }

   public Register getFramePointer() {
      return null;
   }
}

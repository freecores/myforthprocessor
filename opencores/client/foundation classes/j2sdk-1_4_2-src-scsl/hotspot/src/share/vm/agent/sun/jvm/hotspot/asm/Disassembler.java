/*
 * @(#)Disassembler.java	1.2 03/01/23 11:14:46
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.asm;

public abstract class Disassembler {
   protected long   startPc;
   protected byte[] code;

   public Disassembler(long startPc, byte[] code) {
      this.startPc = startPc;
      this.code = code;
   }

   public long getStartPC() {
      return startPc;
   }

   public byte[] getCode() {
      return code;
   }

   public abstract void decode(InstructionVisitor visitor);
}

/*
 * @(#)Instruction.java	1.2 03/01/23 11:14:58
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.asm;

public interface Instruction {
   public String getName();

   // total size in bytes (operands + opcode).
   // for eg. in sparc it is always 4 (= 32bits)
   public int getSize();

   // some type testers
   public boolean isIllegal();
   public boolean isArithmetic();
   public boolean isLogical();
   public boolean isShift();
   public boolean isMove();
   public boolean isBranch();
   public boolean isCall();
   public boolean isReturn();
   public boolean isLoad();
   public boolean isStore();
   public boolean isFloat();
   public boolean isTrap();
   public boolean isNoop();

   // convert the instruction as String given currentPc
   // and SymbolFinder

   public String asString(long currentPc, SymbolFinder symFinder);
}

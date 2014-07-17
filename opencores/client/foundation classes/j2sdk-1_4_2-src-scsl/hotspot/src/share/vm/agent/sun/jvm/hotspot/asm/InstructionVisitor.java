/*
 * @(#)InstructionVisitor.java	1.2 03/01/23 11:15:01
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.asm;

public interface InstructionVisitor {
   public void prologue();
   public void visit(long currentPc, Instruction instr);
   public void epilogue();
}

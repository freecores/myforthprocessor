/*
 * @(#)ShiftInstruction.java	1.2 03/01/23 11:15:27
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.asm;

public interface ShiftInstruction extends Instruction, RTLOperations {
   public Operand getShiftSource();
   public Operand  getShiftLength(); // number of bits to shift
   public Operand getShiftDestination();
}

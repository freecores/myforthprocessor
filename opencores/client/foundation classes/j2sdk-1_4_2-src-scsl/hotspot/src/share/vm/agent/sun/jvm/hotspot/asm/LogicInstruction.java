/*
 * @(#)LogicInstruction.java	1.2 03/01/23 11:15:05
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.asm;

public interface LogicInstruction extends Instruction, RTLOperations {
   public Operand[]  getLogicSources();
   public Operand    getLogicDestination();
   public int        getOperation(); // one of RTLOperations
}

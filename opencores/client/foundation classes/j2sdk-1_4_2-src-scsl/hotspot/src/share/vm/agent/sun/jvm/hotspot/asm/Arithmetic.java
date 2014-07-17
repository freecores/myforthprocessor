/*
 * @(#)Arithmetic.java	1.2 03/01/23 11:14:27
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.asm;

public interface Arithmetic extends Instruction, RTLOperations {
   public Operand[]  getArithmeticSources();
   public Operand    getArithmeticDestination();
   public int        getOperation(); // one of RTLOperations
}

/*
 * @(#)BranchInstruction.java	1.2 03/01/23 11:14:35
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.asm;

public interface BranchInstruction extends Instruction {
   public boolean isConditional();
   public Address getBranchDestination();
}

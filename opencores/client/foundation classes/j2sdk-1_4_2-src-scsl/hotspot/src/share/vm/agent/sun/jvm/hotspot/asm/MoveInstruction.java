/*
 * @(#)MoveInstruction.java	1.2 03/01/23 11:15:10
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.asm;

public interface MoveInstruction extends Instruction {
   public ImmediateOrRegister  getMoveSource();
   public Register getMoveDestination();
   // for condition moves
   public boolean  isConditional();
}

/*
 * @(#)SPARCIllegalInstruction.java	1.2 03/01/23 11:17:26
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.asm.sparc;

import sun.jvm.hotspot.asm.SymbolFinder;

public final class SPARCIllegalInstruction extends SPARCInstruction {
    final private int instruction;
    final private String description;

    public SPARCIllegalInstruction(int instruction) {
        super("illegal");
        this.instruction = instruction;
        description = "bad opcode - " + Integer.toHexString(instruction);
    }

    public String asString(long currentPc, SymbolFinder symFinder) {
        return description;
    }

    public int getInstruction() {
        return instruction;
    }

    public boolean isIllegal() {
        return true;
    }
}

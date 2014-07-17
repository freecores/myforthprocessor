/*
 * @(#)SPARCV9SpecialRegisterInstruction.java	1.3 03/01/23 11:20:10
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.asm.sparc;

import sun.jvm.hotspot.asm.*;

public abstract class SPARCV9SpecialRegisterInstruction 
                       extends SPARCInstruction
                       implements /* import */ SPARCV9SpecialRegisters, SPARCV9Instruction {
    protected SPARCV9SpecialRegisterInstruction(String name) {
        super(name);
    }

    protected abstract String getDescription();

    public String asString(long currentPc, SymbolFinder symFinder) {
        return getDescription();
    }

    protected static String[] specialRegNames = new String[] {
       "%y",
       null,
       "%ccr",
       "%asi",
       "%tick",
       "%pc",
       "%fprs",
       "%asr",
    };

    protected static String getSpecialRegisterName(int index) {
       return specialRegNames[index];
    }
}

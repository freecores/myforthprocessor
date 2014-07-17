/*
 * @(#)SPARCSpecialRegisterInstruction.java	1.3 03/01/23 11:18:31
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.asm.sparc;

import sun.jvm.hotspot.asm.*;

public abstract class SPARCSpecialRegisterInstruction 
                       extends SPARCInstruction
                       implements /* import */ SPARCSpecialRegisters {
    protected SPARCSpecialRegisterInstruction(String name) {
        super(name);
    }

    protected abstract String getDescription();

    public String asString(long currentPc, SymbolFinder symFinder) {
        return getDescription();
    }

    protected static String[] specialRegNames = new String[] {
       "%y",
       "%psr",
       "%wim",
       "%tbr",
       "%asr",
       "%fsr",
       "%csr",
       "%fq",
       "%cq"
    };

    protected static String getSpecialRegisterName(int index) {
       return specialRegNames[index];
    }
}

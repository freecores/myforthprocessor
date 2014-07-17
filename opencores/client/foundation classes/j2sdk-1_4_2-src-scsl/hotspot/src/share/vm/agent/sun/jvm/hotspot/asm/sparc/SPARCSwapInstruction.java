/*
 * @(#)SPARCSwapInstruction.java	1.2 03/01/23 11:18:43
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.asm.sparc;

import sun.jvm.hotspot.asm.*;

public class SPARCSwapInstruction extends SPARCAtomicLoadStoreInstruction {
    public SPARCSwapInstruction(String name, SPARCRegisterIndirectAddress addr, SPARCRegister rd) {
        super(name, addr, rd);
    }

    public int getDataType() {
        return RTLDT_UNSIGNED_WORD;
    }

    public boolean isConditional() {
        return false;
    }
}

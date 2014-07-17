/*
 * @(#)SPARCLdstubInstruction.java	1.2 03/01/23 11:17:42
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.asm.sparc;

import sun.jvm.hotspot.asm.*;

public class SPARCLdstubInstruction extends SPARCAtomicLoadStoreInstruction {
    public SPARCLdstubInstruction(String name, SPARCRegisterIndirectAddress addr, SPARCRegister rd) {
        super(name, addr, rd);
    }

    public int getDataType() {
        return RTLDT_UNSIGNED_BYTE;
    }

    public boolean isConditional() {
        return false;
    }
}

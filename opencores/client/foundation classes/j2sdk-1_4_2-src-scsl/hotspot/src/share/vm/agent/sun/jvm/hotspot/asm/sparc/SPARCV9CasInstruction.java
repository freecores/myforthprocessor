/*
 * @(#)SPARCV9CasInstruction.java	1.2 03/01/23 11:18:55
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.asm.sparc;

import sun.jvm.hotspot.asm.*;

public class SPARCV9CasInstruction extends SPARCAtomicLoadStoreInstruction 
                    implements SPARCV9Instruction {
    final private SPARCRegister rs2;
    final private int dataType;

    public SPARCV9CasInstruction(String name, SPARCRegisterIndirectAddress addr, 
                      SPARCRegister rs2, SPARCRegister rd, int dataType) {
        super(name, addr, rd);
        this.rs2 = rs2;
        this.dataType = dataType;
    }

    public int getDataType() {
        return dataType;
    }

    public boolean isConditional() {
        return true;
    }

    public SPARCRegister getComparisonRegister() {
        return rs2;
    }
}

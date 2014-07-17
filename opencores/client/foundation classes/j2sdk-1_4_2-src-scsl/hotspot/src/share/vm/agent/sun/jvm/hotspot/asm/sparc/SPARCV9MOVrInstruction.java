/*
 * @(#)SPARCV9MOVrInstruction.java	1.3 03/01/23 11:19:27
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.asm.sparc;

import sun.jvm.hotspot.asm.*;

public class SPARCV9MOVrInstruction extends SPARCMoveInstruction 
                     implements SPARCV9Instruction {
    final private int regConditionCode;
    final private SPARCRegister rs1;

    public SPARCV9MOVrInstruction(String name, SPARCRegister rs1, 
                                  ImmediateOrRegister operand2, SPARCRegister rd,
                                  int regConditionCode) {
        super(name, MOVr, operand2, rd);
        this.regConditionCode = regConditionCode;
        this.rs1 = rs1;
    }

    protected String getDescription() {
        StringBuffer buf = new StringBuffer();
        buf.append(getName());
        buf.append(spaces);
        buf.append(rs1.toString());
        buf.append(comma);
        buf.append(getOperand2String());
        buf.append(comma);
        buf.append(rd.toString());
        return buf.toString();
    }

    public int getRegisterConditionCode() {
        return regConditionCode;
    }

    public boolean isConditional() {
        return true;
    }

    public Register getConditionRegister() {
        return getSourceRegister1();
    }
}

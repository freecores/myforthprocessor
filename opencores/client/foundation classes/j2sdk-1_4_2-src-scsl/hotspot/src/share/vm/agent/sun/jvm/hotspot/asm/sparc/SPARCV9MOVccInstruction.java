/*
 * @(#)SPARCV9MOVccInstruction.java	1.3 03/01/23 11:19:25
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.asm.sparc;

import sun.jvm.hotspot.asm.*;

public class SPARCV9MOVccInstruction extends SPARCMoveInstruction 
                     implements SPARCV9Instruction {
    final private int conditionFlag; // condition flag used icc, xcc, fccn etc.
    final private int conditionCode;

    public SPARCV9MOVccInstruction(String name, int conditionCode, int conditionFlag, 
                                   ImmediateOrRegister source, SPARCRegister rd) {
        super(name, MOVcc, source, rd);
        this.conditionCode = conditionCode;
        this.conditionFlag = conditionFlag;
    }

    protected String getDescription() {
        StringBuffer buf = new StringBuffer();
        buf.append(getName());
        buf.append(spaces);
        buf.append(SPARCV9ConditionFlags.getFlagName(conditionFlag));
        buf.append(getOperand2String());
        buf.append(comma);
        buf.append(rd.toString());
        return buf.toString();
    }

    public int getConditionCode() {
        return conditionCode;
    }

    public int getConditionFlag() {
        return conditionFlag;
    }

    public String getConditionFlagName() {
        return SPARCV9ConditionFlags.getFlagName(conditionFlag);
    }

    public boolean isConditional() {
        return true;
    }
}

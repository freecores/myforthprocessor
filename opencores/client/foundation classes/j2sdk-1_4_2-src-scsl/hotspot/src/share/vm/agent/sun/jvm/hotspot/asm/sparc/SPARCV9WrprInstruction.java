/*
 * @(#)SPARCV9WrprInstruction.java	1.3 03/01/23 11:20:18
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.asm.sparc;

import sun.jvm.hotspot.asm.*;

public class SPARCV9WrprInstruction extends SPARCV9PrivilegedRegisterInstruction {
    final private SPARCRegister rs1;
    final private ImmediateOrRegister operand2;

    public SPARCV9WrprInstruction(SPARCRegister rs1, ImmediateOrRegister operand2,
                                  int regNum) {
        super("wrpr", regNum);
        this.rs1 = rs1;
        this.operand2 = operand2;
    }

    protected String getDescription() {
        StringBuffer buf = new StringBuffer();
        buf.append(getName());
        buf.append(spaces);
        buf.append(rs1.toString());
        buf.append(comma);
        if (operand2.isRegister()) {
            buf.append(operand2.toString());
        } else {
            int value = ((Immediate)operand2).getNumber().intValue();
            buf.append(Integer.toHexString(value));
        }
        buf.append(comma);
        buf.append(getPrivilegedRegisterName(regNum));
        return buf.toString();
    }

    public SPARCRegister getSourceRegister1() {
        return rs1;
    }

    public ImmediateOrRegister getOperand2() {
        return operand2;
    }
}

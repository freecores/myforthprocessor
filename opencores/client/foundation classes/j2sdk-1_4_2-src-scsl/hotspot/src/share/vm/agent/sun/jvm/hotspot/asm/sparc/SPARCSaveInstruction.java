/*
 * @(#)SPARCSaveInstruction.java	1.3 03/01/23 11:18:20
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.asm.sparc;

import sun.jvm.hotspot.asm.*;

public class SPARCSaveInstruction extends SPARCFormat3AInstruction {
    final private boolean trivial;
    public SPARCSaveInstruction(SPARCRegister rs1, ImmediateOrRegister operand2, SPARCRegister rd) {
        super("save", SAVE, rs1, operand2, rd);
        SPARCRegister G0 = SPARCRegisters.G0;
        trivial = (rs1 == G0 && operand2 == G0 && rd == G0);
    }

    public boolean isTrivial() {
        return trivial;
    }

    protected String getOperand2String() {
        StringBuffer buf = new StringBuffer();
        if (operand2.isRegister()) {
            buf.append(operand2.toString());
        } else {
            Number number = ((Immediate)operand2).getNumber();
            int value = number.intValue();
            if (value < 0) {
                buf.append("-0x");
                value = -value;
            } else {
                buf.append("0x");
            }

            buf.append(Integer.toHexString(value));
        }
        return buf.toString();
    }

    protected String getDescription() {
        return (trivial) ? getName() : super.getDescription();
    }
}

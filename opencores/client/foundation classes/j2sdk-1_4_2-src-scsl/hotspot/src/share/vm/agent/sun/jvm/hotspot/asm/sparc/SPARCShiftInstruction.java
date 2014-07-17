/*
 * @(#)SPARCShiftInstruction.java	1.2 03/01/23 11:18:25
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.asm.sparc;

import sun.jvm.hotspot.asm.*;

public class SPARCShiftInstruction extends SPARCFormat3AInstruction
    implements ShiftInstruction {
    final private int operation;

    public SPARCShiftInstruction(String name, int opcode, int operation, SPARCRegister rs1, 
                                 ImmediateOrRegister operand2, SPARCRegister rd) {
        super(name, opcode, rs1, operand2, rd);
        this.operation = operation;
    }

    public int getOperation() {
        return operation;
    }

    public Operand getShiftDestination() {
        return getDestinationRegister();
    }

    public Operand getShiftLength() {
        return operand2;
    }

    public Operand getShiftSource() {
        return rs1;
    }

    public boolean isShift() {
        return true;
    }

    protected String getOperand2String() {
        return operand2.toString();
    }
}

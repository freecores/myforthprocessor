/*
 * @(#)V9WrprDecoder.java	1.2 03/01/23 11:22:24
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.asm.sparc;

import sun.jvm.hotspot.asm.*;

class V9WrprDecoder extends V9PrivilegedReadWriteDecoder {
    Instruction decode(int instruction, SPARCInstructionFactory factory) {
        SPARCV9InstructionFactory v9factory = (SPARCV9InstructionFactory) factory;
        Instruction instr = null;
        int prNum = getDestinationRegister(instruction);
        if (isLegalPrivilegedRegister(prNum)) {
            SPARCRegister rs1 = SPARCRegisters.getRegister(getSourceRegister1(instruction));
            ImmediateOrRegister operand2 = getOperand2(instruction);
            instr = v9factory.newV9WrprInstruction(rs1, operand2, prNum);
        } else {
            instr = v9factory.newIllegalInstruction(instruction);
        }

        return instr;
    }
}

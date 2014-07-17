/*
 * @(#)CoprocessorDecoder.java	1.2 03/01/23 11:15:56
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.asm.sparc;

import sun.jvm.hotspot.asm.*;

class CoprocessorDecoder extends InstructionDecoder {
    private int op3;
    CoprocessorDecoder(int op3) {
        this.op3 = op3;
    }

    Instruction decode(int instruction, SPARCInstructionFactory factory) {
        int rs1Num = getSourceRegister1(instruction);
        int rs2Num = getSourceRegister2(instruction);
        int rdNum = getDestinationRegister(instruction);

        return factory.newCoprocessorInstruction(instruction, op3,
                                     (instruction & OPC_MASK) >> OPF_START_BIT,
                                     rs1Num, rs2Num, rdNum);
    }
}


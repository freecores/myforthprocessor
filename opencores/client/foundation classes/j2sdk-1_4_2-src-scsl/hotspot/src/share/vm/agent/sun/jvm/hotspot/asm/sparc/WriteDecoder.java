/*
 * @(#)WriteDecoder.java	1.2 03/01/23 11:22:26
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.asm.sparc;

import sun.jvm.hotspot.asm.*;

class WriteDecoder extends ReadWriteDecoder {
    WriteDecoder(int specialRegNum) {
        super(specialRegNum);
    }

    Instruction decodeReadWrite(int instruction, SPARCInstructionFactory factory,
                                int rs1Num, int rdNum) {
        Instruction instr = null;
        int specialReg = specialRegNum;
        if (rdNum == 0)
            specialReg = SPARCSpecialRegisters.Y;
        return factory.newWriteInstruction(specialReg, rdNum,
                              SPARCRegisters.getRegister(rs1Num),
                              getOperand2(instruction));
    }
}


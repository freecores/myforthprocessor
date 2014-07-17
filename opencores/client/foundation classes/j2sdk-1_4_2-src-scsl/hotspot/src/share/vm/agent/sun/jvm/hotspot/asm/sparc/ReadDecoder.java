/*
 * @(#)ReadDecoder.java	1.2 03/01/23 11:16:38
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.asm.sparc;

import sun.jvm.hotspot.asm.*;

class ReadDecoder extends ReadWriteDecoder {
    ReadDecoder(int specialRegNum) {
        super(specialRegNum);
    }

    Instruction decodeReadWrite(int instruction, SPARCInstructionFactory factory,
                                int rs1Num, int rdNum) {
        Instruction instr = null;
        int specialReg = specialRegNum;
        if (rs1Num == 0)
            specialReg = SPARCSpecialRegisters.Y;
        if (rs1Num == 15 && rdNum == 0) {
            instr = factory.newStbarInstruction();
        } else {
            instr = factory.newReadInstruction(specialReg, rs1Num,
                                  SPARCRegisters.getRegister(rdNum));
        }
        return instr;
    }
}


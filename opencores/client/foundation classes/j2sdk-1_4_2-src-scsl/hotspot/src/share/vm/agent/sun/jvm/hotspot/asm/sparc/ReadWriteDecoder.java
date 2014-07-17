/*
 * @(#)ReadWriteDecoder.java	1.2 03/01/23 11:16:42
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.asm.sparc;

import sun.jvm.hotspot.asm.*;

abstract class ReadWriteDecoder extends InstructionDecoder {
    final int specialRegNum;

    abstract Instruction decodeReadWrite(int instruction,
                               SPARCInstructionFactory factory,
                               int rs1Num, int rdNum);

    ReadWriteDecoder(int specialRegNum) {
        this.specialRegNum = specialRegNum;
    }

    Instruction decode(int instruction, SPARCInstructionFactory factory) {
        Instruction instr = null;
        int rs1Num = getSourceRegister1(instruction);
        int rdNum = getDestinationRegister(instruction);
        return decodeReadWrite(instruction, factory, rs1Num, rdNum);
    }
}


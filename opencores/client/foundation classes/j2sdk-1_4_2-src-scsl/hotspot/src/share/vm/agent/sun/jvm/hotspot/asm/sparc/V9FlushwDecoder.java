/*
 * @(#)V9FlushwDecoder.java	1.2 03/01/23 11:21:37
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.asm.sparc;

import sun.jvm.hotspot.asm.*;

class V9FlushwDecoder extends InstructionDecoder 
               implements V9InstructionDecoder {
    Instruction decode(int instruction, SPARCInstructionFactory factory) {
        SPARCV9InstructionFactory v9factory = (SPARCV9InstructionFactory) factory;
        Instruction instr = null;
        // "i" bit has to be zero. see page 169 - A.21 Flush Register Windows.
        if (isIBitSet(instruction)) {
            instr = v9factory.newIllegalInstruction(instruction);
        } else {
            instr = v9factory.newV9FlushwInstruction();
        }
        return instr;
    }
}

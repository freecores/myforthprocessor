/*
 * @(#)V9DoneRetryDecoder.java	1.2 03/01/23 11:21:22
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.asm.sparc;

import sun.jvm.hotspot.asm.*;

class V9DoneRetryDecoder extends InstructionDecoder 
              implements V9InstructionDecoder {
    Instruction decode(int instruction, SPARCInstructionFactory factory) {
        SPARCV9InstructionFactory v9factory = (SPARCV9InstructionFactory) factory;
        Instruction instr = null;
        int rdNum = getDestinationRegister(instruction);
        // "rd" field is "fcn". Only values 0 and 1 are defined.
        // see page 157 - A.11 Done and Retry
        switch (rdNum) {
            case 0:
                instr = v9factory.newV9DoneInstruction();
                break;
            case 1:
                instr = v9factory.newV9RetryInstruction();
                break;
            default:
                instr = v9factory.newIllegalInstruction(instruction);
                break;
        }
        return instr;
    }
}

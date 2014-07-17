/*
 * @(#)V9SpecialLoadDecoder.java	1.2 03/01/23 11:22:16
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.asm.sparc;

import sun.jvm.hotspot.asm.*;

class V9SpecialLoadDecoder extends MemoryInstructionDecoder 
               implements V9InstructionDecoder {
    V9SpecialLoadDecoder(int op3) {
        super(op3, "ld[x]fsr", RTLDT_UNKNOWN);
    }
    
    Instruction decodeMemoryInstruction(int instruction,
                               SPARCRegisterIndirectAddress addr,
                               SPARCRegister rd, SPARCInstructionFactory factory) {
        return factory.newSpecialLoadInstruction(rd == SPARCRegisters.G0? "ld" : "ldx", 
                                                SPARCSpecialRegisters.FSR, -1,
                                                addr);
    }
}

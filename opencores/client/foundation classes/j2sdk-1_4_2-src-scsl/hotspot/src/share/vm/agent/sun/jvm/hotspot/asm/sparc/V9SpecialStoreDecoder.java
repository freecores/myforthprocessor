/*
 * @(#)V9SpecialStoreDecoder.java	1.2 03/01/23 11:22:18
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.asm.sparc;

import sun.jvm.hotspot.asm.*;

class V9SpecialStoreDecoder extends MemoryInstructionDecoder 
               implements V9InstructionDecoder {
    V9SpecialStoreDecoder(int op3) {
        super(op3, "st[x]fsr", RTLDT_UNKNOWN);
    }
    
    Instruction decodeMemoryInstruction(int instruction,
                               SPARCRegisterIndirectAddress addr,
                               SPARCRegister rd, SPARCInstructionFactory factory) {
        return factory.newSpecialStoreInstruction(rd == SPARCRegisters.G0? "st" : "stx", 
                                                SPARCSpecialRegisters.FSR, -1,
                                                addr);
    }
}

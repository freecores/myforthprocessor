/*
 * @(#)RettDecoder.java	1.2 03/01/23 11:16:49
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.asm.sparc;

import sun.jvm.hotspot.asm.*;

class RettDecoder extends MemoryInstructionDecoder {
    RettDecoder() {
        super(RETT, "rett", RTLDT_UNKNOWN);
    }

    Instruction decodeMemoryInstruction(int instruction, SPARCRegisterIndirectAddress addr,
                      SPARCRegister rd, SPARCInstructionFactory factory) {
        return factory.newRettInstruction(addr);
    }
}

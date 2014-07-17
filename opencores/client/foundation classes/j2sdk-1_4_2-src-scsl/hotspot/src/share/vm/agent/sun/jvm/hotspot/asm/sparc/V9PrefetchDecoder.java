/*
 * @(#)V9PrefetchDecoder.java	1.2 03/01/23 11:21:56
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.asm.sparc;

import sun.jvm.hotspot.asm.*;

class V9PrefetchDecoder extends MemoryInstructionDecoder 
           implements V9InstructionDecoder {
    V9PrefetchDecoder() {
        super(PREFETCH, "prefetch", RTLDT_UNKNOWN);
    }

    Instruction decodeMemoryInstruction(int instruction,
                                   SPARCRegisterIndirectAddress addr,
                                   SPARCRegister rd, SPARCInstructionFactory factory) {
        SPARCV9InstructionFactory v9factory = (SPARCV9InstructionFactory) factory;
        return v9factory.newV9PrefetchInstruction(name, addr, rd.getNumber());
    } 
}


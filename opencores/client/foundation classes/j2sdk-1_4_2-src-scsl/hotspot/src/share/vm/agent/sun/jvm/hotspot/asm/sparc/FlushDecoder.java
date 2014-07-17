/*
 * @(#)FlushDecoder.java	1.2 03/01/23 11:16:13
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.asm.sparc;

import sun.jvm.hotspot.asm.*;

class FlushDecoder extends MemoryInstructionDecoder {
    FlushDecoder() {
        super(FLUSH, "flush", RTLDT_UNKNOWN);
    }

    Instruction decodeMemoryInstruction(int instruction, SPARCRegisterIndirectAddress addr,
                      SPARCRegister rd, SPARCInstructionFactory factory) {
        return factory.newFlushInstruction(addr);
    }
}

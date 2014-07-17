/*
 * @(#)CallDecoder.java	1.2 03/01/23 11:15:50
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.asm.sparc;

import sun.jvm.hotspot.asm.*;

class CallDecoder extends InstructionDecoder {
    Instruction decode(int instruction, SPARCInstructionFactory factory) {
        // sign extend, word align the offset
        int offset = (instruction & DISP_30_MASK) << 2;
        return factory.newCallInstruction(new PCRelativeAddress(offset));
    }
}

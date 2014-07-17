/*
 * @(#)SaveDecoder.java	1.2 03/01/23 11:20:23
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.asm.sparc;

import sun.jvm.hotspot.asm.*;

class SaveDecoder extends Format3ADecoder {
    SaveDecoder() {
        super(SAVE, "save", RTLOP_UNKNOWN);
    }

    Instruction decodeFormat3AInstruction(int instruction,
                                       SPARCRegister rs1,
                                       ImmediateOrRegister operand2,
                                       SPARCRegister rd,
                                       SPARCInstructionFactory factory) {
        return factory.newSaveInstruction(rs1, operand2, rd);
    }
}

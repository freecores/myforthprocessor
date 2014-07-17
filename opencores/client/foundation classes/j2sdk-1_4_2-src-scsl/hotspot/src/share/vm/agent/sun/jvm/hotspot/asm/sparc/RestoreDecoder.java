/*
 * @(#)RestoreDecoder.java	1.2 03/01/23 11:16:47
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.asm.sparc;

import sun.jvm.hotspot.asm.*;

class RestoreDecoder extends Format3ADecoder {
    RestoreDecoder() {
        super(RESTORE, "restore", RTLOP_UNKNOWN);
    }

    Instruction decodeFormat3AInstruction(int instruction,
                                           SPARCRegister rs1,
                                           ImmediateOrRegister operand2,
                                           SPARCRegister rd,
                                           SPARCInstructionFactory factory) {
        return factory.newRestoreInstruction(rs1, operand2, rd);
    }
}

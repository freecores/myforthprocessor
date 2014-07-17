/*
 * @(#)IllegalInstructionDecoder.java	1.2 03/01/23 11:16:18
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.asm.sparc;

import sun.jvm.hotspot.asm.*;

class IllegalInstructionDecoder extends InstructionDecoder {
    Instruction decode(int instruction, SPARCInstructionFactory factory) {
        return factory.newIllegalInstruction(instruction);
    }
}

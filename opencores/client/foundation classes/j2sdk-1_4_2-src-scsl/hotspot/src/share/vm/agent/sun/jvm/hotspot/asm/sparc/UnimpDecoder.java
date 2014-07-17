/*
 * @(#)UnimpDecoder.java	1.2 03/01/23 11:20:47
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.asm.sparc;

import sun.jvm.hotspot.asm.*;

class UnimpDecoder extends InstructionDecoder {
    Instruction decode(int instruction, SPARCInstructionFactory factory) {
        return factory.newUnimpInstruction(instruction & DISP_22_MASK);
    }
}

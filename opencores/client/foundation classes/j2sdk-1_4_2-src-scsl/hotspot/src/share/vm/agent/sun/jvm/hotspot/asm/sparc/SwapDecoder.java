/*
 * @(#)SwapDecoder.java	1.2 03/01/23 11:20:43
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.asm.sparc;

import sun.jvm.hotspot.asm.*;

class SwapDecoder extends MemoryInstructionDecoder {
    SwapDecoder(int op3, String name, int dataType) {
        super(op3, name, dataType);
    }
   
    Instruction decodeMemoryInstruction(int instruction,
                                     SPARCRegisterIndirectAddress addr,
                                     SPARCRegister rd, 
                                     SPARCInstructionFactory factory) {
        return factory.newSwapInstruction(name, addr, rd);
    }
}


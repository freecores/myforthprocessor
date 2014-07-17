/*
 * @(#)AlternateSpaceSwapDecoder.java	1.2 03/01/23 11:15:43
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.asm.sparc;

import sun.jvm.hotspot.asm.*;

class AlternateSpaceSwapDecoder extends SwapDecoder {
    AlternateSpaceSwapDecoder(int op3, String name, int dataType) {
        super(op3, name, dataType);
    }
   
    Instruction decodeMemoryInstruction(int instruction,
                                     SPARCRegisterIndirectAddress addr,
                                     SPARCRegister rd, 
                                     SPARCInstructionFactory factory) {
        setAddressSpace(instruction, addr);        
        return factory.newSwapInstruction(name, addr, rd);
    }
}


/*
 * @(#)AlternateSpaceLoadDecoder.java	1.2 03/01/23 11:15:37
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.asm.sparc;

import sun.jvm.hotspot.asm.*;

class AlternateSpaceLoadDecoder extends LoadDecoder {
    AlternateSpaceLoadDecoder(int op3, String name, int dataType) {
        super(op3, name, dataType);
    }
   
    Instruction decodeMemoryInstruction(int instruction,
                                     SPARCRegisterIndirectAddress addr,
                                     SPARCRegister rd, 
                                     SPARCInstructionFactory factory) {
        setAddressSpace(instruction, addr);        
        return factory.newLoadInstruction(name, op3, addr, rd, dataType);
    }
}


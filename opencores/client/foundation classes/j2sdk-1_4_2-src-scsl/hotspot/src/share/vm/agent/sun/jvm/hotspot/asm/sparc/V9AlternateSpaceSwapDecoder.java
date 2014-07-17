/*
 * @(#)V9AlternateSpaceSwapDecoder.java	1.2 03/01/23 11:21:08
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.asm.sparc;

import sun.jvm.hotspot.asm.*;

class V9AlternateSpaceSwapDecoder extends V9AlternateSpaceDecoder {
    V9AlternateSpaceSwapDecoder(int op3, String name, int dataType) {
        super(op3, name, dataType);
    }

    Instruction decodeV9AsiLoadStore(int instruction,
                                     SPARCV9RegisterIndirectAddress addr,
                                     SPARCRegister rd,
                                     SPARCV9InstructionFactory factory) {
        return factory.newSwapInstruction(name, addr, rd);
    } 
}


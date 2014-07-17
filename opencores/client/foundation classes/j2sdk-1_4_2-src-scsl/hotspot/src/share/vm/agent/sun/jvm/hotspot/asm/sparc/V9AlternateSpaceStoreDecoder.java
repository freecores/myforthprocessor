/*
 * @(#)V9AlternateSpaceStoreDecoder.java	1.2 03/01/23 11:21:05
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.asm.sparc;

import sun.jvm.hotspot.asm.*;

class V9AlternateSpaceStoreDecoder extends V9AlternateSpaceDecoder {
    V9AlternateSpaceStoreDecoder(int op3, String name, int dataType) {
        super(op3, name, dataType);
    }

    Instruction decodeV9AsiLoadStore(int instruction,
                                     SPARCV9RegisterIndirectAddress addr,
                                     SPARCRegister rd,
                                     SPARCV9InstructionFactory factory) {
        return factory.newStoreInstruction(name, op3, addr, rd, dataType);
    }
}


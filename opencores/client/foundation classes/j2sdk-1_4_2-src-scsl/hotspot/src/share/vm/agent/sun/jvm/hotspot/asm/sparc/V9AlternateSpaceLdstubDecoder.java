/*
 * @(#)V9AlternateSpaceLdstubDecoder.java	1.2 03/01/23 11:20:58
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.asm.sparc;

import sun.jvm.hotspot.asm.*;

class V9AlternateSpaceLdstubDecoder extends V9AlternateSpaceDecoder {
    V9AlternateSpaceLdstubDecoder(int op3, String name, int dataType) {
        super(op3, name, dataType);
    }

    Instruction decodeV9AsiLoadStore(int instruction,
                                     SPARCV9RegisterIndirectAddress addr,
                                     SPARCRegister rd,
                                     SPARCV9InstructionFactory factory) {
        return factory.newLdstubInstruction(name, addr, rd);
    } 
}


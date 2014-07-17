/*
 * @(#)V9AlternateSpacePrefetchDecoder.java	1.2 03/01/23 11:21:03
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.asm.sparc;

import sun.jvm.hotspot.asm.*;

class V9AlternateSpacePrefetchDecoder extends V9AlternateSpaceDecoder {
    V9AlternateSpacePrefetchDecoder() {
        super(PREFETCHA, "prefetcha", RTLDT_UNKNOWN);
    }

    Instruction decodeV9AsiLoadStore(int instruction,
                                     SPARCV9RegisterIndirectAddress addr,
                                     SPARCRegister rd,
                                     SPARCV9InstructionFactory factory) {
        SPARCV9InstructionFactory v9factory = (SPARCV9InstructionFactory) factory;
        return v9factory.newV9PrefetchInstruction(name, addr, rd.getNumber());
    } 
}


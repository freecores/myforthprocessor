/*
 * @(#)SpecialLoadDecoder.java	1.2 03/01/23 11:20:31
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.asm.sparc;

import sun.jvm.hotspot.asm.*;

class SpecialLoadDecoder extends SpecialLoadStoreDecoder {
    SpecialLoadDecoder(int op3, String name, int specialRegNum) {
        super(op3, name, specialRegNum);
    }

    Instruction decodeSpecialLoadStoreInstruction(int cregNum, 
                                    SPARCRegisterIndirectAddress addr,
                                    SPARCInstructionFactory factory) {
        return factory.newSpecialLoadInstruction(name, specialRegNum, cregNum, addr);
    }
}


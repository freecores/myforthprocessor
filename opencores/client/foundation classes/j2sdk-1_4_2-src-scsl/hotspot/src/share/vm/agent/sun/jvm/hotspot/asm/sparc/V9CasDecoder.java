/*
 * @(#)V9CasDecoder.java	1.2 03/01/23 11:21:20
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.asm.sparc;

import sun.jvm.hotspot.asm.*;

class V9CasDecoder extends V9AlternateSpaceDecoder {
    V9CasDecoder(int op3, String name, int dataType) {
        super(op3, name, dataType);
    }

    Instruction decodeV9AsiLoadStore(int instruction,
                                     SPARCV9RegisterIndirectAddress addr,
                                     SPARCRegister rd,
                                     SPARCV9InstructionFactory factory) {
        SPARCV9InstructionFactory v9factory = (SPARCV9InstructionFactory) factory;
        SPARCRegister rs2 = SPARCRegisters.getRegister(getSourceRegister2(instruction));
        return v9factory.newV9CasInstruction(name, addr, rs2, rd, dataType);
    } 
}


/*
 * @(#)JmplDecoder.java	1.2 03/01/23 11:16:26
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.asm.sparc;

import sun.jvm.hotspot.asm.*;

class JmplDecoder extends MemoryInstructionDecoder {
    JmplDecoder() {
        super(JMPL, "jmpl", RTLDT_UNSIGNED_WORD);
    }

    Instruction decodeMemoryInstruction(int instruction, SPARCRegisterIndirectAddress addr,
                       SPARCRegister rd,  SPARCInstructionFactory factory) {
        // this may be most probably indirect call or ret or retl
        Instruction instr = null;
        if (rd == SPARCRegisters.O7) {
            instr = factory.newIndirectCallInstruction(addr, rd);
        } else if (rd == SPARCRegisters.G0) {
            int disp = (int) addr.getDisplacement();
            Register base = addr.getBase();
            if (base == SPARCRegisters.I7 && disp == 8) {
                instr = factory.newReturnInstruction(addr, rd, false /* not leaf */);
            } else if (base == SPARCRegisters.O7 && disp == 8) {
                instr = factory.newReturnInstruction(addr, rd, true /* leaf */);
            } else {
                instr = factory.newJmplInstruction(addr, rd);
            }
        } else {
            instr = factory.newJmplInstruction(addr, rd);
        }
        return instr;
    }
}

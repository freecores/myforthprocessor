/*
 * @(#)TrapDecoder.java	1.2 03/01/23 11:20:45
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.asm.sparc;

import sun.jvm.hotspot.asm.*;

class TrapDecoder extends InstructionDecoder {
    private static final String trapConditionNames[] = {
        "tn", "te", "tle", "tl", "tleu", "tcs", "tneg", "tvs",
        "ta", "tne", "tg", "tge", "tgu" , "tcc", "tpos", "tvc"
    };

    static String getTrapConditionName(int index) {
        return trapConditionNames[index];
    }

    Instruction decode(int instruction, SPARCInstructionFactory factory) {
        int conditionCode = getConditionCode(instruction);
        return factory.newTrapInstruction(getTrapConditionName(conditionCode),
                                                     conditionCode);
    }
}

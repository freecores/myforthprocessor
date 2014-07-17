/*
 * @(#)V9IntRegisterBranchDecoder.java	1.2 03/01/23 11:21:43
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.asm.sparc;

import sun.jvm.hotspot.asm.*;

class V9IntRegisterBranchDecoder extends V9RegisterBranchDecoder {
    static final String integerRegisterConditionNames[] = {
        null, "brz", "brlez", "brlz", null, "brnz", "brgz", "brgez"
    };

    String getRegisterConditionName(int index) {
        return integerRegisterConditionNames[index];
    }
}

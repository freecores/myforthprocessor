/*
 * @(#)FloatBranchDecoder.java	1.2 03/01/23 11:16:08
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.asm.sparc;

class FloatBranchDecoder extends BranchDecoder {
    String getConditionName(int conditionCode, boolean isAnnuled) {
        return isAnnuled ? floatAnnuledConditionNames[conditionCode]
                         : floatConditionNames[conditionCode];
    }
}

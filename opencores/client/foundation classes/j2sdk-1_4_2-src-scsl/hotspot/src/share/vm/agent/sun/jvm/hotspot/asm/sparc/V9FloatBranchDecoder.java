/*
 * @(#)V9FloatBranchDecoder.java	1.2 03/01/23 11:21:35
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.asm.sparc;

class V9FloatBranchDecoder extends V9CCBranchDecoder {
    String getConditionName(int conditionCode, boolean isAnnuled) {
        return isAnnuled ? floatAnnuledConditionNames[conditionCode]
                         : floatConditionNames[conditionCode];
    }

    int getConditionFlag(int instruction) {
        return (FBPfcc_CC_MASK & instruction) >>> FBPfcc_CC_START_BIT;
    }
}

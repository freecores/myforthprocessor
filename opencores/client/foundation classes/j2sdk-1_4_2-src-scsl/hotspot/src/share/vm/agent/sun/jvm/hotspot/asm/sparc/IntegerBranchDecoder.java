/*
 * @(#)IntegerBranchDecoder.java	1.2 03/01/23 11:16:23
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.asm.sparc;

class IntegerBranchDecoder extends BranchDecoder {
    String getConditionName(int conditionCode, boolean isAnnuled) {
        return isAnnuled ? integerAnnuledConditionNames[conditionCode]
                         : integerConditionNames[conditionCode];
    }
}  

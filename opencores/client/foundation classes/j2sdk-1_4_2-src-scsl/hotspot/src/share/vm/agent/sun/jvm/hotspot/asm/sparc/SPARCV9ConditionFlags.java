/*
 * @(#)SPARCV9ConditionFlags.java	1.2 03/01/23 11:18:58
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.asm.sparc;

class SPARCV9ConditionFlags {
    private static final String ccFlagNames[] = {
        "%fcc0", "%fcc1", "%fcc2", "%fcc3", "%icc", null, "%xcc", null
    };

    public static String getFlagName(int index) {
        return ccFlagNames[index];
    }
}

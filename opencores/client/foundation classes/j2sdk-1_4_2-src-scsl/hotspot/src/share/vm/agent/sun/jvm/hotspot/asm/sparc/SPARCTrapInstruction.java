/*
 * @(#)SPARCTrapInstruction.java	1.2 03/01/23 11:18:45
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.asm.sparc;

import sun.jvm.hotspot.asm.*;

public class SPARCTrapInstruction extends SPARCInstruction
    implements BranchInstruction {
    final protected int conditionCode;

    public SPARCTrapInstruction(String name, int conditionCode) {
        super(name);
        this.conditionCode = conditionCode;
    }

    public Address getBranchDestination() {
        return null;
    }

    public int getConditionCode() {
        return conditionCode;
    }

    public boolean isConditional() {
        return conditionCode != CONDITION_TN && conditionCode != CONDITION_TA;
    }

    public boolean isTrap() {
        return true;
    }
}

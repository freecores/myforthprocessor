/*
 * @(#)SPARCRestoreInstruction.java	1.3 03/01/23 11:18:13
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.asm.sparc;

import sun.jvm.hotspot.asm.*;

public class SPARCRestoreInstruction extends SPARCFormat3AInstruction {
    final private boolean trivial;
    public SPARCRestoreInstruction(SPARCRegister rs1, ImmediateOrRegister operand2, SPARCRegister rd) {
        super("restore", RESTORE, rs1, operand2, rd);
        SPARCRegister G0 = SPARCRegisters.G0;
        trivial = (rs1 == G0 && operand2 == G0 && rd == G0);
    }

    public boolean isTrivial() {
        return trivial;
    }

    protected String getDescription() {
        return (trivial) ? getName() : super.getDescription();
    }
}

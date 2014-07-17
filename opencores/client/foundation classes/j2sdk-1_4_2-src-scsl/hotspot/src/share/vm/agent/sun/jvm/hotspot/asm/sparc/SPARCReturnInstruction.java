/*
 * @(#)SPARCReturnInstruction.java	1.3 03/01/23 11:18:18
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.asm.sparc;

import sun.jvm.hotspot.asm.*;

public class SPARCReturnInstruction extends SPARCJmplInstruction
    implements ReturnInstruction {

    private final boolean leaf;

    public SPARCReturnInstruction(SPARCRegisterIndirectAddress addr, SPARCRegister rd, boolean leaf) {
        super(leaf? "retl" : "ret", addr, rd);
        this.leaf = leaf;
    }

    public boolean isLeaf() {
        return leaf;
    }

    protected String getDescription() {
        return getName();
    }
}

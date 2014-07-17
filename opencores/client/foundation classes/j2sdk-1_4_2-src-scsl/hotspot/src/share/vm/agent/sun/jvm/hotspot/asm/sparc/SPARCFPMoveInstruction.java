/*
 * @(#)SPARCFPMoveInstruction.java	1.2 03/01/23 11:17:11
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.asm.sparc;

import sun.jvm.hotspot.asm.*;

public class SPARCFPMoveInstruction extends SPARCFP2RegisterInstruction
    implements MoveInstruction {

    public SPARCFPMoveInstruction(String name, int opf, SPARCFloatRegister rs, SPARCFloatRegister rd) {
        super(name, opf, rs, rd);
    }

    public Register getMoveDestination() {
        return rd;
    }

    public ImmediateOrRegister getMoveSource() {
        return rs;
    }

    public int getMoveOpcode() {
        return opf;
    }

    public boolean isConditional() {
        return false;
    }

    public boolean isMove() {
        return true;
    }
}

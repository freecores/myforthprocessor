/*
 * @(#)SPARCV9PopcInstruction.java	1.3 03/01/23 11:19:35
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.asm.sparc;

import sun.jvm.hotspot.asm.*;

public class SPARCV9PopcInstruction extends SPARCFormat3AInstruction 
                  implements SPARCV9Instruction {
    public SPARCV9PopcInstruction(ImmediateOrRegister source, SPARCRegister rd) {
        super("popc", POPC, null, source, rd);
    }

    protected String getDescription() {
        StringBuffer buf = new StringBuffer();
        buf.append(getName());
        buf.append(spaces);
        buf.append(getOperand2String());
        buf.append(comma);
        buf.append(rd.toString());
        return buf.toString();
    }

    public ImmediateOrRegister getSource() {
        return operand2;
    }

    public SPARCRegister getDestination() {
        return rd;
    }
}

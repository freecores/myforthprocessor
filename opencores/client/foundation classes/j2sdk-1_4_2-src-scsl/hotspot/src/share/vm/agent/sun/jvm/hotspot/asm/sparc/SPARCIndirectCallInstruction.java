/*
 * @(#)SPARCIndirectCallInstruction.java	1.3 03/01/23 11:17:30
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.asm.sparc;

import sun.jvm.hotspot.asm.*;

public class SPARCIndirectCallInstruction extends SPARCJmplInstruction
    implements CallInstruction {

    public SPARCIndirectCallInstruction(SPARCRegisterIndirectAddress addr, SPARCRegister rd) {
        super("call", addr, rd);
    }

    protected String getDescription() {
        StringBuffer buf = new StringBuffer();
        buf.append(getName());
        buf.append(spaces);
        // remove '[' & ']' from jmp address
        String addrStr = addr.toString();
        buf.append(addrStr.substring(1, addrStr.length() - 1));
        return buf.toString();
    }
}

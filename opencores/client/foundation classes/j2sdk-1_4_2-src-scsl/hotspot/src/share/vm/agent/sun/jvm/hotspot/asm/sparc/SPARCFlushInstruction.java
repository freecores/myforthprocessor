/*
 * @(#)SPARCFlushInstruction.java	1.2 03/01/23 11:17:18
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.asm.sparc;

import sun.jvm.hotspot.asm.*;

public class SPARCFlushInstruction extends SPARCInstruction {
    final protected SPARCRegisterIndirectAddress addr;
    final String description;

    public SPARCFlushInstruction(SPARCRegisterIndirectAddress addr) {
        super("flush");
        this.addr = addr;
        description = initDescription();
    }

    private String initDescription() {
        StringBuffer buf = new StringBuffer();
        buf.append(getName());
        buf.append(spaces);
        buf.append(addr.toString());
        return buf.toString();
    }

    public String asString(long currentPc, SymbolFinder symFinder) {
        return description;
    }
}

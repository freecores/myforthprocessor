/*
 * @(#)SPARCInstruction.java	1.2 03/01/23 11:17:32
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.asm.sparc;

import sun.jvm.hotspot.asm.*;

public abstract class SPARCInstruction 
                      extends AbstractInstruction
                      implements /* imports */ SPARCOpcodes {
    public SPARCInstruction(String name) {
        super(name);
    }

    public int getSize() {
        return 4;
    }

    protected static String comma = ", ";
    protected static String spaces = "\t";
}

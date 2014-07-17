/*
 * @(#)SPARCNoopInstruction.java	1.2 03/01/23 11:17:54
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.asm.sparc;

import sun.jvm.hotspot.asm.*;

public class SPARCNoopInstruction extends SPARCInstruction {
    public SPARCNoopInstruction() {
        super("nop");
    }

    public boolean isNoop() {
        return true;
    }
}

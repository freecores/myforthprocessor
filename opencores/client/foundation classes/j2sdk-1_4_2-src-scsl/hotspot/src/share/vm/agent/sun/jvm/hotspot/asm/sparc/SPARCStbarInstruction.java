/*
 * @(#)SPARCStbarInstruction.java	1.2 03/01/23 11:18:38
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.asm.sparc;

import sun.jvm.hotspot.asm.SymbolFinder;

public class SPARCStbarInstruction extends SPARCInstruction {
    public SPARCStbarInstruction() {
        super("stbar");
    }
}

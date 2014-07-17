/*
 * @(#)SPARCV9IlltrapInstruction.java	1.2 03/01/23 11:19:13
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.asm.sparc;

import sun.jvm.hotspot.asm.*;

public class SPARCV9IlltrapInstruction extends SPARCUnimpInstruction 
                    implements SPARCV9Instruction {
    public SPARCV9IlltrapInstruction(int const22) {
        super("illtrap", const22);
    }
}

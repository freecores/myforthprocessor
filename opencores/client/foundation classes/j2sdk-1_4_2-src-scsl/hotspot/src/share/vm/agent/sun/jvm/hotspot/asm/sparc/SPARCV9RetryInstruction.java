/*
 * @(#)SPARCV9RetryInstruction.java	1.2 03/01/23 11:19:58
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.asm.sparc;

import sun.jvm.hotspot.asm.*;

public class SPARCV9RetryInstruction extends SPARCInstruction 
                    implements SPARCV9Instruction {
    public SPARCV9RetryInstruction() {
        super("retry");
    }
}

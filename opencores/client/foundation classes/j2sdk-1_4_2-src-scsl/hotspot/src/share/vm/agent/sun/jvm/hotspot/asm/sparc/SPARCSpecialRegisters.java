/*
 * @(#)SPARCSpecialRegisters.java	1.2 03/01/23 11:18:33
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.asm.sparc;

public interface SPARCSpecialRegisters {
    public static final int Y    = 0;
    public static final int PSR  = 1;
    public static final int WIM  = 2;
    public static final int TBR  = 3;
    public static final int ASR  = 4;
    public static final int FSR  = 5;
    public static final int CSR  = 6;
    public static final int FQ   = 7;
    public static final int CQ   = 8;
    public static final int CREG = 9; // co-processor reg
}

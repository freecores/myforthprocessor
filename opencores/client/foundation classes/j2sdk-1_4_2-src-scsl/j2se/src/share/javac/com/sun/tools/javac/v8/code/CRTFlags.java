/**
 * @(#)CRTFlags.java	1.4 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.tools.javac.v8.code;

/**
 * The CharacterRangeTable flags indicating type of an entry.
 */
public interface CRTFlags {

    /**
     * CRTEntry flags.
     */
    public static final int CRT_STATEMENT = 1;
    public static final int CRT_BLOCK = 2;
    public static final int CRT_ASSIGNMENT = 4;
    public static final int CRT_FLOW_CONTROLLER = 8;
    public static final int CRT_FLOW_TARGET = 16;
    public static final int CRT_INVOKE = 32;
    public static final int CRT_CREATE = 64;
    public static final int CRT_BRANCH_TRUE = 128;
    public static final int CRT_BRANCH_FALSE = 256;

    /**
     * The mask for valid flags
     */
    public static final int CRT_VALID_FLAGS =
            CRT_STATEMENT | CRT_BLOCK | CRT_ASSIGNMENT | CRT_FLOW_CONTROLLER |
            CRT_FLOW_TARGET | CRT_INVOKE | CRT_CREATE | CRT_BRANCH_TRUE |
            CRT_BRANCH_FALSE;
}

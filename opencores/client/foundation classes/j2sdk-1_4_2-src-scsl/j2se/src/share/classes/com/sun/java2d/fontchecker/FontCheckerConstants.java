/*
 * @(#)FontCheckerConstants.java	1.2 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 * 
 * <PRE>
 * FontCheckerConstants contains constants shared between
 * FontChecker and FontCheckDummy classes. 
 *
 * </PRE>
 *
 * @author Ilya Bagrak
 */

package com.sun.java2d.fontchecker;

public interface FontCheckerConstants {

    /* error codes returned from child process */ 
    public static final int ERR_FONT_OK         = 65; 
    public static final int ERR_FONT_NOT_FOUND  = 60; 
    public static final int ERR_FONT_BAD_FORMAT = 61; 
    public static final int ERR_FONT_READ_EXCPT = 62; 
    public static final int ERR_FONT_DISPLAY    = 64; 
    public static final int ERR_FONT_EOS        = -1;
    /* nl char sent after child crashes */
    public static final int ERR_FONT_CRASH      = 10; 
}

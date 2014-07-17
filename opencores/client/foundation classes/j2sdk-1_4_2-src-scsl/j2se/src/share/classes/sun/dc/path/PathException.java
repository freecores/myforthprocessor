/*
 * @(#)PathException.java	1.7 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * DO NOT EDIT THIS FILE - it is automatically generated
 *
 * PathException.java		Tue Nov 18 16:15:07 PST 1997
 *
 * ---------------------------------------------------------------------
 *	Copyright (c) 1996-1997 by Ductus, Inc. All Rights Reserved.
 * ---------------------------------------------------------------------
 *
 */

package sun.dc.path;

public class PathException extends java.lang.Exception
{

    public static final String
	BAD_PATH_endPath = "endPath: bad path",
	BAD_PATH_useProxy = "useProxy: bad path",
	DUMMY = "";

    // Constructors
    public PathException() {
	super();
    }

    public PathException(String s) {
	super(s);
    }
}

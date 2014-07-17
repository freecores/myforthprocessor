/*
 * @(#)CouldNotLoadArgumentException.java	1.6 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.exceptions;
import com.sun.javaws.Resources;

public class CouldNotLoadArgumentException extends JNLPException {
    private String _argument;
    
    public CouldNotLoadArgumentException(String arg, Exception ioe) {
	super(Resources.getString("launch.error.category.arguments"), ioe);
	_argument = arg;
    }
    
    /** Returns message */
    public String getRealMessage() {
	return Resources.getString("launch.error.couldnotloadarg", _argument);
    }
    
    /** Returns the name of the offending field */
    public String getField() { return getMessage(); }
    
    /** toString implementation */
    public String toString() { return "CouldNotLoadArgumentException[ " + getRealMessage() + "]"; }
}



/*
 * @(#)JreExecException.java	1.7 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.exceptions;
import com.sun.javaws.jnl.LaunchDesc;
import com.sun.javaws.Resources;

/**
 * Exception thrown if an exec. of a JRE failed
 *
 */
public class JreExecException extends JNLPException {
    private String _version;
    
    public JreExecException(String version, Exception e) {
	super(Resources.getString("launch.error.category.unexpected"), e);
	_version = version;
    }
    
    public String getRealMessage() {
	return Resources.getString("launch.error.failedexec", _version);
    }
        
    public String toString() {
	return "JreExecException[ " + getMessage() + "]"; };
}



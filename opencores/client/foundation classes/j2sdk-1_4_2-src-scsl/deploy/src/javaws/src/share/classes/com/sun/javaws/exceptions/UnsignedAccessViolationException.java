/*
 * @(#)UnsignedAccessViolationException.java	1.7 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.exceptions;
import java.net.URL;
import com.sun.javaws.Resources;
import com.sun.javaws.jnl.LaunchDesc;

public class UnsignedAccessViolationException extends JNLPException {

    URL _url;
    boolean _initial;
    
    public UnsignedAccessViolationException(LaunchDesc ld, URL url, boolean initial) {
        super(Resources.getString("launch.error.category.security"), ld);
	_url = url;
	_initial = initial;
    }
    
    /** Returns message */
    public String getRealMessage() {
        return (Resources.getString("launch.error.unsignedAccessViolation") + 
		"\n" +Resources.getString("launch.error.unsignedResource",_url.toString()));
    }

    /** 
     *  can override brief message 
     */
     public String getBriefMessage() {
	if (_initial) {
	    return null;
	}
	return Resources.getString("launcherrordialog.brief.continue");
    }
}



/*
 * %W% %E%
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.exceptions;
import com.sun.javaws.Resources;

public class MissingFieldException extends LaunchDescException {
    private String  _field;
    private String _launchDescSource; // Reference to JNLP file contents
    
    public MissingFieldException(String source, String field) {
	super();
	_field = field;
	_launchDescSource = source;
    }
    
    /** Returns message */
    public String getRealMessage() {
	if (!isSignedLaunchDesc()) {
	    return Resources.getString("launch.error.missingfield", _field);
	} else {
	    return Resources.getString("launch.error.missingfield-signedjnlp", _field);
	}
    }
    
    /** Returns the name of the offending field */
    public String getField() { return getMessage(); }
    
    /** Overwrite this to return the source */
    public String getLaunchDescSource() {
	return _launchDescSource;
	
    }
    
    /** toString implementation */
    public String toString() { return "MissingFieldException[ " + getField() + "]"; }
}



/*
 * %W% %E%
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.exceptions;
import com.sun.javaws.Resources;

public class BadFieldException extends LaunchDescException {
    private String  _field;
    private String  _value;
    private String _launchDescSource;  // Reference to JNLP file contents
    
    public BadFieldException(String source, String field, String value) {
	super();
	_value = value;
	_field = field;
	_launchDescSource = source;
    }
    
    /** Returns the name of the offending field */
    public String getField() { return getMessage(); }
    
    /** Returns the value of the offending field */
    public String getValue() { return _value; }
            
    /** Returns message */
    public String getRealMessage() {
	if (getValue().equals("https")) {
	    return Resources.getString("launch.error.badfield", _field, _value) + "\n" + Resources.getString("launch.error.badfield.https");
	} else if (!isSignedLaunchDesc()) {
	    return Resources.getString("launch.error.badfield", _field, _value);
	} else {
	    return Resources.getString("launch.error.badfield-signedjnlp", _field, _value);
	}
    }
    
    /** Overwrite this to return the source */
    public String getLaunchDescSource() {
	return _launchDescSource;
	
    }
    
    /** toString implementation */
    public String toString() { 
	if (getValue().equals("https")) {
	    return "BadFieldException[ " + getRealMessage() + "]"; 
	}
	return "BadFieldException[ " + getField() + "," + getValue() + "]"; 
    }
}



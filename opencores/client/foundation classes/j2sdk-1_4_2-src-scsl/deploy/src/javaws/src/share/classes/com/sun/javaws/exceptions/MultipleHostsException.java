/*
 * @(#)MultipleHostsException.java	1.6 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.exceptions;
import com.sun.javaws.Resources;

public class MultipleHostsException extends JNLPException {
    
    public MultipleHostsException() {
        super(Resources.getString("launch.error.category.security"));
    }
    
    /** Returns message */
    public String getRealMessage() { return Resources.getString("launch.error.multiplehostsreferences");
    }
}



/*
 * @(#)OfflineLaunchException.java	1.4 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.exceptions;
import com.sun.javaws.Resources;

public class OfflineLaunchException extends JNLPException {
    
    public OfflineLaunchException() {
        super(Resources.getString("launch.error.category.download"));
    }
    
    /** Returns message */
    public String getRealMessage() { return Resources.getString("launch.error.offlinemissingresource");
    }
}



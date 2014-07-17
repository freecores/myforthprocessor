/*
 * @(#)BadJARFileException.java	1.6 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.exceptions;
import java.net.URL;
import com.sun.javaws.Resources;

/**
 *  This is to wrap an exception creating a JAR file. This will typically
 *  wrap a java.util.ZipException
 */

public class BadJARFileException extends DownloadException {
    
    public BadJARFileException (URL location, String version, Exception e) {
        super(null, location, version, e);
    }
    
    /** Returns the message */
    public String getRealMessage() {
        return Resources.getString("launch.error.badjarfile", getResourceString());
    }
}


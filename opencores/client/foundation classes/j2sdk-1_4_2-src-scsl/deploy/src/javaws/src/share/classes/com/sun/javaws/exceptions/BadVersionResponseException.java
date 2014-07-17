/*
 * @(#)BadVersionResponseException.java	1.6 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.exceptions;
import java.net.URL;
import com.sun.javaws.jnl.LaunchDesc;
import com.sun.javaws.jnl.LaunchDesc;
import com.sun.javaws.Resources;

/** Root exception for all exceptions that releates
 *  to download resources
 *
 *  It's main thing is to set the category for this
 *  kind of exception.
 */

public class BadVersionResponseException extends DownloadException {
    private String _responseVersionID;
    
    /** Creates an exception */
    public BadVersionResponseException(URL location, String versionID, String responseVersionID) {
        super(location, versionID);
        _responseVersionID = responseVersionID;
    }
    
    /** Returns the message */
    public String getRealMessage() {
        return Resources.getString("launch.error.badversionresponse", getResourceString(), _responseVersionID);
    }
}



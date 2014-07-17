/*
 * @(#)FailedDownloadingResourceException.java	1.7 03/01/23
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

public class FailedDownloadingResourceException extends DownloadException {
    /** Creates an exception */
    public FailedDownloadingResourceException(LaunchDesc ld, URL location, String versionID, Exception e) {
        super(ld, location, versionID, e);
    }
    
    /** Creates an exception */
    public FailedDownloadingResourceException(URL location, String versionID, Exception e) {
        this(null, location, versionID, e);
    }
    
    /** Returns the message */
    public String getRealMessage() {
        return Resources.getString("launch.error.failedloadingresource", getResourceString());
    }
}



/*
 * @(#)CanceledDownloadException.java	1.3 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.exceptions;
import java.net.URL;
import com.sun.javaws.jnl.LaunchDesc;
import com.sun.javaws.jnl.LaunchDesc;
import com.sun.javaws.Resources;

/** An exception indicated that the download was
 *  cancled by the user
 *
 */
public class CanceledDownloadException extends DownloadException {
    /** Creates an exception */
    public CanceledDownloadException(URL location, String versionID) {
        super(location, versionID);
    }
    
    /** Returns the message */
    public String getRealMessage() {
        return Resources.getString("launch.error.canceledloadingresource", getResourceString());
    }
}



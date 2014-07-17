/*
 * %W% %E%
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws;
import java.net.URL;
import com.sun.javaws.debug.Globals;
import java.security.SecureRandom;

/**
 * The class encapsulates the methods for interacting
 * with a browser on the native platform.
 * Currently, this means getting it to show a specific URL
 *
 */

public abstract class BrowserSupport {
    
    private static BrowserSupport _browserSupportImplementation = null;
    
    public synchronized static BrowserSupport getInstance() {
        if (_browserSupportImplementation  == null) {
	    // Platform-depenendent Browser Lookup
	    _browserSupportImplementation  = BrowserSupportFactory.newInstance();
        }
        return _browserSupportImplementation;
    }
    
    /** All our current supported platforms supports web-browsers */
    static public boolean isWebBrowserSupported() {
	return getInstance().isWebBrowserSupportedImpl();
    }
    
    /** Instructs the native browser to show a specific URL
     *
     * @return If the operation succeeded
     */
    static public boolean showDocument(URL url) {
	return getInstance().showDocumentImpl(url);
    }
    

    static public SecureRandom getSecureRandom() {
	return getInstance().getSecureRandomImpl();
    }

    /** Platform dependent */
    public abstract boolean isWebBrowserSupportedImpl();
    public abstract boolean showDocumentImpl(URL url);
    public abstract String getNS6MailCapInfo();
    public abstract SecureRandom getSecureRandomImpl();
}



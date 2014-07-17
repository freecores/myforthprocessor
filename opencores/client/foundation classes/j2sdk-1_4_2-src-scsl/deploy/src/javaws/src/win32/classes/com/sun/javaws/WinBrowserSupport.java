/*
 * @(#)WinBrowserSupport.java	1.6 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws;
import java.net.URL;
import com.sun.javaws.debug.Globals;
import java.security.SecureRandom;
import java.security.Provider;
import java.security.Security;
import com.sun.javaws.security.WSecureRandom;

/** Concrete implementation of the BrowserSupport class for Windows */
public class WinBrowserSupport extends BrowserSupport  {
    
    // Load native dll
    static {
	NativeLibrary.getInstance().load();
    }
            
    /** Platform dependent */
    // no mailcap file for NS6 on win32
    public String getNS6MailCapInfo() { return null; }    
    public boolean isWebBrowserSupportedImpl() { return true; }
    public boolean showDocumentImpl(URL url)   {
	if (url == null) return false;
	return showDocument(url.toString());
    }

    public SecureRandom getSecureRandomImpl() {
	// On Windows, MS has provided Cryto APIs to generate seed
	// for secure random generator, and it is way faster than 
	// the JDK implementation.

	// Obtain Sun service provider
	Provider provider = Security.getProvider("SUN");

	// Reset secure random support
	provider.put("SecureRandom.SHA1PRNG", "com.sun.javaws.security.WSecureRandom");	    

	return new SecureRandom();
    }      

    // Native method that does all the work
    static native boolean showDocument(String url);
}

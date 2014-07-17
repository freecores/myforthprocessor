/*
 * @(#)UnixBrowserSupport.java	1.7 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws;
import java.net.URL;
import com.sun.javaws.debug.Globals;
import java.io.*;
import java.security.SecureRandom;

/** Concrete implementation of the BrowserSupport class for Windows */
public class UnixBrowserSupport extends BrowserSupport  {
    
    /** Platform dependent */
    // mailcap info for NS6
    public String getNS6MailCapInfo() {
	return  "user_pref(\"helpers.private_mailcap_file\", \"" + 
           System.getProperty("user.home") + 
           "/.mailcap\");\nuser_pref(\"helpers.private_mime_types_file\", \"" +           System.getProperty("user.home") + "/.mime.types\");\n";
    }

    /** Platform dependent */
    public boolean isWebBrowserSupportedImpl() { return true; }


    public SecureRandom getSecureRandomImpl() {
	// On Unix, some OSes (e.g. Linux) has provided /dev/urandom 
	// to generate seed for secure random generator, and it is way 
	// faster than the JDK implementation.

	try
	{
	    // Check if OS level secure random generator exists.
	    // If so, use it!!
	    //
	    File randomFile = new File("/dev/urandom");

	    if (randomFile != null && randomFile.exists())
	    {
		java.security.Security.setProperty("securerandom.source", "file:/dev/urandom");
		
	    }
	}
	catch (Throwable e)
	{
	    //    
	}

	return new SecureRandom();
    }

    /** This code is heavily inspired by the JavaWorld article:
     * http://www.javaworld.com/javaworld/javatips/jw-javatip66.html
     */
    public boolean showDocumentImpl(URL url)   {
	try {          
            ConfigProperties cp = ConfigProperties.getInstance();
            
            String browserPath = cp.getBrowserPath();
            // Netscape has a special option for launch a page, if netscape is
            // already running.
            if (cp.isBrowserNetscape()) {
                // Under Unix/Netscape has to be running for the "-remote"
                // command to work. So, we try sending the command and
                // check for an exit value. If the exit command is 0,
                // cmd = netscape -remote 'openURL(http://www.javaworld.com)'
		// fix for 4481188
		String[] cmd = new String[4];
		cmd[0] = browserPath;
		cmd[1] = "-raise";
		cmd[2] = "-remote";
		cmd[3] = "openURL(" + url.toString() + ")";
                
                Process p = Runtime.getRuntime().exec(cmd);
		InputStream er = p.getErrorStream();
		byte[] b = new byte[1024];
                try {
                    // wait for exit code -- if it's 0, command worked,
                    // otherwise we need to start the browser up.
                    int exitCode = p.waitFor();
		    // there should be no error msg if it succeed
		    int ret = er.read(b, 0, 1024); 

                    if (exitCode == 0 && ret == -1) {		
			return true;
		    }
                } catch(InterruptedException x) {
                    // Ignore and try without special netscape flag
                }
            }
            
            // Try normal browser startup
            // e.g., cmd = 'netscape http://www.javaworld.com'
	    // fix for 4481188
            String[] cmd = new String[2];
	    cmd[0] = browserPath;
	    cmd[1] = url.toString();

            Process p = Runtime.getRuntime().exec(cmd);
            if (p == null) return false;
        } catch(java.io.IOException x) {
            return false;
        }
        return true;
    }
}

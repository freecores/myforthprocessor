/*
 * @(#)UnixInternetProxy.java	1.9 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.proxy;
import java.io.*;
import com.sun.javaws.debug.*;

public class UnixInternetProxy extends InternetProxy
{

    // this works with both NS 6 and 7 since they use
    // the same prefs.js file
    public File getNS6RegFile() {
	// Platform dependent check
	String homeDir = null; 
	try { 
	    homeDir = System.getProperty("user.home");
	}
	catch (SecurityException e) {
	    e.printStackTrace();
	}

	// first check for netscape 6.x

	String ns6Dir = homeDir + File.separator + ".mozilla";

	return new File(ns6Dir + File.separator + "appreg");
	
    }


    /** 
     * Return all of the available information about the Netscape internet 
     * proxy addresses.
     */
    public InternetProxyInfo getBrowserInfo() 
    {
	float version = -1;

        // Check platform indepenent setup
	InternetProxyInfo info = new InternetProxyInfo();
 
        // Platform dependent check
	File regFile = getNS6RegFile();

	File prefsFile = null;

	if (regFile != null) {
	   
	    prefsFile  = getNS6PrefsFile(regFile);
	  
	    if (prefsFile != null) {
		version = 6;
		NSPreferences.parseFile(prefsFile, info, version);
		return info;	    
	    }	 	   
	}

	// then check for netscape 4.x
	prefsFile = new File(System.getProperty("user.home") + "/.netscape/preferences.js");
	version = 4;
	NSPreferences.parseFile(prefsFile, info, version);

	return info;
    }
}

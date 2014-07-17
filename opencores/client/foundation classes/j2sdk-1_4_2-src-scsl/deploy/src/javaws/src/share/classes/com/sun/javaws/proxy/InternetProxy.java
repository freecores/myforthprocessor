/*
 * @(#)InternetProxy.java	1.12 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.proxy;
import com.sun.javaws.ConfigProperties;
import java.util.List;
import java.io.File;

/**
 *   Factory for the Internet proxy object
 *
 *   @version 1.3, 03/20/00
 */
public abstract class InternetProxy
{
    private static InternetProxy _internetProxy = null;
    
    public synchronized static InternetProxy getInstance() {
        if (_internetProxy == null) {
	    // Platform-depenendent Browser Lookup
	    _internetProxy = InternetProxyFactory.newInstance();
        }
        return _internetProxy;
    }


    private static String getNS6PrefsDirFromReg(File registryFile) {
	
	NSRegistry reg = new NSRegistry().open(registryFile);
	
	if (reg == null) return null;

        String username = reg.get("Common/Profiles/CurrentProfile");
	String path = null;
	if (reg != null) {
	    path = reg.get("Common/Profiles/" + username + "/directory");
	}
	
	// try default profile
	if (path == null) {	    
	    path = reg.get("Common/Profiles/default/directory");
	}
	
	reg.close();

	return path;
    }
   

    static File getNS6PrefsFile(File registryFile) {
	
	String path = getNS6PrefsDirFromReg(registryFile);

	return (path != null) ? new File(path, "prefs.js") : null;
    }
    
    /** Default implementation does a lookup from configuration file or
     *  from browser settings. We do not want to cache this, since the
     *  configuration file might change
     */
    public InternetProxyInfo getDefaultInfo() {
        InternetProxyInfo iProxyInfo = new InternetProxyInfo();
	ConfigProperties cp = ConfigProperties.getInstance();
	int proxyType = cp.getProxyType();
	if (proxyType == com.sun.javaws.proxy.InternetProxyInfo.MANUAL){
	    // Platform-independent lookup
	    try {
		int port = Integer.parseInt(cp.getHTTPProxyPort());
		String host = cp.getHTTPProxy();
		List hnames = cp.getProxyOverrideList();
		if (host != null && host.length() > 0  && port != 0) {
		    iProxyInfo.setHTTPPort(port);
		    iProxyInfo.setHTTPHost(host);
		    iProxyInfo.setOverrides(hnames);
		    iProxyInfo.setType(proxyType);
		}
	    } catch(NumberFormatException e) { /* Just ignore */ }
	} else if (proxyType == com.sun.javaws.proxy.InternetProxyInfo.AUTO){
	    // Browser-settings (platform-dependent)
	    iProxyInfo = getBrowserInfo();
	}
	return iProxyInfo;
    }
         
    /**
     *  This implementation is platform-dependent
     */
    public InternetProxyInfo getBrowserInfo() {
	return new InternetProxyInfo();
    }


    /**
     *  This implementation is platform-dependent
     */
    File getNS6RegFile() {
	return null;
    }

    public String getNS6PrefsDir() {
	
	File regFile = getNS6RegFile();

	if (regFile != null) {
	    return getNS6PrefsDirFromReg(regFile);
	}

	return null;

    }
}


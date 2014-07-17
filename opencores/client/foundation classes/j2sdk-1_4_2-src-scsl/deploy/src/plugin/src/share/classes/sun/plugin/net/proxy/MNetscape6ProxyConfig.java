/*
 *  @(#)MNetscape6ProxyConfig.java	1.5 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.net.proxy;

import java.io.File;
import java.io.IOException;
import sun.plugin.util.Trace;


/**
 * Proxy configuration for Netscape Navigator 4 on Unix
 */
public final class MNetscape6ProxyConfig implements BrowserProxyConfig 
{
    /* 
     * Returns browser proxy info
     */
    public BrowserProxyInfo getBrowserProxyInfo()
    {
	Trace.msgNetPrintln("net.proxy.loading.ns");
	
	BrowserProxyInfo info = new BrowserProxyInfo();
	String homeDir = null;
	String usrName = null;

	// Determine home directory
	try 
	{
	    homeDir = System.getProperty("user.home");
	    
	    File regFile = new File(homeDir + "/.mozilla/appreg");
	    
	    File file = null;
	    try {
	        file = NSPreferences.getNS6PrefsFile(regFile);
                Trace.msgNetPrintln("net.proxy.browser.pref.read", new Object[] {file.getPath()});
                NSPreferences.parseFile(file, info);
	    }
	    catch (IOException e)
	    {
		Trace.msgNetPrintln("net.proxy.ns6.regs.exception", new Object[] {regFile.getPath()});
	    }
	}
	catch (SecurityException e) 
	{
	    return info;
	}
	
	// This is a workaroud for NS6 since the bug of Liveconnect
        info.setType(ProxyType.BROWSER);
	Trace.msgNetPrintln("net.proxy.loading.done");
	
	return info;
    }
}

/*
 * @(#)WNetscape6ProxyConfig.java	1.15 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.net.proxy;

import java.io.File;
import java.io.IOException;
import sun.plugin.util.Trace;
import sun.plugin.services.WinRegistry;

/**
 * Proxy configuration for Netscape Navigator 6.
 */
public final class WNetscape6ProxyConfig implements BrowserProxyConfig 
{
    /* 
     * Returns browser proxy info
     */
    public BrowserProxyInfo getBrowserProxyInfo()
    {
	Trace.msgNetPrintln("net.proxy.loading.ns");
	
	BrowserProxyInfo info = new BrowserProxyInfo();
	
	WinRegistry.init();

	/* Get ApplicationData directory since registry.dat is stored there. */
	String appDataDir = WinRegistry.getString(WinRegistry.HKEY_CURRENT_USER, 
					        "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders",
						"AppData");
	if (appDataDir != null)
	{
	    File regFile = new File(appDataDir + "\\Mozilla\\registry.dat");
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

	// This is a workaroud for NS6 since the bug of Liveconnect
        info.setType(ProxyType.BROWSER);
	Trace.msgNetPrintln("net.proxy.loading.done");
	return info;
    }
}

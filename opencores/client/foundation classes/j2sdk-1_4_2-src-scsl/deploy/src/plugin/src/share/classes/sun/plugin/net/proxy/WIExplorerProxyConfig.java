/*
 * @(#)WIExplorerProxyConfig.java	1.16 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.net.proxy;

import java.net.URL;
import java.util.ArrayList;
import java.util.List;
import java.util.StringTokenizer;
import sun.plugin.services.WinRegistry;
import sun.plugin.util.Trace;


/**
 * Proxy configuration for Internet Explorer on Win32.
 */
public final class WIExplorerProxyConfig implements BrowserProxyConfig 
{
    // Constant defined in MSDN
    private static final String REGSTR_PATH_INTERNET_SETTINGS = "Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings";
    private static final String REGSTR_VAL_PROXYENABLE = "ProxyEnable";
    private static final String REGSTR_VAL_PROXYSERVER = "ProxyServer";
    private static final String REGSTR_VAL_PROXYOVERRIDE = "ProxyOverride";
    private static final String REGSTR_VAL_AUTOCONFIGURL = "AutoConfigURL";

    /* 
     * Returns browser proxy info
     */
    public BrowserProxyInfo getBrowserProxyInfo()
    {
	Trace.msgNetPrintln("net.proxy.loading.ie");

	BrowserProxyInfo info = new BrowserProxyInfo();

	/* Internet Explorer
	 *
	 * The Proxy{Server,Override,Enable} registry keys are covered by several
	 * MS web pages however only as a sidebar.  Here's an example:
	 *   http://support.microsoft.com/support/kb/articles/Q164/0/35.ASP
	 */
	WinRegistry.init();

	int key = WinRegistry.HKEY_CURRENT_USER;

	// Somtimes ProxyEnable is an integer
	Integer proxyEnable = WinRegistry.getInteger(key, REGSTR_PATH_INTERNET_SETTINGS, REGSTR_VAL_PROXYENABLE);
        
	Trace.msgNetPrintln("net.proxy.browser.proxyEnable", new Object[] {proxyEnable});

	if (proxyEnable != null) 
	    info.setType(proxyEnable.intValue());

	// Read manual proxy info
	if (info.getType() == ProxyType.MANUAL) 
	{
	    String proxyList = WinRegistry.getString(key, REGSTR_PATH_INTERNET_SETTINGS, REGSTR_VAL_PROXYSERVER);

    	    Trace.msgNetPrintln("net.proxy.browser.proxyList", new Object[] {proxyList});

	    if (proxyList != null) 
		ProxyUtils.parseProxyServer(proxyList, info);
	    
	    // Read proxy override list
	    String proxyOverride = WinRegistry.getString(key, REGSTR_PATH_INTERNET_SETTINGS, REGSTR_VAL_PROXYOVERRIDE);

   	    Trace.msgNetPrintln("net.proxy.browser.proxyOverride", new Object[] {proxyOverride});

	    // Break down proxy override list into token
	    if (proxyOverride != null) 
	    {
		// Elements in proxy by-pass list in IE is separated by ";".
		// Wildcard is also accepted
		//
		StringTokenizer st = new StringTokenizer(proxyOverride, ";");
		ArrayList list = new ArrayList();
		while (st.hasMoreTokens()) 
		{
		    // Convert the list to lower case
		    String item = st.nextToken().toLowerCase(java.util.Locale.ENGLISH).trim();

		    if (item != null)
		    {
			list.add(item);
		    }
		}

		info.setOverrides(list);
	    }
	}

	// Check if auto config is enabled
	String autoConfigURL = WinRegistry.getString(key, REGSTR_PATH_INTERNET_SETTINGS, REGSTR_VAL_AUTOCONFIGURL);

        if (autoConfigURL != null) 
	{
    	    Trace.msgNetPrintln("net.proxy.browser.autoConfigURL", new Object[] {autoConfigURL});

	    info.setType(ProxyType.AUTO);
	    info.setAutoConfigURL(autoConfigURL);
	}

	Trace.msgNetPrintln("net.proxy.loading.done");

	return info;
    }
}




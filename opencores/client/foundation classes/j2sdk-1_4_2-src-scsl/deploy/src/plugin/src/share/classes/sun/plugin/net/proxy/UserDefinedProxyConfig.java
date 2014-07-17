/*
 * @(#)UserDefinedProxyConfig.java	1.19 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.net.proxy;

import java.io.BufferedInputStream;
import java.io.InputStream;
import java.io.FileInputStream;
import java.io.IOException;
import java.net.URL;
import java.util.ArrayList;
import java.util.List;
import java.util.Properties;
import java.util.StringTokenizer;
import sun.plugin.util.Trace;
import sun.plugin.util.UserProfile;


/**
 * Proxy configuration defined by users in Control Panel
 */
final class UserDefinedProxyConfig implements BrowserProxyConfig 
{
    /* 
     * Returns browser proxy info
     */
    public BrowserProxyInfo getBrowserProxyInfo()
    {
	Trace.msgNetPrintln("net.proxy.loading.userdef");

	BrowserProxyInfo info = new BrowserProxyInfo();
	info.setType(ProxyType.BROWSER);

	// Determine the user property file location
	String propFile = UserProfile.getPropertyFile(); 

	try
	{
	    // Read settings from user property file
	    InputStream is = new BufferedInputStream(new FileInputStream(propFile));

	    Properties props = new Properties();
	    props.load(is);

	    is.close();


	    if ("false".equals(props.getProperty("javaplugin.proxy.usebrowsersettings", "true")))
	    {
		info.setType(ProxyType.MANUAL);

		String autoConfigURL = props.getProperty("javaplugin.proxy.auto.url", "");

    		Trace.msgNetPrintln("net.proxy.browser.autoConfigURL", new Object[] {autoConfigURL});

		if (autoConfigURL != null && !("".equals(autoConfigURL.trim())))
		{
		    info.setType(ProxyType.AUTO);
		    info.setAutoConfigURL(autoConfigURL);
		}

		String proxyList = props.getProperty("javaplugin.proxy.settings", "");

		Trace.msgNetPrintln("net.proxy.browser.proxyList", new Object[] {proxyList});
		
		if (proxyList != null && !("".equals(proxyList.trim()))) 
		    ProxyUtils.parseProxyServer(proxyList, info);

		String proxyOverride = props.getProperty("javaplugin.proxy.bypass", "");

    		Trace.msgNetPrintln("net.proxy.browser.proxyOverride", new Object[] {proxyOverride});

		// Elements in proxy by-pass list in IE is separated by ";".
		// Wildcard is also accepted
		//
		if (proxyOverride != null && !("".equals(proxyOverride.trim())))
		{
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
	}
	catch (IOException e)
	{
	    // Exception may be thrown if the file does not exist
	}

	Trace.msgNetPrintln("net.proxy.loading.done");

	return info;
    }
}





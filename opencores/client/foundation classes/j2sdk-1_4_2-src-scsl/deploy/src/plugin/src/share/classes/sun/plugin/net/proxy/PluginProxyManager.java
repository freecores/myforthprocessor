/*
 * @(#)PluginProxyManager.java	1.43 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.plugin.net.proxy;

import java.net.URL;
import java.util.HashMap;
import java.util.Properties;
import sun.plugin.services.BrowserService;
import sun.plugin.services.ServiceProvider;
import sun.plugin.util.Trace;


/*
 * This class holds a reference to the default proxy handler.
 */
public class PluginProxyManager 
{
    // Proxy info cache
    private static HashMap proxyCache = new HashMap();

    // Proxy Handler
    private static ProxyHandler handler = null;

    /**
     * Return proxy info for a given URL
     *
     * @param u URL 
     * @retur Proxy info for a given URL
     */
    public synchronized static ProxyInfo getProxyInfo(URL url)
    {
	StringBuffer keyBuffer = new StringBuffer();
	keyBuffer.append(url.getProtocol());
	keyBuffer.append(url.getHost());
	keyBuffer.append(url.getPort());

	String key = keyBuffer.toString();

        ProxyInfo pInfo = null;
	
	// Check if proxy cache should be used
	//
	if (handler.isProxyCacheSupported())
	    pInfo = (ProxyInfo) proxyCache.get(key);
    
	if (pInfo == null)
	{
	    try
	    {
		ProxyInfo[] retProxyArray = handler.getProxyInfo(url);
		pInfo = retProxyArray[0];

		if (pInfo == null)
		    pInfo = new ProxyInfo(null);

		// Store it in cache for further use
		proxyCache.put(key.toString(), pInfo);
	    }
	    catch (sun.plugin.services.ServiceUnavailableException e)
	    {
                Trace.msgNetPrintln("net.proxy.service.not_available", new Object[] {url});
                
		// Notice that we should NOT cache the result here because we should
		// determine the proxy again once the service is available
		//
		pInfo = new ProxyInfo(null);
	    }
	}

	return pInfo;
    }


    /**
     * Reset proxy info
     */
    public synchronized static void reset()
    {
	try{
	    // Clear proxy cache
	    proxyCache.clear();

	    // Determine if the users have set the proxy configuration 
	    // in Control Panel.
	    //
	    UserDefinedProxyConfig upc = new UserDefinedProxyConfig();

	    BrowserProxyInfo bpi = upc.getBrowserProxyInfo();

	    if (bpi.getType() == ProxyType.BROWSER)
	    {    
		// User have decided to use browser settings
		//

		// Obtain browser service
		BrowserService service = sun.plugin.services.ServiceProvider.getService();	
		
		// Obtain browser proxy config
		BrowserProxyConfig bpc = service.getProxyConfig();

		// Obtain browser proxy info
		bpi = bpc.getBrowserProxyInfo();
	    }

	    switch (bpi.getType())
	    {
		case ProxyType.BROWSER: 
		    try
		    {
			handler = new SmartProxyHandler(new BrowserProxyHandler());
			handler.init(bpi);
		    }
		    catch (ProxyConfigException e)
		    {
			// This should never happen
			assert false;
		    }
		    break;
		case ProxyType.NONE: 
		    try
		    {
			handler = new DirectProxyHandler();
			handler.init(bpi);
		    }
		    catch (ProxyConfigException e0)
		    {
			// This should never happen
			assert false;
		    }
		    break;
		case ProxyType.MANUAL:
		    try
		    {
			handler = new ManualProxyHandler();
			handler.init(bpi);
		    }
		    catch (ProxyConfigException e0)
		    {
			// System.out.println(e0.toString());

			Trace.msgNetPrintln("net.proxy.loading.manual.error");

			try
			{
			    bpi.setType(ProxyType.NONE);
			    handler = new DirectProxyHandler();
			    handler.init(bpi);
			}
			catch (ProxyConfigException e1)
			{
			    // This should never happen
			    assert false;
			}
		    }

		    break;
		case ProxyType.AUTO:
		    try
		    {
			handler = new SmartProxyHandler(new AutoProxyHandler());
			handler.init(bpi);
		    }
		    catch (ProxyConfigException e0)
		    {
			// System.out.println(e0.toString());

			Trace.msgNetPrintln("net.proxy.loading.auto.error");

			try
			{
			    bpi.setType(ProxyType.MANUAL);
			    handler = new ManualProxyHandler();
			    handler.init(bpi);
			}
			catch (ProxyConfigException e1)
			{
			    // e1.printStackTrace();

			    Trace.msgNetPrintln("net.proxy.loading.manual.error");

			    try
			    {
				bpi.setType(ProxyType.NONE);
				handler = new DirectProxyHandler();
				handler.init(bpi);
			    }
			    catch (ProxyConfigException e2)
			    {
				// This should never happen
				assert false;
			    }
			}
		    }
		    break;
    		default:
		    assert false;
		    break;
	    }

	    // Set RMI socket factory for proxy.
	    try
	    {
		if (bpi.getType() != ProxyType.NONE)
		    java.rmi.server.RMISocketFactory.setSocketFactory(new sun.plugin.net.protocol.rmi.RMIPluginSocketFactory());
	    }
	    catch (Throwable e)
	    {
	    }


	    // Try to set the system properties to reflect the proxy settings
	    final BrowserProxyInfo info = bpi;

	    java.security.AccessController.doPrivileged(
		new java.security.PrivilegedAction() {
		public Object run() {
                    setProperties(info);
		    return null;
		}
	    });

	    // Display proxy configuration information 
	    System.err.println(bpi.toString());
	}
	catch (Throwable e)
	{
	    e.printStackTrace();
	    sun.plugin.util.DialogFactory.showExceptionDialog(e);
	}
    }

    /**
     * setProperties set all the proxy related system properties.
     */
    private static void setProperties(BrowserProxyInfo info)
    {
	Properties props = System.getProperties();

	int proxyType = info.getType();
	StringBuffer sb = new StringBuffer();

	if (info.getHttpHost() != null)
	{
	    sb.append("http=" + info.getHttpHost());
	    if (info.getHttpPort() != -1)
		sb.append(":" + info.getHttpPort());
	}
	if (info.getHttpsHost() != null)
	{
	    sb.append(",https=" + info.getHttpsHost());
	    if (info.getHttpsPort() != -1)
		sb.append(":" + info.getHttpsPort());
	}
	if (info.getFtpHost() != null)
	{
	    sb.append(",ftp=" + info.getFtpHost());
	    if (info.getFtpPort() != -1)
		sb.append(":" + info.getFtpPort());
	}
	if (info.getGopherHost() != null)
	{
	    sb.append(",gopher=" + info.getGopherHost());
	    if (info.getGopherPort() != -1)
		sb.append(":" + info.getGopherPort());
	}
	if (info.getSocksHost() != null)
	{
	    sb.append(",socks=" + info.getSocksHost());
	    if (info.getSocksPort() != -1)
		sb.append(":" + info.getSocksPort());
	}
	
	String proxyList = sb.toString();
		
	String[] overrides = info.getOverrides();
	String proxyOverride = null;

	if (overrides != null) 
	{
	    sb = new StringBuffer();

	    boolean first = true;
	    for (int idx = 0 ; idx < overrides.length ; idx++) 
	    {
		if (idx != 0)
		    sb.append(",");    

		sb.append(overrides[idx]);
	    }

	    proxyOverride = sb.toString();
	}


	props.remove("javaplugin.proxy.config.type");
	props.remove("javaplugin.proxy.config.list");
	props.remove("javaplugin.proxy.config.bypass");

	if (proxyType == ProxyType.NONE)
	{
	    props.put("javaplugin.proxy.config.type", "direct");
	}	    
	else if (proxyType == ProxyType.MANUAL)
	{
	    props.put("javaplugin.proxy.config.type", "manual");

	    if (proxyList != null)
	        props.put("javaplugin.proxy.config.list", proxyList);

	    if (proxyOverride != null)
	        props.put("javaplugin.proxy.config.bypass", proxyOverride);
	}
	else if (proxyType == ProxyType.AUTO)
	{
	    props.put("javaplugin.proxy.config.type", "auto");
	}
	else if (proxyType == ProxyType.BROWSER)
	{
	    props.put("javaplugin.proxy.config.type", "browser");
	    
	    if (proxyList != null)
	        props.put("javaplugin.proxy.config.list", proxyList);

	    if (proxyOverride != null)
	        props.put("javaplugin.proxy.config.bypass", proxyOverride);
	}
	else
	{
	    props.put("javaplugin.proxy.config.type", "unknown");
	}
	
	// If this is manual proxy configuration and SOCKS is set,
	// we need to setup the SOCKS properties. Notice that
	// by setting the SOCKS properties, all the protocols
	// will now go through SOCKS, no matter what the exception
	// list is.
	//
	if (proxyType == ProxyType.MANUAL)
	{		
	    // Extract info about SOCKS
	    String socksProxy = info.getSocksHost();
	    int socksPort = info.getSocksPort();
				    
	    if (socksProxy != null)
		props.put("socksProxyHost", socksProxy);

	    if (socksPort != -1)
		props.put("socksProxyPort", new Integer(socksPort));
	}

	System.setProperties(props);
    }
}

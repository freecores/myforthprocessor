/*
 * @(#)AutoProxyHandler.java	1.34 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.net.proxy;

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.Reader;
import java.io.StringWriter;
import java.net.InetAddress;
import java.net.MalformedURLException;
import java.net.URL;
import java.net.URLConnection;
import java.util.StringTokenizer;
import netscape.javascript.JSObject;
import sun.applet.AppletPanel;
import sun.plugin.net.protocol.http.HttpURLConnection;
import sun.plugin.viewer.AppletPanelCache;
import sun.plugin.viewer.context.PluginAppletContext;
import sun.plugin.util.Trace;
import sun.plugin.services.BrowserService;

/**
 * Proxy handler for auto proxy configuration.
 */
public class AutoProxyHandler implements ProxyHandler 
{
    // Browser proxy info
    private BrowserProxyInfo bpi = null;

    // JavaScript for auto proxy config
    private StringBuffer autoProxyScript = null;

    /**
     * Check if the proxy handler supports the proxy type
     *
     * @param proxyType Proxy type
     * @return true if proxy type is supported
     */
    public boolean isSupported(int proxyType)
    {
	return (proxyType == ProxyType.AUTO);
    }

    /**
     * Check if the proxy result should be cached
     *
     * @return true if proxy result should be cached
     */
    public boolean isProxyCacheSupported()
    {
	return true;
    }

    /**
     * Initialize the auto proxy handler.
     *
     * @param info Browser proxy info
     */
    public void init(BrowserProxyInfo info) 
		throws ProxyConfigException
    {
	Trace.msgNetPrintln("net.proxy.loading.auto");

	// Check if proxy type is supported
	if (isSupported(info.getType()) == false)
	    throw new ProxyConfigException("Unable to support proxy type: " + info.getType());

	// Store browser proxy info
	bpi = info;

	// Check the browser type for different implementation of dnsResolve 
	// and isInet function;
	// Fix for 4670449 (5/9/02)
	BrowserService browSvc = sun.plugin.services.ServiceProvider.getService();
	// Construct the JavaScript
	//
	autoProxyScript = new StringBuffer();

	// Combine Auto proxy script
	autoProxyScript.append(AutoProxyScript.jsGlobal);
	autoProxyScript.append(AutoProxyScript.jsDnsDomainIs);
	autoProxyScript.append(AutoProxyScript.jsIsPlainHostName);
	if (browSvc.isIExplorer())
	{
	    autoProxyScript.append(AutoProxyScript.jsIsInNetForIE);
	    autoProxyScript.append(AutoProxyScript.jsDnsResolveForIE);
	}
	else
	{
	    autoProxyScript.append(AutoProxyScript.jsIsInNetForNS);
	    autoProxyScript.append(AutoProxyScript.jsDnsResolveForNS);
	}
	autoProxyScript.append(AutoProxyScript.jsIsResolvable);
	autoProxyScript.append(AutoProxyScript.jsLocalHostOrDomainIs);
	autoProxyScript.append(AutoProxyScript.jsDnsDomainLevels);
	autoProxyScript.append(AutoProxyScript.jsMyIpAddress_0);

	try
	{
	    InetAddress address = InetAddress.getLocalHost();
	    autoProxyScript.append(address.getHostAddress());
	}
	catch (Throwable e)
	{
	    e.printStackTrace();

	    // If somehow we fail to obtain the IP address,
	    // use loop back
	    autoProxyScript.append("127.0.0.1");
	}

	autoProxyScript.append(AutoProxyScript.jsMyIpAddress_1);
	autoProxyScript.append(AutoProxyScript.jsShExpMatch);
	autoProxyScript.append(AutoProxyScript.jsEnableDateRange);
	autoProxyScript.append(AutoProxyScript.jsEnableTimeRange);
	autoProxyScript.append(AutoProxyScript.jsEnableWeekdayRange);


	// Download the auto proxy config file if necessary
	//
	String autoConfigURL = bpi.getAutoConfigURL();

	try
	{
	    URL url = new URL(autoConfigURL);
	}
	catch (MalformedURLException e)
	{
	    throw new ProxyConfigException("Auto config URL is malformed");
	}

	// If we are here, the URL should be okay
	
	// Check if auto proxy file is INS file supported by IEAK
	if (autoConfigURL.toLowerCase().endsWith(".ins"))
	{
	    // Try to download the IEAK file and parse the AutoConfigJSURL field
	    autoConfigURL = getAutoConfigURLFromINS(autoConfigURL);
	}
	
	// If we are here, the URL should be a JavaScript file
	autoProxyScript.append(getJSFileFromURL(autoConfigURL));

	Trace.msgNetPrintln("net.proxy.loading.done");
    }


    /**
     * Returns proxy info for a given URL
     *
     * @param u URL
     * @return proxy info for a given URL
     */
    public ProxyInfo[] getProxyInfo(URL u)
		     throws sun.plugin.services.ServiceUnavailableException
    {
	// Auto proxy service in is emulated through JavaScript URL by 
	// using one of the plugin instances. Thus, there MUST be at 
	// least one plugin instance exists. Otherwise, accessing 
	// the proxy service without any plugin instance will likely 
	// to confuse the browser and fail. Thus, it is VERY important 
	// to check if there is at least one applet instance before 
	// calling back to the browser.
	//
	if (AppletPanelCache.hasValidInstance() == false)
	{
	    // Proxy service NOT available
	    throw new sun.plugin.services.ServiceUnavailableException("Proxy service unavailable");
	}

	// To determine the proxy info for a given URL,
	// we use JSObject from one of the applets to
	// determine the result.
	//

	Object[] appletPanels = AppletPanelCache.getAppletPanels();
	    
	AppletPanel p = (AppletPanel) appletPanels[0];
	PluginAppletContext pac = (PluginAppletContext) p.getAppletContext();
	String result = null;
		    
	try
	{
	    JSObject win = pac.getJSObject();

	    StringBuffer buffer = new StringBuffer();
	    buffer.append(autoProxyScript);
	    buffer.append("FindProxyForURL('");
	    buffer.append(u);
	    buffer.append("','");
	    buffer.append(u.getHost());
	    buffer.append("');");

	    result = (String) win.eval(buffer.toString());

	    return extractAutoProxySetting(result);
	}
	catch (Throwable e)
	{
	    Trace.msgNetPrintln("net.proxy.auto.result.error");

	    return new ProxyInfo[] {new ProxyInfo(null)};
	}
    }

    /**
     * Download INS file for Internet Explorer Administration Kit (IEAK), 
     * and obtain the AutoConfigURL from there.
     *
     * @param URL url to INS file
     * @return Auto config URL in INS file
     */
    private String getAutoConfigURLFromINS(String u)
		   throws ProxyConfigException
    {
	Trace.msgNetPrintln("net.proxy.auto.download.ins", new Object[] {u});

	try
	{
	    URL url = new URL(u);

	    // Obtain protocol
	    String protocol = url.getProtocol();

	    URLConnection conn = null;

	    if (protocol.equals("file"))
	    {
		conn = url.openConnection();
	    }
	    else
	    {
		// Create a HTTP connection without proxy
		conn = new HttpURLConnection(url, null, -1, false);
	    }

	    // Add Comment filter
	    BufferedReader br = new BufferedReader(new InputStreamReader(conn.getInputStream()));
	    String buffer = null;
	    String autoConfigURL = null;

	    do
	    {
		buffer = br.readLine();

		if (buffer != null)
		{
		    // It is VERY important to check if the index
		    // is 0 because it is only valid if AutoConfigJSURL is
		    // not commented out by users.
		    //
		    if (buffer.indexOf("AutoConfigJSURL=") == 0)
		    {
			autoConfigURL = buffer.substring(16);
			break;
		    }
		}		    
	    }
	    while (buffer != null);

	    // Close the streams
	    br.close();

	    if (autoConfigURL != null)
		return autoConfigURL;
	    else 
		throw new ProxyConfigException("Unable to locate 'AutoConfigJSURL' in INS file");
	}
	catch (ProxyConfigException e0)
	{   
	    throw e0;
	}
	catch (Throwable e1)
	{   
	    throw new ProxyConfigException("Unable to obtain INS file from " + u, e1);
	}
    }


    /**
     * getJSFilefromURL downloads a JS file through HTTP with no proxy, and return
     * the file content.
     **/
    private String getJSFileFromURL(String u)
		   throws ProxyConfigException
    {
	Trace.msgNetPrintln("net.proxy.auto.download.js", new Object[] {u});

        try  
	{
             URL url = new URL(u);

             // Obtain protocol
             String protocol = url.getProtocol();

	     URLConnection conn = null;

	     if (protocol.equals("file"))
	     { 
		conn = url.openConnection();
	     }
	     else
	     {
		// Create a HTTP connection without proxy
		conn = new HttpURLConnection(url, null, -1, false);
	     }

            // Add Comment filter
	    Reader rin = new RemoveCommentReader(new InputStreamReader(conn.getInputStream()));
	    BufferedReader br = new BufferedReader(rin);
	    StringWriter sw = new StringWriter();

	    char[] buffer = new char[4096];
	    int numchars;
	    while ((numchars = br.read(buffer)) != -1)
	    {
		sw.write(buffer, 0, numchars);
	    }

	    // Close the streams
	    br.close();
	    rin.close();
	    sw.close();

	    return sw.toString();
        }
        catch (Throwable e) 
	{
	    throw new ProxyConfigException("Unable to obtain auto proxy file from " + u, e);
        }
    }
   

    /**
     * extractAutoProxySetting is a function which takes a proxy-info-string
     * which returned from the JavaScript function FindProxyForURL, and returns
     * the corresponding proxy information.
     *
     * parameters :
     *	s         [in]	a string which contains all the proxy information
     *
     * out:
     *   ProxyInfo [out] ProxyInfo contains the corresponding proxy result.
     *
     * Notes: i) s contains all the proxy information in the form of
     *        "PROXY webcache1-cup:8080;SOCKS webcache2-cup". There are three
     *        possible values inside the string:
     *	        a) "DIRECT" -- no proxy is used.
     *          b) "PROXY"  -- Proxy is used.
     *          c) "SOCKS"  -- SOCKS support is used.
     *        Information for each proxy settings are seperated by ';'. If a
     *	      port number is specified, it is specified by using ':' following
     *        the proxy host.
     *
     */
    private ProxyInfo[] extractAutoProxySetting(String s)
    {
	if (s != null)
	{
	    StringTokenizer st = new StringTokenizer(s, ";", false);
	    ProxyInfo proxyInfoArray[] = new ProxyInfo[st.countTokens()];
	    int index = 0;
	    
	    while (st.hasMoreTokens()) 
	    {  
		String pattern = st.nextToken();     

		int i = pattern.indexOf("PROXY");

		if (i != -1)   {
		    // "PROXY" is specified
		    proxyInfoArray[index++] = new ProxyInfo(pattern.substring(i + 6));
		    continue;
		}

		i = pattern.indexOf("SOCKS");

		if (i != -1) 
		{
		    // "SOCKS" is specified
		    proxyInfoArray[index++] = new ProxyInfo(null, pattern.substring(i + 6));
		    continue;
		}
                // proxy string contains 'DIRECT' or unrecognized text
		proxyInfoArray[index++] = new ProxyInfo(null, -1);
	    }
	    return proxyInfoArray;
	}
	//In order to make the return value safe to use, created a null ProxyInfo object
	return new ProxyInfo[] {new ProxyInfo(null)};
    }
}




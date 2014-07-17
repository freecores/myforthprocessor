/*
 * @(#)HttpURLConnection.java	1.80 02/05/30
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.net.protocol.http;

import java.net.URL;
import java.net.ProtocolException;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.io.BufferedInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.FileInputStream;
import java.io.PrintStream;
import java.text.MessageFormat;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.ResourceBundle;
import java.util.StringTokenizer;
import java.util.Map;
import java.util.Iterator;//test
import java.security.AccessController;
import java.security.PrivilegedExceptionAction;
import java.security.PrivilegedActionException;
import sun.plugin.net.proxy.ProxyInfo;
import sun.plugin.net.proxy.PluginProxyManager;
import sun.plugin.net.protocol.http.Handler;
import sun.plugin.net.protocol.http.HttpClient;
import sun.plugin.AppletViewer;
import sun.plugin.util.ProgressInputStream;
import sun.plugin.util.Trace;
import sun.plugin.util.TraceFilter;
import sun.plugin.util.ThreadGroupLocal;
import sun.plugin.resources.ResourceHandler;
import sun.plugin.net.cookie.PluginCookieManager;
import sun.plugin.cache.FileCache;
import sun.plugin.cache.CachedFile;
import sun.plugin.cache.DownloadException;
import sun.net.www.MessageHeader;

/**
 * A class to represent an HTTP connection to a remote object.
 *
 * @author  Graham Hamilton
 *          Stanley Man-Kit Ho
 *          Robert Szewczyk
 */

public class HttpURLConnection extends
sun.net.www.protocol.http.HttpURLConnection {

    private String proxy = null;
    private int proxyPort = -1;
    private boolean resolveProxy = true;

    // This is to keep trace of the failedOnce value in the super class. Since failedOnce
    // in the super one are declared private, we do some hack to get this value.
    boolean failedOnce = false;
    private MessageHeader cachedHeaders = null;
    private boolean progressTracking = false;

    // Message format
    private static MessageFormat mf1 = new MessageFormat(ResourceHandler.getMessage("net.connect.no_proxy"));
    private static MessageFormat mf2 = new MessageFormat(ResourceHandler.getMessage("net.connect.proxy_is"));

    private int callCount = 0;

    private InputStream cacheStream = null;
    private IOException failedException = null;

    public HttpURLConnection(URL u, Handler handler)
    throws IOException {
	super(u, handler);
    }

    /** this constructor is used by other protocol handlers such as ftp
        that want to use http to fetch urls on their behalf. */
    public HttpURLConnection(URL u, String host, int port) 
    throws IOException {
	this(u, host, port, true);
    }

    /** this constructor is used by other protocol handlers such as ftp
        that want to use http to fetch urls on their behalf. */
    public HttpURLConnection(URL u, String host, int port, boolean resolveProxy) 
    throws IOException {
	super(u, host, port);
        this.proxy = host;
        this.proxyPort = port;
	this.resolveProxy = resolveProxy;
    }

    void createConnection() throws IOException {
	if ("http".equals(url.getProtocol()) && !failedOnce) {
	    http = HttpClient.New(url, proxy, proxyPort, resolveProxy);
	} else {
	    // make sure to construct new connection if first attempt failed
	    http = getProxiedClient(url, proxy, proxyPort);
	}
	ps = (PrintStream)http.getOutputStream();
    }


    // true if the security check n has been made
    private boolean securityCheck = false;

    // true if the cache connection has been made
    private boolean cacheConnect = false;

    // true if the real connection has been made
    private boolean realConnect = false;

    // true if the set-cookie header has been checked
    private boolean checkSetCookie = false;

    // overridden in HTTPS subclass
    public synchronized void connect() throws IOException {

	if (connected)
	    return;

	// Perform security check
	checkPermission();

	// Check if progress should be tracked
	if (getRequestMethod().equalsIgnoreCase("GET")) 
	{
	    if (ThreadGroupLocal.get(ThreadGroupLocal.PROGRESS_TRACKING) != null)
		progressTracking = true;
	}

	// If cache is enabled, try the browser cache.
	try {
	    connectWithCache();
	}
	catch(DownloadException dle) {
	    // we already tried but failed
	    cachedHeaders = dle.getHeaders();
	    failedException = dle.getIOException();
	    throw failedException;
	}
	catch(IOException e) {
	    // ignore here
	}


	// If cache is disabled, make a connection
	if (cacheStream == null) 
	{
	    // Setup real connection
	    connectSetup();
	    createConnection();
	}

	connected = true;
    }

    //set the cacheStream to null whenever disconnect() is called
    //bug #4514843
    public synchronized void disconnect(){
      super.disconnect();
      if(cacheStream != null) {
          cacheStream = null;
      }
      if(cachedHeaders != null) {
			cachedHeaders.reset();
      }
    }


    /**
     * Perform security check before establish a connection
     */     
    private void checkPermission() throws IOException {

	if (securityCheck)
	    return;

	// Check whether the applet is allowed to connect to the host.
	// This is necessary because we will enable privilege to 
	// connect to the proxy
	SecurityManager m = System.getSecurityManager();
	if (m != null)	{
	    m.checkConnect(url.getHost(), url.getPort());
	}

	securityCheck = true;
    }


    /**
     * Perform steps to establish a connection: determine if file is
     * already in cache.
     */     
    private void connectWithCache() throws IOException 
    {
	if (cacheConnect)
	    return;

	if (getUseCaches() && getRequestMethod().equalsIgnoreCase("GET")) 
	{
	    //Check in cache first
	    final CachedFile file = FileCache.get(url);
	    if (file != null) {
		// We sucessfully obtained the cache file, 
		InputStream is = null;
		try {
		    is = (InputStream) AccessController.doPrivileged(
			 new PrivilegedExceptionAction() {
			    public Object run() throws IOException {
				return new BufferedInputStream(new FileInputStream(file));
			 } 
		    });
		} catch (PrivilegedActionException e) {
		    //no-op
		}

		if (is != null) {
		    // HTTP_OK must be set.
		    responseCode = HTTP_OK;
		    // get the header values from cache
		    cachedHeaders = file.getHeaderFields();
		    URL targetURL = file.getURL();
		    if( targetURL != null) {
			url = targetURL;
		    }

		    // Create ProgressInputStream only if we need it.
		    if (progressTracking) 
			cacheStream = new ProgressInputStream(is, this);                        
		    else
			cacheStream = is;                        
		}
	    }
	}


	cacheConnect = true;
    }

    /**
     * Gets a header field by name. Returns null if not known.
     * @param name the name of the header field
     */
    public String getHeaderField(String name) {
	String field = null;
	try {
	    getInputStream();
	} catch (IOException e) {}

	if(cachedHeaders != null) {
	    field = cachedHeaders.findValue(name);
	}else {
	    field = super.getHeaderField(name);
	}

	return field;
    }

	/**
	 * Gets a header field by index.
	 * Fix bug# 4670429 
	 */
	 public String getHeaderField(int index) {
		try {
			getInputStream();
		} catch(IOException e) {}

		if(null != cachedHeaders)
			return cachedHeaders.getValue(index);
		
		return super.getHeaderField(index);
	 }
    
	/**
	 * Gets a header field key by index.
	 * Fix bug# 4670429 
	 */
	 public String getHeaderFieldKey(int index) {
		try {
			getInputStream();
		} catch(IOException e) {}

		if(null != cachedHeaders)
			return cachedHeaders.getKey(index);
		
		return super.getHeaderFieldKey(index);
	}

    /**
     * Perform steps to establish a connection: determine proxy/cookie
     * setting, and setup a real connection.
     */     
    private void connectSetup() throws IOException {

	if (realConnect)
	    return;

	// Determine proxy setting for the connection

        if (proxy == null && resolveProxy == true) 
	{
	    ProxyInfo pinfo = PluginProxyManager.getProxyInfo(url);

	    if (pinfo != null)
	    {
        	proxy = pinfo.getProxy();
		proxyPort = pinfo.getPort();
	    }
	}


	// Output connection message to tracing facilities
	//
	if (proxy == null) 
	{
	    Object[] args = { url };
	    Trace.netPrintln(mf1.format(args), TraceFilter.JAVA_CONSOLE_ONLY);
	} 
	else 
	{
	    Object[] args = { url, (proxy + ((proxyPort != -1) ? (":" + proxyPort) : "")) };
	    Trace.netPrintln(mf2.format(args), TraceFilter.JAVA_CONSOLE_ONLY);
	}

        // Determine cookie value

	// Use browser cookie only if the cookie has not 
	// been set by the applet.
	if (getRequestProperty("cookie") == null)
	{
	    String cookie = PluginCookieManager.getCookieInfo(url);

	    // Make sure we don't overwrite existing headers
	    if (cookie != null)
		addRequestProperty("cookie", cookie);
	}

	realConnect = true;
    }

    public synchronized void checkCookieHeader()
    {
	if (checkSetCookie)
	    return;

	checkSetCookie = true;
	
	// We have been connected already, so we need to check 
	// if "Set-Cookie" is in the respond headers

	// Fixed #4593204: Unable to deal with multiple "Set-Cookie"
	// HTTP headers.  [stanleyh]
	//
	Map headersMap = getHeaderFields();

	// Iterator all headers
	for (Iterator keyIter = headersMap.keySet().iterator(); keyIter.hasNext(); )
	{
	    String key = (String) keyIter.next();

	    if (key != null && key.equalsIgnoreCase("Set-Cookie"))
	    {
    		List setCookieList = (List) headersMap.get(key);

		// Iterate all "Set-Cookie" headers
		for (Iterator listIter = setCookieList.iterator(); listIter.hasNext(); )
		{
		    String cookieValue = (String) listIter.next();

		    if (cookieValue != null)
		    {
			PluginCookieManager.setCookieInfo(url, cookieValue);
		    }
		}
	    }
	}
    }

    public synchronized InputStream getInputStream() throws IOException
    {
	// already tried but failed
	if(failedException != null)
	    throw failedException;

	// Perform security check, without making real connection
	connect();

	try {
	    callCount++;

	    // Return cached stream if possible
	    if (cacheStream != null)
		return cacheStream;

	    InputStream is = super.getInputStream();

	    if(callCount == 1) {
		// If we got the input stream, wrap it so its progress will
		// be tracked if progress bar should be displayed.  Else use
		// InputStream.
		if (is != null)
		{
		    if (progressTracking && !(is instanceof ProgressInputStream))
			cacheStream = new ProgressInputStream(is, this);                        
		    else
			cacheStream = is;            
		}

		// Check "Set-cookie" header
		checkCookieHeader();
		return cacheStream;
	    } else {
		return is;
	    }
	}finally {
	    callCount--;
	}
    }


    /**
     * Create a new HttpClient object, bypassing the cache of
     * HTTP client objects/connections.
     *
     * @param url	the URL being accessed
     */
    protected sun.net.www.http.HttpClient getNewClient (URL url)
    throws IOException {

	// This is a hack to get the failedOnce value. Since getNewClient is only called by the 
	// super class when failedOnce is true, we can obtain the failedOnce value this way. 
	failedOnce = true;

	return getProxiedClient(url, proxy, proxyPort);
    }


    /**
     * Create a new HttpClient object, set up so that it uses
     * per-instance proxying to the given HTTP proxy.  This
     * bypasses the cache of HTTP client objects/connections.
     *
     * @param url	the URL being accessed
     * @param proxyHost	the proxy host to use
     * @param proxyPort	the proxy port to use
     */
    protected sun.net.www.http.HttpClient getProxiedClient (URL url, String proxyHost, int proxyPort)
    throws IOException {
	return new HttpClient (url, proxyHost, proxyPort, resolveProxy);
    }
}



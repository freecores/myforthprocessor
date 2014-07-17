/*
 * @(#)PluginDelegateHttpsURLConnection.java	1.13 02/05/30
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.net.www.protocol.https;

import java.io.BufferedInputStream;
import java.io.InputStream;
import java.io.FileInputStream;
import java.io.IOException;
import java.lang.reflect.Method;
import java.lang.reflect.InvocationTargetException;
import java.net.URL;
import java.net.URLConnection;
import java.security.AccessController;
import java.security.PrivilegedAction;
import java.security.PrivilegedExceptionAction;
import java.security.PrivilegedActionException;
import java.text.MessageFormat;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import javax.net.ssl.SSLSocketFactory;
import javax.net.ssl.HostnameVerifier;
import sun.net.www.protocol.https.DelegateHttpsURLConnection;
import sun.plugin.net.cookie.PluginCookieManager;
import sun.plugin.net.proxy.ProxyInfo;
import sun.plugin.net.proxy.PluginProxyManager;
import sun.plugin.resources.ResourceHandler;
import sun.plugin.util.ProgressInputStream;
import sun.plugin.util.ThreadGroupLocal;
import sun.plugin.util.Trace;
import sun.plugin.util.TraceFilter;
import sun.plugin.cache.CachedFile;
import sun.plugin.cache.FileCache;
import sun.net.www.MessageHeader;

/**
 * A class to represent an HTTPS connection to a remote object. This is
 * a custom HttpsURLConnection built on top of JSSE to provide additional
 * proxy and cookie support.
 */
final class PluginDelegateHttpsURLConnection extends DelegateHttpsURLConnection
{
    // Proxy setting for this connection
    private String proxy = null;
    private int proxyPort = -1;
    private MessageHeader cachedHeaders = null;
    private boolean progressTracking = false;
	private int callCount = 0;

    // Message format
    private static MessageFormat mf1 = new MessageFormat(ResourceHandler.getMessage("net.connect.no_proxy"));
    private static MessageFormat mf2 = new MessageFormat(ResourceHandler.getMessage("net.connect.proxy_is"));

    private static Method New;

    static {
	try {
	    //This code is to obtain access to a protected method called New in HttpsClient class
	    New = (Method)AccessController.doPrivileged(new PrivilegedExceptionAction() {
	        public Object run() throws NoSuchMethodException {
		    Method ms[];
		    ms = HttpsClient.class.getDeclaredMethods();
		    Class[] paramTypes = {  SSLSocketFactory.class, URL.class, 
					    HostnameVerifier.class, String.class, 
					    Integer.TYPE, Boolean.TYPE };
		    //Method names are obfuscated, therefore matching the parameter types			
		    for(int i=0;i<ms.length;i++) {
			Class params[] = ms[i].getParameterTypes();
			if(params.length == paramTypes.length) {
			    boolean matchFound = true;
			    for(int j=0;j<paramTypes.length;j++) {
				if(paramTypes[j] != params[j]) {
				    matchFound = false;
				    break;
				}
			    }
			    if(matchFound == true) {
				ms[i].setAccessible(true);
				return ms[i];
			    }
			}
		    }
		    return null;
		}
	    });
	} catch (PrivilegedActionException e) {
	    Trace.netPrintException(e);
	}  
    }

    /**
     * Construct a PluginHttpsURLConnection object.
     * 
     * @param u URL of the connection.
     * @param handler Protocol handler.
     */
    public PluginDelegateHttpsURLConnection(URL u, Handler handler, javax.net.ssl.HttpsURLConnection conn) throws IOException 
    {
	super(u, handler, conn);
    }

    // true if the security check n has been made
    private boolean securityCheck = false;

    // true if the cache connection has been made
    private boolean cacheConnect = false;

    // true if the real connection has been made
    private boolean realConnect = false;

    // true if the set-cookie header has been checked
    private boolean checkSetCookie = false;


    public synchronized void connect() throws IOException 
    {

	if (isConnected())
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
	connectWithCache();

	// If cache is disabled, make a connection
	if (cacheStream == null) 
	{
		// Setup real connection
		connectSetup();

		// Call super.connect()    
		superConnect();
	}

	setConnected(true);
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
     * Perform steps to establish a connection: determine if the
     * file is already in cache.
     */     
    private void connectWithCache() throws IOException 
    {
	if (cacheConnect)
	    return;

	if (getUseCaches() && getRequestMethod().equalsIgnoreCase("GET")) 
	{
    	    try {
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

			if (progressTracking)
			    cacheStream = new ProgressInputStream(is, this);
			else
			    cacheStream = is;
		    }
		}
	    } catch (IOException exc) { //IOException. fall through, and try
		//remote
		Trace.netPrintException(exc);
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
     * Perform steps to establish a connection: determine proxy/cookie
     * setting, and setup a real connection.
     */     
    private void connectSetup() throws IOException {

	if (realConnect)
	    return;

	// Determine proxy setting for the connection

        if (proxy == null) 
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


    /**
     * Call super.connect()
     */
    private void superConnect() throws IOException
    {
	if (isConnected())
	    return;

	// The underlying APIs provides access to cached connection
	//
	if (proxy == null)
	    setNewClient(url, true);
	else
	    setProxiedClient(url, proxy, proxyPort, true);
    }

    protected void proxiedConnect(URL url, String proxyHost, int proxyPort,
            boolean useCache) throws IOException {

        if (isConnected())
            return;

        SSLSocketFactory sf = getSSLSocketFactory();
	HostnameVerifier hv = getHostnameVerifier();
	Object[] params = {sf, url, hv, proxyHost,
	    new Integer(proxyPort), new Boolean(useCache)};
	try {
	    http = (HttpsClient) New.invoke(null, params);
	} catch (IllegalAccessException iae) {
	    // can't happen
	    Error err = new IllegalAccessError();
	    err.initCause(iae);
	    throw err;
	} catch (InvocationTargetException ite) {
	    // XXX unexpected Throwables
	    throw (IOException)ite.getTargetException();
	}

        setConnected(true);
    }


    private InputStream cacheStream = null;

    /**
     * Returns an input stream.
     */
    public synchronized InputStream getInputStream() throws IOException
    {
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
     * Determine is proxy is used.
     *
     * @return true if proxy is used.
     */
    public boolean usingProxy() 
    {
	ProxyInfo info = PluginProxyManager.getProxyInfo(url);

	if (info !=  null && info.getProxy() != null)
	    return true;
	else
	    return false;
    }


    /**
     * Check cookie header after connection is made.
     */
    private synchronized void checkCookieHeader()
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
}


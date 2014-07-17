/*
 *  @(#)SmartProxyHandler.java	1.11 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.net.proxy;

import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.URL;
import java.net.Socket;
import java.util.HashMap;
import java.util.Calendar;
import java.security.AccessController;
import java.security.PrivilegedAction;
import java.io.IOException;
import sun.plugin.util.Trace;


/**
 * Samrt Proxy handler for auto proxy configuration.
 */
final class SmartProxyHandler implements ProxyHandler
{

    // The decorated ProxyHandler;
    private ProxyHandler realHandler = null;
    private HashMap      proxyCache = new HashMap();
    // The three timeout value is associated with
    // pingTimeout: the timeout when we do ping to the proxy server
    // aliveTimeout: the timeout when we re-ping the alive proxy server
    // deadTimeout: the timeout when we re-ping the dead proxy server
    private int   pingTimeout;
    private long  aliveTimeout;
    private long  deadTimeout;
   
    /**
     *
     */
    public SmartProxyHandler(ProxyHandler realHandler)
    {
	// Prepopulate the proxy cache with "DIRECT" infor
        proxyCache.put(new ProxyInfo(null, -1), new ProxyRecord(true, -1));

	AccessController.doPrivileged(new PrivilegedAction() {
	    public Object run() {
		pingTimeout = (Integer.getInteger("javaplugin.proxy.pingTimeout", 2000)).intValue();
	        aliveTimeout = (Long.getLong("javaplugin.proxy.aliveTimeout", 1800000)).longValue();
		deadTimeout = (Long.getLong("javaplugin.proxy.deadTimeout", 600000)).longValue();
	        return null;
	    }
	    });
	this.realHandler = realHandler;
    }

    /**
     * Check if the proxy handler supports the proxy type
     *
     * @param proxyType Proxy type
     * @return true if proxy type is supported
     */
    public boolean isSupported(int proxyType)
    {
	return realHandler.isSupported(proxyType);
    }

    /**
     * Check if the proxy result should be cached
     *
     * @return true if proxy result should be cached
     */
    public boolean isProxyCacheSupported()
    {
	return realHandler.isProxyCacheSupported();
    }

    /**
     * Initialize the auto proxy handler.
     *
     * @param info Browser proxy info
     */
    public void init(BrowserProxyInfo info)
		throws ProxyConfigException
    {
	realHandler.init(info);
	return;
    }

    /**
     * ProxyRecord is used to represent the entry in the proxy cache
     *
     */
    private class ProxyRecord
    {
	// if status is true means proxy is alive.
	private boolean isAlive = false;
	private long    timeStamp = 0;

	/**
	 *
	 */
        public ProxyRecord(boolean isAlive, long timeStamp)
	{
	    this.isAlive = isAlive;
	    this.timeStamp = timeStamp;
	}

	/**
	 * Retrieve the status of proxy
	 *
	 * @return true means proxy is alive
	 */
	public boolean getStatus()
	{
	    return isAlive;
	}

	/**
	 * Retrieve the times stamp of the proxy
	 *
	 * @return the timeStamp of the last ping
	 */
	public long  getTimeStamp()
	{
	    return timeStamp;
	}

	/**
	 * Set the status of the proxy
	 *
	 * @param isAlive
	 */
        public void setStatus(boolean isAlive)
	{
	    this.isAlive = isAlive;
	}

	/**
	 * Set the time stamp of the proxy entry
	 *
	 * @param timeStamp
	 */
	public void setTimeStamp(long timeStamp)
	{
	    this.timeStamp = timeStamp;
	}
    }

    /**
     * Returns proxy info for a given URL
     *
     * The proxy detection algorithm is:
     *  Given a proxy server list p (proxy server/socks server)
     *  If pi is not in the proxy cache (proxyCache)
     *     Ping pi with a certain timeout specified with pingTimeout
     *     put it into the proxyCache, if it is alive, return
     *     otherwise continue searching
     *  If pi is timeout (certainly it must be in proxyCache
     *     Ping pi and update the record
     *     if pi is alive, return, otherwise, continue
     *  If pi is alive, return
     *  Else continue
     * If no alive proxy found, we use the first entry in the array.
     *
     * @param u URL
     * @return proxy info for a given URL
     */
    public ProxyInfo[] getProxyInfo(URL u)
		     throws sun.plugin.services.ServiceUnavailableException
    {
	ProxyInfo[] proxyList = realHandler.getProxyInfo(u);

	for (int i = 0; i < proxyList.length; i++)
	{
	    String cacheKey = proxyList[i].toString();
            ProxyRecord proxyRecord = (ProxyRecord)(proxyCache.get(cacheKey));
	    // if proxy is not in cache
            if (proxyRecord == null)
	    {
		ProxyRecord newRecord = new ProxyRecord(false, Calendar.getInstance().getTimeInMillis());
		proxyCache.put(cacheKey, newRecord);

		if (pingProxy(proxyList[i]) == true)
		{
		    // The proxy is alive, store in proxy cache and return
                    newRecord.setStatus(true);
		    return new ProxyInfo[] {proxyList[i]};
		}
	    }
	    else
	    {
		long currentTime = Calendar.getInstance().getTimeInMillis();
		boolean status = proxyRecord.getStatus();
		long timeStamp = proxyRecord.getTimeStamp();
		// The timeStamp could be negative only for the prepopulated entry "DIRECT".
		if (timeStamp > 0 &&
                    ((status == true && (currentTime - timeStamp) >= aliveTimeout) ||
		     (status == false && (currentTime - timeStamp) >= deadTimeout)))
		{
                        //If timeout, ping it and update the record.
			status = pingProxy(proxyList[i]);
                        proxyRecord.setTimeStamp(currentTime);
                        proxyRecord.setStatus(status);
		}
	        if (status == true)
		    return new ProxyInfo[] {proxyList[i]};
	    }
	}
	// Return the first proxy if none of them are reachable;
	return new ProxyInfo[] {proxyList[0]};
    }

    /**
     * Ping the proxy server to see if it is still alive
     *
     * @param ProxyInfo
     * @return boolean
     * if return true, the proxy is alive, otherwise it is dead probably
     */
    private boolean pingProxy(ProxyInfo proxy)
    {
        String host = proxy.getProxy();
        int    port = -1;
	// Three possblities: PROXY/SOCKS/DIRECT
	if (host != null) // "PROXY" specified
	{
	    port = proxy.getPort();
	}
	else
	{
	    host = proxy.getSocksProxy();
	    if (host != null)  // "SOCKS specified"
		port = proxy.getSocksPort();
	    else
		return true; // "DIRECT" spcified
	}
        final String proxyHost = host;
	final int proxyPort = port;
       
        Boolean isConnectable = (Boolean)(
	AccessController.doPrivileged(new PrivilegedAction() {
	    public Object run() {
		Object[] args = {proxyHost, new Integer(proxyPort)};
		Socket proxySo = new Socket();
		Trace.msgNetPrintln("net.proxy.browser.smartConfig", args);
		try{
		   
		    proxySo.connect(new InetSocketAddress(proxyHost, proxyPort),
				    pingTimeout);
		    return Boolean.TRUE;

		}
		catch(Exception e)
		{
		    
		    Trace.msgNetPrintln("net.proxy.browser.connectionException", args);
		}
		finally {
		    try {
			proxySo.close();
		    }
		    catch(IOException ex)
		    {
		    }
		}
		return Boolean.FALSE;
	    }

	})
	);
	return isConnectable.booleanValue();
    }
}

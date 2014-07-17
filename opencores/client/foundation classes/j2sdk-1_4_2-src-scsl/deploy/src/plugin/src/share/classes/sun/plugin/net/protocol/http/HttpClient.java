/*
 * @(#)HttpClient.java	1.26 03/06/18
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.net.protocol.http;

import java.io.IOException;
import java.io.InputStream;
import java.net.Socket;
import java.net.URL;
import java.net.UnknownHostException;
import java.util.ResourceBundle;
import sun.plugin.net.protocol.socks.SocksSocket;
import sun.plugin.net.proxy.PluginProxyManager;
import sun.plugin.net.proxy.ProxyInfo;
import java.security.AccessController;
import java.security.PrivilegedExceptionAction;
import java.security.PrivilegedActionException;

/**
 * @author  Stanley Man-Kit Ho
 */
public final class HttpClient extends sun.net.www.http.HttpClient 
{
    // true if proxy should be resolved
    private boolean resolveProxy = true;

    /* This package-only CTOR should only be used for FTP piggy-backed on HTTP 
     * HTTP URL's that use this won't take advantage of keep-alive.
     * Additionally, this constructor may be used as a last resort when the
     * first HttpClient gotten through New() failed (probably b/c of a 
     * Keep-Alive mismatch).
     *
     * XXX That documentation is wrong ... it's not package-private any more
     */
    public HttpClient(URL url, String proxy, int proxyPort, boolean resolveProxy)
			throws IOException {
	super (url, proxy, proxyPort);

	this.resolveProxy = resolveProxy;
    }

    /* This class has no public constructor for HTTP.  This method is used to
     * get an HttpClient to the specifed URL.  If there's currently an 
     * active HttpClient to that server/port, you'll get that one.
     */
    public static sun.net.www.http.HttpClient New(URL url, String proxy, int proxyPort, boolean resolveProxy) 
    throws IOException {
	/* see if one's already around */
	//HttpClient ret = (HttpClient) kac.get(url);

	Object obj = kac.get(url, null);
	HttpClient ret = null;

	if (obj instanceof HttpClient)
	    ret = (HttpClient) obj;

	if (ret == null) {
	    ret = new HttpClient(url, proxy, proxyPort, resolveProxy);  // CTOR called openServer()
	} else {
	    SecurityManager security = System.getSecurityManager();
	    if (security != null) {
		security.checkConnect(url.getHost(), url.getPort());
	    }
	    ret.url = url;
	}
	// don't know if we're keeping alive until we parse the headers
	// for now, keepingAlive is false
	return ret;
    }

    /**
     * Return a socket connected to the server, with any
     * appropriate options pre-established
     */
    protected Socket doConnect (final String server, final int port)
    throws IOException, UnknownHostException 
    {
	if (resolveProxy)
	{
	    final ProxyInfo pinfo = PluginProxyManager.getProxyInfo(url);

	    if (pinfo != null && pinfo.isSocksUsed())
	    {
		// Use SOCKS !!
		try {

		    return (Socket)java.security.AccessController.doPrivileged(new PrivilegedExceptionAction()
		    {
			public Object run() throws IOException
			{
			    return new SocksSocket(server, port, pinfo.getSocksProxy(), pinfo.getSocksPort());
			}
		    });
		} catch (PrivilegedActionException e) {
		    IOException ioe = (IOException)e.getException();
		    throw ioe;
		}  

	    }
	}

	return super.doConnect(server, port);
    }
}

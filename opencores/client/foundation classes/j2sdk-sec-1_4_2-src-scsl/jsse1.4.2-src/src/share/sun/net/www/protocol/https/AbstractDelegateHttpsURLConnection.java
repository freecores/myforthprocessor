/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */
 
package sun.net.www.protocol.https;

import java.net.URL;
import java.io.IOException;
import javax.net.ssl.SSLPeerUnverifiedException;
import sun.net.www.http.*;
import sun.net.www.protocol.http.HttpURLConnection;

/**
 * HTTPS URL connection support. 
 * We need this delegate because HttpsURLConnection is a subclass of 
 * java.net.HttpURLConnection. We will avoid copying over the code from 
 * sun.net.www.protocol.http.HttpURLConnection by having this class 
 *
 * @version 1.9 06/24/03
 */
public abstract class AbstractDelegateHttpsURLConnection extends 
	HttpURLConnection {

    protected AbstractDelegateHttpsURLConnection(URL url, 
	    sun.net.www.protocol.http.Handler handler) throws IOException {
	super(url, handler);
    }
    
    protected abstract javax.net.ssl.SSLSocketFactory getSSLSocketFactory();
    
    protected abstract javax.net.ssl.HostnameVerifier getHostnameVerifier();

    /**
     * No user application is able to call these routines, as no one
     * should ever get access to an instance of
     * DelegateHttpsURLConnection (sun.* or com.*)
     */

    /**
     * Create a new HttpClient object, bypassing the cache of
     * HTTP client objects/connections.
     *
     * Note: this method is changed from protected to public because
     * the com.sun.ssl.internal.www.protocol.https handler reuses this
     * class for its actual implemantation
     *
     * @param url the URL being accessed
     */
    public void setNewClient (URL url)
	throws IOException {
 	setNewClient (url, false);
    }
 
    /**
     * Obtain a HttpClient object. Use the cached copy if specified. 
     *
     * Note: this method is changed from protected to public because
     * the com.sun.ssl.internal.www.protocol.https handler reuses this
     * class for its actual implemantation
     *
     * @param url       the URL being accessed
     * @param useCache  whether the cached connection should be used
     *        if present
     */
    public void setNewClient (URL url, boolean useCache)
	throws IOException {
	http = HttpsClient.New (getSSLSocketFactory(),
				url,
				getHostnameVerifier(),
				useCache);
	((HttpsClient)http).afterConnect();
    }

    /**
     * Create a new HttpClient object, set up so that it uses
     * per-instance proxying to the given HTTP proxy.  This
     * bypasses the cache of HTTP client objects/connections.
     *
     * Note: this method is changed from protected to public because
     * the com.sun.ssl.internal.www.protocol.https handler reuses this
     * class for its actual implemantation
     *
     * @param url	the URL being accessed
     * @param proxyHost	the proxy host to use
     * @param proxyPort	the proxy port to use
     */
    public void setProxiedClient (URL url, String proxyHost, int proxyPort)
 	    throws IOException {
 	setProxiedClient(url, proxyHost, proxyPort, false);
    }
    
    /**
     * Obtain a HttpClient object, set up so that it uses per-instance
     * proxying to the given HTTP proxy. Use the cached copy of HTTP
     * client objects/connections if specified.
     *
     * Note: this method is changed from protected to public because
     * the com.sun.ssl.internal.www.protocol.https handler reuses this
     * class for its actual implemantation
     *
     * @param url       the URL being accessed
     * @param proxyHost the proxy host to use
     * @param proxyPort the proxy port to use
     * @param useCache  whether the cached connection should be used
     *        if present
     */
    public void setProxiedClient (URL url, String proxyHost, int proxyPort,
	    boolean useCache) throws IOException {
	proxiedConnect(url, proxyHost, proxyPort, useCache);
	if (!http.isCachedConnection()) {
	    doTunneling();
	}       
	((HttpsClient)http).afterConnect();
    }

    protected void proxiedConnect(URL url, String proxyHost, int proxyPort,
	    boolean useCache) throws IOException {
	if (connected) 
	    return;
	SecurityManager security = System.getSecurityManager(); 
	if (security != null) { 
	    security.checkConnect(proxyHost, proxyPort); 
	}
	http = HttpsClient.New (getSSLSocketFactory(),
				url,
				getHostnameVerifier(), 
				proxyHost, proxyPort, useCache);
	connected = true;
    }

    /**
     * Used by subclass to access "connected" variable.
     */
    public boolean isConnected() {
	return connected;
    }

    /**
     * Used by subclass to access "connected" variable.
     */
    public void setConnected(boolean conn) {
	connected = conn;
    }

    /**
     * Implements the HTTP protocol handler's "connect" method,
     * establishing an SSL connection to the server as necessary.
     */
    public void connect() throws IOException {
	if (connected)
	    return;
	plainConnect();
	if (!http.isCachedConnection() && http.needsTunneling()) {
	    doTunneling();
	}       
	((HttpsClient)http).afterConnect();
    }
    
    protected void plainConnect() throws IOException {
	if (connected)
	    return;
	http = (HttpsClient)HttpsClient.New (
			  getSSLSocketFactory(),
			  url,
			  getHostnameVerifier());
	connected = true;
    }

    /**
     * Returns the cipher suite in use on this connection.
     */
    public String getCipherSuite () {
	if (http == null) {
	    throw new IllegalStateException("connection not yet open");
	} else {
	   return ((HttpsClient)http).getCipherSuite ();
	}	
    }
    
    /**
     * Returns the certificate chain the client sent to the
     * server, or null if the client did not authenticate.
     */
    public java.security.cert.Certificate[] getLocalCertificates() {
 	if (http == null) {
 	    throw new IllegalStateException("connection not yet open");
 	} else {
 	    return (((HttpsClient)http).getLocalCertificates ());
 	}
    }
    
    /**
     * Returns the server's certificate chain, or throws
     * SSLPeerUnverified Exception if
     * the server did not authenticate.
     */
    public java.security.cert.Certificate[] getServerCertificates() 
	    throws SSLPeerUnverifiedException {
 	if (http == null) {
 	    throw new IllegalStateException("connection not yet open");
 	} else {
 	    return (((HttpsClient)http).getServerCertificates ());
 	}
    }

    /**
     * Returns the server's X.509 certificate chain, or null if
     * the server did not authenticate.
     */
    public javax.security.cert.X509Certificate[] getServerCertificateChain()
	    throws SSLPeerUnverifiedException {
	if (http == null) {
	    throw new IllegalStateException("connection not yet open");
	} else {
	    return ((HttpsClient)http).getServerCertificateChain ();
	}
    }
}

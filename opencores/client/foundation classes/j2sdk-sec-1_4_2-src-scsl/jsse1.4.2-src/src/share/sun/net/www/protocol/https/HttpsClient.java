/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */


package sun.net.www.protocol.https;

import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.FileInputStream;
import java.io.PrintStream;
import java.io.BufferedOutputStream;
import java.net.Socket;
import java.net.URL;
import java.net.UnknownHostException;
import java.net.InetAddress;
import java.net.Authenticator;
import java.net.PasswordAuthentication;
import java.security.Principal;
import java.security.KeyStore;
import java.security.PrivateKey;
import java.security.cert.*;
import java.util.StringTokenizer;
import java.util.Vector;
import java.util.Collection;
import java.util.List;
import java.util.Iterator;
import java.security.AccessController;

import javax.security.auth.x500.X500Principal;

import javax.net.ssl.*;
import sun.security.x509.X500Name;
import sun.misc.Regexp;
import sun.misc.RegexpPool;
import sun.net.www.HeaderParser;
import sun.net.www.MessageHeader;
import sun.net.www.http.HttpClient;
import sun.security.action.*;

import sun.security.util.HostnameChecker;

import com.sun.net.ssl.internal.ssl.ExportControl;
import com.sun.net.ssl.internal.ssl.SSLSocketFactoryImpl;


/**
 * This class provides HTTPS client URL support, building on the standard
 * "sun.net.www" HTTP protocol handler.  HTTPS is the same protocol as HTTP,
 * but differs in the transport layer which it uses:  <UL>
 *
 *	<LI>There's a <em>Secure Sockets Layer</em> between TCP
 *	and the HTTP protocol code.
 *
 *	<LI>It uses a different default TCP port.
 *
 *	<LI>It doesn't use application level proxies, which can see and
 *	manipulate HTTP user level data, compromising privacy.  It uses
 *	low level tunneling instead, which hides HTTP protocol and data
 *	from all third parties.  (Traffic analysis is still possible).
 *
 *	<LI>It does basic server authentication, to protect
 *	against "URL spoofing" attacks.  This involves deciding
 *	whether the X.509 certificate chain identifying the server
 *	is trusted, and verifying that the name of the server is
 *	found in the certificate.  (The application may enable an
 *	anonymous SSL cipher suite, and such checks are not done
 *	for anonymous ciphers.)
 *
 *	<LI>It exposes key SSL session attributes, specifically the
 *	cipher suite in use and the server's X509 certificates, to
 *	application software which knows about this protocol handler.
 *
 *	</UL>
 *
 * <P> System properties used include:  <UL>
 *
 *	<LI><em>https.proxyHost</em> ... the host supporting SSL
 *	tunneling using the conventional CONNECT syntax
 *
 *	<LI><em>https.proxyPort</em> ... port to use on proxyHost
 *
 *	<LI><em>https.cipherSuites</em> ... comma separated list of
 *	SSL cipher suite names to enable.
 *
 *	<LI><em>http.nonProxyHosts</em> ...
 *
 *	</UL>
 *
 * @version 1.14 06/24/03
 * @author David Brownell
 * @author Bill Foote
 */

// final for export control reasons (access to APIs); remove with care
final class HttpsClient extends HttpClient
    implements HandshakeCompletedListener
{
    // STATIC STATE and ACCESSORS THERETO

    // HTTPS uses a different default port number than HTTP.
    private static final int	httpsPortNumber = 443;

    /** Returns the default HTTPS port (443) */
    protected int getDefaultPort() { return httpsPortNumber; }

    private HostnameVerifier hv;
    private SSLSocketFactory sslSocketFactory;

    // HttpClient.proxyDisabled will always be false, because we don't
    // use an application-level HTTP proxy.  We might tunnel through
    // our http proxy, though.


    // INSTANCE DATA

    // tunnel host/port for this instance.  instTunnelHost will be null
    // if tunneling is turned off for this client for any reason.
    private String	instTunnelHost;
    private int		instTunnelPort;

    // last negotiated SSL session
    private SSLSession	session;


    private String getProxyHost() {
	String host = (String) java.security.AccessController.doPrivileged(
                new sun.security.action.GetPropertyAction("https.proxyHost"));
	if (host != null && host.length() == 0) {
	    host = null;
	}
	return host;
    }

    private int getProxyPort() {
	final int port[] = {0};
	java.security.AccessController.doPrivileged(
	    new java.security.PrivilegedAction() {
	    public Object run() {
		if (System.getProperty("https.proxyHost") != null) {
		    port[0] =
			Integer.getInteger("https.proxyPort", 80).intValue();
		}
		return null;
	    }
	});
	return (port[0] < 0) ? super.getDefaultPort(): port[0];
    }

    private String [] getCipherSuites() {
	//
	// If ciphers are assigned, sort them into an array.
	//
        String ciphers [];
	String cipherString = (String) AccessController.doPrivileged(
                new GetPropertyAction("https.cipherSuites"));

	if (cipherString == null || "".equals(cipherString)) {
	    ciphers = null;
	} else {
	    StringTokenizer	tokenizer;
	    Vector		v = new Vector();

	    tokenizer = new StringTokenizer(cipherString, ",");
	    while (tokenizer.hasMoreElements())
		v.addElement(tokenizer.nextElement());
	    ciphers = new String [v.size()];
	    for (int i = 0; i < ciphers.length; i++)
		ciphers [i] = (String) v.elementAt(i);
	}
	return ciphers;
    }

    private String [] getProtocols() {
	//
	// If protocols are assigned, sort them into an array.
	//
        String protocols [];
	String protocolString = (String) AccessController.doPrivileged(
                new GetPropertyAction("https.protocols"));

	if (protocolString == null || "".equals(protocolString)) {
	    protocols = null;
	} else {
	    StringTokenizer	tokenizer;
	    Vector		v = new Vector();

	    tokenizer = new StringTokenizer(protocolString, ",");
	    while (tokenizer.hasMoreElements())
		v.addElement(tokenizer.nextElement());
	    protocols = new String [v.size()];
	    for (int i = 0; i < protocols.length; i++) {
		protocols [i] = (String) v.elementAt(i);
	    }
	}
	return protocols;
    }

    private RegexpPool getDontProxy() {
        RegexpPool dontProxy = new RegexpPool();
	String rawList = (String) AccessController.doPrivileged(
                new GetPropertyAction("http.nonProxyHosts"));
	if (rawList != null) {
	    java.util.StringTokenizer st
		= new java.util.StringTokenizer(rawList, "|", false);
	    try {
		while (st.hasMoreTokens()) {
		    dontProxy.add(st.nextToken().toLowerCase(),
			    new Boolean(true));
		}
	    } catch (Exception e) {
		e.printStackTrace();
	    }
	}
        return dontProxy;
    }

    private String getUserAgent() {
	String userAgent = (String) java.security.AccessController.doPrivileged(
                new sun.security.action.GetPropertyAction("https.agent"));
	if (userAgent == null || userAgent.length() == 0) {
	    userAgent = "JSSE";
	}
	return userAgent;
    }

    // CONSTRUCTOR, FACTORY


    /**
     * Create an HTTPS client URL.  Traffic will be tunneled through any
     * intermediate nodes rather than proxied, so that confidentiality
     * of data exchanged can be preserved.  However, note that all the
     * anonymous SSL flavors are subject to "person-in-the-middle"
     * attacks against confidentiality.  If you enable use of those
     * flavors, you may be giving up the protection you get through
     * SSL tunneling.
     *
     * Use New to get new HttpsClient. This constructor is meant to be
     * used only by New method. New properly checks for URL spoofing.
     *
     * @param URL https URL with which a connection must be established
     */
    private HttpsClient(SSLSocketFactory sf, URL url)
    throws IOException
    {
	// HttpClient-level proxying is always disabled,
	// because we override doConnect to do tunneling instead.
	this(sf, url, (String)null, -1); 
    }

    /**
     *  Create an HTTPS client URL.  Traffic will be tunneled through
     * the specified proxy server.
     */
    HttpsClient(SSLSocketFactory sf, URL url, String proxy, int proxyPort)
	throws IOException {
	setSSLSocketFactory(sf); 
	if (proxy != null) { 
	    setTunnelProxy(proxy, proxyPort); 
	}
	this.proxyDisabled = true;
	/*
	 *try to set host to "%d.%d.%d.%d" string if
	 * visible - Sprint bug - brown
	 */
	try {
	    InetAddress addr = InetAddress.getByName(url.getHost());
	    this.host = addr.getHostAddress();
	} catch (UnknownHostException ignored) {
	    this.host = url.getHost();
	}
	this.url = url;
	port = url.getPort();
	if (port == -1) {
	    port = getDefaultPort();
	}
	openServer();
    }


    // This code largely ripped off from HttpClient.New, and
    // it uses the same keepalive cache.

    static HttpClient New(SSLSocketFactory sf, URL url, HostnameVerifier hv)
	    throws IOException {
	return HttpsClient.New(sf, url, hv, true);
    }

    /** See HttpClient for the model for this method. */
    static HttpClient New(SSLSocketFactory sf, URL url,
	    HostnameVerifier hv, boolean useCache) throws IOException {
	return HttpsClient.New(sf, url, hv, (String)null, -1, useCache);
    }

    /**
     * Get a HTTPS client to the URL.  Traffic will be tunneled through
     * the specified proxy server.
     */
    static HttpClient New(SSLSocketFactory sf, URL url, HostnameVerifier hv,
			   String proxy, int proxyPort) throws IOException {
	return HttpsClient.New(sf, url, hv, proxy, proxyPort, true);
    }

    static HttpClient New(SSLSocketFactory sf, URL url, HostnameVerifier hv,
			   String proxy, int proxyPort, boolean useCache)
	throws IOException {
	HttpsClient ret = null;
	if (useCache) {
	    /* see if one's already around */
	    ret = (HttpsClient) kac.get(url, sf);
	    if (ret != null) {
		ret.cachedHttpClient = true;
	    }
	}
	if (ret == null) {
	    ret = new HttpsClient(sf, url, proxy, proxyPort);
	} else {
	    SecurityManager security = System.getSecurityManager();
	    if (security != null) {
		security.checkConnect(url.getHost(), url.getPort());
	    }
	    ret.url = url;
	}
	ret.setHostnameVerifier(hv); 

	return ret;
    }

    // METHODS
    void setHostnameVerifier(HostnameVerifier hv) {
	this.hv = hv;
    }
    
    void setSSLSocketFactory(SSLSocketFactory sf) {
	sslSocketFactory = sf;
    }

    void setTunnelProxy(String proxy, int proxyPort) {
	instTunnelHost = proxy;
    	instTunnelPort =  (proxyPort < 0)
	   ? getDefaultPort()
	   : proxyPort;
    }

    SSLSocketFactory getSSLSocketFactory() {
	return sslSocketFactory;
    }

    /**
     * Returns true if host is on the "don't proxy" list.
     */
    private boolean isNonProxyHost() {
	RegexpPool dontProxy = getDontProxy();
	if (dontProxy.match(url.getHost().toLowerCase()) != null) {
	    return true;
	}
	try {
	    InetAddress addr = InetAddress.getByName(url.getHost());
	    String host = addr.getHostAddress();
	    if (dontProxy.match(host) != null) {
		return true;
	    }
	} catch (UnknownHostException ignored) {
	}
	return false;
    }

    public boolean needsTunneling() {
	return (instTunnelHost != null && !isNonProxyHost());
    }

    /**
     * Overrides HTTP protocol handler method so that we return an SSL
     * socket, not a TCP socket.  This establishes a secure tunnel if
     * appropriate.
     *
     * @param host the host to connect to
     * @param port the port on that host.
     * @exception IOException on errors including a host doesn't
     *	authenicate corectly.
     * @exception UnknownHostException if "host" is unknown
     */
    protected Socket doConnect(String host, int port)
    throws IOException, UnknownHostException
    {
	// doConnect() ends up getting called from the
	// HttpClient constructor, so our constructor can't do any
	// meaningful initialization.  This could be made nicer
	// by refactoring HttpClient and HttpsClient so that they
	// have a common base class.
	instTunnelHost = ((instTunnelHost != null) ?
	    instTunnelHost : getProxyHost());
	instTunnelPort = ((instTunnelPort != 0) ?
	    instTunnelPort : getProxyPort());

	Socket s = null;
	SSLSocketFactory factory;
	factory = sslSocketFactory;

	if (instTunnelHost == null || isNonProxyHost()) {
	    s = factory.createSocket(host, port);
	} else {
	    try {
		s =
		    (Socket) java.security.AccessController.doPrivileged(
			new java.security.PrivilegedExceptionAction() {
			public Object run() throws IOException {
			    return new Socket(instTunnelHost, instTunnelPort);
			}
		    });
	    } catch (java.security.PrivilegedActionException pae) {
		throw(IOException) pae.getException();
	    } catch (IOException ex) {
		// If we fail to connect through the tunnel, try it
		// locally, as a last resort.  If this doesn't work,
		// throw the original exception.
		try {
		    s = (SSLSocket)factory.createSocket(host, port);
		} catch (IOException ignored) {
		    throw ex;
		}
	    }
	}
	return s;
    }

    public void afterConnect() throws IOException, UnknownHostException {
	if (!isCachedConnection()) {
	    SSLSocket s = null;
	    SSLSocketFactory factory;
	    factory = sslSocketFactory;
	    try {
		if (!(serverSocket instanceof SSLSocket)) {
		    s = (SSLSocket)factory.createSocket(serverSocket,
							host, port, true);
		} else {
		    s = (SSLSocket)serverSocket;
		}
	    } catch (IOException ex) {
		// If we fail to connect through the tunnel, try it
		// locally, as a last resort.  If this doesn't work,
		// throw the original exception.
		try {
		    s = (SSLSocket)factory.createSocket(host, port);
		} catch (IOException ignored) {
		    throw ex;
		}
	    }
	    
	    SSLSocketFactoryImpl.checkCreate(s);
	    
	    //
	    // Force handshaking, so that we get any authentication.
	    // Register a handshake callback so our session state tracks any
	    // later session renegotiations.
	    //
	    String [] protocols = getProtocols();
	    String [] ciphers = getCipherSuites();
	    if (protocols != null)
		s.setEnabledProtocols(protocols);
	    if (ciphers != null)
		s.setEnabledCipherSuites(ciphers);
	    s.addHandshakeCompletedListener(this);
	    s.startHandshake();
	    session = s.getSession();
	    // change the serverSocket and serverOutput
	    serverSocket = s;
	    try {
		serverOutput = new PrintStream(
		    new BufferedOutputStream(serverSocket.getOutputStream()), 
		    false, encoding);
	    } catch (UnsupportedEncodingException e) {
		throw new InternalError(encoding+" encoding not found");
	    }
	
	    // check URL spoofing 
	    checkURLSpoofing(hv); 
	} else {
	    // if we are reusing a cached https session,
	    // we don't need to do handshaking etc. But we do need to 
	    // set the ssl session
	    session = ((SSLSocket)serverSocket).getSession();
	}
    }

    // Server identity checking is done according to RFC 2818: HTTP over TLS
    // Section 3.1 Server Identity
    private void checkURLSpoofing(HostnameVerifier hostnameVerifier)
	    throws IOException
    {
	//
	// Get authenticated server name, if any
	//
	boolean done = false;
	String host = url.getHost();

	// if IPv6 strip off the "[]"
	if (host != null && host.startsWith("[") && host.endsWith("]")) {
	    host = host.substring(1, host.length()-1);
	}

	Certificate[] peerCerts = null;
	try {
	    // get the subject's certificate
	    peerCerts = session.getPeerCertificates();

	    X509Certificate peerCert;
	    if (peerCerts [0] instanceof java.security.cert.X509Certificate) {
		peerCert = (java.security.cert.X509Certificate) peerCerts [0];
	    } else {
		throw new SSLPeerUnverifiedException("");
	    }
	    
	    HostnameChecker checker = HostnameChecker.getInstance(
	    					HostnameChecker.TYPE_TLS);
	    
	    checker.match(host, peerCert);

	    // if it doesn't throw an exception, we passed. Return.
	    return;
	    
	} catch (SSLPeerUnverifiedException e) {

	    //
	    // client explicitly changed default policy and enabled
	    // anonymous ciphers; we can't check the standard policy
	    //
	    // ignore
	} catch (java.security.cert.CertificateException cpe) {
	    // ignore
	}

	// Assume the peerCerts are already cloned.
	String cipher = session.getCipherSuite();
	if ((cipher != null) && (cipher.indexOf("_anon_") != -1)) {
	    return;
	} else if ((hostnameVerifier != null) &&
                   (hostnameVerifier.verify(host, session))) {
	    return;
	}

	serverSocket.close();
	session.invalidate();

	throw new IOException("HTTPS hostname wrong:  should be <"
			      + url.getHost() + ">");
    }

    protected void putInKeepAliveCache() {
	kac.put(url, sslSocketFactory, this);
    }

    /**
     * Returns the cipher suite in use on this connection.
     */
    String getCipherSuite() {
	return session.getCipherSuite();
    }

    /**
     * Returns the certificate chain the client sent to the
     * server, or null if the client did not authenticate.
     */
    public java.security.cert.Certificate [] getLocalCertificates() {
	return session.getLocalCertificates();
    }

    /**
     * Returns the certificate chain with which the server
     * authenticated itself, or throw a SSLPeerUnverifiedException
     * if the server did not authenticate.
     */
    java.security.cert.Certificate [] getServerCertificates()
	    throws SSLPeerUnverifiedException
    {
	return session.getPeerCertificates();
    }

    /**
     * Returns the X.509 certificate chain with which the server
     * authenticated itself, or null if the server did not authenticate.
     */
    javax.security.cert.X509Certificate [] getServerCertificateChain()
	    throws SSLPeerUnverifiedException
    {
	return session.getPeerCertificateChain();
    }

    /**
     * This method implements the SSL HandshakeCompleted callback,
     * remembering the resulting session so that it may be queried
     * for the current cipher suite and peer certificates.  Servers
     * sometimes re-initiate handshaking, so the session in use on
     * a given connection may change.  When sessions change, so may
     * peer identities and cipher suites.
     */
    public void handshakeCompleted(HandshakeCompletedEvent event)
    {
	session = event.getSession();
    }

    /**
     * @return the proxy host being used for this client, or null
     *          if we're not going through a proxy
     */
    public String getProxyHostUsed() {
	if (!needsTunneling()) {
	    return null;
	} else {
	    return instTunnelHost;
	}
    }
    
    /**
     * @return the proxy port being used for this client.  Meaningless
     *          if getProxyHostUsed() gives null.
     */
    public int getProxyPortUsed() {
	return instTunnelPort;
    }
}

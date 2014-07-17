/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package javax.net.ssl;

import java.net.URL;
import java.net.HttpURLConnection;
import java.io.IOException;

/**
 * <code>HttpsURLConnection</code> extends <code>HttpURLConnection</code>
 * with support for https-specific features.
 * <P>
 * See <A HREF="http://www.w3.org/pub/WWW/Protocols/">
 * http://www.w3.org/pub/WWW/Protocols/</A> and
 * <A HREF="http://www.ietf.org/"> RFC 2818 </A>
 * for more details on the
 * https specification.
 * <P>
 * This class uses <code>HostnameVerifier</code> and
 * <code>SSLSocketFactory</code>.
 * There are default implementations defined for both classes.
 * However, the implementations can be replaced on a per-class (static) or
 * per-instance basis.  All new <code>HttpsURLConnection</code>s instances
 * will be assigned
 * the "default" static values at instance creation, but they can be overriden
 * by calling the appropriate per-instance set method(s) before
 * <code>connect</code>ing.
 *
 * @since 1.4
 * @version 1.20
 */
abstract public
class HttpsURLConnection extends HttpURLConnection
{
    /**
     * Creates an <code>HttpsURLConnection</code> using the
     * URL specified.
     *
     * @param url the URL
     * @throws IOException if the connection can't be established
     */
    protected HttpsURLConnection(URL url) throws IOException {
	super(url);
    }

    /**
     * Returns the cipher suite in use on this connection.
     *
     * @return the cipher suite
     * @throws IllegalStateException if this method is called before
     *		the connection has been established.
     */
    public abstract String getCipherSuite();

    /**
     * Returns the certificate(s) that were sent to the server during
     * handshaking.
     * <P>
     * When multiple certificates are available for use in a
     * handshake, the implementation chooses what it considers the
     * "best" certificate chain available, and transmits that to
     * the other side.  This method allows the caller to know
     * which certificate chain was actually sent.
     *
     * @return an ordered array of certificates,
     *		with the client's own certificate first followed by any
     *		certificate authorities.  If no certificates were sent,
     *		then null is returned.
     * @throws IllegalStateException if this method is called before
     *		the connection has been established.
     */
    public abstract java.security.cert.Certificate [] getLocalCertificates();

    /**
     * Returns the server's certificate chain which was established
     * as part of defining the session.
     *
     * @return an ordered array of server certificates,
     *		with the peer's own certificate first followed by
     *		any certificate authorities.
     * @throws SSLPeerUnverifiedException if the peer is not verified.
     * @throws IllegalStateException if this method is called before
     *		the connection has been established.
     */
    public abstract java.security.cert.Certificate [] getServerCertificates()
	    throws SSLPeerUnverifiedException;

    /**
     * HostnameVerifier provides a callback mechanism so that
     * implementers of this interface can supply a policy for
     * handling the case where the host to connect to and
     * the server name from the certificate mismatch.
     *
     * The default implementation will deny such connections.
     */
    private static HostnameVerifier
	defaultHostnameVerifier = new DefaultHostnameVerifier();

    /*
     * The initial default HostnameVerifier.  Should be updated for
     * another other type of HostnameVerifier that are created.
     */
    private static class DefaultHostnameVerifier
	    implements HostnameVerifier {
	public boolean verify(String hostname, SSLSession session) {
	    return false;
	}
    }

    /**
     * The hostnameVerifier for this object.
     */
    protected HostnameVerifier hostnameVerifier = defaultHostnameVerifier;

    /**
     * Sets the default HostnameVerifier inherited when an instance
     * of this class is created.
     * <P>
     * If this method is not called, the default hostname verifier
     * assumes the connection should not be permitted.
     *
     * @param v the default host name verifier
     * @throws IllegalArgumentException if the HostnameVerifier
     *		parameter is null.
     * @see #getDefaultHostnameVerifier()
     */
    public static void setDefaultHostnameVerifier(HostnameVerifier v) {
	if (v == null) {
	    throw new IllegalArgumentException(
		"no default HostnameVerifier specified");
	}

	SecurityManager sm = System.getSecurityManager();
	if (sm != null) {
	    sm.checkPermission(new SSLPermission("setHostnameVerifier"));
	}
	defaultHostnameVerifier = v;
    }

    /**
     * Gets the default HostnameVerifier that it inherited when an
     * instance of this class is created.
     *
     * @return the default host name verifier
     * @see #setDefaultHostnameVerifier(HostnameVerifier)
     */
    public static HostnameVerifier getDefaultHostnameVerifier() {
	return defaultHostnameVerifier;
    }

    /**
     * Sets the HostnameVerifier.
     * <P>
     * When called on a new instance of this class, this method overrides
     * the default static hostname verifier set by
     * {@link #setDefaultHostnameVerifier(HostnameVerifier)
     *		setDefaultHostnameVerifier}.
     *
     * @param v the host name verifier
     * @throws IllegalArgumentException if the HostnameVerifier
     *	parameter is null.
     * @see #getHostnameVerifier()
     * @see #setDefaultHostnameVerifier(HostnameVerifier)
     */
    public void setHostnameVerifier(HostnameVerifier v) {
	if (v == null) {
	    throw new IllegalArgumentException(
		"no HostnameVerifier specified");
	}

	hostnameVerifier = v;
    }

    /**
     * Gets the HostnameVerifier in place on this instance.
     * <P>
     *
     * @return the host name verifier
     * @see #setHostnameVerifier(HostnameVerifier)
     * @see #setDefaultHostnameVerifier(HostnameVerifier)
     */
    public HostnameVerifier getHostnameVerifier() {
	return hostnameVerifier;
    }

    private static SSLSocketFactory defaultSSLSocketFactory = null;

    /**
     * The SSL Socket Factory inherited when an instance
     * of this class is created.
     */
    private SSLSocketFactory sslSocketFactory = getDefaultSSLSocketFactory();

    /**
     * Sets the default SSL socket factory inherited when an instance
     * of this class is created.
     * <P>
     * The socket factories are used when creating sockets for secure
     * https URL connections.
     *
     * @param sf the default SSL socket factory
     * @throws IllegalArgumentException if the SSLSocketFactory
     *		parameter is null.
     * @see #getDefaultSSLSocketFactory()
     */
    public static void setDefaultSSLSocketFactory(SSLSocketFactory sf) {
	if (sf == null) {
	    throw new IllegalArgumentException(
		"no default SSLSocketFactory specified");
	}

	SecurityManager sm = System.getSecurityManager();
	if (sm != null) {
	    sm.checkSetFactory();
	}
	defaultSSLSocketFactory = sf;
    }

    /**
     * Gets the default static SSL socket factory used when creating sockets
     * for secure https URL connections.
     *
     * @return the default <code>SSLSocketFactory</code>
     * @see #setDefaultSSLSocketFactory(SSLSocketFactory)
     */
    public static SSLSocketFactory getDefaultSSLSocketFactory() {
	if (defaultSSLSocketFactory == null) {
	    defaultSSLSocketFactory =
		(SSLSocketFactory)SSLSocketFactory.getDefault();
	}
	return defaultSSLSocketFactory;
    }

    /**
     * Sets the SSL socket factory to be used when creating sockets
     * for secure https URL connections.
     * <P>
     * If called on a new instance of this class, this method overrides
     * the default static socket factory set by
     * {@link #setDefaultSSLSocketFactory(SSLSocketFactory)
     *		setDefaultSSLSocketFactory}.
     *
     * @param sf the SSL socket factory
     * @throws IllegalArgumentException if the SSLSocketFactory
     *		parameter is null.
     * @see #getSSLSocketFactory()
     */
    public void setSSLSocketFactory(SSLSocketFactory sf) {
	if (sf == null) {
	    throw new IllegalArgumentException(
		"no SSLSocketFactory specified");
	}

	sslSocketFactory = sf;
    }

    /**
     * Gets the SSL socket factory to be used when creating sockets
     * for secure https URL connections.
     *
     * @return the <code>SSLSocketFactory</code>
     * @see #setSSLSocketFactory(SSLSocketFactory)
     */
    public SSLSocketFactory getSSLSocketFactory() {
	return sslSocketFactory;
    }
}

/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package javax.net.ssl;

import java.util.EventObject;
import java.security.cert.Certificate;

/**
 * This event indicates that an SSL handshake completed on a given
 * SSL connection.  All of the core information about that handshake's
 * result is captured through an "SSLSession" object.  As a convenience,
 * this event class provides direct access to some important session
 * attributes.
 *
 * <P> The source of this event is the SSLSocket on which handshaking
 * just completed.
 *
 * @see SSLSocket
 * @see HandshakeCompletedListener
 * @see SSLSession
 *
 * @since 1.4
 * @version 1.19
 * @author David Brownell
 */
public class HandshakeCompletedEvent extends EventObject
{
    private transient SSLSession session;

    /**
     * Constructs a new HandshakeCompletedEvent.
     *
     * @param sock the SSLSocket acting as the source of the event
     * @param s the SSLSession this event is associated with
     */
    public HandshakeCompletedEvent(SSLSocket sock, SSLSession s)
    {
	super(sock);
	session = s;
    }


    /**
     * Returns the session that triggered this event.
     *
     * @return the <code>SSLSession</code> for this handshake
     */
    public SSLSession getSession()
    {
	return session;
    }


    /**
     * Returns the cipher suite in use by the session which was produced
     * by the handshake.  (This is a convenience method for
     * getting the ciphersuite from the SSLsession.)
     *
     * @return the name of the cipher suite negotiated during this session.
     */
    public String getCipherSuite()
    {
	return session.getCipherSuite();
    }


    /**
     * Returns the certificate(s) that were sent to the peer during
     * handshaking.
     *
     * When multiple certificates are available for use in a
     * handshake, the implementation chooses what it considers the
     * "best" certificate chain available, and transmits that to
     * the other side.  This method allows the caller to know
     * which certificate chain was actually used.
     *
     * @return an ordered array of certificates, with the local
     *		certificate first followed by any
     *		certificate authorities.  If no certificates were sent,
     *		then null is returned.
     */
    public java.security.cert.Certificate [] getLocalCertificates()
    {
	return session.getLocalCertificates();
    }


    /**
     * Returns the identity of the peer which was established as part
     * of defining the session.
     *
     * @return an ordered array of the peer certificates,
     *		with the peer's own certificate first followed by
     *		any certificate authorities.
     * @exception SSLPeerUnverifiedException if the peer is not verified.
     */
    public java.security.cert.Certificate [] getPeerCertificates()
	    throws SSLPeerUnverifiedException
    {
	return session.getPeerCertificates();
    }


    /**
     * Returns the identity of the peer which was identified as part
     * of defining the session.
     *
     * <p><em>Note: this method exists for compatibility with previous
     * releases. New applications should use
     * {@link #getPeerCertificates} instead.</em></p>
     *
     * @return an ordered array of peer X.509 certificates,
     *		with the peer's own certificate first followed by any
     *		certificate authorities.  (The certificates are in
     *		the original JSSE
     *		{@link javax.security.cert.X509Certificate} format).
     * @exception SSLPeerUnverifiedException if the peer is not verified.
     */
    public javax.security.cert.X509Certificate [] getPeerCertificateChain()
	    throws SSLPeerUnverifiedException
    {
	return session.getPeerCertificateChain();
    }


    /**
     * Returns the socket which is the source of this event.
     * (This is a convenience function, to let applications
     * write code without type casts.)
     *
     * @return the socket on which the connection was made.
     */
    public SSLSocket getSocket()
    {
	return (SSLSocket) getSource();
    }
}

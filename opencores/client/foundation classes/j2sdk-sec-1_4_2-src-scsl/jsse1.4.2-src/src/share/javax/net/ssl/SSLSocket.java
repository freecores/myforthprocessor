/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */


package javax.net.ssl;

import java.io.IOException;
import java.net.*;
import java.util.Enumeration;
import java.util.Vector;


/**
 * This class extends <code>Socket</code>s and provides secure
 * socket using protocols such as the "Secure
 * Sockets Layer" (SSL) or IETF "Transport Layer Security" (TLS) protocols.
 * <P>
 * Such sockets are normal stream sockets, but they
 * add a layer of security protections over the underlying network transport
 * protocol, such as TCP.  Those protections include: <UL>
 *
 *	<LI> <em>Integrity Protection</em>.  SSL protects against
 *	modification of messages by an active wiretapper.
 *
 *	<LI> <em>Authentication</em>.  In most modes, SSL provides
 *	peer authentication.  Servers are usually authenticated,
 *	and clients may be authenticated as requested by servers.
 *
 *	<LI> <em>Confidentiality (Privacy Protection)</em>.  In most
 *	modes, SSL encrypts data being sent between client and server.
 *	This protects the confidentiality of data, so that passive
 *	wiretappers won't see sensitive data such as financial
 *	information or personal information of many kinds.
 *
 *	</UL>
 *
 * <P>These kinds of protection are specified by a "cipher suite", which
 * is a combination of cryptographic algorithms used by a given SSL connection.
 * During the negotiation process, the two endpoints must agree on
 * a ciphersuite that is available in both environments.
 * If there is no such suite in common, no SSL connection can
 * be established, and no data can be exchanged.
 *
 * <P> The cipher suite used is established by a negotiation process
 * called "handshaking".  The goal of this
 * process is to create or rejoin a "session", which may protect many
 * connections over time.  After handshaking has completed, you can access
 * session attributes by using the <em>getSession</em> method.
 * The initial handshake on this connection can be initiated in
 * one of three ways: <UL>
 *
 *      <LI> calling <code>startHandshake</code> which explicitly
 *		begins handshakes, or
 *      <LI> any attempt to read or write application data on
 *		this socket causes an implicit handshake, or
 *      <LI> a call to <code>getSession</code> tries to set up a session
 *		if there is no currently valid session, and
 *		an implicit handshake is done.
 * </UL>
 *
 * <P>If handshaking fails for any reason, the <code>SSLSocket</code>
 * is closed, and no futher communications can be done.
 *
 * <P>There are two groups of cipher suites which you will need to know
 * about when managing cipher suites: <UL>
 *
 *	<LI> <em>Supported</em> cipher suites:  all the suites which are
 *	supported by the SSL implementation.  This list is reported
 *	using <em>getSupportedCipherSuites</em>.
 *
 *	<LI> <em>Enabled</em> cipher suites, which may be fewer
 *	than the full set of supported suites.  This group is
 *	set using the <em>setEnabledCipherSuites</em> method, and
 *	queried using the <em>getEnabledCipherSuites</em> method.
 *	Initially, a default set of cipher suites will be enabled on
 *      a new socket that represents the minimum suggested configuration.
 *
 *	</UL>
 *
 * <P> Implementation defaults require that only cipher
 * suites which authenticate servers and provide confidentiality
 * be enabled by default.
 * Only if both sides explicitly agree to unauthenticated and/or
 * non-private (unencrypted) communications will such a ciphersuite be
 * selected.
 *
 * <P>When <code>SSLSocket</code>s are first created, no handshaking
 * is done so that applications may first set their communication
 * preferences:  what cipher suites to use, whether the socket should be
 * in client or server mode, etc.
 * However, security is always provided by the time that application data
 * is sent over the connection.
 *
 * <P> You may register to receive event notification of handshake
 * completion.  This involves
 * the use of two additional classes.  <em>HandshakeCompletedEvent</em>
 * objects are passed to <em>HandshakeCompletedListener</em> instances,
 * which are registered by users of this API.
 *
 * <code>SSLSocket</code>s are created by <code>SSLSocketFactory</code>s,
 * or by <code>accept</code>ing a connection from a
 * <code>SSLServerSocket</code>.
 *
 * <P>A SSL socket may choose to operate in the client or server mode.
 * This will determine who begins the handshaking process, as well
 * as which messages should be sent by each party.  However, each
 * connection must have one client and one server, or handshaking
 * will not progress properly.
 *
 * @see java.net.Socket
 * @see SSLServerSocket
 * @see SSLSocketFactory
 *
 * @since 1.4
 * @version 1.23
 * @author David Brownell
 */
public abstract class SSLSocket extends Socket
{
    /**
     * Used only by subclasses.
     * Constructs an uninitialized, unconnected TCP socket.
     */
    protected SSLSocket()
	{ super(); }


    /**
     * Used only by subclasses.
     * Constructs a TCP connection to a named host at a specified port.
     * This acts as the SSL client.
     *
     * @param host name of the host with which to connect
     * @param port number of the server's port
     * @throws IOException if an I/O error occurs when creating the socket
     * @throws UnknownHostException if the host is not known
     */
    protected SSLSocket(String host, int port)
    throws IOException, UnknownHostException
	{ super(host, port); }


    /**
     * Used only by subclasses.
     * Constructs a TCP connection to a server at a specified address
     * and port.  This acts as the SSL client.
     *
     * @param address the server's host
     * @param port its port
     * @throws IOException if an I/O error occurs when creating the socket
     * @throws UnknownHostException if the host is not known
     */
    protected SSLSocket(InetAddress address, int port)
    throws IOException, UnknownHostException
	{ super(address, port); }


    /**
     * Used only by subclasses.
     * Constructs an SSL connection to a named host at a specified port,
     * binding the client side of the connection a given address and port.
     * This acts as the SSL client.
     *
     * @param host name of the host with which to connect
     * @param port number of the server's port
     * @param clientAddress the client's host
     * @param clientPort number of the client's port
     * @throws IOException if an I/O error occurs when creating the socket
     * @throws UnknownHostException if the host is not known
     */
    protected SSLSocket(String host, int port,
	InetAddress clientAddress, int clientPort)
    throws IOException, UnknownHostException
	{ super(host, port, clientAddress, clientPort); }


    /**
     * Used only by subclasses.
     * Constructs an SSL connection to a server at a specified address
     * and TCP port, binding the client side of the connection a given
     * address and port.  This acts as the SSL client.
     *
     * @param address the server's host
     * @param port its port
     * @param clientAddress the client's host
     * @param clientPort number of the client's port
     * @throws IOException if an I/O error occurs when creating the socket
     * @throws UnknownHostException if the host is not known
     */
    protected SSLSocket(InetAddress address, int port,
	InetAddress clientAddress, int clientPort)
    throws IOException, UnknownHostException
	{ super(address, port, clientAddress, clientPort); }


    /**
     * Returns the names of the cipher suites which could be enabled for use
     * on this connection.  Normally, only a subset of these will actually
     * be enabled by default, since this list may include cipher suites which
     * do not meet quality of service requirements for those defaults.  Such
     * cipher suites are useful in specialized applications.
     *
     * @return an array of cipher suite names
     * @see #getEnabledCipherSuites()
     * @see #setEnabledCipherSuites(String [])
     */
    public abstract String [] getSupportedCipherSuites();

    /**
     * Returns the names of the SSL cipher suites which are currently enabled
     * for use on this connection.  When an SSL socket is first created,
     * all enabled cipher suites support a minium quality of service.  Thus,
     * in some environments this value might be empty.
     *
     * <P> There are several reasons why an enabled cipher suite might
     * not actually be used.  For example:  the server socket might
     * not have appropriate private keys available to it or the cipher
     * suite might be anonymous, precluding the use of client authentication,
     * while the server socket has been told to require that sort of
     * authentication.
     *
     * @return an array of cipher suite names
     * @see #getSupportedCipherSuites()
     * @see #setEnabledCipherSuites(String [])
     */
    public abstract String [] getEnabledCipherSuites();


    /**
     * Controls which particular cipher suites are enabled for use on
     * this connection.  The cipher suites must have been listed by
     * getSupportedCipherSuites() as being supported.  Even if a suite
     * has been enabled, it might never be used if no peer supports it,
     * or the requisite certificates (and private keys) are not available.
     *
     * @param suites Names of all the cipher suites to enable
     * @exception IllegalArgumentException when one or more of the ciphers
     *		named by the parameter is not supported, or when the
     *		parameter is null.
     * @see #getSupportedCipherSuites()
     * @see #getEnabledCipherSuites()
     */
    public abstract void setEnabledCipherSuites(String suites []);


    /**
     * Returns the names of the protocols which could be enabled for use
     * on an SSL connection.
     *
     * @return an array of protocols supported
     */
    public abstract String [] getSupportedProtocols();

    /**
     * Returns the names of the protocol versions which are currently
     * enabled for use on this connection.
     * @see #setEnabledProtocols
     * @return an array of protocols
     */
    public abstract String [] getEnabledProtocols();

    /**
     * Controls which particular protocol versions are enabled for use on
     * this connection.  The protocols must have been listed by
     * getSupportedProtocols() as being supported.
     *
     * @param protocols Names of all the protocols to enable.
     * @exception IllegalArgumentException when one or more of
     *		  the protocols named by the parameter is not supported or
     *		  when the protocols parameter is null.
     * @see #getEnabledProtocols
     */
    public abstract void setEnabledProtocols(String protocols[]);

    /**
     * Returns the SSL Session in use by this connection.  These can
     * be long lived, and frequently correspond to an entire login session
     * for some user.  The session specifies a particular cipher suite
     * which is being actively used by all connections in that session,
     * as well as the identities of the session's client and server.
     * <P>
     * This method will initiate the initial handshake if
     * necessary and then block until the handshake has been
     * established.
     * <P>
     * If an error occurs during the initial handshake, this method
     * returns an invalid session object which reports an invalid
     * cipher suite of "SSL_NULL_WITH_NULL_NULL".
     *
     * @return the <code>SSLSession</code>
     */
    public abstract SSLSession getSession();


    /**
     * Registers an event listener to receive notifications that an
     * SSL handshake has completed on this connection.
     *
     * @param listener the HandShake Completed event listener
     * @see #startHandshake()
     * @see #removeHandshakeCompletedListener(HandshakeCompletedListener)
     * @throws IllegalArgumentException if the argument is null.
     */
    public abstract void addHandshakeCompletedListener(
	HandshakeCompletedListener listener);


    /**
     * Removes a previously registered handshake completion listener.
     *
     * @param listener the HandShake Completed event listener
     * @throws IllegalArgumentException if the listener is not registered,
     * or the argument is null.
     * @see #addHandshakeCompletedListener(HandshakeCompletedListener)
     */
    public abstract void removeHandshakeCompletedListener(
	HandshakeCompletedListener listener);


    /**
     * Starts an SSL handshake on this connection.  Common reasons include
     * a need to use new encryption keys, to change cipher suites, or to
     * initiate a new session.  To force complete reauthentication, the
     * current session could be invalidated before starting this handshake.
     *
     * <P> If data has already been sent on the connection, it continues
     * to flow during this handshake.  When the handshake completes, this
     * will be signaled with an event.
     *
     * This method is synchronous for the initial handshake on a connection
     * and returns when the negotiated handshake is complete. Some
     * protocols may not support multiple handshakes on an existing socket
     * and may throw an IOException.
     *
     * @throws IOException on a network level error
     * @see #addHandshakeCompletedListener(HandshakeCompletedListener)
     */
    public abstract void startHandshake() throws IOException;


    /**
     * Configures the socket to use client (or server) mode in its first
     * handshake.  Servers normally authenticate themselves, and clients
     * are not required to do so.
     *
     * @param mode true if the socket should start its first handshake
     *		in "client" mode
     * @throws IllegalArgumentException if a mode change is attempted
     *		after handshaking has begun.
     * @see #getUseClientMode()
     */
    public abstract void setUseClientMode(boolean mode);

    /**
     * Returns true if the socket is set to use client mode in its first
     * handshake.
     *
     * @return true if the socket should start its first handshake
     *		in "client" mode
     * @see #setUseClientMode(boolean)
     */
    public abstract boolean getUseClientMode();

    /**
     * Configures the socket to require client authentication.  This
     * option is only useful for sockets in the server mode.
     * <P>
     * Unlike {@link #setWantClientAuth(boolean)},
     * if the client chooses not to provide authentication information
     * about itself, the negotiations will stop and the connection
     * will be dropped.
     *
     * @param need should be set to true if the clients must
     *		authenticate themselves.
     *		Setting this parameter to true overrides the current
     *		setting of {@link #setWantClientAuth(boolean)}.
     * @see #getNeedClientAuth()
     * @see #setWantClientAuth(boolean)
     * @see #getWantClientAuth()
     * @see #setUseClientMode(boolean)
     */
    public abstract void setNeedClientAuth(boolean need);

    /**
     * Returns true if the socket will require client authentication. The
     * option is only useful to sockets in the server mode.
     *
     * @return true if the server mode socket should request that
     *		the client authenticate itself.
     * @see #setNeedClientAuth(boolean)
     * @see #setWantClientAuth(boolean)
     * @see #getWantClientAuth()
     * @see #setUseClientMode(boolean)
     */
    public abstract boolean getNeedClientAuth();

    /**
     * Configures the socket to request client authentication, but
     * only if such a request is appropriate to the cipher suite
     * negotiated.  This option is only useful
     * for sockets in the server mode.
     *
     * Unlike {@link #setNeedClientAuth(boolean)},
     * if the client chooses not to provide authentication information
     * about itself, the negotiations will continue.
     * <P>
     * The socket must be a server mode socket.
     *
     * @param want should be set to true if the clients should
     *		try to authenticate themselves.
     *		Setting this parameter to true overrides the current
     *		setting of {@link #setNeedClientAuth(boolean)}.
     * @see #getWantClientAuth()
     * @see #setNeedClientAuth(boolean)
     * @see #getNeedClientAuth()
     * @see #setUseClientMode(boolean)
     */
    public abstract void setWantClientAuth(boolean want);

    /**
     * Returns true if the socket will request client authentication.
     * This option is only useful for sockets in the server mode.
     *
     * @return true if the server mode socket should request that
     *		the client authenticate itself
     * @see #setNeedClientAuth(boolean)
     * @see #getNeedClientAuth()
     * @see #setWantClientAuth(boolean)
     * @see #setUseClientMode(boolean)
     */
    public abstract boolean getWantClientAuth();

    /**
     * Controls whether new SSL sessions may be established by this socket.
     * If sesssion creations are not allowed, and there are no
     * existing sessions to resume, there will be no successful
     * handshaking.
     *
     * @param flag true indicates that sessions may be created; this
     *		is the default.  false indicates that an existing session
     *		must be resumed
     * @see #getEnableSessionCreation()
     */
    public abstract void setEnableSessionCreation(boolean flag);

    /**
     * Returns true if new SSL sessions may be established by this socket.
     *
     * @return true indicates that sessions may be created; this
     *		is the default.  false indicates that an existing session
     *		must be resumed
     * @see #setEnableSessionCreation(boolean)
     */
    public abstract boolean getEnableSessionCreation();


}

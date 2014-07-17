/*
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)SSLContext.java	1.7 03/01/23
 */
  
/*
 * NOTE:
 * Because of various external restrictions (i.e. US export
 * regulations, etc.), the actual source code can not be provided
 * at this time. This file represents the skeleton of the source
 * file, so that javadocs of the API can be created.
 */

package javax.net.ssl;

import java.security.*;
import java.util.*;

import com.sun.net.ssl.internal.ssl.SSLSocketFactoryImpl;
import com.sun.net.ssl.internal.ssl.SSLServerSocketFactoryImpl;
import com.sun.net.ssl.internal.ssl.ExportControl;

/** 
 * Instances of this class represent a secure socket protocol
 * implementation which acts as a factory for secure socket
 * factories. This class is initialized with an optional set of
 * key and trust managers and source of secure random bytes.
 *
 * @since 1.4
 * @version 1.18
 */
public class SSLContext
{

    /** 
     * Creates an SSLContext object.
     *
     * @param contextSpi the delegate
     * @param provider the provider
     * @param protocol the protocol
     */
    protected SSLContext(SSLContextSpi contextSpi, Provider provider, String
        protocol)
    { }

    /** 
     * Generates a <code>SSLContext</code> object that implements the
     * specified secure socket protocol.
     * <P>
     * If the default provider package provides an implementation of the
     * requested key management algorithm, an instance of
     * <code>SSLContext</code> containing that implementation is
     * returned.  If the algorithm is not available in the default provider
     * package, other provider packages are searched.
     *
     * @param protocol the standard name of the requested protocol.
     * @return the new <code>SSLContext</code> object
     * @exception NoSuchAlgorithmException if the specified protocol is not
     *		  available in the default provider package or any of the
     *		  other provider packages that were searched.
     */
    public static SSLContext getInstance(String protocol)
        throws NoSuchAlgorithmException
    { }

    /** 
     * Generates a <code>SSLContext</code> object that implements the
     * specified secure socket protocol from the specified provider.
     *
     * @param protocol the standard name of the requested protocol.
     * @param provider the name of the provider
     * @return the new <code>SSLContext</code> object
     * @throws NoSuchAlgorithmException if the specified protocol is not
     *	       available from the specified provider.
     * @throws NoSuchProviderException if the specified provider has not
     *	       been configured.
     * @throws IllegalArgumentException if provider is not specified
     */
    public static SSLContext getInstance(String protocol, String provider)
        throws NoSuchAlgorithmException, NoSuchProviderException
    { }

    /** 
     * Generates a <code>SSLContext</code> object that implements the
     * specified secure socket protocol from the specified provider.
     *
     * @param protocol the standard name of the requested protocol.
     * @param provider an instance of the provider
     * @return the new <code>SSLContext</code> object
     * @throws NoSuchAlgorithmException if the specified protocol is not
     *         available from the specified provider.
     * @throws IllegalArgumentException if provider is not specified
     */
    public static SSLContext getInstance(String protocol, Provider provider)
        throws NoSuchAlgorithmException
    { }

    /** 
     * Returns the protocol name of this <code>SSLContext</code> object.
     *
     * <p>This is the same name that was specified in one of the
     * <code>getInstance</code> calls that created this
     * <code>SSLContext</code> object.
     *
     * @return the protocol name of this <code>SSLContext</code> object.
     */
    public final String getProtocol() { }

    /** 
     * Returns the provider of this <code>SSLContext</code> object.
     *
     * @return the provider of this <code>SSLContext</code> object
     */
    public final Provider getProvider() { }

    /** 
     * Initializes this context. Either of the first two parameters
     * may be null in which case the installed security providers will
     * be searched for the highest priority implementation of the
     * appropriate factory. Likewise, the secure random parameter may
     * be null in which case the default implementation will be used.
     * <P>
     * Only the first instance of a particular key and/or trust manager 
     * implementation type in the array is used.  (For example, only
     * the first javax.net.ssl.X509KeyManager in the array will be used.)
     *
     * @param km the sources of authentication keys or null
     * @param tm the sources of peer authentication trust decisions or null
     * @param random the source of randomness for this generator or null
     * @throws KeyManagementException if this operation fails
     */
    public final void init(KeyManager[] km, TrustManager[] tm, SecureRandom
        random) throws KeyManagementException
    { }

    /** 
     * Returns a <code>SocketFactory</code> object for this
     * context.
     *
     * @return the <code>SocketFactory</code> object
     */
    public final SSLSocketFactory getSocketFactory() { }

    /** 
     * Returns a <code>ServerSocketFactory</code> object for
     * this context.
     *
     * @return the <code>ServerSocketFactory</code> object
     */
    public final SSLServerSocketFactory getServerSocketFactory() { }

    /** 
     * Returns the server session context, which represents the set of
     * SSL sessions available for use during the handshake phase of
     * server-side SSL sockets.
     * <P>
     * This context may be unavailable in some environments, in which
     * case this method returns null. For example, when the underlying
     * SSL provider does not provide an implementation of SSLSessionContext
     * interface, this method returns null. A non-null session context
     * is returned otherwise.
     *
     * @return server session context bound to this SSL context
     */
    public final SSLSessionContext getServerSessionContext() { }

    /** 
     * Returns the client session context, which represents the set of
     * SSL sessions available for use during the handshake phase of
     * client-side SSL sockets.
     * <P>
     * This context may be unavailable in some environments, in which
     * case this method returns null. For example, when the underlying
     * SSL provider does not provide an implementation of SSLSessionContext
     * interface, this method returns null. A non-null session context
     * is returned otherwise.
     *
     * @return client session context bound to this SSL context
     */
    public final SSLSessionContext getClientSessionContext() { }
}

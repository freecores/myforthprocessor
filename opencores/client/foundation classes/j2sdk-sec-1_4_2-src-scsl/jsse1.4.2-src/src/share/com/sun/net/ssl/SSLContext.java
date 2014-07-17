/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

/*
 * NOTE:  this file was copied from javax.net.ssl.SSLContext
 */

package com.sun.net.ssl;

import java.security.*;
import java.util.*;
import javax.net.ssl.*;
import com.sun.net.ssl.internal.ssl.SSLSocketFactoryImpl;
import com.sun.net.ssl.internal.ssl.SSLServerSocketFactoryImpl;
import com.sun.net.ssl.internal.ssl.ExportControl;

/**
 * Instances of this class represent a secure socket protocol
 * implementation which acts as a factory for secure socket
 * factories. This class is initialized with an optional set of
 * key and trust managers and source of secure random bytes.
 *
 * @deprecated As of JDK 1.4, this implementation-specific class was
 *      replaced by {@link javax.net.ssl.SSLContext}.
 */
public class SSLContext {
    private Provider provider;

    private SSLContextSpi contextSpi;

    private String protocol;

    /**
     * Creates an SSLContext object.
     *
     * @param contextSpi the delegate
     * @param provider the provider
     * @param algorithm the algorithm
     */
    protected SSLContext(SSLContextSpi contextSpi, Provider provider,
	String protocol) {
	this.contextSpi = contextSpi;
	this.provider = provider;
	this.protocol = protocol;
    }

    /**
     * Generates a <code>SSLContext</code> object that implements the
     * specified secure socket protocol.
     *
     * @param protocol the standard name of the requested protocol.
     *
     * @return the new <code>SSLContext</code> object
     *
     * @exception NoSuchAlgorithmException if the specified protocol is not
     * available in the default provider package or any of the other provider
     * packages that were searched.
     */
    public static SSLContext getInstance(String protocol)
	throws NoSuchAlgorithmException
    {
	try {
	    Object[] objs = SSLSecurity.getImpl(protocol, "SSLContext",
						(String) null);
	    return new SSLContext((SSLContextSpi)objs[0], (Provider)objs[1],
		protocol);
	} catch (NoSuchProviderException e) {
	    throw new NoSuchAlgorithmException(protocol + " not found");
	}
    }

    /**
     * Generates a <code>SSLContext</code> object that implements the
     * specified secure socket protocol.
     *
     * @param protocol the standard name of the requested protocol.
     * @param provider the name of the provider
     *
     * @return the new <code>SSLContext</code> object
     *
     * @exception NoSuchAlgorithmException if the specified protocol is not
     * available from the specified provider.
     * @exception NoSuchProviderException if the specified provider has not
     * been configured.
     */
    public static SSLContext getInstance(String protocol, String provider)
	throws NoSuchAlgorithmException, NoSuchProviderException
    {
	if (provider == null || provider.length() == 0)
	    throw new IllegalArgumentException("missing provider");
	Object[] objs = SSLSecurity.getImpl(protocol, "SSLContext",
					    provider);
	return new SSLContext((SSLContextSpi)objs[0], (Provider)objs[1],
	    protocol);
    }

    /**
     * Generates a <code>SSLContext</code> object that implements the
     * specified secure socket protocol.
     *
     * @param protocol the standard name of the requested protocol.
     * @param provider an instance of the provider
     *
     * @return the new <code>SSLContext</code> object
     *
     * @exception NoSuchAlgorithmException if the specified protocol is not
     * available from the specified provider.
     */
    public static SSLContext getInstance(String protocol, Provider provider)
	throws NoSuchAlgorithmException
    {
	if (provider == null)
	    throw new IllegalArgumentException("missing provider");
	Object[] objs = SSLSecurity.getImpl(protocol, "SSLContext",
					    provider);
	return new SSLContext((SSLContextSpi)objs[0], (Provider)objs[1],
	    protocol);
    }

    /**
     * Returns the protocol name of this <code>SSLContext</code> object.
     *
     * <p>This is the same name that was specified in one of the
     * <code>getInstance</code> calls that created this
     * <code>SSLContext</code> object.
     *
     * @return the protocol name of this <code>SSLContext</code> object.
     */
    public final String getProtocol() {
	return this.protocol;
    }

    /**
     * Returns the provider of this <code>SSLContext</code> object.
     *
     * @return the provider of this <code>SSLContext</code> object
     */
    public final Provider getProvider() {
	return this.provider;
    }

    /**
     * Initializes this context. Either of the first two parameters
     * may be null in which case the installed security providers will
     * be searched for the highest priority implementation of the
     * appropriate factory. Likewise, the secure random parameter may
     * be null in which case the default implementation will be used.
     *
     * @param km the sources of authentication keys or null
     * @param tm the sources of peer authentication trust decisions or null
     * @param random the source of randomness for this generator or null
     */
    public final void init(KeyManager[] km, TrustManager[] tm,
				SecureRandom random)
	throws KeyManagementException {
	contextSpi.engineInit(km, tm, random);
    }

    /**
     * Returns a <code>SocketFactory</code> object for this
     * context.
     *
     * @return the factory
     */
    public final SSLSocketFactory getSocketFactory() {
	SSLSocketFactory sf;
	sf = contextSpi.engineGetSocketFactory();
	if (!ExportControl.isReplaceable) {
	    if (sf.getClass() != SSLSocketFactoryImpl.class)
		throw new RuntimeException("Export restriction: SunJSSE only");
	}
	return sf;
    }

    /**
     * Returns a <code>ServerSocketFactory</code> object for
     * this context.
     *
     * @return the factory
     */
    public final SSLServerSocketFactory getServerSocketFactory() {
	SSLServerSocketFactory ssf;
	ssf = contextSpi.engineGetServerSocketFactory();
	if (!ExportControl.isReplaceable) {
	    if (ssf.getClass() != SSLServerSocketFactoryImpl.class)
		throw new RuntimeException("Export restriction: SunJSSE only");
	}
	return ssf;
    }
}

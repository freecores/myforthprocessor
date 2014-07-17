/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

/*
 * NOTE:  this file was copied from javax.net.ssl.TrustManagerFactory
 */

package com.sun.net.ssl;

import java.security.*;

/**
 * This class acts as a factory for trust managers based on a
 * source of trust material. Each trust manager manages a specific
 * type of trust material for use by secure sockets. The trust
 * material is based on a KeyStore and/or provider specific sources.
 *
 * @deprecated As of JDK 1.4, this implementation-specific class was
 *      replaced by {@link javax.net.ssl.TrustManagerFactory}.
 */
public class TrustManagerFactory {
    // The provider
    private Provider provider;

    // The provider implementation (delegate)
    private TrustManagerFactorySpi factorySpi;

    // The name of the trust management algorithm.
    private String algorithm;

    /**
     * <p>The default TrustManager can be changed by setting the value of the
     * "sun.ssl.trustmanager.type" security property
     * (in the Java security properties file) to the desired name.
     *
     * @return the default type as specified in the
     * Java security properties file, or an implementation-specific default
     * if no such property exists.
     */
    public final static String getDefaultAlgorithm() {
	String type;
	type = (String)AccessController.doPrivileged(new PrivilegedAction() {
	    public Object run() {
		return Security.getProperty("sun.ssl.trustmanager.type");
	    }
	});
	if (type == null) {
	    type = "SunX509";
	}
	return type;

    }

    /**
     * Creates a TrustManagerFactory object.
     *
     * @param factorySpi the delegate
     * @param provider the provider
     * @param algorithm the algorithm
     */
    protected TrustManagerFactory(TrustManagerFactorySpi factorySpi,
	    Provider provider, String algorithm) {
	this.factorySpi = factorySpi;
	this.provider = provider;
	this.algorithm = algorithm;
    }

    /**
     * Returns the algorithm name of this <code>TrustManagerFactory</code>
     * object.
     *
     * <p>This is the same name that was specified in one of the
     * <code>getInstance</code> calls that created this
     * <code>TrustManagerFactory</code> object.
     *
     * @return the algorithm name of this <code>TrustManagerFactory</code>
     * object.
     */
    public final String getAlgorithm() {
	return this.algorithm;
    }

    /**
     * Generates a <code>TrustManagerFactory</code> object that implements the
     * specified trust management algorithm.
     * If the default provider package provides an implementation of the
     * requested trust management algorithm, an instance of
     * <code>TrustManagerFactory</code> containing that implementation is
     * returned.  If the algorithm is not available in the default provider
     * package, other provider packages are searched.
     *
     * @param algorithm the standard name of the requested trust management
     * algorithm.
     *
     * @return the new <code>TrustManagerFactory</code> object
     *
     * @exception NoSuchAlgorithmException if the specified algorithm is not
     * available in the default provider package or any of the other provider
     * packages that were searched.
     */
    public static final TrustManagerFactory getInstance(String algorithm)
	throws NoSuchAlgorithmException
    {
	try {
	    Object[] objs = SSLSecurity.getImpl(algorithm,
		"TrustManagerFactory", (String) null);
	    return new TrustManagerFactory((TrustManagerFactorySpi)objs[0],
				    (Provider)objs[1],
				    algorithm);
	} catch (NoSuchProviderException e) {
	    throw new NoSuchAlgorithmException(algorithm + " not found");
	}
    }

    /**
     * Generates a <code>TrustManagerFactory</code> object for the specified
     * trust management algorithm from the specified provider.
     *
     * @param algorithm the standard name of the requested trust management
     * algorithm.
     * @param provider the name of the provider
     *
     * @return the new <code>TrustManagerFactory</code> object
     *
     * @exception NoSuchAlgorithmException if the specified algorithm is not
     * available from the specified provider.
     * @exception NoSuchProviderException if the specified provider has not
     * been configured.
     */
    public static final TrustManagerFactory getInstance(String algorithm,
						 String provider)
	throws NoSuchAlgorithmException, NoSuchProviderException
    {
	if (provider == null || provider.length() == 0)
	    throw new IllegalArgumentException("missing provider");
	Object[] objs = SSLSecurity.getImpl(algorithm, "TrustManagerFactory",
					    provider);
	return new TrustManagerFactory((TrustManagerFactorySpi)objs[0],
	    (Provider)objs[1], algorithm);
    }

    /**
     * Generates a <code>TrustManagerFactory</code> object for the specified
     * trust management algorithm from the specified provider.
     *
     * @param algorithm the standard name of the requested trust management
     * algorithm.
     * @param provider an instance of the provider
     *
     * @return the new <code>TrustManagerFactory</code> object
     *
     * @exception NoSuchAlgorithmException if the specified algorithm is not
     * available from the specified provider.
     */
    public static final TrustManagerFactory getInstance(String algorithm,
						 Provider provider)
	throws NoSuchAlgorithmException
    {
	if (provider == null)
	    throw new IllegalArgumentException("missing provider");
	Object[] objs = SSLSecurity.getImpl(algorithm, "TrustManagerFactory",
					    provider);
	return new TrustManagerFactory((TrustManagerFactorySpi)objs[0],
	    (Provider)objs[1], algorithm);
    }

    /**
     * Returns the provider of this <code>TrustManagerFactory</code> object.
     *
     * @return the provider of this <code>TrustManagerFactory</code> object
     */
    public final Provider getProvider() {
	return this.provider;
    }


    /**
     * Initializes this factory with a source of certificate
     * authorities and related trust material. The
     * provider may also include a provider-specific source
     * of key material.
     *
     * @param ks the key store or null
     */
    public void init(KeyStore ks) throws KeyStoreException {
	factorySpi.engineInit(ks);
    }

    /**
     * Returns one trust manager for each type of trust material.
     * @return the trust managers
     */
    public TrustManager[] getTrustManagers() {
	return factorySpi.engineGetTrustManagers();
    }
}

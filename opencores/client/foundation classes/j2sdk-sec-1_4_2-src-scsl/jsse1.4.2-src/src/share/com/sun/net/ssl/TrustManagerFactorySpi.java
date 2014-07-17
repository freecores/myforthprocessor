/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

/*
 * NOTE:  this file was copied from javax.net.ssl.TrustManagerFactorySpi
 */

package com.sun.net.ssl;

import java.security.*;

/**
 * This class defines the <i>Service Provider Interface</i> (<b>SPI</b>)
 * for the <code>TrustManagerFactory</code> class.
 *
 * <p> All the abstract methods in this class must be implemented by each
 * cryptographic service provider who wishes to supply the implementation
 * of a particular trust manager factory.
 *
 * @deprecated As of JDK 1.4, this implementation-specific class was
 *      replaced by {@link javax.net.ssl.TrustManagerFactorySpi}.
 */
public abstract class TrustManagerFactorySpi {
    /**
     * Initializes this factory with a source of certificate
     * authorities and related trust material. The
     * provider may also include a provider-specific source
     * of key material.
     *
     * @param ks the key store or null
     */
    protected abstract void engineInit(KeyStore ks) throws KeyStoreException;

    /**
     * Returns one trust manager for each type of trust material.
     * @return the trust managers
     */
    protected abstract TrustManager[] engineGetTrustManagers();
}

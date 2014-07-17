/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

/* 
 * NOTE:  this file was copied from javax.net.ssl.SSLContextSpi
 */ 

package com.sun.net.ssl;

import java.util.*;
import java.security.*;
import javax.net.ssl.*;

/**
 * This class defines the <i>Service Provider Interface</i> (<b>SPI</b>)
 * for the <code>SSLContext</code> class.
 *
 * <p> All the abstract methods in this class must be implemented by each
 * cryptographic service provider who wishes to supply the implementation
 * of a particular SSL context.
 *
 * @deprecated As of JDK 1.4, this implementation-specific class was
 *      replaced by {@link javax.net.ssl.SSLContextSpi}.
 */
public abstract class SSLContextSpi {
    /**
     * Initializes this context.
     *
     * @param km the sources of authentication keys
     * @param tm the sources of peer authentication trust decisions
     * @param random the source of randomness for this generator
     */
    protected abstract void engineInit(KeyManager[] ah, TrustManager[] th,
	SecureRandom sr) throws KeyManagementException;

    /**
     * Returns a <code>SocketFactory</code> object for this
     * context.
     *
     * @return the factory
     */
    protected abstract SSLSocketFactory engineGetSocketFactory();

    /**
     * Returns a <code>ServerSocketFactory</code> object for
     * this context.
     *
     * @return the factory
     */
    protected abstract SSLServerSocketFactory engineGetServerSocketFactory();
}

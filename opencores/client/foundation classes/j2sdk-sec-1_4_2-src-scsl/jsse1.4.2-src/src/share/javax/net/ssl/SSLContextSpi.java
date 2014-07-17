/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package javax.net.ssl;

import java.util.*;
import java.security.*;

/**
 * This class defines the <i>Service Provider Interface</i> (<b>SPI</b>)
 * for the <code>SSLContext</code> class.
 *
 * <p> All the abstract methods in this class must be implemented by each
 * cryptographic service provider who wishes to supply the implementation
 * of a particular SSL context.
 *
 * @since 1.4
 * @see SSLContext
 * @version 1.11
 */
public abstract class SSLContextSpi {
    /**
     * Initializes this context.
     *
     * @param km the sources of authentication keys
     * @param tm the sources of peer authentication trust decisions
     * @param sr the source of randomness
     * @throws KeyManagementException if this operation fails
     * @see SSLContext#init(KeyManager [], TrustManager [], SecureRandom)
     */
    protected abstract void engineInit(KeyManager[] km, TrustManager[] tm,
        SecureRandom sr) throws KeyManagementException;

    /**
     * Returns a <code>SocketFactory</code> object for this
     * context.
     *
     * @return the <code>SocketFactory</code> object
     * @see javax.net.ssl.SSLContext#getSocketFactory()
     */
    protected abstract SSLSocketFactory engineGetSocketFactory();

    /**
     * Returns a <code>ServerSocketFactory</code> object for
     * this context.
     *
     * @return the <code>ServerSocketFactory</code> object
     * @see javax.net.ssl.SSLContext#getServerSocketFactory()
     */
    protected abstract SSLServerSocketFactory engineGetServerSocketFactory();

    /**
     * Returns a server <code>SSLSessionContext</code> object for
     * this context.
     *
     * @return the <code>SSLSessionContext</code> object
     * @see javax.net.ssl.SSLContext#getServerSessionContext()
     */
    protected abstract SSLSessionContext engineGetServerSessionContext();

    /**
     * Returns a client <code>SSLSessionContext</code> object for
     * this context.
     *
     * @return the <code>SSLSessionContext</code> object
     * @see javax.net.ssl.SSLContext#getClientSessionContext()
     */
    protected abstract SSLSessionContext engineGetClientSessionContext();
}

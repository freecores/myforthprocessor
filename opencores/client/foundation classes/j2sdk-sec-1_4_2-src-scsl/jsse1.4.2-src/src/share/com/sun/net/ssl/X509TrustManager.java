/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

/*
 * NOTE:  this file was copied from javax.net.ssl.X509TrustManager
 */

package com.sun.net.ssl;

import java.security.cert.X509Certificate;

/**
 * Instance of this interface manage which X509 certificates
 * may be used to authenticate the remote side of a secure
 * socket. Decisions may be based on trusted certificate
 * authorities, certificate revocation lists, online
 * status checking or other means.
 *
 * @deprecated As of JDK 1.4, this implementation-specific class was
 *      replaced by {@link javax.net.ssl.X509TrustManager}.
 */
public interface X509TrustManager extends TrustManager {
    /**
     * Given the partial or complete certificate chain
     * provided by the peer, build a certificate path
     * to a trusted root and return true if it can be
     * validated and is trusted for client SSL authentication.
     *
     * @param chain the peer certificate chain
     */
    public boolean isClientTrusted(X509Certificate[] chain);

    /**
     * Given the partial or complete certificate chain
     * provided by the peer, build a certificate path
     * to a trusted root and return true if it can be
     * validated and is trusted for server SSL authentication.
     *
     * @param chain the peer certificate chain
     */
    public boolean isServerTrusted(X509Certificate[] chain);

    /**
     * Return an array of certificate authority certificates
     * which are trusted for authenticating peers.
     *
     * @return the acceptable CA issuer certificates
     */
    public X509Certificate[] getAcceptedIssuers();
}

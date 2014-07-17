/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

/*
 * NOTE:  this file was copied from javax.net.ssl.HostnameVerifier
 */

package com.sun.net.ssl;

/**
 * HostnameVerifier provides a callback mechanism so that
 * implementers of this interface can supply a policy for
 * handling the case where the host to connect to and
 * the server name from the certificate mismatch.
 *
 * @deprecated As of JDK 1.4, this implementation-specific class was
 *	replaced by {@link javax.net.ssl.HostnameVerifier} and
 *	{@link javax.net.ssl.CertificateHostnameVerifier}.
 */

public interface HostnameVerifier {
    /**
     * Verify that the hostname from the URL is an acceptable
     * match with the value from the common name entry in the
     * server certificate's distinguished name.
     *
     * @param urlHostname the host name of the URL
     * @param certHostname the common name entry from the certificate
     * @return true if the certificate host name is acceptable
     */
    public boolean verify(String urlHostname, String certHostname);
}

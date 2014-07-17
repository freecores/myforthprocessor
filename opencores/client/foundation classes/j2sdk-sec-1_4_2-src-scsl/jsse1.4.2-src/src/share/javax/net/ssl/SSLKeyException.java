/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */


package javax.net.ssl;

/**
 * Reports a bad SSL key.  Normally, this indicates misconfiguration
 * of the server or client SSL certificate and private key.
 *
 * @since 1.4
 * @version 1.10
 * @author David Brownell
 */
public
class SSLKeyException extends SSLException
{
    /**
     * Constructs an exception reporting a key management error
     * found by an SSL subsystem.
     *
     * @param reason describes the problem.
     */
    public SSLKeyException(String reason)
    {
        super(reason);
    }
}

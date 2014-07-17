/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */


package javax.net.ssl;


/**
 * Indicates that the client and server could not negotiate the
 * desired level of security.  The connection is no longer usable.
 *
 * @since 1.4
 * @version 1.10
 * @author David Brownell
 */
public
class SSLHandshakeException extends SSLException
{
    /**
     * Constructs an exception reporting an error found by
     * an SSL subsystem during handshaking.
     *
     * @param reason describes the problem.
     */
    public SSLHandshakeException(String reason)
    {
        super(reason);
    }
}


/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */


package javax.net.ssl;

/**
 * Reports an error in the operation of the SSL protocol.  Normally
 * this indicates a flaw in one of the protocol implementations.
 *
 * @since 1.4
 * @version 1.10
 * @author David Brownell
 */
public
class SSLProtocolException extends SSLException
{
    /**
     * Constructs an exception reporting an SSL protocol error
     * detected by an SSL subsystem.
     *
     * @param reason describes the problem.
     */
    public SSLProtocolException(String reason)
    {
        super(reason);
    }
}

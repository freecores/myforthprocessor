/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */


package javax.net.ssl;


/**
 * Indicates that the peer's identity has not been verified.
 * <P>
 * When the peer was not able to
 * identify itself (for example; no certificate, the particular
 * cipher suite being used does not support authentication, or no
 * peer authentication was established during SSL handshaking) this
 * exception is thrown.
 *
 * @since 1.4
 * @version 1.14
 * @author David Brownell
 */
public
class SSLPeerUnverifiedException extends SSLException
{
    /**
     * Constructs an exception reporting that the SSL peer's
     * identity has not been verifiied.
     *
     * @param reason describes the problem.
     */
    public SSLPeerUnverifiedException(String reason)
    {
	super(reason);
    }
}

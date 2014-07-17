/*
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)SSLPeerUnverifiedException.java	1.5 03/01/23
 */
  
/*
 * NOTE:
 * Because of various external restrictions (i.e. US export
 * regulations, etc.), the actual source code can not be provided
 * at this time. This file represents the skeleton of the source
 * file, so that javadocs of the API can be created.
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
 * @version 1.13
 * @author David Brownell
 */
public class SSLPeerUnverifiedException extends SSLException
{

    /** 
     * Constructs an exception reporting that the SSL peer's
     * identity has not been verifiied.
     *
     * @param reason describes the problem.
     */
    public SSLPeerUnverifiedException(String reason) { }
}
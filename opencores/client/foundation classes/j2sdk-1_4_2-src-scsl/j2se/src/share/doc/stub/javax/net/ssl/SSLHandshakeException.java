/*
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)SSLHandshakeException.java	1.4 03/01/23
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
 * Indicates that the client and server could not negotiate the
 * desired level of security.  The connection is no longer usable.
 *
 * @since 1.4
 * @version 1.9
 * @author David Brownell
 */
public class SSLHandshakeException extends SSLException
{

    /** 
     * Constructs an exception reporting an error found by
     * an SSL subsystem during handshaking.
     *
     * @param reason describes the problem.
     */
    public SSLHandshakeException(String reason) { }
}

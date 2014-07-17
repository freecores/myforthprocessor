/*
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)SSLException.java	1.5 03/01/23
 */
  
/*
 * NOTE:
 * Because of various external restrictions (i.e. US export
 * regulations, etc.), the actual source code can not be provided
 * at this time. This file represents the skeleton of the source
 * file, so that javadocs of the API can be created.
 */

package javax.net.ssl;

import java.io.IOException;

/** 
 * Indicates some kind of error detected by an SSL subsystem.
 * This class is the general class of exceptions produced
 * by failed SSL-related operations.
 *
 * @since 1.4
 * @version 1.10
 * @author David Brownell
 */
public class SSLException extends IOException
{

    /** 
     * Constructs an exception reporting an error found by
     * an SSL subsystem.
     *
     * @param reason describes the problem.
     */
    public SSLException(String reason) { }
}

/*
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)ShortBufferException.java	1.3 03/01/23
 */
  
/*
 * NOTE:
 * Because of various external restrictions (i.e. US export
 * regulations, etc.), the actual source code can not be provided
 * at this time. This file represents the skeleton of the source
 * file, so that javadocs of the API can be created.
 */

package javax.crypto;

import java.security.GeneralSecurityException;

/** 
 * This exception is thrown when an output buffer provided by the user
 * is too short to hold the operation result.
 *
 * @author Jan Luehe
 *
 * @version 1.3, 01/23/03
 * @since 1.4
 */
public class ShortBufferException extends GeneralSecurityException
{

    /** 
     * Constructs a ShortBufferException with no detail
     * message. A detail message is a String that describes this
     * particular exception.
     */
    public ShortBufferException() { }

    /** 
     * Constructs a ShortBufferException with the specified
     * detail message. A detail message is a String that describes
     * this particular exception, which may, for example, specify which
     * algorithm is not available.  
     *
     * @param msg the detail message.  
     */
    public ShortBufferException(String msg) { }
}

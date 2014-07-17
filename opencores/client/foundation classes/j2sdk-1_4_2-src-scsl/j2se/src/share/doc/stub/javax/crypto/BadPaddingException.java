/*
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)BadPaddingException.java	1.4 03/01/23
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
 * This exception is thrown when a particular padding mechanism is
 * expected for the input data but the data is not padded properly.
 *
 * @author Gigi Ankney
 *
 * @version 1.7, 07/16/01
 * @since 1.4
 */
public class BadPaddingException extends GeneralSecurityException
{

    /** 
     * Constructs a BadPaddingException with no detail
     * message. A detail message is a String that describes this
     * particular exception.
     */
    public BadPaddingException() { }

    /** 
     * Constructs a BadPaddingException with the specified
     * detail message. A detail message is a String that describes
     * this particular exception, which may, for example, specify which
     * algorithm is not available.  
     *
     * @param msg the detail message.  
     */
    public BadPaddingException(String msg) { }
}

/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package javax.crypto;

import java.security.GeneralSecurityException;

/**
 * This exception is thrown when an output buffer provided by the user
 * is too short to hold the operation result.
 *
 * @author Jan Luehe
 *
 * @version 1.9, 06/24/03
 * @since 1.4
 */

public class ShortBufferException extends GeneralSecurityException {

    /** 
     * Constructs a ShortBufferException with no detail
     * message. A detail message is a String that describes this
     * particular exception.
     */
    public ShortBufferException() {
	super();
    }

    /**
     * Constructs a ShortBufferException with the specified
     * detail message. A detail message is a String that describes
     * this particular exception, which may, for example, specify which
     * algorithm is not available.  
     *
     * @param msg the detail message.  
     */
    public ShortBufferException(String msg) {
	super(msg);
    }
}

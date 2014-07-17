/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package javax.crypto;

import java.security.GeneralSecurityException;

/**
 * This exception is thrown when a particular padding mechanism is
 * requested but is not available in the environment.
 *
 * @author Jan Luehe
 *
 * @version 1.9, 06/24/03
 * @since 1.4
 */

public class NoSuchPaddingException extends GeneralSecurityException {

    /** 
     * Constructs a NoSuchPaddingException with no detail
     * message. A detail message is a String that describes this
     * particular exception.
     */
    public NoSuchPaddingException() {
	super();
    }

    /**
     * Constructs a NoSuchPaddingException with the specified
     * detail message. A detail message is a String that describes
     * this particular exception, which may, for example, specify which
     * algorithm is not available.  
     *
     * @param msg the detail message.  
     */
    public NoSuchPaddingException(String msg) {
	super(msg);
    }
}

/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */
 
package javax.crypto;

/** 
 * This exception is thrown when the length of data provided to a block
 * cipher is incorrect, i.e., does not match the block size of the cipher.
 *
 * @author Jan Luehe
 *
 * @version 1.15, 06/24/03
 * @since 1.4
 */

public class IllegalBlockSizeException
    extends java.security.GeneralSecurityException {

    /** 
     * Constructs an IllegalBlockSizeException with no detail message.
     * (A detail message is a String that describes this particular
     * exception.)  
     */
    public IllegalBlockSizeException() {
        super();
    }

    /**
     * Constructs an IllegalBlockSizeException with the specified
     * detail message. (A detail message is a String that describes
     * this particular exception.)  
     *
     * @param msg the detail message. 
     */
    public IllegalBlockSizeException(String msg) {
	super(msg);
    }
}

/*
 * @(#)InvalidCodeIndexException.java	1.8 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jdi;

/**
 * Thrown to indicate that the requested operation cannot be 
 * completed because the specified code index is not valid.
 *
 * @deprecated This exception is no longer thrown
 *
 * @author Gordon Hirsch
 * @since  1.3
 */
public class InvalidCodeIndexException extends RuntimeException {
    public InvalidCodeIndexException() {
	super();
    }

    public InvalidCodeIndexException(String s) {
	super(s);
    }
}

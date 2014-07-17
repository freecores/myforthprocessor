/*
 * @(#)InvalidLineNumberException.java	1.8 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jdi;

/**
 * Thrown to indicate that the requested operation cannot be 
 * completed because the specified line number is not valid.
 *
 * @deprecated This exception is no longer thrown
 *
 * @author Gordon Hirsch
 * @since  1.3
 */
public class InvalidLineNumberException extends RuntimeException {
    public InvalidLineNumberException() {
	super();
    }

    public InvalidLineNumberException(String s) {
	super(s);
    }
}

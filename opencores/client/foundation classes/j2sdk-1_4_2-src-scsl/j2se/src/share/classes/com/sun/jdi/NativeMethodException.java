/*
 * @(#)NativeMethodException.java	1.7 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jdi;

/**
 * Thrown to indicate an operation cannot be completed because
 * it is not valid for a native method.
 *
 * @author Gordon Hirsch
 * @since  1.3
 */
public class NativeMethodException extends RuntimeException {

    public NativeMethodException() {
	super();
    }
    
    public NativeMethodException(String message) {
	super(message);
    }
}

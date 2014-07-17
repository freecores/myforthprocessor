/*
 * @(#)InternalException.java	1.7 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jdi;

/**
 * Thrown to indicate that an unexpected internal error has 
 * occurred.
 *
 * @author Gordon Hirsch
 * @since  1.3
 */
public class InternalException extends RuntimeException {
     private int errorCode;

     public InternalException() {
         super();
         this.errorCode = 0;
     }
 
     public InternalException(String s) {
         super(s);
         this.errorCode = 0;
     }

    public InternalException(int errorCode) {
	super();
        this.errorCode = errorCode;
    }

    public InternalException(String s, int errorCode) {
	super(s);
        this.errorCode = errorCode;
    }

    public int errorCode() {
        return errorCode;
    }
}

/*
 * @(#)VMMismatchException.java	1.7 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jdi;

/**
 * Thrown to indicate that the requested operation cannot be 
 * completed because the a mirror from one target VM is being
 * combined with a mirror from another target VM.
 *
 * @author Gordon Hirsch
 * @since  1.3
 */
public class VMMismatchException extends RuntimeException {
    public VMMismatchException() {
	super();
    }

    public VMMismatchException(String s) {
	super(s);
    }
}

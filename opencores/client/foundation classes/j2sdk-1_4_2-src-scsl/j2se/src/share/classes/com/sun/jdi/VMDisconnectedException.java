/*
 * @(#)VMDisconnectedException.java	1.8 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jdi;

/**
 * Unchecked exception thrown to indicate that the 
 * requested operation cannot be 
 * completed because there is no longer a connection to the target VM.
 *
 * @author Robert Field
 * @since  1.3
 */
public class VMDisconnectedException extends RuntimeException {

    public VMDisconnectedException() {
	super();
    }
    public VMDisconnectedException(String message) {
	super(message);
    }
}

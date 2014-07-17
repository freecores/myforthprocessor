/*
 * @(#)ObjectCollectedException.java	1.8 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jdi;

/**
 * Thrown to indicate that the requested operation cannot be 
 * completed because the specified object has been garbage collected.
 *
 * @author Gordon Hirsch
 * @since  1.3
 */
public class ObjectCollectedException extends RuntimeException {
    public ObjectCollectedException() {
	super();
    }

    public ObjectCollectedException(String s) {
	super(s);
    }
}

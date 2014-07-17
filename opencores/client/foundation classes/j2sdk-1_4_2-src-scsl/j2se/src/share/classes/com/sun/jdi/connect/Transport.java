/*
 * @(#)Transport.java	1.9 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jdi.connect;

/**
 * A method of communication between a debugger and a target VM.
 *
 * @author Gordon Hirsch
 * @since  1.3
 */
public interface Transport {
    /**
     * Returns a short identifier for the transport.
     *
     * @return the name of this transport.
     */
    String name();
}

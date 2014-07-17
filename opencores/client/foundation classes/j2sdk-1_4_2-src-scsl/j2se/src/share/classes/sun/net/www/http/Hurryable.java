/*
 * @(#)Hurryable.java	1.3 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.net.www.http;

/**
 * A <code>Hurryable</code> is a class that has been instructed to complete
 * its input processing so as to make resource associated with that input
 * available to others.
 */
public interface Hurryable {

    /**
     * @return a <code>boolean</code> indicating if the stream has been
     *	       hurried or not.
     */
    boolean hurry();

}


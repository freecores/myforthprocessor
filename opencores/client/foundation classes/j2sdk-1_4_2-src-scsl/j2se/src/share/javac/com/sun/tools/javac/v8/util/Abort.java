/**
 * @(#)Abort.java	1.10 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.tools.javac.v8.util;

/**
 * Throwing an instance of
 *  this class causes (silent) termination of the main compiler method.
 */
public class Abort extends Error {

    public Abort() {
        super();
    }
}

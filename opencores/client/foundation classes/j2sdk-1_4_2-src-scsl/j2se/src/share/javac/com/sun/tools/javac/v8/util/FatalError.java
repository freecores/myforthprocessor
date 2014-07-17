/**
 * @(#)FatalError.java	1.7 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.tools.javac.v8.util;

/**
 * Throwing an instance of this class causes immediate termination
 *  of the main compiler method.  It is used when some non-recoverable
 *  error has been detected in the compiler environment at runtime.
 */
public class FatalError extends Error {

    /**
     * Construct a <code>FatalError</code> with no detail message.
     */
    public FatalError() {
        super();
    }

    /**
      * Construct a <code>FatalError</code> with the specified detail message.
      *  @param s A string describing the failure (in most cases, this will be
      *           a locazlized string preceeded by the localized string
      *           "Fatal Error:").
      */
    public FatalError(String s) {
        super(s);
    }
}

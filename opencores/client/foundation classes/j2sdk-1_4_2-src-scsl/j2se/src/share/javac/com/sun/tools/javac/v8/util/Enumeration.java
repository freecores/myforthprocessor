/**
 * @(#)Enumeration.java	1.10 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.tools.javac.v8.util;

/**
 * A generic version of java.util.Enumeration.
 */
public interface Enumeration {

    boolean hasMoreElements();

    Object nextElement();
}

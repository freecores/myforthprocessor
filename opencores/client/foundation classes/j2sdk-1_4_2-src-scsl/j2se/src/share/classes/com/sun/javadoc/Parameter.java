/*
 * @(#)Parameter.java	1.7 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javadoc;

/**
 * Parameter information.
 * This includes a parameter type and parameter name.
 *
 * @author Robert Field
 */
public interface Parameter {

    /**
     * Get the type of this parameter.
     */
    Type type();

    /**
     * Get local name of this parameter.
     * For example if parameter is the short 'index', returns "index".
     */
    String name();

    /**
     * Get type name of this parameter.
     * For example if parameter is the short 'index', returns "short".
     */
    String typeName();

    /**
     * Returns a string representation of the class.
     * <p>
     * For example if parameter is the short 'index', returns "short index".
     *
     * @return type name and parameter name of this parameter.
     */
    String toString();
}



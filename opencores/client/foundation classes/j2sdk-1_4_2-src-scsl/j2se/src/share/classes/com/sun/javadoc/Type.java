/*
 * @(#)Type.java	1.14 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javadoc;

/**
 * Represents a java type. Type can be
 * a class, interface or primitive data type, like int or char.
 *
 * @since JDK1.2
 * @author Kaiyang Liu (original)
 * @author Robert Field (rewrite)
 */
public interface Type {

    /**
     * Return unqualified name of type excluding any dimension information.
     * <p>
     * For example, a two dimensional array of String returns 'String'.
     */
    String typeName();

    /**
     * Return qualified name of type excluding any dimension information.
     *<p>
     * For example, a two dimensional array of String
     * returns 'java.lang.String'.
     */
    String qualifiedTypeName();

    /**
     * Return the type's dimension information, as a string.
     * <p>
     * For example, a two dimensional array of String returns '[][]'.
     */
    String dimension();

    /**
     * Returns a string representation of the type.
     *
     * Return name of type including any dimension information.
     * <p>
     * For example, a two dimensional array of String returns
     * <code>String[][]</code>.
     *
     * @return name of type including any dimension information.
     */
    String toString();

    /**
     * Return this type as a class doc if this type is a class
     * or interface.  Array dimensions are ignored.
     *
     * @return a ClassDoc if the type is a class or interface.
     * Return null if it is a primitive type..
     */
    ClassDoc asClassDoc();
}

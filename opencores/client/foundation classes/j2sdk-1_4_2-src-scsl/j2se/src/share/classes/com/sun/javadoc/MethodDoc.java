/*
 * @(#)MethodDoc.java	1.8 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javadoc;

/**
 * Represents a method of a java class.
 *
 * @since JDK1.2
 * @author Robert Field
 */
public interface MethodDoc extends ExecutableMemberDoc {

    /**
     * Return true if this method is abstract
     */
    boolean isAbstract();

    /**
     * Get return type.
     *
     * @return the return type of this method, null if it
     * is a constructor.
     */
    Type returnType();

    /**
     * Return the class containing the method that this method overrides.
     *
     * @return a ClassDoc representing the superclass
     * defining a method that this method overrides, null if
     * this method does not override.
     */
    ClassDoc overriddenClass();


    /**
     * Return the method that this method overrides.
     *
     * @return a MethodDoc representing a method definition
     * in a superclass this method overrides, null if
     * this method does not override.
     */
    MethodDoc overriddenMethod();
}


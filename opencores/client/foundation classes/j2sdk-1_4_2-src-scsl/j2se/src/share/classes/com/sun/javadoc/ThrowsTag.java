/*
 * @(#)ThrowsTag.java	1.7 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javadoc;

/**
 * Represents a @throws or @exception documentation tag.
 * Parses and holds the exception name and exception comment.
 * Note: @exception is a backwards compatible synonymy for @throws.
 *
 * @author Robert Field
 * @author Atul M Dambalkar
 * @see ExecutableMemberDoc#throwsTags()
 *
 */
public interface ThrowsTag extends Tag {

    /**
     * Return the name of the exception
     * associated with this <code>ThrowsTag</code>.
     *
     * @return name of the exception.
     */
    String exceptionName();

    /**
     * Return the exception comment
     * associated with this <code>ThrowsTag</code>.
     *
     * @return exception comment.
     */
    String exceptionComment();

    /**
     * Return a <code>ClassDoc</code> which represents the exception.
     *
     * @return <code>ClassDoc</code> which represents the exception.
     */
    ClassDoc exception();
}

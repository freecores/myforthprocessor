/*
 * @(#)ParamTag.java	1.7 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javadoc;

/**
 * Represents an @param documentation tag.
 * Parses and stores the name and comment parts of the
 * method/constructor parameter tag.
 *
 * @author Robert Field
 *
 */
public interface ParamTag extends Tag {

    /**
     * Return the name of the parameter
     * associated with this <code>ParamTag</code>.
     *
     * @return the parameter name.
     */
    String parameterName();

    /**
     * Return the parameter comment
     * associated with this <code>ParamTag</code>.
     *
     * @return the parameter comment.
     */
    String parameterComment();
}

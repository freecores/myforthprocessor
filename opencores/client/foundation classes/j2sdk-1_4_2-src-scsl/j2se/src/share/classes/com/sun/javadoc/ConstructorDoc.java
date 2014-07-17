/*
 * @(#)ConstructorDoc.java	1.6 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javadoc;

/**
 * Represents a constructor of a java class.
 *
 * @since JDK1.2
 * @author Robert Field
 */
public interface ConstructorDoc extends ExecutableMemberDoc {

    /**
     * Get the name.
     *
     * @return the qualified name of the member.
     */
    String qualifiedName();
}

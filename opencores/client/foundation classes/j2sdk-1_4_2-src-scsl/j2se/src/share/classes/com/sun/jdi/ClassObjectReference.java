/*
 * @(#)ClassObjectReference.java	1.9 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jdi;

/**
 * An instance of java.lang.Class from the target VM.
 * Use this interface to access type information 
 * for the class, array, or interface that this object reflects.
 *
 * @see ReferenceType
 *
 * @author Gordon Hirsch
 * @since  1.3
 */
public interface ClassObjectReference extends ObjectReference {

    /**
     * Returns the {@link ReferenceType} corresponding to this 
     * class object. The returned type can be used to obtain 
     * detailed information about the class.
     *
     * @return the {@link ReferenceType} reflected by this class object.
     */
    ReferenceType reflectedType();
}


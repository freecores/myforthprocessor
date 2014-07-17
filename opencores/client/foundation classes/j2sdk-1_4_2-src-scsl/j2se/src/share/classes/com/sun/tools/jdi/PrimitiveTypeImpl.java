/*
 * @(#)PrimitiveTypeImpl.java	1.13 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.jdi;

import com.sun.jdi.*;

abstract class PrimitiveTypeImpl extends TypeImpl implements PrimitiveType {

    PrimitiveTypeImpl(VirtualMachine vm) {
        super(vm);
    }

    /*
     * Converts the given primitive value to a value of this type.
     */
    abstract PrimitiveValue convert(PrimitiveValue value) throws InvalidTypeException;

    public String toString() {
        return name();
    }
}

/*
 * @(#)PrimitiveTypeImpl.java	1.1 02/01/07
 *
 * Copyright 2003 Sun Microsystems, Inc. All Rights Reserved.
 * 
 * This software is the proprietary information of Sun Microsystems, Inc.  
 * Use is subject to license terms.
 * 
 */

package sun.jvm.hotspot.jdi;

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

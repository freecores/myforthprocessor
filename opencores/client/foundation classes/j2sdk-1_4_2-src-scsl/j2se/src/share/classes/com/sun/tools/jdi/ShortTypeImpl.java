/*
 * @(#)ShortTypeImpl.java	1.12 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.jdi;

import com.sun.jdi.*;

public class ShortTypeImpl extends PrimitiveTypeImpl implements ShortType {
    ShortTypeImpl(VirtualMachine vm) {
        super(vm);
    }


    public String signature() {
        return String.valueOf((char)JDWP.Tag.SHORT);
    }

    PrimitiveValue convert(PrimitiveValue value) throws InvalidTypeException {
        return vm.mirrorOf(((PrimitiveValueImpl)value).checkedShortValue());
    }
    
}

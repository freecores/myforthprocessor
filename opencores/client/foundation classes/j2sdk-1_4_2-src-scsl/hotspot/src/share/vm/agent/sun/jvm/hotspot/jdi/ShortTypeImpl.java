/*
 * @(#)ShortTypeImpl.java	1.1 02/01/07  *
 * Copyright 2003 Sun Microsystems, Inc. All Rights Reserved.
 * 
 * This software is the proprietary information of Sun Microsystems, Inc.  
 * Use is subject to license terms.
 * 
 */

package sun.jvm.hotspot.jdi;

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

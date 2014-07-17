/*
 * @(#)CharTypeImpl.java	1.1 02/01/07
 *
 * Copyright 2003 Sun Microsystems, Inc. All Rights Reserved.
 * 
 * This software is the proprietary information of Sun Microsystems, Inc.  
 * Use is subject to license terms.
 * 
 */

package sun.jvm.hotspot.jdi;

import com.sun.jdi.*;

public class CharTypeImpl extends PrimitiveTypeImpl implements CharType {
    CharTypeImpl(VirtualMachine vm) {
        super(vm);
    }


    public String signature() {
        return "C";
    }
    
    PrimitiveValue convert(PrimitiveValue value) throws InvalidTypeException {
        return vm.mirrorOf(((PrimitiveValueImpl)value).checkedCharValue());
    }
    
}

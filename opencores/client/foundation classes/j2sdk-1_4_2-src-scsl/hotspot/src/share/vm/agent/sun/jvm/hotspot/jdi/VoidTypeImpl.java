/*
 * @(#)VoidTypeImpl.java	1.1 02/01/07
 *
 * Copyright 2003 Sun Microsystems, Inc. All Rights Reserved.
 * 
 * This software is the proprietary information of Sun Microsystems, Inc.  
 * Use is subject to license terms.
 * 
 */

package sun.jvm.hotspot.jdi;

import com.sun.jdi.*;

public class VoidTypeImpl extends TypeImpl implements VoidType {
    VoidTypeImpl(VirtualMachine vm) {
        super(vm);
    }

    public String signature() {
        return "V";
    }

    public String toString() {
        return name();
    }
}

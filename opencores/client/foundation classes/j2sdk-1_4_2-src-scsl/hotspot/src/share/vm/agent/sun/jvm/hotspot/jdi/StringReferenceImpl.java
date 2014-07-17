/*
 * @(#)StringReferenceImpl.java	1.1 02/03/04
 *
 * Copyright 2003 Sun Microsystems, Inc. All Rights Reserved.
 * 
 * This software is the proprietary information of Sun Microsystems, Inc.  
 * Use is subject to license terms.
 * 
 */

package sun.jvm.hotspot.jdi;

import com.sun.jdi.*;

public class StringReferenceImpl extends ObjectReferenceImpl
    implements StringReference
{
    private String value;

    StringReferenceImpl(VirtualMachine aVm, sun.jvm.hotspot.oops.Instance aRef) {
        super(aVm,aRef);
    }

    public String value() {  //fixme jjh
        if (value == null) {
            value = null;
//             // Does not need synchronization, since worst-case
//             // static info is fetched twice
//             try {
//                 value = JDWP.StringReference.Value.
//                     process(vm, this).stringValue;
//             } catch (JDWPException exc) {
//                 throw exc.toJDIException();
//             }
        }
        return value;
    }

    public String toString() {
        return "\"" + value() + "\"";
    }

//     byte typeValueKey() {
//         return JDWP.Tag.STRING;
//     }
}

/*
 * @(#)ClassObjectReferenceImpl.java	1.1 02/03/04
 *
 * Copyright 2003 Sun Microsystems, Inc. All Rights Reserved.
 * 
 * This software is the proprietary information of Sun Microsystems, Inc.  
 * Use is subject to license terms.
 * 
 */

package sun.jvm.hotspot.jdi;

import com.sun.jdi.*;
import java.util.*;

public class ClassObjectReferenceImpl extends ObjectReferenceImpl
                                      implements ClassObjectReference {
    private ReferenceType reflectedType;

    ClassObjectReferenceImpl(VirtualMachine vm, sun.jvm.hotspot.oops.Instance ref) {
        super(vm, ref);
    }

    public ReferenceType reflectedType() { //fixme jjh
        if (reflectedType == null) {
            reflectedType = null;
//              try {
//                 JDWP.ClassObjectReference.ReflectedType reply =
//                     JDWP.ClassObjectReference.ReflectedType.process(vm, this);
//                 reflectedType = vm.referenceType(reply.typeID,
//                                                  reply.refTypeTag);
                
//             } catch (JDWPException exc) {
//                 throw exc.toJDIException();
//             }
        }
        return reflectedType;
    }

//     byte typeValueKey() {
//         return JDWP.Tag.CLASS_OBJECT;
//     }

    public String toString() {
        return "instance of " + referenceType().name() + 
               "(reflected class=" + reflectedType().name() + ", " + "id=" + uniqueID() + ")";
    }
}

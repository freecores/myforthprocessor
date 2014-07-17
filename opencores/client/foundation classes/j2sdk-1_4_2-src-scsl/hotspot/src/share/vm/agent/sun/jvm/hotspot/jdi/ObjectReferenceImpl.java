/*
 * @(#)ObjectReferenceImpl.java	1.2 02/03/03
 *
 * Copyright 2003 Sun Microsystems, Inc. All Rights Reserved.
 * 
 * This software is the proprietary information of Sun Microsystems, Inc.  
 * Use is subject to license terms.
 * 
 */

package sun.jvm.hotspot.jdi;

import com.sun.jdi.*;

import sun.jvm.hotspot.oops.Instance;
import sun.jvm.hotspot.oops.OopUtilities;
import sun.jvm.hotspot.oops.Klass;
import java.util.*;
import java.util.ArrayList;

public class ObjectReferenceImpl extends ValueImpl implements ObjectReference {
    static long uniqueCounter = 0;
    protected Instance myInstance;
    private long myID;
    ObjectReferenceImpl(VirtualMachine aVm, sun.jvm.hotspot.oops.Instance obj) {
        super(aVm);
        myInstance = obj;
        myID = ++uniqueCounter;
    }

    Instance ref() {
        return myInstance;
    }

    public Type type() { 
        return referenceType();
    }

    public ReferenceType referenceType() {
        Klass myKlass = ref().getKlass();
        return jjvm.referenceType(myKlass);
    }


    public Value getValue(Field sig) {
        List list = new ArrayList(1);
        list.add(sig);
        Map map = getValues(list);
        return(Value)map.get(sig);
    }

    public Map getValues(List theFields) {  //fixme jjh:
        return null;
    }

    public void setValue(Field field, Value value) 
                   throws InvalidTypeException, ClassNotLoadedException {
        throw new UnsupportedOperationException("Not Read Only: ObjectReference.setValue(...)");
    }

    public Value invokeMethod(ThreadReference threadIntf, Method methodIntf, 
                              List arguments, int options) 
                              throws InvalidTypeException,
                                     IncompatibleThreadStateException,
                                     InvocationException,
                                     ClassNotLoadedException {
        throw new UnsupportedOperationException("Not Read Only: ObjectReference.invokeMethod(...)");
    }

    public void disableCollection() {
        throw new UnsupportedOperationException("Not Read Only: ObjectReference.disableCollection()");
    }

    public void enableCollection() {
        throw new UnsupportedOperationException("Not Read Only: ObjectReference.enableCollection()");
    }

    public boolean isCollected() {  //fixme jjh:
        return false;
    }

    public long uniqueID() {
        return myID;
    }

    public List waitingThreads() throws IncompatibleThreadStateException { //fixme jjh:
        return null;
    }

    public ThreadReference owningThread() throws IncompatibleThreadStateException { //fixme jjh:
        return null;
    }

    public int entryCount() throws IncompatibleThreadStateException { //fixme jjh:
        return 0;
    }


    public boolean equals(Object obj) {
        if ((obj != null) && (obj instanceof ObjectReferenceImpl)) {
            ObjectReferenceImpl other = (ObjectReferenceImpl)obj;
            return (ref() == other.ref()) &&
                   super.equals(obj);
        } else {
            return false;
        }
    }

    public int hashCode() { //fixme jjh:
        return 0;
    }
}

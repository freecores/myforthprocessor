/*
 * @(#)ThreadGroupReferenceImpl.java	1.1 02/03/04
 *
 * Copyright 2003 Sun Microsystems, Inc. All Rights Reserved.
 * 
 * This software is the proprietary information of Sun Microsystems, Inc.  
 * Use is subject to license terms.
 * 
 */

package sun.jvm.hotspot.jdi;

import sun.jvm.hotspot.oops.Oop;
import sun.jvm.hotspot.oops.Instance;
import sun.jvm.hotspot.oops.OopUtilities;
import sun.jvm.hotspot.runtime.JavaThread;
import com.sun.jdi.*;
import java.util.*;

public class ThreadGroupReferenceImpl extends ObjectReferenceImpl
    implements ThreadGroupReference
{
    ThreadGroupReferenceImpl(VirtualMachine aVm, sun.jvm.hotspot.oops.Instance obj) {
        super(aVm,obj);
    }

    protected String description() {
        return "ThreadGroupReference " + uniqueID();
    }

    public String name() { 
        return OopUtilities.threadGroupOopGetName(ref());
    }

    public ThreadGroupReference parent() {
        return (ThreadGroupReferenceImpl)jjvm.objectMirror(
                        (Instance)OopUtilities.threadGroupOopGetParent(ref()), 
                        JDWP.Tag.THREAD_GROUP);
    }

    public void suspend() {
        System.out.println("Not read only:  ThreadGroupReference.suspend()");
    }

    public void resume() {
        System.out.println("Not read only: ThreadGroupReference.resume()");
    }
  
    public List threads() {
        // Each element of this array is the Oop for a thread;
        // NOTE it is not the JavaThread that we need to create
        // a ThreadReferenceImpl.
        Oop[] myThreads = OopUtilities.threadGroupOopGetThreads(ref());

        ArrayList myList = new ArrayList(myThreads.length);
        for (int ii = 0; ii < myThreads.length; ii++) {
            JavaThread jt = OopUtilities.threadOopGetJavaThread(myThreads[ii]);
            if (jt != null) {
                ThreadReferenceImpl xx = (ThreadReferenceImpl)jjvm.objectMirror(
                        jt, JDWP.Tag.THREAD);
                myList.add(xx);
            }
        }
        return myList;
    }

    public List threadGroups() {
        Oop[] myGroups = OopUtilities.threadGroupOopGetGroups(ref());
        ArrayList myList = new ArrayList(myGroups.length);
        for (int ii = 0; ii < myGroups.length; ii++) {
            ThreadGroupReferenceImpl xx = (ThreadGroupReferenceImpl)jjvm.objectMirror(
                        (Instance)myGroups[ii], JDWP.Tag.THREAD_GROUP);
            myList.add(xx);

        }
        return myList;
    }

    public String toString() {
        return "instance of " + referenceType().name() + 
               "(name='" + name() + "', " + "id=" + uniqueID() + ")";
    }
}

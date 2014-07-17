/*
 * @(#)ThreadReferenceImpl.java	1.1 02/03/04
 *
 * Copyright 2003 Sun Microsystems, Inc. All Rights Reserved.
 * 
 * This software is the proprietary information of Sun Microsystems, Inc.  
 * Use is subject to license terms.
 * 
 */

package sun.jvm.hotspot.jdi;

import sun.jvm.hotspot.runtime.VMObject;
import sun.jvm.hotspot.runtime.JavaThread;
//import sun.jvm.hotspot.runtime.StackFrameStream;
import sun.jvm.hotspot.runtime.JavaVFrame;
import sun.jvm.hotspot.runtime.JavaThreadState;
import sun.jvm.hotspot.oops.Instance;
import sun.jvm.hotspot.oops.OopUtilities;
import sun.jvm.hotspot.oops.Klass;
import com.sun.jdi.*;
import java.util.*;

public class ThreadReferenceImpl extends ObjectReferenceImpl
             implements ThreadReference {
    static final int SUSPEND_STATUS_SUSPENDED = 0x1;
    static final int SUSPEND_STATUS_BREAK = 0x2;
    
    private JavaThread myJavaThread;
    private int suspendedZombieCount = 0;
    private int threadStatus;
    private boolean threadStatusSet = false;
    private ArrayList frames;    // StackFrames 

    ThreadReferenceImpl(VirtualMachine aVm, sun.jvm.hotspot.runtime.JavaThread aRef) {
        // We are given a JavaThread and save it in our myJavaThread field.
        // But, our parent class is an ObjectReferenceImpl so we need an Oop
        // for it.  JavaThread is a wrapper around a Thread Oop so we get
        // that Oop and give it to our super.
        // We can get it back again by calling ref().
        super(aVm, (Instance)aRef.getThreadObj());
        myJavaThread = aRef;
    }

    protected String description() {
        return "ThreadReference " + uniqueID();
    }


    /**
     * Note that we only cache the name string while suspended because 
     * it can change via Thread.setName arbitrarily
     */
    public String name() { 
        String name = myJavaThread.getThreadName();
        return name;
    }

    public void suspend() {
        System.out.println("Not read only:  ThreadReference.suspend()");
    }

    public void resume() {
        System.out.println("Not read only:  ThreadReference.resume()");
    }

    public int suspendCount() {  //fixme jjh missing
        return 0;
    }

    public void stop(ObjectReference throwable) throws InvalidTypeException {
        System.out.println("Not read only:  ThreadReference.stop()");
    }

    public void interrupt() {
        System.out.println("Not read only:  ThreadReference.interrupt()");
    }

    public int status() { //fixme jjh
        int threadStatus = THREAD_STATUS_UNKNOWN;
        JavaThreadState myState = myJavaThread.getThreadState();
        /* Here are the values that can be returned.  We have
           to map them into THREAD_STATUS_...
           UNINITIALIZED
           NEW
           NEW_TRANS         
           IN_NATIVE         
           IN_NATIVE_TRANS   
           IN_VM             
           IN_VM_TRANS       
           IN_JAVA           
           IN_JAVA_TRANS     
           BLOCKED          
           BLOCKED_TRANS     
           IN_NATIVE_BLOCKED 
        */
        threadStatus = THREAD_STATUS_UNKNOWN;
        if (myState == JavaThreadState.NEW ||
            myState == JavaThreadState.NEW_TRANS) {
            threadStatus = THREAD_STATUS_NOT_STARTED;
        } else if (myState == JavaThreadState.IN_NATIVE ||
                   myState == JavaThreadState.IN_NATIVE_TRANS ||
                   myState == JavaThreadState.IN_VM ||
                   myState == JavaThreadState.IN_VM_TRANS ||
                   myState == JavaThreadState.IN_JAVA ||
                   myState == JavaThreadState.IN_JAVA_TRANS) {
            threadStatus = THREAD_STATUS_RUNNING;
        } else if (myState == JavaThreadState.BLOCKED ||
                   myState == JavaThreadState.BLOCKED_TRANS ||
                   myState == JavaThreadState.IN_NATIVE_BLOCKED) {
            threadStatus = THREAD_STATUS_WAIT; //fixme jjh
        }
        return threadStatus;
    }

    public boolean isSuspended() { //fixme jjh
        // If we want to support doing this for a VM which was being
        // debugged, then we need to fix this.
        // In the meantime, we will say all threads are suspended,
        // otherwise, some things won't work, like the jdb 'up' cmd.
        return true;
    }

    public boolean isAtBreakpoint() { //fixme jjh
        // If we want to support doing this for a VM which was being
        // debugged, then we need to fix this.
        return false;
    }

    public ThreadGroupReference threadGroup() { 
        return (ThreadGroupReferenceImpl)jjvm.objectMirror(
                                    (Instance)OopUtilities.threadOopGetThreadGroup(ref()),
                                    JDWP.Tag.THREAD_GROUP);
    }

    public int frameCount() throws IncompatibleThreadStateException  { //fixme jjh
        privateFrames(0, -1);
        return frames.size();
    }

    public List frames() throws IncompatibleThreadStateException  {
        return privateFrames(0, -1);
    }

    public StackFrame frame(int index) throws IncompatibleThreadStateException  {
        List list = privateFrames(index, 1);
        return (StackFrame)list.get(0);
    }

    public List frames(int start, int length) 
                              throws IncompatibleThreadStateException  {
        if (length < 0) {
            throw new IndexOutOfBoundsException(
                "length must be greater than or equal to zero");
        }
        return privateFrames(start, length);
    }

    /**
     * Private version of frames() allows "-1" to specify all 
     * remaining frames.
     */

    private List privateFrames(int start, int length)  //fixme jjh
                              throws IncompatibleThreadStateException  {
        if (frames == null) {
            frames = new ArrayList(10);
            JavaVFrame myvf = myJavaThread.getLastJavaVFrameDbg();
            while (myvf != null) {
                StackFrame myFrame = new StackFrameImpl(vm, this, myvf)
                    ;//fixme jjh null should be a Location
                frames.add(myFrame);
                myvf = (JavaVFrame)myvf.sender();
            }
//             for (StackFrameStream fst = new StackFrameStream(this.myJavaThread);
//                  !fst.isDone(); fst.next()) {
//                 Frame ff = fst.getCurrent();
//                 //                Location ll = new Location(
//                 StackFrame myFrame = new StackFrameImpl(vm, this, ff, null); //fixme jjh null should be a Location
//                 frames.add(myFrame);
//             }
        }

        List retVal;
        if (frames.size() == 0) {
            retVal = new ArrayList(0);
        } else {
            int toIndex = start + length;
            if (length == -1) {
                toIndex = frames.size();
            }
            retVal = frames.subList(start, toIndex);
        }
        return Collections.unmodifiableList(retVal);
    }

    public List ownedMonitors()  throws IncompatibleThreadStateException  { //fixme jjh missing
        List monitors = null;
//         try {
//             Cache local = (Cache)getCache();

//             if (local != null) {
//                 monitors = local.ownedMonitors;
//             }
//             if (monitors == null) {
//                 monitors = Arrays.asList(
//                                  JDWP.ThreadReference.OwnedMonitors.
//                                          process(vm, this).owned);
//                 if (local != null) {
//                     local.ownedMonitors = monitors;
//                     if ((vm.traceFlags & vm.TRACE_OBJREFS) != 0) {
//                         vm.printTrace(description() + 
//                                       " temporarily caching owned monitors"+
//                                       " (count = " + monitors.size() + ")");
//                     }
//                 }
//             }
//         } catch (JDWPException exc) {
//             switch (exc.errorCode()) {
//             case JDWP.Error.THREAD_NOT_SUSPENDED:
//             case JDWP.Error.INVALID_THREAD:   /* zombie */
//                 throw new IncompatibleThreadStateException();
//             default:
//                 throw exc.toJDIException();
//             }
//         }
        return monitors;
    }

    public ObjectReference currentContendedMonitor()  //fixme jjh missing
                              throws IncompatibleThreadStateException  {
        ObjectReference monitor = null;
//         try {
//             Cache local = (Cache)getCache();

//             if (local != null && local.triedCurrentContended) {
//                 monitor = local.contendedMonitor;
//             } else {
//                 monitor = JDWP.ThreadReference.CurrentContendedMonitor.
//                     process(vm, this).monitor;
//                 if (local != null) {
//                     local.triedCurrentContended = true;
//                     local.contendedMonitor = monitor;
//                     if ((monitor != null) &&
//                         ((vm.traceFlags & vm.TRACE_OBJREFS) != 0)) {
//                         vm.printTrace(description() + 
//                               " temporarily caching contended monitor"+
//                               " (id = " + monitor.uniqueID() + ")");
//                     }
//                 }
//             }
//         } catch (JDWPException exc) {
//             switch (exc.errorCode()) {
//             case JDWP.Error.THREAD_NOT_SUSPENDED:
//             case JDWP.Error.INVALID_THREAD:   /* zombie */
//                 throw new IncompatibleThreadStateException();
//             default:
//                 throw exc.toJDIException();
//             }
//         }
        return monitor;
    }

    public void popFrames(StackFrame frame) throws IncompatibleThreadStateException {
        System.out.println("Not read only:  ThreadReference.popFrames()");
    }

    public String toString() {
        return "instance of " + referenceType().name() + 
               "(name='" + name() + "', " + "id=" + uniqueID() + ")";
    }

//     byte typeValueKey() {
//         return JDWP.Tag.THREAD;
//     }
}

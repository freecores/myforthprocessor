/*
 * @(#)StackFrameImpl.java	1.1 02/03/04
 *
 * Copyright 2003 Sun Microsystems, Inc. All Rights Reserved.
 * 
 * This software is the proprietary information of Sun Microsystems, Inc.  
 * Use is subject to license terms.
 * 
 */

package sun.jvm.hotspot.jdi;

import com.sun.jdi.*;
import sun.jvm.hotspot.runtime.JavaVFrame;
import java.util.List;
import java.util.Map;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Collections;

public class StackFrameImpl extends MirrorImpl 
                            implements StackFrame
{
    /* Once false, frame should not be used.
     * access synchronized on (vm.state())
     */
    private boolean isValid = true;  

    private final ThreadReferenceImpl thread;
    private final JavaVFrame SAid;
    private final Location location;
    private Map visibleVariables =  null;
    private ObjectReference thisObject = null;

    StackFrameImpl(VirtualMachine vm, ThreadReferenceImpl thread, 
                   JavaVFrame jvf) {
        super(vm);
        this.thread = thread;
        this.SAid = jvf;
        
        sun.jvm.hotspot.oops.Method SAMethod = jvf.getMethod();
        
        ReferenceType rt = ((VirtualMachineImpl)vm).referenceType(SAMethod.getMethodHolder());
        
        this.location = new LocationImpl(vm, rt, SAMethod, (long)jvf.getBCI());
    }

    private void validateStackFrame() {
        if (!isValid) {
            throw new InvalidStackFrameException("Thread has been resumed");
        }
    }

    /**
     * Return the frame location.
     * Need not be synchronized since it cannot be provably stale.
     */
    public Location location() {
        validateStackFrame();
        return location;
    }

    /**
     * Return the thread holding the frame.
     * Need not be synchronized since it cannot be provably stale.
     */
    public ThreadReference thread() {
        validateStackFrame();
        return thread;
    }

    public boolean equals(Object obj) {
        if ((obj != null) && (obj instanceof StackFrameImpl)) {
            StackFrameImpl other = (StackFrameImpl)obj;
            return (SAid == other.SAid) &&
                   (thread().equals(other.thread())) &&
                   (location().equals(other.location())) &&
                    super.equals(obj);
        } else {
            return false;
        }
    }

    public int hashCode() {
        return (thread().hashCode() << 4) + 89; //fixme jjh ((int)SAid);
    }

    public ObjectReference thisObject() { //fixme jjh
        validateStackFrame();
        MethodImpl currentMethod = (MethodImpl)location.method();
        if (currentMethod.isStatic() || currentMethod.isNative()) {
            return null;
        } 

        if (thisObject == null) {
//             PacketStream ps;

//             /* protect against defunct frame id */
//             synchronized (vm.state()) {
//                 validateStackFrame();
//                 ps = JDWP.StackFrame.ThisObject.
//                     enqueueCommand(vm, thread, id);
//             }
            
//             /* actually get it, now that order is guaranteed */
//             try {
//                 thisObject = JDWP.StackFrame.ThisObject.
//                     waitForReply(vm, ps).objectThis;
//             } catch (JDWPException exc) {
//                 switch (exc.errorCode()) {
//                 case JDWP.Error.INVALID_FRAMEID:
//                 case JDWP.Error.THREAD_NOT_SUSPENDED:
//                 case JDWP.Error.INVALID_THREAD:
//                     throw new InvalidStackFrameException();
//                 default:
//                     throw exc.toJDIException();
//                 }
//             }
        }
        return thisObject;
    }

    /**
     * Build the visible variable map.
     * Need not be synchronized since it cannot be provably stale.
     */
    private void createVisibleVariables() throws AbsentInformationException {
        if (visibleVariables == null) {
            List allVariables = location.method().variables();
            Map map = new HashMap(allVariables.size());
        
            Iterator iter = allVariables.iterator();
            while (iter.hasNext()) {
                LocalVariableImpl variable = (LocalVariableImpl)iter.next();
                String name = variable.name();
                if (variable.isVisible(this)) {
                    LocalVariable existing = (LocalVariable)map.get(name);
                    if ((existing == null) || 
                        variable.hides(existing)) {
                        map.put(name, variable);
                    }
                }
            }
            visibleVariables = map;
        }
    }

    /**
     * Return the list of visible variable in the frame.
     * Need not be synchronized since it cannot be provably stale.
     */
    public List visibleVariables() throws AbsentInformationException {
        validateStackFrame();
        createVisibleVariables();
        List mapAsList = new ArrayList(visibleVariables.values());
        Collections.sort(mapAsList);
        return mapAsList;
    }

    /**
     * Return a particular variable in the frame.
     * Need not be synchronized since it cannot be provably stale.
     */
    public LocalVariable visibleVariableByName(String name) throws AbsentInformationException  {
        validateStackFrame();
        createVisibleVariables();
        return (LocalVariable)visibleVariables.get(name);
    }

    public Value getValue(LocalVariable variable) {
        List list = new ArrayList(1);
        list.add(variable);
        Map map = getValues(list);
        return (Value)map.get(variable);
    }

    public Map getValues(List variables) {  //fixme jjh
        validateStackFrame();
//      validateMirrors(variables);

        int count = variables.size();
//         JDWP.StackFrame.GetValues.SlotInfo[] slots = 
//                            new JDWP.StackFrame.GetValues.SlotInfo[count];

        for (int i=0; i<count; ++i) {
            LocalVariableImpl variable = (LocalVariableImpl)variables.get(i);
            if (!variable.isVisible(this)) {
                throw new IllegalArgumentException(variable.name() + 
                                 " is not valid at this frame location");
            }
//             slots[i] = new JDWP.StackFrame.GetValues.SlotInfo(variable.slot(),
//                                       (byte)variable.signature().charAt(0));
        }

//         PacketStream ps;

//         /* protect against defunct frame id */
//         synchronized (vm.state()) {
//             validateStackFrame();
//             ps = JDWP.StackFrame.GetValues.enqueueCommand(vm, thread, id, slots);
//         }

        /* actually get it, now that order is guaranteed */
        ValueImpl[] values = null;
//         try {
//             values = JDWP.StackFrame.GetValues.waitForReply(vm, ps).values;
//         } catch (JDWPException exc) {
//             switch (exc.errorCode()) {
//                 case JDWP.Error.INVALID_FRAMEID:
//                 case JDWP.Error.THREAD_NOT_SUSPENDED:
//                 case JDWP.Error.INVALID_THREAD:
//                     throw new InvalidStackFrameException();
//                 default:
//                     throw exc.toJDIException();
//             }
//         }

        if (count != values.length) {
            throw new InternalException(
                      "Wrong number of values returned from target VM");
        }
        Map map = new HashMap(count);
        for (int i=0; i<count; ++i) {
            LocalVariableImpl variable = (LocalVariableImpl)variables.get(i);
            map.put(variable, values[i]);
        }
        return map;
    }

    public void setValue(LocalVariable variableIntf, Value valueIntf)
        throws InvalidTypeException, ClassNotLoadedException {

        System.out.println("Not read only: StackFrame.setValue()");
    }

//     void pop() throws IncompatibleThreadStateException {
//         validateStackFrame();
//         // flush caches and disable caching until command completion
//         CommandSender sender = 
//             new CommandSender() {
//                 public PacketStream send() {
//                     return JDWP.StackFrame.PopFrames.enqueueCommand(vm, 
//                                  thread, id);
//                 }
//         };
//         try {
//             PacketStream stream = thread.sendResumingCommand(sender);
//             JDWP.StackFrame.PopFrames.waitForReply(vm, stream);
//         } catch (JDWPException exc) {
//             switch (exc.errorCode()) {
//             case JDWP.Error.THREAD_NOT_SUSPENDED:
//                 throw new IncompatibleThreadStateException(
//                          "Thread not current or suspended");
//             case JDWP.Error.INVALID_THREAD:   /* zombie */
//                 throw new IncompatibleThreadStateException("zombie");
//             case JDWP.Error.NO_MORE_FRAMES:
//                 throw new InvalidStackFrameException(
//                          "No more frames on the stack");
//             default:
//                 throw exc.toJDIException();
//             }
//         }

//         // enable caching - suspended again
//         vm.state().freeze();
//     }

    public String toString() {
        return location.toString() + " in thread " + thread.toString();
    }
}

 

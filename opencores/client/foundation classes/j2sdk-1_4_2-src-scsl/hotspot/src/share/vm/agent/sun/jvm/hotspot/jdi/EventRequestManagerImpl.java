/*
 * @(#)EventRequestManagerImpl.java	1.2 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.jvm.hotspot.jdi;

import com.sun.jdi.*;
import com.sun.jdi.request.*;

import java.util.*;

/**
 * This interface is used to create and remove Breakpoints, Watchpoints,
 * etc.
 * It include implementations of all the request interfaces..
 */
class EventRequestManagerImpl extends MirrorImpl
                                       implements EventRequestManager
{
    static class EventKind {
        static final int VM_DISCONNECTED = 100;
        static final int VM_START = EventRequestManagerImpl.EventKind.VM_INIT;
        static final int THREAD_DEATH = EventRequestManagerImpl.EventKind.THREAD_END;
        static final int SINGLE_STEP = 1;
        static final int BREAKPOINT = 2;
        static final int FRAME_POP = 3;
        static final int EXCEPTION = 4;
        static final int USER_DEFINED = 5;
        static final int THREAD_START = 6;
        static final int THREAD_END = 7;
        static final int CLASS_PREPARE = 8;
        static final int CLASS_UNLOAD = 9;
        static final int CLASS_LOAD = 10;
        static final int FIELD_ACCESS = 20;
        static final int FIELD_MODIFICATION = 21;
        static final int EXCEPTION_CATCH = 30;
        static final int METHOD_ENTRY = 40;
        static final int METHOD_EXIT = 41;
        static final int VM_INIT = 90;
        static final int VM_DEATH = 99;
    }

    List[] requestLists;

    /*
     * Override superclass back to default equality
     */
    public boolean equals(Object obj) {
        return this == obj;
    }

    public int hashCode() {
        return System.identityHashCode(this);
    }

    abstract class EventRequestImpl extends MirrorImpl implements EventRequest {
        int id;

        /*
         * This list is not protected by a synchronized wrapper. All 
         * access/modification should be protected by synchronizing on
         * the enclosing instance of EventRequestImpl.
         */
        List filters = new ArrayList();
                                               
        boolean isEnabled = false;
        boolean deleted = false;
        int suspendPolicy = 0;
        private Map clientProperties = null;
    
        EventRequestImpl() {
            super(EventRequestManagerImpl.this.vm);
            requestList().add(this);
        }

        /*
         * Override superclass back to default equality
         */
        public boolean equals(Object obj) {
            return this == obj;
        }
    
        public int hashCode() {
            return System.identityHashCode(this);
        }
    
        abstract int eventCmd();

        InvalidRequestStateException invalidState() {
            return new InvalidRequestStateException(toString());
        }

        String state() {
            return deleted? " (deleted)" : 
                (isEnabled()? " (enabled)" : " (disabled)");
        }

        /**
         * @return all the event request of this kind
         */
        List requestList() {
            return EventRequestManagerImpl.this.requestList(eventCmd());
        }
 
        /**
         * delete the event request 
         */
        void delete() {
            if (!deleted) {
                requestList().remove(this);
                disable(); /* must do BEFORE delete */
                deleted = true;
            }
        }        

        public boolean isEnabled() {
            return isEnabled;
        }

        public void enable() {
            setEnabled(true);
        }

        public void disable() {
            setEnabled(false);
        }

        public synchronized void setEnabled(boolean val) {
            if (deleted) {
                throw invalidState();
            } else {
                if (val != isEnabled) {
                    if (isEnabled) {
                        clear();
                    } else {
                        set();
                    }
                }
            }
        }

        public synchronized void addCountFilter(int count) {
            if (isEnabled() || deleted) {
                throw invalidState();
            }
            if (count < 1) {
                throw new IllegalArgumentException("count is less than one");
            }
        }

        public void setSuspendPolicy(int policy) {
            if (isEnabled() || deleted) {
                throw invalidState();
            }
            suspendPolicy = policy;
        }

        public int suspendPolicy() {
            return suspendPolicy;
        }

        /**
         * set (enable) the event request 
         */
        synchronized void set() {
            isEnabled = true;
        }

        synchronized void clear() {
            isEnabled = false;
        }
  
        /**
         * @return a small Map
         * @see #putProperty
         * @see #getProperty
         */
        private Map getProperties() {
            if (clientProperties == null) {
                clientProperties = new HashMap(2);
            }
            return clientProperties;
        }

        /**
         * Returns the value of the property with the specified key.  Only
         * properties added with <code>putProperty</code> will return
         * a non-null value.  
         * 
         * @return the value of this property or null
         * @see #putProperty
         */
        public final Object getProperty(Object key) {
            if (clientProperties == null) {
                return null;
            } else {
                return getProperties().get(key);
            }
        }

        /**
         * Add an arbitrary key/value "property" to this component.
         * 
         * @see #getProperty
         */
        public final void putProperty(Object key, Object value) {
            if (value != null) {
                getProperties().put(key, value);
            } else {
                getProperties().remove(key);
            }
        }
    }

    abstract class ThreadVisibleEventRequestImpl extends EventRequestImpl {
        public synchronized void addThreadFilter(ThreadReference thread) {
            // validateMirror(thread);
            if (isEnabled() || deleted) {
                throw invalidState();
            }
        }
    }

    abstract class ClassVisibleEventRequestImpl 
                                  extends ThreadVisibleEventRequestImpl {
        public synchronized void addClassFilter(ReferenceType clazz) {
            // validateMirror(clazz);
            if (isEnabled() || deleted) {
                throw invalidState();
            }
        }

        public synchronized void addClassFilter(String classPattern) {
            if (isEnabled() || deleted) {
                throw invalidState();
            }
            if (classPattern == null) {
                throw new NullPointerException();
            }
        }

        public synchronized void addClassExclusionFilter(String classPattern) {
            if (isEnabled() || deleted) {
                throw invalidState();
            }
            if (classPattern == null) {
                throw new NullPointerException();
            }
        }

        public synchronized void addInstanceFilter(ObjectReference instance) {
            // validateMirror(instance);
            if (isEnabled() || deleted) {
                throw invalidState();
            }
            if (!vm.canUseInstanceFilters()) {
                throw new UnsupportedOperationException(
                     "target does not support instance filters");
            }
        }
    }

    class BreakpointRequestImpl extends ClassVisibleEventRequestImpl 
                                     implements BreakpointRequest {
        private final Location location;

        BreakpointRequestImpl(Location location) {
            this.location = location;
        }

        public Location location() {
            return location;
        }

        int eventCmd() {
            return EventRequestManagerImpl.EventKind.BREAKPOINT;
        }

        public String toString() {
            return "breakpoint request " + location() + state();
        }
    }

    class ClassPrepareRequestImpl extends ClassVisibleEventRequestImpl
                                     implements ClassPrepareRequest {
        int eventCmd() {
            return EventRequestManagerImpl.EventKind.CLASS_PREPARE;
        }

        public String toString() {
            return "class prepare request " + state();
        }
    }

    class ClassUnloadRequestImpl extends ClassVisibleEventRequestImpl
                                     implements ClassUnloadRequest {   
        int eventCmd() {
            return EventRequestManagerImpl.EventKind.CLASS_UNLOAD;
        }

        public String toString() {
            return "class unload request " + state();
        }
    }

    class ExceptionRequestImpl extends ClassVisibleEventRequestImpl 
                                      implements ExceptionRequest {
        ReferenceType exception = null;
        boolean caught = true;
        boolean uncaught = true;
    
        ExceptionRequestImpl(ReferenceType refType, 
                          boolean notifyCaught, boolean notifyUncaught) {
            exception = refType;
            caught = notifyCaught;
            uncaught = notifyUncaught;
        }

        public ReferenceType exception() {
            return exception;
        }

        public boolean notifyCaught() {
            return caught;
        }

        public boolean notifyUncaught() {
            return uncaught;
        }

        int eventCmd() {
            return EventRequestManagerImpl.EventKind.EXCEPTION;
        }

        public String toString() {
            return "exception request " + exception() + state();
        }
    }

    class MethodEntryRequestImpl extends ClassVisibleEventRequestImpl
                                      implements MethodEntryRequest {
        int eventCmd() {
            return EventRequestManagerImpl.EventKind.METHOD_ENTRY;
        }

        public String toString() {
            return "method entry request " + state();
        }
    }

    class MethodExitRequestImpl extends ClassVisibleEventRequestImpl
                                      implements MethodExitRequest {
        int eventCmd() {
            return EventRequestManagerImpl.EventKind.METHOD_EXIT;
        }

        public String toString() {
            return "method exit request " + state();
        }
    }

    class StepRequestImpl extends ClassVisibleEventRequestImpl
                                      implements StepRequest {
        ThreadReferenceImpl thread;
        int size;
        int depth;

        StepRequestImpl(ThreadReference thread, int size, int depth) {
            this.thread = (ThreadReferenceImpl)thread;
            this.size = size;
            this.depth = depth;


            /*
             * Make sure this isn't a duplicate
             */
            List requests = stepRequests();
            Iterator iter = requests.iterator();
            while (iter.hasNext()) {
                StepRequest request = (StepRequest)iter.next();
                if ((request != this) &&
                        request.isEnabled() &&
                        request.thread().equals(thread)) {
                    throw new DuplicateRequestException(
                        "Only one step request allowed per thread");
                }
            }
        }
        public int depth() {
            return depth;
        }

        public int size() {
            return size;
        }

        public ThreadReference thread() {
            return thread;
        }

        int eventCmd() {
            return EventRequestManagerImpl.EventKind.SINGLE_STEP;
        }

        public String toString() {
            return "step request " + thread() + state();
        }
    }

    class ThreadDeathRequestImpl extends ThreadVisibleEventRequestImpl
                                      implements ThreadDeathRequest {
        int eventCmd() {
            return EventRequestManagerImpl.EventKind.THREAD_DEATH;
        }

        public String toString() {
            return "thread death request " + state();
        }
    }

    class ThreadStartRequestImpl extends ThreadVisibleEventRequestImpl 
                                      implements ThreadStartRequest {    
        int eventCmd() {
            return EventRequestManagerImpl.EventKind.THREAD_START;
        }

        public String toString() {
            return "thread start request " + state();
        }
    }

    abstract class WatchpointRequestImpl extends ClassVisibleEventRequestImpl 
                                      implements WatchpointRequest {
        final Field field;
    
        WatchpointRequestImpl(Field field) {
            this.field = field;
        }

        public Field field() {
            return field;
        }
    }

    class AccessWatchpointRequestImpl extends WatchpointRequestImpl 
                                  implements AccessWatchpointRequest {
        AccessWatchpointRequestImpl(Field field) {
            super(field);
        }

        int eventCmd() {
            return EventRequestManagerImpl.EventKind.FIELD_ACCESS;
        }

        public String toString() {
            return "access watchpoint request " + field + state();
        }
    }

    class ModificationWatchpointRequestImpl extends WatchpointRequestImpl 
                                  implements ModificationWatchpointRequest {
        ModificationWatchpointRequestImpl(Field field) {
            super(field);
        }
    
        int eventCmd() {
            return EventRequestManagerImpl.EventKind.FIELD_MODIFICATION;
        }

        public String toString() {
            return "modification watchpoint request " + field + state();
        }
    }

    class VMDeathRequestImpl extends EventRequestImpl 
                                        implements VMDeathRequest {
        int eventCmd() {
            return EventRequestManagerImpl.EventKind.VM_DEATH;
        }

        public String toString() {
            return "VM death request " + state();
        }
    }

    /**	
     * Constructor.
     */
    EventRequestManagerImpl(VirtualMachine vm) {
        super(vm);
        java.lang.reflect.Field[] ekinds = 
            EventRequestManagerImpl.EventKind.class.getDeclaredFields();
        int highest = 0;
        for (int i = 0; i < ekinds.length; ++i) {
            int val;
            try {
                val = ekinds[i].getInt(null);
            } catch (IllegalAccessException exc) {
                throw new RuntimeException("Got: " + exc);
            }
            if (val > highest) {
                highest = val;
            }
        }
        requestLists = new List[highest+1];
        for (int i=0; i <= highest; i++) {
            requestLists[i] = new ArrayList();
        }
    }

    public ClassPrepareRequest createClassPrepareRequest() {
        return new ClassPrepareRequestImpl();
    }

    public ClassUnloadRequest createClassUnloadRequest() {
        return new ClassUnloadRequestImpl();
    }

    public ExceptionRequest createExceptionRequest(ReferenceType refType, 
                                                   boolean notifyCaught,  
                                                   boolean notifyUncaught) {
        // validateMirrorOrNull(refType);
        return new ExceptionRequestImpl(refType, notifyCaught, notifyUncaught);
    }

    public StepRequest createStepRequest(ThreadReference thread,
                                         int size, int depth) {
        // validateMirror(thread);
        return new StepRequestImpl(thread, size, depth);
    }

    public ThreadDeathRequest createThreadDeathRequest() {
        return new ThreadDeathRequestImpl();
    }

    public ThreadStartRequest createThreadStartRequest() {
        return new ThreadStartRequestImpl();
    }

    public MethodEntryRequest createMethodEntryRequest() {
        return new MethodEntryRequestImpl();
    }

    public MethodExitRequest createMethodExitRequest() {
        return new MethodExitRequestImpl();
    }

    public BreakpointRequest createBreakpointRequest(Location location) {
        // validateMirror(location);
        if (location.codeIndex() == -1) {
            throw new NativeMethodException("Cannot set breakpoints on native methods");
        }
        return new BreakpointRequestImpl(location);
    }

    public AccessWatchpointRequest 
                              createAccessWatchpointRequest(Field field) {
        // validateMirror(field);
        if (!vm.canWatchFieldAccess()) {
            throw new UnsupportedOperationException(
          "target VM does not support access watchpoints");
        }
        return new AccessWatchpointRequestImpl(field);
    }

    public ModificationWatchpointRequest 
                        createModificationWatchpointRequest(Field field) {
        // validateMirror(field);
        if (!vm.canWatchFieldModification()) {
            throw new UnsupportedOperationException(
          "target VM does not support modification watchpoints");
        }
        return new ModificationWatchpointRequestImpl(field);
    }

    public VMDeathRequest createVMDeathRequest() {
        if (!vm.canRequestVMDeathEvent()) {
            throw new UnsupportedOperationException(
          "target VM does not support requesting VM death events");
        }
        return new VMDeathRequestImpl();
    }

    public void deleteEventRequest(EventRequest eventRequest) {
        // validateMirror(eventRequest);
        ((EventRequestImpl)eventRequest).delete();
    }

    public void deleteEventRequests(List eventRequests) {
        // validateMirrors(eventRequests);
        // copy the eventRequests to avoid ConcurrentModificationException
        Iterator iter = (new ArrayList(eventRequests)).iterator();
        while (iter.hasNext()) {
            ((EventRequestImpl)iter.next()).delete();
        }
    }

    public void deleteAllBreakpoints() {
        requestList(EventRequestManagerImpl.EventKind.BREAKPOINT).clear();
    }
  
    public List stepRequests() {
        return unmodifiableRequestList(EventRequestManagerImpl.EventKind.SINGLE_STEP);
    }

    public List classPrepareRequests() {
        return unmodifiableRequestList(EventRequestManagerImpl.EventKind.CLASS_PREPARE);
    }

    public List classUnloadRequests() {
        return unmodifiableRequestList(EventRequestManagerImpl.EventKind.CLASS_UNLOAD);
    }

    public List threadStartRequests() {
        return unmodifiableRequestList(EventRequestManagerImpl.EventKind.THREAD_START);
    }

    public List threadDeathRequests() {
        return unmodifiableRequestList(EventRequestManagerImpl.EventKind.THREAD_DEATH);
    }

    public List exceptionRequests() {
        return unmodifiableRequestList(EventRequestManagerImpl.EventKind.EXCEPTION);
    }

    public List breakpointRequests() {
        return unmodifiableRequestList(EventRequestManagerImpl.EventKind.BREAKPOINT);
    }

    public List accessWatchpointRequests() {
        return unmodifiableRequestList(EventRequestManagerImpl.EventKind.FIELD_ACCESS);
    }

    public List modificationWatchpointRequests() {
        return unmodifiableRequestList(EventRequestManagerImpl.EventKind.FIELD_MODIFICATION);
    }

    public List methodEntryRequests() {
        return unmodifiableRequestList(EventRequestManagerImpl.EventKind.METHOD_ENTRY);
    }

    public List methodExitRequests() {
        return unmodifiableRequestList(EventRequestManagerImpl.EventKind.METHOD_EXIT);
    }

    public List vmDeathRequests() {
        return unmodifiableRequestList(EventRequestManagerImpl.EventKind.VM_DEATH);
    }

    List unmodifiableRequestList(int eventCmd) {
        return Collections.unmodifiableList(requestList(eventCmd));
    }

    EventRequest request(int eventCmd, int requestId) {
        List rl = requestList(eventCmd);
        for (int i = rl.size() - 1; i >= 0; i--) {
            EventRequestImpl er = (EventRequestImpl)rl.get(i);
            if (er.id == requestId) {
                return er;
            }
        }
        return null;
    }
        
    List requestList(int eventCmd) {
        return requestLists[eventCmd];
    }

}



			

/*
 * @(#)ClassPrepareEvent.java	1.14 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jdi.event;

import com.sun.jdi.*;

/**
 * Notification of a class prepare in the target VM. See the JVM 
 * specification for a definition of class preparation. Class prepare
 * events are not generated for primtiive classes (for example, 
 * java.lang.Integer.TYPE).
 *
 * @see EventQueue
 * @see VirtualMachine
 *
 * @author Robert Field
 * @since  1.3
 */
public interface ClassPrepareEvent extends Event {
    /**
     * Returns the thread in which this event has occurred. 
     * <p>
     * In rare cases, this event may occur in a debugger system 
     * thread within the target VM. Debugger threads take precautions
     * to prevent these events, but they cannot be avoided under some
     * conditions, especially for some subclasses of 
     * {@link java.lang.Error}. 
     * If the event was generated by a debugger system thread, the 
     * value returned by this method is null, and if the requested 
     * suspend policy for the event was 
     * {@link com.sun.jdi.request.EventRequest#SUSPEND_EVENT_THREAD},
     * all threads will be suspended instead, and the 
     * {@link EventSet#suspendPolicy} will reflect this change. 
     * <p>
     * Note that the discussion above does not apply to system threads
     * created by the target VM during its normal (non-debug) operation.
     *
     * @return a {@link ThreadReference} which mirrors the event's thread in 
     * the target VM, or null in the rare cases described above. 
     */
    public ThreadReference thread();
    
    /**
     * Returns the reference type for which this event was generated.
     *
     * @return a {@link ReferenceType} which mirrors the class, interface, or
     * array which has been linked.
     */
    public ReferenceType referenceType();
}


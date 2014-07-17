/*
 * @(#)ThreadGroupReference.java	1.14 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jdi;

import java.util.List;

/**
 * A thread group object from the target VM.
 * A ThreadGroupReference is an {@link ObjectReference} with additional
 * access to threadgroup-specific information from the target VM.
 *
 * @author Robert Field
 * @author Gordon Hirsch
 * @author James McIlree
 * @since  1.3
 */
public interface ThreadGroupReference extends ObjectReference
{
    /**
     * Returns the name of this thread group.
     * 
     * @return the string containing the thread group name.
     */
    String name();

    /**
     * Returns the parent of this thread group.
     *
     * @return a {@link ThreadGroupReference} mirroring the parent of this
     * thread group in the target VM, or null if this is a top-level
     * thread group.
     */
    ThreadGroupReference parent();

    /**
     * Suspends all threads in this thread group. Each thread
     * in this group and in all of its subgroups will be 
     * suspended as described in {@link ThreadReference#suspend}.
     * This is not guaranteed to be an atomic
     * operation; if the target VM is not interrupted at the time 
     * this method is
     * called, it is possible that new threads will be created 
     * between the time that threads are enumerated and all of them
     * have been suspended.
     */
    void suspend();

    /**
     * Resumes all threads in this thread group. Each thread
     * in this group and in all of its subgroups will be 
     * resumed as described in {@link ThreadReference#resume}.
     */
    void resume();

    /**
     * Returns a List containing each {@link ThreadReference} in this
     * thread group. Only the threads in this immediate thread group
     * (and not its subgroups) are returned.
     * 
     * @return a List of {@link ThreadReference} objects mirroring the 
     * threads from this thread group in the target VM.
     */
    List threads();

    /**
     * Returns a List containing each {@link ThreadGroupReference} in this
     * thread group. Only the thread groups in this immediate thread group
     * (and not its subgroups) are returned.
     * 
     * @return a List of {@link ThreadGroupReference} objects mirroring the 
     * threads from this thread group in the target VM.
     */
    List threadGroups();
}

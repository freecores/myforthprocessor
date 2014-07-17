/*
 * @(#)ProgressTracker.java	1.18 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.util;

import java.lang.ref.SoftReference;
import java.util.HashMap;
import java.util.Set;


/**
 * ProgressTracker is a class that provides basic progress tracking capability 
 * in Java Plug-in.
 */
public class ProgressTracker
{
    // HashMap for ThreadGroups
    private static HashMap threadGroups = new HashMap();
    
    /** 
     * Start progress binding. 
     *
     * @param identifier Object to be binded
     */
    public static void onStartBinding(Object identifier)
    {
	// Find the proper progress listener from the registered list 
	// accordingly to the caller thread group.
	//

	// 
	// It is VERY VERY important that the event dispatch 
	// doesn't occur until we get out of the synchronization
	// block. Otherwise, multiple dispatches from different
	// threads in the same thread group may result in
	// deadlock in case the callee is blocked
	//
	Thread t = Thread.currentThread();
	ThreadGroup tg = t.getThreadGroup();
	ProgressListener listener = null;

	synchronized(threadGroups)
	{
	    listener = (ProgressListener) threadGroups.get(new Integer(tg.hashCode()));
	}

	// We MUST not be in synchronization block 
	// when we do dispatch
	//
	if (listener != null)
	{
	    listener.onStartBinding(identifier);
	}
    }


    /** 
     * Stop progress binding. 
     *
     * @param identifier Object to be unbinded
     */
    public static void onStopBinding(Object identifier)
    {
	// Find the proper progress listener from the registered list 
	// accordingly to the caller thread group.
	//

	// 
	// It is VERY VERY important that the event dispatch 
	// doesn't occur until we get out of the synchronization
	// block. Otherwise, multiple dispatches from different
	// threads in the same thread group may result in
	// deadlock in case the callee is blocked
	//
	Thread t = Thread.currentThread();
	ThreadGroup tg = t.getThreadGroup();
	ProgressListener listener = null;

	synchronized(threadGroups)
	{
	    listener = (ProgressListener) threadGroups.get(new Integer(tg.hashCode()));
	}

	// We MUST not be in synchronization block 
	// when we do dispatch
	//
	if (listener != null)
	{
	    listener.onStopBinding(identifier);
	}
    }
    
    /**
     * This would happen if we loaded JAR file from cache or file:/ URL
     */
    public static void onProgressComplete(Object identifier)
    {
	// Find the proper progress listener from the registered list 
	// accordingly to the caller thread group.
	//

	// 
	// It is VERY VERY important that the event dispatch 
	// doesn't occur until we get out of the synchronization
	// block. Otherwise, multiple dispatches from different
	// threads in the same thread group may result in
	// deadlock in case the callee is blocked
	//
	Thread t = Thread.currentThread();
	ThreadGroup tg = t.getThreadGroup();
	ProgressListener listener = null;

	synchronized(threadGroups)
	{
	    listener = (ProgressListener) threadGroups.get(new Integer(tg.hashCode()));
	}

	// We MUST not be in synchronization block 
	// when we do dispatch
	//
	if (listener != null)
	{
	    listener.onProgressComplete(identifier);
	}
    }

    /** 
     * Update progress. 
     *
     * @param identifier Object to be binded
     * @param current Current progress
     * @param max Maximum progress
     */
    public static void onProgressAvailable(Object identifier, int current, int max)
    {
	// Find the proper progress listener from the registered list 
	// accordingly to the caller thread group.
	//

	// 
	// It is VERY VERY important that the event dispatch 
	// doesn't occur until we get out of the synchronization
	// block. Otherwise, multiple dispatches from different
	// threads in the same thread group may result in
	// deadlock in case the callee is blocked
	//
	Thread t = Thread.currentThread();
	ThreadGroup tg = t.getThreadGroup();
	ProgressListener listener = null;

	synchronized(threadGroups)
	{
	    listener = (ProgressListener) threadGroups.get(new Integer(tg.hashCode()));
	}

	// We MUST not be in synchronization block 
	// when we do dispatch
	//
	if (listener != null)
	{
	    listener.onProgressAvailable(identifier, current, max);
	}
    }


    /**
     * Add progress listener.
     *
     * @param listener ProgressListener
     */
    public static void addProgressListener(ThreadGroup tg, ProgressListener l)
    {
	Trace.msgPrintln("progress.listener.added", new Object[] {l});

	// Soft reference is used to ensure that ThreadGroup and
	// ProgressListener will not be held even if the caller 
	// fails to remove the listener for some reasons.
	//
	synchronized(threadGroups)
	{
            // First check if we have this ThreadGroup in the HashMap table.
	    ProgressListener listener = (ProgressListener) threadGroups.get(new Integer(tg.hashCode()));

	    // Use event multicaster to broadcast events
	    listener = EventMulticaster.add(listener, l);

	    // Add back to HashMap
	    threadGroups.put(new Integer(tg.hashCode()), listener);
        }
    }

    /**
     * Remove progress listener.
     *
     * @param l ProgressListener
     */
    public static void removeProgressListener(ThreadGroup tg, ProgressListener l)
    {
	Trace.msgPrintln("progress.listener.removed", new Object[] {l});

	synchronized(threadGroups)
	{
            // First check if we have this ThreadGroup in the HashMap table.
	    ProgressListener listener = (ProgressListener) threadGroups.get(new Integer(tg.hashCode()));

	    // Use event multicaster to broadcast events
	    listener = EventMulticaster.remove(listener, l);

	    // Add back to HashMap
	    if (listener != null)
		threadGroups.put(new Integer(tg.hashCode()), listener);
	    else
		threadGroups.remove(new Integer(tg.hashCode()));
        }                 
    }
}



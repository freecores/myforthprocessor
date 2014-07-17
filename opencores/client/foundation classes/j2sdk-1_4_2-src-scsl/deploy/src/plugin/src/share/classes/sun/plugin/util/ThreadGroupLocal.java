/*
 * @(#)ThreadGroupLocal.java	1.4 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.util;

import java.util.HashMap;


/**
 * @author  mfisher
 * @author  stanleyh
 */
public class ThreadGroupLocal
{
   public static final String PROGRESS_TRACKING = "javaplugin.progressTracking";

   /*
     * HashMap contains information in a particular thread group.
     */
    private static HashMap threadGroup2Data = new HashMap();
    
    /** Creates new ThreadGroupLocal */
    public ThreadGroupLocal() 
    {
    }
    
    /*
     * Store data associated with the ThreadGroup.
     */
    public static void put(String name, Object value)
    {
	Thread t = Thread.currentThread();

	put(t.getThreadGroup(), name, value);
    }

    /*
     * Store data associated with the ThreadGroup.
     */
    public static void put(ThreadGroup tg, String name, Object value)
    {
	synchronized(threadGroup2Data)
	{
	    HashMap dataMap = (HashMap) threadGroup2Data.get(new Integer(tg.hashCode()));

	    if (dataMap == null)
		dataMap = new HashMap();

	    dataMap.put(name, value);
	    
	    threadGroup2Data.put(new Integer(tg.hashCode()), dataMap);
        }        
    }
    
    /*
     * Retrieve data associated with the ThreadGroup.
     */
    public static Object get(String name)
    {
	Thread t = Thread.currentThread();

	return get(t.getThreadGroup(), name);	
    }

    /*
     * Retrieve data associated with the ThreadGroup.
     */
    public static Object get(ThreadGroup tg, String name)
    {
	synchronized(threadGroup2Data)
	{
	    HashMap dataMap = (HashMap) threadGroup2Data.get(new Integer(tg.hashCode()));

	    if (dataMap == null)
		return null;

	    return dataMap.get(name);
        }        
    }
    
    public static void destroy(ThreadGroup tg){
        synchronized(threadGroup2Data){
            threadGroup2Data.remove(new Integer(tg.hashCode()));
        }
    }

}

/*
 * @(#)Trace.java	1.39 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.util;

import java.awt.Component;
import java.lang.ref.SoftReference;
import java.util.HashMap;
import java.util.Set;
import java.util.StringTokenizer;
import sun.plugin.resources.ResourceHandler;
import java.text.MessageFormat;


/**
 * Trace is a class that provides basic tracing capability in Java Plug-in.
 * It is enabled through two properties:
 *
 * 1) javaplugin.trace
 * 2) javaplugin.trace.option
 *
 * These two properties control how fine-grained the Java Plug-in tracing
 * should be.
 */
public class Trace 
{
    private static boolean bBasicTraceOn = false;
    private static boolean bExtTraceOn = false;
    private static boolean bSecurityTraceOn = false;
    private static boolean bNetTraceOn = false;	
    private static boolean bLiveConnectTraceOn = false;	
    private static boolean bAutomationOn = false;	

    static
    {
	// Determine if tracing is enabled;
	reset();

	// Determine if the tracing is enabled at all.
	//
	bAutomationOn = ((Boolean) java.security.AccessController.doPrivileged(
    			       new sun.security.action.GetBooleanAction("javaplugin.automation"))).booleanValue();
    }

    
    //HashMap for Thread groups.
    private static HashMap threadGroups = new HashMap();
    
    /** 
     * Initialize the Trace facilities
     */
    public static void reset()
    {
	// Determine if the tracing is enabled at all.
	//
	if (((Boolean) java.security.AccessController.doPrivileged(
    		new sun.security.action.GetBooleanAction("javaplugin.trace"))).booleanValue())
	{
	    // Determine how fine-grained tracing should be.
	    //
            String traceOptions = (String) java.security.AccessController.doPrivileged(
    				  new sun.security.action.GetPropertyAction("javaplugin.trace.option"));

	    if (traceOptions == null)
	    {
		bBasicTraceOn = true;
		bNetTraceOn = true;
		bSecurityTraceOn = true;
		bExtTraceOn = true;
		bLiveConnectTraceOn = true;
	    }
	    else
	    {
    		StringTokenizer st = new StringTokenizer(traceOptions, "|");
     
		while (st.hasMoreTokens()) 
		{
		    String option = (String) st.nextToken();

		    if (option == null || option.equalsIgnoreCase("all"))
		    {
			bBasicTraceOn = true;
			bNetTraceOn = true;
			bSecurityTraceOn = true;
			bExtTraceOn = true;
			bLiveConnectTraceOn = true;

			// If "all", then there is no need to check the rest
			break;
		    }
		    else if (option.equalsIgnoreCase("basic"))
		    {
			bBasicTraceOn = true;
		    }
		    else if (option.equalsIgnoreCase("net"))
		    {
			bNetTraceOn = true;
		    }
		    else if (option.equalsIgnoreCase("security"))
		    {
			bSecurityTraceOn = true;
		    }
		    else if (option.equalsIgnoreCase("ext"))
		    {
			bExtTraceOn = true;
		    }
		    else if (option.equalsIgnoreCase("liveconnect"))
		    {
			bLiveConnectTraceOn = true;
		    }
		}
	    }
	}
    }

    
    /**
     * Determine if tracing is enabled.
     *
     * @return true if tracing is enabled
     */
    public static boolean isEnabled()
    {
	return (bBasicTraceOn || bNetTraceOn || bSecurityTraceOn || bExtTraceOn || bLiveConnectTraceOn);
    }

    
    /**
     * Turn on/off basic tracing.
     *
     * @param true if basic tracing should be turned on.
     */
    public static void setBasicTrace(boolean trace)
    {
	bBasicTraceOn = trace;
    }

    
    /**
     * Turn on/off network tracing.
     *
     * @param true if network tracing should be turned on.
     */
    public static void setNetTrace(boolean trace)
    {
	bNetTraceOn = trace;
    }


    /**
     * Turn on/off security tracing.
     *
     * @param true if security tracing should be turned on.
     */
    public static void setSecurityTrace(boolean trace)
    {
	bSecurityTraceOn = trace;
    }


    /**
     * Turn on/off extension tracing.
     *
     * @param true if extension tracing should be turned on.
     */
    public static void setExtTrace(boolean trace)
    {
	bExtTraceOn = trace;
    }


    /**
     * Turn on/off LiveConnect tracing.
     *
     * @param true if liveconnect tracing should be turned on.
     */
    public static void setLiveConnectTrace(boolean trace)
    {
	bLiveConnectTraceOn = trace;
    }


    /**
     * Determine if automation is enabled.
     *
     * @return true if automation is enabled
     */
    public static boolean isAutomationEnabled()
    {
	return bAutomationOn;
    }


    /** 
     * Output message to System.out if tracing is enabled. 
     *
     * @param msg Message to be printed
     */
    public static void println(String msg)
    {
	println(msg, null);
    }


    /** 
     * Output message to System.out if tracing is enabled. 
     *
     * @param msg Message to be printed
     * @param filter Trace filter
     */
    public static void println(String msg, int filter)
    {
    	println(msg, null, filter);
    }


    /** 
     * Output message to System.out if tracing is enabled. Also,
     * output message to the trace listener that is passed in
     * the parameter.
     *
     * @param msg Message to be printed
     * @param listener Trace listener
     */
    public static void println(String msg, TraceListener listener)
    {
	println(msg, listener, TraceFilter.DEFAULT);
    }


    /** 
     * Output message to System.out if tracing is enabled. Also,
     * output message to the trace listener that is passed in
     * the parameter.
     *
     * @param msg Message to be printed
     * @param listener Trace listener
     * @param filter Trace filter
     */
    public static void println(String msg, TraceListener listener, int filter)
    {
	// Check if tracing is enabled at certain levels
	//
	if ((filter & TraceFilter.JAVA_CONSOLE_ONLY) == TraceFilter.JAVA_CONSOLE_ONLY)
	{
	    // If no level is specified, default to basic level
	    //
	    if ((filter & TraceFilter.LEVEL_MASK) == 0)
		filter |= TraceFilter.LEVEL_BASIC;    

	    switch (filter & TraceFilter.LEVEL_MASK)
	    {
		case TraceFilter.LEVEL_BASIC:
		{
		    if (!bBasicTraceOn)
			return;
		    else
			break;
		}
		case TraceFilter.LEVEL_NET:
		{
		    if (!bNetTraceOn)
			return;
		    else
			break;
		}
		case TraceFilter.LEVEL_SECURITY:
		{
		    if (!bSecurityTraceOn)
			return;
		    else
			break;
		}
		case TraceFilter.LEVEL_EXT:
		{
		    if (!bExtTraceOn)
			return;
		    else
			break;
		}
		case TraceFilter.LEVEL_LIVECONNECT:
		{
		    if (!bLiveConnectTraceOn)
			return;
		    else
			break;
		}
		default:
		{
		    break;
		}
	    }

	    // Output message
	    System.err.println(msg);
	    System.err.flush();

	    // Log message
	    PluginLogger.log(msg);
	}

	// Fire event to the trace listener
	firePrintlnEvent(msg, listener, filter);
    }

    /** 
     * Output resource to System.out if tracing is enabled. 
     *
     * @param resource Resource message to be printed
     */
    public static void msgPrintln(String resource)
    {
	msgPrintln(resource, null, TraceFilter.DEFAULT);
    }


    /** 
     * Output resource to System.out if tracing is enabled. 
     *
     * @param resource Resource message to be printed
     * @param filter Trace filter
     */
    public static void msgPrintln(String resource, int filter)
    {
    	msgPrintln(resource, null, filter);
    }


    /** 
     * Output resource to System.out if tracing is enabled. Also,
     * output resource to the trace listener that is passed in
     * the parameter.
     *
     * @param resource Resource message to be printed
     * @param listener Trace listener
     */
    public static void msgPrintln(String resource, TraceListener listener)
    {
	msgPrintln(resource, listener, TraceFilter.DEFAULT);
    }

    /** 
     * Output resource to System.out if tracing is enabled. 
     *
     * @param resource Resource message to be printed
     * @param params for the resource string
     */
    public static void msgPrintln(String resource, Object[] params)
    {
    	msgPrintln(resource, params, null, TraceFilter.DEFAULT);
    }

    /** 
     * Output resource to System.out if tracing is enabled. 
     *
     * @param resource Resource message to be printed
     * @param listener Trace listener
     * @param filter Trace filter
     */
    public static void msgPrintln(String resource, TraceListener listener, int filter)
    {
    	msgPrintln(resource, null, listener, filter);
    }

    /** 
     * Output resource to System.out if tracing is enabled. 
     *
     * @param resource Resource message to be printed
     * @param params for the resource string
     * @param listener Trace listener
     */
    public static void msgPrintln(String resource, Object[] params, TraceListener listener)
    {
    	msgPrintln(resource, params, listener, TraceFilter.DEFAULT);
    }

    /*
     * Output resource message to System.out if basic tracing is enabled.
     *
     * @param resource to be printed
     * @param params for the resource string
     * @param listener a possible trace listener, can be null.
     * @param fileter Trace filter
     *
     * Check if tracing is on or if message is to be printed in status bar
     * or gray box - then get message from ResourceHandler and call Println;
     * else - don't spend any time getting message.
     */
    public static void msgPrintln(String resource, Object[] params, TraceListener listener, int filter) 
    {
	// Check if tracing is enabled at certain levels
	//
	if ((filter & TraceFilter.JAVA_CONSOLE_ONLY) == TraceFilter.JAVA_CONSOLE_ONLY)
	{
	    // If no level is specified, default to basic level
	    //
	    if ((filter & TraceFilter.LEVEL_MASK) == 0)
		filter |= TraceFilter.LEVEL_BASIC;    

	    switch (filter & TraceFilter.LEVEL_MASK)
	    {
		case TraceFilter.LEVEL_BASIC:
		{
		    if (!bBasicTraceOn)
			return;
		    else
			break;
		}
		case TraceFilter.LEVEL_NET:
		{
		    if (!bNetTraceOn)
			return;
		    else
			break;
		}
		case TraceFilter.LEVEL_SECURITY:
		{
		    if (!bSecurityTraceOn)
			return;
		    else
			break;
		}
		case TraceFilter.LEVEL_EXT:
		{
		    if (!bExtTraceOn)
			return;
		    else
			break;
		}
		case TraceFilter.LEVEL_LIVECONNECT:
		{
		    if (!bLiveConnectTraceOn)
			return;
		    else
			break;
		}
		default:
		{
		    break;
		}
	    }
	}

	String msg = null;

	if (params == null)
	{
	    msg = ResourceHandler.getMessage(resource);
	}
	else
	{
	    MessageFormat formatter = new MessageFormat(ResourceHandler.getMessage(resource));            
	    msg = formatter.format(params);
	}

	// Display text if JAVA_CONSOLE_ONLY filter is enabled
	//
	if ((filter & TraceFilter.JAVA_CONSOLE_ONLY) == TraceFilter.JAVA_CONSOLE_ONLY)
	{
	    // Output message
	    System.err.println(msg);
	    System.err.flush();

	    // Log message
	    PluginLogger.log(msg);
	}

	// Fire event to the trace listener
	firePrintlnEvent(msg, listener, filter);
    }

    /** 
     * Output exception to System.out if tracing is enabled.
     *
     * @param e exception to be printed
     */
    public static void printException(Throwable e)
    {
	printException(null, e);
    }

    /** 
     * Output exception to System.out if tracing is enabled.
     *
     * @param parentComponent Parent Component
     * @param e exception to be printed
     */
    public static void printException(Component parentComponent, Throwable e)
    {
	printException(parentComponent, e, 
		       null,
		       null);
    }

    /** 
     * Output exception to System.out if tracing is enabled.
     *
     * @param e exception to be printed
     * @param desc Description about the exception
     * @param caption Caption for the exception dialog
     */
    public static void printException(Throwable e, String desc, String caption)
    {
	printException(null, e, desc, caption);
    }

    /** 
     * Output exception to System.out if tracing is enabled.
     *
     * @param parentComponent Parent Component
     * @param e exception to be printed
     * @param desc Description about the exception
     * @param caption Caption for the exception dialog
     */
    public static void printException(Component parentComponent, Throwable e, String desc, String caption)
    {
	printException(parentComponent, e, desc, caption, true);
    }

    /** 
     * Output exception to System.out if tracing is enabled.
     *
     * @param parentComponent Parent Component
     * @param e exception to be printed
     * @param desc Description about the exception
     * @param caption Caption for the exception dialog
     * @param show Show exception dialog
     */
    public static void printException(Component parentComponent, Throwable e, 
				      String desc, String caption, boolean show)
    {
	// Exception should be shown all the time
	e.printStackTrace();

	// Log exception using logger
	if (desc == null)
	    PluginLogger.logException(e);
	else
	    PluginLogger.logException(desc, e);

	// Show exception only if automation is disabled
	if (show && isAutomationEnabled() == false)
	{
	    if (desc == null)
		desc = ResourceHandler.getMessage("usability.general_error");

	    // Show Exception dialog
	    DialogFactory.showExceptionDialog(parentComponent, e, desc, caption);
	}
    }

    /** 
     * Output message to System.out if network tracing is enabled.
     *
     * @param msg Message to be printed
     */
    public static void netPrintln(String msg)
    {
	println(msg, null, TraceFilter.DEFAULT | TraceFilter.LEVEL_NET);
    }


    /** 
     * Output message to System.out if network tracing is enabled.
     *
     * @param msg Message to be printed
     * @param filter Trace filter
     */
    public static void netPrintln(String msg, int filter)
    {
	println(msg, null, filter | TraceFilter.LEVEL_NET);
    }

    /** 
     * Output resource to System.out if network tracing is enabled.
     *
     * @param resource Resource message to be printed
     */
    public static void msgNetPrintln(String resource)
    {
	msgPrintln(resource, null, null, TraceFilter.DEFAULT | TraceFilter.LEVEL_NET);
    }


    /** 
     * Output resource to System.out if network tracing is enabled.
     *
     * @param resource Resource message to be printed
     * @param filter Trace filter
     */
    public static void msgNetPrintln(String resource, int filter)
    {
	msgPrintln(resource, null, null, filter | TraceFilter.LEVEL_NET);
    }
    
    /** 
     * Output resource to System.out if network tracing is enabled.
     *
     * @param resource Resource message to be printed
     * @param params parameters for the message string
     */
    public static void msgNetPrintln(String resource, Object[] params)
    {
	msgPrintln(resource, params, null, TraceFilter.DEFAULT | TraceFilter.LEVEL_NET);
    }

    /** 
     * Output exception to System.out if net tracing is enabled.
     *
     * @param e exception to be printed
     */
    public static void netPrintException(Throwable e)
    {
	printException(null, e, ResourceHandler.getMessage("usability.net_error"), null, false);
    }


    /** 
     * Output exception to System.out if tracing is enabled.
     *
     * @param e exception to be printed
     * @param desc Description about the exception
     * @param caption Caption for the exception dialog
     */
    public static void netPrintException(Throwable e, String desc, String caption)
    {
	printException(null, e, desc, caption, false);
    }


    /** 
     * Output message to System.out if security tracing is enabled.
     *
     * @param msg Message to be printed
     */
    public static void securityPrintln(String msg)
    {
    	 println(msg, null, TraceFilter.DEFAULT | TraceFilter.LEVEL_SECURITY);
    }


    /** 
     * Output resource message to System.out if security tracing is enabled.
     *
     * @param resource Resource Message to be printed
     */
    public static void msgSecurityPrintln(String resource)
    {
    	 msgPrintln(resource, null, null, TraceFilter.DEFAULT | TraceFilter.LEVEL_SECURITY);
    }


    /** 
     * Output resource message to System.out if security tracing is enabled.
     *
     * @param resource Resource Message to be printed
     * @param params for the resource string
     */
    public static void msgSecurityPrintln(String resource, Object[] params)
    {
    	 msgPrintln(resource, params, null, TraceFilter.DEFAULT | TraceFilter.LEVEL_SECURITY);
    }

    /** 
     * Output exception to System.out if security tracing is enabled.
     *
     * @param e exception to be printed
     */
    public static void securityPrintException(Throwable e)
    {
	printException(null, e, ResourceHandler.getMessage("usability.security_error"), null, true);
    }


    /** 
     * Output exception to System.out if tracing is enabled.
     *
     * @param e exception to be printed
     * @param desc Description about the exception
     * @param caption Caption for the exception dialog
     */
    public static void securityPrintException(Throwable e, String desc, String caption)
    {
	printException(null, e, desc, caption, true);
    }


    /** 
     * Output message to System.out if extension tracing is enabled.
     *
     * @param msg Message to be printed
     */
    public static void extPrintln(String msg)
    {
	println(msg, null, TraceFilter.DEFAULT | TraceFilter.LEVEL_EXT);
    }


    /** 
     * Output message to System.out if extension tracing is enabled.
     *
     * @param msg Message to be printed
     * @param filter Trace filter
     */
    public static void extPrintln(String msg, int filter)
    {
	println(msg, null, filter | TraceFilter.LEVEL_EXT);
    }

   
    /** 
     * Output resource message to System.out if extension tracing is enabled.
     *
     * @param resource Resource message to be printed
     */
    public static void msgExtPrintln(String resource)
    {
	msgPrintln(resource, null, null, TraceFilter.DEFAULT | TraceFilter.LEVEL_EXT);
    }


    /** 
     * Output resource message to System.out if extension tracing is enabled.
     *
     * @param resource Resource message to be printed
     * @param filter Trace filter
     */
    public static void msgExtPrintln(String resource, int filter)
    {
	msgPrintln(resource, null, null, filter | TraceFilter.LEVEL_EXT);
    }

    /** 
     * Output resource to System.out if extension tracing is enabled.
     *
     * @param resource Resource message to be printed
     * @param params parameters for the message string
     */
    public static void msgExtPrintln(String resource, Object[] params)
    {
	msgPrintln(resource, params, null, TraceFilter.DEFAULT | TraceFilter.LEVEL_EXT);
    }

    /** 
     * Output exception to System.out if extension tracing is enabled.
     *
     * @param e exception to be printed
     */
    public static void extPrintException(Throwable e)
    {
	printException(null, e, ResourceHandler.getMessage("usability.ext_error"), null, true);
    }


    /** 
     * Output exception to System.out if tracing is enabled.
     *
     * @param e exception to be printed
     * @param desc Description about the exception
     * @param caption Caption for the exception dialog
     */
    public static void extPrintException(Throwable e, String desc, String caption)
    {
	printException(null, e, desc, caption, true);
    }


    /** 
     * Output message to System.out if LiveConnect tracing is enabled.
     *
     * @param msg Message to be printed
     */
    public static void liveConnectPrintln(String msg)
    {
	println(msg, null, TraceFilter.DEFAULT | TraceFilter.LEVEL_LIVECONNECT);
    }


    /** 
     * Output message to System.out if LiveConnect tracing is enabled.
     *
     * @param msg Message to be printed
     * @param filter Trace filter
     */
    public static void liveConnectPrintln(String msg, int filter)
    {
	println(msg, null, filter | TraceFilter.LEVEL_LIVECONNECT);
    }

    
    /** 
     * Output resource message to System.out if LiveConnect tracing is enabled.
     *
     * @param resource Resource message to be printed
     */
    public static void msgLiveConnectPrintln(String resource)
    {
	msgPrintln(resource, null, null, TraceFilter.DEFAULT | TraceFilter.LEVEL_LIVECONNECT);
    }


    /** 
     * Output resource message to System.out if LiveConnect tracing is enabled.
     *
     * @param resource Resource message to be printed
     * @param filter Trace filter
     */
    public static void msgLiveConnectPrintln(String resource, int filter)
    {
	msgPrintln(resource, null, null, filter | TraceFilter.LEVEL_LIVECONNECT);
    }

    /** 
     * Output resource to System.out if LiveConnect tracing is enabled.
     *
     * @param resource Resource message to be printed
     * @param params parameters for the message string
     */
    public static void msgLiveConnectPrintln(String resource, Object[] params)
    {
	msgPrintln(resource, params, null, TraceFilter.DEFAULT | TraceFilter.LEVEL_LIVECONNECT);
    }

    /** 
     * Output exception to System.out if LiveConnect tracing is enabled.
     *
     * @param e exception to be printed
     */
    public static void liveConnectPrintException(Throwable e)
    {
	// Notice that we should never popup dialog in LiveConnect because
	// it may be called from the main thread, and we may end up deadlock
	// ourselves.

	printException(null, e, null, null, false);
    }


    /**
     * Add trace listener.
     *
     * @param listener TraceListener
     */
    public static void addTraceListener(ThreadGroup tg, TraceListener l)
    {
	Trace.msgPrintln("trace.listener.added", new Object[] {l});

	// Soft reference is used to ensure that ThreadGroup and
	// TraceListener will not be held even if the caller 
	// fails to remove the listener for some reasons.
	//
	synchronized(threadGroups)
	{
            // First check if we have this ThreadGroup in the HashMap table.
	    TraceListener listener = (TraceListener) threadGroups.get(new Integer(tg.hashCode()));

	    // Use event multicaster
	    listener = EventMulticaster.add(listener, l);

	    threadGroups.put(new Integer(tg.hashCode()), listener);
        } 
    }

    /**
     * Remove trace listener.
     *
     * @param l TraceListener
     */
    public static void removeTraceListener(ThreadGroup tg, TraceListener l)
    {
	Trace.msgPrintln("trace.listener.removed", new Object[] {l});

	synchronized(threadGroups)
	{
            // First check if we have this ThreadGroup in the HashMap table.
	    TraceListener listener = (TraceListener) threadGroups.get(new Integer(tg.hashCode()));

	    // Use event multicaster
	    listener = EventMulticaster.remove(listener, l);

	    if (listener != null)
		threadGroups.put(new Integer(tg.hashCode()), listener);
	    else
		threadGroups.remove(new Integer(tg.hashCode()));
        }  
    }


    /**
     * Fire println event.
     *
     * @param msg Message
     * @param listener Trace Listener
     * @param filter Trace Filter
     */
    private static void firePrintlnEvent(String msg, TraceListener listener, int filter)
    {
	// If trace listener is defined, fire println event to the dedicated
	// trace listener. Otherwise, find the proper trace listener from
	// the registered list accordingly to the caller thread group.
	//
	if (listener != null && listener.isSupported(filter))
	{
	    listener.println(msg, filter);
	}
	else
	{
	    if (threadGroups.size() > 0)
	    {
		// 
		// It is VERY VERY important that the event dispatch 
		// doesn't occur until we get out of the synchronization
		// block. Otherwise, multiple dispatches from different
		// threads in the same thread group may result in
		// deadlock.
		//
		Thread t = Thread.currentThread();
		ThreadGroup tg = t.getThreadGroup();
		TraceListener l = null;

		synchronized(threadGroups)
		{
		    l = (TraceListener) threadGroups.get(new Integer(tg.hashCode()));
		}

		// Dispatch only if there is any valid listener
		if (l != null)
		{
		    l.println(msg, filter);
		}
	    }
	}
    }
}



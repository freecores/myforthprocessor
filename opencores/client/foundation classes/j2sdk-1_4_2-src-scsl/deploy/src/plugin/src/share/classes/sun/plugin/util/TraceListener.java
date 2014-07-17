/*
 * @(#)TraceListener.java	1.9 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.util;

import java.util.EventListener;


/**
 * TraceListener is an interface that acts as a basic trace listener.
 */
public interface TraceListener extends java.util.EventListener
{
    /**
     * Check if TraceListener supports a particular filter.
     */
    public boolean isSupported(int filter);

    /** 
     * Output message to listener.
     *
     * @param msg Message to be printed
     * @param filter Trace filter
     */
    public void println(String msg, int filter);
}


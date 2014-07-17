/*
 * @(#)AppletStatusListener.java	1.4 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin;

/**
 * AppletStatusListener is an interface for listening applet
 * status changes.
 */
public interface AppletStatusListener
{    
    /**
     * Notify applet status change
     */
    public void statusChanged(int status);
}   


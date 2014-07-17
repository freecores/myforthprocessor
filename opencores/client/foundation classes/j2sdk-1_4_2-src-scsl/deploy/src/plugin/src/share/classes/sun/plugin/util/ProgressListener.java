/*
 * @(#)ProgressListener.java	1.9 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.util;

import java.util.EventListener;


/**
 * ProgressListener is an interface that acts as a basic progress listener.
 */
public interface ProgressListener extends EventListener
{
    /**
     * Start progress binding.
     *
     * @param identifier Progress identifier
     */
    public void onStartBinding(Object identifier);

    /**
     * Update progress.
     *
     * @param identifier Progress identifier
     * @param current Current progress.
     * @param max Maximum progress.
     */
    public void onProgressAvailable(Object identifier, int current, int max);
    
    /**
     * Stop progress binding.
     *
     * @param identifier Progress identifier
     */
    public void onStopBinding(Object identifier);
    
    /**
     * Send a notification to repaint progress.
     */
    public void onProgressComplete(Object identifier);
}


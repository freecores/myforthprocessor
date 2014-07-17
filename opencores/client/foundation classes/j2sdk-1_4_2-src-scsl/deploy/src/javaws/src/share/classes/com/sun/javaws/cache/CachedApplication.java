/*
 * @(#)CachedApplication.java	1.6 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws.cache;
import com.sun.javaws.LocalApplicationProperties;
import com.sun.javaws.jnl.*;
import java.net.URL;

/**
 * Used to return an application in the cache.
 *
 * @version 1.6 01/23/03
 */
public interface CachedApplication {
    /**
     * This can return null, indicating there was an error in loading the
     * JNL file, or the cache changed between the time the
     * <code>CachedApplication</code> was created and the time this
     * method is invoked.
     */
    public LaunchDesc getLaunchDescriptor();
    /**
     * Returns the codebase for the LaunchDescriptor
     */
    public URL getCodebase();

    public LocalApplicationProperties getLocalApplicationProperties();
}

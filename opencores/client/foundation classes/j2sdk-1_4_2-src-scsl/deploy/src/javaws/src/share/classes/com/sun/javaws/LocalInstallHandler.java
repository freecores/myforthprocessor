/*
 * @(#)LocalInstallHandler.java	1.10 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws;

import java.awt.Window;
import com.sun.javaws.debug.*;
import com.sun.javaws.jnl.*;
import java.security.*;
import java.util.*;

/**
 * Instances of LocalInstallHandler are used to handle installing/uninstalling
 * of Applications. Handling installing/uninstalling is platform specific,
 * and therefore not all platforms may support any sort of additional
 * install options. The instance to use for installing can be located
 * via the class method <code>getInstance</code>. A
 * null return value from <code>getInstance</code> indicates the
 * current platform does not support any install options.
 * <p>
 * Instances of LocalInstallHandler are thread safe, that is
 * <code>install</code> or <code>uninstall</code> can be invoked from any
 * thread. Although it is possible for <code>uninstall</code> to be
 * invoked while <code>install</code> is still running, implementations should
 * not worry about this happening.
 *
 * @version 1.10 01/23/03
 */
public abstract class LocalInstallHandler {
    /** The shared instance of LocalInstallHandler. */
    private static LocalInstallHandler _installHandler;

    /**
     * Returns the LocalInstallHandler appropriate for the current platform.
     * This may return null, indicating the platform does not support
     * an installer.
     */
    public static synchronized LocalInstallHandler getInstance() {
        if (_installHandler == null) {
            _installHandler = LocalInstallHandlerFactory.newInstance();
        }
        return _installHandler;
    }

    /**
     * Installs the Application identified by <code>lp</code> in whatever
     * manner is appropriate for the given platform.
     */
    public abstract void install(
	Window owner, LaunchDesc descriptor, LocalApplicationProperties lap);

    /**
     * Invoked when an application is first launched. This should determine
     * if the user should be prompted for an install, and if necessary do
     * the install.
     */
    public abstract void installIfNecessaryFromLaunch(
	Window owner, LaunchDesc descriptor, LocalApplicationProperties lap);

    /**
     * Uninstalls a previously installed Application
     */
    public abstract void uninstall(LaunchDesc ld,
                                   LocalApplicationProperties lap);
   /**
     * determines if this platform supports Local Install
     */
    public abstract boolean isLocalInstallSupported();
     
}

/*
 * @(#)UnixInstallHandler.java	1.2 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws;

import com.sun.javaws.debug.Globals;
import com.sun.javaws.debug.Debug;
import com.sun.javaws.cache.*;
import com.sun.javaws.jnl.*;
import com.sun.javaws.ui.general.GeneralUtilities;
import com.sun.javaws.exceptions.JNLPException;
import java.awt.*;
import java.awt.event.*;
import java.awt.image.*;
import java.io.*;
import java.net.*;
import javax.swing.*;

/**
 * Instances of InstallHandler are used to handle installing/uninstalling
 * of Applications. Handling installing/uninstalling is platform specific,
 * and therefore not all platforms may support any sort of additional
 * install options. The instance to use for installing can be located
 * via the class method <code>getInstallHandler</code>. This method
 * will look up the class from the property
 * <code>javaws.installer.className</code>, and create it if non-null. A
 * null return value from <code>getInstallHandler</code> indicates the
 * current platform does not support any install options.
 *
 * @version 1.8 03/02/00
 */
public class UnixInstallHandler extends LocalInstallHandler {
    
    /**
     * Invoked when an application is first launched. This should determine
     * if the user should be prompted for an install, and if necessary do
     * the install.
     */
    public void installIfNecessaryFromLaunch(final Window owner, 
		final LaunchDesc desc, final LocalApplicationProperties lap) {
                 
        int whenInstall = ConfigProperties.getInstance().getWhenInstall();
	switch (whenInstall) {
	    case ConfigProperties.INSTALL_NEVER:
		return;
	    case ConfigProperties.INSTALL_ALWAYS:
		install(owner, desc, lap);
		return;
	    case ConfigProperties.INSTALL_ASK:
	    default:
		if (lap.getAskedForInstall()) {
		    return; // already asked
		}
		break;		// OK - go ahead and ask
	}
        new Thread(new Runnable() {
            public void run() {
	        int ret = GeneralUtilities.showLocalInstallDialog(
				    owner, desc.getInformation().getTitle());
	        if (Globals.TraceLocalAppInstall) {
	            Debug.println("showLocalInstallDialog returned: "+ret);
	        }
	        switch (ret) {
	            case GeneralUtilities.INSTALL_YES:
		        install(owner, desc, lap);
		        lap.setAskedForInstall(true);
		        break;
	            case GeneralUtilities.INSTALL_NO:		
		        lap.setAskedForInstall(true);
		        break;
	            case GeneralUtilities.INSTALL_ASK:
		        lap.setAskedForInstall(false);
		        break;
	        }	
            }
	}).start();
    }

    /**
     * Handles the install of the passed in application.
     */
    public void install(
	Window owner, LaunchDesc desc, LocalApplicationProperties lap) {

	if (Globals.TraceLocalAppInstall) {
	    Debug.println("install called in UnixInstallHandler");
	}
	lap.setLocallyInstalled(true);
    }


    /**
     * Uninstalls a previously installed application 
     */
    public void uninstall(
	LaunchDesc desc, LocalApplicationProperties lap) {

	if (Globals.TraceLocalAppInstall) {
	    Debug.println("uninstall called in UnixInstallHandler");
	}
	lap.setLocallyInstalled(false);
    }

    /**
     *  Determine if this platform supports Local Application Installation.
     *  We will want to return true for the window managers that we support
     *  in install and uninstall above.
     */
    public boolean isLocalInstallSupported() {
 
        if (Globals.TraceLocalAppInstall) {
            Debug.println("isLocalInstallSupported called in UnixInstallHandler");
	    return true;
        }
	return false;
    } 
}
 

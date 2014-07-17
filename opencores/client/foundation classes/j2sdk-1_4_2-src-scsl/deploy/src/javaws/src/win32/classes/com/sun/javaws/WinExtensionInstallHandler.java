/*
 * @(#)WinExtensionInstallHandler.java	1.4 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws;

import com.sun.javaws.debug.*;
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
 * Instances of ExtensionInstallHandler are used to handle platform-specific
 * installing/uninstalling actions Extension Installers.  The instance to use
 * for installing can be located via the class method <code>getInstance</code>. A
 * null return value from <code>getInstance</code> indicates the
 * current platform does not support any install options.
 * <p>
 * @version 1.0 04/07/02
 */
public class WinExtensionInstallHandler extends ExtensionInstallHandler {
    private static final String KEY_RUNONCE =
	"Software\\Microsoft\\Windows\\CurrentVersion\\RunOnce";
    
    static {
	NativeLibrary.getInstance().load();
    }
    
    public boolean doPreRebootActions(Window owner) {
	// Ask user if they want to reboot
	int result[] = { JOptionPane.NO_OPTION };
	owner.setVisible(true);
	owner.requestFocus();
	result[0] = GeneralUtilities.showOptionDialog (owner,
	    Resources.getString("extensionInstall.rebootMessage"),
	    Resources.getString("extensionInstall.rebootTitle"),
	    JOptionPane.YES_NO_OPTION,
	    JOptionPane.QUESTION_MESSAGE);
	owner.setVisible(false);
					  
	if (result[0] == JOptionPane.YES_OPTION) {
	    return true;
	}
	return false;
    }
    
    public boolean doReboot() {
	return WinRegistry.doReboot();
    }
}

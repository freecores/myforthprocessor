/*
 * @(#)ProxyDialog.java	1.15 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.proxy;

import javax.swing.*;
import java.net.*;
import java.awt.*;
import java.awt.event.*;
import com.sun.javaws.*;
import com.sun.javaws.ui.prefs.*;
import com.sun.javaws.ui.general.*;

/**
 * In the case of AUTO detect settings, there could be cases
 * we are unable to detected the actual setting used by the
 * browser, this is so that we can a. prompt the user to
 * enter a new proxy or if we find a legacy setting in the
 * browser preferences we ask the user to validate it.
 * @version 1.1, 08/25/00
 */

public class ProxyDialog {
    GeneralPanel _prefsGeneralPanel;
        
    public ProxyDialog() {

        _prefsGeneralPanel = new GeneralPanel();

	Object message = new Object [] {
	    Resources.getString("proxydialog.message"),
	    _prefsGeneralPanel.getProxyPanel()
	};
        
	GeneralUtilities.
	    showMessageDialog(null,
			      message,
			      Resources.getString("proxydialog.title"),
			      JOptionPane.QUESTION_MESSAGE);
	
	_prefsGeneralPanel.apply();
	
	return;
    }
}



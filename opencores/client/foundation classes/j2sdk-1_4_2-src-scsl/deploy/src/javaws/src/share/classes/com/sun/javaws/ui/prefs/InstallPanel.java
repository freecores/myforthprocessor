/*
 * @(#)InstallPanel.java	1.2 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws.ui.prefs;

import javax.swing.*;
import javax.swing.border.*;
import javax.swing.event.*;
import java.awt.*;
import java.awt.event.*;
import java.io.*;
import java.util.*;
import com.sun.javaws.*;
import com.sun.javaws.debug.Globals;
import com.sun.javaws.ui.general.*;

/**
 * InstallPanel
 */
public class InstallPanel extends PrefsPanel.PropertyPanel {
    private WhenInstallPanel _whenPanel;

    protected void createPropertyControls() {

        setLayout(new BorderLayout());

	_whenPanel = new WhenInstallPanel();
	Border titleBorder = new TitledBorder(new EtchedBorder(), Resources.
               getString("controlpanel.install.installBorderTitle"),
                         TitledBorder.LEFT, TitledBorder.TOP);

	_whenPanel.setBorder(BorderFactory.createCompoundBorder(
		BorderFactory.createEmptyBorder(32,0,32,0), titleBorder));

	add(_whenPanel, "Center");
        revert();
    }

    //
    // Controller methods
    //
    public void apply() {
        ConfigProperties cp = getConfigProperties();
        cp.put(ConfigProperties.WHEN_INSTALL_KEY, 
	       Integer.toString(_whenPanel.getWhen()));
    }


    public void revert() {
        // Then when to install
        ConfigProperties cp = getConfigProperties();
        _whenPanel.setWhen(cp.getWhenInstall());
    }

}

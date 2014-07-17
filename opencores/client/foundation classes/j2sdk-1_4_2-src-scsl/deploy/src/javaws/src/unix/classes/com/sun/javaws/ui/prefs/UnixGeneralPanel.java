/*
 * @(#)UnixGeneralPanel.java	1.4 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws.ui.prefs;

import com.sun.javaws.*;
import com.sun.javaws.proxy.*;
import java.awt.*;
import java.awt.event.*;
import java.util.*;
import javax.swing.*;
import javax.swing.border.*;

/**
 * @version 1.12 02/14/01
 */
public class UnixGeneralPanel extends GeneralPanel {
    private JCheckBox _netscapeBrowserCB;
    private JTextField _browserCommandTF;

    protected void createPropertyControls() {
        setLayout(new GridBagLayout());

        addGridBagComponent(this, Box.createVerticalGlue(),
                            0, 0, 1, 1, 1, .33, GridBagConstraints.CENTER,
                            GridBagConstraints.BOTH, null, 0, 0);

        addGridBagComponent(this, createProxyPanel(),
                            0, 1, 1, 1, 1, 0, GridBagConstraints.CENTER,
                            GridBagConstraints.HORIZONTAL, null, 0, 0);

        addGridBagComponent(this, Box.createVerticalGlue(),
                            0, 2, 1, 1, 1, .33, GridBagConstraints.CENTER,
                            GridBagConstraints.BOTH, null, 0, 0);

        addGridBagComponent(this, createBrowserPanel(),
                            0, 3, 1, 1, 1, 0, GridBagConstraints.CENTER,
                            GridBagConstraints.HORIZONTAL, null, 0, 0);

        addGridBagComponent(this, Box.createVerticalGlue(),
                            0, 4, 1, 1, 1, .33, GridBagConstraints.CENTER,
                            GridBagConstraints.BOTH, null, 0, 0);

        revert();
    }

    private Component createBrowserPanel() {
        JPanel subPanel = new JPanel(new GridBagLayout());
        Insets insets = new Insets(0, 2, 5, 8);
        JLabel pathLabel = new JLabel(
	    Resources.getString("controlpanel.general.browserCommand"));
	pathLabel.setDisplayedMnemonic(
            Resources.getVKCode("controlpanel.general.browserCommandMnemonic")
				      );

        addGridBagComponent(subPanel, pathLabel,
                            0, 0, 1, 1, 0, 0, GridBagConstraints.CENTER,
                            GridBagConstraints.NONE, insets, 0, 0);

        insets.right = 2;
        _browserCommandTF = new JTextField();
	pathLabel.setLabelFor(_browserCommandTF);

        addGridBagComponent(subPanel, _browserCommandTF, 1, 0, 1, 1, 1, 0,
                            GridBagConstraints.CENTER,
                            GridBagConstraints.HORIZONTAL, insets, 0, 0);

        _netscapeBrowserCB = new JCheckBox(
	    Resources.getString("controlpanel.general.isNetscapeBrowser"));
        _netscapeBrowserCB.setRequestFocusEnabled(false);
	_netscapeBrowserCB.setMnemonic(
	    Resources.getVKCode("controlpanel.general.isNetscapeMnemonic"));
        addGridBagComponent(subPanel, _netscapeBrowserCB, 0, 2, 2, 1, 0, 0,
                            GridBagConstraints.CENTER,
                            GridBagConstraints.NONE, insets, 0, 0);

        subPanel.setBorder(new TitledBorder
            (LineBorder.createBlackLineBorder(),
             Resources.getString("controlpanel.general.browserTitle"),
             TitledBorder.LEFT, TitledBorder.TOP));
        return subPanel;
    }

    //
    // Controller methods
    //
    public void apply() {
        if (_netscapeBrowserCB != null) {
            ConfigProperties cp = ConfigProperties.getInstance();
            boolean isNetscapeBrowser = _netscapeBrowserCB.isSelected();
            cp.put(ConfigProperties.NETSCAPE_BROWSER_KEY,
                   String.valueOf(isNetscapeBrowser));
            
            // browser command
            cp.put(ConfigProperties.BROWSER_PATH_KEY,
                   _browserCommandTF.getText());
        }
        super.apply();
    }

    public void revert() {
        ConfigProperties cp = getConfigProperties();
        String isNetscapeBrowser = (String)cp.get
                             (ConfigProperties.NETSCAPE_BROWSER_KEY);
        if (_netscapeBrowserCB != null && _browserCommandTF != null) {
            _netscapeBrowserCB.setSelected(isNetscapeBrowser != null ?
                                               Boolean.valueOf(isNetscapeBrowser).
                                               booleanValue() : true);
            
            _browserCommandTF.setText((String)cp.get
                                          (ConfigProperties.BROWSER_PATH_KEY));
        }

        super.revert();
    }
}


/*
 * @(#)WhenInstallPanel.java	1.2 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws.ui.general;

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
 * InstallWhenPanel
 */
public class WhenInstallPanel extends JPanel {
    private static final Object ID_KEY = new Object();

    private ButtonGroup _whenInstallGroup;
    private JRadioButton _radios[];

    public WhenInstallPanel() {

	setLayout(new BorderLayout());

	JTextArea ta = new JTextArea(Resources.getString(
		"prefs.install.message")) {
	    public boolean isFocusTraversable() {
		return false;
	    }
	};
        ta.setLineWrap(true);
        ta.setWrapStyleWord(true);
        ta.setOpaque(false);
        ta.setEditable(false);
	ta.setRows(4);
	ta.setBorder(BorderFactory.createEmptyBorder(16,40,12,40));
        add(ta, "North");


        // Panel for choosing when to ask for install
        ConfigProperties cp = ConfigProperties.getInstance();
        _radios = new JRadioButton[3];
        _radios[0] = new JRadioButton(Resources.getString(
			"prefs.install.always"), false);
	_radios[0].setMnemonic(Resources.getVKCode(
			"prefs.install.alwaysMnemonic"));
	_radios[0].putClientProperty(ID_KEY, new Integer(cp.INSTALL_ALWAYS));

        _radios[1] = new JRadioButton(Resources.getString(
                        "prefs.install.never"), false);
	_radios[1].setMnemonic(Resources.getVKCode(
			"prefs.install.neverMnemonic"));
	_radios[1].putClientProperty(ID_KEY, new Integer(cp.INSTALL_NEVER));

        _radios[2] = new JRadioButton(Resources.getString(
                        "prefs.install.ask"), true);
	_radios[2].setMnemonic(Resources.getVKCode(
			"prefs.install.askMnemonic"));
	_radios[2].putClientProperty(ID_KEY, new Integer(cp.INSTALL_ASK));

	JPanel radioPanel = new JPanel(new GridLayout(0,1));
        radioPanel.add(_radios[0]);
        radioPanel.add(_radios[1]);
        radioPanel.add(_radios[2]);
	radioPanel.setBorder(BorderFactory.createEmptyBorder(0,16,16,16));
        _whenInstallGroup = new ButtonGroup();
        for (int i=0; i<3; _whenInstallGroup.add(_radios[i++]));


        add(radioPanel, "South");
    }


    /**
     * Updates the selection in the ButtonGroup . 
     * This iterates over the elements in the ButtonGroup
     * selecting the one that has the client property <code>ID_KEY</code>
     * equal to <code>value</code>.
     */
    public void setWhen(int value) {
        Enumeration elements = _whenInstallGroup.getElements();
        while (elements.hasMoreElements()) {
            JRadioButton rb = (JRadioButton)elements.nextElement();
            if (((Integer)rb.getClientProperty(ID_KEY)).intValue() == value) {
                rb.setSelected(true);
                return;
            }
        }
    }

    /**
     * Returns the value of the selected JRadioButton in the ButtonGroup.
     * The value is determined from the client property
     * <code>ID_KEY</code>. If none of the elements 
     * are selected this will return -1.
     */
    public int getWhen() {
        Enumeration elements = _whenInstallGroup.getElements();
        while (elements.hasMoreElements()) {
            JRadioButton rb = (JRadioButton)elements.nextElement();
            if (rb.isSelected()) {
                return ((Integer)rb.getClientProperty(ID_KEY)).intValue();
            }
        }
        return -1;
    }
}

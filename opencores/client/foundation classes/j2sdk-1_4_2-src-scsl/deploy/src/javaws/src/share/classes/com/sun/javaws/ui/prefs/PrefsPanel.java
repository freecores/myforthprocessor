/*
 * @(#)PrefsPanel.java	1.6 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws.ui.prefs;

import javax.swing.*;
import javax.swing.border.*;
import javax.swing.event.*;
import javax.swing.table.*;
import java.awt.*;
import java.awt.event.*;
import java.io.*;
import java.lang.reflect.*;
import java.util.*;
import com.sun.javaws.*;
import com.sun.javaws.debug.*;
import com.sun.javaws.ui.general.*;
import com.sun.javaws.ui.player.Player;
import com.sun.javaws.cache.InstallCache;

public class PrefsPanel extends JDialog implements ChangeListener {
    private JTabbedPane tabPane;
    private ConfigProperties configProperties;
    private boolean exitOnClose;
    private AbstractController controller;
    private JButton _cancelButton, _resetButton, _okButton;


    public PrefsPanel(Frame owner, boolean exitOnClose) {
        super(owner, Resources.getString("controlpanel.title"), false);
	this.exitOnClose = exitOnClose;
	configProperties = ConfigProperties.getInstance();
	// tab pane
	tabPane = new JTabbedPane();
	getContentPane().setLayout(new BorderLayout());
	getContentPane().add(tabPane, BorderLayout.CENTER);
	controller = new AbstractController() {
	    protected Subcontroller createSubcontroller(int index) {
                return PrefsPanel.this.createSubcontroller(index);
	    }
	};
        addTabs();

	setSelectedTab(0);
	tabPane.addChangeListener(this);
	
	// widgets to accept changes
	JPanel buttons = new JPanel();
	
	ActionListener cancelListener = new ActionListener() {
	    public void actionPerformed(ActionEvent ae) {
	        closeControlPanel();
	    }
	};
	
	_cancelButton = new JButton(Resources.getString("controlpanel.cancel"));
	int cancel = Resources.getVKCode("controlpanel.cancelMnemonic");
	KeyStroke stroke = KeyStroke.getKeyStroke((char)cancel);
	_cancelButton.addActionListener(cancelListener);
	getRootPane().registerKeyboardAction(cancelListener, stroke, 
		JComponent.WHEN_IN_FOCUSED_WINDOW);

	_resetButton = new JButton(Resources.getString("controlpanel.reset"));
	_resetButton.setMnemonic(
		Resources.getVKCode("controlpanel.resetMnemonic"));
	_resetButton.addActionListener(new ActionListener() {
		public void actionPerformed(ActionEvent ae) {
		    revert();
		}
	    });

	_okButton = new JButton(Resources.getString("controlpanel.ok"));
	_okButton.setMnemonic(Resources.getVKCode("controlpanel.okMnemonic"));
	this.getRootPane().setDefaultButton(_okButton);
	_okButton.addActionListener(new ActionListener() {
		public void actionPerformed(ActionEvent ae) {
		    ok();
		}
	    });

	Dimension max = _cancelButton.getPreferredSize();
	Dimension d = _resetButton.getPreferredSize(); 
	if (d.width > max.width) max.width = d.width;
	if (d.height > max.height) max.height = d.height;
	d = _okButton.getPreferredSize(); 
	if (d.width > max.width) max.width = d.width;
	if (d.height > max.height) max.height = d.height;
	_cancelButton.setPreferredSize(max);
	_resetButton.setPreferredSize(max);
	_okButton.setPreferredSize(max);

	buttons.add(_okButton);
	buttons.add(_resetButton);
	buttons.add(_cancelButton);
	
	JPanel tButtons = new JPanel(new BorderLayout());
	tButtons.add(buttons, BorderLayout.EAST);
	
	getContentPane().add(tButtons, BorderLayout.SOUTH);
	
        addWindowListener(new WindowAdapter() {
                public void windowClosing(WindowEvent we) {
                    closeControlPanel();
                }
            });
	pack();
	setSize(420,400);
	GeneralUtilities.placeWindow(this);
    }
    
    //
    // ChangeListener on the JTabbedPane
    //
    public void stateChanged(ChangeEvent e) {
        int index = tabPane.getSelectedIndex();
        if (index >= 0) {
            // Can get -1 if more than one PrefsPanel created.
            setSelectedTab(index);
        }
    }
    
    private void closeControlPanel() {
	setVisible(false);
        tabPane.removeChangeListener(this);
	if (exitOnClose == true) {
	    System.exit(0);
	}
        dispose();
    }
    private void ok() {
	apply();
	closeControlPanel();
    }
    
    /**
    * Applys any pending changes.
    */
    private void apply() {
	controller.apply(true);
	if (InstallCache.refreshCacheLocation()) {
	    Player.getPlayer().resetCacheEntryManager();
	}
        ConfigProperties cp = ConfigProperties.getInstance();
        boolean didStore = false;
        synchronized(cp) {
            if (cp.doesNewVersionExist()) {
                cp.refresh();
            }
            else {
                cp.store();
                didStore = true;
            }
        }
        if (!didStore) {
            // Tell user changes failed.
            GeneralUtilities.showMessageDialog
                  (this, Resources.getString("controlpanel.storeFailed"),
                   Resources.getString("controlpanel.storeFailedTitle"),
                   JOptionPane.ERROR_MESSAGE);
        }
    }
    
    /**
     * Reverts the current changes.
     */
    private void revert() {
	controller.revert(true);
    }

    /**
     * Creates the tabs for the JTabbedPane.
     */
    private void addTabs() {
        String pID = Globals.getOperatingSystemID();
        SubcontrollerCreator sc = SubcontrollerCreator.getInstance();
        int tabCount = sc.getSubcontrollerCount();
        for (int counter = 0; counter < tabCount; counter++) {
            tabPane.addTab(sc.getSubcontrollerTitle(counter),
                           new JPanel());
            Subcontroller sub = sc.getSubcontroller(false, counter);
            if (sub != null) {
                sub.revert();
            }
        }
    }

    /**
     * Updates the selected tab.
     */
    private void setSelectedTab(int tab) {
	if (tab != controller.getActiveSubcontrollerIndex()) {
	    Subcontroller subcontroller = controller.getSubcontroller(tab);
	    controller.setActiveSubcontrollerIndex(tab);
	    if (subcontroller == null) {
		// First time this Subcontroller encountered.
		//
		// This is done instead of setComponentAt, as there is a bug
		// in setComponentAt in kestrel-beta which causes an NPE! YECK!
		// We have to change the selection back to 0, otherwise
		// we will cause the previous tab to be selected, and thus
		// loaded, which we don't want.
		tabPane.setSelectedIndex(0);
		controller.setActiveSubcontrollerIndex(tab);
		String title = tabPane.getTitleAt(tab);
		tabPane.removeTabAt(tab);
		tabPane.insertTab(title, null,
				  controller.getActiveSubcontroller().
				      getComponent(), null, tab);
		tabPane.setSelectedIndex(tab);
	    }
	}
    }

    private Subcontroller createSubcontroller(int index) {
        return SubcontrollerCreator.getInstance().getSubcontroller
                                    (true, index);
    }
    
    
    public static class PropertyPanel extends JPanel implements Subcontroller {
	private boolean started;

	public PropertyPanel() {
	    super();
	}

	public void start() {
	    if (!started) {
		started = true;
		createPropertyControls();
	    }
	}
	
	public void stop() {
	}
	
	public Component getComponent() {
	    return PropertyPanel.this;
	}
	
	public void apply() {
	}
	
	public void revert() {
	}
	
	protected void createPropertyControls() {
	}

        protected ConfigProperties getConfigProperties() {
            return ConfigProperties.getInstance();
        }

        protected void addGridBagComponent(Container parent,
                                           Component c, int gridx, int gridy,
                                           int width, int height,
                                           double weightx, double weighty,
                                           int anchor, int fill,
                                           Insets insets, int ipadx,
                                           int ipady) {
            GridBagHelper.addGridBagComponent(parent, c, gridx, gridy,
                                              width, height, weightx,
                                              weighty, anchor, fill,
                                              insets, ipadx, ipady);
        }
    }
    
    
    static public void main(String args[]) {
	PrefsPanel cp = new PrefsPanel(null, true);
	cp.show();
    }
}




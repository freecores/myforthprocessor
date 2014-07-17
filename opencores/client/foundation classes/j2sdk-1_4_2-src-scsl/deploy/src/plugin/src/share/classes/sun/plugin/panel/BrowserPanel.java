/*
 * @(#)BrowserPanel.java	1.10 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.panel;

/**
 * Panel to display browser default VM options
 *
 * @version 	1.0 
 * @author	Stanley Man-Kit Ho
 */

import javax.swing.*;
import javax.swing.border.*;
import java.awt.*;
import java.awt.event.*;
import java.awt.Dimension;
import java.io.File;
import sun.plugin.util.*;

public class BrowserPanel extends ActivatorSubPanel 
			implements  ActionListener{

    /**
     * Contruct a new proxy panel
     */
    public BrowserPanel(ConfigurationInfo model) {
	super(model);
	setLayout(new BoxLayout(this, BoxLayout.Y_AXIS));

	Border border = BorderFactory.createEtchedBorder();

	JPanel browserPanel = new JPanel();
	browserPanel.setLayout(new BoxLayout(browserPanel, BoxLayout.Y_AXIS));
	browserPanel.setBorder(BorderFactory.createCompoundBorder(BorderFactory.createEmptyBorder(4, 4, 4, 4),
			       BorderFactory.createTitledBorder(border, mh.getMessage("settings"))));

	JPanel selectionPanel = new JPanel();
	selectionPanel.setLayout(new BoxLayout(selectionPanel, BoxLayout.Y_AXIS));

	// Add vertical pad
	selectionPanel.setBorder(BorderFactory.createEmptyBorder(4,30,4,30));	
	selectionPanel.add(Box.createRigidArea(new Dimension(1,5)));
	selectionPanel.add(new JLabel(mh.getMessage("desc.text")));
	selectionPanel.add(Box.createRigidArea(new Dimension(1,15)));
	selectionPanel.add(iexplorerCheckBox);
	selectionPanel.add(Box.createRigidArea(new Dimension(1,5)));
	selectionPanel.add(netscape6CheckBox);
	selectionPanel.add(Box.createRigidArea(new Dimension(1,5)));
        selectionPanel.add(mozillaCheckBox);
	selectionPanel.add(Box.createRigidArea(new Dimension(1,5)));
        
	iexplorerCheckBox.addActionListener(this);
	netscape6CheckBox.addActionListener(this);
        mozillaCheckBox.addActionListener(this);
	selectionPanel.add(Box.createRigidArea(new Dimension(1,40)));
	selectionPanel.add(Box.createGlue());
	browserPanel.add(selectionPanel);

	add(browserPanel);

	reset();
    }

    /**
     * Reset the checkbox
     */
    public void reset() 
    {
	// Reset settings
	iexplorerCheckBox.setSelected(model.isIExplorerDefault());
	netscape6CheckBox.setSelected(model.isNetscape6Default());
        mozillaCheckBox.setSelected(model.isMozillaDefault());        
    }


    /**
     * ActionListener interface implementation. 
     * Responds to user selection/deselection of the debug checkbox
     * 
     * @param actionEvent
     */
    public void actionPerformed(ActionEvent e) {
	if (e.getSource() == iexplorerCheckBox) 
	{
	    model.setIExplorerDefault(iexplorerCheckBox.isSelected());
	}
	if (e.getSource() == netscape6CheckBox) 
	{
	    model.setNetscape6Default(netscape6CheckBox.isSelected());
	}
        if (e.getSource() == mozillaCheckBox)
        {
            model.setMozillaDefault(mozillaCheckBox.isSelected());
        }

    }
    
    private static MessageHandler mh = new MessageHandler("browser");
    JCheckBox iexplorerCheckBox = new JCheckBox(mh.getMessage("ie.text"));
    JCheckBox netscape6CheckBox = new JCheckBox(mh.getMessage("ns6.text"));
    JCheckBox mozillaCheckBox   = new JCheckBox(mh.getMessage("moz.text"));

    /**
     * Show Error dialog for setting default VM for MSIE
     */
    private static void showIExplorerErrorDialog()
    {
	DialogFactory.showErrorDialog(mh.getMessage("settings.fail.ie.text"), 
				      mh.getMessage("settings.fail.caption"));
    }

    /**
     * Show Error dialog for setting default VM for Netscape 6
     */
    private static void showNetscape6ErrorDialog()
    {
	DialogFactory.showErrorDialog(mh.getMessage("settings.fail.ns6.text"), 
				      mh.getMessage("settings.fail.caption"));
    }

    /**
     * Show Error dialog for setting default VM for Mozilla
     */
    private static void showMozillaErrorDialog()
    {
        DialogFactory.showErrorDialog(mh.getMessage("settings.fail.moz.text"),
                                      mh.getMessage("settings.fail.caption"));
    }

    /**
     * Show Success dialog for setting default VM
     */
    private static void showSuccessDialog()
    {
	DialogFactory.showInformationDialog(mh.getMessage("settings.success.text"), 
					    mh.getMessage("settings.success.caption"));
    }

    /**
     * Show Alert dialog for setting default VM
     */
    private static boolean showAlertDialog()
    {
	int userChoice;
	userChoice = DialogFactory.showConfirmDialog(mh.getMessage("settings.alert.text"));
	return (userChoice == JOptionPane.YES_OPTION)?true:false;
    }
}

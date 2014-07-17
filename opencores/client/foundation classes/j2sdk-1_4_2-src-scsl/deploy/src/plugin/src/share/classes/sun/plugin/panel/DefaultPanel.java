/*
 * @(#)DefaultPanel.java	1.13 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.panel;

/*
 * The default panel shows the runtime version of the JRE, VM and
 * Java Plug-in
 *
 * @author Jerome Dochez
 * @version 1.13, 01/23/03
 */

import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import java.beans.PropertyChangeListener;
import java.beans.PropertyChangeEvent;
import java.util.ResourceBundle;
import sun.plugin.util.DialogFactory;


public class DefaultPanel extends ActivatorSubPanel { 

    /**
     * Construct the panel, add widgets
     */ 
    DefaultPanel(ConfigurationInfo model) {
	super(model);

	ResourceBundle rb = ResourceBundle.getBundle("sun.plugin.resources.Activator");
	setBorder(BorderFactory.createEmptyBorder(4,30,4,30));	

	setLayout(new BoxLayout(this, BoxLayout.Y_AXIS));

	// Add vertical pad
	add(Box.createGlue());
	Dimension vpad5 = new Dimension(1,5);
	add(Box.createRigidArea(vpad5));

	//create panels to add to the boxlayout for centering purposes
	JPanel panel1 = new JPanel();
    
	JLabel welcome = new JLabel(rb.getString("panel.caption"));
	welcome.setForeground(Color.black);
	welcome.setIcon(DialogFactory.loadIcon());
	panel1.add(welcome);
	add(panel1);

	// Add vertical pad
	add(Box.createRigidArea(vpad5));
	add(Box.createRigidArea(vpad5));

	JPanel panel2 = new JPanel();
	JLabel versionLabel = new JLabel(System.getProperty("java.runtime.name") + " " + System.getProperty("java.version"));
	panel2.add(versionLabel);
	add(panel2);	

	// Add vertical pad
	add(Box.createRigidArea(vpad5));

	JPanel panel3 = new JPanel();
	JLabel defaultLabel = new JLabel(rb.getString("default_vm_version") + System.getProperty("java.vm.version"), SwingConstants.CENTER);
	panel3.add(defaultLabel);
	add(panel3);

	// Add vertical pad
	add(Box.createRigidArea(vpad5));

        // Add the plug-in's product name and build version.  The 
        // version string is updated during the build using sed, so
        // you should only see the string listed below if this file
        // was built without using make (which should never happen).
	JPanel panel4 = new JPanel();
	JLabel productLabel = new JLabel(rb.getString("product_name") + " " + System.getProperty("javaplugin.version"), SwingConstants.CENTER);
	panel4.add(productLabel);
	add(panel4);

	// Add vertical pad
	add(Box.createRigidArea(new Dimension(1,10)));
	add(Box.createGlue());
	add(Box.createGlue());
    }


    private MessageHandler mh = new MessageHandler("sun.plugin.resources.Activator");
}


/*
 * @(#)JarCacheViewer.java	1.20 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 
package sun.plugin.cache;

import java.awt.*;
import java.awt.event.*;
import java.io.File;
import javax.swing.*;
import javax.swing.table.*;
import javax.swing.ListSelectionModel;
import javax.swing.event.ListSelectionListener;
import javax.swing.event.ListSelectionEvent;
import java.text.MessageFormat;
import java.util.*;
import sun.plugin.resources.ResourceHandler;
import sun.plugin.util.UserProfile;

public class JarCacheViewer extends JDialog {
    private  JarCacheTable viewerTable;
    private static String cacheLocation;

    public JarCacheViewer(String location) {
	if(location != null) {
	    cacheLocation = location;
	}
	Container contentPane = getContentPane();
	setTitle(ResourceHandler.getMessage("cache_viewer.caption"));	
	setModal(true);
	
	viewerTable = new JarCacheTable();
	JScrollPane pane = new JScrollPane (viewerTable);
	contentPane.add(pane, BorderLayout.CENTER);

	// refresh/remove/close button panel
	JPanel buttonPanel = new JPanel();
	buttonPanel.setLayout(new FlowLayout(FlowLayout.CENTER));

	JButton refresh = new JButton(ResourceHandler.getMessage("cache_viewer.refresh"));
	JButton remove = new JButton(ResourceHandler.getMessage("cache_viewer.remove"));
	JButton close = new JButton(ResourceHandler.getMessage("cache_viewer.close"));

	refresh.setMnemonic(ResourceHandler.getAcceleratorKey("cache_viewer.refresh")); 
        remove.setMnemonic(ResourceHandler.getAcceleratorKey("cache_viewer.remove")); 
        close.setMnemonic(ResourceHandler.getAcceleratorKey("cache_viewer.close")); 

	buttonPanel.setLayout(new FlowLayout(FlowLayout.RIGHT));
	buttonPanel.add(refresh);
	buttonPanel.add(remove);
	buttonPanel.add(close);

	contentPane.add(buttonPanel, BorderLayout.SOUTH);

	this.addWindowListener (
	    new WindowAdapter() {
		public void windowClosing(WindowEvent e) {
		    dispose();
		    //System.exit(0);
		}
	    }
        );

	//Action listeners for refresh button
	ActionListener refreshViewer = new ActionListener() 
	{
	    public void actionPerformed(ActionEvent e)  
	    {
		viewerTable.refresh();
	    }
	};

	//Action listeners for remove button
	ActionListener removeEntry = new ActionListener() 
	{
	    public void actionPerformed(ActionEvent e)  
	    {
		viewerTable.removeRows();
	    }
	};

	//Action listeners for close button
	ActionListener closeViewer = new ActionListener() 
	{ 
	    public void actionPerformed(ActionEvent e)  
	    {
		dispose();
		//System.exit(0);
	    }
	};

	refresh.addActionListener(refreshViewer);
	remove.addActionListener(removeEntry);
	close.addActionListener(closeViewer);

	getRootPane().addComponentListener (
	    new ComponentAdapter() {
		public void componentResized(ComponentEvent e) {
		    viewerTable.adjustColumnSize((Container)e.getComponent());
		}
	    }
        );

	setSize(800, 400);
	setVisible(true);
    }

    public static String getPluginCacheLocation() {
	if(cacheLocation != null) {
	    return cacheLocation;
	}else {
	    return UserProfile.getPluginCacheDirectory();
	}
    }

    public static void main(String args[]) {
	JarCacheViewer viewer = new JarCacheViewer(args[0]);
    }
}

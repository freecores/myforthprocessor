/*
 * @(#)ActivatorPanel.java	1.19 02/04/18
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.panel;

/**
 * This is the main Activator Panel panel. It contains the tabbed panes
 * the user can use to set its preferences for running the java plugin
 *
 * @version 	1.3
 * @author	Jerome Dochez
 */

import sun.plugin.*;
import javax.swing.*;
import java.util.*;
import java.util.jar.*;
import java.awt.*;
import java.awt.event.*;
import java.beans.*;
import java.net.URL;
import java.io.*;
import javax.swing.event.*;
import sun.plugin.util.Trace;
import sun.plugin.util.UserProfile;
import javax.swing.text.html.*;


public class ActivatorPanel extends ActivatorSubPanel 
		    implements ActionListener, PropertyChangeListener {

    /**
     * Construct the new panel
     * Add all the widgets
     */
    ActivatorPanel(ConfigurationInfo model) 
    {
	super(model);

	PlatformDependentInterface pdi = getPlatformObject();
	if (pdi!=null) 
	    pdi.onLoad(model);

	model.addPropertyChangeListener(this);

	JTabbedPane tabbedPanel = new JTabbedPane();       
	
	basicPanel = new BasicPanel(model);
	tabbedPanel.addTab(mh.getMessage("basic"), basicPanel);

        advancedPanel = new AdvancedPanel(model);
	tabbedPanel.addTab(mh.getMessage("advanced"), advancedPanel);

	if (model.isShowBrowserPanel())
	{
	    browserPanel = new BrowserPanel(model);
	    tabbedPanel.addTab(mh.getMessage("browser"), browserPanel);
	    // Windows platform, so Browser and Update panels are shown
	    showWindowPanels = true;
	}

	proxyPanel = new ProxyPanel(model);
	tabbedPanel.addTab(mh.getMessage("proxies"), proxyPanel);

	cachePanel = new CachePanel(model);
	tabbedPanel.addTab(mh.getMessage("cache"), cachePanel);

	certPanel = new CertificatePanel(model);
	tabbedPanel.addTab(mh.getMessage("cert"), certPanel);

/*
	// Add Java Update panel
	if (model.isShowBrowserPanel())
	{
	    updatePanel = new UpdatePanel(model);
	    tabbedPanel.addTab(mh.getMessage("update"), updatePanel);
	}
*/

	defaultPanel = new DefaultPanel(model);
	tabbedPanel.addTab(mh.getMessage("about"), defaultPanel);

	setLayout(new BorderLayout());
	add("Center", tabbedPanel);

	JPanel buttons = new JPanel();
	apply = addButton(buttons, mh.getMessage("apply"));
	apply.setMnemonic(mh.getAcceleratorKey("apply")); 
	cancel = addButton(buttons, mh.getMessage("cancel"));
	cancel.setMnemonic(mh.getAcceleratorKey("cancel")); 
	help = addButton(buttons, mh.getMessage("help"));
	help.setMnemonic(mh.getAcceleratorKey("help"));	
	apply.setEnabled(false);
	add("South", buttons);
	
	tabbedPanel.addChangeListener(new ChangeListener() {
    	    public void stateChanged(ChangeEvent e) {      	        
      	        JTabbedPane tabbedPane = (JTabbedPane)e.getSource();
      		
      		// The help html file needs to have the same ref names for each
      		// of the tabs (a href, a name). This is used for help.
      		if (showWindowPanels) {
	    	    if (tabbedPane.getSelectedIndex() == 0) {
            	   	currentTab = "#basic";
      		    } else if(tabbedPane.getSelectedIndex() == 1) {
	    	    	currentTab = "#advanced";
	    	    } else if(tabbedPane.getSelectedIndex() == 2) {
	    	    	currentTab = "#browser";
      		    } else if(tabbedPane.getSelectedIndex() == 3) {
	    	    	currentTab = "#proxies";
      		    } else if(tabbedPane.getSelectedIndex() == 4) {
	    	    	currentTab = "#cache";
      		    } else if(tabbedPane.getSelectedIndex() == 5) {
	    	    	currentTab = "#certificates";
	    	    } else if(tabbedPane.getSelectedIndex() == 6) {
	    	    	currentTab = "#update";	
      		    } else {
      	    	    	currentTab = "";
      		    }
		} else {
		    if (tabbedPane.getSelectedIndex() == 0) {
            	   	currentTab = "#basic";
      		    } else if(tabbedPane.getSelectedIndex() == 1) {
	    	    	currentTab = "#advanced";
      		    } else if(tabbedPane.getSelectedIndex() == 2) {
	    	    	currentTab = "#proxies";
      		    } else if(tabbedPane.getSelectedIndex() == 3) {
	    	    	currentTab = "#cache";
      		    } else if(tabbedPane.getSelectedIndex() == 4) {
	    	    	currentTab = "#certificates";
      		    } else {
      		    	currentTab = "";
      		    }
	   	}
    	    }
 	});

    }

    /**
     * The model has changed, enable the apply button to persist
     * the new user settings
     */
    public void propertyChange(PropertyChangeEvent evt) {
	if (apply != null) {
	    apply.setEnabled(true);
	}
    }

    /**
     * ActionListener implementation
     * If the user presses the Cancel button, the model is released from
     * its persisted state and the panels are reset.
     * If the user presses the Apply button, the model is persisted
     */
    public void actionPerformed(ActionEvent e) 
    {
	if (e.getSource()==cancel) {
	    try
	    {
		model.reset();
		PlatformDependentInterface pdi = getPlatformObject();
		if (pdi!=null) 
		    pdi.onLoad(model);
		basicPanel.reset();
		advancedPanel.reset();

		// Browser panel could be null if it is hidden
		if (browserPanel != null)
		{
		    browserPanel.reset();
		}
/*
		if (updatePanel != null)
		{
		    updatePanel.reset();
		}
*/

		proxyPanel.reset();
		certPanel.reset();
                cachePanel.reset();
		apply.setEnabled(false);
	    }
	    catch (Exception ex1)
	    {
		Trace.printException(this, ex1);
	    }
	}

	if (e.getSource()==apply) {
	    try {
	        model.applyChanges();
	        apply.setEnabled(false);
	        PlatformDependentInterface pdi = getPlatformObject();
	        if (pdi!=null) {
	   	    pdi.onSave(model);
	        }

	    } 
	    catch (Exception ex2) 
	    {
		Trace.printException(this, ex2, mh.getMessage("apply_failed") + ": " + ex2.getMessage(), 
				     mh.getMessage("apply_failed_title"));
	    }
	}
	
	if (e.getSource()==help) {
	    try {
	        if (help.isEnabled()) {
	            help.setEnabled(false);
	    	    showControlPanelHelp();	    	    
	    	}
 	    } catch (Exception ex3) {
 	        Trace.printException(this, ex3);
	    }
	}
    }

    /**
     * @return the model for this application
     */
    public ConfigurationInfo getModel() {
	return model;
    }

    /**
     * Utility method to add buttons to a panel
     */
    private JButton addButton(JPanel panel, String label) {
	JButton button = new JButton(label);
	button.addActionListener(this);
	panel.add(button);
	return button;
    }
    
    /*

     *   Call setJavaParmsText() function in AdvancedPanel to set text.
     *   This way it will not affect the size of ControlPanel - it's been packed by now.
     *   Fix for bug# 4415691.
     */
    public void setAdvancedPanelText()
    {
        advancedPanel.setJavaParmsText();
    }
    
    // Same for Certificate Panel
    public void setCertificatePanelText()
    {
        certPanel.reset();
    }

    public void setAdvancedPanel(int width)
    {
        advancedPanel.setTextSize(width);
    }        
    
    /**
     * Construct and show the Control Panel Help window
     *
     */
    public void showControlPanelHelp() 
    {
	      	
	final JEditorPane editorPane = new JEditorPane();
	editorPane.setEditable(false);

	    try {
		rb = ResourceBundle.getBundle("sun.plugin.resources.ControlPanelHelp");
	    } catch (Exception ex) {                        	    
		Trace.printException(this, ex, mh.getMessage("help.error.text"), mh.getMessage("help.error.caption"));
	    }

	    try {
		if (rb != null)
		    helpFile = rb.getString("help.file");
	    } catch (Exception ex) {
		Trace.printException(this, ex, mh.getMessage("help.error.text"), mh.getMessage("help.error.caption"));	
	    }


	JScrollPane editorScrollPane = new JScrollPane(editorPane);
	editorScrollPane.setVerticalScrollBarPolicy(
	    JScrollPane.VERTICAL_SCROLLBAR_ALWAYS);
	        editorScrollPane.setHorizontalScrollBarPolicy(
	    JScrollPane.HORIZONTAL_SCROLLBAR_ALWAYS);
	        editorScrollPane.setPreferredSize(new Dimension(700, 500));
                                                        	              	     	    
                  
	editorPane.setText(helpFile);       	    
	editorPane.setCaretPosition(0);  
	              	
	// close button panel
	JPanel buttonPanel = new JPanel();
	buttonPanel.setLayout(new FlowLayout(FlowLayout.CENTER));
	
	JButton close = new JButton(mh.getMessage("help_close"));
	
	close.setMnemonic(mh.getAcceleratorKey("help_close"));	    
	
	buttonPanel.add(close);
			
	final JFrame frame;
	frame = new JFrame(mh.getMessage("help_title"));
	frame.setDefaultCloseOperation(JFrame.DISPOSE_ON_CLOSE);

	frame.addWindowListener(new WindowAdapter() {
	    public void windowClosing(WindowEvent e) {
		frame.dispose();
		help.setEnabled(true);          	    
	    }
	});

	//Action listeners for close button
	ActionListener closeHelp = new ActionListener() {
	    public void actionPerformed(ActionEvent e) {
		frame.dispose();
		help.setEnabled(true);		    
	    }
	};
	
	close.addActionListener(closeHelp);
		
	frame.getContentPane().add(editorScrollPane, BorderLayout.CENTER);
	frame.getContentPane().add(buttonPanel, BorderLayout.SOUTH);
	frame.pack();
	frame.setVisible(true);
        
    }
    
    private DefaultPanel defaultPanel;
    private BasicPanel basicPanel;
    private AdvancedPanel advancedPanel;
    private BrowserPanel browserPanel;
    private ProxyPanel proxyPanel;
    private CertificatePanel certPanel;
    private CachePanel	cachePanel;
/*
    private UpdatePanel	updatePanel;
*/
    JButton cancel, apply, help;
    String currentTab = "";
    boolean showWindowPanels = false;
    private ResourceBundle rb = null;
    String helpFile ="";
			

    private MessageHandler mh = new MessageHandler("panel");
}



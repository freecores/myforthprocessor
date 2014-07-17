/*
 * @(#)ControlPanel.java	1.30 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.panel;

/**
  * Main Application class for displaying the Activator Control Panel
  *
  * @author Jerome Dochez
  * @version 1.1
  * @date 01/15/98
  */

import java.awt.BorderLayout;
import java.awt.Dimension;
import java.awt.Font;
import java.awt.Toolkit;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import javax.swing.JApplet;
import javax.swing.JFrame;
import sun.plugin.util.PluginUIManager;


public class ControlPanel extends JApplet
{
    static 
    {
	// Change metal theme to plugin customized one
	PluginUIManager.setTheme();
    }

    /**
     * The Java Control Panel to display useful info
     */
    private static ControlPanel panel = null;

    /*
     * Need to keep this around to set Text in Basic Panel, bug# 4415691
     * Also used to set the size of the JTextField (jdkPath) in AdvancedPanel.
     */
    public static ActivatorPanel apanel = null;
    
    /**
     * This will retrieve localized messages from the resouce bundles
     */
    private static MessageHandler mh = new MessageHandler("main");

    /**
     * Contructor for the applet/application
     */
    public ControlPanel() 
    {
    }

    /**
     * Initialize GUI for the control panel
     */
    public void init()
    {
	// Trigger native libraries to be loaded if necessary.
	ActivatorPanel.getPlatformObject();

	ConfigurationInfo model = new ConfigurationInfo();
        apanel = new ActivatorPanel(model);
	getContentPane().add(apanel);
	validate();
    }

    public void start(){
        /*
         * Set text in JRE runtime parameters.  Fix for bug# 4415691.
         */        
        apanel.setAdvancedPanelText();
        apanel.setCertificatePanelText();
    }

    /**
     * <p>
     * Return the control panel for this running Java Virtual Machine.
     * If the control panel is not created yet, create a new Java 
     * Control panel.
     *
     * @return control panel object
     * </p>
     */
    public static ControlPanel getJavaControlPanel() 
    {

	// Return control panel if exists
	if (panel != null)
	    return panel;

	// Otherwise, create a new control panel
	panel = new ControlPanel();
	panel.init();

	return panel;
    }

    /**
     * <p>
     * Checks if the Java Control Panel is visible.
     *
     * @return true if visible.
     * </p>
     */
    public static boolean isJavaControlPanelVisible()  {

	// Lazy control panel creation.
	if (panel == null)
	    return false;

	ControlPanel f = getJavaControlPanel();
	
	if (f != null)
	    return f.isVisible();
	else
	    return false;
    }

    /**
     * <p>
     * Show or hide Java Control Panel
     *
     * @param visible true if to show, else to hide.
     * </p>
     */
    public static void showJavaControlPanel(boolean visible)  {
	if (visible != isJavaControlPanelVisible())
        {
    	    ControlPanel f = getJavaControlPanel();
	    
	    if (f != null)
		f.setVisible(visible);
	}
    }

    /**
     * Main method for application.
     * @param args The commandline parameters.
     */
    public static void main(String[] args) {
     
	// Instantiate the control panel GUI and initialize.

	ControlPanel cp = new ControlPanel();  
	cp.init();   


	// Create the application frame and add the control panel.

	JFrame mainFrame = new JFrame(mh.getMessage("control_panel_caption"));

	mainFrame.getContentPane().setLayout(new BorderLayout());
	mainFrame.getContentPane().add(cp,"Center");
	mainFrame.pack();
//	mainFrame.setSize(400, 350);
	mainFrame.setResizable(false);


	// Exit when the window is closed.
	
	mainFrame.addWindowListener( new WindowAdapter() {
	    public void windowClosing(WindowEvent e) {
		System.exit(0);
	    }
	});
		

	// Size and center the frame.
	
	Dimension app_dimension = mainFrame.getSize();
	Dimension screen_dimension = Toolkit.getDefaultToolkit().getScreenSize();

	app_dimension.width += 30;	    
	
	mainFrame.setSize(app_dimension);
	mainFrame.setLocation((screen_dimension.width-app_dimension.width)/2,
			      (screen_dimension.height-app_dimension.height)/2);	


	// Start the application

	cp.start();

	mainFrame.setVisible(true);   
	mainFrame.toFront(); 
    }   
}

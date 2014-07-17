/*
 * @(#)AdvancedPanel.java	1.19 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.panel;

/**
 * The advanced panel contains settings for the JDK to run the javaplugin with
 * as well as debug information
 *
 * @version 	1.3
 * @author	Jerome Dochez
 */

import javax.swing.*;
import javax.swing.border.*;
import javax.swing.text.*;
import java.awt.event.*;
import java.awt.*;
import java.text.MessageFormat;
import sun.plugin.util.DialogFactory;

public class AdvancedPanel extends ActivatorSubPanel 
			   implements   ItemListener, 
					KeyListener
{

    /**
     * Construct the panel
     * Add the widgets
     */
    AdvancedPanel(ConfigurationInfo model) {

	super(model);

	setLayout(new BoxLayout(this, BoxLayout.Y_AXIS));

	JPanel jdkPanel = new JPanel();

	Border border = BorderFactory.createEtchedBorder();
	jdkPanel.setBorder(BorderFactory.createCompoundBorder(BorderFactory.createEmptyBorder(4, 4, 4, 4),
							      BorderFactory.createTitledBorder(border, mh.getMessage("jre_name"))));
				
	JPanel jdkVersion = new JPanel();
	jdkVersion.setLayout(new BoxLayout(jdkVersion, BoxLayout.Y_AXIS));
	jdkPanel.setLayout(new BoxLayout(jdkPanel, BoxLayout.X_AXIS));
	jdkVersion.setBorder(BorderFactory.createEmptyBorder(4,4,4,4));
	jdkPanel.add(jdkVersion);
	jdkInstalled = new JComboBox();
	jdkInstalled.setMaximumRowCount(3);
        
        jdkVersion.add(Box.createGlue());

	if (setInstalledJavaList()) {
	    jdkInstalled.addItemListener(this);
	    jdkVersion.add(jdkInstalled);
	}
        
        jdkVersion.add(Box.createGlue());
	
	add(jdkPanel);
//	add(Box.createRigidArea(new java.awt.Dimension(1,10)));
        add(Box.createGlue());
	JPanel pathPanel = new JPanel();
	pathPanel.setLayout(new GridLayout(3, 0));
	pathPanel.setBorder(BorderFactory.createEmptyBorder(15,8,4,4));
	pathPanel.add(Box.createGlue());
	//for spacial layout purposes
//	JPanel blankPanel = new JPanel();
//	pathPanel.add(blankPanel);
//	pathPanel.add(blankPanel);
//	pathPanel.add(blankPanel);

	
	JPanel smallPanel = new JPanel();
	smallPanel.setLayout(new BoxLayout(smallPanel, BoxLayout.X_AXIS));
	pathLabel = new JLabel(mh.getMessage("path"));
	smallPanel.add(pathLabel);
	jdkPath = new JTextField();
	jdkPath.setEnabled(false);
	smallPanel.add(jdkPath);
	pathPanel.add(smallPanel);
        pathPanel.add(Box.createGlue());
	
	jdkPanel.setLayout(new BoxLayout(jdkPanel, BoxLayout.Y_AXIS));
	jdkPanel.add(pathPanel);

        //Glue after jdkPanel
        add(Box.createGlue());

	JPanel options = new JPanel();
	options.setBorder(BorderFactory.createEmptyBorder(4,4,4,4));
	GridLayout gridLayout = new GridLayout(2,1);
	options.setLayout(gridLayout);

	options.add(new JLabel(mh.getMessage("java_parms"),
			       JLabel.LEFT));
	javaParms = new JTextField();
	options.add(javaParms);
	javaParms.addKeyListener(this);

	add(options);
//        add(Box.createGlue());
// Do not call reset here, it streches Control Panel depending on then length of JRE params string.
//	reset();
    }

    /* 
     * Setting text here.  By now we have created frame where this pane is shown
     * and packed it, so we can set text - it will not affect the size of
     * Control Panel.  Fix for bug# 4415691.
     */
    public void setJavaParmsText()
    {
        javaParms.setText(model.getJavaParms());

        /*
         * Call reset() now to update settings in Basic Panel.
         */
        reset();
    }

    /**
     * Initialize the list of JDK/JRE available for a particular JDK version
     * 
     * @param version the JDK version the user wants to use
     */
    private boolean setInstalledJavaList() {

	// Clean existing keys
	if (jdkInstalled.getItemCount()>0)
	    jdkInstalled.removeAllItems();

        jdkInstalled.addItem(defaultJRE);
	String[][] jreItems = model.getInstalledJREList();
	if (jreItems == null) {
	    jreNb = 0;
	}
	else {
	    jreNb = jreItems.length;
	}
	for (int i=0;i<jreNb;i++) {
	    String version = jreItems[i][0];
	    String path = jreItems[i][1];
            if (version != null && path != null){
	        Object []values = { version, path };
	        jdkInstalled.addItem(MessageFormat.format(mh.getMessage("jre_format"),
						          values));
            }
	}

	String[][] jdkItems = model.getInstalledJDKList();
	if (jdkItems == null) {
	    jdkNb = 0;
	} else {
	    jdkNb = jdkItems.length;
	}
	for (int i=0;i<jdkNb;i++) {
	    String version = jdkItems[i][0];
	    String path = jdkItems[i][1];
            if (version != null && path != null){
	        Object []values = { version, path };
	        jdkInstalled.addItem(MessageFormat.format(mh.getMessage("jdk_format"),
		                                          values));
            }
	}
	jdkInstalled.addItem(otherJDK);

	return true;
    }

    /**
     * ItemListener interface implementation. 
     * Responds to user changes to the jdk selection
     * 
     * @param actionEvent
     */
    public void itemStateChanged(ItemEvent e) {
        
        if (e.getSource()==jdkInstalled && e.getStateChange() == ItemEvent.SELECTED) {
	    int index = jdkInstalled.getSelectedIndex();
	    if (index==0) {
		model.setJavaRuntimeType("Default");
		model.setJavaRuntimeVersion(null);
		jdkPath.setText(""); 
	    }
	    if (index>0 && index<=jreNb) {
		model.setJavaRuntimeType("JRE");
		String[][] jres = model.getInstalledJREList();	       
		model.setJavaRuntimeVersion(jres[index-1][0]);
		jdkPath.setText("");                 
	    }
	    if (index>jreNb && index<=(jreNb+jdkNb)) {
		model.setJavaRuntimeType("JDK");
		String[][] jdks = model.getInstalledJDKList();	       
		model.setJavaRuntimeVersion(jdks[index-jreNb-1][0]);
		jdkPath.setText("");                  
	    }
	    if (index==(jdkInstalled.getItemCount()-1)) {
                
                /*
                 * See if we just resetting.  If that's the case, do not
                 * show warning dialog and file chooser, just display existing values.
                 */
                if(jdkPath.getText() == null || jdkPath.getText().length() < 1)
                {
                    /*
                     * Pop up a dialog warning user that this option is
		     * not supported !
                     */
                    Object[] options = {mh.getMessage("warning_popup_ok"), 
                                        mh.getMessage("warning_popup_cancel") };
           
	            int choice = DialogFactory.showOptionDialog(this, DialogFactory.WARNING_MESSAGE,
						    mh.getMessage("jre_selection_warning.info"), mh.getMessage("jre_selection_warning.caption"), options, options[1]);
 
                    if (choice == 0)
                    {
                        /* 
                        * Show filechooser, and put what will be chosen into the jdkPath.Text(...) 
                        */
	                JFileChooser chooser = new JFileChooser(); 
		        chooser.setFileSelectionMode(JFileChooser.FILES_AND_DIRECTORIES);
                        
                        /*
                         * Check if user entered/chose an existing directory.  If directory
                         * user entered does not exist, show a message dialog.
                         */
                        boolean valid_answer = false;
                        while( !valid_answer )
                        {
                            int returnVal = chooser.showOpenDialog(this);
		            if (returnVal == JFileChooser.APPROVE_OPTION) 
		            { 
    		                //Set jdkPath to what user selected.
                                if(chooser.getSelectedFile().isDirectory())
                                {
			            jdkPath.setText(chooser.getSelectedFile().getAbsolutePath());
		                    model.setJavaRuntimeType(jdkPath.getText());
                                    jdkInstalled.setSelectedIndex(jdkInstalled.getItemCount()-1);
                                    valid_answer = true;
                                }
                                else
                                {
                                    DialogFactory.showErrorDialog(this, mh.getMessage("error.text"),
								  mh.getMessage("error.caption"));
                                    valid_answer = false;
                                }
                            }
	                    else
	                    {
                                /*
                                * User pressed Cancel button.  Set current selection of JDK
                                * to Default JDK.
                                */
                                model.setJavaRuntimeType("Default");
                                jdkInstalled.setSelectedIndex(0);
                                valid_answer = true;
                            }
                        }//end while
                    }
                    else
                    {
                        //User pressed "CANCEL" in OptionDialog
                        model.setJavaRuntimeType("Default");
                        jdkInstalled.setSelectedIndex(0);
                    }       
                }
	    }
	    reset();
        }
    }

    /**
     * Key Listener implementation
     * Sets the custom JDK settings
     */
    public void keyTyped(KeyEvent e) {
	if (e.getSource() == javaParms) {
	    model.setJavaParms(javaParms.getText());
	} else if (e.getSource() == jdkPath) {
	    model.setJavaRuntimeType(jdkPath.getText());
	}
    }

    public void keyReleased(KeyEvent e) {
	// This shouldn't be necessary.  Unfortunately it seems to be.
	//					KGH 2/9/98
	keyTyped(e);
    }

    public void keyPressed(KeyEvent e) {}

    /**
     * Reset all the settings from the model
     */
    public void reset() {

        javaParms.setText(model.getJavaParms());

	boolean other = true;
	String jdk = model.getJavaRuntimeType();
	pathLabel.setEnabled(false);
	jdkPath.setEnabled(false);

	try {
	    if (jdk.equalsIgnoreCase("Default")) {
		jdkInstalled.setSelectedIndex(0);
	    } else {
		if (jdk.equalsIgnoreCase("JDK")) {
		    String[][] jdks = model.getInstalledJDKList();
		    String version = model.getJavaRuntimeVersion();
		    int jdkIndex = 0;
		    while (!jdks[jdkIndex][0].equalsIgnoreCase(version))
			jdkIndex++;
                    jdkInstalled.setSelectedIndex(jreNb+jdkIndex+1);
		} else { 
		    if (jdk.equalsIgnoreCase("JRE")) {
			String[][] jres = model.getInstalledJREList();
			String version = model.getJavaRuntimeVersion();
			int jreIndex = 0;
			while (!jres[jreIndex][0].equalsIgnoreCase(version))
			    jreIndex++;
			jdkInstalled.setSelectedIndex(jreIndex+1);
		    } else {
			jdkPath.setText(jdk);
			pathLabel.setEnabled(true);
			jdkInstalled.setSelectedIndex(jdkInstalled.getItemCount()-1);
		    }
		}
	    } 
	} catch(Exception e) {
	    // This is nasty but any kind of exception should trigger the
	    // reset of the property.
	    jdkInstalled.setSelectedIndex(0);
	    model.firePropertyChange();
	} 
	
    }
   
    public void setTextSize(int length)
    {
        jdkPath.setMaximumSize(new Dimension(length, 25));
        jdkPath.revalidate(); //This is to change the appearance (size)
    }

    private MessageHandler mh = new MessageHandler("advanced");
    private JTextField javaParms;
    private JTextField jdkPath;
    private JComboBox jdkInstalled;
    private final String otherJDK = mh.getMessage("other_jdk");
    private final String defaultJRE = mh.getMessage("default_jdk");
    private JLabel pathLabel;
    private int    jreNb, jdkNb;
}


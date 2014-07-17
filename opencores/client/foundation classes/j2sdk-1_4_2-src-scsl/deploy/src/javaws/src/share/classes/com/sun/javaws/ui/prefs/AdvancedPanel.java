/*
 * @(#)AdvancedPanel.java	1.10 03/01/23
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
import java.util.*;
import java.io.*;
import java.net.URL;
import java.net.URLConnection;
import java.net.HttpURLConnection;
import com.sun.javaws.*;
import com.sun.javaws.cache.*;
import com.sun.javaws.debug.Globals;
import com.sun.javaws.debug.Debug;
import com.sun.javaws.ui.player.Player;
import com.sun.javaws.ui.general.GeneralUtilities;

/**
 * @version 1.15 09/29/00
 */
public class AdvancedPanel extends PrefsPanel.PropertyPanel implements ActionListener {
    private JCheckBox _consoleCB;
    private JCheckBox _logFileCB;
    private JButton _logFileChooserButton;
    private JTextField _baseDirTF;
    private JLabel _currentSizeLabel;
    private JSlider _sizeTF;
    private JButton _clearButton;
    private JTextField _logFileTF;
    
    protected void createPropertyControls() {
        Insets insets = new Insets(0, 2, 5, 2);
        setBorder(new TitledBorder
		      (javax.swing.border.LineBorder.createBlackLineBorder(),
		       Resources.getString("controlpanel.advanced.title"),
		       TitledBorder.LEFT, TitledBorder.TOP));
        setLayout(new GridBagLayout());
        
	JPanel outputPanel = new JPanel(new GridBagLayout());

        _consoleCB = new JCheckBox(
	    Resources.getString("controlpanel.advanced.consoleCB"));
        _consoleCB.setRequestFocusEnabled(false);
	_consoleCB.setMnemonic(
	    Resources.getVKCode("controlpanel.advanced.consoleMnemonic"));

        outputPanel.add(_consoleCB, new GridBagConstraints(0, 1, 2, 1,
			    0, 0, GridBagConstraints.WEST,
			    GridBagConstraints.NONE, insets, 0, 0));
        
        _logFileCB = new JCheckBox(
	    Resources.getString("controlpanel.advanced.logOutputCB"));
        _logFileCB.setRequestFocusEnabled(false);
	_logFileCB.setMnemonic(
            Resources.getVKCode("controlpanel.advanced.logOutputMnemonic"));
        _logFileCB.addActionListener(this);
        outputPanel.add(_logFileCB, new GridBagConstraints(0, 2, 2, 1,
			    0, 0, GridBagConstraints.WEST,
			    GridBagConstraints.NONE, insets, 0, 0));
        
        _logFileChooserButton = new JButton(
	    Resources.getString("controlpanel.advanced.chooseLogButton"));
	_logFileChooserButton.setMnemonic(
            Resources.getVKCode("controlpanel.advanced.chooseLogMnemonic"));
        _logFileChooserButton.setEnabled(false);
        _logFileChooserButton.setRequestFocusEnabled(false);
        _logFileChooserButton.addActionListener(this);
        outputPanel.add(_logFileChooserButton, new GridBagConstraints(
			2, 2, 2, 1, 0, 0, GridBagConstraints.EAST,
			GridBagConstraints.NONE, insets, 0, 0));
        
	JLabel logFileName = new JLabel(
	    Resources.getString("controlpanel.advanced.logFileLabel"));
        
        outputPanel.add(logFileName, new GridBagConstraints(
			0, 3, 1, 1, 0, 0, GridBagConstraints.WEST,
			GridBagConstraints.NONE, insets, 0, 0));
        
        _logFileTF = new JTextField("NONE");
        _logFileTF.setEnabled(false);
        outputPanel.add(_logFileTF, new GridBagConstraints(
			2, 3, 1, 1, 1, 0, GridBagConstraints.CENTER,
			GridBagConstraints.HORIZONTAL, insets, 0, 0));

	logFileName.setDisplayedMnemonic(
            Resources.getVKCode("controlpanel.advanced.logFileMnemonic"));
	logFileName.setLabelFor(_logFileTF);
        
        
	JPanel cachePanel = new JPanel(new GridBagLayout());
	JLabel baseDirLabel = new JLabel(
	    Resources.getString("controlpanel.advanced.baseDirLabel"));
        cachePanel.add(baseDirLabel, new GridBagConstraints(
			0, 0, 1, 1, 0, 0, GridBagConstraints.WEST,
			GridBagConstraints.NONE, insets, 0, 0));
        
        _baseDirTF = new JTextField();
        _baseDirTF.setEditable(true);
        cachePanel.add(_baseDirTF, new GridBagConstraints(
			1, 0, 2, 1, 1, 0, GridBagConstraints.CENTER,
			GridBagConstraints.HORIZONTAL, insets, 0, 0));
        
	baseDirLabel.setLabelFor(_baseDirTF);
	baseDirLabel.setDisplayedMnemonic(
	    Resources.getVKCode("controlpanel.advanced.baseDirMnemonic"));

	JLabel sizeLabel = new JLabel(
	    Resources.getString("controlpanel.advanced.currentSizeLabel"));

        cachePanel.add(sizeLabel, new GridBagConstraints(
			0, 1, 1, 1, 0, 0, GridBagConstraints.WEST,
			GridBagConstraints.NONE, insets, 0, 0));
        
        _currentSizeLabel = new JLabel();
        cachePanel.add(_currentSizeLabel, new GridBagConstraints(
			1, 1, 1, 1, 0, 0, GridBagConstraints.WEST,
			GridBagConstraints.NONE, insets, 0, 0));
        
        _clearButton = new JButton(
	    Resources.getString("controlpanel.advanced.clearCacheButton"));
        _clearButton.setRequestFocusEnabled(false);
	_clearButton.setMnemonic(
            Resources.getVKCode("controlpanel.advanced.clearCacheMnemonic"));

        _clearButton.addActionListener(this);
        cachePanel.add(_clearButton, new GridBagConstraints(
			2, 1, 2, 1, 0, 0, GridBagConstraints.EAST,
			GridBagConstraints.NONE, insets, 0, 0));
        

        outputPanel.setBorder(new TitledBorder
                      (javax.swing.border.LineBorder.createBlackLineBorder(),
			Resources.getString("controlpanel.advanced.output"),
                       TitledBorder.LEFT, TitledBorder.TOP));

        cachePanel.setBorder(new TitledBorder
                      (javax.swing.border.LineBorder.createBlackLineBorder(),
			Resources.getString("controlpanel.advanced.cache"),
                       TitledBorder.LEFT, TitledBorder.TOP));

        this.add(outputPanel, new GridBagConstraints(
			0, 1, 1, 1, 1, 0, GridBagConstraints.CENTER,
			GridBagConstraints.HORIZONTAL, insets, 0, 0));
        
        this.add(cachePanel, new GridBagConstraints(
			0, 2, 1, 1, 1, 0, GridBagConstraints.CENTER,
			GridBagConstraints.HORIZONTAL, insets, 0, 0));
        revert();
    }

      private boolean isCacheDirValid(String path) {
	
	path = path + File.separator;

	File newcache = new File(path);	
	
	if (!newcache.exists()) {	  	
            newcache.mkdirs();	    
        } else {
	    // can get into lot of trouble pointing to existing file, or dir
            // that was not previously a cache dir - so don't allow.
	    
	    // fix for 4651873
	    // valid cache directory:
	    // 1.  empty directory
	    // 2.  existing javaws cache directory (directory with 
	    //   lastAccessed file in it)
	    File[] fileList = newcache.listFiles();

	    // not a directory
	    if (fileList == null) return false;

	    boolean valid_cache = false;

	    // check if the directory contains lastAccessed file
	    for (int i = 0; i<fileList.length; i++) {	
		if (fileList[i].getName().equals("lastAccessed")) {
		    valid_cache = true;
		}
	    }

	    // empty directory
	    if (fileList.length == 0) valid_cache = true;	    

	    if (!valid_cache) return false;
	   
        }

	if (newcache.canRead() && newcache.canWrite()) {
	    // This is to workaround a problem with File.canRead
	    // and File.canWrite on WinNT
	    // even the directory is set to read only, canWrite still
	    // return true on the directory
	    // try creating a temp file to make sure we can
	    // create a cache in this location
	    File tmp = null;
	    try {
		tmp = File.createTempFile("javaws", null, newcache);
		tmp.delete();
	    } catch (IOException ioe) {
		// cannot create temp file, return false
		Debug.ignoredException(ioe);
		return false;
	    }	   	  
	    return true;
	} else {	   	   	 
	    return false;
	}

    }

    
    //
    // Controller methods
    //
    public void apply() {
        ConfigProperties cp = getConfigProperties();
        String baseDir = _baseDirTF.getText();
	// check to make sure Cache Dir is valid before setting it
	if (isCacheDirValid(baseDir)) {
	    cp.put(ConfigProperties.CACHE_KEY+".dir", baseDir);
        } else {
	    GeneralUtilities.showMessageDialog(this, 
	      Resources.getString("cacheOption.invalidDir", baseDir), Resources.getString("cacheOption.invalidDir.title"), JOptionPane.ERROR_MESSAGE);
	}
        cp.put(ConfigProperties.SHOW_CONSOLE_KEY,
	       String.valueOf(_consoleCB.isSelected()));
        
        cp.put(ConfigProperties.LOG_FILENAME_KEY, _logFileTF.getText());
        cp.put(ConfigProperties.LOG_TO_FILE_KEY,
	       String.valueOf(_logFileCB.isSelected()));
        
    }
    
    public void revert() {
        ConfigProperties cp = getConfigProperties();
        
        String showConsole = (String)cp.get
	    (ConfigProperties.SHOW_CONSOLE_KEY);
        
        String logToFile = (String)cp.get
	    (ConfigProperties.LOG_TO_FILE_KEY);
        
        _consoleCB.setSelected(showConsole != null ?
				   Boolean.valueOf(showConsole).booleanValue() : true);
        
        _logFileCB.setSelected(logToFile != null ?
				   Boolean.valueOf(logToFile).booleanValue() : true);
        
        
        _logFileTF.setText((String)cp.get(ConfigProperties.LOG_FILENAME_KEY));
        
        _baseDirTF.setText((String)cp.get
			       (ConfigProperties.CACHE_KEY+".dir"));
        
        long cacheSize = 0;
        try {
	    cacheSize = InstallCache.getDiskCache().getCacheSize();
        } catch(IOException ioe) {
	    Debug.ignoredException(ioe);
        }
        
        _currentSizeLabel.setText(String.valueOf(cacheSize / 1024));
        _clearButton.setEnabled((cacheSize > 0));
        
        
        if (_logFileCB.isSelected() == true) {
	    _logFileTF.setEnabled(true);
	    _logFileTF.setEditable(true);
	    _logFileChooserButton.setEnabled(true);
	    
        } else {
	    _logFileTF.setEnabled(false);
	    _logFileTF.setEditable(false);
	    _logFileChooserButton.setEnabled(false);
        }
       
    }
    
    /**
     * Invoked from the <code>clearButton, fileChooserButton, logToFileCheck Box</code>,
     * depending on who the action was fired from, this method will do the neccessary
     * UI manipulations etc.
     */
    public void actionPerformed(ActionEvent ae) {
        Object srcObject = ae.getSource();
        if  (srcObject == _clearButton ) {
	    doClearButtonActions(ae);
        } else if (srcObject == _logFileChooserButton) {
	    doLogFileChooserActions(ae);
	    // fix for 4687857: focus is back to wrong window after closing 
	    // dialog
	    this.requestFocus();
        } else if (srcObject == _logFileCB) {
	    doLogToFileCB(ae);
        }
    }
    
    /**
     * A helper method to do all the necessary actions, to clear the
     * cache etc.
     */
    private void doClearButtonActions(ActionEvent ae) {
	String msg1 = Resources.getString("controlpanel.advanced.clearCache1");
	String msg2 = Resources.getString("controlpanel.advanced.clearCache2");
	Object [] message = new Object[2];

	message[0] = msg1;
	message[1] = msg2;

        if (GeneralUtilities.showOptionDialog (this, message, 
		Resources.getString("controlpanel.advanced.clearCacheTitle"),
		JOptionPane.YES_NO_OPTION, 
		JOptionPane.QUESTION_MESSAGE) == JOptionPane.YES_OPTION) {	 
	    
	    // fix for 4474867
	    Remover remover = new Remover();
	    new Thread(remover).start();
	  
	}
    }
 
    private class Remover implements Runnable {       

        public Remover() {
        }
        public void run() {
	    // clear cache
	    InstallCache.getCache().remove();
	    // refresh cache entry manager
	    Player.getPlayer().resetCacheEntryManager();
	    // FIXIT: It this right
	    // Invoke clear cache action closure
	    // getPrefsPanel().notifyCacheAction();
	    revert();
        }
    }

    /**
     * A helper method to launch the logFileChooser.
     */
    private void doLogFileChooserActions(ActionEvent ae){
	JFileChooser logFileChooser = new JFileChooser();
	int returnVal = logFileChooser.showOpenDialog(_logFileChooserButton);
	if (returnVal == JFileChooser.APPROVE_OPTION) {
	    ConfigProperties cp = getConfigProperties();
	    File file = logFileChooser.getSelectedFile();
	    _logFileTF.setText(file.getAbsolutePath());
	    cp.put(ConfigProperties.LOG_FILENAME_KEY,file.getAbsolutePath());
	}
    }
    /**
     * A helper method to perform the logToFileCB things which is
     * if false should disable the logFileTF and logFileChooserButton.
     * and vice versa.
     */
    private void doLogToFileCB(ActionEvent ae) {
	if (_logFileCB.isSelected() == true) {
	    _logFileTF.setEnabled(true);
	    _logFileTF.setEditable(true);
	    _logFileChooserButton.setEnabled(true);
	} else {
	    _logFileTF.setEnabled(false);
	    _logFileTF.setEditable(false);
	    _logFileChooserButton.setEnabled(false);
	}
    }
}


/*
 * @(#)GeneralPanel.java	1.29 03/01/23
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
import com.sun.javaws.debug.*;

/**
 * General Panel -
 *     This is the first tab on the Prefs Panel (also called controlpanel)
 *     It contains Browser setup information and Proxy settings.
 */
public class GeneralPanel extends PrefsPanel.PropertyPanel implements ActionListener {
    private JTextField _httpTF;
    private JTextField _portTF;
    private JRadioButton _noProxyJRB;
    private JTextArea  _proxyOverRideTA;
    private JRadioButton _manualProxyJRB;
    private JRadioButton _autoProxyJRB;
    private JTextField _autoProxyTF;
    private InternetProxyInfo _iProxyInfo;
    private InternetProxyInfo _browserIProxyInfo;
    
    public GeneralPanel() {
        InternetProxy iProxy = InternetProxy.getInstance();
        _iProxyInfo = iProxy.getDefaultInfo();
        // We try to see what the browser is set to
        _browserIProxyInfo = iProxy.getBrowserInfo();
    }
    
    public JPanel getProxyPanel() {
        return (JPanel) createProxyPanel();
    }
    
    protected void createPropertyControls() {
        setLayout(new GridBagLayout());
        
        // NOTE: If you change this, be sure to also change UnixGeneralPanel
        addGridBagComponent(this, Box.createVerticalGlue(),
			    0, 0, 1, 1, 1, .5, GridBagConstraints.CENTER,
			    GridBagConstraints.BOTH, null, 0, 0);
        
        addGridBagComponent(this, createProxyPanel(),
			    0, 1, 1, 1, 1, 0, GridBagConstraints.CENTER,
			    GridBagConstraints.HORIZONTAL, null, 0, 0);
        
        addGridBagComponent(this, Box.createVerticalGlue(),
			    0, 2, 1, 1, 1, .5, GridBagConstraints.CENTER,
			    GridBagConstraints.BOTH, null, 0, 0);
        
        revert();
    }
    
    protected Component createProxyPanel() {
        JPanel subPanel = new JPanel(new GridBagLayout());
        Insets insets = new Insets(0, 2, 5, 8);
        
        _noProxyJRB = new JRadioButton(
	    Resources.getString("controlpanel.proxy.noProxy"));
        _noProxyJRB.setMnemonic(
	    Resources.getVKCode("controlpanel.proxy.noProxyMnemonic"));
        
        _manualProxyJRB = new JRadioButton(
	    Resources.getString("controlpanel.proxy.manualProxy"));
        _manualProxyJRB.setMnemonic(
	    Resources.getVKCode("controlpanel.proxy.manualProxyMnemonic"));
        
        _autoProxyJRB = new JRadioButton(
	    Resources.getString("controlpanel.proxy.autoProxy"));
        
        _autoProxyJRB.setMnemonic(
	    Resources.getVKCode("controlpanel.proxy.autoProxyMnemonic"));
        ButtonGroup bgroup = new ButtonGroup();
        bgroup.add(_noProxyJRB);
        bgroup.add(_autoProxyJRB);
        bgroup.add(_manualProxyJRB);
        
        addGridBagComponent(subPanel,_noProxyJRB,
			    0, 0, 1, 1, 0, 0, GridBagConstraints.WEST,
			    GridBagConstraints.NONE, insets, 0, 0);
        _noProxyJRB.addActionListener(this);
        
        addGridBagComponent(subPanel,_autoProxyJRB,
			    0, 1, 1, 1, 0, 0, GridBagConstraints.WEST,
			    GridBagConstraints.NONE, insets, 0, 0);
        _autoProxyJRB.addActionListener(this);
        
        _autoProxyTF = new JTextField();
        _autoProxyTF.setEditable(false);
        _autoProxyTF.setEnabled(false);
        _autoProxyTF.setDisabledTextColor(Color.black);
        addGridBagComponent(subPanel,_autoProxyTF,
			    1, 1, GridBagConstraints.REMAINDER, 1, 1, 0,
                            GridBagConstraints.CENTER,
                            GridBagConstraints.HORIZONTAL, 
			    new Insets(0,2,5,2), 0, 0);
        
        addGridBagComponent(subPanel,_manualProxyJRB,
			    0, 2, 1, 2, 0, 0, GridBagConstraints.WEST,
			    GridBagConstraints.NONE, insets, 0, 0);
        _manualProxyJRB.addActionListener(this);
        
        JLabel proxyLabel = new JLabel(
	    Resources.getString("controlpanel.proxy.httpLabel"));
        JLabel portLabel = new JLabel(
	    Resources.getString("controlpanel.proxy.portLabel"));
        JLabel overRideLabel =  new JLabel(
	    Resources.getString("controlpanel.proxy.overRideLabel"));
        
        
        proxyLabel.setDisplayedMnemonic(
	    Resources.getVKCode("controlpanel.proxy.httpMnemonic"));
        portLabel.setDisplayedMnemonic(
	    Resources.getVKCode("controlpanel.proxy.portMnemonic"));
        overRideLabel.setDisplayedMnemonic(
	    Resources.getVKCode("controlpanel.proxy.overRideMnemonic"));
        
        
        
        addGridBagComponent(subPanel, proxyLabel,
			    1, 2, 1, 1, 0, 0, GridBagConstraints.EAST,
			    GridBagConstraints.NONE, insets, 0, 0);
        
        addGridBagComponent(subPanel, portLabel,
			    1, 3, 1, 1, 0, 0, GridBagConstraints.WEST,
			    GridBagConstraints.NONE, insets, 0, 0);
        
        addGridBagComponent(subPanel, overRideLabel,
			    0, 5, GridBagConstraints.REMAINDER, 1, 0, 0,
			    GridBagConstraints.WEST,
			    GridBagConstraints.HORIZONTAL, insets, 0, 0);
        
        
        insets.right = 2;
        _httpTF = new JTextField();
        proxyLabel.setLabelFor(_httpTF);
        addGridBagComponent(subPanel, _httpTF, 2, 2, 1, 1, 1, 0,
			    GridBagConstraints.CENTER,
			    GridBagConstraints.HORIZONTAL, insets, 0, 0);
        
        _portTF = new JTextField();
        portLabel.setLabelFor(_portTF);
        addGridBagComponent(subPanel, _portTF, 2, 3, 1, 1, 1, 0,
			    GridBagConstraints.CENTER,
			    GridBagConstraints.HORIZONTAL, insets, 0, 0);
        /**
	 * TODO: Need to add a Scrollable Pane at some time.
	 */
        _proxyOverRideTA = new JTextArea("localhost",3,10);
        _proxyOverRideTA.setLineWrap(true);
        _proxyOverRideTA.setWrapStyleWord(true);
        _proxyOverRideTA.setBorder(BorderFactory.createLineBorder(Color.black));
        overRideLabel.setLabelFor(_proxyOverRideTA);
        addGridBagComponent(subPanel, _proxyOverRideTA, 0, 6,3, 1, 1, 0,
			    GridBagConstraints.CENTER,
			    GridBagConstraints.HORIZONTAL, insets, 0, 0);
        

	// fix for 4450928
	_autoProxyJRB.setNextFocusableComponent(_manualProxyJRB);
	_manualProxyJRB.setNextFocusableComponent(_httpTF);
	_httpTF.setNextFocusableComponent(_portTF);

        
        subPanel.setBorder(new TitledBorder
			       (LineBorder.createBlackLineBorder(),
				Resources.getString("controlpanel.proxy.title"),
				TitledBorder.LEFT, TitledBorder.TOP));
        revert(); // Setup initial settings
        return subPanel;
    }
    
    //
    // Controller methods
    //
    public void apply() {
        ConfigProperties cp = getConfigProperties();
        
	int selection = -1;
	String host = null;
	int port = -1;
	String proxyOverride="";

        if (_noProxyJRB.isSelected()) {
	    selection = com.sun.javaws.proxy.InternetProxyInfo.NONE;
        } else if (_manualProxyJRB.isSelected()) {
	    selection = com.sun.javaws.proxy.InternetProxyInfo.MANUAL;
	    host = _httpTF.getText();
	    try {
	        port = Integer.parseInt(_portTF.getText()) ;
	    } catch (NumberFormatException n) {
	        port = -1;
	    }
	    proxyOverride = _proxyOverRideTA.getText();
	} else {
	    selection = com.sun.javaws.proxy.InternetProxyInfo.AUTO;	   
	    // do not set host and port if it is DirectConnection
	    if (_browserIProxyInfo.getType() != InternetProxyInfo.NONE) {
		host = _browserIProxyInfo.getHTTPHost();
		port = _browserIProxyInfo.getHTTPPort();
		proxyOverride = _browserIProxyInfo.getOverrides();
		
	    }
	}

	if (host == null) { 
	    host = "";
	}

	// update internet proxy info
	_iProxyInfo.setType(selection);
	_iProxyInfo.setHTTPHost(host);
	_iProxyInfo.setHTTPPort(port);
	String [] proxyOverrideArray = new String[1];
	proxyOverrideArray[0] = proxyOverride;
	_iProxyInfo.setOverrides(proxyOverrideArray);

	// update system properties 
	Properties p = System.getProperties(); 
	p.put("proxyHost", host); 
	p.put("proxyPort",String.valueOf(port)); 
	p.put("trustProxy", "true"); 
	p.put("https.proxyHost", host); 
	p.put("https.proxyPort", String.valueOf(port)); 
	p.put("http.nonProxyHosts", proxyOverride);
	

	// save manual settings	
	host = _httpTF.getText();
	try {
	    port = Integer.parseInt(_portTF.getText()) ;
	} catch (NumberFormatException n) {
	    port = -1;
	}
	proxyOverride = _proxyOverRideTA.getText();


	cp.setProxy(selection,
		    host,
		    port,
		    proxyOverride);
	
	// If we launch an application from command line or browser
	// the settings never get stored, in order to ensure it does
	// we store it now.
	cp.store(); 
	
	updateProxys();
    }
    
    public void revert() {
	ConfigProperties cp = getConfigProperties();
	
	if (cp.getProxyType() == com.sun.javaws.proxy.InternetProxyInfo.NONE) {
	    _noProxyJRB.setSelected(true);
	} else if (cp.getProxyType() == com.sun.javaws.proxy.InternetProxyInfo.MANUAL) {
	    _manualProxyJRB.setSelected(true);
	} else {
	    _autoProxyJRB.setSelected(true);
	}
	
	updateProxys();
	setInitialDialogPromptSettings();
	
	// If auto is selected but not enabled, default to none
	if (!_autoProxyJRB.isEnabled() && _autoProxyJRB.isSelected()) {
	    _noProxyJRB.setSelected(true);
	}
    }
    
    /**
     * Invoked from the <code>_useBrowserCB</code>, in which case the
     * textfields for the proxy are updated.
     */
    public void actionPerformed(ActionEvent ae) {
	updateProxys();
    }
    
    private void updateProxys() {
	if (_noProxyJRB.isSelected() == true){
	    enableManualUIGadgets(false);
	} else if (_manualProxyJRB.isSelected() == true){
	    enableManualUIGadgets(true);
	} else if (_autoProxyJRB.isSelected() == true) {//auto setting or Browser setting detection
	    enableManualUIGadgets(false);
	}
	
	if (Globals.TraceProxies == true) {
	    Debug.println("Browser info=" + _browserIProxyInfo);
	    Debug.println("Current info=" + _iProxyInfo);
	}
    }
    /**
     * Helper method: used to disable all the MANUAL related
     * GUI Items.
     */
    private void enableManualUIGadgets(boolean enable) {
	_httpTF.setEnabled(enable);
	_httpTF.setEditable(enable);
	_portTF.setEnabled(enable);
	_portTF.setEditable(enable);
	_proxyOverRideTA.setEditable(enable);
	_proxyOverRideTA.setEnabled(enable);
    }
    
    private void setInitialDialogPromptSettings() {
	// Initialize browser auto-detect fields
	if (_browserIProxyInfo.isValidAutoHTTPConfiguration() ||
	    _browserIProxyInfo.isValidManualHTTPConfiguration()) {
	    _autoProxyTF.setText(
		Resources.getString("controlpanel.proxy.BrowserAutoProxyInfo",
				    _browserIProxyInfo.getHTTPHost(),
				    Integer.toString(_browserIProxyInfo.getHTTPPort())));
	    _autoProxyJRB.setEnabled(true);
	    _noProxyJRB.setSelected(false);
	} else if (_browserIProxyInfo.getType() == InternetProxyInfo.NONE) {
	    _autoProxyTF.setText(
		Resources.getString("controlpanel.proxy.BrowserDirect"));
	    _autoProxyJRB.setEnabled(true);
	} else {
	    _autoProxyTF.setText(
		Resources.getString("controlpanel.proxy.BrowserNoDetect"));
	    _autoProxyJRB.setEnabled(false);
	}
	
	// Initialize manual text fields
	ConfigProperties cp = getConfigProperties();
	boolean havehints = false;
	if (cp.getHTTPProxy() != null && cp.getHTTPProxy() != "") {
	    _httpTF.setText(cp.getHTTPProxy());
	    _portTF.setText(cp.getHTTPProxyPort() == null ? "8080" :
			    cp.getHTTPProxyPort());
	    havehints = true;
	} else if (_browserIProxyInfo.getType() == InternetProxyInfo.AUTO &&
		   _browserIProxyInfo.getInvalidHTTPHost() != null &&
		   _browserIProxyInfo.getInvalidHTTPHost() != "") {
	    _httpTF.setText(_browserIProxyInfo.getInvalidHTTPHost());
	    _portTF.setText(_browserIProxyInfo.getInvalidHTTPPort() == -1 ?
			    "8080" :
			    Integer.toString(_browserIProxyInfo.
					     getInvalidHTTPPort()));
	    havehints = true;
	}
	
	// if auto proxy is selected but not enabled because we couldn't
	// find a setting, and if we have found what would be valid manual
	// settings, select the manual proxy radio button
	if (havehints && 
	    !_autoProxyJRB.isEnabled() && _autoProxyJRB.isSelected()) {
	    _manualProxyJRB.setSelected(true);
	    updateProxys();
	}

	String proxyOverRideStr = cp.getProxyOverrideString();
	if (proxyOverRideStr != null && proxyOverRideStr.compareTo("") == 0) {
	    // fix for 4418502
	    // do not put in "localhost" if user clears the field
	    proxyOverRideStr = "";
	}
	_proxyOverRideTA.setText(proxyOverRideStr);
	
    }
}



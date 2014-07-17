/*
 * @(#)Updater.java	1.13 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.autoupdater;

import com.sun.javaws.debug.Globals;
import com.sun.javaws.debug.Debug;
import com.sun.javaws.Resources;
import com.sun.javaws.BrowserSupport;
import javax.swing.*;
import java.net.URL;
import java.net.MalformedURLException;
import java.awt.*;
import java.awt.event.*;
import com.sun.javaws.ui.general.LinkButton;
import com.sun.javaws.ui.general.GeneralUtilities;


public class Updater {
    /**
     * This class is a front end to showDialog etc for the AutoUpdater.
     * The UpdateChecker is the class which does most of the work.
     * 1. We run the Updater as thread  and we run the UpdateChecker
     * as a separate thread, therefore we have the ability to Cancel the
     * UpdateChecker or once a url is found will use the Browser to visit
     * the URL.
     *
     */
    JDialog _theDialog;
    private JButton _closeButton;
    private JButton _gotoButton;
    JTextArea _statusMessageTA;
    
    String _updateUrlStr;
    
    /** Background checker thread */
    class UpdateCheckerThread extends Thread {
        
        public void run() {
            // Contact server and get update information
            String[] update = new UpdateChecker().isUpdateAvailable();
            // Post done message
            postMessage(Resources.getString("auto-updater.checkingDoneMsg"));
            //
            
            if (update != null) {
                _updateUrlStr = update[0];
                final String newVersion = update[1];
                SwingUtilities.invokeLater(new Runnable() {
                            public void run() {
                                synchronized(Updater.this) {
                                    if (_theDialog != null) {
                                        _gotoButton.setEnabled(true);
                                        _statusMessageTA.append(
                                            Resources.getString("auto-updater.availableMsg", newVersion));
                                        _gotoButton.setText(_updateUrlStr);
                                        _theDialog.pack();
                                        _theDialog.show();
                                    }
                                }
                            }
                        });
            } else {
                postMessage(Resources.getString("auto-updater.notAvailMsg", Globals.getBuildID()));
            }
        }
    }
    
    private void postMessage(final String message) {
        SwingUtilities.invokeLater(new Runnable() {
                    public void run() {
                        synchronized(Updater.this) {
                            if (_theDialog != null) {
                                _statusMessageTA.append(message + "\n");
                            }
                        }
                    }
                });
    }
    
    static public void showDialog(JFrame parent) {
        Updater updater = new Updater();
        updater.showmyDialog(parent);
    }
    
    private void showmyDialog(JFrame parent) {
        _theDialog = GeneralUtilities.createDialog(parent,
                         Resources.getString("auto-updater.updateMsgTitle"),
                         true);
        
        GridBagLayout gridbag = new GridBagLayout();
        GridBagConstraints constraint  = null;
        new GridBagConstraints();
        
        Container c = _theDialog.getContentPane();
        c.setLayout(gridbag);
        
        JPanel srvrmsgPanel = new JPanel(new GridLayout());
        
        constraint=new GridBagConstraints();
        constraint.gridx=0;
        constraint.gridy=0;
        constraint.gridwidth=java.awt.GridBagConstraints.REMAINDER;
        constraint.anchor=java.awt.GridBagConstraints.CENTER;
        constraint.fill=java.awt.GridBagConstraints.VERTICAL;
        gridbag.setConstraints(srvrmsgPanel,constraint);
        c.add(srvrmsgPanel,constraint);
        
        _statusMessageTA = new JTextArea(5,30) {
            public Dimension getPreferredSize() {
                Dimension dvalue = super.getPreferredSize();
                if (dvalue.width < 450) {
                    dvalue.width = 450;
                }
                return dvalue;
            }
        };
	// fix for 4651860
	_statusMessageTA.setText(Resources.getString("appname") + "\n");
        _statusMessageTA.append(Resources.getString("auto-updater.checkingMsg"));
        _statusMessageTA.setEditable(false);
        _statusMessageTA.setEnabled(false);
        _statusMessageTA.setDisabledTextColor(Color.black);
        Color bgColor=srvrmsgPanel.getBackground();
        _statusMessageTA.setBackground(bgColor);
        _statusMessageTA.setBorder(
            BorderFactory.createCompoundBorder(
                                      BorderFactory.createEmptyBorder(4,4,4,4),BorderFactory.createLineBorder(Color.black))
        );
        
        srvrmsgPanel.add(_statusMessageTA);
        
        JPanel buttonPanel = new JPanel(new GridLayout(1,0,5,10));
        buttonPanel.setBorder(BorderFactory.createEmptyBorder(0,0,4,0));
        ActionListener buttonActionListener = new ActionListener() {
            public void actionPerformed(ActionEvent ae) {
                Object srcObject = ae.getSource();
                
                if (srcObject == _closeButton) {
                    _statusMessageTA.append(Resources.getString("auto-updater.checkingAbortMsg")+"\n");
                    JDialog dialog = _theDialog;
                    // Need to synchronzie for the invokeLaters
                    synchronized(this) {
                        _theDialog = null;
                    }
                    dialog.dispose();
                } else if (srcObject == _gotoButton) {
                    // Do the Browser support
                    BrowserSupport bs = BrowserSupport.getInstance();
                    
                    try {
                        bs.showDocument(new URL(_updateUrlStr));
                    } catch (MalformedURLException m) {
                        // For now dont do anything.
                        if (Globals.TraceAutoUpdater) {
                            Debug.println("Malformed URL = " + _updateUrlStr);
                        }
                    }
                } else {
                    // Do nothing....
                }
            }
        };

        _theDialog.addKeyListener(new KeyAdapter() {
            public void keyPressed(KeyEvent e) {
                int close = Resources.getVKCode("auto-updater.closeMnemonic");
                if (e.getKeyCode() == close) {
                    _closeButton.doClick();
                }
            }
        } );

        JPanel linkPanel = new JPanel(new BorderLayout());
        
        JLabel urlLabel = new JLabel(Resources.getString("auto-updater.URL"));
        urlLabel.setBorder(BorderFactory.createEmptyBorder(0,0,0,10));
        linkPanel.add(urlLabel,BorderLayout.WEST);
        
        _gotoButton = new LinkButton();
        _gotoButton.setEnabled(false);
        _gotoButton.addActionListener(buttonActionListener);
        linkPanel.add(_gotoButton, BorderLayout.CENTER);
        
        constraint=new GridBagConstraints();
        constraint.gridx=0;
        constraint.gridy=1;
        //constraint.insets = new Insets(10,0,0,0);
        constraint.gridwidth=java.awt.GridBagConstraints.REMAINDER;
        constraint.anchor=java.awt.GridBagConstraints.WEST;
        c.add(linkPanel,constraint);
        
        linkPanel.setBorder(BorderFactory.createEmptyBorder(4,4,4,4));
        
        _closeButton = new JButton(
            Resources.getString("auto-updater.closeButton"));
        buttonPanel.add(_closeButton);
        _closeButton.addActionListener(buttonActionListener);
        
        constraint=new GridBagConstraints();
        constraint.gridx=0;
        constraint.gridy=2;
        constraint.gridwidth=java.awt.GridBagConstraints.REMAINDER;
        constraint.anchor=java.awt.GridBagConstraints.CENTER;
        gridbag.setConstraints(buttonPanel,constraint);
        c.add(buttonPanel,constraint);
        
        _theDialog.pack();
        
        // Center the dialog stolen from LaunchErrorDialog.java
        Rectangle size =  _theDialog.getBounds();
        Dimension screenSize = Toolkit.getDefaultToolkit().getScreenSize();
        size.width = Math.min(screenSize.width, size.width);
        size.height = Math.min(screenSize.height, size.height);
        // Center the window
        _theDialog.setBounds((screenSize.width - size.width) / 2,
                                 (screenSize.height - size.height) / 2,
                             size.width, size.height);
        
        // We start the update checker thread just before we show.
        new UpdateCheckerThread().start();
        
        // Show dialog
        _theDialog.show();
    }
}



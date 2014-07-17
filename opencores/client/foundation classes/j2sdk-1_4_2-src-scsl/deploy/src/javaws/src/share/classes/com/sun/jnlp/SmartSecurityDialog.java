/*
 * @(#)SmartSecurityDialog.java	1.24 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jnlp;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import java.security.*;
import com.sun.javaws.ui.general.GeneralUtilities;
import com.sun.javaws.Resources;
import com.sun.javaws.util.GeneralUtil;
import com.sun.javaws.security.JavaWebStartSecurity;
import com.sun.javaws.Main;

/** Package private */
final class SmartSecurityDialog extends Thread{
    private boolean _remembered = false;
    private int _lastResult = -1;
    private boolean _cbChecked = false;
    private int _answer;
    private Object _signalObject = null;
    private String _message = null;
    private DummyDialog _dummyDialog;
    private EventQueue _sysEventQueue = null;
    private static final ThreadGroup _secureGroup = 
	Main.getSecurityThreadGroup();

    /** Package private */
    
    SmartSecurityDialog() {
	this(null);
    }
    
    SmartSecurityDialog(String message) {
	_signalObject = new Object();
	_message = message;
    }
    
    boolean showDialog(String message) {
	_message = message;
	return showDialog();
    }
    
    /** Return true/false for accept */
    boolean showDialog() {
	Integer intResult = (Integer)AccessController.doPrivileged(
	    new PrivilegedAction() {
		public Object run() {
		    return new Integer(getUserDecision(null, _message));
		}
	    });
	return (intResult.intValue() == JOptionPane.YES_OPTION ? true : false);
    }
    
    private int getUserDecision(Frame f, String message) {
        if (_remembered) {
	    _answer = _lastResult;
	    return _answer;
	}
	
	synchronized (_signalObject) {
	    
	    _sysEventQueue = Toolkit.getDefaultToolkit().getSystemEventQueue();
	    
	    final Thread handler = new Thread(_secureGroup, this, "userDialog");
	    
	    _message = message;
	    
	    _dummyDialog = new DummyDialog((Frame)null, true);
	    _dummyDialog.addWindowListener(new WindowAdapter() {
		public void windowOpened(WindowEvent e) {
		    // We must insure (on unix) that the dummy dialog is shown
		    // first, or else it be on top and modal. Last modal dialog
		    // up is winner, so gaurentee handler not started till the
		    // dummy dialog is allready opened.
		    handler.start();
		}
		public void windowClosing(WindowEvent e) {
		    _dummyDialog.hide();
		}
	    });
            Rectangle rect = new Rectangle(new Point(0,0),
                                 Toolkit.getDefaultToolkit().getScreenSize());
            _dummyDialog.setLocation(rect.x + rect.width/2 - 50,
                                     rect.y + rect.height/2);

	    _dummyDialog.setResizable(false);
	    _dummyDialog.toBack();
	    _dummyDialog.show();
	    
	    try {
		_signalObject.wait();
	    } catch (Exception e) {
		e.printStackTrace();
		_dummyDialog.hide();
	    }
	    return _answer;
	}
	
    }
    
    public void run() {
        JPanel panel = new JPanel();
        final JCheckBox cb = new JCheckBox(Resources.getString("APIImpl.securityDialog.remember"));
        Font font = cb.getFont();
        Font newF = null;
        if (font != null) {
	    newF = font.deriveFont(Font.PLAIN);
	    if (newF != null) {
		cb.setFont(newF);
	    }
        }
        
        class csiCheckBoxListener implements ItemListener {
	    public void itemStateChanged(ItemEvent e) {
		Object source = e.getItemSelectable();
		if (source == cb) {
		    if (e.getStateChange() == ItemEvent.DESELECTED) {
			setCBChecked(false);
		    } else if (e.getStateChange() == ItemEvent.SELECTED) {
			setCBChecked(true);
		    }
		}
	    }
        }
        
        cb.addItemListener(new csiCheckBoxListener());
        panel.add(cb, BorderLayout.CENTER);
        cb.setOpaque(false);
        panel.setOpaque(false);
        
        Object [] newArray = { _message, panel };
	
        Object [] opts = { Resources.getString("APIImpl.securityDialog.no"),
		Resources.getString("APIImpl.securityDialog.yes") };
        
	int realResult = JOptionPane.NO_OPTION;
	try {
	    int result = GeneralUtilities.showOptionDialog(null, newArray,
			 Resources.getString("APIImpl.securityDialog.title"),
			 JOptionPane.YES_NO_OPTION,
			 JOptionPane.QUESTION_MESSAGE,
			 opts, opts[1]);
	    
	    realResult = (result == 1 ? JOptionPane.YES_OPTION : JOptionPane.NO_OPTION);
	    
	    if (_cbChecked) {
		_remembered = true;
		_lastResult = realResult;
	    }
	} finally {
	    _dummyDialog.secureHide();
	    
	    synchronized (_signalObject) {
	        _answer = realResult;
	        _signalObject.notify();
	    }
	}
    }
    
    private void setCBChecked(boolean checked) {
        _cbChecked = checked;
    }

    private class DummyDialog extends JDialog {
	private ThreadGroup _unsecureGroup;

	DummyDialog(Frame owner, boolean isModal) {
	    super(owner, isModal);
	    _unsecureGroup = Thread.currentThread().getThreadGroup();
	}

	public void secureHide() {
            (new Thread(_unsecureGroup, new Runnable() {
                    public void run() { DummyDialog.this.hide(); }
                } )
            ).start();
	}
    }    
}


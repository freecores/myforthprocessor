/*
 * @(#)JAuthenticator.java	1.25 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws;

import javax.swing.*;
import java.net.*;
import java.awt.*;
import java.awt.event.*;
import java.util.*;
import com.sun.javaws.debug.*;
import com.sun.javaws.ui.general.GeneralUtilities;
import com.sun.javaws.jnl.LaunchDesc;
import com.sun.jnlp.JNLPClassLoader;
import java.security.AccessControlException;

public class JAuthenticator extends Authenticator implements ActionListener {
    private Frame _parentFrame;
    private boolean _isMyFrame=false;
    private DownloadWindow _downloadWindow;
    private JButton _okButton;
    private JButton _clearButton;
    private JButton _cancelButton;
    private JTextField _usernameTF;
    private JPasswordField _password;
    private JDialog _thedialog;
    private static JAuthenticator _instance;
    private boolean _challanging = false;
    private boolean _cancel = false;
 
    private JAuthenticator() {
        super();
    }

    static public synchronized JAuthenticator getInstance(Frame f) {
        if (_instance == null) {
            _instance = new JAuthenticator();
        }
        _instance._downloadWindow = null;
        _instance._parentFrame = f;
        return _instance;
    }
    
    
    static public synchronized JAuthenticator getInstance(DownloadWindow dw) {
        if (_instance == null) {
            _instance = new JAuthenticator();
        }
        _instance._downloadWindow = dw;
        _instance._parentFrame = dw.getFrame();
        return _instance;
    }
    private void stopProgressBar(boolean on) {
        if (_downloadWindow != null ) {
            /* This is causing a problem and never worked right
             so for now I am commenting it out
             _downloadWindow.setIndeterminedProgressBar(on);
             */
        }
    }
 
    protected synchronized PasswordAuthentication getPasswordAuthentication() {
        String hostname = "";
        InetAddress siteAddr;
        if ( (siteAddr = getRequestingSite()) != null ) {
            hostname = siteAddr.getHostName();
        }
        String resource = getRequestingPrompt();

	/*
	 * Theory behind the following code:
	 * 1.) If the host making the chalange is neither the proxy nor the
	 *     host we downloaded from, then the application is connecting
	 *     to this host itself.  In that case we have no business poping
	 *     up dialog or suggesting login name and password. Let app handle.
	 * We now do the following:
	 * 2.) There may, however, be cases where the app is it'self connecting
	 *     to another host on the net and still want to use our dialog to
	 *     get password (since the app can't call setDefaults() itself, 
	 *     unless it is running java 1.4 or later) for that reason we 
	 *     check another config property which the app can set either way.
	 */
	ConfigProperties cp = ConfigProperties.getInstance();
	String proxyHost = cp.getHTTPProxy();
	int jaType = cp.getJAuthenticator();
        // JA_ALL is default, so normally all this is skipped
	if (jaType != ConfigProperties.JA_ALL) {
	    if (jaType == ConfigProperties.JA_NONE) {
		return null;
	    }
	    boolean isProxy = hostname != null && hostname.equals(proxyHost);
	    boolean isLaunch = true; // is launch host til we prove otherwise 
	    if (isProxy) {
	        if ((jaType & ConfigProperties.JA_PROXY) == 0) {
		    /* from proxy, JA_PROXY not included */
		    return null;
		}
	    } else {
	        ClassLoader cl = Thread.currentThread().getContextClassLoader();
	        while (cl != null && !(cl instanceof JNLPClassLoader)) {
		    cl = cl.getParent();
	        }
	        if (cl != null) {
		    LaunchDesc ld = ((JNLPClassLoader) cl).getLaunchDesc();
		    if (ld != null) {		 
		        String launchHost = (ld.getLocation()).getHost();
		        isLaunch = ((hostname != null) && 
				    hostname.equals(launchHost));
		    }
		}
	        if (isLaunch) {
		    if ((jaType & ConfigProperties.JA_DOWNLOAD) == 0) {
			/* from launch host JA_DOWNLOAD not included */
			return null;
		    }
		} else {
		    /* not from download host or proxy, and not JA_ALL */
		    return null;
		}
	    }
        } 
        
        
        if (Globals.TraceProxies) {
            Debug.println("JAuthenticator: Did not find entry for:"+hostname+"/"+resource);
        }
        if (_downloadWindow != null) {
            _parentFrame = _downloadWindow.getFrame();
        }
        
	if (_parentFrame != null && !_parentFrame.isVisible()) {
	    // give other thread a chance to finish making DownloadWindow vis
	    try { Thread.sleep(1000); } catch (InterruptedException ie) {/* ignore */}
	}

        _thedialog = GeneralUtilities.createDialog(this._parentFrame,
                         Resources.getString("authenticator.title"),true);
        
        _thedialog.setLocationRelativeTo(this._parentFrame);
        

	_thedialog.addWindowListener(new WindowAdapter() {

		public void windowClosing(WindowEvent e) {
		    cancel();
		}

	    });

        stopProgressBar(true);
        
        GridBagLayout gridbag = new GridBagLayout();
        GridBagConstraints constraint  = null;
        new GridBagConstraints();
        
        Container c = _thedialog.getContentPane();
        c.setLayout(gridbag);
        
        JPanel srvrmsgPanel = new JPanel(new GridLayout());
        
        srvrmsgPanel.add(new JLabel(Resources.getString("authenticator.serverprompt", hostname, resource)));
        
        constraint=new GridBagConstraints();
        constraint.gridx=0;
        constraint.gridy=0;
        constraint.gridwidth=java.awt.GridBagConstraints.REMAINDER;
        constraint.anchor=java.awt.GridBagConstraints.CENTER;
        gridbag.setConstraints(srvrmsgPanel,constraint);
        c.add(srvrmsgPanel,constraint);
        
        JLabel usernameLabel = new JLabel(
            Resources.getString("authenticator.username"));
        constraint=new GridBagConstraints();
        constraint.insets = new Insets(12, 5, 0, 11);
        constraint.anchor=java.awt.GridBagConstraints.WEST;
        constraint.gridx=0;
        constraint.gridy=1;
        c.add (usernameLabel, constraint);
        
        _usernameTF = new JTextField(15);
        constraint=new GridBagConstraints();
        constraint.insets = new Insets(12, 0, 0, 11);
        constraint.fill = java.awt.GridBagConstraints.HORIZONTAL;
        constraint.anchor=java.awt.GridBagConstraints.EAST;
        constraint.gridx=1;
        constraint.gridy=1;
        constraint.weightx = 1.0;
        c.add (_usernameTF, constraint);
        
        JLabel passwordLabel = new JLabel(
            Resources.getString("authenticator.password"));
        constraint=new GridBagConstraints();
        constraint.insets = new Insets(12, 5, 0, 0);
        constraint.anchor=java.awt.GridBagConstraints.WEST;
        constraint.gridx=0;
        constraint.gridy=2;
        c.add (passwordLabel, constraint);
        
        
        _password = new JPasswordField(15);
        _password.setEchoChar('*');
        constraint=new GridBagConstraints();
        constraint.fill = java.awt.GridBagConstraints.HORIZONTAL;
        constraint.insets = new Insets(12, 0, 0, 11);
        constraint.anchor=java.awt.GridBagConstraints.EAST;
        constraint.weightx=1.0;
        constraint.gridx=1;
        constraint.gridy=2;
        c.add(_password, constraint);
        
        constraint=new GridBagConstraints();
        constraint.insets = new Insets(11, 7, 0, 11);
        constraint.fill = java.awt.GridBagConstraints.HORIZONTAL;
        constraint.gridwidth=java.awt.GridBagConstraints.REMAINDER;
        constraint.gridx=0;
        constraint.gridy=3;
        c.add( new JSeparator(javax.swing.SwingConstants.HORIZONTAL), constraint);
        
        JPanel buttonPanel = new JPanel(new GridLayout(1,0,5,10));
        
        _okButton= new JButton(
            Resources.getString("authenticator.okbutton"));
        buttonPanel.add(_okButton);
        _okButton.addActionListener(this);
	_thedialog.getRootPane().setDefaultButton(_okButton);
        
        _clearButton = new JButton(
            Resources.getString("authenticator.clearbutton"));
        buttonPanel.add(_clearButton);
        _clearButton.addActionListener(this);
        
        _cancelButton = new JButton(
            Resources.getString("authenticator.cancelbutton"));
        buttonPanel.add(_cancelButton);
        _cancelButton.addActionListener(this);
        
        constraint=new GridBagConstraints();
        constraint.gridx=0;
        constraint.gridy=4;
        constraint.gridwidth=java.awt.GridBagConstraints.REMAINDER;
        constraint.anchor=java.awt.GridBagConstraints.CENTER;
        gridbag.setConstraints(buttonPanel,constraint);
        c.add(buttonPanel,constraint);
        
        _thedialog.pack();
        // Center the dialog stolen from LaunchErrorDialog.java

        Rectangle size =  _thedialog.getBounds();
        Dimension screenSize = Toolkit.getDefaultToolkit().getScreenSize();
        size.width = Math.min(screenSize.width, size.width);
        size.height = Math.min(screenSize.height, size.height);
        // Center the window
        _thedialog.setBounds((screenSize.width - size.width) / 2,
                             (screenSize.height - size.height) / 2,
                         size.width, size.height);
        
        SplashScreen.hide();
	_challanging = true;
        _thedialog.show();
	_challanging = false;
        
	// return null if user choose cancel the password authentication
	if (_cancel) {
	    _cancel = false;
	    return null;
	}

        return new PasswordAuthentication (_usernameTF.getText(),
                _password.getPassword());
    }
    public void actionPerformed(ActionEvent ae) {
        Object srcObject = ae.getSource();
        if (srcObject == _okButton) {
            stopProgressBar(false);
            _thedialog.setVisible(false);
            _thedialog.dispose();
        } else if (srcObject == _clearButton) {
	    _usernameTF.setText(null);
	    _password.setText(null);
        } else {
	    cancel();
        }
    }

    private void cancel() {
	_cancel = true;
	stopProgressBar(false);
	_thedialog.setVisible(false);
	_thedialog.dispose();
	//Just continue and let the system throw an exception if need be.
    }

    boolean isChallanging() {
	return _challanging;
    }
    
}








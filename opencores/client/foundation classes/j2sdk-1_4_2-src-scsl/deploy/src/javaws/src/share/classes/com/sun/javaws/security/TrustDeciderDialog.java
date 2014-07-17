/*
 * @(#)TrustDeciderDialog.java	1.6 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws.security;

import java.awt.Component;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;
import java.io.IOException;
import java.net.URL;
import java.security.Principal;
import java.security.PrivilegedExceptionAction;
import java.security.cert.X509Certificate;
import java.util.ArrayList;
import javax.swing.Icon;
import javax.swing.ImageIcon;
import javax.swing.JButton;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JOptionPane;
import com.sun.javaws.ui.general.GeneralUtilities;
import com.sun.javaws.Resources;
import com.sun.javaws.debug.Debug;


class TrustDeciderDialog {

    public static final int TrustOption_GrantThisSession = 0;
    public static final int TrustOption_Deny = 1;
    public static final int TrustOption_GrantAlways = 2;


    private java.security.cert.Certificate[] certs = null;
    private int start  = 0;
    private int end = 0;
    private boolean rootCANotValid = false;
    private boolean timeNotValid = false;
    private boolean httpsDialog = false;
    private DeploymentCertificateDialog _dialog = null;

    /**
     * <P> Constructor for TrustDeciderDialog.
     * </P>
     *
     * @param certs Array of certificates
     */
    TrustDeciderDialog(java.security.cert.Certificate[] certs, int start, int end, boolean rootCANotValid, boolean timeNotValid)  {
	this.certs = certs;
	this.start = start;
	this.end = end;
	this.rootCANotValid = rootCANotValid;
	this.timeNotValid = timeNotValid;
    	this._dialog = new DeploymentCertificateDialog(certs, start, end);
    }

    /**
     * <P> Show TrustDeciderDialog.
     * </P>
     *
     * @return 0 if "Grant this session" button is clicked.
     * @return 1 or -1 if "Deny" is clicked.
     * @return 2 if "Grant Always" button is clicked.
     */
    int DoModal()  
    {

	    int ret = -1;

	    // Check if the certificate is a x.509 certificate
	    //
	    if (certs[start] instanceof X509Certificate 
		&& certs[end-1] instanceof X509Certificate)
	    {
		X509Certificate cert = (X509Certificate) certs[start];
		X509Certificate cert2 = (X509Certificate) certs[end-1];

		Principal prinSubject = cert.getSubjectDN();
		Principal prinIssuer = cert2.getIssuerDN();

		// Extract subject name
		String subjectDNName = prinSubject.getName();

		String subjectName = null;

		int i = subjectDNName.indexOf("CN=");
		int j = 0;

		if (i < 0)
		{
		    subjectName = getMessage("security_dialog.unknown.subject");
		}
		else
		{
		    try
		    {
			// Shift to the beginning of the CN text
			i = i + 3;

			// Check if it begins with a quote
			if (subjectDNName.charAt(i) == '\"')
			{
			    // Skip the quote
			    i = i + 1;

			    // Search for another quote
			    j = subjectDNName.indexOf('\"', i);
			}
			else
			{
			    // No quote, so search for comma
			    j = subjectDNName.indexOf(',', i);
			}

			if (j < 0)
			    subjectName = subjectDNName.substring(i);
			else
			    subjectName = subjectDNName.substring(i, j);
		    }
		    catch (Throwable e)
		    {
			subjectName = getMessage("security_dialog.unknown.subject");
		    }
		}


		// Extract issuer name
		String issuerDNName = prinIssuer.getName();
		String issuerName = null;

		i = issuerDNName.indexOf("O=");
		j = 0;	

		if (i < 0)
		{
		    issuerName = getMessage("security_dialog.unknown.issuer");
		}
		else
		{
		    try
		    {
			// Shift to the beginning of the O text
			i = i + 2;

			// Check if it begins with a quote
			if (issuerDNName.charAt(i) == '\"')
			{
			    // Skip the quote
			    i = i + 1;

			    // Search for another quote
			    j = issuerDNName.indexOf('\"', i);
			}
			else
			{
			    // No quote, so search for comma
			    j = issuerDNName.indexOf(',', i);
			}

			if (j < 0)
			    issuerName = issuerDNName.substring(i);
			else
			    issuerName = issuerDNName.substring(i, j);
		    }
		    catch (Throwable e)
		    {
			issuerName = getMessage("security_dialog.unknown.issuer");
		    }
		}

		// Construct dialog message
		ArrayList dialogMsgArray = new ArrayList();
		String message;
	    
		// Based on signed applet or Https site, display different messages in dialog box
		// Bug 4517290 fix
		if (getHttpsDialog())
		   message = Resources.getString("security_dialog_https.text0",
						  subjectName, issuerName);
		else
		   message = Resources.getString("security_dialog.text0",
						  subjectName, issuerName);


		dialogMsgArray.add(message);
		dialogMsgArray.add("");

		if (rootCANotValid)
		{
		    JLabel label = new JLabel(getMessage("security_dialog.rootCANotValid"));
		    label.setIcon(GeneralUtilities.getWarningIcon());
		    dialogMsgArray.add(label);
		}
		else
		{
		    JLabel label = new JLabel(getMessage("security_dialog.rootCAValid"));
		    label.setIcon(GeneralUtilities.getInfoIcon());
		    dialogMsgArray.add(label);
		}

		dialogMsgArray.add("");

		if (timeNotValid)
		{
		    JLabel label = new JLabel(getMessage("security_dialog.timeNotValid"));
		    label.setIcon(GeneralUtilities.getWarningIcon());
		    dialogMsgArray.add(label);
		}
		else
		{
		    JLabel label = new JLabel(getMessage("security_dialog.timeValid"));
		    label.setIcon(GeneralUtilities.getInfoIcon());
		    dialogMsgArray.add(label);
		}

		dialogMsgArray.add("");

		dialogMsgArray.add(Resources.getString("security_dialog.text1", 
							subjectName, subjectName));

        	JButton buttons[] = new JButton[4];

        	buttons[0] = new JButton(Resources.getString(
			"security_dialog.buttonYes"));
        	buttons[0].setMnemonic(Resources.getVKCode(
			"security_dialog.yesMnemonic"));

        	buttons[1] = new JButton(Resources.getString(
			"security_dialog.buttonNo"));
        	buttons[1].setMnemonic(Resources.getVKCode(
			"security_dialog.noMnemonic"));

        	buttons[2] = new JButton(Resources.getString(
			"security_dialog.buttonAlways"));
        	buttons[2].setMnemonic(Resources.getVKCode(
			"security_dialog.alwaysMnemonic"));

        	buttons[3] = new JButton(Resources.getString(
			"security_dialog.buttonViewCert"));
        	buttons[3].setMnemonic(Resources.getVKCode(
			"security_dialog.viewMnemonic"));
		final JButton viewCertButton = buttons[3];



                for (int index=0; index<4; index++) {
                    buttons[index].addActionListener(new ActionListener() {
                        public void actionPerformed(ActionEvent ae) {
                            Component c = (Component) ae.getSource();
                            JButton thisButton = (JButton)c;
                            if (thisButton == viewCertButton) {
				// Show the certificate dialog
				_dialog.DoModal();			
                            } else while (c != null) {
                                c = c.getParent();
                                if (c instanceof JOptionPane) {
                                    ((JOptionPane)c).setValue(thisButton);
                                    break;
                                }
                            }
                        }  
                    });
                }



		// Show dialog
	
		ret = GeneralUtilities.showOptionDialog(null, 
			dialogMsgArray.toArray(), 
			getMessage("security_dialog.caption"), 
			JOptionPane.DEFAULT_OPTION, 
			JOptionPane.PLAIN_MESSAGE, buttons, buttons[1],
			GeneralUtilities.getLockIcon());
	    }	    	

	    return ret;	    	
    
    }

  
    /**
     * Method to get an internationalized string from the Activator resource.
     */
    private static String getMessage(String key)  {
	return Resources.getString(key);
    }

    /**
     * Method returns TRUE value when it came from HTTPS site 
     */
    public boolean getHttpsDialog()
    {
	return httpsDialog;
    }

    /**
     * Method set boolean value to TRUE if it call from HTTPS site.
     */
    public void setHttpsDialog(boolean httpsDialogType)
    {
	httpsDialog = httpsDialogType;
    }
}






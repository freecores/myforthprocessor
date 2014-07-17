/*
 * @(#)TrustDeciderDialog.java	1.41 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.security;

import java.awt.FlowLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;
import java.io.IOException;
import java.net.URL;
import java.security.Principal;
import java.security.PrivilegedExceptionAction;
import java.security.PrivilegedActionException;
import java.security.cert.X509Certificate;
import java.text.MessageFormat;
import java.util.ArrayList;
import javax.swing.Icon;
import javax.swing.ImageIcon;
import javax.swing.JButton;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.LookAndFeel;
import sun.plugin.resources.ResourceHandler;
import sun.plugin.util.DialogFactory;
import sun.plugin.util.Trace;
import sun.plugin.util.PluginUIManager;



class TrustDeciderDialog implements ActionListener {

    public static final int TrustOption_GrantThisSession = 0;
    public static final int TrustOption_Deny = 1;
    public static final int TrustOption_GrantAlways = 2;


    private java.security.cert.Certificate[] certs = null;
    private int start  = 0;
    private int end = 0;
    private boolean rootCANotValid = false;
    private boolean timeNotValid = false;
    private boolean httpsDialog = false;
    private CertificateDialog dialog = null;

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
    	this.dialog = new CertificateDialog(certs, start, end);
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
	LookAndFeel lookAndFeel = null;

	try
	{
	    // Change theme
	    lookAndFeel = PluginUIManager.setTheme();

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
		    catch (IndexOutOfBoundsException e)
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
		    catch (IndexOutOfBoundsException e)
		    {
			issuerName = getMessage("security_dialog.unknown.issuer");
		    }
		}

		// Construct dialog message
		ArrayList dialogMsgArray = new ArrayList();
		MessageFormat mf = null;
	    
		// Based on signed applet or Https site, display different messages in dialog box
		// Bug 4517290 fix
		if (getHttpsDialog())
		   mf = new MessageFormat(getMessage("security_dialog_https.text0"));
		else
		   mf = new MessageFormat(getMessage("security_dialog.text0"));

		Object[] args = { subjectName, issuerName };

		dialogMsgArray.add(mf.format(args));
		dialogMsgArray.add("");

		if (rootCANotValid)
		{
		    JLabel label = new JLabel(getMessage("security_dialog.rootCANotValid"));
		    label.setIcon(getWarningIcon());
		    dialogMsgArray.add(label);
		}
		else
		{
		    JLabel label = new JLabel(getMessage("security_dialog.rootCAValid"));
		    label.setIcon(getInfoIcon());
		    dialogMsgArray.add(label);
		}

		dialogMsgArray.add("");

		if (timeNotValid)
		{
		    JLabel label = new JLabel(getMessage("security_dialog.timeNotValid"));
		    label.setIcon(getWarningIcon());
		    dialogMsgArray.add(label);
		}
		else
		{
		    JLabel label = new JLabel(getMessage("security_dialog.timeValid"));
		    label.setIcon(getInfoIcon());
		    dialogMsgArray.add(label);
		}


		mf = new MessageFormat(getMessage("security_dialog.text1"));
		Object[] args2 = { subjectName, subjectName };

		dialogMsgArray.add(mf.format(args2));

		JButton infoButton = new JButton(getMessage("security_dialog.buttonViewCert"));
		infoButton.setMnemonic(getAcceleratorKey("security_dialog.buttonViewCert")); 
		infoButton.addActionListener(this);

		JPanel buttonPanel = new JPanel();
		buttonPanel.add(infoButton);
		buttonPanel.setLayout(new FlowLayout(FlowLayout.RIGHT));
		dialogMsgArray.add(buttonPanel);

		JButton yesButton = new JButton(getMessage("security_dialog.buttonYes")); 
		JButton noButton = new JButton(getMessage("security_dialog.buttonNo")); 
		JButton alwaysButton = new JButton(getMessage("security_dialog.buttonAlways")); 
		yesButton.setMnemonic(getAcceleratorKey("security_dialog.buttonYes")); 
		noButton.setMnemonic(getAcceleratorKey("security_dialog.buttonNo")); 
		alwaysButton.setMnemonic(getAcceleratorKey("security_dialog.buttonAlways")); 
		Object[] options = { yesButton, 
				     noButton, 
				     alwaysButton };

		// Show dialog
		if (Trace.isAutomationEnabled() == false)
		{
		    ret = DialogFactory.showOptionDialog(DialogFactory.QUESTION_MESSAGE, 
				    dialogMsgArray.toArray(), getMessage("security_dialog.caption"), options, options[0]);
		}
		else
		{	// If automation is enabled
		    Trace.msgSecurityPrintln("trustdecider.automation.trustcert");
		    ret = 0;
		}
	    }	    	

	    return ret;	    	
	}
	finally
	{
	    // Restore theme
	    PluginUIManager.restoreTheme(lookAndFeel);
	}
    }

  
    /**
     * <P> Launch Certificate dialog if the "More Info" button is clicked.
     * </P>
     *
     * @param e ActionEvent object.
     */
    public void actionPerformed(ActionEvent e)  
    {
	// Show the certificate dialog
	dialog.DoModal();			
    }

    /**
     * Method to get an internationalized string from the Activator resource.
     */
    private static String getMessage(String key)  {
	return sun.plugin.resources.ResourceHandler.getMessage(key);
    }

    private static int getAcceleratorKey(String key) {
        return sun.plugin.resources.ResourceHandler.getAcceleratorKey(key);
    }
    
    /**
     * Method returns warning icon.
     */
    private Icon getWarningIcon()
    {
      return getIcon("javax/swing/plaf/metal/icons/Warn.gif");
    }

    /**
     * Method returns info icon.
     */
    private Icon getInfoIcon()
    {
      return getIcon("javax/swing/plaf/metal/icons/Inform.gif");
    }

    private Icon getIcon(final String image) {
	Icon icon = null;

	try {
	    icon = (Icon) java.security.AccessController.doPrivileged(new PrivilegedExceptionAction() {
		public Object run() throws IOException {
		    URL url = ClassLoader.getSystemResource(image);
		    return new ImageIcon(url);
		}
	    });
	} catch(PrivilegedActionException e) {
	    e.printStackTrace();
	}

	return icon;
    }

    /**
     * Method returns TRUE value when it came from HTTPS site 
     */
    private boolean getHttpsDialog()
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






/*
 * @(#)CertAllPanel.java	1.22 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.panel;

/**
 * Panel to display information about the certificates to be used by 
 * the javaplugin
 *
 * @version 	1.0 
 * @author	Dennis Gu	
 */

import javax.swing.*;
import javax.swing.border.*;
import java.awt.*;
import java.awt.event.*;
import java.awt.Dimension;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.FileInputStream;
import java.io.PrintStream;
import java.io.InputStream;
import java.util.Collection;
import java.util.Enumeration;
import java.util.TreeSet;
import java.security.*;
import java.security.cert.*;
import sun.plugin.security.*;
import sun.plugin.util.*;
import sun.misc.BASE64Encoder;
import sun.security.provider.X509Factory;

final class CertAllPanel extends ActivatorSubPanel 
			      implements ActionListener {

    /**
     * Contruct a new certificate panel
     */
    CertAllPanel(ConfigurationInfo model) {
	super(model);

	setLayout(new BoxLayout(this, BoxLayout.Y_AXIS));
	add(Box.createRigidArea(new java.awt.Dimension(1,5)));

	// Create a main cert panel
	JPanel certPanel = new JPanel();
	certPanel.setLayout(new BorderLayout());
	certPanel.setBorder(new EmptyBorder(0, 5, 0, 5));

	// Create a JList and attach to certPanel
	certList = new JList();
	certList.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
	certPanel.add(new JScrollPane(certList), BorderLayout.CENTER);
	
	// Create four button and attach ActionListener to them
	removeButton = new JButton(mh.getMessage("remove_button"));
	removeButton.setMnemonic(mh.getAcceleratorKey("remove_button")); 
	removeButton.addActionListener(this);

	exportButton = new JButton(mh.getMessage("export_button"));
	exportButton.setMnemonic(mh.getAcceleratorKey("export_button")); 
	exportButton.addActionListener(this);

	importButton = new JButton(mh.getMessage("import_button"));
	importButton.setMnemonic(mh.getAcceleratorKey("import_button")); 
	importButton.addActionListener(this);

	viewCertButton = new JButton(mh.getMessage("details_button"));
	viewCertButton.setMnemonic(mh.getAcceleratorKey("details_button")); 
	viewCertButton.addActionListener(this);

	// Create a button panel and attach four buttons to it
	JPanel buttonPanel = new JPanel();
	buttonPanel.setLayout(new FlowLayout(FlowLayout.CENTER));

	// Add four buttons
	buttonPanel.add(importButton);
	buttonPanel.add(exportButton);
	buttonPanel.add(removeButton);
	buttonPanel.add(viewCertButton);

	// Add buttons panel to certPanel
	certPanel.add(buttonPanel, BorderLayout.SOUTH);

	// Add certPanel to Main panel
	add(certPanel);

	// Call reset in ActivatorPanel, not here.
	//reset();
    }

    /**
     * Enable / Disable the UI depending on the number of outstanding 
     * certificates. Changed certificate from active set into inactive
     * set if "Remove" button is clicked.
     * Changed certificate from active set into inactive import
     * set if "Import" button is clicked.
     */
    public void actionPerformed(ActionEvent e) 
    {
	int i = certList.getSelectedIndex();
	
	//if (i < 0)
	//    return;
	    	    
	// Changed the certificate from the active set into the 
	// inactive set. This is for removing the certificate from
	// the store when the user clicks on Apply.
	//
	String alias = (String) certList.getSelectedValue();

	if (e.getSource()==removeButton) 
	{	    		
	    // Changed the certificate from the active set into the 
	    // inactive set. This is for removing the certificate from
	    // the store when the user clicks on Apply.
	    //
	    if (getRadioPos() == mh.getMessage("SignedApplet_value"))
    	       model.deactivateCertificate(alias);
	    else
	       model.deactivateHttpsCertificate(alias);

	    reset();
	}
	else if (e.getSource() == viewCertButton) 
	{
	    X509Certificate myCert = null;

	    if (getRadioPos() == mh.getMessage("SignedApplet_value"))
		myCert = (X509Certificate) model.getCertificate(alias);
	    else if (getRadioPos() == mh.getMessage("SecureSite_value")) 
		myCert = (X509Certificate) model.getHttpsCertificate(alias);
	    else if (getRadioPos() == mh.getMessage("SignerCA_value")) 
		myCert = (X509Certificate) model.getRootCACertificate(alias);
	    else if (getRadioPos() == mh.getMessage("SecureSiteCA_value")) 
		myCert = (X509Certificate) model.getHttpsRootCACertificate(alias);

	    X509Certificate[] certs = new X509Certificate[] {myCert};

	    // Make sure the certificate has been stored or in the import HashMap.
	    if (certs.length > 0 && certs[0] instanceof X509Certificate)
	    {
            	CertificateDialog dialog = new CertificateDialog(this, certs, 0, certs.length);
    	    	dialog.DoModal();			
	    }
	    else 
	    {
		// The certificate you are trying to view is still in Import HashMap
		X509Certificate impCert = null;

		if (getRadioPos() == mh.getMessage("SignedApplet_value"))
		   impCert = model.getImpCertificate(alias);
		else
		   impCert = model.getImpHttpsCertificate(alias);
	
	        X509Certificate[] impCerts = new X509Certificate[] {impCert};
            	CertificateDialog impDialog = new CertificateDialog(this, impCerts, 0, impCerts.length);
    	    	impDialog.DoModal();			
	    }
	}
	else if (e.getSource() == importButton)
	{
	    // Popup FileChooser
	    JFileChooser jfc = new JFileChooser();

	    // Set filter for File Chooser Dialog Box
	    CertFileFilter impFilter = new CertFileFilter(); 
	    impFilter.addExtension("csr");
	    impFilter.addExtension("p12");
	    impFilter.setDescription("Certificate Files");
	    jfc.setFileFilter(impFilter);
 
	    jfc.setFileSelectionMode(JFileChooser.FILES_ONLY);
	    jfc.setDialogType(JFileChooser.OPEN_DIALOG);
	    jfc.setMultiSelectionEnabled(false);
	    int result = jfc.showOpenDialog(this);
	    if (result == JFileChooser.APPROVE_OPTION)
	    {
		File f = jfc.getSelectedFile();
		if (f == null) return;

		try
		{
		    InputStream inStream = System.in;
		    inStream = new FileInputStream(f);

		    // Import certificate from file to deployment.certs
		    boolean impStatus = false;
		    impStatus = importCertificate(inStream);
			
		    // Check if it is belong to PKCS#12 format
		    if (!impStatus)
		    {
			// Create another inputStream for PKCS#12 foramt
		        InputStream inP12Stream = System.in;
		        inP12Stream = new FileInputStream(f);
		        importPKCS12Certificate(inP12Stream);
		    }
		}
		catch(Throwable e2)
		{
		    DialogFactory.showExceptionDialog(this, e2,  getMessage("dialog.import.file.text"), 
						      getMessage("dialog.import.error.caption"));
		}
	    }

	    reset();
	}
	else if (e.getSource() == exportButton)
	{
	    X509Certificate cert = null;

	    if (getRadioPos() == mh.getMessage("SignedApplet_value"))
		cert = (X509Certificate) model.getCertificate(alias);
	    else if (getRadioPos() == mh.getMessage("SecureSite_value")) 
		cert = (X509Certificate) model.getHttpsCertificate(alias);
	    else if (getRadioPos() == mh.getMessage("SignerCA_value")) 
		cert = (X509Certificate) model.getRootCACertificate(alias);
	    else if (getRadioPos() == mh.getMessage("SecureSiteCA_value")) 
		cert = (X509Certificate) model.getHttpsRootCACertificate(alias);

	    // Make sure the certificate has been stored, if not, get from import table.
	    if (!(cert instanceof X509Certificate))
	    {
		// The certificate you are trying to export is still in Import HashMap
		if (getRadioPos() == mh.getMessage("SignedApplet_value"))
		   cert = model.getImpCertificate(alias);
		else
		   cert = model.getImpHttpsCertificate(alias);

	    } //not saved certificate 

	    if (cert != null)
	    {
	      // Popup FileChooser
	      JFileChooser jfc = new JFileChooser();
	      jfc.setFileSelectionMode(JFileChooser.FILES_ONLY);
	      jfc.setDialogType(JFileChooser.SAVE_DIALOG);
	      jfc.setMultiSelectionEnabled(false);
	      int result = jfc.showSaveDialog(this);
	      if (result == JFileChooser.APPROVE_OPTION)
	      {
		File f = jfc.getSelectedFile();
		if (f == null) return;
		PrintStream ps = null;
		try {
		    ps = new PrintStream(new BufferedOutputStream(new FileOutputStream(f)));
		    exportCertificate(cert, ps);
		}
		catch(Throwable e2) 
		{
		    DialogFactory.showExceptionDialog(this, e2, getMessage("dialog.export.text"), 
						      getMessage("dialog.export.error.caption"));
		}
		finally {
		    if (ps != null)
			ps.close();
		}
	      }
	    } // Cert not null
	    else {
		DialogFactory.showErrorDialog(this, getMessage("dialog.export.text"), getMessage("dialog.export.error.caption"));
	    }
	}
    }

    /* Export a certificate to a file */
    void exportCertificate(X509Certificate cert, PrintStream ps)
    {
	BASE64Encoder encoder = new BASE64Encoder();
	ps.println(X509Factory.BEGIN_CERT);
	try
	{
	    encoder.encodeBuffer(cert.getEncoded(), ps);
	}
	catch (Throwable e)
	{
	    Trace.printException(this, e);
	}
	ps.println(X509Factory.END_CERT);
    }

    /* Import a PKCS#12 format certificate and adds it to the list of trusted certificates */
    void importPKCS12Certificate(InputStream is)
    {
	KeyStore  myKeySto = null;
	// passord
	char[] password = null;
	
	try
	{
	  myKeySto = KeyStore.getInstance("PKCS12");

	  // Pop up password dialog box
          Object dialogMsg = getMessage("dialog.password.text");
          JPasswordField passwordField = new JPasswordField();

          Object[] msgs = new Object[2];
          msgs[0] = new JLabel(dialogMsg.toString());
          msgs[1] = passwordField;

          JButton okButton = new JButton(getMessage("dialog.password.okButton"));
          JButton cancelButton = new JButton(getMessage("dialog.password.cancelButton"));

          String title = getMessage("dialog.password.caption");
          Object[] options = {okButton, cancelButton};
          int selectValue = DialogFactory.showOptionDialog(this, msgs, title, options, options[0]);

	  // for security purpose, DO NOT put password into String. Reset password as soon as 
	  // possible.
	  password = passwordField.getPassword();

	  // User click OK button
          if (selectValue == 0)
          {
	     // Load KeyStore based on the password
	     myKeySto.load(is,password);

	     // Get Alias list from KeyStore.
	     Enumeration aliasList = myKeySto.aliases();

	     while (aliasList.hasMoreElements())
	     {
			X509Certificate cert = null;

			// Get Certificate based on the alias name
			String certAlias = (String)aliasList.nextElement();
			cert = (X509Certificate)myKeySto.getCertificate(certAlias);

			// Add to import list based on radio button selection
				if (getRadioPos() == mh.getMessage("SignedApplet_value"))
    				   model.deactivateImpCertificate(cert);
				else if (getRadioPos() == mh.getMessage("SecureSite_value"))
    				   model.deactivateImpHttpsCertificate(cert);
	     }
	  } // OK button
	}
	catch(Throwable e)
	{
	    // Show up Error dialog box if the user enter wrong password
	    // Avoid to convert password array into String - security reason
	    String uninitializedValue =  "uninitializedValue";
	    if (!compareCharArray(password, uninitializedValue.toCharArray()))
	    {
			String errorMsg = getMessage("dialog.import.password.text");
					String errorTitle = getMessage("dialog.import.error.caption");
	    		DialogFactory.showExceptionDialog(this, e, errorMsg, errorTitle);
	    }
	}
	finally {
		// Reset password
		if(password != null) {
			java.util.Arrays.fill(password, ' ');
		}
	}
    }

	private boolean compareCharArray(char[] c1, char[] c2) {
		if(c1.length != c2.length)
			return false;
		
		for(int index = 0; index < c1.length; index ++) {
			if(c1[index] != c2[index]) {
				return false;
			}
		}

		return true;
	}

    /* Import a certificate and adds it to the list of trusted certificates */
    boolean importCertificate(InputStream is)
    {
	CertificateFactory cf = null;
	X509Certificate cert = null;

	try
	{
	    cf = CertificateFactory.getInstance("X.509");
	    cert = (X509Certificate)cf.generateCertificate(is);

	    // Changed the certificate from the active set into the 
	    // inactive Import set. This is for import the certificate from
	    // the store when the user clicks on Apply.
	    //
	    if (getRadioPos() == mh.getMessage("SignedApplet_value"))
    	       model.deactivateImpCertificate(cert);
	    else if (getRadioPos() == mh.getMessage("SecureSite_value"))
    	       model.deactivateImpHttpsCertificate(cert);
	}
	catch (CertificateParsingException cpe)
	{
	    // It is PKCS#12 format.
	    return false;
	}
	catch (CertificateException e)
	{
	    // Wrong format of the selected file
	    DialogFactory.showExceptionDialog(this, e, getMessage("dialog.import.format.text"), getMessage("dialog.import.error.caption"));
	}

	return true;
    }

    /**
     * Refill the list box with the active set of certificates. Disable
     * UI if necessary.
     */
    void reset() {

	Collection certs = null;

	if ( getRadioPos() == mh.getMessage("SignedApplet_value"))
	   certs = model.getCertificateAliases();
	else if ( getRadioPos() == mh.getMessage("SecureSite_value"))
	   certs = model.getHttpsCertificateAliases();
	else if ( getRadioPos() == mh.getMessage("SignerCA_value"))
	   certs = model.getRootCACertificateAliases();
	else if ( getRadioPos() == mh.getMessage("SecureSiteCA_value"))
	   certs = model.getHttpsRootCACertAliases();

	if (certs == null || certs.size() == 0)
	{
	    certList.setListData(new String[0]);
	}
	else
	{
	    // Construct a TreeSet object to sort the certificate list
            TreeSet tsCerts = new TreeSet(certs);

	    // Fill up list box with the sorted certificates
	    certList.setListData(tsCerts.toArray());
	}

	// Make sure we do have content in certificate
	boolean enable = (certs != null && certs.size() > 0);

	// For Root CA and Web Sites, only enable View button
	if (getRadioPos() == mh.getMessage("SignerCA_value") || 
		getRadioPos() == mh.getMessage("SecureSiteCA_value"))
	{
	    setEnabled(removeButton, false);
	    setEnabled(exportButton, false);
	    setEnabled(importButton, false);
	    setEnabled(viewCertButton, enable);
	}
	else
	{
	    setEnabled(removeButton, enable);
	    setEnabled(exportButton, enable);
	    setEnabled(importButton, true);
	    setEnabled(viewCertButton, enable);
	}

	if (enable)
	    certList.setSelectedIndex(0);
    }


    /**
     * Enable/Disabled a component
     *
     * @param field the component to enable/disable
     * @param b true to enable to text field, false to disable it
     */
    private void setEnabled(JComponent field, boolean b) {
	field.setEnabled(b);
	field.repaint();
    }

    void setRadioPos(String inStr)
    {
	radioPos = inStr;
    }

    String getRadioPos()
    {
	if (radioPos == null)
	   return mh.getMessage("SignedApplet_value");
	else
	   return radioPos;
    }

    /**
     * Method to get an internationalized string from the Activator resource.
     */
    private static String getMessage(String key)
    {
        return mh.getMessage(key);
    }

    private JList   certList;
    private JButton removeButton;
    private JButton viewCertButton;
    private JButton exportButton;
    private JButton importButton;
    private String  radioPos;

    private static MessageHandler mh = new MessageHandler("cert");
}

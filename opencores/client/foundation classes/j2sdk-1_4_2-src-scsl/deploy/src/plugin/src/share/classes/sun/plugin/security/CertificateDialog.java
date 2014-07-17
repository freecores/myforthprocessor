/*
 * @(#)CertificateDialog.java	1.26 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.security;

import java.awt.BorderLayout;
import java.awt.Component;
import java.awt.Dimension;
import java.awt.FlowLayout;
import java.awt.Window;
import java.security.cert.Certificate;
import java.security.cert.X509Certificate;
import java.security.Principal;
import java.text.MessageFormat;
import javax.swing.BorderFactory;
import javax.swing.JButton;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTable;
import javax.swing.JTree;
import javax.swing.JTextArea;
import javax.swing.JViewport;
import javax.swing.ListSelectionModel;
import javax.swing.LookAndFeel;
import javax.swing.UIManager;
import javax.swing.border.Border;
import javax.swing.border.TitledBorder;
import javax.swing.event.ListSelectionEvent;
import javax.swing.event.ListSelectionListener;
import javax.swing.event.TreeSelectionEvent;
import javax.swing.event.TreeSelectionListener;
import javax.swing.table.DefaultTableModel;
import javax.swing.tree.TreeSelectionModel;
import javax.swing.tree.DefaultMutableTreeNode;
import sun.security.x509.SerialNumber;
import sun.misc.HexDumpEncoder;
import sun.plugin.util.DialogFactory;
import sun.plugin.util.Trace;
import sun.plugin.util.PluginUIManager;


public class CertificateDialog
{
    private Certificate[] certs = null;
    private int start = 0;
    private int end = 0;
    private Component parentComponent;

    JTree certChainTree = null;
    final JTable certInfoTable = new JTable();
    final JTextArea textArea = new JTextArea();

    Object[] msg, options;

    /**
     * <P> Constructor for CertificateDialog.
     * </P>
     *
     * @param certs Array of certificates
     * @param index Index of the certificate array to show
     */
    public CertificateDialog(Certificate[] certs, int start, int end)
    {
	this(null, certs, start, end);
    }

    /**
     * <P> Constructor for CertificateDialog.
     * </P>
     *
     * @param certs Array of certificates
     * @param index Index of the certificate array to show
     */
    public CertificateDialog(Component parentComponent, Certificate[] certs, int start, int end)
    {
	assert (certs != null && start >= 0 && end >= 0 && start <= end);

	LookAndFeel lookAndFeel = null;

	try
	{
	    // Change theme
	    lookAndFeel = PluginUIManager.setTheme();

	    this.certs = certs;
	    this.start = start;
	    this.end = end;
	    this.parentComponent = parentComponent;

	    /*
	      Swing components lazy loads some of the classes. Since classloader will be locked
	      when certificate dialog is shown, it is necessary to load those classes before
	      hand to avoid the deadlock.
	      
	      UIManager.getBorder("TableHeader.cellBorder") avoids lazy loading of 
	      classes which was the cause for #4491501
	    */
	    UIManager.getBorder("TableHeader.cellBorder");
	    UIManager.getBorder("Table.focusCellHighlightBorder");
	    UIManager.getColor("Table.focusCellForeground");
	    UIManager.getColor("Table.focusCellBackground");
	    
	    JTable dummy = new JTable();
	    dummy.getDefaultRenderer(String.class);
	    dummy.getDefaultRenderer(Object.class);

	    if (certs.length > start && certs[start] instanceof X509Certificate)
	    {
		// Create borders
		Border etchBorder = BorderFactory.createEtchedBorder();

		// Build cert chain tree view
		//
		certChainTree = buildCertChainTree(certs, start, end);
		certChainTree.addTreeSelectionListener(new TreeSelectionListener() 
		{
		    public void valueChanged(TreeSelectionEvent e) 
		    {
			DefaultMutableTreeNode node = (DefaultMutableTreeNode) certChainTree.getLastSelectedPathComponent();

			if (node == null)
			    return;

			CertificateInfo certInfo = (CertificateInfo) node.getUserObject();

			// Show certificate in the cert info panel
			showCertificateInfo(certInfo.getCertificate());
		    }
		});	    

		// Build cert info table
		showCertificateInfo((X509Certificate) certs[start]);

		// Select one row at a time in cert info table
		//
		certInfoTable.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);

		// Hook up selection model with cert info table
		//
		ListSelectionModel rowSM = certInfoTable.getSelectionModel();
		rowSM.addListSelectionListener(new ListSelectionListener() {
		    public void valueChanged(ListSelectionEvent e)
		    {
			int row = certInfoTable.getSelectedRow();

			if (row == -1)
			    return;

			String value = (String) certInfoTable.getValueAt(row, 1);

			// Update text area whenever selection in JTree changes
			textArea.setText(value);
			textArea.repaint();
		    }   	    
		});

		// Create JTextArea for rhs	    
		//
		textArea.setLineWrap(false);
		textArea.setEditable(false);
		textArea.setRows(10);
		textArea.setColumns(40);


		// Select last row by default
		certInfoTable.setRowSelectionInterval(6, 6);

		// Create cert info panel
		//
		JPanel panelInfo = new JPanel();
		panelInfo.setLayout(new BorderLayout());

		// Workaround JTable problem - height is always higher than needed	
		//							    
		Dimension dim = certInfoTable.getPreferredScrollableViewportSize();
		dim.setSize(dim.getWidth(), 120.0);
		certInfoTable.setPreferredScrollableViewportSize(dim);

		JScrollPane scrollPane = new JScrollPane(certInfoTable);
		scrollPane.setBorder(BorderFactory.createCompoundBorder(BorderFactory.createEmptyBorder(0, 0, 5, 0),
									scrollPane.getBorder()));

		panelInfo.add(scrollPane, BorderLayout.CENTER);
		panelInfo.add(new JScrollPane(textArea), BorderLayout.SOUTH);
		panelInfo.setBorder(BorderFactory.createEmptyBorder(0, 5, 0, 0));

		// Add tree to the panel
		//	    
		JPanel panel = new JPanel();
		panel.setLayout(new BorderLayout());

		// Workaround JTree problem - width is always longer than needed	
		//							    
		dim = certChainTree.getPreferredScrollableViewportSize();
		dim.setSize(200.0, 100.0);
		scrollPane = new JScrollPane(certChainTree);
		scrollPane.setPreferredSize(dim);

		panel.add(scrollPane, BorderLayout.WEST);	
		panel.add(panelInfo, BorderLayout.EAST);  	

		msg = new Object[] { panel };
        
		JButton closeButton = new JButton(getMessage("cert_dialog.close")); 
		closeButton.setMnemonic(getAcceleratorKey("cert_dialog.close")); 

		options = new Object[] { closeButton }; 
	    }
	}
	finally
	{
	    // Restore theme
	    PluginUIManager.restoreTheme(lookAndFeel);
	}
   }


    /**
     * <P> Show CertificateDialog.
     * </P>
     */
    public void DoModal()  
    {
	// Show dialog
	if (Trace.isAutomationEnabled() == false)
	{
	    DialogFactory.showOptionDialog(parentComponent, msg, getMessage("cert_dialog.caption"), options, options[0]);
	}
    }

    /**
     * Break down DN string into multi-line
     */
    private String formatDNString(String dnString)
    {
	int len = dnString.length();
	int last = 0;
	boolean inQuote = false;

	StringBuffer buffer = new StringBuffer();

	for (int i=0; i < len; i++)
	{
	    char ch = dnString.charAt(i);

	    // Check if we are in quote
	    if (ch == '\"' || ch == '\'')
		inQuote = !inQuote;

	    if (ch == ',' && inQuote == false)
		buffer.append(",\n");
	    else
		buffer.append(ch);
	}

	return buffer.toString();
    }

    /**
     * Method to reflect certificate chain in the tree view
     */
    private JTree buildCertChainTree(Certificate cert[], int start, int end)
    {
	DefaultMutableTreeNode root = null;
	DefaultMutableTreeNode currentNode = null;
	
	for (int i=start; i < cert.length && i < end; i++)
	{
	    DefaultMutableTreeNode childNode = new DefaultMutableTreeNode(new CertificateInfo((X509Certificate) cert[i]));

	    if (root == null)
	    {
		root = childNode;
		currentNode = childNode;
	    }
	    else
	    {
		currentNode.add(childNode);
		currentNode = childNode;
	    }
	}
	
	JTree tree = new JTree(root);

	// Allow single node selection only
	tree.getSelectionModel().setSelectionMode(TreeSelectionModel.SINGLE_TREE_SELECTION);

	// Display angled line
	tree.putClientProperty("JTree.lineStyle", "Angled");

	return tree;
    }


    /**
     * Method to reflect table data based on the certificate
     */
    private void showCertificateInfo(X509Certificate cert)
    {
	String certVersion = "V" + cert.getVersion();

	SerialNumber serial = new SerialNumber(cert.getSerialNumber());
	String certSerialNumber = "[" + serial.getNumber() + "]";

	String certSigAlg = "[" + cert.getSigAlgName() + "]";
	String certIssuer = formatDNString(cert.getIssuerDN().toString());
	String certValidity = "[From: " + cert.getNotBefore() + ",\n To: " + cert.getNotAfter() + "]";
	String certSubject = formatDNString(cert.getSubjectDN().toString());

	HexDumpEncoder encoder = new HexDumpEncoder();
	String certSig = encoder.encodeBuffer(cert.getSignature());

	Object[][] data = {
	    { getMessage("cert_dialog.field.Version"), certVersion },
	    { getMessage("cert_dialog.field.SerialNumber"), certSerialNumber },
	    { getMessage("cert_dialog.field.SignatureAlg"), certSigAlg },
	    { getMessage("cert_dialog.field.Issuer"), certIssuer },
	    { getMessage("cert_dialog.field.Validity"), certValidity },
	    { getMessage("cert_dialog.field.Subject"), certSubject },
	    { getMessage("cert_dialog.field.Signature"), certSig } };

	String[] columnNames = { getMessage("cert_dialog.field"), 
				 getMessage("cert_dialog.value") };
 

	certInfoTable.setModel(new DefaultTableModel(data, columnNames) {
	    public boolean isCellEditable(int row, int col) { return false; }	
	});

	// Select last row by default
	certInfoTable.setRowSelectionInterval(6, 6);
	certInfoTable.repaint();
	textArea.repaint();
    }


    /**
     * Method to get an internationalized string from the Activator resource.
     */
    private static String getMessage(String key)
    {
	return sun.plugin.resources.ResourceHandler.getMessage(key);
    }

    private static int getAcceleratorKey(String key) { 
        return sun.plugin.resources.ResourceHandler.getAcceleratorKey(key); 
    } 
}


class CertificateInfo 
{
    X509Certificate cert;

    CertificateInfo(X509Certificate cert)
    {
	this.cert = cert;
    }

    public X509Certificate getCertificate()
    {
	return cert;
    }

    /**
     * Extrace CN from DN in the certificate.
     *
     * @param cert X509 certificate
     * @return CN
     */
    private String extractAliasName(X509Certificate cert)
    {
	String subjectName = getMessage("security_dialog.unknown.subject");
	String issuerName = getMessage("security_dialog.unknown.issuer");

	// Extract CN from the DN for each certificate
	try 
	{
       	    Principal principal = cert.getSubjectDN();
       	    Principal principalIssuer = cert.getIssuerDN();

	    // Extract subject name
	    String subjectDNName = principal.getName();
	    String issuerDNName = principalIssuer.getName();

	    // Extract subject name
	    subjectName = extractFromQuote(subjectDNName, "CN=");

	    if (subjectName == null)
		subjectName = extractFromQuote(subjectDNName, "O=");

	    if (subjectName == null)
		subjectName = getMessage("security_dialog.unknown.subject");

	    // Extract issuer name
	    issuerName = extractFromQuote(issuerDNName, "CN=");

	    if (issuerName == null)
		issuerName = extractFromQuote(issuerDNName, "O=");

	    if (issuerName == null)
		issuerName = getMessage("security_dialog.unknown.issuer");
	}
	catch (Exception e) 
	{
	    Trace.printException(e);
	}

	// Add Subject name and Issuer name in the return string
	MessageFormat mf = new MessageFormat(getMessage("security_dialog.certShowName"));
	Object[] args = {subjectName, issuerName};
	return mf.format(args);
    }

    /** 
     * Extract from quote
     */
    private String extractFromQuote(String s, String prefix)
    {
	if ( s == null)
	    return null;

	// Search for issuer name
	//
	int x = s.indexOf(prefix);
	int y = 0;

	if (x >= 0)
	{
	    x = x + prefix.length();

	    // Search for quote
	    if (s.charAt(x) == '\"')
	    {
		// if quote is found, search another quote

		// skip the first quote
		x = x + 1;
		
		y = s.indexOf('\"', x);
	    }
	    else
	    {
		// quote is not found, search for comma
		y = s.indexOf(',', x);
	    }

	    if (y < 0)
		return s.substring(x);			
	    else
		return s.substring(x, y);			
	}
	else
	{
	    // No match
	    return null;
	}
    }

    /**
     * Method to get an internationalized string from the Activator resource.
     */
    private static String getMessage(String key)
    {
	return sun.plugin.resources.ResourceHandler.getMessage(key);
    }

    public String toString()
    {
	return extractAliasName((X509Certificate) cert);
    }
}

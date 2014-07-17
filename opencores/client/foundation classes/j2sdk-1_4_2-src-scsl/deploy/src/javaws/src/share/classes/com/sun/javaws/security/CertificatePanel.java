/*
 * @(#)CertificatePanel.java	1.34 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.security;

import com.sun.javaws.*;
import com.sun.javaws.ui.general.*;
import java.awt.*;
import java.awt.event.*;
import java.security.cert.*;
import java.text.*;
import java.util.*;
import javax.swing.*;
import javax.swing.event.*;
import javax.swing.table.*;
import javax.swing.tree.*;
import javax.swing.border.*;
import java.security.Key;
import java.security.UnrecoverableKeyException;
import java.security.PublicKey;
import java.security.PrivateKey;
import java.security.Principal;
import java.security.KeyStore;
import java.security.KeyStoreException;
import com.sun.javaws.debug.Debug;
import java.io.*;
import com.sun.javaws.util.BASE64Encoder;

/**
 * @version 1.34 01/23/03
 */
public class CertificatePanel extends PropertyPanel implements ChangeListener {
    private JSplitPane _spane; 
    private AbstractController _controller;
    private CertificateInfo[][] _certificates;
    private Frame _frame;
    private Vector _aliases = new Vector();
    private java.security.cert.Certificate[][] _certs;
    private boolean _allowAliasSelection = false;
    private boolean _standAlone = false;
    private boolean _showImportExport = false;
    private int _selectedIndex = 0;
    JComboBox _aliasList = null;
    private final String DEFAULT_CERTIFICATE_ALIAS = "mykey";
    private AncestorListener _aliasAncestorListener = new aliasComboAncestorListener();
    private ActionListener _aliasActionListener = new aliasComboActionListener();
    
    public CertificatePanel() {
	this(true, false, true);
    }
    
    public CertificatePanel(boolean allowAliasSelection, boolean standAlone, boolean showImportExport) {
	_allowAliasSelection = allowAliasSelection;
	_standAlone = standAlone;
	_showImportExport = showImportExport;
    }
    
    public static void showCertificateDialog(Frame parent,
					     CertificateInfo[] certs,
					     int trustedIndex) {
        final JDialog frame = GeneralUtilities.createDialog(parent, 
	    Resources.getString ("certificateDialog.title"), true);
        CertificatePanel cp = new CertificatePanel(false, true, false);
        cp.setFrame(parent);
	int actualLength = -1;
	if (trustedIndex < 0) {
	    actualLength = certs.length;
	} else {
	    actualLength = Math.min(trustedIndex+1, certs.length);
	}
	cp._certificates = new CertificateInfo[1][actualLength];
	System.arraycopy(certs, 0, cp._certificates[0], 0, actualLength);
	cp._selectedIndex = 0;
	cp.start();
        cp.setVisible(true);
        frame.getContentPane().add(cp.getComponent(), BorderLayout.CENTER);
        Box b = Box.createHorizontalBox();
        b.add(Box.createHorizontalGlue());
        JButton button = new JButton(
	    Resources.getString("certificateDialog.okButton"));
        button.addActionListener(new ActionListener() {
		    public void actionPerformed(ActionEvent ae) {
			frame.setVisible(false);
		    }
		});
        b.add(button);
        b.add(Box.createHorizontalStrut(5));
        Box vBox = Box.createVerticalBox();
        vBox.add(Box.createVerticalStrut(5));
        vBox.add(b);
        vBox.add(Box.createVerticalStrut(5));
        frame.getContentPane().add(vBox, BorderLayout.SOUTH);
        frame.pack();
        frame.setLocationRelativeTo(null);
	// fix for 4726525: Certificate detail panel should have focus on OK button
	// for some reason requestFocus only works on 1.4+ here
	button.requestDefaultFocus();
	button.requestFocus();
	frame.getRootPane().setDefaultButton(button);
        frame.show();
    }
    
    protected void createPropertyControls() {
	_controller = new AbstractController() {
	    protected Subcontroller createSubcontroller(int index) {
		switch (index) {
		    case 0:
			return new GeneralPanel();
		    default:
			return new CertPathPanel();
		}
	    }
	};
	
	updateData(false);
	
	createComponents();
	
	setSelectedTab(0);
    }
    
    private void updateData(boolean createNewVector) {
	
	if (_standAlone) return;
	KeyStore ks = null;
	Enumeration aliasEnum = null;
	if (createNewVector) _aliases = new Vector();
	
	try {
	    ks = KeyStoreManager.getKeyStore();
	    if (ks == null) {
		return;
	    }
	    aliasEnum = ks.aliases();
	} catch (KeyStoreException kse) {
	    kse.printStackTrace();
	}
	
	while (aliasEnum.hasMoreElements()) {
	    String s = (String)aliasEnum.nextElement();
	    _aliases.addElement(s);
	}

	Collections.sort(_aliases);
	
	_certs = new java.security.cert.Certificate[_aliases.size()][];
	_certificates = new CertificateInfo[_aliases.size()][];
	
	for (int j = 0; j < _aliases.size(); j++) {
	    String s = (String)_aliases.get(j);
	    try {
	        _certs[j] = (java.security.cert.Certificate [])ks.getCertificateChain(s);
		if (_certs[j] != null) {
		    _certificates[j] = new CertificateInfo[_certs[j].length];
		    for (int k = 0; k < _certs[j].length; k++) {
			if (_certs[j][k] == null) continue;
			try {
			    _certificates[j][k] =
			        CertificateInfo.getCertificateInfo((java.security.cert.Certificate)_certs[j][k]);
			} catch (CertificateException ce) {
			    Debug.ignoredException(ce);
			}
		    }
		}
	    } catch (KeyStoreException kse) {
		Debug.ignoredException(kse);
	    }
	    // if we're still null then its a ROOT CERT ENTRY or KEY ENTRY
	    // and we need to use getCertificateInfo(Certificate c)
	    if (_certs[j] == null) {
		_certs[j] = new java.security.cert.Certificate[1];
		try {
		    // codeguide complains unless I explicitly cast here (?)
		    _certs[j][0] = (java.security.cert.Certificate)ks.getCertificate(s);
		    if (_certs[j][0] != null) {
			_certificates[j] = new CertificateInfo[1];
			try {
			    _certificates[j][0] =
			        CertificateInfo.getCertificateInfo(_certs[j][0]);
			} catch (CertificateException ce) {
			    Debug.ignoredException(ce);
			}
		    }
		} catch (KeyStoreException kse) {
		    Debug.ignoredException(kse);
		}
	    }
	}
	// fix for 4655183
	if (createNewVector) setVisible(true);
    }
    
    private Vector getAliases() {
	return _aliases;
    }
    
    private void setFrame(Frame frame) {
        _frame = frame;
    }
    
    private Frame getFrame() {
        return _frame;
    }
    
    public Component getComponent() {
        return _spane;
    }
    
    private int getSelectedIndex() {
	return _selectedIndex;
	// return (_aliasList == null ? 0 : _aliasList.getSelectedIndex());
    }

    public void setVisible(boolean b) {
	setVisible(b, _selectedIndex);
    }
    
    public void setVisible(boolean b, int index) {
	_selectedIndex = index;
	// Restart the current one so that it reloads as necessary
	if ( _controller.getSubcontroller(0) != null) { 
	    _controller.getSubcontroller(0).stop(); 
	    _controller.getSubcontroller(0).start(); 
	}
	if ( _controller.getSubcontroller(1) != null) {
	    _controller.getSubcontroller(1).stop();
	    _controller.getSubcontroller(1).start();
	}
	
    }
    
    
    private CertificateInfo getCertificate(int index) {
        return (_certificates != null && _certificates[index] != null) ? _certificates[index][0] : null;
    }
    
    private CertificateInfo getCertificate() {
	return getCertificate(0);
    }
    
    private CertificateInfo[] getCertificates() {
        return getCertificates(0);
    }
    
    private CertificateInfo[] getCertificates(int index) {
	if (index >= 0 && _certificates != null) {
            return _certificates[index];
	} else {
	    return null;
	}
    }
    
    public void stateChanged(ChangeEvent e) {
    }
    
    private void createComponents() {
      _spane = new JSplitPane(); 
                                      
      _controller.setActiveSubcontrollerIndex(0); 
      Component left = _controller.getActiveSubcontroller().getComponent(); 
      _spane.setLeftComponent(left); 
      _controller.setActiveSubcontrollerIndex(1); 
      Component right = _controller.getActiveSubcontroller().getComponent(); 
      _spane.setRightComponent(right); 
      
      _spane.setDividerLocation(225); 
      
    }
    
    private void setSelectedTab(int tab) {
	if (tab != _controller.getActiveSubcontrollerIndex()) {
	    Subcontroller subcontroller = _controller.getSubcontroller(tab);
	    _controller.setActiveSubcontrollerIndex(tab);
	    if (subcontroller == null) {
		  _controller.setActiveSubcontrollerIndex(0); 
		  
		  Component left = _controller.getActiveSubcontroller().getComponent(); 
		  _spane.setLeftComponent(left); 
		  _controller.setActiveSubcontrollerIndex(1); 
		  
		  Component right = _controller.getActiveSubcontroller().getComponent(); 
		  _spane.setRightComponent(right); 
		  
	    }
	}
    }
    
    
    class aliasComboActionListener implements ActionListener {
	public void actionPerformed(ActionEvent e) {
	    JComboBox cb = (JComboBox)e.getSource();
	    int index = cb.getSelectedIndex();
	    if (index != -1 && cb.getItemAt(index) != null ) {
		CertificatePanel.this.setVisible(true, index);
	    }
	}
    }
    
    class aliasComboAncestorListener implements AncestorListener {
	public void ancestorAdded(AncestorEvent ae) {
	    if (ae.getSource() == _aliasList && _selectedIndex == -1) {
		_aliasList.setSelectedIndex(_selectedIndex);	
	    }
	}
	public void ancestorRemoved(AncestorEvent ae) {}
	public void ancestorMoved(AncestorEvent ae) {}
    }
    
    private class GeneralPanel extends JPanel implements Subcontroller {
        private boolean _createdComponents;
        private JLabel _issuedToLabel;
        private JLabel _issuerLabel;
        private JLabel _dateLabel;
        private DefaultTableModel _tableModel;
	
        public void start() {
	    if (!_createdComponents) {
		_createdComponents = true;
		createPropertyControls();
	    }
	    
	    CertificateInfo ci = getCertificate(_selectedIndex);
	    
	    if (ci != null) {
		updateTable(ci);
		updateWidgets(ci);
	    }
        }
	
        public void stop() {
        }
	
        public java.awt.Component getComponent() {
	    return this;
        }
	
        public void apply() {}
        public void revert() {}
	
	private void createPropertyControls() {
	    // Big JLabel saying Certificate Information
	    // JLabel with Issued To:
	    // JLabel with Issued By:
	    // JLabel with Valid Dates: Valid from xxx to xxx
	    Dimension minSize = new Dimension(0,0);
	    setMinimumSize(minSize);

	    setLayout(new GridBagLayout());
	    JLabel label;
	    label = new JLabel(Resources.getString
			           ("certificatePanel.general.label"));
	    label.setFont(label.getFont().deriveFont
	         (Font.BOLD, (float)(label.getFont().getSize() +4)));
	    label.setForeground(Color.black);
	    label.setBorder(new com.sun.javaws.ui.general.LineBorder
	    	(Color.black, 1, 1 << SwingConstants.BOTTOM));	    
	    Insets insets = new Insets(2, 2, 2, 2);
	    GridBagHelper.addGridBagComponent(GeneralPanel.this, label,
					      0, 1, 2, 1, 0, 0,
					      GridBagConstraints.WEST,
					      GridBagConstraints.NONE,
					      insets, 0, 0);
	    
	    insets.top = 0;
	    
	    if (_allowAliasSelection) {
		_aliasList = new JComboBox();
		_aliasList.addActionListener(_aliasActionListener);
		_aliasList.addAncestorListener(_aliasAncestorListener);
		
		GridBagHelper.addGridBagComponent(GeneralPanel.this, _aliasList,
						  0, 0, 1, 1, 1, 0,
						  GridBagConstraints.WEST,
						  GridBagConstraints.NONE,
						  insets, 0, 0);
	    }
	    
	    _issuedToLabel = new JLabel("");
	    GridBagHelper.addGridBagComponent(GeneralPanel.this,_issuedToLabel,
					      0, 2, 1, 1, 12, 0,
					      GridBagConstraints.WEST,
					      GridBagConstraints.HORIZONTAL,
					      insets, 0, 0);
	    
	    _issuerLabel = new JLabel("");
	    GridBagHelper.addGridBagComponent(GeneralPanel.this, _issuerLabel,
					      0, 3, 1, 1, 12, 0,
					      GridBagConstraints.WEST,
					      GridBagConstraints.HORIZONTAL,
					      insets, 0, 0);
	    
	    _dateLabel = new JLabel("");
	    GridBagHelper.addGridBagComponent(GeneralPanel.this, _dateLabel,
					      0, 4, 2, 1, 1, 0,
					      GridBagConstraints.WEST,
					      GridBagConstraints.HORIZONTAL,
					      insets, 0, 0);
	    
	    label = new JLabel(Resources.getString
				   ("certificatePanel.general.details"));
	    // insets.top = 5;
	    GridBagHelper.addGridBagComponent(GeneralPanel.this, label,
					      0, 5, 2, 1, 1, 0,
					      GridBagConstraints.WEST,
					      GridBagConstraints.HORIZONTAL,
					      insets, 0, 0);
	    
	    insets.top = 0;
	    final String[] names = { Resources.getString
		    ("certificatePanel.general.table.column0"),
		    Resources.getString
		    ("certificatePanel.general.table.column1") };
	    _tableModel = new DefaultTableModel(names, 0) {
		public boolean isCellEditable(int row, int column) {
		    return false;
		}
	    };
	    JTable table = new JTable(_tableModel);
	    table.setAutoResizeMode(JTable.AUTO_RESIZE_OFF);
	    JScrollPane sp = new JScrollPane(table);
	    table.setPreferredScrollableViewportSize(new Dimension(600, 300));
	    for (int i = 0; i < table.getColumnCount(); i++ ) {
		if (i == 0) {
		    table.getColumnModel().getColumn(i).setPreferredWidth(100);
		} else {
		    table.getColumnModel().getColumn(i).setPreferredWidth(1100);
		}
	    }
	    sp.setBackground(Color.white);
	    sp.getViewport().setBackground(Color.white);
	    GridBagHelper.addGridBagComponent(GeneralPanel.this, sp,
					      0, 6, 2, 1, 1, 1,
					      GridBagConstraints.WEST,
					      GridBagConstraints.BOTH,
					      insets, 0, 0);
	    
	}
	
	private void updateWidgets(CertificateInfo ci) {
	    
	    if (_aliasList != null) {
		// remove listeners
		_aliasList.removeActionListener(_aliasActionListener);
		_aliasList.removeAncestorListener(_aliasAncestorListener);
		_aliasList.removeAllItems();
		Vector aliases = getAliases();
		if (aliases != null) {
		    for (int i=0; i<aliases.size(); i++) {
			_aliasList.addItem(aliases.elementAt(i));
		    }
		}
		_aliasList.addActionListener(_aliasActionListener);
		_aliasList.addAncestorListener(_aliasAncestorListener);
		_aliasList.setSelectedIndex(_selectedIndex);
	    }
	    
	    _issuedToLabel.setText(Resources.getString
				   ("certificatePanel.general.issuedTo") + " " + ci.getSubjectName());
	    _issuerLabel.setText(Resources.getString
				   ("certificatePanel.general.issuer") + " " + ci.getIssuerName());
	    DateFormat df = DateFormat.getDateInstance(DateFormat.SHORT);
	    Date date = ci.getStartDate();
	    String start = (date == null) ? null : df.format(date);
	    
	    date = ci.getExpirationDate();
	    String end = (date == null) ? null : df.format(date);
	    String dateString;
	    
	    if (start != null) {
		if (end != null) {
		    dateString = Resources.getString
			("certificatePanel.general.dateString0", start, end);
		}
		else {
		    dateString = Resources.getString
			("certificatePanel.general.dateString1", start);
		}
	    }
	    else if (end != null) {
		dateString = Resources.getString
		    ("certificatePanel.general.dateString2", end);
	    }
	    else {
		dateString = Resources.getString
		    ("certificatePanel.general.dateString3", start);
	    }
	    _dateLabel.setText(dateString);
	}
	
	private void updateTable(CertificateInfo ci) {
	    _tableModel.setNumRows(0);
	    
	    Iterator iterator = ci.getKeys();
	    Object[] value = new Object[2];
	    
	    while (iterator.hasNext()) {
		Object key = iterator.next();
		
		value[0] = key.toString();
		value[1] = ci.getValue(key);
		_tableModel.addRow(value);
	    }
	}
    }
    
    
    private class CertPathPanel extends JPanel implements ActionListener, Subcontroller, TreeSelectionListener {
	private boolean _createdComponents;
	private JTree _tree;
	private JButton _viewButton;
	private JButton _importButton;
	private JButton _exportButton;
	
	public void start() {
	    if (!_createdComponents) {
		_createdComponents = true;
		createPropertyControls();
	    }
	    
	    updateWidgets();
	}
	
	public void stop() {
	}
	
	public java.awt.Component getComponent() {
	    return CertPathPanel.this;
	}
	
	public void apply() {}
	public void revert() {}
	
	public void actionPerformed(ActionEvent ae) {
	    if (ae.getSource() == _viewButton) {
		int row = _tree.getMinSelectionRow();
		int index = getSelectedIndex();
		CertificateInfo[] certs = getCertificates(getSelectedIndex());
		
		if (row >= 0 && row < certs.length - 1) {
		    CertificateInfo[] subCerts = new CertificateInfo[row + 1];
		    
		    System.arraycopy(certs, certs.length - row - 1,
				     subCerts, 0, row + 1);
		    showCertificateDialog(getFrame(), subCerts, -1);
		}
	    } else if (_showImportExport && ae.getSource() == _importButton) {
		// do import here
		KeyStore ks = null;
		try {
		    ks = KeyStoreManager.getKeyStore();
		} catch (KeyStoreException kse) {
		    kse.printStackTrace();
		    return;
		}
		JFileChooser jfc = new JFileChooser();
		jfc.setFileSelectionMode(JFileChooser.FILES_ONLY);
		jfc.setDialogType(JFileChooser.OPEN_DIALOG);
		jfc.setMultiSelectionEnabled(false);
		int result = jfc.showOpenDialog(this);
		// fix for 4687857: focus is back to wrong window after closing 
		// dialog
		this.requestFocus();
		if (result == JFileChooser.APPROVE_OPTION) {
		    File f = jfc.getSelectedFile();
		    if (f == null) return;
		    BufferedInputStream bis = null;
		    try {
			bis = new BufferedInputStream(new FileInputStream(f));
			String [] s = askUserForCertAliasAndPassword();
			if (s != null) {
			    doImportCert(s[0], s[1], bis);
			}
		    } catch (Exception e) {
			e.printStackTrace();
		    } finally {
			try {
			    if (bis != null) bis.close();
			} catch (IOException ioe) {
			    Debug.ignoredException(ioe);
			}
			updateData(true);
			updateWidgets();
		    }
		}
	    } else if (_showImportExport && ae.getSource() == _exportButton) {
		// do export here
		int row = _tree.getMinSelectionRow();
		CertificateInfo[] certs = getCertificates(getSelectedIndex());
		if (row >= 0 && row < certs.length) {
		    JFileChooser jfc = new JFileChooser();
		    jfc.setFileSelectionMode(JFileChooser.FILES_ONLY);
		    jfc.setDialogType(JFileChooser.SAVE_DIALOG);
		    jfc.setMultiSelectionEnabled(false);
		    int result = jfc.showSaveDialog(this);
		    // fix for 4687857: focus is back to wrong window after closing 
		    // dialog
		    this.requestFocus();
		    if (result == JFileChooser.APPROVE_OPTION) {
			File f = jfc.getSelectedFile();
			if (f == null) return;
			PrintStream ps = null;
			try {
			    ps = new PrintStream(new BufferedOutputStream(new FileOutputStream(f)));
			    doExportCert(certs[row].getCertificate(), ps);
			} catch (IOException ioe) {
			    ioe.printStackTrace();
			} finally {
			    if (ps != null) ps.close();
			}
		    }
		}
	    }
	}
	
	private String [] askUserForCertAliasAndPassword() {
	    JTextField textfield1 = new JTextField();
	    JTextField textfield2 = new JPasswordField();
	    JLabel label1 = new JLabel(Resources.getString("certificatePanel.import.aliasquery"));
	    JLabel label2 = new JLabel(Resources.getString("certificatePanel.import.passwordquery"));
	    Object [] objs = { label1, textfield1, label2, textfield2 };

	    int result = GeneralUtilities.showOptionDialog(this, objs,
		Resources.getString("certificatePanel.import.aliasquerytitle"),
		JOptionPane.OK_OPTION, JOptionPane.QUESTION_MESSAGE);

	    String s = null;
	    String pass = null;
	    if (result == JOptionPane.OK_OPTION) {
		s = textfield1.getText();
		pass = textfield2.getText();	
	    } else {
		// fix for 4777764: Cancelling import certificate will cause ca chain completely broken		
		return null;
	    }
	    if (s != null && s.equals("")) { 
		s = null;
	    }	   
	    String [] arr = { (s==null ? DEFAULT_CERTIFICATE_ALIAS : s) , pass };	    
	    return arr;
	}
	
	public void valueChanged(TreeSelectionEvent e) {
	    updateViewButton();
	}
	
	private void updateWidgets() {
	    CertificateInfo[] certs = getCertificates(getSelectedIndex());
		
	    if (certs != null && certs.length > 0) {
		DefaultMutableTreeNode last;
		
		last = new DefaultMutableTreeNode(certs[0].getSubjectName());
		for (int counter = 1; counter < certs.length; counter++) {
		    DefaultMutableTreeNode current = new
			DefaultMutableTreeNode(certs[counter].
						   getSubjectName());
		    
		    current.add(last);
		    last = current;
		}
		_tree.setModel(new DefaultTreeModel(last));
		_tree.setRootVisible(true);
		// Make all the nodes visible.
		for (int counter = 0; counter < _tree.getRowCount();
		     counter++) {
		    _tree.expandRow(counter);
		}
	    }
	    else {
		_tree.setModel(new DefaultTreeModel
				   (new DefaultMutableTreeNode("")));
		_tree.setRootVisible(false);
	    }
	    updateViewButton();
	}
	
	private void updateViewButton() {
	    int row = _tree.getMinSelectionRow();
	    CertificateInfo[] certs = getCertificates(getSelectedIndex());
	    
	    _viewButton.setEnabled((certs != null && row >= 0 &&
					row < certs.length - 1));
		if (_exportButton != null) {
			_exportButton.setEnabled(certs != null && row >= 0 && row < certs.length);
		}
	}
	
	private void createPropertyControls() {
	    Dimension minSize = new Dimension(0,0); 
	    setMinimumSize(minSize); 
	    
	    _tree = new JTree();
	    _tree.addTreeSelectionListener(this);
	    _tree.getSelectionModel().setSelectionMode
		(TreeSelectionModel.SINGLE_TREE_SELECTION);	    	  
	 
	    JLabel label;

	    String java_ver = System.getProperty("java.version");       
	    // only use this work around if it is running 1.3.x or below
	    // only need to check java version that starts with 1.2 or 1.3
	    // since java web start only support Java 2 (1.2 or above)
	    if (java_ver.startsWith("1.2") || java_ver.startsWith("1.3")) {
		
		label = new JLabel("<html><u><font size=4 color=black face=dialog>" + Resources.getString
				      ("certificatePanel.certPath.title") + "</font></u></html>");
	    } else {
		label = new JLabel("<html><u>" + Resources.getString
				    ("certificatePanel.certPath.title") + "</u></html>");
		label.setFont(label.getFont().deriveFont
			      (Font.BOLD, (float)(label.getFont().getSize() +4)));
	    } 

	   
	 
	    label.setForeground(Color.black);

	    label.setBorder(new EmptyBorder(2,2,2,2));

	    FlowLayout myLayout = new FlowLayout(FlowLayout.LEFT) {
		    public Dimension preferredLayoutSize(Container target) {
			return minimumLayoutSize(target);
		    }
		    public Dimension minimumLayoutSize(Container target) {
			Container parent = target.getParent();
			Component[] children = target.getComponents();
			int         numChildren = children.length;
			Insets      cInsets = target.getInsets();
			int         extraH = cInsets.top + cInsets.bottom + 8;
			int         extraW = cInsets.left + cInsets.right;
			
			if (parent != null &&
			    children != null && children.length > 0) {
			    Dimension aSize, total;
			    int width = 0;
			    int height = 0;
			    for(int counter = 0; counter < numChildren; counter++){
				aSize = children[counter].getPreferredSize();
				height = Math.max(height, aSize.height);
				width += aSize.width;
			    }
			    
			    int totalWidth = extraW + width +
                            numChildren * getHgap();
			    			
			    if (parent.getWidth() < totalWidth) {

				int buttonWidth = (totalWidth/3) + 1;
			
				int curr_hold = (parent.getWidth()/(buttonWidth));
				int num_rows = 3 - curr_hold;
			
				if (curr_hold > 0) num_rows++;	

				return new Dimension (totalWidth,
						      extraH + num_rows * ( height + getVgap()));
			    }
			    return new Dimension(totalWidth, extraH + height);
			}
			return new Dimension(0,0);
		    }
		};
	    
	    JPanel buttonPanel = new JPanel(myLayout);

	    if (_showImportExport) {
		_importButton = new JButton(Resources.getString
						("certificatePanel.certPath.import"));
		_exportButton = new JButton(Resources.getString
						("certificatePanel.certPath.export"));
		_importButton.setMnemonic(Resources.
		    getVKCode("certificatePanel.certPath.importMnemonic"));
		_exportButton.setMnemonic(Resources.
		    getVKCode("certificatePanel.certPath.exportMnemonic"));

		_importButton.addActionListener(CertPathPanel.this);
		_exportButton.addActionListener(CertPathPanel.this);
	
		buttonPanel.add(_importButton);
		buttonPanel.add(_exportButton);
	    }
	    
	    _viewButton = new JButton(
		Resources.getString("certificatePanel.certPath.details"));

	    _viewButton.setMnemonic(
		Resources.getVKCode("certificatePanel.certPath.detailsMnemonic"));

	    _viewButton.addActionListener(CertPathPanel.this);
	   
	    buttonPanel.add(_viewButton);
	 
	    CertPathPanel.this.setLayout(new BorderLayout(2,2));
	    CertPathPanel.this.add(label, BorderLayout.NORTH);

	    JScrollPane treePane = new JScrollPane(_tree, JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED, JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED);

	    Border border = treePane.getBorder();
	    Border margin = new EmptyBorder(2, 2, 2, 2);

	    treePane.setBorder(new CompoundBorder(margin, border));

	    CertPathPanel.this.add(treePane, BorderLayout.CENTER);
	    CertPathPanel.this.add(buttonPanel, BorderLayout.SOUTH);
	    
	}
    }
    
    private void doImportCert(String alias, String password, InputStream in) throws Exception {
	if (alias == null) {
	    throw new Exception(Resources.getString("certificatePanel.import.aliasnull"));
	}
	
	KeyStore ks = null;
	try {
	    ks = KeyStoreManager.getKeyStore();
	} catch (KeyStoreException kse) {
	    kse.printStackTrace();
	    // SBFIX: error handling
	}
	if (ks == null) {
	    throw new Exception(Resources.getString("certificatePanel.import.keystorereaderror"));
	}
	
	if (ks.isKeyEntry(alias)) {
	    // we are installing a certificate reply from a CA	    
	    installReply(ks, alias, password.toCharArray(), in);
	    KeyStoreManager.saveKeyStore(ks, password);
	    return;
	}
	
	if (ks.containsAlias(alias) == true) {
	    // didn't add certificate because alias already exist
	    GeneralUtilities.showMessageDialog(this, Resources.getString("certificatePanel.import.aliasexists", alias), Resources.getString("certificatePanel.import.querytitle"), JOptionPane.WARNING_MESSAGE);
	    return;
	}
	// read the cert
	X509Certificate cert = null;
	CertificateFactory cf = CertificateFactory.getInstance("X.509");
	try {
	    cert = (X509Certificate)cf.generateCertificate(in);
	} catch (ClassCastException cce) {
	    throw new Exception("Input not an X.509 Certificate");
	}
	
	// if the certificate is self-signed, make sure it verifies
	boolean selfSigned = false;
	if (isSelfSigned(cert)) {
	    cert.verify(cert.getPublicKey());
	    selfSigned = true;
	}
	
	String trustalias = ks.getCertificateAlias(cert);
	boolean reply = false;
	if (trustalias != null) {
	    reply = askUser(Resources.getString("certificatePanel.import.certexistsquery", trustalias));	    
	} else if (selfSigned) {
	    if (trustalias == null) {
		reply = askUserAboutCertificate(cert);
	    }
	}
	if (reply) {	    
	    ks.setCertificateEntry(alias, cert);
	} else {
	    // fix for 4777764: Cancelling import certificate will cause ca chain completely broken
	    // should not add certificate if user denied
	    return;
	}
	
	// try to establish trust chain
	try {
	    Certificate[] chain = establishCertChain(null, cert);
	    if (chain != null) {		
		ks.setCertificateEntry(alias, cert);		
	    }
	} catch (Exception e) {
	    // Print the cert and ask user if they really want to add it to
	    // their keystore
	    if (askUserAboutCertificate(cert)) {		
		ks.setCertificateEntry(alias, cert);
	    } else {
		return;
	    }
	}	
	KeyStoreManager.saveKeyStore(ks, password);
	return;
    }
    
    /**
     * Processes a certificate reply from a certificate authority.
     *
     * <p>Builds a certificate chain on top of the certificate reply,
     * using trusted certificates from the keystore. The chain is complete
     * after a self-signed certificate has been encountered. The self-signed
     * certificate is considered a root certificate authority, and is stored
     * at the end of the chain.
     *
     * <p>The newly generated chain replaces the old chain associated with the
     * key entry.
     *
     * @return true if the certificate reply was installed, otherwise false.
     */
    private boolean installReply(KeyStore keyStore, String keyAlias, char [] storePass, InputStream in)
	throws Exception
    {
	String alias = null;
	if (alias == null) {
	    alias = keyAlias;
	}
	
	char [] keyPass = storePass;
	
	Object[] objs = recoverPrivateKey(keyStore, alias, storePass, keyPass);
	PrivateKey privKey = (PrivateKey)objs[0];
	if (keyPass == null)
	    keyPass = (char[])objs[1];
	
	Certificate userCert = keyStore.getCertificate(alias);
	if (userCert == null) {
	    throw new Exception(Resources.getString("certificatePanel.import.nopublickey", alias));
	}
	
	// Read the certificates in the reply
	CertificateFactory cf = CertificateFactory.getInstance("X.509");
	Collection c = cf.generateCertificates(in);
	if (c.isEmpty()) {
	    throw new Exception(Resources.getString("certificatePanel.import.nocert"));
	}
	Certificate[] replyCerts = (Certificate[])c.toArray();
	Certificate[] newChain;
	if (replyCerts.length == 1) {
	    // single-cert reply
	    newChain = establishCertChain(userCert, replyCerts[0]);
	} else {
	    // cert-chain reply (e.g., PKCS#7)
	    newChain = validateReply(keyStore, alias, userCert, replyCerts);
	}
	
	// Now store the newly established chain in the keystore. The new
	// chain replaces the old one.
	if (newChain != null) {
	    keyStore.setKeyEntry(alias, privKey, (keyPass != null) ? keyPass : storePass, newChain);
	    return true;
	} else {
	    return false;
	}
    }
    
    /**
     * Recovers (private) key associated with given alias.
     *
     * @return an array of objects, where the 1st element in the array is the
     * recovered private key, and the 2nd element is the password used to
     * recover it.
     */
    private Object[] recoverPrivateKey(KeyStore keyStore, String alias, char[] storePass,
				       char[] keyPass)
	throws Exception
    {
	Key key = null;
	
	if (keyStore.containsAlias(alias) == false) {
	    throw new Exception(Resources.getString("certificatePanel.import.noalias", alias));
	}
	if (keyStore.isKeyEntry(alias) == false) {
	    throw new Exception(Resources.getString("certificatePanel.import.noprivatekey", alias));
	}
	
	if (keyPass == null) {
	    // Try to recover the key using the keystore password
	    try {
		key = keyStore.getKey(alias, storePass);
		keyPass = storePass;
	    } catch (UnrecoverableKeyException e) {
		// Did not work out, so prompt user for key password
		keyPass = getKeyPasswd(alias);
		key = keyStore.getKey(alias, keyPass);
	    }
	} else {
	    key = keyStore.getKey(alias, keyPass);
	}
	if (!(key instanceof PrivateKey)) {
	    throw new Exception(Resources.getString("certificatePanel.import.notaprivkey"));
	}
	return new Object[] {(PrivateKey)key, keyPass};
    }
    
    /**
     * Validates chain in certification reply, and returns the ordered
     * elements of the chain (with user certificate first, and root
     * certificate last in the array).
     *
     * @param alias the alias name
     * @param userCert the user certificate of the alias
     * @param replyCerts the chain provided in the reply
     */
    private Certificate[] validateReply(KeyStore keyStore,
					String alias,
					Certificate userCert,
					Certificate[] replyCerts)
	throws Exception
    {
	// order the certs in the reply (bottom-up).
	// we know that all certs in the reply are of type X.509, because
	// we parsed them using an X.509 certificate factory
	int i;
	PublicKey userPubKey = userCert.getPublicKey();
	for (i=0; i<replyCerts.length; i++) {
	    if (userPubKey.equals(replyCerts[i].getPublicKey()))
		break;
	}
	if (i == replyCerts.length) {
	    throw new Exception(Resources.getString("certificatePanel.import.nocertreplypubkey", alias));
	}
	
	Certificate tmpCert = replyCerts[0];
	replyCerts[0] = replyCerts[i];
	replyCerts[i] = tmpCert;
	Principal issuer = ((X509Certificate)replyCerts[0]).getIssuerDN();
	
	for (i=1; i < replyCerts.length-1; i++) {
	    // find a cert in the reply whose "subject" is the same as the
	    // given "issuer"
	    int j;
	    for (j=i; j<replyCerts.length; j++) {
		Principal subject;
		subject = ((X509Certificate)replyCerts[j]).getSubjectDN();
		if (subject.equals(issuer)) {
		    tmpCert = replyCerts[i];
		    replyCerts[i] = replyCerts[j];
		    replyCerts[j] = tmpCert;
		    issuer = ((X509Certificate)replyCerts[i]).getIssuerDN();
		    break;
		}
	    }
	    if (j == replyCerts.length) {
		throw new Exception(Resources.getString("certificatePanel.import.incompletechain"));
	    }
	}
	
	// now verify each cert in the ordered chain
	for (i=0; i<replyCerts.length-1; i++) {
	    PublicKey pubKey = replyCerts[i+1].getPublicKey();
	    try {
		replyCerts[i].verify(pubKey);
	    } catch (Exception e) {
		throw new Exception(Resources.getString("certificatePanel.import.cantverify", e.getMessage()));
	    }
	}
	
	// do we trust the (root) cert at the top?
	Certificate topCert = replyCerts[replyCerts.length-1];
	if (!isTrusted(keyStore, topCert)) {
	    boolean verified = false;
	    Certificate rootCert = null;
	    if (!verified) {
		if (!askUserAboutCertificate(topCert)) {
		    return null;
		}
	    } else {
		if (!isSelfSigned((X509Certificate)topCert)) {
		    // append the (self-signed) root CA cert to the chain
		    Certificate[] tmpCerts =
			new Certificate[replyCerts.length+1];
		    System.arraycopy(replyCerts, 0, tmpCerts, 0,
				     replyCerts.length);
		    tmpCerts[tmpCerts.length-1] = rootCert;
		    replyCerts = tmpCerts;
		}
	    }
	}
	
	return replyCerts;
    }
    
    /**
     * Returns true if the given certificate is trusted, false otherwise.
     */
    private boolean isTrusted(KeyStore keyStore, Certificate cert)
	throws Exception
    {
	if (keyStore.getCertificateAlias(cert) != null)
	    return true; // found in own keystore
	return false;
    }
    
    private char [] getKeyPasswd(String alias) {
	return askUserForCertPassword(alias).toCharArray();
    }
    
    private String askUserForCertPassword(String alias) {
	JTextField textfield = new JPasswordField();
	JLabel label = new JLabel(Resources.getString("certificatePanel.import.certpasswordquery", alias));
	Object [] objs = { label, textfield };

	int result = GeneralUtilities.showOptionDialog(this, objs,
		Resources.getString("certificatePanel.import.aliasquerytitle"),
		JOptionPane.OK_OPTION, JOptionPane.QUESTION_MESSAGE);

	String s = null;
	if (result == JOptionPane.OK_OPTION) {
	    s = textfield.getText();
	}
	return s;
    }
    
    private boolean askUserAboutCertificate(Certificate cert) throws Exception {
	JLabel label = new JLabel(Resources.getString("certificatePanel.import.certtrustquery"));
	Insets insets = new Insets(5, 5, 5, 5);
	JPanel panel = new JPanel(new BorderLayout());
	panel.add(label, BorderLayout.NORTH);
	CertificatePanel certPanel = new CertificatePanel(false, true, false);
	certPanel._certificates = new CertificateInfo[1][1];
	certPanel._certificates[0][0] = CertificateInfo.getCertificateInfo(cert);
	certPanel._selectedIndex = 0;
	certPanel.start();
	certPanel.setVisible(true);
	((JComponent)certPanel.getComponent()).setPreferredSize(new Dimension(400,400));
	panel.add(certPanel.getComponent(), BorderLayout.CENTER);
	return askUser(panel);
    }
    
    
    private Certificate[] establishCertChain(Certificate userCert, Certificate certToVerify) throws Exception {
	if (userCert != null) {
	    // Make sure that the public key of the certificate reply matches
	    // the original public key in the keystore
	    PublicKey origPubKey = userCert.getPublicKey();
	    PublicKey replyPubKey = certToVerify.getPublicKey();
	    if (!origPubKey.equals(replyPubKey)) {
		String msg = Resources.getString("certificatePanel.import.keymatcherror");
		throw new Exception(msg);
	    }
	    
	    // If the two certs are identical, we're done: no need to import
	    // anything
	    if (certToVerify.equals(userCert)) {
		String msg = Resources.getString("certificatePanel.import.replyequalscert");
		throw new Exception(msg);
	    }
	}
	
	// Build a hash table of all certificates in the keystore.
	// Use the subject distinguished name as the key into the hash table.
	// All certificates associated with the same subject distinguished
	// name are stored in the same hash table entry as a vector.
	Hashtable certs = null;
	KeyStore ks = null;
	try {
	    ks = KeyStoreManager.getKeyStore();
	} catch (KeyStoreException kse) {
	    kse.printStackTrace();
	}
	if (ks == null) return null;
	if (ks.size() > 0) {
	    certs = new Hashtable(11);
	    keystorecerts2Hashtable(ks, certs);
	}
	
	// start building chain
	Vector chain = new Vector(2);
	if (buildChain((X509Certificate)certToVerify, chain, certs)) {
	    Certificate[] newChain = new Certificate[chain.size()];
	    // buildChain() returns chain with self-signed root-cert first and
	    // user-cert last, so we need to invert the chain before we store
	    // it
	    int j=0;
	    for (int i=chain.size()-1; i>=0; i--) {
		newChain[j] = (Certificate)chain.elementAt(i);
		j++;
	    }
	    return newChain;
	} else {
	    throw new Exception("Failed to establish chain from reply");
	}
    }
    
    /**
     * Recursively tries to establish chain from pool of trusted certs.
     *
     * @param certToVerify the cert that needs to be verified.
     * @param chain the chain that's being built.
     * @param certs the pool of trusted certs
     *
     * @return true if successful, false otherwise.
     */
    private boolean buildChain(X509Certificate certToVerify, Vector chain,
			       Hashtable certs) {
	Principal subject = certToVerify.getSubjectDN();
	Principal issuer = certToVerify.getIssuerDN();
	if (subject.equals(issuer)) {
	    // reached self-signed root cert;
	    // no verification needed because it's trusted.
	    chain.addElement(certToVerify);
	    return true;
	}
	
	// Get the issuer's certificate(s)
	Vector vec = (Vector)certs.get(issuer);
	if (vec == null)
	    return false;
	
	// Try out each certificate in the vector, until we find one
	// whose public key verifies the signature of the certificate
	// in question.
	for (Enumeration issuerCerts = vec.elements();
	     issuerCerts.hasMoreElements(); ) {
	    X509Certificate issuerCert
		= (X509Certificate)issuerCerts.nextElement();
	    PublicKey issuerPubKey = issuerCert.getPublicKey();
	    try {
		certToVerify.verify(issuerPubKey);
	    } catch(Exception e) {
		continue;
	    }
	    if (buildChain(issuerCert, chain, certs)) {
		chain.addElement(certToVerify);
		return true;
	    }
	}
	return false;
    }
    
    /**
     * Stores the (leaf) certificates of a keystore in a hashtable.
     * All certs belonging to the same CA are stored in a vector that
     * in turn is stored in the hashtable, keyed by the CA's subject DN
     */
    private void keystorecerts2Hashtable(KeyStore ks, Hashtable hash)
	throws Exception
    {
	for (Enumeration aliases = ks.aliases(); aliases.hasMoreElements(); ) {
	    String alias = (String)aliases.nextElement();
	    Certificate cert = (Certificate)ks.getCertificate(alias);
	    if (cert != null) {
		Principal subjectDN = ((X509Certificate)cert).getSubjectDN();
		Vector vec = (Vector)hash.get(subjectDN);
		if (vec == null) {
		    vec = new Vector();
		    vec.addElement(cert);
		} else {
		    if (!vec.contains(cert))
			vec.addElement(cert);
		}
		hash.put(subjectDN, vec);
	    }
	}
    }
    
    private boolean askUser(Object message) {
	int result = GeneralUtilities.showOptionDialog(null, message,
		Resources.getString("certificatePanel.import.querytitle"),
		JOptionPane.YES_NO_OPTION, JOptionPane.QUESTION_MESSAGE);

	return (result == JOptionPane.YES_OPTION ? true : false);
    }
    
    private boolean isSelfSigned(X509Certificate cert) {
	return cert.getSubjectDN().equals(cert.getIssuerDN());
    }
    
    private void doExportCert(Certificate cert, PrintStream out) {
	X509Certificate xcert = (X509Certificate)cert;
	dumpX509Cert(xcert, out);
    }
    
    private void dumpX509Cert(X509Certificate cert, PrintStream out) {
	BASE64Encoder encoder = new BASE64Encoder();
	out.println(SunSecurityUtil.getBeginCert());
	try {
	    encoder.encodeBuffer(cert.getEncoded(), out);
	} catch (IOException ioe) {
	    ioe.printStackTrace();
	    // SBFIX: error handling
	} catch (CertificateEncodingException cee) {
	    cee.printStackTrace();
	    // SBFIX: error handling
	}
	out.println(SunSecurityUtil.getEndCert());
    }
}



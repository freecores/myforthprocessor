/*
 * @(#)JREPanel.java	1.7 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws.ui.prefs;

import javax.swing.*;
import javax.swing.border.*;
import javax.swing.event.*;
import javax.swing.table.*;
import java.awt.*;
import java.awt.event.*;
import java.io.*;
import java.net.*;
import java.util.*;
import com.sun.javaws.*;
import com.sun.javaws.debug.Globals;
import com.sun.javaws.ui.general.*;

/**
 * @version 1.21 11/29/01
 */
public class JREPanel extends PrefsPanel.PropertyPanel implements
    ActionListener, ListSelectionListener, TableModelListener {
    /**
     * Border used in the editor/renderer to indicate the current path is
     * bogus.
     */
    private static Border badBorder = new javax.swing.border.LineBorder(Color.red);
    
    private JTable jresTable;
    private JTextField locationTF;
    private JRETableModel tableModel;
    private JButton findButton;
    private JButton removeButton;
    private JButton chooseButton;
    private int _nextAvailableJREIndex = 0;

    // hide jres with incorrect os and arch
    private HashSet _hiddenJres = new HashSet();

    
    protected void createPropertyControls() {
        tableModel = new JRETableModel();
        
        // And create/layout the widgets.
        setLayout(new GridBagLayout());
        GridBagConstraints cons = new GridBagConstraints();
        TitledBorder border = new TitledBorder
            (javax.swing.border.LineBorder.
                 createBlackLineBorder(),
             Resources.getString("controlpanel.jre.title"),
             TitledBorder.LEFT, TitledBorder.TOP);
        JPanel jrePanel = new JPanel(new GridBagLayout());
        jrePanel.setBorder(border);
        
        jresTable = new JTable(tableModel);
	// set a renderer that greys out uneditable JREs
	Enumeration e = jresTable.getColumnModel().getColumns();
	JRETableCellRenderer renderer = new JRETableCellRenderer();
	while (e.hasMoreElements()) {
	    TableColumn tc = (TableColumn)e.nextElement();
	    if (tc.getModelIndex() < 4) {
		tc.setCellRenderer(renderer);
	    }
	}
        jresTable.setPreferredScrollableViewportSize(new Dimension(450, 150));
        jresTable.getSelectionModel().addListSelectionListener(JREPanel.this);
        // Create a renderer that shows the badBorder if the path isn't
        // valid.
        jresTable.getColumnModel().getColumn(3).setCellRenderer
            (new JRETableCellRenderer() {
                    public Component getTableCellRendererComponent
                        (JTable table, Object value,
                         boolean isSelected, boolean hasFocus,
                         int row, int column) {
                        Component retValue =super.getTableCellRendererComponent
                            (table, value, isSelected, hasFocus, row,
                             column);
                        if (retValue instanceof JComponent &&
                            !tableModel.isPathValid(row)) {
                            
                            ((JComponent)retValue).setBorder(badBorder);
                        }
                        return retValue;
                    }
                });
        // Set an editor that will show badBorder if the border isn't
        // valid.
        jresTable.getColumnModel().getColumn(3).setCellEditor
            (new PathEditor());
        cons.gridx = 0;
        cons.gridy++;
        cons.fill = GridBagConstraints.BOTH;
        cons.weightx = cons.weighty = 1;
        cons.gridwidth = 2;
        jrePanel.add(new JScrollPane(jresTable), cons);
        
        Box b = Box.createHorizontalBox();
        chooseButton = new JButton(
	    Resources.getString("controlpanel.jre.choose"));
	chooseButton.setMnemonic(
	    Resources.getVKCode("controlpanel.jre.chooseMnemonic"));
        chooseButton.setRequestFocusEnabled(false);
        chooseButton.addActionListener(JREPanel.this);
        b.add(chooseButton);
        b.add(Box.createHorizontalGlue());
        
        JButton button = new JButton(
	    Resources.getString("controlpanel.jre.find"));
        button.setRequestFocusEnabled(false);
        button.addActionListener(JREPanel.this);
        b.add(button);
        b.add(Box.createHorizontalStrut(5));
        findButton = button;
	findButton.setMnemonic(
	    Resources.getVKCode("controlpanel.jre.findMnemonic"));
        
        button = new JButton(Resources.getString("controlpanel.jre.add"));
        button.setRequestFocusEnabled(false);
        button.addActionListener(JREPanel.this);
	button.setMnemonic(
	    Resources.getVKCode("controlpanel.jre.addMnemonic"));
        b.add(button);
        b.add(Box.createHorizontalStrut(5));
        
        button = new JButton(Resources.getString("controlpanel.jre.remove"));
        button.setRequestFocusEnabled(false);
        b.add(button);
        removeButton = button;
        removeButton.addActionListener(JREPanel.this);
	removeButton.setMnemonic(
	    Resources.getVKCode("controlpanel.jre.removeMnemonic"));
        
        cons.weightx = 1;
        cons.weighty = 0;
        cons.fill = GridBagConstraints.HORIZONTAL;
        cons.gridy++;
        cons.anchor = GridBagConstraints.EAST;
        jrePanel.add(b, cons);
        
        cons.gridy++;
        cons.fill = GridBagConstraints.BOTH;
        cons.weightx = cons.weighty = 1;
        cons.insets.top = 0;
        cons.insets.bottom = 5;
        add(jrePanel, cons);
        
        updateRemoveButton();
	tableModel.addTableModelListener(this);
        revert();
    }
    
    //
    // ActionListener
    //
    /**
     * Invoked from the <code>removeButton</code>, in which case the
     * selected rows are removed, or the <code>addButton</code>, in which
     * case a new row is added.
     */
    public void actionPerformed(ActionEvent ae) {
        JComponent source = (JComponent)ae.getSource();
        if (source == removeButton) {
            tableModel.remove(jresTable.getSelectedRows());
	    // the following is needed on 1.2.2 only - (and no prob on others)
	    // if you remove the selected rows, getSelectedRow will still 
            // return it and so can cause index out of bounds exception.
	    jresTable.clearSelection(); // needed on 1.2.2 - or find 
        } else if (source == findButton) {
            findJREs();
	    // fix for 4687857: focus is back to wrong window after closing 
	    // dialog
	    this.requestFocus();
        } else if (source == chooseButton) {
            int row = jresTable.getSelectedRow();
            if (row != -1) {
                changePath(row);
		// fix for 4687857: focus is back to wrong window after closing 
		// dialog
		this.requestFocus();
            }
        } else {
            // add
            tableModel.add(new ConfigProperties.JREInformation
                               (null, null, null, null,
				ConfigProperties.JREInformation.SRC_USER,
				true,
				getNextAvailableJREIndex(), Globals.osName, Globals.osArch));
            int row = tableModel.getRowCount() - 1;
            jresTable.requestFocus();
            jresTable.setRowSelectionInterval(row, row);
        }
    }
    
    private int getNextAvailableJREIndex() {
	return _nextAvailableJREIndex++;
    }
    

    //
    // ListSelectionListener
    //
    public void valueChanged(ListSelectionEvent e) {
        updateRemoveButton();
    }
    
    //
    // TableModelListener
    //
    public void tableChanged(TableModelEvent tme) {
	updateRemoveButton();
    }

    //
    // Controller methods
    //
    public void apply() {
        // flushes any changes in the table.
        if (jresTable.isEditing()) {
            jresTable.getCellEditor().stopCellEditing();
        }
        
        // Remove any existing defaults.
        ConfigProperties cp = ConfigProperties.getInstance();
        ArrayList jres = cp.getJREInformationList();
	if (tableModel.getRowCount() > 0 ) {
            jres.clear();
            // Apply the new ones.
            for (int n = 0; n < tableModel.getRowCount(); n++) {
                jres.add(tableModel.getJRE(n));
            }
	    
            //  add back the hidden jres           
	    for (Iterator n = _hiddenJres.iterator(); n.hasNext(); ) {
		ConfigProperties.JREInformation hide = (ConfigProperties.JREInformation)n.next(); 
		jres.add(hide);         
	    }
	    
	}
    }
    
    public void revert() {
        jresTable.removeEditor();
        tableModel.refresh();
    }
    
    /**
     * Updates the remove button state based on the selection.
     */
    private void updateRemoveButton() {
        int selectedRow = jresTable.getSelectedRow();
        int count = jresTable.getRowCount();
	// fix for 4441333
	removeButton.setEnabled(selectedRow != -1 && count > 1 &&
				selectedRow < count &&
				(tableModel.getJRE(selectedRow).getWhereSpecified() ==
				 ConfigProperties.JREInformation.SRC_USER));
        chooseButton.setEnabled(selectedRow != -1 && count > 1 &&
				selectedRow < count &&
				(tableModel.getJRE(selectedRow).getWhereSpecified() ==
				 ConfigProperties.JREInformation.SRC_USER));
    }
    
    /**
     * Invoked when the version string has changed for <code>row</code>.
     */
    private void versionChanged(int row) {
    }
    
    /**
     * Returns true if the path <code>path</code> is a valid path to
     * a jre.
     */
    // PENDING: make this more real.
    private boolean isValidJREPath(String path) {
        if (path != null) {
            File f = new File(path);
            return (f.exists() && !f.isDirectory());
        }
        return false;
    }
    
    /**
     * Brings up a file chooser for the user to choose a new path for
     * <code>row</code>.
     */
    private void changePath(int row) {
        JFileChooser fc = new JFileChooser();
        File f = getFirstValidParent(row);
        
        if (f != null && f.exists()) {
            fc.setSelectedFile(f);
        }
        // Bring up the file chooser.
        if (fc.showOpenDialog(JREPanel.this) == JFileChooser.APPROVE_OPTION) {
            tableModel.setValueAt(fc.getSelectedFile().getPath(),
                                  row, 3);
        }
    }
    
    /**
     * Gives the user a chance to choose a path to search for JREs from.
     */
    private void findJREs() {
        JFileChooser fc = new JFileChooser();
        File f = getFirstValidParent(jresTable.getSelectedRow());
        
        if (f != null && f.exists()) {
            fc.setSelectedFile(f);
        }
        fc.setDialogType(JFileChooser.OPEN_DIALOG);
        ConfigProperties.JREInformation jres[] = new JRESearcher().search
            (getComponent(), Resources.getResources(),
             Resources.getString("controlpanel.findJREIntro"));
        if (jres != null) {
            for (int counter = 0; counter < jres.length; counter++) {
                tableModel.add(jres[counter]);
            }
        }
    }
    
    /**
     * Convenience method that returns the first parent directory of
     * the entry at row that exists.
     */
    private File getFirstValidParent(int row) {
        if (row != -1) {
            String path = tableModel.getJRE(row).getInstalledPath();
            
            if (path != null) {
                File f = new File(path);
                // Find the first valid directory starting at path
                while (f != null && !f.exists()) {
                    path = f.getParent();
                    if (path != null) {
                        f = new File(path);
                    }
                    else {
                        f = null;
                    }
                }
                return f;
            }
        }
        return null;
    }
    
    /**
     * Editor used for the path to the JRE. Changes the border based on
     * the validity of the path being edited.
     */
    private class PathEditor extends DefaultCellEditor implements
        DocumentListener {
        private int row;
        
        PathEditor() {
            super(new JTextField());
            ((JTextField)editorComponent).getDocument().
                addDocumentListener(this);
        }
        
        public Component getTableCellEditorComponent(JTable table,
                                                     Object value,
                                                     boolean isSelected,
                                                     int row, int column) {
            this.row = row;
            super.getTableCellEditorComponent(table, value, isSelected,
                                              row, column);
            updateBorder(row);
            return editorComponent;
        }
        
        public void insertUpdate(DocumentEvent e) {
            updateBorderFromEditor();
        }
        public void removeUpdate(DocumentEvent e) {
            updateBorderFromEditor();
        }
        public void changedUpdate(DocumentEvent e) {}
        
        private void updateBorder(int row) {
            Border b = (tableModel.isPathValid(row)) ?
                javax.swing.border.LineBorder.createBlackLineBorder() :
                badBorder;
            editorComponent.setBorder(b);
        }
        
        private void updateBorderFromEditor() {
            Object value = getCellEditorValue();
            boolean valid;
            if (value instanceof String) {
                valid = isValidJREPath((String)value);
            }
            else {
                valid = false;
            }
            if (valid) {
                editorComponent.setBorder(javax.swing.border.LineBorder.
                                              createBlackLineBorder());
            }
            else {
                editorComponent.setBorder(badBorder);
            }
        }
    }

    private class JRETableCellRenderer extends DefaultTableCellRenderer {
	public Component getTableCellRendererComponent(JTable table,
						       Object value,
						       boolean isSelected,
						       boolean hasFocus,
						       int row, int column) {
	    Component retValue = super.getTableCellRendererComponent(table,
								     value,
								     isSelected,
								     hasFocus,
								     row, column);
	    if (retValue instanceof Component) {
		if (((JRETableModel)table.getModel()).getJRE(row).getWhereSpecified() !=
		    ConfigProperties.JREInformation.SRC_USER && column < 4) {
		    retValue.setEnabled(false);
		} else {
		    retValue.setEnabled(true);
		}
	    }
	    return retValue;
	}
    }

    
    /**
     * TableModel containing two columns, one showing the version, and
     * the other showing the path.
     * <p>A better implementation of this would implement the
     * ComboBoxModel as well, so that it didn't have to be recreated
     * as well.
     * <p>The data is store in two List's, one containg the paths
     * and the other containing the versions.
     */
    private class JRETableModel extends AbstractTableModel {
        private ArrayList _jres;
        private ArrayList _validPaths;
        private String[] _columnNames;
        
        JRETableModel() {
            _jres = new ArrayList();
            _validPaths = new ArrayList();
            refresh();
        }
        
        public void refresh() {
            _jres.clear();
            _validPaths.clear();
	    _hiddenJres.clear();

            ConfigProperties cp = ConfigProperties.getInstance();
            ArrayList jres = cp.getJREInformationList();
            for(int i = 0; i < jres.size(); i++) {
                ConfigProperties.JREInformation jreInfo =
                    (ConfigProperties.JREInformation)jres.get(i);

		if (jreInfo.getOsName() == null || 
		    jreInfo.getOsArch() == null ||
		   (jreInfo.getOsName().equals(Globals.osName) && 
		    jreInfo.getOsArch().equals(Globals.osArch))) {
		
		    add((ConfigProperties.JREInformation)(jreInfo.copy()), false, false);
		    
		    if (jreInfo.getIndex() >= _nextAvailableJREIndex)
			_nextAvailableJREIndex = jreInfo.getIndex() + 1;

		} else {
		    _hiddenJres.add((ConfigProperties.JREInformation)(jreInfo.copy()));
		}

            }
            fireTableDataChanged();
        }
        
        public void add(ConfigProperties.JREInformation jre) {
            add(jre, false, true);
        }
        
        void add(ConfigProperties.JREInformation jre, boolean isValid,
                 boolean notify) {
            _jres.add(jre);
            if (isValid) {
                _validPaths.add(Boolean.TRUE);
            }
            else {
                _validPaths.add(null);
            }
            if (notify) {
                fireTableRowsInserted(_jres.size() - 1,
                                      _jres.size() - 1);
            }
        }
        
        public int getRowCount() {
            return _jres.size();
        }
        
        public int getColumnCount() {
            return 5;
        }
	
	public Class getColumnClass(int c) {
	    if (c < 4) return String.class;
	    else return Boolean.class;
    	}
        
        public Object getValueAt(int row, int column) {
            switch (column) {
                case 0:
                    return getJRE(row).getPlatformVersionId();
                case 1:
                    return getJRE(row).getProductVersionId();
                case 2:
                    return getJRE(row).getProductLocation();
                case 3:
                    return getJRE(row).getInstalledPath();
	        default:
		    return new Boolean(getJRE(row).isEnabled());
            }
        }
        
        public boolean isPathValid(int row) {
            Boolean b = (Boolean)_validPaths.get(row);
            if (b == null) {
                // Haven't checked it yet, check it out.
                if (isValidJREPath(getJRE(row).getInstalledPath())) {
                    b = Boolean.TRUE;
                }
                else {
                    b = Boolean.FALSE;
                }
                _validPaths.set(row, b);
            }
            return Boolean.TRUE.equals(b);
        }
        
        public String getColumnName(int column) {
            if (_columnNames == null) {
                _columnNames = new String[]
                { 
		    Resources.getString("controlpanel.jre.platformVersionTableColumnTitle"),
                    Resources.getString("controlpanel.jre.productVersionTableColumnTitle"),
                    Resources.getString("controlpanel.jre.productLocationTableColumnTitle"),
                    Resources.getString("controlpanel.jre.pathTableColumnTitle"),
		    Resources.getString("controlpanel.jre.enabledColumnTitle")
                };
            }
            return _columnNames[column];
        }
        
        public boolean isCellEditable(int rowIndex, int columnIndex) {
	    return (getJRE(rowIndex).getWhereSpecified() == ConfigProperties.JREInformation.SRC_USER ||
		    columnIndex == 4 ?
		    true : false);
        }
        
        public void setValueAt(Object aValue, int rowIndex,
                               int columnIndex) {
            if (rowIndex >= _jres.size()) {
                // bug in JTable that this is messaged
                return;
            }
            
            ConfigProperties.JREInformation jre = getJRE(rowIndex);
            
            switch (columnIndex) {
                case 0:
                    _jres.set(rowIndex, new ConfigProperties.JREInformation(
                                                                               (String)aValue,
                                  jre.getProductVersionId(),
                                  jre.getProductLocation(),
                                  jre.getInstalledPath(),
				  jre.getWhereSpecified(),
				  jre.isEnabled(),
				  jre.getIndex(), jre.getOsName(), jre.getOsArch()));
                    break;
                case 1:
                    _jres.set(rowIndex, new ConfigProperties.JREInformation(
                                  jre.getPlatformVersionId(), (String)aValue,
                                  jre.getProductLocation(), jre.getInstalledPath(),  jre.getWhereSpecified(),
				  jre.isEnabled(),
				  jre.getIndex(), jre.getOsName(), jre.getOsArch()));
                    versionChanged(rowIndex);
                    break;
                case 2:
                    URL location = jre.getProductLocation();
                    try {
                        location = new URL((String)aValue);
                    } catch(MalformedURLException mue) {
                        /* ignore */
                    }
                    _jres.set(rowIndex, new ConfigProperties.JREInformation(
                                  jre.getPlatformVersionId(), jre.getProductVersionId(),
                                  location, jre.getInstalledPath(), jre.getWhereSpecified(),
				  jre.isEnabled(),
				  jre.getIndex(), jre.getOsName(), jre.getOsArch()));
                    versionChanged(rowIndex);
                    break;
	         case 3:
                    _jres.set(rowIndex, new ConfigProperties.JREInformation(
                                  jre.getPlatformVersionId(), jre.getProductVersionId(),
                                  jre.getProductLocation(), (String)aValue, jre.getWhereSpecified(),
				  jre.isEnabled(),
				  jre.getIndex(), jre.getOsName(), jre.getOsArch()));
                    // Force a recheck.
                    _validPaths.set(rowIndex, null);
                    break;
		 default:
                    _jres.set(rowIndex, new ConfigProperties.JREInformation(
                                  jre.getPlatformVersionId(), jre.getProductVersionId(),
                                  jre.getProductLocation(), jre.getInstalledPath(), jre.getWhereSpecified(),
				  ((Boolean)aValue).booleanValue(),
				  jre.getIndex(), jre.getOsName(), jre.getOsArch()));                  
                    break;
            }
            fireTableRowsUpdated(rowIndex, rowIndex);
        }
        
        public void remove(int[] rows) {
            if (rows != null) {
                int count = getRowCount();
                for (int counter = rows.length - 1; counter >= 0;
                     counter--) {
                    if (rows[counter] != -1 && rows[counter] < count) {
                        _jres.remove(rows[counter]);
                        _validPaths.remove(rows[counter]);
                    }
                }
            }
            fireTableDataChanged();
        }
        
        public ConfigProperties.JREInformation getJRE(int index) {
            return (ConfigProperties.JREInformation)_jres.get(index);
        }
    }
}

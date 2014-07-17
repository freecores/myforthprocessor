/*
 * @(#)BookmarkEntryEditor.java	1.4 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.ui.player;
import javax.swing.*;
import javax.swing.table.*;
import java.awt.*;
import java.util.*;
import java.awt.event.*;
import com.sun.javaws.Resources;
import com.sun.javaws.ui.general.GeneralUtilities;
import com.sun.javaws.debug.Globals;
import com.sun.javaws.debug.Debug;

class BookmarkEntryEditor {

    private JDialog _editor;
    private AbstractTableModel _dataModel;
    private JTable _tableView;
    private ArrayList _bookmarkEntryList;

    BookmarkEntryEditor(Frame parentFrame) {
	// create a new JDialog for the Bookmark entry editor
	_editor = GeneralUtilities.createDialog(parentFrame, 
		Resources.getString("player.htmlEntryEditorTitle"), true);
	_bookmarkEntryList = Player.getPlayer().getBookmarkEntryList();
	Container parent = _editor.getContentPane();
	parent.setLayout(new BorderLayout());

	JPanel bottomPanel = new JPanel(new BorderLayout());


	JButton close = new JButton(Resources.getString("player.htmlEntryEditorCloseButton"));
	JButton remove = new JButton(Resources.getString("player.htmlEntryEditorRemoveButton"));

	bottomPanel.add(close, BorderLayout.EAST);
	bottomPanel.add(remove, BorderLayout.WEST);
        bottomPanel.setBorder(BorderFactory.createEmptyBorder(8,2,6,2));

	close.addActionListener(new ActionListener() {
	    public void actionPerformed(ActionEvent ev) {
	    	_editor.dispose();
	    }
	});

	remove.addActionListener(new ActionListener() {
	    public void actionPerformed(ActionEvent ev) {
	    	removeBookmarkEntry();
	    }
	});
 
	close.requestFocus();
	_editor.getRootPane().setDefaultButton(close);
	parent.add(bottomPanel, BorderLayout.SOUTH);

	JScrollPane entryTable = createBookmarkEntryTable(_editor);

	parent.add(entryTable, BorderLayout.CENTER);
	_editor.setBounds(0,0,400,400);
	GeneralUtilities.placeWindow(_editor);
	_editor.show();
    } 

    private Object getBookmarkEntryName(int index) {
	return ((BookmarkEntry)_bookmarkEntryList.get(index)).getName();
    }

    private Object getBookmarkEntryURL(int index) {
	return ((BookmarkEntry)_bookmarkEntryList.get(index)).getUrl();
    }

    private JScrollPane createBookmarkEntryTable(final Component parent) {
	// Table Column Title
	final String[] names = {
	    Resources.getString("player.htmlEntryEditorEntryName"),
	    Resources.getString("player.htmlEntryEditorEntryUrl")
	};

	// Create a model of the data.
        _dataModel = new AbstractTableModel() {
	    public int getColumnCount() { return 2; }
            public int getRowCount() { 		
		return _bookmarkEntryList.size();
	    }
	    public Object getValueAt(int row, int col) {
		switch (col) {
                case 0:
		    return getBookmarkEntryName(row);
		default:
		    return getBookmarkEntryURL(row);
		}
		    
	    }

	    // only allow rename of entries - do not allow URL change
	    public boolean isCellEditable(int rowIndex, int columnIndex) {
		if (columnIndex == 0)
		    return true;
		return false;
	    }

	    public void setValueAt(Object aValue, int rowIndex,
				   int columnIndex) {
				

		String oldName = (String)getBookmarkEntryName(rowIndex);	
	
		// no change
		if (oldName.equals(aValue)) return;
		
		if (((String)aValue).trim().equals("")) {		   
		    Player.showInvalidInputDialog(parent, Resources.getString("player.htmlEntryMissingName"), Resources.getString("player.htmlEntryRenameEntry"));
		    return;
		}
		
		// name already exist
		if (Player.getPlayer().getBookmarkIndex((String)aValue) != -1) {
		    String msg = Resources.getString("player.htmlEntryRenameAlreadyExist", oldName);		    
		    Player.showInvalidInputDialog(parent, msg, Resources.getString("player.htmlEntryRenameError")); 
		    return;
		}
 
		// update html table
		// add new entry
		if (Globals.TracePlayer) {
		    Debug.println("put: " + aValue + " " + getBookmarkEntryURL(rowIndex));
		}

		BookmarkEntry e = new BookmarkEntry((String)aValue, (String)getBookmarkEntryURL(rowIndex));

		// update html list
		_bookmarkEntryList.set(rowIndex, e);


		fireTableRowsUpdated(rowIndex, rowIndex);
	    }

	    public String getColumnName(int column) {return names[column];}
	};

	// Create the table
        _tableView = new JTable(_dataModel);
	JScrollPane scrollpane = new JScrollPane(_tableView);
        return scrollpane;
    }

    private void removeBookmarkEntry() {

	int[] rows =  _tableView.getSelectedRows();
	String[] name = new String[rows.length];

	// get the name of the entry to be removed
	for (int i = 0; i<rows.length; i++) {

	   
	    name[i] = (String)_tableView.getValueAt(rows[i], 0);
	    

	    if (Globals.TracePlayer) {
		Debug.println("remove: " + name[i]);	   	   
	    }
	    	 
	}
	for (int i = 0; i<name.length; i++) {
	    
	    // remove entry in list
	    _bookmarkEntryList.remove(Player.getPlayer().getBookmarkIndex(name[i]));
	    
	}

	
	_tableView.clearSelection();
	_dataModel.fireTableDataChanged();
    }

}

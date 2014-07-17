/*
 * @(#)RemoteURL.java	1.4 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.ui.player;

import javax.swing.JComboBox;
import java.net.URL;
import java.net.URLConnection;
import java.awt.event.*;
import com.sun.javaws.debug.Debug;
import com.sun.javaws.ConfigProperties;
import javax.swing.JTextField;
// Moved out from AdvancedPanel.java
class RemoteURL extends JComboBox {

    private boolean _load = false;

    RemoteURL() {
	
	setEditable(true);

	final JTextField jt = (JTextField)getEditor().getEditorComponent();

	jt.addKeyListener(new KeyAdapter() {

		public void keyPressed(KeyEvent e) {
		    if (e.getKeyCode() == KeyEvent.VK_ENTER) {		
		
			String [] remoteURLs = ConfigProperties.getInstance().getRemoteURLs();		  
		
			// handle reload - user press enter
			if (jt.getText().equals(remoteURLs[0])) {
			    
			    EntryManager manager = Player.getPlayer().getCurrentManager();
			    if (manager instanceof HTMLEntryManager) {
				// stop the current loading of HTML entries
				// if it has not finished yet
				((HTMLEntryManager)manager).stop();
				
				Player.getPlayer().resetHTMLEntryManager();
			    }
			}
		    }
		}
	    });
	

	addActionListener(new ActionListener() {
	    public void actionPerformed(ActionEvent e) {
		Object item = getSelectedItem();
		if (item != null) {
		   
		    // changed to Downloaded Application view if user enter
		    // nothing
		    if (((String)item).trim().equals("")) {
			_load = true;
			Player.getPlayer().setSelectedManagerIndex(1);
			return;
		    }

	
		    addToList(item.toString());
		    String [] remoteApps = getRemoteURLs();
		    
		    String [] remoteURLs = ConfigProperties.getInstance().getRemoteURLs();
		    boolean changed = false;
		    int len = ConfigProperties.getMaxUrlsLength();
		    for (int i=0; i<len; i++) {		
			if (remoteApps[i] != null && !remoteApps[i].equals(remoteURLs[i])) {
			    changed = true;
			}
		    }
		    // make sure it will load the new html page if
		    // it was in downloaded apps before
		    if (_load) {
			changed = true;
			_load = false;
		    }
		    if (changed) {
			EntryManager manager = Player.getPlayer().getCurrentManager();
			if (manager instanceof HTMLEntryManager) {
			    // stop the current loading of HTML entries
			    // if it has not finished yet
			    ((HTMLEntryManager)manager).stop();
			}
			if (!(manager instanceof HTMLEntryManager)) {
			    Player.getPlayer().setSelectedManagerIndex(0);
			}
			ConfigProperties.getInstance().setRemoteURLs(remoteApps);
			Player.getPlayer().resetHTMLEntryManager();
		    }
		}
	    }
	}); 

	setRemoteURLs(ConfigProperties.getInstance().getRemoteURLs());

    }
    
    String [] getRemoteURLs() {
	int len = ConfigProperties.getMaxUrlsLength();
	String [] urls = new String[len];
	for (int i=0; i<len; i++) {
	    urls[i] = (String) getItemAt(i);
	}
	return urls;
    }
    void setRemoteURLs(String[] urls) {
	removeAllItems();
	for (int i=0; i<urls.length; i++) {
	    if (urls[i] != null && urls[i].length() > 0) {
		addItem(urls[i]);
	    }
	}
    }
    
    void addToList(String text) {
	int count;
	int maxLen = ConfigProperties.getMaxUrlsLength();
	
	if (!isValid(text)) {
	    removeItem(text);
	    return;
	}
	insertItemAt(text, 0);
	count = getItemCount();
	for (int i=count-1; i>0; i--) {
	    if (text.equals(getItemAt(i)) || i >= maxLen) {
		removeItemAt(i);
	    }
	}
	setSelectedIndex(0);
	getEditor().setItem(text);
    }
    
    // only test for a well formed URL
    boolean isValid(String urlString) {
	try {
	    URL url = new URL(urlString);
	    URLConnection connection = url.openConnection();
	    return true;
	} catch (Exception e) {
	    return false;
	}
    }
}

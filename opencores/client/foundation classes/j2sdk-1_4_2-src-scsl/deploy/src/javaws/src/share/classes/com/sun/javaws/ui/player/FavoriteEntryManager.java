/*
 * @(#)FavoriteEntryManager.java	1.4 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.ui.player;

import com.sun.javaws.*;
import java.io.*;
import java.net.*;
import java.util.*;
import java.net.*;
import javax.swing.*;
import com.sun.javaws.debug.*;
import com.sun.javaws.jnl.*;
import com.sun.javaws.cache.InstallCache;

/**
 * An EntryManager where the set of JNLEntrys can added/removed by the user.
 * The initial list is obtained from the ConfigProperties
 * (<code>getFavorites</code>).
 *
 * @version 1.18 02/14/01
 */
public class FavoriteEntryManager extends EntryManager {
    /**
     * This list of JNLEntries obtained from the cache.
     */
    private ArrayList _entries;
    /**
     * Used to load JNLEntry's that are not in the local cache.
     */
    private RemoteLoader _loader;
    
    public FavoriteEntryManager() {
        _entries = new ArrayList();
    }
    
    /**
     * Reloads the JNLEntries from the ConfigProperties. If this is invoked
     * before <code>save</code>, there is the possibility entries will be
     * lost.
     */
    public void refresh() {
        JNLEntry selEntry = getSelectedEntry();
        URL sCodebase = (selEntry != null) ? selEntry.getCodebase() : null;
        
        setSelectedIndex(-1);
        
        _entries.clear();
        
        Iterator favs = ConfigProperties.getInstance().getFavorites();
        
        if (favs != null) {
            while (favs.hasNext()) {
                try {
                    URL codebase = new URL((String)favs.next());
                    LaunchDesc ld = InstallCache.getCache().getLaunchDesc(codebase, null);
                    if (ld != null) {
                        LocalApplicationProperties lap = InstallCache.
                            getCache().getLocalApplicationProperties
                            (codebase, ld);
                        InformationDesc id = ld.getInformation();
                        if (id != null && lap != null &&
                                (ld.getLaunchType() == LaunchDesc.APPLICATION_DESC_TYPE ||
                                     ld.getLaunchType() == LaunchDesc.APPLET_DESC_TYPE)) {
                            _entries.add(new JNLEntry(ld, id, lap,
                                                      codebase, 0l));
                        }
                    }
                    else {
                        if (_loader == null) {
                            _loader = new RemoteLoader();
                        }
                        _loader.loadJNLEntry(codebase);
                    }
                } catch (Exception ooops) {
                    // System.out.println("Oops: " + ooops);
                }
            }
	    Collections.sort(_entries);
        }
        
        fireStateChanged();
        
        if (sCodebase != null) {
            selectEntryWithCodebase(sCodebase);
        }
        if (getSelectedIndex() == -1 && size() > 0) {
            setSelectedIndex(0);
        }
    }
    
    /**
     * Returns the JNLEntry at the specified index.
     */
    public JNLEntry get(int index) {
        return (JNLEntry)_entries.get(index);
    }
    
    /**
     * Returns the number of JNLEntries.
     */
    public int size() {
        return _entries.size();
    }
    
    /**
     * Yes, we support adding.
     */
    public boolean canAdd() {
        return true;
    }
    
    /**
     * Adds the new entry as long as it isn't currently in the set of
     * favorites.
     */
    public void add(int index, JNLEntry entry) {
        if (!_entries.contains(entry)) {
            _entries.add(index, entry);
	    Collections.sort(_entries);
            save();
        }
    }
    
    /**
     * Yes, we support removing.
     */
    public boolean canRemove() {
        return true;
    }
    
    /**
     * Returns a string that can be presented to the user when an entry
     * is removed.
     */
    public String getRemoveString() {
        return Resources.getString("player.FavoriteEntryManager.remove");
    }
    
    /**
     * Removes the specified entry.
     */
    public void remove(int index) {
        _entries.remove(index);
        super.remove(index);
        save();
    }
    
    /**
     * Removes the JNLEntry identified by <code>codebase</code>.
     */
    public void remove(URL codebase) {
        if (codebase != null) {
            String codebaseS = codebase.toExternalForm();
            
            for (int counter = size() - 1; counter >= 0; counter--) {
                JNLEntry entry = get(counter);
                
                if (entry != null && entry.getCodebase() != null &&
                    entry.getCodebase().toExternalForm().equals(codebaseS)) {
                    remove(counter);
                    break;
                }
            }
        }
    }
    
    /**
     * Saves the entries to the ConfigProperties. The ConfigProperties aren't
     * saved though.
     */
    public void save() {
        int size = _entries.size();
        ArrayList names = new ArrayList(size);
        for (int counter = 0; counter < size; counter++) {
            names.add(get(counter).getCodebase().toString());
        }
        ConfigProperties.getInstance().refreshIfNecessary();
        ConfigProperties.getInstance().setFavorites(names.iterator());
    }
    
    /**
     * This is invoked from the parser as a new entry is found. As this
     * can come in on any thread, <code>SwingUtilities.invokeLater</code>
     * is used before the entry is added.
     */
    void addEntry(final JNLEntry entry) {
        SwingUtilities.invokeLater(new Runnable() {
                    public void run() {
                        _entries.add(entry);
        		JNLEntry selEntry = getSelectedEntry();
			Collections.sort(_entries);
                        fireStateChanged();
			if (selEntry != null) {
			    setSelectedIndex(_entries.indexOf(selEntry));
			} else if (getSelectedIndex() == -1 && size() > 0) {
                            setSelectedIndex(0);
                        }
                    }
                });
    }
    
    
    private class RemoteLoader implements Runnable {
        /** This is true while the other Thread is running. */
        private boolean _running;
        /** List of URLs (as Strings) to images that should be loaded. */
        private ArrayList _toLoad;
        
        public RemoteLoader() {
            _toLoad = new ArrayList();
        }
        
        public void loadJNLEntry(URL url) {
            boolean start = false;
            synchronized(RemoteLoader.this) {
                _toLoad.add(url);
                if (!_running) {
                    _running = true;
                    start = true;
                }
            }
            if (start) {
                new Thread(RemoteLoader.this).start();
            }
        }
        
        public void run() {
            boolean done = false;
            while (!done) {
                URL url = null;
                synchronized(RemoteLoader.this) {
                    if (_toLoad.size() > 0) {
                        url = (URL)_toLoad.remove(0);
                    }
                    else {
                        done = true;
                        _running = false;
                    }
                }
                if (!done) {
                    handleLoadJNLEntry(url);
                }
            }
        }
        
        private void handleLoadJNLEntry(URL url) {
            try {
                LaunchDesc ld = LaunchDescFactory.buildDescriptor(url);
                // Ignore those without a home as we won't be able to
                // lookup the properties file in the cache.
                if (ld != null && ld.getCanonicalHome() != null &&
                        (ld.getLaunchType() == LaunchDesc.APPLICATION_DESC_TYPE ||
                        ld.getLaunchType() == LaunchDesc.APPLET_DESC_TYPE)) {
                        InformationDesc id = ld.getInformation();
                        LocalApplicationProperties lap = InstallCache.getCache().
                            getLocalApplicationProperties
                            (ld.getCanonicalHome(), ld);
                        if (id != null && lap != null) {
                            JNLEntry je = new JNLEntry(ld, id, lap, null, 0l,
                                                       false, url);
                            addEntry(je);
                        }
                    }
            } catch (Exception e) {
                // Assume its invalid
                if (Globals.TraceHTMLEntryParsing) {
                    Debug.println("Error loading JNL file from " + url +
                                      " error " + e);
                    e.printStackTrace();
                }
                
            }
        }
    }
}

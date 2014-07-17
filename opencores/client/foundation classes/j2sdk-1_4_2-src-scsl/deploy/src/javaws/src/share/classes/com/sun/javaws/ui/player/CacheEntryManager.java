/*
 * @(#)CacheEntryManager.java	1.4 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws.ui.player;

import com.sun.javaws.*;
import com.sun.javaws.debug.*;
import java.io.*;
import java.net.*;
import java.util.*;
import javax.swing.*;
import com.sun.javaws.jnl.*;
import com.sun.javaws.cache.CachedApplication;
import com.sun.javaws.cache.InstallCache;

/**
 * An EntryManager that will obtain its list of JNLEntries from the cache, eg
 * this is a list of the JNL apps that have previously been run. This will
 * start a thread that checks if the cache has changed, and if so reload
 * it self (currently checks every 10 seconds).
 *
 * @version 1.20 02/14/01
 */
public class CacheEntryManager extends EntryManager {
    /**
     * This list of JNLEntries obtained from the cache.
     */
    private ArrayList _entries;
    
    public CacheEntryManager() {
        _entries = new ArrayList();
        new Thread(new CacheChecker()).start();
    }
    
    /**
     * Recreates the JNL entries from the Cache.
     */
    public void refresh() {
        JNLEntry selEntry = getSelectedEntry();
        URL sCodebase = (selEntry != null) ? selEntry.getCodebase() : null;

        setSelectedIndex(-1);
        
        _entries.clear();
        
        InstallCache cache = InstallCache.getCache();
        if (cache != null) {
            Iterator cachedApps = cache.getCachedApplications();
            if (cachedApps != null) {
                while (cachedApps.hasNext()) {
                    CachedApplication cachedApp = (CachedApplication)
                        cachedApps.next();
                    LaunchDesc ld = cachedApp.getLaunchDescriptor();
                    if (ld != null && (ld.getLaunchType() == LaunchDesc.APPLICATION_DESC_TYPE ||
                                      ld.getLaunchType() == LaunchDesc.APPLET_DESC_TYPE)) {
                        InformationDesc id = ld.getInformation();
                        if (id != null) {
                            URL codebase = cachedApp.getCodebase();
                            LocalApplicationProperties lap = cachedApp.
                                         getLocalApplicationProperties();

                            lap.refreshIfNecessary();
                            _entries.add(new JNLEntry(ld, id, lap,
                                                      codebase, 0l));
                        }
                    }
                }
                Collections.sort(_entries);
            }
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
     * Yes, we allow removing entries.
     */
    public boolean canRemove() {
        return true;
    }
    
    /**
     * Returns a string that can be presented to the user when an entry
     * is removed.
     */
    public String getRemoveString() {
        return Resources.getString("player.CacheEntryManager.remove");
    }

    /**
     * Removes the specified JNLEntry from the cache.
     */
    public void remove(int index) {
        if (index >= 0 && index < _entries.size()) {
            JNLEntry entry = get(index);
            LocalApplicationProperties lap = entry.
                                       getLocalApplicationProperties();
            LaunchDesc ld = entry.getLaunchDescriptor();
            LocalInstallHandler lih = LocalInstallHandler.getInstance();

            // Do an uninstall first.
            if (lap.isLocallyInstalled() && lih != null) {
                lih.uninstall(ld, lap);
                try {
                    lap.store();
                } catch (IOException ioe) { }
            }
            InstallCache.getCache().remove(lap, ld);
            _entries.remove(index);
            super.remove(index);
        }
    }
    
    
    /**
     * Time before checking if the cache has changed.
     */
    private static final int SLEEP_DELAY = 10000;
    
    /**
     * Checks for changes to the cache, and invokes <code>refresh</code>
     * when the lastAccessed time of the cache has changed.
     */
    private class CacheChecker implements Runnable {
        private long _lastTime;
        
        CacheChecker() {
            _lastTime = InstallCache.getCache().getLastAccessed();
        }
        
        public void run() {
            boolean done = false;
            while (!done) {
                try {
                    Thread.sleep(SLEEP_DELAY);
                } catch (InterruptedException ie) {
                    done = true;
                }
                long newTime = InstallCache.getCache().getLastAccessed();
                if (newTime != _lastTime) {
                    _lastTime = newTime;
                    if (Globals.TraceCacheEntryManager) {
                        Debug.println("Date stamp changed: updating");
                    }
                    SwingUtilities.invokeLater(new Runnable() {
                                public void run() {
                                    refresh();
                                }
                            });
                }
            }
        }
    }
}

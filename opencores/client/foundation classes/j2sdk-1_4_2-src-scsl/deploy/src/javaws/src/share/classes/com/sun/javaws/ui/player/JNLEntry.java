/*
 * @(#)JNLEntry.java	1.5 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.ui.player;

import com.sun.javaws.*;
import com.sun.javaws.debug.Globals;
import com.sun.javaws.debug.Debug;
import com.sun.javaws.cache.InstallCache;
import com.sun.javaws.jnl.*;
import com.sun.javaws.ui.general.GeneralUtilities;
import com.sun.javaws.util.URLUtil;
import java.io.File;
import java.net.*;
import javax.swing.*;

/**
 * JNLEntry represents a JNL file.
 *
 * @version 1.28 05/07/01
 */
public class JNLEntry implements Comparable {
    private String  _title;
    private String  _vendor;
    private boolean _offline; // Can be run off-line
    private String  _home;
    private String _description;
    private int _downloadSize;
    private URL _codebase;
    private LaunchDesc _launchDescriptor;
    private LocalApplicationProperties _lap;
    private long _mostRecentUsed; // A most recent used timesamp
    private boolean _cached;
    private URL _launchHome;
    private IconDesc _imageLoc;
    
    
    public JNLEntry() {
    }
    
    public JNLEntry(LaunchDesc ld, InformationDesc id,
                    LocalApplicationProperties lap, URL codebase,
                    long mostRecentUsed) {
        this(ld, id, lap, codebase, mostRecentUsed, true, null);
    }
    
    public JNLEntry(LaunchDesc ld, InformationDesc id,
                    LocalApplicationProperties lap, URL codebase,
                    long mostRecentUsed, boolean cached, URL launchHome) {
        URL url;
        _lap = lap;
        _title = id.getTitle();
        _vendor = id.getVendor();
        _offline = id.supportsOfflineOperation();
        _cached = cached;
        url = id.getHome();
        if (url != null) {
            _home = url.toExternalForm();
        }
        _launchHome = launchHome;
        _description = id.getDescription(InformationDesc.DESC_SHORT);
        _downloadSize = (int)LaunchDownload.getCachedSize(ld);
        _imageLoc = id.getIconLocation(InformationDesc.ICON_SIZE_LARGE,
                                       IconDesc.ICON_KIND_DEFAULT);
        // The codebase here is really the location of the JNLP file. Use the
        // one specified in the JNLP file - if not explicit given.
                        
        if (ld.getLocation() != null) {
            _codebase = ld.getLocation();
        } else {
            // If not given, default to URL passed in
            _codebase = codebase;
        }
        _launchDescriptor = ld;
        _mostRecentUsed = mostRecentUsed;
    }
    
    public LocalApplicationProperties getLocalApplicationProperties() {
        return _lap;
    }
    
    public LaunchDesc getLaunchDescriptor() {
        return _launchDescriptor;
    }
    
    public IconDesc getImageLocation() {
        return _imageLoc;
    }
    
    public String getTitle() {
        return _title;
    }
    
    public String getVendor() {
        return _vendor;
    }
    
    public String getDescription() {
        return _description;
    }
    
    public boolean canRunOffline() {
        return _offline;
    }
    
    public String getHome() {
        return _home;
    }
    
    public int getDownloadSize() {
        return _downloadSize;
    }
    
    public boolean isCached() {
        return _cached;
    }
    
    public URL getCodebase() {
        return _codebase;
    }
    
    public long getMostRecentUsed() {
        return _mostRecentUsed;
    }
    
    /**
     * Shows the home defined in the JNL file by bringing up a browser pointing
     * out the home URL.
     */
    public void showHome() {
        String home = getHome();
        if (home != null) {
            try {
                URL url = new URL(home);
                BrowserSupport.showDocument(url);
            } catch (MalformedURLException murle) {
                GeneralUtilities.showMessageDialog(null,
                    Resources.getString("player.invalidHome"),
                    Resources.getString("appname"), 
		    JOptionPane.ERROR_MESSAGE);
            }
        }
    }
    
    /**
     * Launchs the application this JNL file represents.
     */
    public boolean launch() {
        try {
            String jnlp_arg = null;
            // Check if launch file is in cache. Otherwise, just use URL.
            // (Note: It might be better to download the file. E.g. this should
            // be factored and overwritten in the RemoveJNLEntry)
            if (_codebase != null) {
                File file = InstallCache.getCache().
                    getCachedLaunchedFile(_codebase);
                if (file != null) {
                    jnlp_arg = file.toString();
                }
            }
            if (jnlp_arg == null && _launchHome != null) {
                jnlp_arg = _launchHome.toString();
            }
            else if (jnlp_arg == null) {
                return false;
            }                      
       
	    // fix for 4481188
	    String cmd[] = new String[2];
	    cmd[0] = ConfigProperties.getStartCommand();
	    cmd[1] = jnlp_arg;	 
	    Runtime.getRuntime().exec(cmd);
	
            return true;
        } catch (java.io.IOException ioe) {
            // FIXIT: Should show an error!
            Debug.ignoredException(ioe);
        }
        return false;
    }
    
    public int hashCode() {
        URL url = getCodebase();
        if (url != null) {
            return url.hashCode();
        }
        return super.hashCode();
    }
    
    public boolean equals(Object o) {
        if (o instanceof JNLEntry) {
            URL codebase = getCodebase();
            URL oCodebase = ((JNLEntry)o).getCodebase();
	    // use URLUtil.equals to compare URLs to avoid slow
	    // DNS lookup which will not work offline.  This 
	    // function checks for null URL too.
	    return URLUtil.equals(codebase, oCodebase);
        }
        return super.equals(o);
    }
    
    public String toString() {
        return getClass().getName() + "[ " + getTitle() + ", " + getVendor() +
            ", " + getDescription() + " ]";
    }

    public int compareTo(Object obj) {
	JNLEntry other = (JNLEntry) obj;
	return getTitle().compareTo(other.getTitle());
    }
}

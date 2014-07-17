/*
 * @(#)UpdateChecker.java	1.13 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.autoupdater;

/**
 *  This class holds the update information available from the file.
 *  Basically it reads and parses the update file and creates a table
 *  on its own, which is cached or periodically can be used to check
 *  if the update file has been changed since the last read.
 */

/**
 * Assumption Props file format.
 * auto-update.0.name=javaws
 * auto-update.0.new-version=1.0_01
 * auto-update.0.url=http://whatever
 * auto-update.0.current-version=1.0-beta
 * auto-update.0.os=Linux
 * auto-update.0.locale=En
 * Therefore we do this:
 * COMP_NAME_PREFIX.index.NAME_SUFFIX
 */
import java.util.Properties;
import java.net.URL;
import java.net.MalformedURLException;
import java.io.BufferedInputStream;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import com.sun.javaws.ConfigProperties;
import com.sun.javaws.debug.Globals;
import com.sun.javaws.debug.Debug;
import com.sun.javaws.JavawsFactory;
import com.sun.javaws.net.HttpRequest;
import com.sun.javaws.net.HttpResponse;
import com.sun.javaws.util.GeneralUtil;
import com.sun.javaws.util.VersionID;
import com.sun.javaws.util.VersionString;

public class UpdateChecker {
    /**
     * Debug option TraceAutoUpdater
     */
    private static final String COMP_NAME_PREFIX="auto-update";
    private static final String NAME_SUFFIX = "name";
    private static final String NEW_VERSION_SUFFIX = "new-version";
    private static final String DOWNLOAD_URL_SUFFIX = "url";
    private static final String CURRENT_VERSION_SUFFIX = "current-version";
    private static final String OS_NAME_SUFFIX = "os";
    private static final String ARCH_SUFFIX = "arch";
    private static final String LOCALE_SUFFIX = "locale";
    private static final char   DOT = '.';
    
    // Help class to hold the value for each entry
    
    private Properties doRead() {
        Properties props = null;
        ConfigProperties cp = ConfigProperties.getInstance();
        String urlfile = cp.getAutoUpdateURL();
        
        BufferedInputStream is = null;
        
        try {
	    HttpRequest hr = JavawsFactory.getHttpRequestImpl();
	    is = hr.doGetRequest(new URL(urlfile)).getInputStream();
        } catch(MalformedURLException e) {
	    // Try to open as file
	    try {
		is = new BufferedInputStream(new FileInputStream(urlfile));
	    } catch (FileNotFoundException fnfe) {
		Debug.ignoredException(fnfe);
		return null; // Failed
	    }
        } catch (IOException ioe) {
	    Debug.ignoredException(ioe);
	    return null;  // Failed
        }
        
        if (is != null) {
	    try {
		props = new Properties();
		props.load(is);
		if (Globals.TraceAutoUpdater) {
		    props.list(System.err);
		}
	    } catch (IOException ioe) {
		Debug.ignoredException(ioe);
		return null;  // Failed
	    }
	    
	    try { is.close(); }
	    catch (IOException ioe) {}
        }
        return props;
    }
    
    private boolean isMatch(String name, String curVersion, String osValue, String archValue, String localeValue) {
	/* Check name */
	if (name == null || !name.equalsIgnoreCase(Globals.getComponentName())) {
	    return false;
	}
	
	/* Check version */
	VersionID vid = new VersionID(Globals.getBuildID());
	if (curVersion == null || !VersionString.contains(curVersion, Globals.getBuildID())) {
	    return false;
	}
	
        String[] os = GeneralUtil.getStringList(osValue);
        String[] arch = GeneralUtil.getStringList(archValue);
        String[] locale = GeneralUtil.getStringList(localeValue);
        
	/* Check os, arch, and locale */
        return
	    GeneralUtil.prefixMatchStringList(os, Globals.getOperatingSystemID()) &&
	    GeneralUtil.prefixMatchStringList(arch, Globals.getArchitectureID()) &&
	    GeneralUtil.matchLocale(locale, Globals.getLocale());
    }
    
    /**
     * If a match is found return the URL and New Version else
     * return a null
     */
    private String[] findUpdateRecord(Properties props) {
	int idx = 0;
	String name = null;
	do {
	    String prefix = COMP_NAME_PREFIX + DOT + idx + DOT;
	    /*         */ name = props.getProperty(prefix + NAME_SUFFIX);
	    String curVersion  = props.getProperty(prefix + CURRENT_VERSION_SUFFIX);
	    String osValue     = props.getProperty(prefix + OS_NAME_SUFFIX);
	    String archValue   = props.getProperty(prefix + ARCH_SUFFIX);
	    String localeValue = props.getProperty(prefix + LOCALE_SUFFIX);
	    String newVersion  = props.getProperty(prefix + NEW_VERSION_SUFFIX);
	    String dlURL       = props.getProperty(prefix + DOWNLOAD_URL_SUFFIX);
	    
	    if (isMatch(name, curVersion, osValue, archValue, localeValue)) {
		String[] out = new String[2];
		if (dlURL != null && newVersion != null) {
		    out[0] = dlURL ;
		    out[1] = newVersion;
		    return out;
		}
	    }
	    idx++;
	} while(name != null);
	return null;
    }
    
    String[] isUpdateAvailable() {
	Properties props = doRead();
	if (props == null) return null;
	return findUpdateRecord(props);
    }
}


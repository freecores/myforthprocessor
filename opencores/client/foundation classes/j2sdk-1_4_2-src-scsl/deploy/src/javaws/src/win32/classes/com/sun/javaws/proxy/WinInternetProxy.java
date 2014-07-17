/*
 * @(#)WinInternetProxy.java	1.14 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.proxy;

import java.io.*;
import java.util.*;
import java.net.*;

import com.sun.javaws.WinRegistry;
import com.sun.javaws.debug.*;

/**
 * Looks up whatever information is available about the default
 * HTTP proxy server address.  For IE this means consulting a few
 * registry entries, for Netscape this means finding the JavaScript
 * preferences file and looking up the address information there.
 *
 * @version 1.7, 02/09/01
 */
public class WinInternetProxy extends InternetProxy {

    public File getNS6RegFile() {
	if (getNSVersion() >= 6) {
	    String windowsDir = null;
	    // special case for windows 95	
	    if (System.getProperty("os.name").equals("Windows 95")) {
		windowsDir = System.getProperty("user.home");
		
	    } else {
		
		windowsDir = WinRegistry.getString(WinRegistry.HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders", "AppData");
	    }
	    windowsDir += File.separator + "MOZILLA";
	    	 	    
	    if (windowsDir != null) {
		return new File(windowsDir, "registry.dat");
	    }
	}

	return null;
	
    }


    /**
     * Returns the NS Navigator version as a float if it was possible
     * to parse the string, -1.0 otherwise.  A typical version string
     * is "4.61 (en)".
     */
    private static float getNSVersion() {
	// NS 6.x
	String path = "Software\\Netscape\\Netscape 6";
	String s = WinRegistry.getString(WinRegistry.HKEY_LOCAL_MACHINE, path, "CurrentVersion");

	// NS 4.x
	if (s == null) {
	    path = "Software\\Netscape\\Netscape Navigator";

	    s = WinRegistry.getString(WinRegistry.HKEY_LOCAL_MACHINE, path, "CurrentVersion");

	}

	if (Globals.TraceProxies) Debug.println("NS version string: " + s);

	// no NS version found
	if (s == null) return -1.0f;
	
	int start = -1, end = -1;
	boolean dot = false;
	/* Find the beginning and the end of the version number. */
	for(int i = 0; (i < s.length()) && (end == -1); i++) {
	    char c = s.charAt(i);
	    // 6.x.y -> 6.x
	    if (c == '.' && start != -1 && dot == true) {
		end = i;
		break;
	    }
	    if (c == '.') dot = true;
	    boolean isFloatChar = Character.isDigit(c) || (c == '.');
	    if (start == -1) {
		if (isFloatChar) {
		    start = i;
		}
	    }
	    else if (!isFloatChar) {
		end = i;
	    }


	}
	/* Parse */
	if ((start != -1) && (end > start)) {
	    try {
		
		return Float.parseFloat(s.substring(start, end));
	    }
	    catch (NumberFormatException e) {
		return -1.0f;
	    }
	}
	else {	    
	    return -1.0f;
	}
    }
    
  
    
    /**
     * Return the location of the "prefs.js" user profile file in the
     * netscape registry or null if we can't figure that out.  This method
     * should work with versions 4.5 - 4.7 of Navigator.
     */
    private static File getNSPrefsFile(File registryFile) {
	NSRegistry reg = new NSRegistry().open(registryFile);
	String path = null;
	if (reg != null) {
	    String user = reg.get("Common/Netscape/ProfileManager/LastNetscapeUser");
	    if (user != null) {
		path = reg.get("Users/" + user + "/ProfileLocation");
	    }
	    reg.close();
	}
	return (path != null) ? new File(path, "prefs.js") : null;
    }
    
    
    /**
     * Return the location of the "prefs.js" user profile file or null
     * if we can't figure that out.  The directory that contains this file
     * is found under this registry entry:
     * <pre>
     *     Software\Netscape\Netscape Navigator\Users\<CurrentUser>\DirRoot
     * </pre>
     * This method should work with versions 4.0.x of Navigator.
     */
    private static File getNS40PrefsFile() {
	String path = null;
	int key = WinRegistry.HKEY_LOCAL_MACHINE;
	String usersPath = "Software\\Netscape\\Netscape Navigator\\Users";
	String currentUser = WinRegistry.getString(key, usersPath, "CurrentUser");
	if (currentUser != null) {
	    String userPath = "Software\\Netscape\\Netscape Navigator\\Users\\" + currentUser;
	    path = WinRegistry.getString(key, userPath, "DirRoot");
	}
	return (path != null) ? new File(path, "prefs.js") : null;
    }
    
    
    /**
     * Parse the "host:port" format address string and set the httpHost and
     * httpPort InternetProxyInfo properties if possible.  Not that the ":port"
     * part of the address is optional in the sense that the IE preferences
     * dialog will not gripe if it's not provided.
     */
     private static void parseIEProxyAddress(String adString,
                           InternetProxyInfo info) {
	 AutoConfigProxy.parseProxyAddress(adString, info);
     }
    
    /**
     * The value of the ProxyServer registry entry at
     * <pre>
     *     Software\Microsoft\Windows\CurrentVersion\Internet Settings
     * </pre>
     * is either a ';' separated list of proxy addresses per protocol,
     * like "ftp=host:port;http=host:port" or a single address.  The latter
     * is used when the user selects the "use same address for all proxies
     * checkbox.  In all cases the ":port" part of the address is optional.
     */
    private static void parseIEProxyServer(String server, InternetProxyInfo info) {
	if (server.indexOf("=") != -1) {
	    StringTokenizer st = new StringTokenizer(server, ";");
	    while(st.hasMoreTokens()) {
		String s = st.nextToken();
		if (s.startsWith("http=")) {
		    parseIEProxyAddress(s.substring(5, s.length()), info);
		}
	    }
	}
	else {
	    parseIEProxyAddress(server, info);
	}
    }
    
    
    /**
     * Return all of the available information about the internet proxy addresses.
     * This is as simple as looking up a few registry entries for Internet Explorer,
     * it's a fairly convoluted process for Netscape.
     */
    public InternetProxyInfo getBrowserInfo()
    {
	// First do platform independent lookup;
	InternetProxyInfo info = new InternetProxyInfo();
	
	/* Determine if the default browser is NS4.x or IE.  The registry key
	 * used for this is documented here:
	 *    http://help.netscape.com/kb/consumer/19980502-2.html
	 */
	String appPath = "http\\shell\\open\\command";
	int appKey = WinRegistry.HKEY_CLASSES_ROOT;
	String browser = WinRegistry.getString(appKey, appPath, "");
	
	if (Globals.TraceProxies) Debug.println("Browser is " + browser);
	
	/* Netscape
	 *
	 * All of the interestring Navigator information is stored in a private
	 * binary registry file, see the NSPreferences class for more information.
	 */
	if ((browser != null) &&
	    (browser.toLowerCase().indexOf("netscape") != -1 ||
	     browser.toLowerCase().indexOf("netscp") != -1 )) {
	    if (Globals.TraceProxies) Debug.println("Browser is Netscape");
	    float version = getNSVersion();
	    File file = null;
	    
	    if (Globals.TraceProxies) Debug.println("version: " + version);
	    if (version >= 6) {
		File regFile = getNS6RegFile();
		    
		if (regFile != null) {
		    file = getNS6PrefsFile(regFile);
		}
	    }
	    else if (version >= 4.5) {
		String windowsDir = WinRegistry.getWindowsDirectory();
		if (windowsDir != null) {
		    file = getNSPrefsFile(new File(windowsDir, "nsreg.dat"));
		}
	    }
	    else {
		file = getNS40PrefsFile();
	    }
	    if (file != null) {	
		NSPreferences.parseFile(file, info, version);
		return info;
	    }
	}
	    
	// Defaults to use IE settings if Netscape is not default browser

	/* Internet Explorer
	 *
	 * The Proxy{Server,Override,Enable} registry keys are covered by several
	 * MS web pages however only as a sidebar.  Here's an example:
	 *   http://support.microsoft.com/support/kb/articles/Q164/0/35.ASP
	 */
	
	if (Globals.TraceProxies) Debug.println("Browser is IE");
	String path = "Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings";
	int key = WinRegistry.HKEY_CURRENT_USER;
	// Somtimes ProxyEnable is an integer
	Integer proxyEnable = WinRegistry.getInteger(key, path, "ProxyEnable");
	String autoConfigEnable = WinRegistry.getString(key, path, "AutoConfigURL");
	if (autoConfigEnable != null) proxyEnable = new Integer(2);
	// Sometimes it is a word
	if (Globals.TraceProxies) Debug.println("IE proxyEnable: " + proxyEnable);
	if (proxyEnable != null) {
	    info.setType(proxyEnable.intValue());
	}
	if (info.getType() == InternetProxyInfo.MANUAL) {
	    String server = WinRegistry.getString(key, path, "ProxyServer");
	    if (Globals.TraceProxies) Debug.println("IE ProxyServer setting: " + server);
	    if (server != null) {
		    parseIEProxyServer(server, info);
	    }
	    String overrides = WinRegistry.getString(key, path, "ProxyOverride");
	    if (overrides != null) {
		StringTokenizer st = new StringTokenizer(overrides, ";");
		ArrayList list = new ArrayList();
		while (st.hasMoreTokens()) {
		    list.add(st.nextToken());
		}
		info.setOverrides(list);
	    }
	}
	else if (info.getType() == InternetProxyInfo.AUTO) {
	    String autoConfigURLString = WinRegistry.getString(key, path, "AutoConfigURL");
	    info.setAutoConfigURL(autoConfigURLString);
	    URL u = null;
	    try {
		u = new URL(autoConfigURLString);
	    } catch (MalformedURLException mue) {
		try {
		    u = new URL("http://" + autoConfigURLString);
		} catch (MalformedURLException mue2) {
		    Debug.ignoredException(mue2);
		}
	    }
	    if (u != null) {
		String s = AutoConfigProxy.getAutoConfigProxy(u);
		if (s != null) {
		    InternetProxyInfo temp = new InternetProxyInfo();
		    parseIEProxyServer(s, temp);
		    if (temp.getHTTPHost() != null) {
			info.setInvalidHTTPHost(temp.getHTTPHost());
			info.setInvalidHTTPPort(temp.getHTTPPort());
		    }
		}
	    }
	}
	
	return info;
    }
}

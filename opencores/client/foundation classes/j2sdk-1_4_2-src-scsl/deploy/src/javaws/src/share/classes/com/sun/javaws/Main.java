/*
 * @(#)Main.java	1.96 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws;

import java.io.*;
import java.net.*;
import java.util.*;
import java.awt.*;
import javax.swing.*;
import javax.jnlp.ServiceManager;
import com.sun.jnlp.JnlpLookupStub;
import com.sun.jnlp.JNLPClassLoader;
import com.sun.javaws.debug.*;
import com.sun.javaws.jnl.*;
import com.sun.javaws.proxy.*;
import com.sun.javaws.exceptions.*;
import com.sun.javaws.ui.player.Player;
import com.sun.javaws.ui.general.GeneralUtilities;
import com.sun.javaws.cache.InstallCache;
import com.sun.javaws.cache.CachedApplication;
import com.sun.javaws.security.AppContextUtil;
import com.sun.javaws.util.VersionString;


/*
 *   Main Entry point for Java Web Start
 *
 *   @version 1.96, 01/23/03
 */
public class Main {
    
    /** Main entry point for Java Web Start. It passes
     *  the arguments, and initiates either the Player
     *  UI or launches an application, or shows a JPDA
     *  debugging mode notification window (see first
     *  statement in main()).
     */

    private static String[] _tempfile = new String[1];

    static public void main(String args[]) {

	if (args.length > 0) {         
	     _tempfile[0] = args[0];
	}


        // awt will staticly save the ContextClassLoader in EventQueue.java, 
        // so install (even though not initialized yet) our JNLPClassLoader
        // as the ContextClassLoader before any reference to an awt object
        // that may cause that static initialization. (see bug #4665132)
        Thread.currentThread().setContextClassLoader(
		JNLPClassLoader.createClassLoader());
	Toolkit.getDefaultToolkit();
 

	// May pop up a "JPDA Notification" window:
	JPDA.setup();

        // Parse and remove debugging arguments
        // e.g arguments of the form -X<option>=<value>
        args = Globals.parseOptions(args);
        
        // Check for -Xclearcache argument. This is an unsupported argument
        // used by testing
        if (args.length > 0 && args[0].equals("-Xclearcache")) {
	    SplashScreen.hide();
	    try {
		InstallCache.getCache().remove();
		long cacheSize = InstallCache.getDiskCache().getCacheSize();
		if (cacheSize != 0) {
		    System.err.println("Could not clean all entries in cache since they are in use");
		    if (Globals.TCKHarnessRun) {
			Debug.tckprintln(Globals.CACHE_CLEAR_FAILED);
		    }
		    systemExit(-1);
		}
	    } catch(IOException ioe) {
		System.err.println("Clear cached failed: " + ioe.getMessage());
		if (Globals.TCKHarnessRun) {
		    Debug.tckprintln(Globals.CACHE_CLEAR_FAILED);
		}
		systemExit(-1);
	    }
	    if (Globals.TCKHarnessRun) {
		Debug.tckprintln(Globals.CACHE_CLEAR_OK);
	    }
	    systemExit(0);
        }
        
        // Check for -offline argument (This should get cleaned up)
        if (args.length > 0 && args[0].equals("-offline")) {
	    JnlpxArgs.SetIsOffline();
	    Globals.setOffline(true);
	    String[] na = new String[args.length - 1];
	    for(int i = 0; i < args.length -1; i++) {
		na[i] = args[i + 1];
	    }
	    args = na;
        }
        
        // Check for -Xnosplash argument (This should get cleaned up)
        if (args.length > 0 && args[0].equals("-Xnosplash")) {
	    String[] na = new String[args.length - 1];
	    for(int i = 0; i < args.length -1; i++) na[i] = args[i + 1];
	    args = na;
        }
        
        if (args.length > 0 && args[0].equals("-uninstall")) {
	    uninstall(args);
        }
        
	ConfigProperties cp = ConfigProperties.getInstance();

	boolean oldUpdateVersions = 
            (args.length > 0 && args[0].equals("-updateVersions"));

	if (oldUpdateVersions || !cp.isVersionCurrent()) {
	    // This is called when a new version is installed.

	    // register JNLP mimeTypes for Netscape 6
	    setupNS6();

	    // for any changes in cache format:
	    updateCache();

            // for any changes in configuration file format:
	    cp.updateConfiguration();

	    if (oldUpdateVersions) {
		systemExit(0);
	    }
        }

        // Check that all properties are setup correctly
        JnlpxArgs.verify();
        
        // Initialize JRE with the right proxies, etc.
        initializeExecutionEnvironment();
	
        //We need to let the TCK know we started up Java.
        if (Globals.TCKHarnessRun) {
	    Debug.tckprintln(Globals.JAVA_STARTED);
        }
        
        // Parse arguments.
        // Note: The proxies must be setup at this point, since
        // this might involve accessing a remote URL
	if (args.length == 0) {
	    // No arguments -> Launch Player
	    launchPlayer(args);
	} else {
	    
	    // Expecting exactly one argument
	    if (args.length > 1) {
		JnlpxArgs.removeArgumentFile(args);
		LaunchErrorDialog.show(null, new TooManyArgumentsException(args));
	    }
	    
	    // Load argument. First try to load it as a file, then as a URL
	    LaunchDesc ld = null;
	    try {
		ld = LaunchDescFactory.buildDescriptor(args[0]);	
	    } catch(IOException ioe) {
		JnlpxArgs.removeArgumentFile(args);
		// Failed to load argument. Show error message
		LaunchErrorDialog.show(null,
				       new CouldNotLoadArgumentException(args[0], ioe));
	    } catch(JNLPException jnlpe) {
		JnlpxArgs.removeArgumentFile(args);
		// Missing field in launch file
		LaunchErrorDialog.show(null, jnlpe);
	    }
	
	    // Alright, we got a launch descriptor.Check for internal types, 
	    //  e.g., if we should launch the player
	    if (ld.getLaunchType() == LaunchDesc.INTERNAL_TYPE) {
		JnlpxArgs.removeArgumentFile(args);
		// Only internal type supported is the Player
		launchPlayer(args);
	    } else {	
		// initialize a ThreadGroup and AppContext for security dialogs
		initializeSecurity();
		// Do launch
		new Launcher(ld, args);
	    }
	}
    }

    /** Launch player GUI */
    static private void launchPlayer(String[] args) {
        JnlpxArgs.removeArgumentFile(args);
        if (Globals.TraceStartup) Debug.println("Launching player");
        Player.main(args);
    }

    
    /**
     * Uninstalls the first application named in <code>args</code>
     */
    static private void uninstall(String[] args) {
        if (Globals.TraceLocalAppInstall) {
	    if (args.length == 1) {
		Debug.println("Uninstall all!");
	    }
	    else {
		Debug.println("Uninstall: " + args[1]);
	    }
        }
        
        if (args.length == 1) {
	    // Uninstall everything!
	    uninstallAll();
	    if (Globals.TCKHarnessRun) {
		Debug.tckprintln(Globals.CACHE_CLEAR_OK);
	    }
	    systemExit(0);
	    return;
        }
        LaunchDesc ld = null;
        try {
	    ld = LaunchDescFactory.buildDescriptor(args[1]);
        } catch(IOException io) {
	    Debug.ignoredException(io);
        } catch(JNLPException jnlpe) {
	    Debug.ignoredException(jnlpe);
        }
        if (ld != null) {
	    LocalApplicationProperties lap =  InstallCache.getCache().
		getLocalApplicationProperties(args[1], ld);
	    
	    if (lap != null) {
		SplashScreen.hide();
		InstallCache.getCache().remove(lap, ld);
		if (Globals.TCKHarnessRun) {
		    Debug.tckprintln(Globals.CACHE_CLEAR_OK);
		}
		// We are done!
		systemExit(0);
	    }
        }
        
        // Error uninstalling
        if (Globals.TraceLocalAppInstall) {
	    Debug.println("Error uninstalling!");
        }
	if (Globals.TCKHarnessRun) {
	    Debug.tckprintln(Globals.CACHE_CLEAR_FAILED);
	}
        SplashScreen.hide();
        GeneralUtilities.showMessageDialog(null,
					   Resources.getString("uninstall.failedMessage"),
					   Resources.getString("uninstall.failedMessageTitle"),
					   JOptionPane.ERROR_MESSAGE);
        systemExit(0);
    }
    
    /**
     * Uninstalls all the currently installed applications.
     */
    static private void uninstallAll() {
        InstallCache cache = InstallCache.getCache();
        if (cache != null) {
	    cache.remove();
        }
    }
    
    // register mimeTypes for NS6
    static private void setupNS6() {

	String mailCapInfo = null;

	mailCapInfo = BrowserSupport.getInstance().getNS6MailCapInfo();

	String mimeInfo = "user_pref(\"browser.helperApps.neverAsk.openFile\", \"application%2Fx-java-jnlp-file\");\n";


	// check if netscape 6 is installed
	String prefsDir = InternetProxy.getInstance().getNS6PrefsDir();

	if (prefsDir == null) return;

	// append prefs.js to the path
	String prefs_file = prefsDir + File.separator + "prefs.js";

	InputStream is = null;
	// try to open the file for read
	try {
	    String line = null;
	    is = new FileInputStream(prefs_file);
	    // read in the file
	    BufferedReader in = new BufferedReader(new InputStreamReader(is));
	 
	    // check if mimeType setting already exist
	    String contents = "";
	    boolean addMimeInfo = true;
	    boolean addMailCapInfo;
	    if (mailCapInfo == null) {
		addMailCapInfo = false;
	    } else {
		addMailCapInfo = true;
	    }
	    while (true) {
		// Get next line
		try {
		    line = in.readLine();
			  
		    if (line == null) {
			is.close();
			break;
		    }
		    contents += line + "\n";
		    if (line.indexOf("x-java-jnlp-file") != -1) {
			// already defined
			addMimeInfo = false;
		    }
		    if (mailCapInfo != null && line.indexOf(".mime.types") != -1) {
			addMailCapInfo = false;
		    }
		
		} catch (IOException ioe) {
		    ioe.printStackTrace();
		}
	    }

	    if (!addMimeInfo && !addMailCapInfo) {
		return;
	    }
	  
	    // add in new contens
	    if (addMimeInfo) contents += mimeInfo;

	    if (mailCapInfo != null && addMailCapInfo) {
		contents += mailCapInfo;
	    }

	    FileOutputStream fos = new FileOutputStream(prefs_file);
	    try {
		fos.write(contents.getBytes());
		fos.close();
	    } catch (IOException ioe) {
		ioe.printStackTrace();
	    }

	} catch (FileNotFoundException fnfe) {
	    // else create a new pref.js
	    Debug.ignoredException(fnfe);
	    String content = "";

	    // put in mailcap info (Unix only)
	    if (mailCapInfo != null) content += mailCapInfo;

	    // put in mimeTypes
	    content += mimeInfo;
	   
	    try {
		FileOutputStream fos = new FileOutputStream(prefs_file);
	    
		fos.write(content.getBytes());
		fos.close();
	    } catch (IOException ioe) {
		ioe.printStackTrace();
	    }
	}

    }
    
    /**
     * Updates the cache for any version specific changes
     */
    static private void updateCache() {
	/* no cache changes for this release */
    }
 
    
    /**
     * Initializes the execution environment.
     */
    static private void initializeExecutionEnvironment() {
	Properties p = System.getProperties();
	
	// fix for 4772298: proxy authentication dialog pop up twice even if username/password is correct
	p.put("http.auth.serializeRequests", "true");

	// Install https protocol handler if we are running 1.4+
        String vs_running = Globals.getJavaVersion();

	if (vs_running.startsWith("1.2") || vs_running.startsWith("1.3")) {
	    // do nothing
        } else {	   	   
	    String pkgs = (String) p.get("java.protocol.handler.pkgs");
	    if (pkgs != null) {
		p.put("java.protocol.handler.pkgs", pkgs + "|com.sun.javaws.net.protocol");
	    } else {
		p.put("java.protocol.handler.pkgs", "com.sun.javaws.net.protocol");
	    }
	}

	// Set Java Web Start version
	p.setProperty("javawebstart.version", Globals.getComponentName());
	
	// We loop until we get a valid proxy configuration
	boolean retry = true;
	while(retry) {
	    retry = false;
	    
	    // Check if we need to setup proxies
	    InternetProxyInfo info = InternetProxy.getInstance().getDefaultInfo();
	    if (Globals.TraceProxies) {
		Debug.println("ProxyInfo: " + info);
	    }
	    
	    if (info.getType() != com.sun.javaws.proxy.InternetProxyInfo.NONE) {
		// Is configuration OK
		if (info.isValidManualHTTPConfiguration() ||
		    info.isValidAutoHTTPConfiguration()) {
		    String host = info.getHTTPHost();
		    String port = String.valueOf(info.getHTTPPort());
		    String noProxyHostNames = info.getOverrides();
		    // Standard proxy host
		    p.put("proxyHost", host);
		    p.put("proxyPort", port);
		    p.put("trustProxy", "true");
		    // Proxy host for JSSE
		    p.put("https.proxyHost", host);
		    p.put("https.proxyPort", port);
		    		  
		    p.put("http.nonProxyHosts",noProxyHostNames);
		    p.put("https.nonProxyHosts",noProxyHostNames);
	
		} else {
		    if (Globals.TraceProxies) {
			Debug.println("No valid manual HTTP proxies");
		    }
		    ProxyDialog proxydialog = new ProxyDialog();
		    ConfigProperties.getInstance().refresh();
		    // Try again
		    retry = true;
		}
	    }
	}

        /**
         * Because we access the net in various places this is a good starting
         * point to set the JAuthenticator callback method.
         * We initialize the dialog to pop up for user authentication
         * to password prompted URLs.
         */
        JAuthenticator ja = JAuthenticator.getInstance((Frame)null);
	/**
	 * Note: Although we do this again in both Player and Launcher, with
	 * different (non-null) parents, the ja returned is the same so the
	 * extra calls to setDefault(ja) do nothing, even on merlin.
	 */
        Authenticator.setDefault(ja);
	
	// Initialize the JNLP API
	ServiceManager.setServiceManagerStub(new JnlpLookupStub());
	
	// Don't allow launched apps to load or define classes from com.sun.javaws if
	// a security manager gets installed
	addToSecurityProperty("package.access",     "com.sun.javaws");
	addToSecurityProperty("package.definition", "com.sun.javaws");
	addToSecurityProperty("package.definition", "com.sun.jnlp");
    }
    
    static private void addToSecurityProperty(String propertyName, String newValue) {
	String value = java.security.Security.getProperty(propertyName);
	if (Globals.TraceSecurity) {
	    Debug.println("property " + propertyName + " value " + value);
	}
	if (value != null) {
	    value = value + "," + newValue;
	}
	else {
	    value = newValue;
	}
	java.security.Security.setProperty(propertyName, value);
	if (Globals.TraceSecurity) {
	    Debug.println("property " + propertyName + " new value " + value);
	}
    }

    static public void systemExit(int status) {
	// fix for 4654173
	JnlpxArgs.removeArgumentFile(_tempfile);
	System.exit(status);
    }

    static public final ThreadGroup getSecurityThreadGroup() {
	return _securityTG;
    }

    static private void initializeSecurity() {

        _appTG = Thread.currentThread().getThreadGroup();
        for ( _systemTG = _appTG; 
              _systemTG.getParent() != null; 
              _systemTG = _systemTG.getParent()
            );

        _securityTG = new ThreadGroup(_systemTG, "javawsSecurityThreadGroup");

        new Thread(_securityTG, 
                   new Runnable() {
                        public void run() {
                            AppContextUtil.createSecurityAppContext();
                        }
                   }).start();
    }

    static private ThreadGroup _systemTG;
    static private ThreadGroup _securityTG;
    static private ThreadGroup _appTG;
}



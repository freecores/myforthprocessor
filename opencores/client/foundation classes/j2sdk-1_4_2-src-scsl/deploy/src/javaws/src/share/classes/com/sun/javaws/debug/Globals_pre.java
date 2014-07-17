/*
 * @(#)Globals_pre.java	1.80 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws.debug;
import java.util.Arrays;
import java.util.List;
import java.util.ArrayList;
import java.util.ListIterator;
import java.util.Properties;
import java.util.Enumeration;
import java.util.Locale;
import java.lang.reflect.Field;
import java.lang.reflect.Modifier;
import java.lang.NoSuchFieldException;
import java.io.FileInputStream;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.FileNotFoundException;
import java.io.File;

public class Globals {
    
    /**
     * The name of the Java Web Start Component, this is cast
     * in stone, if this changes then the whole updating schema
     * will have to change.
     */
    private static final String JAVAWS_NAME = "javaws-##RELEASE##";
      
    private static final String JAVAWS_VERSION = "##RELEASE##";

    // This needs to be updated whenever there is new release of the JNLP
    // specification
    private static final String JNLP_VERSION = "1.0.1";


    // Used to identify the windows platform.
    private static final String WIN_ID = "Windows";
    
    // State of Client: offline or online?
    private static boolean _isOffline = false;
    
    //Default Host and port to log to
    private static final String DEFAULT_LOGHOST = "localhost:8205";
    
    // List of testing flags
    //
    static public String BootClassPath   = "NONE"; // Allows for -Xbootclasspath
    static public String JCOV            = "NONE"; // Allows for -Xrunjcov
    
    // Tracing flags for various subsystems
    //
    // Warning: Flags that begins with 'Trace' or 'x' can be set from JNLP file as
    // a debugging aid. Don't make any flags that compromises security start with 'x'
    // or 'Trace'. This feature should probably be disabled for FCS.
    //
    static public boolean TraceSecurity           = false; // Trace security settings
    static public boolean TraceStartup            = false; // Trace start-up sequence
    static public boolean TraceSplashScreen       = false; // Trace splash screen code
    static public boolean TraceProxies            = false; // Trace proxy handler code
    static public boolean TraceCache              = false; // Trace the install cache
    static public boolean TraceMuffins            = false; // Trace muffin operations
    static public boolean TraceDiskCache          = false; // Traces low-level DiskCache
    static public boolean TraceAppletViewer       = false; // Trace appletviewer
    static public boolean TraceClassLoading       = false; // Trace classes loaded by the classloader
    static public boolean TraceJRESearch          = false; // Traces searching for a JRE in the prefs panel.
    static public boolean TraceExtensions         = false; // Traces the installation of extension
    static public boolean TraceHTMLEntryParsing   = false; // Traces HTMLEntryManager parsing of URLs in the player.
    static public boolean TracePlayerImageLoading = false; // Traces loading of the images (from JNL files) for the player
    static public boolean TraceCacheEntryManager  = false; // Traces checking of the cached jnlp files
    static public boolean TraceLocalAppInstall    = false; // Traces local installation of applications.
    static public boolean TraceDownload           = false; // Traces download of resources
    static public boolean TraceJarDiff            = false; // Trace JARDiff apply
    static public boolean TraceIgnoredExceptions  = false; // Trace exceptions that are either being ignored or rethrown in a different manner
    static public boolean TraceAutoUpdater        = false; // Traces the autoupdater hickups.
    static public boolean TraceXMLParsing         = false; // Traces xml parsing
    static public boolean TraceThreads            = false; // print thread IDs before Debug.println messages
    static public boolean TraceHttpsCert          = false; // Trace HTTPS certificate handling
    static public boolean TraceIcoCreation        = false; // Trace .ico file creation
    static public boolean TracePlayer             = false; // Trace player related features
    static public boolean TraceConfig             = false; // Trace Config and upgrade
    static public boolean TraceEncoding           = false; // Trace url encoding
    // Output/error redirection
    static public boolean RedirectOutput          = true;  // Redirect output to console window or dev/null
    static public boolean RedirectErrors          = false; // Redirect error window output to System.err instead of dialog
    static public String  LogToHost               = null;  //Format host:port or [ipaddress]:port for IPv6 literal address
    static public boolean DebugWindow             = false; // Pipe debug output in a Window instead of the standard out
    // General
    static public boolean SupportJREinstallation  = true;  // Wether automatic JRE installation is turned on or not
    // Flags that can be set from the JNLP file as added debugging help
    static public boolean OverrideSystemClassLoader  = true; // Sets system classloader to the JNLP ClassLoader (1.4 and above only)
    
    // TCK Flag which sets RedirectErrors and RedirectOutput
    static public boolean TCKHarnessRun           = false;

    // the workaroud for TCP flushing problem on windows
    //  bug #4396040
    //  used to check that TCK receive a message
    static public boolean TCKResponse     = false;

    //Enumerations for the TCK States reported only to the harness
    static public final String JAVA_STARTED             = "Java Started";
    static public final String JNLP_LAUNCHING           = "JNLP Launching";
    static public final String NEW_VM_STARTING          = "JVM Starting";
    static public final String JAVA_SHUTDOWN            = "JVM Shutdown";
    static public final String CACHE_CLEAR_OK           = "Cache Clear Success";
    static public final String CACHE_CLEAR_FAILED       = "Cache Clear Failed";
       
    static public String osName = getGeneralOsName();
    static public String osArch = System.getProperty("os.arch");
    
    private static String getGeneralOsName() {
	String osName = System.getProperty("os.name");
	if (osName != null && osName.startsWith("Windows")) {
	    osName = "Windows";
	}
	return osName;
    }

    // Global helper methods
    
    /** Returns true if client is offline */
    static public boolean isOffline() { return _isOffline; }
    
    /** Set offline/online state */
    static public void setOffline(boolean s) { _isOffline = s; }
    
    // System properties used in various places. We read them once to avoid
    // security exceptions
    private static String _osNameProperty;
    private static String _osArchProperty;
    private static String _javaVersionProperty;
        
    static private String getProperty(String key) {
	return System.getProperty(key);
    }
    /**
     * Try to determine whether this application is running under Windows
     * or some other platform by examing the "os.name" property.
     *
     * @return true if this application is running under a Windows OS
     */
    static public boolean isWindowsPlatform() {
	String os = getOperatingSystemID();
        return os != null && os.startsWith(WIN_ID);
    }
    
    /**
     * Returns an ID for the current architecture.
     */
    static public String getArchitectureID() {
	if (_osArchProperty == null)  {
	    _osArchProperty = getProperty("os.arch");
	}
	return _osArchProperty;
    }
    
    /**
     * Returns an ID for the operating system.
     */
    static public String getOperatingSystemID() {
	if (_osNameProperty == null) {
	    _osNameProperty = getProperty("os.name");
	}
	return _osNameProperty;
    }
    
    /**
     * Returns the default locale.
     */
    static public Locale getLocale() { return Locale.getDefault(); }
    
    /*
     *  Returns the build-id. This is stored in the jar file
     */
    static public String getBuildID() {
	String build = null;
        InputStream s = Globals.class.getResourceAsStream("/build.id");
        if (s != null) {
	    BufferedReader br = new BufferedReader(new InputStreamReader(s));
	    try {
		build = br.readLine();
	    } catch(IOException e) { /* ignore */ }
        }
        return (build == null || build.length() == 0) ? "<internal>" : build;
    }
    
    /*
     * Returns the java.version
     */
    static public String getJavaVersion() {
	if (_javaVersionProperty == null) {
	    _javaVersionProperty = getProperty("java.version");
	}
	return _javaVersionProperty;
    }
    
    /**
     * Returns the name of the JavaWeb Component name
     */
    static public String getComponentName() { return JAVAWS_NAME ; }
    
    /*
     * Returns the user-agent string
     */
    static public String getUserAgent() { 
	// fix for 4676386: better user-agent value in http request header
	return "JNLP/" + JNLP_VERSION + " javaws/" + JAVAWS_VERSION + " (" + getBuildID() + ")" + " J2SE/" + System.getProperty("java.version");	
    }  
    
    /** Gets called with a list of arguments. It weeds out all options starting with -XX:yyy=zzz
     *  an sets the corresponding gloabal argument accordingly. It scans until it finds a non
     *  -option, i.e., a string not starting with a hypen.
     */
    static public String[] parseOptions(String[] argsArray) {
        readOptionFile();
        
        ArrayList args = new ArrayList();
        
        int pos = 0;
        boolean done = false;
        while(pos < argsArray.length) {
	    String option = argsArray[pos++];
	    if (option.startsWith("-XX:") && !done) {
		// Remove element for the arguments
		parseOption(option.substring(4), false);
	    } else {
		args.add(option);
	    }
	    
	    // Stop scanning after the first non-option argument
	    if (!option.startsWith("-")) done = true;
        }
	//Set the options which are triggered by one.
        setTCKOptions();
        Debug.openSocketStream();
        String[] newargs = new String[args.size()];
        return (String[])args.toArray(newargs);
    }
    
    static public void getDebugOptionsFromProperties(Properties props) {
        int i = 0;
        while(true) {
	    String option = props.getProperty("javaws.debug." + i);
	    if (option == null) return;
	    // As a security check, only the options starting with 'x' or 'Trace' are allowed from
	    // a JNLP file
	    parseOption(option, true);
	    i++;
        }
    }
    
    // Helper methods
    static private void setTCKOptions() {
	if (Globals.TCKHarnessRun==true) {
	    RedirectErrors=true;
	    RedirectOutput=true;
	    if (LogToHost == null) {
	    	System.err.println("Warning: LogHost = null");
	    }
	}
    }
    static private void parseOption(String option, boolean restricted) {
        String key = null;
        String value = null;
        
        int i = option.indexOf('=');
        if (i == -1) {
	    key = option;
	    value = null;
        } else {
	    key = option.substring(0, i);
	    value = option.substring(i+1);
        }
        
        // Check for the shorthand form for boolean arguments
        if (key.length() > 0 && (key.startsWith("-") || key.startsWith("+"))) {
	    key   = key.substring(1);
	    value = option.startsWith("+") ? "true" : "false";
        }
	
        // Restrict options that can be set from JNLP file
        if (restricted && !(key.startsWith("x") || key.startsWith("Trace"))) {
	    key = null;
        }
	
        if (key != null && setOption(key, value)) {
	    System.out.println("# Option: " + key + "=" + value);
        } else {
	    System.out.println("# Ignoring option: " + option);
        }
        

    }
    
    static private boolean setOption(String key, String value) {
        Class stringType  = new String().getClass();
        boolean res = true;
        
        // Use reflection to lookup key
        try {
	    Field f = new Globals().getClass().getDeclaredField(key);
	    if ((f.getModifiers() & Modifier.STATIC) == 0) return false;
	    
	    Class type = f.getType();
	    if (type ==  stringType)  {
		f.set(null, value);
	    } else if (type == Boolean.TYPE) {
		f.setBoolean(null, Boolean.valueOf(value).booleanValue());
	    } else if (type == Integer.TYPE) {
		f.setInt(null, Integer.parseInt(value));
	    } else if (type == Float.TYPE) {
		f.setFloat(null, Float.parseFloat(value));
	    } else if (type == Double.TYPE) {
		f.setDouble(null, Double.parseDouble(value));
	    } else if (type == Long.TYPE) {
		f.setLong(null, Long.parseLong(value));
	    } else {
		// Unsupported type
		return false;
	    }
        } catch(IllegalAccessException iae) {
	    return false;
        } catch(NoSuchFieldException nsfe) {
	    return false;
        }
        return res;
    }
    
    /** Try to read the '.javawsrc' property file from current directory */
    static private void readOptionFile() {
	FileInputStream fis = null;
        try {
	    fis = new FileInputStream(".javawsrc");
	} catch (FileNotFoundException fnfe) {
	    try {
	    	fis = new FileInputStream(getProperty("user.home") + File.separator + ".javawsrc");
	    } catch (FileNotFoundException fnfe1) {
		return;
	    }
	}
	try {
	    Properties props = new Properties();
	    props.load(fis);
	    
	    // Iterate through properyies
	    Enumeration enums = props.propertyNames();
	    while(enums.hasMoreElements()) {
		String key   = (String)enums.nextElement();
		String value = props.getProperty(key);
		parseOption(key + "=" + value, false);
	    }
	} catch(IOException ioe) {  }            /* ignore */
    }
}






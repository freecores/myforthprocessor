/*
 * @(#)JnlpxArgs.java	1.39 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws;
import com.sun.javaws.debug.*;
import com.sun.javaws.util.GeneralUtil;
import java.util.Vector;
import java.util.StringTokenizer;
import java.util.Properties;
import java.util.Iterator;
import java.io.File;

/**
 *  Utility class that provides easy access to
 *  all the properterty arguments that are passed in by
 *  the native launcher.
 *
 *  The options are:
 *
 *   jnlpx.cmd  : What command to execute
 *   jnlpx.jvm  : What JVM key was used
 *   jnlpx.splashport: Port the splash screen is listning to
 *   jnlpx.home : Directory where this app. is installed
 *
 *   This file must correspond with the launcher.c file
 */
public class JnlpxArgs {
    static final private String ARG_JVM = "jnlpx.jvm";
    static final private String ARG_SPLASHPORT = "jnlpx.splashport";
    static final private String ARG_HOME = "jnlpx.home";
    static final private  String ARG_REMOVE = "jnlpx.remove";
    static final private  String ARG_OFFLINE = "jnlpx.offline";
    static final private String ARG_HEAPSIZE = "jnlpx.heapsize";
    static final private String ARG_VMARGS = "jnlpx.vmargs";
    static final private String ARG_D_S_HOME = "jnlpx.deployment.system.home";
    static final private String ARG_D_U_HOME = "jnlpx.deployment.user.home";

    
    static private File _currentJVMCommand = null;
    
    /** Returns the splash port */
    static public int getSplashPort() {
	try {
	    return Integer.parseInt(System.getProperty(ARG_SPLASHPORT, "-1"));
	} catch(NumberFormatException nfe) {
	    return -1;
	}
    }

    static public String getVMArgs() {
	return System.getProperty(ARG_VMARGS);
    }
    
    /** Returns the path for the current running JVM */
    static File getJVMCommand() {
	if (_currentJVMCommand == null) {
            String prop = System.getProperty(ARG_JVM, "").trim();
            if (prop.startsWith("\"")) prop = prop.substring(1);
            if (prop.endsWith("\"")) prop = prop.substring(0, prop.length() - 1);
	    _currentJVMCommand = new File(prop);
	}
	return _currentJVMCommand;
    }
    
    /** Returns the home directory of Java Web Start */
    static public String getHome() { return System.getProperty(ARG_HOME); }
    
    /** Returns the Deployment System Home directory  */
    static public String getDeploymentSystemHome() { 
	return System.getProperty(ARG_D_S_HOME); 
    }
    
    /** Returns the Deployment User Home directory  */
    static public String getDeploymentUserHome() { 
	return System.getProperty(ARG_D_U_HOME); 
    }
    
    /** Returns true if the argument file should be removed */
    static public boolean shouldRemoveArgumentFile() { return getBooleanProperty(ARG_REMOVE); }
    /** Returns true if the argument file should be removed */
    
    static public void setShouldRemoveArgumentFile(String value) { System.setProperty(ARG_REMOVE, value); }
    
    /** Returns true if the application is invoked in offline mode */
    static public boolean isOffline() { return getBooleanProperty(ARG_OFFLINE); }
    
    /** Set isOffline property */
    static public void SetIsOffline() { System.setProperty(ARG_OFFLINE, "true"); }
    
    static public String getHeapSize() { return System.getProperty(ARG_HEAPSIZE); }
    
    static public long getInitialHeapSize() {
	String s = getHeapSize();
	if (s == null) return -1;
	String val = s.substring(s.lastIndexOf('=') + 1);
	String initialHeap = val.substring(0, val.lastIndexOf(','));
	return GeneralUtil.heapValToLong(initialHeap);
    }
    
    static public long getMaxHeapSize() {
	String s = getHeapSize();
	if (s == null) return -1;
	String val = s.substring(s.lastIndexOf('=') + 1);
	String maxHeap = val.substring(val.lastIndexOf(',') + 1, val.length());
	return GeneralUtil.heapValToLong(maxHeap);
    }
    
    public static boolean isCurrentRunningJREHeap(long reqMinHeap, long reqMaxHeap) {

	long currMinHeap = getInitialHeapSize();
	long currMaxHeap = getMaxHeapSize();
	if (Globals.TraceStartup && (reqMinHeap != -1 || reqMaxHeap != -1)) {
	    Debug.println("isCurrentRunningJREHeap: passed args: " + reqMinHeap + ", " + reqMaxHeap);
	    Debug.println("JnlpxArgs is " + currMinHeap + ", " + currMaxHeap);
	}
	return ((currMinHeap == reqMinHeap) && (currMaxHeap == reqMaxHeap));
    }
        
    public static boolean isSecurePropsMatch(Properties props) {
	Iterator it = ConfigProperties.getInstance().getSecurePropertyKeys();
	while (it.hasNext()) {
	    String key = (String) it.next();
	    if (props.containsKey(key)) {
	        Object value = props.get(key);
		if ((value != null) && !value.equals(System.getProperty(key))) {
		    return false;
		}
	    }
	}
	return true;
    }

    private static boolean heapSizesValid(long minHeap, long maxHeap) {
	
	return !(minHeap == -1 && maxHeap == -1);

    }
    
    /** Returns a set of arguments that can be supplied to the java command in order to
     *  reproduce the current settings
     */
    static public String[] getArgumentList(String jvmCommand, 
		long reqMinHeap, long reqMaxHeap, Properties props) {
	String heapSizeArg = "-D" + ARG_HEAPSIZE + "=NULL,NULL";
	String minHeapSizeVMArg = "";
	String maxHeapSizeVMArg = "";
	if (heapSizesValid(reqMinHeap, reqMaxHeap)) {

	    heapSizeArg = "-D" + ARG_HEAPSIZE + "=" + reqMinHeap + "," + reqMaxHeap;
	    if (reqMinHeap > 0) minHeapSizeVMArg = "-Xms" + reqMinHeap;
	    if (reqMaxHeap > 0) maxHeapSizeVMArg = "-Xmx" + reqMaxHeap;
	  
	}
		/** 
		 * Note care must be taken to ensure the proper arrangement
		 * of the args, properties and VM options should be before
		 * the Main and all javaws options must be after the Main.
		 */
	String[] args = {
	    	minHeapSizeVMArg,
		maxHeapSizeVMArg,
		(getVMArgs() != null ? ("-D" + ARG_VMARGS + "=" + getVMArgs()) : ""),
	    	"-D" + ARG_JVM + "=" + jvmCommand,
		"-D" + ARG_SPLASHPORT + "=" + getSplashPort(),
		"-D" + ARG_HOME + "=" + getHome(),
		"-D" + ARG_REMOVE + "=" + (shouldRemoveArgumentFile() ? "true" : "false"),
		"-D" + ARG_OFFLINE + "=" + (isOffline() ? "true" : "false"),
		heapSizeArg,
		"-Djava.security.policy=file:" + getHome() + 
					File.separator + "javaws.policy",
		"-DtrustProxy=true",
		"-D" + ARG_D_U_HOME + "=" + getDeploymentUserHome(),
		"-D" + ARG_D_S_HOME + "=" + getDeploymentSystemHome(),
		"-Xbootclasspath/a:"+ 
			getHome() + File.separator + "javaws.jar",
		"-classpath",
		File.pathSeparator + 
			getHome() + File.separator + "javaws-l10n.jar",
		useJCOV(),
		useBootClassPath(),
		"com.sun.javaws.Main",
		setTCKHarnessOption(),
		useLogToHost()
	};
	
	/** a "" argument is not allowed, so they get stripped here */
	int count = 0;
	for(int i = 0; i < args.length; i++) {
	    if (! args[i].equals("")) count++;
	}
	String [] returnArgs = getVMArgList(count, props);
	count = returnArgs.length - count;
	for(int i = 0; i < args.length; i++) {
	    if (! args[i].equals("")) returnArgs[count++] = args[i];
	}
	return returnArgs;
    }

    static private String[] getVMArgList(int count, Properties props) {
	Vector v = new Vector();
	String s = null;
	int i = 0;

	if ((s = getVMArgs()) != null) {
	    StringTokenizer st = new StringTokenizer(s, " \t\n\r\f\"");
	    while(st.hasMoreTokens()) {
		v.add(st.nextToken());
		i++;
	    }
	}
	Iterator it = ConfigProperties.getInstance().getSecurePropertyKeys();
        while (it.hasNext()) {
            String key = (String) it.next();
            if (props.containsKey(key)) {
		String arg = "-D"+key+"="+props.get(key);
		if (!v.contains(arg)) {
	            v.add(arg);
		    i++;
		}
	    }
        }
	
	String [] args = new String[count + i];
	for (int j = 0; j < i; j++) {
	    args[j] = new String((String)v.elementAt(j));
	}
	return args;
    }						     

    /** set the logHost */
    static public String useLogToHost() {
        if (Globals.LogToHost != null) {
            return "-XX:LogToHost="+Globals.LogToHost;
        }
        return "";
    }
    
    /** Set the TCK option */
    static public String setTCKHarnessOption() {
        if (Globals.TCKHarnessRun == true) {
            return "-XX:TCKHarnessRun=true";
        }
        return "";
    }
    
    /** for setting BootClassPath, helpful with Silk testing */
    static public String useBootClassPath() {
	if (Globals.BootClassPath.equals("NONE")) {
	    return "";
	} else {
	    return "-Xbootclasspath" + Globals.BootClassPath;
	}
    }
    
    /** for JCOV tool, used in coverage testing */
    static public String useJCOV() {
	if (Globals.JCOV.equals("NONE")) {
	    return "";
	} else {
	    return "-Xrunjcov:file=" + Globals.JCOV;
	}
    }
    
    /** Removes the argument if neccesary */
    static public void removeArgumentFile(String args[]) {	
	// Remove argument, if we are suppose to
	if (JnlpxArgs.shouldRemoveArgumentFile() && args != null && args.length > 0) {	   
	    new File(args[0]).delete();
	}
    }
    
    /** Method to verify that all properties are setup correctly */
    static void verify() {
	// Check that all properties are setup correctly
	if (Globals.TraceStartup) {
	    Debug.println("Java part started");
	    Debug.println(ARG_JVM        + ": " + getJVMCommand());
	    Debug.println(ARG_SPLASHPORT + ": " + getSplashPort());
	    Debug.println(ARG_HOME       + ": " + getHome());
	    Debug.println(ARG_REMOVE     + ": " + shouldRemoveArgumentFile());
	    Debug.println(ARG_HEAPSIZE   + ": " + getHeapSize());
	}
	Debug.jawsAssert(getJVMCommand() != null, "jvm propety not set");
	Debug.jawsAssert(getHome() != null, "home property not set");
    }
    
    static private boolean getBooleanProperty(String key) {
	String s = System.getProperty(key, "false");
	return s != null && s.equals("true");
    }
}




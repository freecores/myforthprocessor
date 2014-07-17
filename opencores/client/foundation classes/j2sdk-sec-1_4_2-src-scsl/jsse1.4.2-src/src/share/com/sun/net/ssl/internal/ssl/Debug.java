/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package com.sun.net.ssl.internal.ssl;

import java.io.PrintStream;
import java.security.AccessController;

import sun.security.action.GetPropertyAction;

/**
 * This class has be shamefully lifted from sun.security.util.Debug
 *
 * @author Gary Ellison
 * @version 06/24/03 1.8
 */
class Debug {

    private String prefix;

    private static String args;

    static {
	args = (String)java.security.AccessController.doPrivileged(
               new GetPropertyAction("javax.net.debug", ""));
	args = args.toLowerCase();
	if (args.equals("help")) {
	    Help();
	}
    }

    public static void Help() 
    {
	System.err.println();
	System.err.println("all            turn on all debugging");
	System.err.println("ssl            turn on ssl debugging");
	System.err.println();
	System.err.println("The following can be used with ssl:");
	System.err.println("\trecord       enable per-record tracing");
	System.err.println("\thandshake    print each handshake message");
	System.err.println("\tkeygen       print key generation data");
	System.err.println("\tsession      print session activity");
	System.err.println("\tdefaultctx   print default SSL initialization");
	System.err.println("\tsslctx       print SSLContext tracing");
	System.err.println("\tsessioncache print session cache tracing");
	System.err.println("\tkeymanager   print key manager tracing");
	System.err.println("\ttrustmanager print trust manager tracing");
	System.err.println();
	System.err.println("\thandshake debugging can be widened with:");
	System.err.println("\tdata         hex dump of each handshake message");
	System.err.println("\tverbose      verbose handshake message printing");
	System.err.println();
	System.err.println("\trecord debugging can be widened with:");
	System.err.println("\tplaintext    hex dump of record plaintext");
	System.err.println();
	System.exit(0);
    }

    /**
     * Get a Debug object corresponding to whether or not the given
     * option is set. Set the prefix to be the same as option.
     */

    public static Debug getInstance(String option)
    {
	return getInstance(option, option);
    }

    /**
     * Get a Debug object corresponding to whether or not the given
     * option is set. Set the prefix to be prefix.
     */
    public static Debug getInstance(String option, String prefix)
    {
	if (isOn(option)) {
	    Debug d = new Debug();
	    d.prefix = prefix;
	    return d;
	} else {
	    return null;
	}
    }

    /**
     * True if the property "javax.net.debug" contains the 
     * string "option".
     */
    public static boolean isOn(String option)
    {
	if (args == null) {
	    return false;
	} else {
	    int n = 0;

	    if (args.indexOf("all") != -1) {
		return true;
	    } else if ((n = args.indexOf("ssl")) != -1) {
		if (args.indexOf("sslctx", n) == -1) {
		    // don't enable data and plaintext options by default
		    if (!(option.equals("data")
			|| option.equals("plaintext"))) { 
		        return true;
		    }
		}
	    }
	    return (args.indexOf(option.toLowerCase()) != -1);
	}
    }

    /**
     * print a message to stderr that is prefixed with the prefix
     * created from the call to getInstance.
     */

    public void println(String message)
    {
	System.err.println(prefix + ": "+message);
    }

    /**
     * print a blank line to stderr that is prefixed with the prefix.
     */

    public void println()
    {
	System.err.println(prefix + ":");
    }

    /**
     * print a message to stderr that is prefixed with the prefix.
     */

    public static void println(String prefix, String message)
    {
	System.err.println(prefix + ": "+message);
    }
    
    static void println(PrintStream s, String name, byte[] data) {
	s.print(name + ":  { ");
	if (data == null ) {
	    s.print("null");
	} else {
	    for (int i = 0; i < data.length; i++) {
		if (i != 0) s.print(", ");
		s.print(data[i] & 0x0ff);
	    }
	}
	s.println(" }");
    }
    
    /**
     * Return the value of the boolean System property propName.
     *
     * Note use of doPrivileged(). Do make accessible to applications.
     */
    static boolean getBooleanProperty(String propName, boolean defaultValue) {
	// if set, require value of either true or false
	String b = (String)AccessController.doPrivileged(
		new GetPropertyAction(propName));
	if (b == null) {
	    return defaultValue;
	} else if (b.equalsIgnoreCase("false")) {
	    return false;
	} else if (b.equalsIgnoreCase("true")) {
	    return true;
	} else {
	    throw new RuntimeException("Value of " + propName 
	    	+ " must either be 'true' or 'false'");
	}
    }
}

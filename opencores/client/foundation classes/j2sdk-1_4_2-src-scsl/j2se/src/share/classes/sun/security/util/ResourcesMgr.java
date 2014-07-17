/*
 * @(#)ResourcesMgr.java	1.4 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.security.util;

/**
 * @version 1.4, 01/23/03
 */
public class ResourcesMgr {

    // intended for java.security, javax.security and sun.security resources
    private static java.util.ResourceBundle bundle;

    // intended for com.sun.security resources
    private static java.util.ResourceBundle altBundle;

    public static String getString(String s) {

	if (bundle == null) {

	    // only load if/when needed
	    bundle = (java.util.ResourceBundle)
		java.security.AccessController.doPrivileged
		(new java.security.PrivilegedAction() {
		public Object run() {
		    return (java.util.ResourceBundle.getBundle
				("sun.security.util.Resources"));
		}
	    });
	}

	return bundle.getString(s);
    }

    public static String getString(String s, final String altBundleName) {

	if (altBundle == null) {

	    // only load if/when needed
	    altBundle = (java.util.ResourceBundle)
		java.security.AccessController.doPrivileged
		(new java.security.PrivilegedAction() {
		public Object run() {
		    return (java.util.ResourceBundle.getBundle(altBundleName));
		}
	    });
	}

	return altBundle.getString(s);
    }
}

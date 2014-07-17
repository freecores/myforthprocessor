/*
 * @(#)VersionHelper.java	1.6 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.security.sasl.preview;

import java.util.Enumeration;

abstract class VersionHelper {
    private static VersionHelper helper = null;

    final static String[] PROPS = new String[] {
	Sasl.CLIENT_PKGS,
	Sasl.SERVER_PKGS };

    VersionHelper() {}

    static {
	try {
	    Class.forName("java.net.URLClassLoader"); // 1.2 test
	    Class.forName("java.security.PrivilegedAction"); // 1.2 test
	    helper = (VersionHelper)
		Class.forName(
		    "com.sun.security.sasl.preview.VersionHelper12").newInstance();
	} catch (Exception e) {
	}

	// Use 1.1 helper if 1.2 test fails, or if we cannot create 1.2 helper
	if (helper == null) {
	    try {
		helper = (VersionHelper)
		    Class.forName(
			"com.sun.security.sasl.preview.VersionHelper11").newInstance();
	    } catch (Exception e) {
		// should never happen
	    }
	}
    }

    static VersionHelper getVersionHelper() {
	return helper;
    }

    abstract Class loadClass(String className) throws ClassNotFoundException;

    abstract String getSaslProperty(int i);

    abstract Enumeration getFactoriesFromJars(Class cls);
}

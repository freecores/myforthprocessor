/*
 * @(#)DefaultSecurity.java	1.14 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.media.sound;

import java.util.Hashtable;
import java.lang.reflect.Method;
import java.lang.reflect.InvocationTargetException;

class DefaultSecurity implements JSSecurity {

    final static String name = "DefaultSecurity";

    static JSSecurity security;
    private static ClassLoader clsLoader=null;
    private static Class cls=null;
    private static Method dummyMethodRef=null;

    static {

    	security = new DefaultSecurity();
        try {
	    cls = security.getClass();
	    clsLoader = cls.getClassLoader();
	    dummyMethodRef = cls.getMethod("dummyMethod",new Class[0]);
        } catch (Exception e) {
	    if (Printer.err) Printer.err("Default Security, exception caught: "+e);
	}

    }

    public static void dummyMethod() {

    }

    private DefaultSecurity() {

    }

    public String getName() {

	return name;
    }

    public void requestPermission(Method[] m, Class[] c, Object[][] args,
				  int request) throws SecurityException {

	if (clsLoader == null) {
	    m[0] = dummyMethodRef;
	    c[0] = cls;
	    args[0] = null;
	}
	else {
	    throw new SecurityException("DefulatSecurity : Cannot request permission");
	}
    }

    // parameter not used
    public void requestPermission(Method[] m, Class[] c, Object[][] args, int request,
				  String parameter) throws SecurityException {
	requestPermission(m, c, args, request);
    }

    public void checkRecordPermission() throws SecurityException {

	if(Printer.trace)Printer.trace("DefaultSecurity.checkRecordPermission()");
	JSSecurityManager.checkRecord();
    }

    public boolean isLinkPermissionEnabled() {
	if (clsLoader == null)  {
	    return true;
	}
	else {
	    return false;
	}

    }

    public void permissionFailureNotification(int permission) {
    }

    public void loadLibrary(final String name) throws UnsatisfiedLinkError {
	if (clsLoader == null) {
	    System.loadLibrary(name);
	}
	else {
	    throw new UnsatisfiedLinkError("Unable to get link privilege to " + name);
	}
    }

    public String readProperty(final String name) throws UnsatisfiedLinkError {
	return System.getProperty(name);
    }

}



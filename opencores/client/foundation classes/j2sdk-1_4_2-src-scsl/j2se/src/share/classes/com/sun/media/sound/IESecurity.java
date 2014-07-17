/*
 * @(#)IESecurity.java	1.14 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.media.sound;

import java.util.Hashtable;
import java.lang.reflect.Method;
import java.lang.reflect.InvocationTargetException;

class IESecurity implements JSSecurity {

    static JSSecurity security;
    static boolean jview=false;

    private static Class cls=null;
    private static Method dummyMethodRef=null;

    final static String name = "IESecurity";

    static {
	security = new IESecurity();
        cls = security.getClass();
        try {
	    dummyMethodRef = cls.getMethod("dummyMethod",new Class[0]);
        } catch (Exception e) {

        }
    }

    private IESecurity() {
    }


    public static void dummyMethod() {
    }

    public String getName() {
	return name;
    }


    public void requestPermission(Method[] m, Class[] c, Object[][] args,
				  int request) throws SecurityException {
	//        if (!jview)
	//	    throw new SecurityException("IESecurity : Cannot request permission");
	m[0] = dummyMethodRef;
	c[0] = cls;
	args[0] = null;
    }

    // Netscape will not use this parameter.
    public void requestPermission(Method[] m, Class[] c, Object[][] args, int request,
				  String parameter) throws SecurityException {
	requestPermission(m, c, args, request);
    }

    public void checkRecordPermission() throws SecurityException {
	if(Printer.trace)Printer.trace("IESecurity.checkRecordPermission()");
	JSSecurityManager.checkRecord();
    }

    public boolean isLinkPermissionEnabled() {
	return jview;
    }

    public void permissionFailureNotification(int permission) {
    }

    public void loadLibrary(final String name) throws UnsatisfiedLinkError {
	//	if (jview)
	System.loadLibrary(name);
	//	else
	//	    throw new UnsatisfiedLinkError("Unable to get link privilege to " + name);
    }

    public String readProperty(final String name) throws UnsatisfiedLinkError {
	return System.getProperty(name);
    }

}

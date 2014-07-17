/*
 * @(#)DisabledSecurity.java	1.11 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.media.sound;

import java.lang.reflect.Method;
import java.lang.reflect.InvocationTargetException;

class DisabledSecurity implements JSSecurity {

    final static String name = "DisabledSecurity";

    static JSSecurity security;

    static {
	security = new DisabledSecurity();
    }

    private DisabledSecurity() {
    }


    public String getName() {
	return name;
    }

    public static void dummyMethod() {

    }

    public void checkRecordPermission() throws SecurityException {
	throw new SecurityException("DisabledSecurity: Audio record permission denied");
    }

    public void requestPermission(Method[] m, Class[] c, Object[][] args,
				  int request) throws SecurityException {
	throw new SecurityException("DisabledSecurity : Cannot request permission");
    }

    // parameter not used
    public void requestPermission(Method[] m, Class[] c, Object[][] args, int request,
				  String parameter) throws SecurityException {
	requestPermission(m, c, args, request);
    }


    public boolean isLinkPermissionEnabled() {
	return false;
    }

    public void permissionFailureNotification(int permission) {
    }

    public void loadLibrary(final String name) throws UnsatisfiedLinkError {
	throw new UnsatisfiedLinkError("Unable to get link privilege to " + name);
    }

    public String readProperty(final String name) throws UnsatisfiedLinkError {
	throw new UnsatisfiedLinkError("Unable to get link privilege to " + name);
    }

}

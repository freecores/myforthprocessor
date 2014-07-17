/*
 * @(#)JDK12Security.java	1.17 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.media.sound;

import java.util.Hashtable;
import java.lang.reflect.Constructor;
import java.lang.reflect.Method;
import java.lang.reflect.InvocationTargetException;

import java.security.AccessController;
import java.security.PrivilegedAction;

import javax.sound.sampled.AudioPermission;


class JDK12Security implements JSSecurity {

    final static String name = "JDK12Security";

    static JSSecurity security;
    private static Class cls=null;
    private static Method dummyMethodRef=null;

    static {
	if (Printer.debug) Printer.debug("JDK12Security: static");
    	security = new JDK12Security();
        try {
	    cls = security.getClass();
	    dummyMethodRef = cls.getMethod("dummyMethod",new Class[0]);
        } catch (Exception e) {
	    if (Printer.err) Printer.err("JDK12Security: exception caught");
	    if (Printer.debug) e.printStackTrace();
	}
	if (Printer.debug) Printer.debug("JDK12Security class: " + cls);
    }

    public static void dummyMethod() {}

    private JDK12Security() {}

    public String getName() {
	return name;
    }

    public void requestPermission(Method[] m, Class[] c, Object[][] args,
				  int request) throws SecurityException {
	m[0] = dummyMethodRef;
	c[0] = cls;
	args[0] = null;
    }

    // parameter not used
    public void requestPermission(Method[] m, Class[] c, Object[][] args, int request,
				  String parameter) throws SecurityException {
	requestPermission(m, c, args, request);
    }

    public boolean isLinkPermissionEnabled() {
	return true;
    }

    public void permissionFailureNotification(int permission) {
    }

    public void checkRecordPermission() throws SecurityException {
	if(Printer.trace)Printer.trace("JDK12Security.checkRecordPermission()");
	SecurityManager sm = System.getSecurityManager();
	if (sm != null) sm.checkPermission(new AudioPermission("record"));
    }

    public void loadLibrary(final String name) throws UnsatisfiedLinkError {
	/*		try{
			Constructor cons = JDK12LoadLibraryAction.cons;
			JDK12.doPrivM.invoke(
			JDK12.ac,
			new Object[] {
			cons.newInstance( new Object[] { name } )
			});
			} catch(Exception e) {
			throw new UnsatisfiedLinkError( e.getMessage() );
			}
	*/
	AccessController.doPrivileged(new PrivilegedAction() {
		public Object run() {
		    System.loadLibrary(name);
		    return null;
		}
	    });
    }

    public String readProperty(final String name) throws UnsatisfiedLinkError {
	try {
	    Constructor cons = JDK12PropertyAction.cons;
	    String s = null;
	    s = (String) JDK12.doPrivM.invoke(
					      JDK12.ac,
					      new Object[] {
						  cons.newInstance( new Object[] { name } )
					      });
	    return s;
	} catch(Exception e) {
	    throw new UnsatisfiedLinkError( e.getMessage() );
	}
	// return System.getProperty(name);
    }
}

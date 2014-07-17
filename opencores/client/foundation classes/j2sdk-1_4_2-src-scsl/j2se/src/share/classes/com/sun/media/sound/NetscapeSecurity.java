/*
 * @(#)NetscapeSecurity.java	1.13 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.media.sound;

import java.util.Hashtable;
import java.lang.reflect.Method;
import java.lang.reflect.InvocationTargetException;

class NetscapeSecurity implements JSSecurity {

    final static String name = "NetscapeSecurity";

    static JSSecurity security;

    private static Method enablePrivilege;
    private static Class privilegeManager;
    //private static ClassLoader cl =null;

    private static Object [] readPropArgs = new Object[] {"UniversalPropertyRead"};
    private static Object [] readFileArgs = new Object[] {"UniversalFileRead"};
    private static Object [] writeFileArgs = new Object[] {"UniversalFileWrite"};
    private static Object [] deleteFileArgs = new Object[] {"UniversalFileDelete"};
    private static Object [] threadArgs = new Object[] {"UniversalThreadAccess"};
    private static Object [] threadGroupArgs = new Object[] {"UniversalThreadGroupAccess"};
    private static Object [] linkArgs = new Object[] {"UniversalLinkAccess"};
    private static Object [] connectArgs = new Object[] {"UniversalConnect"};
    private static Object [] windowArgs = new Object[] {"UniversalTopLevelWindow"};
    private static Object [] multicastArgs = new Object[] {"UniversalMulticast"};

    private static Hashtable table = new Hashtable();

    static {
	security = new NetscapeSecurity();
	try {
	    privilegeManager =
		Class.forName("netscape.security.PrivilegeManager");

	    enablePrivilege = privilegeManager.getMethod("enablePrivilege",
							 new Class[] {String.class});
            //cl =(Class.forName("javax.JSSecurity")).getClassLoader();
	} catch (ClassNotFoundException  e) {
	    if (Printer.err) Printer.err("NetscapeSecurity: Cannot find class netscape.security.PrivilegeManager");
	} catch (Exception e) {
	    if (Printer.err) Printer.err("NetscapeSecurity: Exception caught: "+ e);
	}

	table.put(new Integer(JSSecurity.READ_PROPERTY), readPropArgs);
	table.put(new Integer(JSSecurity.READ_FILE), readFileArgs);
	table.put(new Integer(JSSecurity.WRITE_FILE), writeFileArgs);
	table.put(new Integer(JSSecurity.DELETE_FILE), deleteFileArgs);
	table.put(new Integer(JSSecurity.THREAD), threadArgs);
	table.put(new Integer(JSSecurity.THREAD_GROUP), threadGroupArgs);
	table.put(new Integer(JSSecurity.LINK), linkArgs);
	table.put(new Integer(JSSecurity.CONNECT), connectArgs);
	table.put(new Integer(JSSecurity.TOP_LEVEL_WINDOW), windowArgs);
	table.put(new Integer(JSSecurity.MULTICAST), multicastArgs);
    }

    private Method methodArray[] = new Method[1];
    private Class classArray[] = new Class[1];
    private Object arguments[][] = new Object[1][0];


    private NetscapeSecurity() {}

    public String getName() {
	return name;
    }


    public void requestPermission(Method[] m, Class[] c, Object[][] args,
				  int request) throws SecurityException {
	//if ( (enablePrivilege == null) || (cl != null) ) {
        if (enablePrivilege == null) {
	    throw new SecurityException("Cannot request permission");
	}
	m[0] = enablePrivilege;
	c[0] = privilegeManager;
	
	Object value = table.get(new Integer(request));
	if (value == null) {
	    throw new SecurityException("Unknown request " + request);
	}

	args[0] = (Object[]) value;
    }

    // Netscape will not use this parameter.
    public void requestPermission(Method[] m, Class[] c, Object[][] args, int request,
				  String parameter) throws SecurityException {
	requestPermission(m, c, args, request);
    }

    public void checkRecordPermission() throws SecurityException {
	if(Printer.trace)Printer.trace("NetscapeSecurity.checkRecordPermission()");
	JSSecurityManager.checkRecord();
    }

    public boolean isLinkPermissionEnabled()  {
        if (table.get(new Integer(JSSecurity.LINK))!=null)
	    return true ;
        else
	    return false ;
    }

    public void permissionFailureNotification(int permission) {
	table.remove(new Integer(permission));
    }

    public void loadLibrary(final String name) throws UnsatisfiedLinkError {
	try {
	    requestPermission(methodArray, classArray, arguments, JSSecurity.LINK);
	    methodArray[0].invoke(classArray[0], arguments[0]);
	    System.loadLibrary(name);
	} catch (Exception e) {
	    if (Printer.err) Printer.err("Unable to get " + name +
					 " privilege  " + e);
	    throw new UnsatisfiedLinkError("Unable to get " + name +
					   " privilege  " + e);
	}
    }

    public String readProperty(final String name) throws UnsatisfiedLinkError {
	return System.getProperty(name);
    }
}

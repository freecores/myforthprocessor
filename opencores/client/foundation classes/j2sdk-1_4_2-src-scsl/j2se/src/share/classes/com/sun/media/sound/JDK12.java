/*
 * @(#)JDK12.java	1.8 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package com.sun.media.sound;

import java.lang.reflect.Method;
import java.lang.reflect.Constructor;




// This class will compile under 1.1 also as reflection alone is used
class JDK12 {

    // Should be able to use final for a one time initialization

    static Class ac;
    static Class accontextC;
    static Class permissionC;
    static Class privActionC;
    static Method checkPermissionM;
    static Method doPrivM;
    static Method doPrivContextM;
    static Method getContextM;


    static {
	try {
	    ac = Class.forName("java.security.AccessController");
	    accontextC = Class.forName("java.security.AccessControlContext");
	    permissionC = Class.forName("java.security.Permission");
	    privActionC = Class.forName("java.security.PrivilegedAction");
	    if(Printer.debug)Printer.debug("ac is " + ac);
	    checkPermissionM = ac.getMethod("checkPermission",
					    new Class[] {
						permissionC
					    });

	    doPrivM = ac.getMethod("doPrivileged",
				   new Class[] {
				       privActionC
				   });

	    getContextM = ac.getMethod("getContext", null);
	    if(Printer.debug)Printer.debug("getContextM is " + getContextM);


	    doPrivContextM = ac.getMethod("doPrivileged",
					  new Class[] {
					      privActionC,
					      accontextC
					  });
	    if(Printer.debug)Printer.debug("doPrivContextM is " + doPrivContextM);
	} catch (Throwable t) {
	    // This shouldn't happen on jdk1.2
	    if(Printer.err)Printer.err("Ok if thrown on non-jdk1.2 VM: " + t);
	}
    }


}



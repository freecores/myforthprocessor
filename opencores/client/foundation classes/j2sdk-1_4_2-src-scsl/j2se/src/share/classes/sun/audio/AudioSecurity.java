/*
 * @(#)AudioSecurity.java	1.6 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.audio;

/**
 * This is Robert Szewczyk's MySecurity.java class moved into the sun.audio package.
 */

/*
 * in version 4.0.4 of netscape, the SuperUser targer no longer works. Oh
 * well. Fine grained security model is a better idea anyway...
 */

import java.lang.reflect.*;

class AudioSecurity {
    static private boolean is12 = false;
    
    static private boolean isNSCP = false;
    
    static private Class privilegeManager;
    
    static private Method enablePrivilege;
    
    static {
	try {
	    privilegeManager =
		Class.forName("java.security.AccessController");
	    is12 = true;
	} catch (Exception e1) {
	    is12 = false;
	    try {
		privilegeManager =
		    Class.forName("netscape.security.PrivilegeManager");
		Class args[] = {java.lang.String.class};
		enablePrivilege =
		    privilegeManager.getMethod("enablePrivilege", args);
		isNSCP = true;
	    } catch (Exception e) {
		isNSCP = false;
	    }
	}
    }
    
    static Object doPrivileged(AudioSecurityAction sa, String priv) {
	
	if (is12) {
	    return java.security.AccessController.doPrivileged(new
		ActionWrapper(sa));
	} else if (isNSCP) {
	    try {
		Object args[] = new Object[1];
		args[0] = priv;
		enablePrivilege.invoke(privilegeManager, args);
		return sa.run();
	    } catch (Exception e) { return null; }
	} else {
	    return sa.run();
	}
    }
    
    static Object doPrivileged(AudioSecurityAction sa) {
	return doPrivileged(sa, "SuperUser");
    }
    
    
    static Object doPrivileged(AudioSecurityExceptionAction sea, String priv) throws
    AudioSecurityActionException {
	if (is12) {
	    try {
		return java.security.AccessController.doPrivileged(new ActionExceptionWrapper(sea));
	    } catch (java.security.PrivilegedActionException e) {
		throw new AudioSecurityActionException(e.getException());
	    }
	} else if (isNSCP) {
	    try {
		Object args[] = new Object[1];
		args[0] = priv;
		enablePrivilege.invoke(privilegeManager, args);
		return sea.run();
	    } catch (Exception e) {
		throw new AudioSecurityActionException(e);
	    }
	} else {
	    try {
		return sea.run();
	    } catch (Exception e) {
		throw new AudioSecurityActionException(e);
	    }
	}
    }
    
    static Object doPrivileged(AudioSecurityExceptionAction sea) throws AudioSecurityActionException{
	return doPrivileged(sea, "SuperUser");
    }
}

class ActionExceptionWrapper implements java.security.PrivilegedExceptionAction {
    AudioSecurityExceptionAction sa;
    ActionExceptionWrapper(AudioSecurityExceptionAction sea) {
	this.sa = sea;
    }
    
    public Object run() throws Exception {
	return sa.run();
    }
}

class ActionWrapper implements java.security.PrivilegedAction {
    AudioSecurityAction sa;
    ActionWrapper(AudioSecurityAction sa) {
	this.sa = sa;
    }
    
    public Object run() {
	return sa.run();
    }
}

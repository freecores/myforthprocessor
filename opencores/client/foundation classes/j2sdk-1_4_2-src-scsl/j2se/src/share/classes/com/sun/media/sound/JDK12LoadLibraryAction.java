/*
 * @(#)JDK12LoadLibraryAction.java	1.11 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.media.sound;

import java.security.PrivilegedAction;
import java.lang.reflect.Constructor;

class JDK12LoadLibraryAction implements java.security.PrivilegedAction {

    static Constructor cons;
    private String name;

    static {
	try {
	    cons = JDK12LoadLibraryAction.class.getConstructor(new Class[] {
		String.class});
	} catch (Throwable e) {
	    if (Printer.err)Printer.err("JDK12LoadLibraryAction threw "+e);
	}
    }


    public JDK12LoadLibraryAction(String name) {
	
	try {
	    this.name = name;
	} catch (Throwable e) {
	}
    }

    public Object run() {
	try {
	    System.loadLibrary(name);
	    return null;
	} catch (Throwable t) {
	    return null;
	}
    }

}

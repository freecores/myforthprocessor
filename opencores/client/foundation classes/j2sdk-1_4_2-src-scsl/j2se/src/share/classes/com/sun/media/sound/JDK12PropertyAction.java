/*
 * @(#)JDK12PropertyAction.java	1.9 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.media.sound;

import java.security.PrivilegedAction;
import java.lang.reflect.Constructor;

class JDK12PropertyAction implements java.security.PrivilegedAction {

    static Constructor cons;
    private String name;

    static {
	try {
	    cons = JDK12PropertyAction.class.getConstructor(new Class[] {
		String.class});
	} catch (Throwable e) {
	    if (Printer.err)Printer.err("JDK12PropertyAction threw "+e);
	}
    }


    public JDK12PropertyAction(String name) {
	
	try {
	    this.name = name;
	} catch (Throwable e) {
	}
    }

    public Object run() {
	try {
	    return System.getProperty(name);
	} catch (Throwable t) {
	    return null;
	}
    }

}

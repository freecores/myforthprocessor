/*
 * @(#)JDK12TopmostThreadGroupAction.java	1.10 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.media.sound;

import java.security.PrivilegedAction;
import java.lang.reflect.Constructor;

class JDK12TopmostThreadGroupAction implements java.security.PrivilegedAction {

    static Constructor cons;

    static {
	try {
	    cons = JDK12TopmostThreadGroupAction.class.getConstructor(new Class[0]);
	} catch (Throwable e) {
	    if (Printer.err)Printer.err("JDK12TopmostThreadGroupAction threw "+e);
	}
    }


    public JDK12TopmostThreadGroupAction() {
	
    }

    public Object run() {
	try {
	    ThreadGroup g = Thread.currentThread().getThreadGroup();
	    while ((g.getParent() != null) && (g.getParent().getParent() != null)) {
		g = g.getParent();
	    }
	    return g;
	} catch (Throwable t) {
	    return null;
	}
    }

}

/*
 * @(#)JDK12ConfigureThreadAction.java	1.10 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.media.sound;

import java.security.PrivilegedAction;
import java.lang.reflect.Constructor;

class JDK12ConfigureThreadAction implements java.security.PrivilegedAction {

    static Constructor cons;
    private String name;
    private Thread thread;

    static {
	try {
	    cons = JDK12ConfigureThreadAction.class.getConstructor(new Class[] {
		Thread.class, String.class});
	} catch (Throwable e) {
	    if (Printer.err)Printer.err("JDK12ConfigureThreadAction threw "+e);
	}
    }


    public JDK12ConfigureThreadAction(Thread thread, String name) {
	
	try {
	    this.thread = thread;
	    this.name = name;
	} catch (Throwable e) {
	}
    }

    public Object run() {
	try {
	    thread.setDaemon(true);
	    thread.setPriority(Thread.MAX_PRIORITY);
	    thread.setName(name);
	    return null;
	} catch (Throwable t) {
	    return null;
	}
    }

}

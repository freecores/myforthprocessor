/*
 * @(#)JDK12NewEventDispatcherAction.java	1.9 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.media.sound;

import java.security.PrivilegedAction;
import java.lang.reflect.Constructor;

class JDK12NewEventDispatcherAction implements java.security.PrivilegedAction {

    static Constructor cons;
    private String name;
    private Object eventDispatcher = null;
    private Object threadGroup = null;

    static {
	try {
	    // $$fb 2001-11-01 part of fix for Bug 4521048: Applets: Java Sound dies with an exception at init time
	    cons = JDK12NewEventDispatcherAction.class.getConstructor(new Class[]{ Object.class, Object.class } );
	} catch (Throwable e) {
	    if (Printer.err)Printer.err("JDK12NewEventDispatcherAction threw "+e);
	}
    }

    // $$fb 2001-11-01 part of fix for Bug 4521048: Applets: Java Sound dies with an exception at init time
    //include thread group
    public JDK12NewEventDispatcherAction(Object threadGroup, Object eventDispatcher ) {
	
	this.eventDispatcher = eventDispatcher;
	this.threadGroup = threadGroup;
    }

    public Object run() {
	try {
	    Thread t = new Thread((ThreadGroup) threadGroup, (Runnable)eventDispatcher);
	    t.start();
	    return t;
	} catch (Throwable t) {
	    return null;
	}
    }

}

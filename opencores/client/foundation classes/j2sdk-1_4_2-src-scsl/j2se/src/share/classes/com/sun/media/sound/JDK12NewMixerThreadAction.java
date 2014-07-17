/*
 * @(#)JDK12NewMixerThreadAction.java	1.10 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.media.sound;

import java.security.PrivilegedAction;
import java.lang.reflect.Constructor;

class JDK12NewMixerThreadAction implements java.security.PrivilegedAction {

    static Constructor cons;
    private String name;

    static {
	try {
	    cons = JDK12NewMixerThreadAction.class.getConstructor(new Class[0]);
	} catch (Throwable e) {
	    if (Printer.err)Printer.err("JDK12NewMixerThreadAction threw "+e);
	}
    }


    public JDK12NewMixerThreadAction() {
	
    }

    public Object run() {
	try {
	    return new MixerThread();
	} catch (Throwable t) {
	    return null;
	}
    }

}

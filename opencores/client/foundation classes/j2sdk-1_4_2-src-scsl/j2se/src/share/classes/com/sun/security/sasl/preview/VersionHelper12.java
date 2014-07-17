/*
 * @(#)VersionHelper12.java	1.7 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.security.sasl.preview;

import java.security.AccessController;
import java.security.PrivilegedAction;
import java.security.PrivilegedActionException;
import java.util.Enumeration;
import java.util.Iterator;

import sun.misc.Service;

final class VersionHelper12 extends VersionHelper {
    VersionHelper12() {} // Disallow external from creating one of these.

    Class loadClass(String className) throws ClassNotFoundException {
	ClassLoader cl = getContextClassLoader();
	return Class.forName(className, true, cl);
    }

    String getSaslProperty(final int i) {
	return (String) AccessController.doPrivileged(
	    new PrivilegedAction() {
		public Object run() {
		    try {
			return System.getProperty(PROPS[i]);
		    } catch (SecurityException e) {
			return null;
		    }
	        }
	    }
	);
    }

    private ClassLoader getContextClassLoader() {
	return (ClassLoader) AccessController.doPrivileged(
	    new PrivilegedAction() {
		public Object run() {
		    return Thread.currentThread().getContextClassLoader();
		}
	    }
	);
    }

    Enumeration getFactoriesFromJars(Class cls) {
	ClassLoader cl = getContextClassLoader();
	Iterator iter = Service.providers(cls, cl);
	return new IterToEnum(iter);
    }

    static class IterToEnum implements Enumeration {
	private Iterator iter;
	IterToEnum(Iterator iter) {
	    this.iter = iter;
	}

	public boolean hasMoreElements() {
	    Boolean answer = (Boolean) AccessController.doPrivileged(
		new PrivilegedAction() {
		public Object run() {
		    return new Boolean(iter.hasNext());
		}
	    });
	    return answer.booleanValue();
	}

	public Object nextElement() {
	    return iter.next();
	}
    }
}

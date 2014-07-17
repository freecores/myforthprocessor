/*
 * @(#)MergedEnumeration.java	1.6 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.security.sasl.preview;

import java.util.StringTokenizer;
import java.util.Enumeration;
import java.util.Map;
import java.util.ArrayList;
import java.util.List;

/**
  * Returns lazily evaled list of factories, starting with the
  * installed factory.
  * @author Rosanna Lee
  */
final class MergedEnumeration implements Enumeration {
    private static final VersionHelper helper = VersionHelper.getVersionHelper();

    private Object first;
    private Object nextRet = null;
    private String propName, className;
    private int propPosn;
    private Class factoryClass;
    private boolean triedProp = false;
    private boolean triedSys = false;
    private boolean triedJars = false;
    private Enumeration currentEnum;
    private Map props;

    MergedEnumeration(Object first, String propName, int propPosn, 
	String className, Class factoryClass, Map props) {
	this.first = first;
	this.propName = propName;
	this.propPosn = propPosn;
	this.className = className;
	this.props = props;
	this.factoryClass = factoryClass;
    }

    public boolean hasMoreElements() {
	if (nextRet != null) {
	    return true;
	} else {
	    return (nextRet = getNext()) != null;
	}
    }

    public Object nextElement() {
	if (nextRet == null) {
	    return getNext();
	} else {
	    Object answer = nextRet;
	    nextRet = null;
	    return answer;
	}
    }

    private Enumeration getNextEnum() {
	Enumeration answer;

	if (!triedProp) {
	    triedProp = true;
	    answer = getFactories((String) props.get(propName));

	    if (answer != null) {
		return answer;
	    }
	}

	if (!triedSys) {
	    triedSys = true;
	    answer = getFactories(helper.getSaslProperty(propPosn)); 
	    if (answer != null) {
		return answer;
	    }
	}

	if (!triedJars) {
	    triedJars = true;
	    answer = helper.getFactoriesFromJars(factoryClass); 
	    if (answer != null) {
		return answer;
	    }
	}
	return null;
    }

    private Object getNext() {
	Object answer;
	if (first != null) {
	    answer = first;
	    first = null;
	    return answer;
	}

	while (true) {
	    if (currentEnum == null) {
		currentEnum = getNextEnum();
	    }

	    if (currentEnum == null) {
		return null;
	    }

	    if (currentEnum.hasMoreElements()) {
		answer = currentEnum.nextElement();
		if (answer != null) {
		    return answer;
		}
	    }
	    currentEnum = null;
	}
    }

    private Enumeration getFactories(String propVal) {

	if (propVal == null) {
	    return null;
	}

	// Populate list with classes named in facProp; skipping
	// those that we cannot load
	StringTokenizer parser = new StringTokenizer(propVal, "|");
	List factories = new ArrayList(5);
	String pkg, clsName;
	while (parser.hasMoreTokens()) {
	    try {
		// System.out.println("loading");
		pkg = parser.nextToken().trim();
		clsName = pkg + "." + className;
		factories.add(helper.loadClass(clsName));
	    } catch (Exception e) {
		// ignore ClassNotFoundException, IllegalArgumentException
	    }
	}
	if (factories.size() == 0) {
	    return null;
	}
	return new FactoryEnumeration(factories);
    }
}

/*
 * @(#)FactoryEnumeration.java	1.5 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.security.sasl.preview;

import java.util.List;
import java.util.Enumeration;
import javax.naming.NamingException;

/**
  * The FactoryEnumeration is used for returning factory instances.
  * 
  * @author Rosanna Lee
  * @author Scott Seligman
 */

// no need to implement Enumeration since this is only for internal use
final class FactoryEnumeration implements Enumeration { 
    private List factories;
    private int posn = 0;

    /**
     * Records the input list and uses it directly to satisfy
     * hasMore()/next() requests. An alternative would have been to
     * get an Enumeration or Iterator from the list, 
     * but we want to update the list so we keep the
     * original list. The list initially contains Class objects.
     * As each element is used, the Class object is replaced by an
     * instance of the Class itself; eventually, the list contains
     * only a list of factory instances and no more updates are required.
     * 
     * @param A non-null list.
     */
    FactoryEnumeration(List factories) {
	this.factories = factories;
    }
 
    public Object nextElement() {
	synchronized (factories) {
	    Object answer = factories.get(posn++);

	    if (!(answer instanceof Class)) {
		return answer;
	    }

	    // Still a Class; need to instantiate
	    try {
		answer = ((Class)answer).newInstance();
		factories.set(posn-1, answer);  // replace Class object
		return answer;
	    } catch (InstantiationException e) {
		throw new InstantiationError(
		    "Cannot instantiate " + answer);
	    } catch (IllegalAccessException e) {
		throw new IllegalAccessError("Cannot access " + answer);
	    }
	}
    }

    public boolean hasMoreElements() {
	synchronized (factories) {
	    return posn < factories.size();
	}
    }
}

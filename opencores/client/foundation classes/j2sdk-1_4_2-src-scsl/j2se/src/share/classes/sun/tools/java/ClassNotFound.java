/*
 * @(#)ClassNotFound.java	1.14 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.java;

/**
 * This exception is thrown when a class definition is needed
 * and the class can't be found.
 */
public
class ClassNotFound extends Exception {
    /**
     * The class that was not found
     */
    public Identifier name;

    /**
     * Create a ClassNotFound exception
     */
    public ClassNotFound(Identifier nm) {
	super(nm.toString());
	name = nm;
    }
}

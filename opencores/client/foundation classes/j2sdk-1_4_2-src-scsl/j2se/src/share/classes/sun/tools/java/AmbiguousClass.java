/*
 * @(#)AmbiguousClass.java	1.16 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.java;

/**
 * This exception is thrown when an unqualified class name
 * is used that can be resolved in more than one way.
 */

public
class AmbiguousClass extends ClassNotFound {
    /**
     * The class that was not found
     */
    public Identifier name1;
    public Identifier name2;

    /**
     * Constructor
     */
    public AmbiguousClass(Identifier name1, Identifier name2) {
	super(name1.getName());
	this.name1 = name1;
	this.name2 = name2;
    }
}

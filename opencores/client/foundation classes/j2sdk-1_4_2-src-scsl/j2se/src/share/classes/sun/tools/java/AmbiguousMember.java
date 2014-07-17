/*
 * @(#)AmbiguousMember.java	1.16 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.java;

import java.util.Enumeration;

/**
 * This exception is thrown when a field reference is
 * ambiguous.
 */
public
class AmbiguousMember extends Exception {
    /**
     * The field that was not found
     */
    public MemberDefinition field1;
    public MemberDefinition field2;

    /**
     * Constructor
     */
    public AmbiguousMember(MemberDefinition field1, MemberDefinition field2) {
	super(field1.getName() + " + " + field2.getName());
	this.field1 = field1;
	this.field2 = field2;
    }
}

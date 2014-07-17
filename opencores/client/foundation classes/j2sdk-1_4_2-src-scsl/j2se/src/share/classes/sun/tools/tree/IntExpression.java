/*
 * @(#)IntExpression.java	1.19 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.tree;

import sun.tools.java.*;
import java.io.PrintStream;

public
class IntExpression extends IntegerExpression {
    /**
     * Constructor
     */
    public IntExpression(long where, int value) {
	super(INTVAL, where, Type.tInt, value);
    }

    /**
     * Equality, this is needed so that switch statements
     * can put IntExpressions in a hashtable
     */
    public boolean equals(Object obj) {
	if ((obj != null) && (obj instanceof IntExpression)) {
	    return value == ((IntExpression)obj).value;
	}
	return false;
    }

    /**
     * Hashcode, this is needed so that switch statements
     * can put IntExpressions in a hashtable
     */
    public int hashCode() {
	return value;
    }

    /**
     * Print
     */
    public void print(PrintStream out) {
	out.print(value);
    }
}

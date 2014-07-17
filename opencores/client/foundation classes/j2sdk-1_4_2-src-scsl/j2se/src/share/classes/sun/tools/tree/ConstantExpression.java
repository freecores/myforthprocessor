/*
 * @(#)ConstantExpression.java	1.18 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.tree;

import sun.tools.java.*;

class ConstantExpression extends Expression {
    /**
     * Constructor
     */
    public ConstantExpression(int op, long where, Type type) {
	super(op, where, type);
    }

    /**
     * Return true if constant
     */
    public boolean isConstant() {
        return true;
    }
}

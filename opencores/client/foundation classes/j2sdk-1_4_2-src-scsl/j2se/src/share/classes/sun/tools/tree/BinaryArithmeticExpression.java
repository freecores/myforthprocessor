/*
 * @(#)BinaryArithmeticExpression.java	1.18 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.tree;

import sun.tools.java.*;

public
class BinaryArithmeticExpression extends BinaryExpression {
    /**
     * constructor
     */
    public BinaryArithmeticExpression(int op, long where, Expression left, Expression right) {
	super(op, where, left.type, left, right);
    }

    /**
     * Select the type
     */
    void selectType(Environment env, Context ctx, int tm) {
	if ((tm & TM_DOUBLE) != 0) {
	    type = Type.tDouble;
	} else if ((tm & TM_FLOAT) != 0) {
	    type = Type.tFloat;
	} else if ((tm & TM_LONG) != 0) {
	    type = Type.tLong;
	} else {
	    type = Type.tInt;
	} 
	left = convert(env, ctx, type, left);
	right = convert(env, ctx, type, right);
    }
}

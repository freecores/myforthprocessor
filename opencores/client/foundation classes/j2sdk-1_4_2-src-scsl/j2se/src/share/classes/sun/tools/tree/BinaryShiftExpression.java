/*
 * @(#)BinaryShiftExpression.java	1.21 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.tree;

import sun.tools.java.*;

public
class BinaryShiftExpression extends BinaryExpression {
    /**
     * constructor
     */
    public BinaryShiftExpression(int op, long where, Expression left, Expression right) {
	super(op, where, left.type, left, right);
    }

    /**
     * Evaluate the expression
     */
    Expression eval() {
	// The eval code in BinaryExpression.java only works correctly
	// for arithmetic expressions.  For shift expressions, we get cases
	// where the left and right operand may legitimately be of mixed
	// types (long and int).  This is a fix for 4082814.
	if (left.op == LONGVAL && right.op == INTVAL) {
	    return eval(((LongExpression)left).value,
			((IntExpression)right).value);
	}

	// Delegate the rest of the cases to our parent, so as to minimize
	// impact on existing behavior.
	return super.eval();
    }

    /**
     * Select the type
     */
    void selectType(Environment env, Context ctx, int tm) {
	if (left.type == Type.tLong) {
	    type = Type.tLong;
	} else if (left.type.inMask(TM_INTEGER)) { 
	    type = Type.tInt;
	    left = convert(env, ctx, type, left);
	} else {
	    type = Type.tError;
	}
	if (right.type.inMask(TM_INTEGER)) {
	    right = new ConvertExpression(where, Type.tInt, right);
	} else {
	    right = convert(env, ctx, Type.tInt, right);
	}
    }
}

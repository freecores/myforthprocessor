/*
 * @(#)BinaryEqualityExpression.java	1.21 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.tree;

import sun.tools.java.*;

public
class BinaryEqualityExpression extends BinaryExpression {
    /**
     * constructor
     */
    public BinaryEqualityExpression(int op, long where, Expression left, Expression right) {
	super(op, where, Type.tBoolean, left, right);
    }

    /**
     * Select the type
     */
    void selectType(Environment env, Context ctx, int tm) {
	Type t;
	if ((tm & TM_ERROR) != 0) {
	    // who cares.  One of them is an error.
	    return;
	} else if ((tm & (TM_CLASS | TM_ARRAY | TM_NULL)) != 0) {
	    try {
		if (env.explicitCast(left.type, right.type) ||
		    env.explicitCast(right.type, left.type)) {
		    return;
		}
		env.error(where, "incompatible.type", 
			  left.type, left.type, right.type);
	    } catch (ClassNotFound e) {
		env.error(where, "class.not.found", e.name, opNames[op]);
	    }
	    return;
	} else if ((tm & TM_DOUBLE) != 0) {
	    t = Type.tDouble;
	} else if ((tm & TM_FLOAT) != 0) {
	    t = Type.tFloat;
	} else if ((tm & TM_LONG) != 0) {
	    t = Type.tLong;
	} else if ((tm & TM_BOOLEAN) != 0) {
	    t = Type.tBoolean;
	} else {
	    t = Type.tInt;
	} 
	left = convert(env, ctx, t, left);
	right = convert(env, ctx, t, right);
    }
}

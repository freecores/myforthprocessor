/*
 * @(#)NegativeExpression.java	1.18 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.tree;

import sun.tools.java.*;
import sun.tools.asm.Assembler;

public
class NegativeExpression extends UnaryExpression {
    /**
     * Constructor
     */
    public NegativeExpression(long where, Expression right) {
	super(NEG, where, right.type, right);
    }
 
    /**
     * Select the type of the expression
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
	right = convert(env, ctx, type, right);
    }

    /**
     * Evaluate
     */
    Expression eval(int a) {
	return new IntExpression(where, -a);
    }
    Expression eval(long a) {
	return new LongExpression(where, -a);
    }
    Expression eval(float a) {
	return new FloatExpression(where, -a);
    }
    Expression eval(double a) {
	return new DoubleExpression(where, -a);
    }

    /**
     * Simplify
     */
    Expression simplify() {
	if (right.op == NEG) {
	    return ((NegativeExpression)right).right;
	}
	return this;
    }

    /**
     * Code
     */
    public void codeValue(Environment env, Context ctx, Assembler asm) {
	right.codeValue(env, ctx, asm);
	asm.add(where, opc_ineg + type.getTypeCodeOffset());
    }
}
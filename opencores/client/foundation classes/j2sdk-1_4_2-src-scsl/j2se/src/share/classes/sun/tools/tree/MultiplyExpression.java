/*
 * @(#)MultiplyExpression.java	1.18 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.tree;

import sun.tools.java.*;
import sun.tools.asm.Assembler;

public
class MultiplyExpression extends BinaryArithmeticExpression {
    /**
     * constructor
     */
    public MultiplyExpression(long where, Expression left, Expression right) {
	super(MUL, where, left, right);
    }

    /**
     * Evaluate
     */
    Expression eval(int a, int b) {
	return new IntExpression(where, a * b);
    }
    Expression eval(long a, long b) {
	return new LongExpression(where, a * b);
    }
    Expression eval(float a, float b) {
	return new FloatExpression(where, a * b);
    }
    Expression eval(double a, double b) {
	return new DoubleExpression(where, a * b);
    }

    /**
     * Simplify
     */
    Expression simplify() {
	if (left.equals(1)) {
	    return right;
	}
	if (right.equals(1)) {
	    return left;
	}
	return this;
    }

    /**
     * Code
     */
    void codeOperation(Environment env, Context ctx, Assembler asm) {
	asm.add(where, opc_imul + type.getTypeCodeOffset());
    }
}

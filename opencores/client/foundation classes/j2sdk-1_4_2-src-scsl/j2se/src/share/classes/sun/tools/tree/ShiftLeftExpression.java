/*
 * @(#)ShiftLeftExpression.java	1.19 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.tree;
import sun.tools.java.*;
import sun.tools.asm.Assembler;

public
class ShiftLeftExpression extends BinaryShiftExpression {
    /**
     * constructor
     */
    public ShiftLeftExpression(long where, Expression left, Expression right) {
	super(LSHIFT, where, left, right);
    }

    /**
     * Evaluate
     */
    Expression eval(int a, int b) {
	return new IntExpression(where, a << b);
    }
    Expression eval(long a, long b) {
	return new LongExpression(where, a << b);
    }

    /**
     * Simplify
     */
    Expression simplify() {
	if (right.equals(0)) {
	    return left;
	}
	if (left.equals(0)) {
	    return new CommaExpression(where, right, left).simplify();
	}
	return this;
    }

    /**
     * Code
     */
    void codeOperation(Environment env, Context ctx, Assembler asm) {
	asm.add(where, opc_ishl + type.getTypeCodeOffset());
    }
}

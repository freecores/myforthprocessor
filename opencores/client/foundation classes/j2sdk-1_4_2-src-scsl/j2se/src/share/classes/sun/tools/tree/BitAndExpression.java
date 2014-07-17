/*
 * @(#)BitAndExpression.java	1.22 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.tree;

import sun.tools.java.*;
import sun.tools.asm.Assembler;

public
class BitAndExpression extends BinaryBitExpression {
    /**
     * constructor
     */
    public BitAndExpression(long where, Expression left, Expression right) {
	super(BITAND, where, left, right);
    }

    /**
     * Evaluate
     */
    Expression eval(boolean a, boolean b) {
	return new BooleanExpression(where, a & b);
    }
    Expression eval(int a, int b) {
	return new IntExpression(where, a & b);
    }
    Expression eval(long a, long b) {
	return new LongExpression(where, a & b);
    }

    /**
     * Simplify
     */
    Expression simplify() {
	if (left.equals(true)) 
	    return right;
	if (right.equals(true)) 
	    return left;
	if (left.equals(false) || left.equals(0))
	    return new CommaExpression(where, right, left).simplify();
	if (right.equals(false) || right.equals(0))
	    return new CommaExpression(where, left, right).simplify();
	return this;
    }

    /**
     * Code
     */
    void codeOperation(Environment env, Context ctx, Assembler asm) {
	asm.add(where, opc_iand + type.getTypeCodeOffset());
    }
}

/*
 * @(#)BinaryBitExpression.java	1.22 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.tree;

import sun.tools.java.*;
import sun.tools.asm.Assembler;
import java.util.Hashtable;

abstract public
class BinaryBitExpression extends BinaryExpression {
    /**
     * constructor
     */
    public BinaryBitExpression(int op, long where, Expression left, Expression right) {
	super(op, where, left.type, left, right);
    }

    /**
     * Select the type
     */
    void selectType(Environment env, Context ctx, int tm) {
	if ((tm & TM_BOOLEAN) != 0) {
	    type = Type.tBoolean;
	} else if ((tm & TM_LONG) != 0) {
	    type = Type.tLong;
	} else {
	    type = Type.tInt;
	} 
	left = convert(env, ctx, type, left);
	right = convert(env, ctx, type, right);
    }

    /**
     * Code
     */
    public void codeValue(Environment env, Context ctx, Assembler asm) {
	left.codeValue(env, ctx, asm);
	right.codeValue(env, ctx, asm);
	codeOperation(env, ctx, asm);
    }
}

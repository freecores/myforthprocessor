/*
 * @(#)LengthExpression.java	1.21 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.tree;

import sun.tools.java.*;
import sun.tools.asm.Assembler;
import java.util.Hashtable;

public
class LengthExpression extends UnaryExpression {
    /**
     * Constructor
     */
    public LengthExpression(long where, Expression right) {
	super(LENGTH, where, Type.tInt, right);
    }
 
    /**
     * Select the type of the expression
     */
    public Vset checkValue(Environment env, Context ctx, Vset vset, Hashtable exp) {
	vset = right.checkValue(env, ctx, vset, exp);
	if (!right.type.isType(TC_ARRAY)) {
	    env.error(where, "invalid.length", right.type);
	}
	return vset;
    }

    /**
     * Code
     */
    public void codeValue(Environment env, Context ctx, Assembler asm) {
	right.codeValue(env, ctx, asm);
	asm.add(where, opc_arraylength);
    }
}

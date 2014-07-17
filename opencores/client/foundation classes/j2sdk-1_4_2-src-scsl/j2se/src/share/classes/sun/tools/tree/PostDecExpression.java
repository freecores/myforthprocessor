/*
 * @(#)PostDecExpression.java	1.18 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.tree;

import sun.tools.java.*;
import sun.tools.asm.Assembler;

public
class PostDecExpression extends IncDecExpression {
    /**
     * Constructor
     */
    public PostDecExpression(long where, Expression right) {
	super(POSTDEC, where, right);
    }

    /**
     * Code
     */
    public void codeValue(Environment env, Context ctx, Assembler asm) {
	codeIncDec(env, ctx, asm, false, false, true);
    }
    public void code(Environment env, Context ctx, Assembler asm) {
	codeIncDec(env, ctx, asm, false, false, false);
    }
}

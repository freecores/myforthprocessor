/*
 * @(#)PostIncExpression.java	1.18 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.tree;

import sun.tools.java.*;
import sun.tools.asm.Assembler;

public
class PostIncExpression extends IncDecExpression {
    /**
     * Constructor
     */
    public PostIncExpression(long where, Expression right) {
	super(POSTINC, where, right);
    }

    /**
     * Code
     */
    public void codeValue(Environment env, Context ctx, Assembler asm) {
	codeIncDec(env, ctx, asm, true, false, true);
    }
    public void code(Environment env, Context ctx, Assembler asm) {
	codeIncDec(env, ctx, asm, true, false, false);
    }
}

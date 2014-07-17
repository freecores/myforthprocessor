/*
 * @(#)PreIncExpression.java	1.18 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.tree;

import sun.tools.java.*;
import sun.tools.asm.Assembler;

public
class PreIncExpression extends IncDecExpression {
    /**
     * Constructor
     */
    public PreIncExpression(long where, Expression right) {
	super(PREINC, where, right);
    }

    /**
     * Code
     */
    public void codeValue(Environment env, Context ctx, Assembler asm) {
	codeIncDec(env, ctx, asm, true, true, true);
    }
    public void code(Environment env, Context ctx, Assembler asm) {
	codeIncDec(env, ctx, asm, true, true, false);
    }
}

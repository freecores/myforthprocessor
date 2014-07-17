/*
 * @(#)AssignBitOrExpression.java	1.17 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.tree;

import sun.tools.java.*;
import sun.tools.asm.Assembler;

public
class AssignBitOrExpression extends AssignOpExpression {
    /**
     * Constructor
     */
    public AssignBitOrExpression(long where, Expression left, Expression right) {
	super(ASGBITOR, where, left, right);
    }


    /**
     * Code
     */
    void codeOperation(Environment env, Context ctx, Assembler asm) {
	asm.add(where, opc_ior + itype.getTypeCodeOffset());
    }
}

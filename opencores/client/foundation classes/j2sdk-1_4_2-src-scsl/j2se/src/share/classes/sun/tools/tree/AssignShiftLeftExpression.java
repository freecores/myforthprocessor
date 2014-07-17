/*
 * @(#)AssignShiftLeftExpression.java	1.17 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.tree;

import sun.tools.java.*;
import sun.tools.asm.Assembler;

public
class AssignShiftLeftExpression extends AssignOpExpression {
    /**
     * Constructor
     */
    public AssignShiftLeftExpression(long where, Expression left, Expression right) {
	super(ASGLSHIFT, where, left, right);
    }

 
    /**
     * Code
     */
    void codeOperation(Environment env, Context ctx, Assembler asm) {
	asm.add(where, opc_ishl + itype.getTypeCodeOffset());
    }
}

/*
 * @(#)NullExpression.java	1.20 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.tree;

import sun.tools.java.*;
import sun.tools.asm.Assembler;
import java.io.PrintStream;

public
class NullExpression extends ConstantExpression {
    /**
     * Constructor
     */
    public NullExpression(long where) {
	super(NULL, where, Type.tNull);
    }

    /**
     * Check if the expression is equal to a value
     */
    public boolean equals(int i) {
	return i == 0;
    }

    public boolean isNull() {
	return true;
    }

    /**
     * Code
     */
    public void codeValue(Environment env, Context ctx, Assembler asm) {
	asm.add(where, opc_aconst_null);
    }

    /**
     * Print
     */
    public void print(PrintStream out) {
	out.print("null");
    }
}

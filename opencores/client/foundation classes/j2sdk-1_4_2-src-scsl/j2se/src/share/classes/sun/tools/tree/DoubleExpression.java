/*
 * @(#)DoubleExpression.java	1.20 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.tree;

import sun.tools.java.*;
import sun.tools.asm.Assembler;
import java.io.PrintStream;

public
class DoubleExpression extends ConstantExpression {
    double value;
    
    /**
     * Constructor
     */
    public DoubleExpression(long where, double value) {
	super(DOUBLEVAL, where, Type.tDouble);
	this.value = value;
    }

    /**
     * Get the value
     */
    public Object getValue() {
	return new Double(value);
    }

    /**
     * Check if the expression is equal to a value
     */
    public boolean equals(int i) {
        return value == i;
    }

    /**
     * Check if the expression is equal to its default static value
     */
    public boolean equalsDefault() {
        // don't allow -0.0
        return (Double.doubleToLongBits(value) == 0);
    }

    /**
     * Code
     */
    public void codeValue(Environment env, Context ctx, Assembler asm) {
	asm.add(where, opc_ldc2_w, new Double(value));
    }

    /**
     * Print
     */
    public void print(PrintStream out) {
	out.print(value + "D");
    }
}

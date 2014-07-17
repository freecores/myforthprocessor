/*
 * @(#)TypeExpression.java	1.24 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.tree;

import sun.tools.java.*;
import java.io.PrintStream;
import java.util.Hashtable;

public
class TypeExpression extends Expression {
    /**
     * Constructor
     */
    public TypeExpression(long where, Type type) {
	super(TYPE, where, type);
    }

    /**
     * Convert to a type
     */
    Type toType(Environment env, Context ctx) {
	return type;
    }

    /**
     * Check an expression
     */
    public Vset checkValue(Environment env, Context ctx, Vset vset, Hashtable exp) {
	env.error(where, "invalid.term");
	type = Type.tError;
	return vset;
    }

    public Vset checkAmbigName(Environment env, Context ctx, Vset vset, Hashtable exp,
			       UnaryExpression loc) {
	return vset;
    }

    public Expression inline(Environment env, Context ctx) {
	return null;
    }

    /**
     * Print
     */
    public void print(PrintStream out) {
	out.print(type.toString());
    }
}

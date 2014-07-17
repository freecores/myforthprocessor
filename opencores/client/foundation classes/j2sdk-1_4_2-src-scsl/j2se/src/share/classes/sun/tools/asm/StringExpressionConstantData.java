/*
 * @(#)StringExpressionConstantData.java	1.17 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.asm;

import sun.tools.java.*;
import sun.tools.tree.StringExpression;
import java.io.IOException;
import java.io.DataOutputStream;

/**
 * This is a string expression constant. This constant
 * represents an Java string constant.
 */
final
class StringExpressionConstantData extends ConstantPoolData {
    StringExpression str;

    /**
     * Constructor
     */
    StringExpressionConstantData(ConstantPool tab, StringExpression str) {
	this.str = str;
	tab.put(str.getValue());
    }

    /**
     * Write the constant to the output stream
     */
    void write(Environment env, DataOutputStream out, ConstantPool tab) throws IOException {
	out.writeByte(CONSTANT_STRING);
	out.writeShort(tab.index(str.getValue()));
    }

    /**
     * Return the order of the constant
     */
    int order() {
	return 0;
    }

    /**
     * toString
     */
    public String toString() {
	return "StringExpressionConstantData[" + str.getValue() + "]=" + str.getValue().hashCode();
    }
}
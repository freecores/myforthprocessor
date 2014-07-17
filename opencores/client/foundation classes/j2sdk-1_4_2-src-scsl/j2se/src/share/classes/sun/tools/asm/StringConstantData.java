/*
 * @(#)StringConstantData.java	1.18 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.asm;

import sun.tools.java.*;
import java.io.IOException;
import java.io.DataOutputStream;

/**
 * This is a string constant pool data item.
 */
final
class StringConstantData extends ConstantPoolData {
    String str;

    /**
     * Constructor
     */
    StringConstantData(ConstantPool tab, String str) {
	this.str = str;
    }

    /**
     * Write the constant to the output stream
     */
    void write(Environment env, DataOutputStream out, ConstantPool tab) throws IOException {
	out.writeByte(CONSTANT_UTF8);
	out.writeUTF(str);
    }

    /**
     * Return the order of the constant
     */
    int order() {
	return 4;
    }

    /**
     * toString
     */
    public String toString() {
	return "StringConstantData[" + str + "]=" + str.hashCode();
    }
}

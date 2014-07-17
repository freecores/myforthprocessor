/*
 * @(#)NameAndTypeConstantData.java	1.16 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.asm;

import sun.tools.java.*;
import java.io.IOException;
import java.io.DataOutputStream;

/**
 * This is a name and type constant pool data item
 */
final
class NameAndTypeConstantData extends ConstantPoolData {
    String name;
    String type;

    /**
     * Constructor
     */
    NameAndTypeConstantData(ConstantPool tab, NameAndTypeData nt) {
	name = nt.field.getName().toString();
	type = nt.field.getType().getTypeSignature();
	tab.put(name);
	tab.put(type);
    }

    /**
     * Write the constant to the output stream
     */
    void write(Environment env, DataOutputStream out, ConstantPool tab) throws IOException {
	out.writeByte(CONSTANT_NAMEANDTYPE);
	out.writeShort(tab.index(name));
	out.writeShort(tab.index(type));
    }

    /**
     * Return the order of the constant
     */
    int order() {
	return 3;
    }
}

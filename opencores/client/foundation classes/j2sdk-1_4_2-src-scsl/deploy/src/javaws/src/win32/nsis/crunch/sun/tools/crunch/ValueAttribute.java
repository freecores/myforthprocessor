/*
 * @(#)ValueAttribute.java	1.3 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.crunch;

/**
 * Describes a "ConstantValue" attribute on a Field.
 *
 * @author Graham Hamilton
 */

import java.io.*;

public class ValueAttribute extends Attribute {
    private ClassFile cfile;
    private short valueIndex;

    private static int count;
    // Statistics by tag value.
    private static int tagCounts[] = new int[16];

    public static Attribute read(ClassFile cf, DataInputStream din) throws IOException {
	return new ValueAttribute(cf, din);
    }

    private ValueAttribute(ClassFile cf, DataInputStream din)
									throws IOException {
	cfile = cf;
	int length = din.readInt();

	if (length != 2) {
	    throw new Error("Unexpected length in ConstantValue attribute");
	}

	valueIndex = din.readShort();	

	// Keep track of statistics.
	count++;
	int tag = cf.constants[valueIndex].tag;
	tagCounts[tag]++;
    }

    public void write(DataOutputStream out) throws IOException {
	out.writeByte(GlobalTables.valueAttribute);
	Object value = cfile.constants[valueIndex].getValue();
	out.writeShort(GlobalTables.findValue(value));
    }

    public static void summarize() {
	System.out.println("    " + count + " ConstantValue attributes," +
			   " written length = " + (count * 3) + " bytes");
	if (false) {
	    for (int i = 0; i < 16; i++) {
	        if (tagCounts[i] != 0) {
		    System.out.println("        " + tagCounts[i] + " of type " +
								Constant.tagNames[i]);
		}
	    }
	}
    }
}

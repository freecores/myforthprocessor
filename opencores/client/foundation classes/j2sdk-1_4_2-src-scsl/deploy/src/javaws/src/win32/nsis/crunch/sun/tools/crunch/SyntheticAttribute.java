/*
 * @(#)SyntheticAttribute.java	1.3 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.crunch;

/**
 * Describes an empty "synthetic" attribute
 *
 * @author Graham Hamilton
 */

import java.io.*;

public class SyntheticAttribute extends Attribute {
    private ClassFile cfile;
    private static int count;

    public static Attribute read(ClassFile cf, DataInputStream din) throws IOException {
	return new SyntheticAttribute(cf, din);
    }

    private SyntheticAttribute(ClassFile cf, DataInputStream din) throws IOException {
	cfile = cf;
	int length = din.readInt();
	if (length != 0) {
	    throw new Error("Unexpected length in Synthetic attribute");
	}
	count++;
    }

    public void write(DataOutputStream out) throws IOException {
	out.writeByte(GlobalTables.syntheticAttribute);
    }

    public static void summarize() {
	System.out.println("    " + count + " Synthetic attributes, written length = "
			+ (count * 1) + " bytes");
    }
}

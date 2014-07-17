/*
 * @(#)InnerClassesAttribute.java	1.3 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.crunch;

/**
 * Describes an "InnerClasses" attribute on a classfile.
 *
 * @author Graham Hamilton
 */

import java.io.*;

public class InnerClassesAttribute extends Attribute {
    private ClassFile cfile;
    private short inners[];
    private short outers[];
    private short names[];
    private short flags[];

    private static int count;
    private static int totalLength;

    public static Attribute read(ClassFile cf, DataInputStream din) throws IOException {
	return new InnerClassesAttribute(cf, din);
    }

    private InnerClassesAttribute(ClassFile cf, DataInputStream din) throws IOException {
	cfile = cf;

	// Skip the attribute length.  We don't need it.
	din.readInt();

	int size = din.readShort();
	if (size > 255) {
	    throw new Error("too many entries in InnerClasses attribute");
	}
	inners = new short[size];
	outers = new short[size];
	names = new short[size];
	flags = new short[size];
	for (int i = 0; i < size; i++) {
	    inners[i] = din.readShort();
	    outers[i] = din.readShort();
	    names[i] = din.readShort();
	    flags[i] = din.readShort();
	}

	// Keep track of statistics.
	count++;
        totalLength += 2 + (8 * size);
    }

    public void write(DataOutputStream out) throws IOException {
	out.writeByte(GlobalTables.innerClassesAttribute);
	out.writeByte(inners.length);
	for (int i = 0; i < inners.length; i++) {
	    out.writeShort(cfile.mapClass(inners[i]));
	    out.writeShort(cfile.mapClass(outers[i]));
	    out.writeShort(cfile.mapUtf(names[i]));
	    out.writeShort(flags[i]);
	}
    }

    public static void summarize() {
	System.out.println("    " + count + " InnerClasses attributes, written length = "
			+ totalLength + " bytes");
    }
}

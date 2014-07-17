/*
 * @(#)ExceptionsAttribute.java	1.3 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.crunch;

/**
 * Describes an "Exceptions" attribute on a Method.
 *
 * @author Graham Hamilton
 */

import java.io.*;

public class ExceptionsAttribute extends Attribute {
    private ClassFile cfile;
    private short exceptions[];

    private static int count;
    private static int totalLength;

    public static Attribute read(ClassFile cf, DataInputStream din) throws IOException {
	return new ExceptionsAttribute(cf, din);
    }

    private ExceptionsAttribute(ClassFile cf, DataInputStream din) throws IOException {
	cfile = cf;

	// Skip the attribute length.  We don't need it.
	din.readInt();

	int size = din.readShort();
	if (size > 255) {
	    throw new Error("too many exceptions in exceptions attribute");
	}
	exceptions = new short[size];
	for (int i = 0; i < size; i++) {
	    exceptions[i] = din.readShort();
	}

	// Keep track of statistics.
	count++;
        totalLength += 2 + (2 * size);
    }

    public void write(DataOutputStream out) throws IOException {
	out.writeByte(GlobalTables.exceptionsAttribute);
	out.writeByte(exceptions.length);
	for (int i = 0; i < exceptions.length; i++) {
	    short fix = exceptions[i];
	    fix = cfile.mapClass(fix);
	    out.writeShort(fix);
	}
    }

    public static void summarize() {
	System.out.println("    " + count + " Exceptions attributes, written length = "
			+ totalLength + " bytes");
    }
}

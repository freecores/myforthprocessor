/*
 * @(#)Method.java	1.3 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.tools.crunch;

/**
 * Describe a classfile method block.
 *
 * @author Graham Hamilton
 */

import java.io.*;

public class Method extends Field {
    private static int totalCount;
    private static int totalPrivates;
    private static int totalAttributes;

    public static Method readMethod(ClassFile cf, DataInputStream din) throws IOException {
        return new Method(cf, din);
    }

    private Method(ClassFile cf, DataInputStream din) throws IOException {
	cfile = cf;
	totalCount++;
	flags = din.readShort();
	if ((flags & 2) != 0) {
	    totalPrivates++;
	}
	nameIndex = din.readShort();
	typeIndex = din.readShort();

	attributes = Attribute.readAttributes(cf, din);
	totalAttributes += attributes.length;
    }

    public static void summarize() {
	System.out.print("    Total number of method definitions = " + totalCount);
        System.out.println(" (" + totalPrivates + " private)");
	System.out.println("        Total number of method attributes = " + totalAttributes);
    }
}


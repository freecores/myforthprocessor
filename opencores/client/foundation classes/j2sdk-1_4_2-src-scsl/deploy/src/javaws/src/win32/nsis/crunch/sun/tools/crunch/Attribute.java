/*
 * @(#)Attribute.java	1.3 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.crunch;

/**
 * Base class for describing classfile attributes
 *
 * @author Graham Hamilton
 */

import java.io.*;
import java.util.*;

public abstract class Attribute {

    protected Attribute() {
    }

    /**
     * Read a set of attributes, discarding boring ones.
     */
    public static Attribute[] readAttributes(ClassFile cf, DataInputStream din) throws IOException {
	int attributeCount = din.readShort();
	Vector v = new Vector();
	for (int i = 0; i < attributeCount; i++) {
	    Attribute a = Attribute.read(cf, din);
	    if (a != null) {
		v.addElement(a);
	    }
	}
	Attribute result[] = new Attribute[v.size()];
	for (int i = 0; i < result.length; i++) {
	    result[i] = (Attribute)v.elementAt(i);
    	}
	return result;
    }

    // Read an attribute. 
    // Returns null if its an attribute we want to ignore.

    private static Attribute read(ClassFile cf, DataInputStream din) throws IOException {
	// Read the constants index and figure out the name of the attribute.
	short index = din.readShort();
	String name = cf.getString(index);

	// Treat some attributes specially.
	if (name.equals("Code")) {
	    return CodeAttribute.read(cf, din);
	} else if (name.equals("ConstantValue")) {
	    return ValueAttribute.read(cf, din);
	} else if (name.equals("Exceptions")) {
	    return ExceptionsAttribute.read(cf, din);
	} else if (name.equals("Synthetic")) {
	    return SyntheticAttribute.read(cf, din);
	} else if (name.equals("InnerClasses")) {
	    return InnerClassesAttribute.read(cf, din);
	} else if (name.equals("Deprecated")) {
	    skipAttribute(din);
	    return null;
	} else if (name.equals("SourceFile")) {
	    skipAttribute(din);
	    return null;
	} else if (name.equals("LineNumberTable")) {
	    skipAttribute(din);
	    return null;
	} else if (name.equals("LocalVariableTable")) {
	    skipAttribute(din);
	    return null;
	} else if (name.equals("Signature")) {
	    skipAttribute(din);
	    return null;
	}
	throw new Error("Unexpected attribute \"" + name + "\"");
    }

    protected static void skipAttribute(DataInputStream din) throws IOException {
	int length = din.readInt();
	int skipCount = 0;
	while (skipCount < length) {
	    int rc = din.skipBytes(length-skipCount);
	    if (rc <= 0) {
		throw new Error("Skip failed");
	    }
	    skipCount += rc;
        }
    }

    public abstract void write(DataOutputStream out) throws IOException;

    public static void summarize() {
	ValueAttribute.summarize();
	ExceptionsAttribute.summarize();
	SyntheticAttribute.summarize();
	InnerClassesAttribute.summarize();
	CodeAttribute.summarize();
    }

}

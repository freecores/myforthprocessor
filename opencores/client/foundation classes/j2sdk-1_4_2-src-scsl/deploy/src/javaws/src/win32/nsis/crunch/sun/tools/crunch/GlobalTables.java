/*
 * @(#)GlobalTables.java	1.3 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.crunch;

/**
 * This represents the set of global constant information on
 * all the classfiles we have processed.
 *
 * Because we need to deal with more than 64K constants we
 * keep separate tables for differnet kinds of constants.
 * Each table is small enough that it can be indexed with
 * a 16 bit index.
 *
 * @author Graham Hamilton
 */

import java.util.*;
import java.io.*;

public class GlobalTables {

    // We write the standard attribute names to the front
    // of the UTF table.  They therefore have fixed indexes.
    public final static byte codeAttribute 	    = 1;
    public final static byte valueAttribute  	    = 2;
    public final static byte exceptionsAttribute    = 3;
    public final static byte syntheticAttribute     = 4;
    public final static byte innerClassesAttribute  = 5;
    public final static byte deprecatedAttribute    = 6;

    private static class Pool {
        Vector vector;
        Hashtable table;

	Pool() {
            vector = new Vector();
            table = new Hashtable();
	    // Entry zero is reserved.  This does not get written.
	    vector.addElement("");
        }

	// Find or create a new pool index for the given key.
	short find(Object key) {
	    Integer ix = (Integer)table.get(key);
	    if (ix == null) {
		ix = new Integer(vector.size());
		vector.addElement(key);
		table.put(key, ix);
		if (ix.intValue() >= 0x10000) {
		    throw new Error("Ran out of 16 bit pool space");
		}
	    }
	    return ix.shortValue();
	}
    }

    // We keep five segregated pools for different kinds of data.
    private static Pool utfPool = new Pool();
    private static Pool classPool = new Pool();
    private static Pool refPool = new Pool();
    private static Pool typePool = new Pool();
    private static Pool valuePool = new Pool();

    static {
	addAttributeNames();
    }

    private static void addAttributeNames() {
 	// Put the standard attribute names at the front of the UTF pool.
	findUtf("Code");
	findUtf("ConstantValue");
	findUtf("Exceptions");
	findUtf("Synthetic");
	findUtf("InnerClasses");
	short ix = findUtf("Deprecated");
	if (ix != deprecatedAttribute) {
	    throw new Error("trouble asigning attribute indexes");
	}
    }

    public static short findUtf(String utf) {
	return utfPool.find(utf);
    }

    public static short findClass(String cname) {
	// Make sure any parent package is also in the pool.
	int ix = cname.lastIndexOf('/');
	if (ix >= 0) {
	    findClass(cname.substring(0,ix));
	}
	return classPool.find(cname);
    }

    public static boolean existsRef(String className, String name, String type) {
	String key = className + " " + name + " " + type;
	Object obj = refPool.table.get(key);
	if (obj == null) {
	    return false;
	} else {
	    return true;
	}
    }

    public static short findRef(String className, String name, String type) {
	// Make sure the className, name, and type are in their own tables.
	findClass(className);
	findUtf(name);
	findType(type);
	String key = className + " " + name + " " + type;
	return refPool.find(key);
    }

    public static short findType(String type) {
	// If the given type is not already in the typetable, make
	// sure any embedded type names are in the class table.
	if (typePool.table.get(type) == null) {
	    int i = 0;
	    while (i < type.length()) {
		char ch = type.charAt(i);
		i++;
		if (ch != 'L') {
		    continue;
		}
		int end = type.indexOf(';', i);
		String cname = type.substring(i, end);
		findClass(cname);
		i = end+1;
	    }
	}
	return typePool.find(type);
    }

    public static short findValue(Object value) {
	return valuePool.find(value);
    }

    public static int writeEncodedType(java.io.DataOutputStream out, String type)
					    throws java.io.IOException {

	// We encode each classname in the type as 'L' followed by
        // a two-byte class table index.
	int len = 0;
        int i = 0;
	while (i < type.length()) {
	    char ch = type.charAt(i);
	    i++;

	    out.writeByte((byte)ch);
	    len++;
	    if (ch != 'L') {
		continue;
	    }

	    int end = type.indexOf(';', i);
	    String cname = type.substring(i, end);
	    i = end+1;

	    out.writeShort(findClass(cname));
	    len += 2;
	}
        out.writeByte(0);
	len++;
	return len;
    }

    /**
     * Write a unicode String as a null-terminated UTF string.
     */
    private static int writeUTF(java.io.DataOutputStream out, String s)
						throws java.io.IOException {
	// If it's ascii it's easy.
	boolean ascii = true;
	for (int i = 0; i < s.length(); i++) {
	    short ch = (short)s.charAt(i);
	    if (ch <= 0 || ch >= 128) {
		ascii = false;
	        break;
	    }
	}
	if (ascii) {
	    for (int i = 0; i < s.length(); i++) {
		out.writeByte((byte)s.charAt(i));
	    }
	    out.writeByte(0);
	    return (s.length() + 1);
	} else {
	    ByteArrayOutputStream bout = new ByteArrayOutputStream();
	    DataOutputStream dout = new DataOutputStream(bout);
	    dout.writeUTF(s);
	    dout.flush();
	    byte data[] = bout.toByteArray();
	    out.write(data, 2, data.length-2);
	    out.writeByte(0);
	    return (data.length - 1);
  	}
    }

    /**
     * Write out the global pools.
     */
    public static void write(java.io.DataOutputStream out) throws java.io.IOException {
	out.writeShort(utfPool.vector.size());
	out.writeShort(classPool.vector.size());
	out.writeShort(refPool.vector.size());
	out.writeShort(typePool.vector.size());
	out.writeShort(valuePool.vector.size());

        int length, overlap;
	int totalCount = 0;
        int totalLength = 0;
	Hashtable utfs = utfPool.table;

	length = 0;
	Vector v = utfPool.vector;
	for (int i = 1; i < v.size(); i++) {
	    String s = (String)v.elementAt(i);
	    length += writeUTF(out, s);
	}
	System.out.println("Wrote " + v.size() + " entries in UTF pool," +
			" estimated length = " + length);
	totalLength += length;
	totalCount += v.size();

	length = 0;
	overlap = 0;
	v = classPool.vector;
	for (int i = 1; i < v.size(); i++) {
	    String cname = (String)v.elementAt(i);

	    int ix = cname.lastIndexOf('/');
	    if (ix >= 0) {
	        short fix = classPool.find(cname.substring(0,ix));
		out.writeByte((byte)'%');
		out.writeShort(fix);
		length += 3;
		length += writeUTF(out, cname.substring(ix+1));
	    } else {
	        length += writeUTF(out, cname);
	    }
	}
	System.out.println("Wrote " + v.size() + " entries in Class pool," +
			" estimated length = " + length);
	totalLength += length;
	totalCount += v.size();

	length = 0;
	v = refPool.vector;
	for (int i = 1; i < v.size(); i++) {
	    String key = (String)v.elementAt(i);
	    int ix = key.indexOf(" ");
	    int iy = key.indexOf(" ", ix+1);
	    String cname = key.substring(0, ix);
	    String name = key.substring(ix+1, iy);
	    String type = key.substring(iy+1);
	    // System.err.println("ref: cname=\"" + cname + "\", name=\"" + 
	    //				name + "\", type = \"" + type + "\"");
	    out.writeShort(findClass(cname));
	    out.writeShort(findUtf(name));
	    out.writeShort(findType(type));
	    length += 6;
	}
	System.out.println("Wrote " + v.size() + " entries in Ref pool," +
			" estimated length = " + length);
	totalLength += length;
	totalCount += v.size();

	length = 0;
	overlap = 0;
	v = typePool.vector;
	for (int i = 1; i < v.size(); i++) {
	    String s = (String)v.elementAt(i);
	    // this string has already had classnames encoded.
	    int len = writeEncodedType(out, s);
	    length += len;
	}
	System.out.println("Wrote " + v.size() + " entries in Type pool," +
			" estimated length = " + length);
	totalLength += length;
	totalCount += v.size();

	length = 0;
	overlap = 0;
        int overlapLength = 0;
	v = valuePool.vector;
	int stotal = 0;
	int itotal= 0;
	int ltotal= 0;
	int ftotal= 0;
	int dtotal= 0;
	for (int i = 1; i < v.size(); i++) {
	    Object obj = v.elementAt(i);
	    if (obj instanceof String) {
		String s = (String)obj;
		out.writeByte(Constant.CONSTANT_String);
	        length += 1;
	        length += writeUTF(out, s);
	        if (utfs.get(s) != null) {
		    overlap++;
		    overlapLength += s.length();
	        }
		stotal++;
	    } else if (obj instanceof Integer) {
		out.writeByte(Constant.CONSTANT_Integer);
	        length += 1;
		out.writeInt(((Integer)obj).intValue());
	        length += 4;
		itotal++;
	    } else if (obj instanceof Long) {
		out.writeByte(Constant.CONSTANT_Long);
	        length += 1;
		out.writeLong(((Long)obj).longValue());
	        length += 8;
		ltotal++;
	    } else if (obj instanceof Float) {
		out.writeByte(Constant.CONSTANT_Float);
	        length += 1;
		out.writeFloat(((Float)obj).floatValue());
	        length += 4;
		ftotal++;
	    } else if (obj instanceof Double) {
		out.writeByte(Constant.CONSTANT_Double);
	        length += 1;
		out.writeDouble(((Double)obj).doubleValue());
	        length += 8;
		dtotal++;
	    } else {
		throw new Error("Unexpected type in value pool: " + obj.getClass());
	    }
	}
	System.out.println("Wrote " + v.size() + " entries in Value pool," +
			" estimated length = " + length);
	System.out.println("        " + stotal + " were Strings");
	System.out.println("            " + overlap + " (" + overlapLength +
						" bytes) were also in UTF pool");
	System.out.println("        " + itotal + " were ints");
	System.out.println("        " + ltotal + " were longs");
	System.out.println("        " + ftotal + " were floats");
	System.out.println("        " + dtotal + " were doubles");
	totalLength += length;
	totalCount += v.size();

	System.out.println("Total of " + totalCount + " pool entries," +
			" estimated length = " + totalLength);

    }
}


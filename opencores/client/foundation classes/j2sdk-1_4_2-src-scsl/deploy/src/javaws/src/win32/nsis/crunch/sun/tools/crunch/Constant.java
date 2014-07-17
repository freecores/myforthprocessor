/*
 * @(#)Constant.java	1.3 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.tools.crunch;

/**
 * Describe a classfile "constant".
 *
 * @author Graham Hamilton
 */

import java.io.*;

public class Constant {
    public final static int CONSTANT_Utf8		=  1;
    public final static int CONSTANT_Integer		=  3;
    public final static int CONSTANT_Float		=  4;
    public final static int CONSTANT_Long		=  5;
    public final static int CONSTANT_Double		=  6;
    public final static int CONSTANT_Class 		=  7;
    public final static int CONSTANT_String		=  8;
    public final static int CONSTANT_FieldRef		=  9;
    public final static int CONSTANT_MethodRef		= 10;
    public final static int CONSTANT_InterfaceMethodRef = 11;
    public final static int CONSTANT_NameAndType	= 12;

    public static String tagNames[] = { "?0?", "UTF8", "?2?", "INTEGER",
			"FLOAT", "LONG", "DOUBLE", "CLASS", "STRING",
			"FIELDREF", "METHODREF", "INTERFACEMETHODREF",
			"NAMEANDTYPE" };

    // Backpointer to our ClassFile object.
    ClassFile cfile;

    // What kind of constant are we?
    byte tag;

    // Primary constant table index.
    private short index;

    // Secondary constant tabel index.
    private short index2;

    // value for primitive type constants:
    Object value;

    String utf;

    public static Constant read(ClassFile cf, DataInputStream din) throws IOException {
	return new Constant(cf, din);
    }

    private Constant(ClassFile cf, DataInputStream din) throws IOException {
	cfile = cf;
	tag = din.readByte();
	switch (tag) {
	  case CONSTANT_Utf8:
	    utf = din.readUTF();
	    break;
	  case CONSTANT_Integer:
	    value = new Integer(din.readInt());
	    break;
	  case CONSTANT_Float:
	    value = new Float(din.readFloat());
	    break;
	  case CONSTANT_Long:
	    value = new Long(din.readLong());
	    break;
	  case CONSTANT_Double:
	    value = new Double(din.readDouble());
	    break;
	  case CONSTANT_Class:
	    // Read index to java class name constant.
	    index = din.readShort();
	    break;
	  case CONSTANT_String:
	    // Read index to utf8 constant.
	    index = din.readShort();
	    break;
	  case CONSTANT_FieldRef:
	  case CONSTANT_MethodRef:
	  case CONSTANT_InterfaceMethodRef:
	    // Read index to class constant.
	    index = din.readShort();
	    // Read index to name-and-type constant.
	    index2 = din.readShort();
	    break;
	  case CONSTANT_NameAndType:
	    // Read index to utf8 constant for name
	    index = din.readShort();
	    // Read index to utf8 constant for field descriptor.
	    index2 = din.readShort();
	    break;
	  default:
	    throw new IOException("Bad classfile constant tag " + tag);
	}
    }

    public String getClassName() {
	if (tag == CONSTANT_Class) {
	    Constant c = cfile.constants[index];
	    if (c.tag != CONSTANT_Utf8) {
 	        throw new Error("CLASS Constant doesn't point at UTF8: " + toIndex());
	    }
	    return c.utf;
	}
	throw new Error("getClassName on non-class constant " + toIndex());
    }

    public String getRefClass() {
	switch (tag) {
	  case CONSTANT_FieldRef:
	  case CONSTANT_MethodRef:
	  case CONSTANT_InterfaceMethodRef:
	    // Extract the class via index.
	    Constant c = cfile.constants[index];
	    return c.getClassName();
	  default:
	    throw new Error("getRefClass on non-ref constant " + toIndex());
	}
    }

    public String getRefName() {
	switch (tag) {
	  case CONSTANT_FieldRef:
	  case CONSTANT_MethodRef:
	  case CONSTANT_InterfaceMethodRef:
	    // Extract the NameAndType via index2.
	    Constant c = cfile.constants[index2];
	    if (c.tag != CONSTANT_NameAndType) {
		throw new Error("Ref doesn't point to NameAndType " + toIndex());
	    }
	   // And extract the name via the NameAndType's index
	    c = cfile.constants[c.index];
	    if (c.tag != CONSTANT_Utf8) {
		throw new Error("Ref doesn't point to NameAndType name " + toIndex());
	    }
	    return c.utf;
	  default:
	    throw new Error("getRefName on non-ref constant " + toIndex());
	}
    }

    public String getRefType() {
	switch (tag) {
	  case CONSTANT_FieldRef:
	  case CONSTANT_MethodRef:
	  case CONSTANT_InterfaceMethodRef:
	    // Extract the NameAndType via index2.
	    Constant c = cfile.constants[index2];
	    if (c.tag != CONSTANT_NameAndType) {
		throw new Error("Ref doesn't point to NameAndType " + toIndex());
	    }
	   // And extract the type via the NameAndType's index2
	    c = cfile.constants[c.index2];
	    if (c.tag != CONSTANT_Utf8) {
		throw new Error("Ref doesn't point to NameAndType type " + toIndex());
	    }
	    return c.utf;
	  default:
	    throw new Error("getRefType on non-ref constant " + toIndex());
	}
    }

    /**
     * Return a value constant.
     */
    public Object getValue() {
	switch (tag) {
	  case CONSTANT_Integer:
	  case CONSTANT_Float:
	  case CONSTANT_Long:
	  case CONSTANT_Double:
	    return value;
	  case CONSTANT_String:
	    Constant c = cfile.constants[index];
	    if (c.tag != CONSTANT_Utf8) {
 	        throw new Error("String Constant doesn't point at UTF8 " + toIndex());
	    }
	    return c.utf;
	  default:
	    throw new Error("getValue on non-value constant " + toIndex());
	}
    }

    public int getSize() {
	switch (tag) {
	  case CONSTANT_Utf8:
	    return (3 + utf.length());   // Approximate!
	  case CONSTANT_Integer:
	    return (5);
	  case CONSTANT_Float:
	    return (5);
	  case CONSTANT_Long:
	    return (9);
	  case CONSTANT_Double:
	    return (9);
	  case CONSTANT_Class:
	    return (5);
	  case CONSTANT_String:
	    return (3);
	  case CONSTANT_FieldRef:
	  case CONSTANT_MethodRef:
	  case CONSTANT_InterfaceMethodRef:
	    return (5);
	  case CONSTANT_NameAndType:
	    return (5);
	  default:
	    throw new Error("Bad tag");
	}
    }
 
    /**
     * Return a hashcode value that is independent of our classfile.
     * I.e. it can be used for comparing Constants from different classfiles.
     */
    public int hashCode() {
	switch (tag) {
	  case CONSTANT_Utf8:
	    return utf.hashCode();
	  case CONSTANT_Integer:
	  case CONSTANT_Float:
	  case CONSTANT_Long:
	  case CONSTANT_Double:
	  case CONSTANT_Class:
	  case CONSTANT_String:
	  case CONSTANT_FieldRef:
	  case CONSTANT_MethodRef:
	  case CONSTANT_InterfaceMethodRef:
	  case CONSTANT_NameAndType:
	    // Hash on the printable string value
	    return toString().hashCode();
	  default:
	    throw new Error("Bad tag");
	}
    }

    /**
     * Do an equality comparison is independent of our classfile.
     * I.e. it can be used for comparing Constants from different classfiles.
     */
    public boolean equals(Object obj) {
	Constant b = (Constant) obj;
	switch (tag) {
	  case CONSTANT_Utf8:
	    return (this.utf.equals(b.utf));
	  case CONSTANT_Integer:
	  case CONSTANT_Float:
	  case CONSTANT_Long:
	  case CONSTANT_Double:
	  case CONSTANT_Class:
	  case CONSTANT_String:
	  case CONSTANT_FieldRef:
	  case CONSTANT_MethodRef:
	  case CONSTANT_InterfaceMethodRef:
	  case CONSTANT_NameAndType:
	    // Compare the printable string values
	    return (this.toString().equals(b.toString()));
	  default:
	    throw new Error("Bad tag");
	}
    }

    public String toString() {
	switch (tag) {
	  case CONSTANT_Utf8:
	    return (utf);
	  case CONSTANT_Integer:
	    return ("INTEGER: " + value);
	  case CONSTANT_Float:
	    return ("FLOAT: " + value);
	  case CONSTANT_Long:
	    return ("LONG: " + value);
	  case CONSTANT_Double:
	    return ("DOUBLE: " + value);
	  case CONSTANT_Class:
	    return ("CLASS: " + cfile.getString(index));
	  case CONSTANT_String:
	    return ("STRING: " + cfile.getString(index));
	  case CONSTANT_FieldRef:
	    return ("FIELD: " + cfile.getString(index) + " " + cfile.getString(index2));
	  case CONSTANT_MethodRef:
	    return ("METHOD: " + cfile.getString(index) + " " + cfile.getString(index2));
	  case CONSTANT_InterfaceMethodRef:
	    return ("INTERFACEMETHOD: " + cfile.getString(index) + " " + cfile.getString(index2));
	  case CONSTANT_NameAndType:
	    return ("N&T: " + cfile.getString(index) + " " + cfile.getString(index2));
	  default:
	    throw new Error("bad tag!!???");
	}
    }

    public boolean takesTwoSlots() {
	if (tag == CONSTANT_Long || tag == CONSTANT_Double) {
	    return true;
        } else {
	    return false;
	}
    }

    public String rawString() {
	switch (tag) {
	  case CONSTANT_Utf8:
	    return ("UTF8: " + utf);
	  case CONSTANT_Integer:
	    return ("INTEGER: " + value);
	  case CONSTANT_Float:
	    return ("FLOAT: " + value);
	  case CONSTANT_Long:
	    return ("LONG: " + value);
	  case CONSTANT_Double:
	    return ("DOUBLE: " + value);
	  case CONSTANT_Class:
	    return ("CLASS: " + index);
	  case CONSTANT_String:
	    return ("STRING: " + index);
	  case CONSTANT_FieldRef:
	    return ("FIELDREF: " + index + " " + index2);
	  case CONSTANT_MethodRef:
	    return ("METHODREF: " + index + " " + index2);
	  case CONSTANT_InterfaceMethodRef:
	    return ("INTERFACEMETHODREF: " + index + " " + index2);
	  case CONSTANT_NameAndType:
	    return ("NAMEANDTYPE: " + index + " " + index2);
	  default:
	    throw new Error("bad tag!!???");
	}
    }

    /**
     * For debugging only.  Map constant back to index into
     * classfile constant pool.
     */
    public int toIndex() {
	for (int i = 0; i < cfile.constants.length; i++) {
	    if (cfile.constants[i] == this) {
		return i;
	    }
        }
	return -1;
    }

}


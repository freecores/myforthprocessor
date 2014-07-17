/*
 * @(#)ClassFile.java	1.4 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.tools.crunch;

/**
 * Overall description of a classfile.
 *
 * @author Graham Hamilton
 */

import java.io.*;
import java.util.*;

public class ClassFile {

    private static boolean printConstants = false;

    private static int totalClassFiles;
    private static int totalConstants;
    private static int totalConstantsLength;
    private static int totalInterfaces;

    Constant constants[];

    private String name;
    private int magic;
    private short majorVersion;
    private short minorVersion;
    private short flags;
    private short thisIndex;
    private short superIndex;
    private short interfaces[];

    private Hashtable privates = new Hashtable();
    private int nextPrivateIndex;

    private Field fields[];
    private Method methods[];
    private Attribute attributes[];

    public static ClassFile read(String fname, InputStream in) throws IOException {
	return new ClassFile(fname, in);
    }

    private ClassFile(String fname, InputStream in) throws IOException {
	this.name = fname;
	OffsetInputStream oin = new OffsetInputStream(in);
	DataInputStream din = new DataInputStream(oin);

    	totalClassFiles++;
	
	magic = din.readInt();
	if (magic != 0xCAFEBABE) {
	    throw new IOException("Bad classfile magic 0x" 
				+ Integer.toString(magic,16));
	}
	minorVersion = din.readShort();
	majorVersion = din.readShort();

	// from the updated ClassFile.java from Crunch in Merlin
	if (minorVersion < 0 || minorVersion > 20) {
	    throw new IOException(name + ": Unexpected classfile minor version " + minorVersion);
	}
	if (majorVersion < 45 || majorVersion > 50) {
	    throw new IOException(name + ": Unexpected classfile major version " + majorVersion);
	}

	int constantCount = din.readShort();
	totalConstants += constantCount;

	int off = oin.getOffset();

	// Note: constants start at index 1 not zero.
	constants = new Constant[constantCount];
	for (int i = 1; i < constantCount; i++) {
	    constants[i] = Constant.read(this, din);
	    // Somes constants (doubles and longs) occupy 2 table slots.
	    if (constants[i].takesTwoSlots()) {
		i++;
	    }
	}

	if (printConstants) {
	    for (int i = 0; i < constantCount; i++) {
	  	if (constants[i] == null) {
		    continue;
		}
	        System.out.println("" + i + ": " + constants[i].rawString());
	    }
	}

	int constantsLength = oin.getOffset() - off;
	totalConstantsLength += constantsLength;

	flags = din.readShort();
	thisIndex = din.readShort();
	superIndex = din.readShort();

	int interfaceCount = din.readShort();
	if (interfaceCount > 255) {
	   throw new Error("Too many interfaces in " + this);
	}
	totalInterfaces += interfaceCount;
	interfaces = new short[interfaceCount];
	for (int i = 0; i < interfaceCount; i++) {
	    interfaces[i] = din.readShort();
	}

	int fieldCount = din.readShort();
	// We allow lots of fields, so no size check here.
	fields = new Field[fieldCount];
	for (int i = 0; i < fieldCount; i++) {
	    fields[i] = Field.read(this, din);
	}

	int methodCount = din.readShort();
	if (methodCount > 255) {
	   throw new Error("Too many methods in " + this);
	}
	methods = new Method[methodCount];
	for (int i = 0; i < methodCount; i++) {
	    methods[i] = Method.readMethod(this, din);
	}

	attributes = Attribute.readAttributes(this, din);
    }

    /**
     * Map a class constant reference in our constant pool to
     * a global class reference.
     */
    public short mapClass(short fix) {	
	if (fix == 0) {
	    return 0;
	}
        fix = GlobalTables.findClass(constants[fix].getClassName());
	return (fix);
    }

    public short mapUtf(short fix) {
	if (fix == 0) {
	    return 0;
	}
	Constant con = constants[fix];
	if (con.tag != Constant.CONSTANT_Utf8) {
	    throw new Error("constant isn't UTF");
	}
        fix = GlobalTables.findUtf(con.utf);
	return (fix);
    }

    public short mapType(short fix) {
	Constant con = constants[fix];
	if (con.tag != Constant.CONSTANT_Utf8) {
	    throw new Error("constant isn't UTF");
	}
        fix = GlobalTables.findType(con.utf);
	return (fix);
    }

    public short mapRef(short fix) {
	Constant con = constants[fix];
        fix = GlobalTables.findRef(con.getRefClass(), con.getRefName(), con.getRefType());
	return fix;
    }


    public void write(DataOutputStream out) throws IOException {
	out.writeShort(flags);

	// Map our class and superclass.
	out.writeShort(mapClass(thisIndex));
	out.writeShort(mapClass(superIndex));

	// Map each implemented interface
	out.writeByte(interfaces.length);
	for (int i = 0; i < interfaces.length; i++) {
	    out.writeShort(mapClass(interfaces[i]));
	}

	// Write the fields.
	out.writeShort(fields.length);
	for (int i = 0; i < fields.length; i++) {
	    fields[i].write(out);
	}

	// Write the methods.
	out.writeByte(methods.length);
	for (int i = 0; i < methods.length; i++) {
	    methods[i].write(out);
	}

	// Write the class attributes.
	out.writeByte(attributes.length);
	for (int i = 0; i < attributes.length; i++) {
	    attributes[i].write(out);
	}
    }

    /**
     * get the value of a string or UTF constant.
     */
    public String getString(short ix) {
	if (ix < 0 || ix > constants.length) {
	    throw new Error("bad constant index " + ix);
	}
	return constants[ix].toString();
    }

    /**
     * Get the name of this classfile.
     */
    public String getName() {
	return name;
    }

    /**
     * Get the name of "this" type,
     */
    public String getClassName() {
	return constants[thisIndex].getClassName();
    }

    public static void summarize() {
	System.out.println("Number of classfiles = " + totalClassFiles);
	System.out.println("    Total number of constants = " + totalConstants);
	System.out.println("    Total length of constants = " + totalConstantsLength + " bytes");
	System.out.println("    Total number of interface refs = " + totalInterfaces);
	Method.summarize();
	Field.summarize();
	Attribute.summarize();
    }

    public String toString() {
	return name;
    }
}

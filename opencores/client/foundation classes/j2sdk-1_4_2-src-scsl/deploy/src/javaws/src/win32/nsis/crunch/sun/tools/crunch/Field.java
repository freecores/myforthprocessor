/*
 * @(#)Field.java	1.3 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.tools.crunch;

/**
 * Describe a classfile field block.
 *
 * @author Graham Hamilton
 */

import java.io.*;

public class Field {
    protected ClassFile cfile;
    short flags;
    protected short nameIndex;
    protected short typeIndex;
    protected Attribute attributes[];

    private static int totalCount;
    private static int totalPrivates;
    private static int totalAttributes;
    private static int writtenLength;

    public static Field read(ClassFile cf, DataInputStream din) throws IOException {
	return new Field(cf, din);
    }

    protected Field() {
    }

    private Field(ClassFile cf, DataInputStream din) throws IOException {
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

    public void write(DataOutputStream out) throws IOException {
	String cname = cfile.getClassName();
	String name = cfile.constants[nameIndex].utf;
	String type = cfile.constants[typeIndex].utf;

	CodeAttribute smallCodeAttribute = null;

	if ((flags & 0xF000) != 0) {
	    throw new Error("Unexpected flag bits: " + cfile.getName() + 
			"." + name + "  0x" + Integer.toString(flags,16));
        }

	// Are we going to write any attributes?
	int alen = 0;

	// Count attributes.
	// We handle small code attributes specially.
	for (int i = 0; i < attributes.length; i++) {
	    if (attributes[i] instanceof CodeAttribute) {
		CodeAttribute ca = (CodeAttribute)attributes[i];
		if (ca.isSmall()) {
		    smallCodeAttribute = ca;
		    flags = (short)(flags | 0x2000);
		    continue;
		}
	    }
	    alen++;
	}
	if (alen > 0) {
	    flags = (short)(flags | 0x8000);
	}

	// Decide whether to write separate name and type 
	// indexes or just a ref index.
	if ((flags & 1) == 0) {
	    // Use references for all non-public fields.  It's very
	    // likely a reference will occur anyway.
	    flags = (short)(flags | 0x4000);
	} else if (GlobalTables.existsRef(cname, name, type)) {
	    // Also use a reference if an appropriate one already exists.
	    flags = (short)(flags | 0x4000);
	}
	out.writeShort(flags);
	writtenLength += 2;

	if ((flags & 0x4000) != 0) {
            // Write a fieldref index.
	    out.writeShort(GlobalTables.findRef(cname, name, type));
	    writtenLength += 2;
	} else {
	    // Write nameIndex and typeIndex.
	    out.writeShort(GlobalTables.findUtf(name));
	    out.writeShort(GlobalTables.findType(type));
	    writtenLength += 4;
	}

	// Write normal attributes.
	if (alen > 0) {
	    out.writeByte(alen);
	    writtenLength += 1;
	    for (int i = 0; i < attributes.length; i++) {
		if (attributes[i] != smallCodeAttribute) {
	            attributes[i].write(out);
		}
	    }
	}
	// Write any small code attribute
	if (smallCodeAttribute != null) {
	    smallCodeAttribute.writeSmall(out);
	}
    }

    public static void summarize() {
	System.out.print("    Total number of field definitions = " + totalCount);
        System.out.println(" (" + totalPrivates + " private)");
	System.out.println("        Total number of field attributes = " + totalAttributes);
	System.out.println("    Written length of field/method defs = " + writtenLength);
    }
}


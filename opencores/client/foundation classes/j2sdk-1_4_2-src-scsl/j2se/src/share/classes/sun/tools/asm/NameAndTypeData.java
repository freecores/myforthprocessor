/*
 * @(#)NameAndTypeData.java	1.16 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.asm;

import sun.tools.java.*;

/**
 * An object to represent a name and type constant pool data item.
 */
final
class NameAndTypeData {
    MemberDefinition field;

    /**
     * Constructor
     */
    NameAndTypeData(MemberDefinition field) {
	this.field = field;
    }

    /**
     * Hashcode
     */
    public int hashCode() {
	return field.getName().hashCode() * field.getType().hashCode();
    }

    /**
     * Equality
     */
    public boolean equals(Object obj) {
	if ((obj != null) && (obj instanceof NameAndTypeData)) {
	    NameAndTypeData nt = (NameAndTypeData)obj;
	    return field.getName().equals(nt.field.getName()) &&
		field.getType().equals(nt.field.getType());
	}
	return false;
    }

    /**
     * Convert to string
     */
    public String toString() {
	return "%%" + field.toString() + "%%";
    }
}

/*
 * @(#)ArrayType.java	1.17 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.java;

/**
 * This class represents an Java array type.
 * It overrides the relevant methods in class Type.
 *
 * @author 	Arthur van Hoff
 * @version 	1.17, 01/23/03
 */
public final
class ArrayType extends Type {
    /**
     * The type of the element.
     */
    Type elemType;

    /**
     * Construct an array type. Use Type.tArray to create
     * a new array type.
     */
    ArrayType(String typeSig, Type elemType) {
	super(TC_ARRAY, typeSig);
	this.elemType = elemType;
    }
    
    public Type getElementType() {
	return elemType;
    }

    public int getArrayDimension() {
	return elemType.getArrayDimension() + 1;
    }
    
    public String typeString(String id, boolean abbrev, boolean ret) {
	return getElementType().typeString(id, abbrev, ret) + "[]";
    }
}

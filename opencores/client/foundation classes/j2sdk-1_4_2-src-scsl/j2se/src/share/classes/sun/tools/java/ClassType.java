/*
 * @(#)ClassType.java	1.18 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.java;

/**
 * This class represents an Java class type.
 * It overrides the relevant methods in class Type.
 *
 * @author 	Arthur van Hoff
 * @version 	1.18, 01/23/03
 */
public final
class ClassType extends Type {
    /**
     * The fully qualified class name.
     */
    Identifier className;

    /**
     * Construct a class type. Use Type.tClass to create
     * a new class type.
     */
    ClassType(String typeSig, Identifier className) {
	super(TC_CLASS, typeSig);
	this.className = className;
    }

    public Identifier getClassName() {
	return className;
    }
    
    public String typeString(String id, boolean abbrev, boolean ret) {
	String s = (abbrev ? getClassName().getFlatName() :
				Identifier.lookup(getClassName().getQualifier(),
								  getClassName().getFlatName())).toString();
	return (id.length() > 0) ? s + " " + id : s;
    }
}

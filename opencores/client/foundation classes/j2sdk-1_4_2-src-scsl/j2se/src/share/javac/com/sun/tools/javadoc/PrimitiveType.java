/**
 * @(#)PrimitiveType.java	1.5 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.tools.javadoc;
import com.sun.javadoc.*;

import com.sun.tools.javac.v8.code.Symbol;

import com.sun.tools.javac.v8.code.Symbol.ClassSymbol;

import com.sun.tools.javac.v8.code.Type;

import com.sun.tools.javac.v8.code.TypeTags;

import com.sun.tools.javac.v8.code.Type.ClassType;


class PrimitiveType implements com.sun.javadoc.Type {
    private final String name;
    static final PrimitiveType voidType = new PrimitiveType("void");
    static final PrimitiveType booleanType = new PrimitiveType("boolean");
    static final PrimitiveType byteType = new PrimitiveType("byte");
    static final PrimitiveType charType = new PrimitiveType("char");
    static final PrimitiveType shortType = new PrimitiveType("short");
    static final PrimitiveType intType = new PrimitiveType("int");
    static final PrimitiveType longType = new PrimitiveType("long");
    static final PrimitiveType floatType = new PrimitiveType("float");
    static final PrimitiveType doubleType = new PrimitiveType("double");
    static final PrimitiveType errorType = new PrimitiveType("");

    PrimitiveType(String name) {
        super();
        this.name = name;
    }

    /**
      * Return unqualified name of type excluding any dimension information.
      * <p>
      * For example, a two dimensional array of String returns 'String'.
      */
    public String typeName() {
        return name;
    }

    /**
      * Return qualified name of type excluding any dimension information.
      * <p>
      * For example, a two dimensional array of String
      * returns 'java.lang.String'.
      */
    public String qualifiedTypeName() {
        return name;
    }

    /**
      * Return the type's dimension information, as a string.
      * <p>
      * For example, a two dimensional array of String returns '[][]'.
      */
    public String dimension() {
        return "";
    }

    /**
      * Return this type as a class.  Array dimensions are ignored.
      *
      * @return a ClassDocImpl if the type is a Class.
      * Return null if it is a primitive type..
      */
    public ClassDoc asClassDoc() {
        return null;
    }

    /**
      * Returns a string representation of the type.
      *
      * Return name of type including any dimension information.
      * <p>
      * For example, a two dimensional array of String returns
      * <code>String[][]</code>.
      *
      * @return name of type including any dimension information.
      */
    public String toString() {
        return qualifiedTypeName();
    }

    /**
      * Return true if this is a primitive Java type. (i.e. if this
      * is not a class type or array type)
      */
    public boolean isPrimitiveJavaType() {
        return true;
    }

    /**
      * Return true if this is an array type
      */
    public boolean isArrayType() {
        return false;
    }
}

/**
 * @(#)TypeMaker.java	1.20 03/01/23
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

import com.sun.tools.javac.v8.code.Type.ArrayType;


public class TypeMaker implements TypeTags {

    public TypeMaker() {
        super();
    }

    public static com.sun.javadoc.Type getType(DocEnv env, Type t) {
        Type v = t;
        switch (v.tag) {
        case ARRAY:
            return new ArrayTypeImpl(env, t);

        case CLASS:
            return env.getClassDoc((ClassType) v);

        case BYTE:
            return PrimitiveType.byteType;

        case CHAR:
            return PrimitiveType.charType;

        case SHORT:
            return PrimitiveType.shortType;

        case INT:
            return PrimitiveType.intType;

        case LONG:
            return PrimitiveType.longType;

        case FLOAT:
            return PrimitiveType.floatType;

        case DOUBLE:
            return PrimitiveType.doubleType;

        case BOOLEAN:
            return PrimitiveType.booleanType;

        case VOID:
            return PrimitiveType.voidType;

        default:
            return new PrimitiveType(t.tsym.fullName().toString());

        }
    }

    public static String getTypeName(Type t, boolean full) {
        if (t.tag == ARRAY) {
            StringBuffer dimension = new StringBuffer();
            while (t.tag == ARRAY) {
                dimension = dimension.append("[]");
                t = t.elemtype();
            }
            return getTypeName(t, full) + dimension;
        } else if (t.tag == CLASS) {
            ClassSymbol c = (ClassSymbol)((ClassType) t).tsym;
            if (full) {
                return c.fullName().toString();
            } else {
                String n = c.name.toString();
                for (c = c.owner.enclClass(); c != null; c = c.owner.enclClass()) {
                    n = c.name.toString() + "." + n;
                }
                if (t.typarams().nonEmpty())
                    return n + "<" + t.typarams().toString() + ">";
                else
                    return n;
            }
        } else {
            return t.tsym.fullName().toString();
        }
    }

    private static class ArrayTypeImpl implements com.sun.javadoc.Type {
        com.sun.tools.javac.v8.code.Type arrayType;
        DocEnv env;

        ArrayTypeImpl(DocEnv env, Type arrayType) {
            super();
            this.env = env;
            this.arrayType = arrayType;
        }

        /**
          * Return the type's dimension information, as a string.
          * <p>
          * For example, a two dimensional array of String returns '[][]'.
          */
        public String dimension() {
            StringBuffer dimension = new StringBuffer();
            for (Type t = this.arrayType; t.tag == ARRAY; t = t.elemtype()) {
                dimension = dimension.append("[]");
            }
            return dimension.toString();
        }

        private Type skipArrays() {
            Type t;
            for (t = this.arrayType; t.tag == ARRAY; t = t.elemtype()) {
            }
            return t;
        }

        /**
          * Return unqualified name of type excluding any dimension information.
          * <p>
          * For example, a two dimensional array of String returns 'String'.
          */
        public String typeName() {
            return TypeMaker.getType(env, skipArrays()).typeName();
        }

        /**
          * Return qualified name of type excluding any dimension information.
          * <p>
          * For example, a two dimensional array of String
          * returns 'java.lang.String'.
          */
        public String qualifiedTypeName() {
            return TypeMaker.getType(env, skipArrays()).qualifiedTypeName();
        }

        /**
          * Return this type as a class.  Array dimensions are ignored.
          *
          * @return a ClassDocImpl if the type is a Class.
          * Return null if it is a primitive type..
          */
        public ClassDoc asClassDoc() {
            return TypeMaker.getType(env, skipArrays()).asClassDoc();
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
            return qualifiedTypeName() + dimension();
        }

        /**
          * Return true if this is a primitive Java type. (i.e. if this
          * is not a class type or array type)
          */
        public boolean isPrimitiveJavaType() {
            return false;
        }

        /**
          * Return true if this is an array type
          */
        public boolean isArrayType() {
            return true;
        }
    }
}

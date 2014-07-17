/**
 * @(#)ClassFile.java	1.21 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.tools.javac.v8.code;
import com.sun.tools.javac.v8.util.*;

import java.io.File;


/**
 * Generic Java classfiles have one additional attribute for classes,
 * methods and fields:
 *
 *   "Signature" (u4 attr-length, u2 signature-index)
 *
 *   A signature gives the full Java type of a method or field. When used as a
 *   class attribute, it indicates type parameters, followed by supertype,
 *   followed by all interfaces.
 *
 *     methodOrFieldSignature ::= type
 *     classSignature         ::= [ typeparams ] supertype { interfacetype }
 *
 *   The type syntax in signatures is extended as follows:
 *
 *     type       ::= ... | classtype | methodtype | typevar
 *     classtype  ::= classsig { '.' classsig }
 *     classig    ::= 'L' name [typeargs] ';'
 *     methodtype ::= [ typeparams ] '(' { type } ')' type
 *     typevar    ::= 'T' name ';'
 *     typeargs   ::= '<' type { type } '>'
 *     typeparams ::= '<' typeparam { typeparam } '>'
 *     typeparam  ::= name ':' type
 *
 *  This class defines constants used in class files as well
 *  as routines to convert between internal ``.'' and external ``/''
 *  separators in class names.
 */
public class ClassFile {

    public ClassFile() {
        super();
    }
    public static final int JAVA_MAGIC = -889275714;
    public static final int CONSTANT_Utf8 = 1;
    public static final int CONSTANT_Unicode = 2;
    public static final int CONSTANT_Integer = 3;
    public static final int CONSTANT_Float = 4;
    public static final int CONSTANT_Long = 5;
    public static final int CONSTANT_Double = 6;
    public static final int CONSTANT_Class = 7;
    public static final int CONSTANT_String = 8;
    public static final int CONSTANT_Fieldref = 9;
    public static final int CONSTANT_Methodref = 10;
    public static final int CONSTANT_InterfaceMethodref = 11;
    public static final int CONSTANT_NameandType = 12;
    public static final int MAX_PARAMETERS = 255;
    public static final int MAX_DIMENSIONS = 255;
    public static final int MAX_CODE = 65535;
    public static final int MAX_LOCALS = 65535;
    public static final int MAX_STACK = 65535;

    /**
     * Return internal representation of buf[offset..offset+len-1],
     *  converting '/' to '.'.
     */
    public static byte[] internalize(byte[] buf, int offset, int len) {
        byte[] translated = new byte[len];
        for (int j = 0; j < len; j++) {
            byte b = buf[offset + j];
            if (b == '/')
                translated[j] = (byte)'.';
            else
                translated[j] = b;
        }
        return translated;
    }

    /**
      * Return internal representation of given name,
      *  converting '/' to '.'.
      */
    public static byte[] internalize(Name name) {
        return internalize(name.table.names, name.index, name.len);
    }

    /**
      * Return external representation of buf[offset..offset+len-1],
      *  converting '.' to '/'.
      */
    public static byte[] externalize(byte[] buf, int offset, int len) {
        byte[] translated = new byte[len];
        for (int j = 0; j < len; j++) {
            byte b = buf[offset + j];
            if (b == '.')
                translated[j] = (byte)'/';
            else
                translated[j] = b;
        }
        return translated;
    }

    /**
      * Return external representation of given name,
      *  converting '/' to '.'.
      */
    public static byte[] externalize(Name name) {
        return externalize(name.table.names, name.index, name.len);
    }

    /**
      * Return external representation of file name s,
      *  converting '.' to File.separatorChar.
      */
    public static String externalizeFileName(Name n) {
        return n.toString().replace('.', File.separatorChar);
    }

    /**
      * A class for the name-and-type signature of a method or field.
      */
    public static class NameAndType {
        Name name;
        Type type;

        NameAndType(Name name, Type type) {
            super();
            this.name = name;
            this.type = type;
        }

        public boolean equals(Object other) {
            return other instanceof NameAndType &&
                    name == ((NameAndType) other).name &&
                    type.equals(((NameAndType) other).type);
        }

        public int hashCode() {
            return name.hashCode() * type.hashCode();
        }
    }
}

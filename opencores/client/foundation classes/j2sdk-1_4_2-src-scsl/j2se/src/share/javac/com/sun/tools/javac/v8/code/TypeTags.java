/**
 * @(#)TypeTags.java	1.12 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.tools.javac.v8.code;

/**
 * An interface for type tag values, which distinguish between different
 *  sorts of types.
 */
public interface TypeTags {

    /**
     * The tag of the basic type `byte'.
     */
    int BYTE = 1;

    /**
     * The tag of the basic type `char'.
     */
    int CHAR = 2;

    /**
     * The tag of the basic type `short'.
     */
    int SHORT = 3;

    /**
     * The tag of the basic type `int'.
     */
    int INT = 4;

    /**
     * The tag of the basic type `long'.
     */
    int LONG = 5;

    /**
     * The tag of the basic type `float'.
     */
    int FLOAT = 6;

    /**
     * The tag of the basic type `double'.
     */
    int DOUBLE = 7;

    /**
     * The tag of the basic type `boolean'.
     */
    int BOOLEAN = 8;

    /**
     * The tag of the type `void'.
     */
    int VOID = 9;

    /**
     * The tag of all class and interface types.
     */
    int CLASS = 10;

    /**
     * The tag of all array types.
     */
    int ARRAY = 11;

    /**
     * The tag of all (monomorphic) method types.
     */
    int METHOD = 12;

    /**
     * The tag of all package "types".
     */
    int PACKAGE = 13;

    /**
     * The tag of the bottom type <null>.
     */
    int BOT = 16;

    /**
     * The tag of a missing type.
     */
    int NONE = 17;

    /**
     * The tag of the error type.
     */
    int ERROR = 18;

    /**
     * The tag of an unknown type
     */
    int UNKNOWN = 19;

    /**
     * The tag of all instantiatable type variables.
     */
    int UNDETVAR = 20;

    /**
     * The number of type tags.
     */
    int TypeTagCount = 21;

    /**
     * The maximum tag of a basic type.
     */
    int lastBaseTag = BOOLEAN;

    /**
     * The minimum tag of a partial type
     */
    int firstPartialTag = ERROR;
}

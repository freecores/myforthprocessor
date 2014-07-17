/**
 * @(#)Kinds.java	1.10 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.tools.javac.v8.code;

/**
 * Internal symbol kinds, which distinguish between elements of
 *  different subclasses of Symbol. Symbol kinds are organized so they can be
 *  or'ed to sets.
 */
public interface Kinds {

    /**
     * The empty set of kinds.
     */
    int NIL = 0;

    /**
     * The kind of package symbols.
     */
    int PCK = 1;

    /**
     * The kind of type symbols (classes, interfaces and type variables).
     */
    int TYP = 2;

    /**
     * The kind of variable symbols.
     */
    int VAR = 4;

    /**
     * The kind of values (variables or non-variable expressions), includes VAR.
     */
    int VAL = 8 | VAR;

    /**
     * The kind of methods.
     */
    int MTH = 16;

    /**
     * The error kind, which includes all other kinds.
     */
    int ERR = 31;

    /**
     * The set of all kinds.
     */
    int AllKinds = ERR;
}

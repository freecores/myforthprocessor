/**
 * @(#)Flags.java	1.21 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.tools.javac.v8.code;

/**
 * Access flags and other modifiers for Java classes and members.
 */
public interface Flags {
    int PUBLIC = 1 << 0;
    int PRIVATE = 1 << 1;
    int PROTECTED = 1 << 2;
    int STATIC = 1 << 3;
    int FINAL = 1 << 4;
    int SYNCHRONIZED = 1 << 5;
    int VOLATILE = 1 << 6;
    int TRANSIENT = 1 << 7;
    int NATIVE = 1 << 8;
    int INTERFACE = 1 << 9;
    int ABSTRACT = 1 << 10;
    int STRICTFP = 1 << 11;
    int StandardFlags = 4095;
    int ACC_SUPER = SYNCHRONIZED;

    /**
     * Flag is set if symbol has a synthetic attribute.
     */
    int SYNTHETIC = 1 << 16;

    /**
     * Flag is set if symbol is deprecated.
     */
    int DEPRECATED = 1 << 17;

    /**
     * Flag is set for a variable symbol if the variable's definition
     *  has an initializer part.
     */
    int HASINIT = 1 << 18;

    /**
     * Flag is set for compiler-generated anonymous method symbols
     *  that `own' an initializer block.
     */
    int BLOCK = 1 << 20;

    /**
     * Flag is set for compiler-generated abstract methods that implement
     *  an interface method (Miranda methods).
     */
    int IPROXY = 1 << 21;

    /**
     * Flag is set for nested classes that do not access instance members or `this'
     *  of an outer class and therefore don't need to be passed a this$n reference.
     *  This flag is currently set only for anonymous classes in superclass
     *  constructor calls and only for pre 1.4 targets.
     *  todo: use this flag for optimizing away this$n parameters in other cases.
     */
    int NOOUTERTHIS = 1 << 22;

    /**
     * Flag is set for package symbols if a package has a member or
     *  directory and therefore exists.
     */
    int EXISTS = 1 << 23;

    /**
     * Flag is set for compiler-generated compound classes
     *  representing multiple variable bounds
     */
    int COMPOUND = 1 << 24;

    /**
     * Flag is set for class symbols if a class file was found for this class.
     */
    int CLASS_SEEN = 1 << 25;

    /**
     * Flag is set for class symbols if a source file was found for this class.
     */
    int SOURCE_SEEN = 1 << 26;

    /**
     * Flag for class symbols is set and later re-set as a lock in Enter
     *  to detect cycles in the superclass/superinterface relations.
     *  Similarly for constructor methods in Attr.
     */
    int LOCKED = 1 << 27;

    /**
     * Flag for class symbols is set and later re-set to indicate that a class
     *  has been entered but has not yet been attributed.
     */
    int UNATTRIBUTED = 1 << 28;

    /**
     * Flag for synthesized default constructors of anonymous classes.
     */
    int ANONCONSTR = 1 << 29;

    /**
     * Flag for class symbols and constructors to indicate it has been checked
     *  and found acyclic.
     */
    int ACYCLIC = 1 << 30;

    /**
     * Flag that marks formal parameters.
     */
    long PARAMETER = 1L << 33;

    /**
     * Modifier masks.
     */
    long AccessFlags = PUBLIC | PROTECTED | PRIVATE;

    /**
     * Modifier masks.
     */
    long LocalClassFlags = FINAL | ABSTRACT | STRICTFP;

    /**
     * Modifier masks.
     */
    long MemberClassFlags = LocalClassFlags | INTERFACE | AccessFlags;

    /**
     * Modifier masks.
     */
    long ClassFlags = LocalClassFlags | INTERFACE | PUBLIC;

    /**
     * Modifier masks.
     */
    long LocalVarFlags = FINAL | PARAMETER;

    /**
     * Modifier masks.
     */
    long InterfaceVarFlags = FINAL | STATIC | PUBLIC;

    /**
     * Modifier masks.
     */
    long VarFlags = AccessFlags | FINAL | STATIC | VOLATILE | TRANSIENT;

    /**
     * Modifier masks.
     */
    long ConstructorFlags = AccessFlags;

    /**
     * Modifier masks.
     */
    long InterfaceMethodFlags = ABSTRACT | PUBLIC;

    /**
     * Modifier masks.
     */
    long MethodFlags =
            AccessFlags | ABSTRACT | STATIC | NATIVE | SYNCHRONIZED | FINAL |
            STRICTFP;
}

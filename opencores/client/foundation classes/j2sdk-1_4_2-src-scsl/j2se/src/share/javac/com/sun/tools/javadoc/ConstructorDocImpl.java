/**
 * @(#)ConstructorDocImpl.java	1.14 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.tools.javadoc;
import com.sun.javadoc.*;

import com.sun.tools.javac.v8.code.Type.MethodType;

import com.sun.tools.javac.v8.code.Symbol.ClassSymbol;

import com.sun.tools.javac.v8.code.Symbol.MethodSymbol;

import com.sun.tools.javac.v8.util.List;

import com.sun.tools.javac.v8.util.Name;


/**
 * Represents a constructor of a java class.
 *
 * @since JDK1.2
 * @author Robert Field
 * @author Neal Gafter (rewrite)
 */
public class ConstructorDocImpl extends ExecutableMemberDocImpl implements ConstructorDoc {

    /**
     * constructor.
     */
    public ConstructorDocImpl(DocEnv env, MethodSymbol sym) {
        super(env, sym);
    }

    /**
      * constructor.
      */
    public ConstructorDocImpl(DocEnv env, MethodSymbol sym, String docComment,
            List argNames) {
        super(env, sym, docComment, argNames);
    }

    /**
      * Return true if it is a constructor, which it is.
      *
      * @return true
      */
    public boolean isConstructor() {
        return true;
    }

    /**
      * Get the name.
      *
      * @return the name of the member qualified by class (but not package)
      */
    public String name() {
        ClassSymbol c = sym.enclClass();
        String n = c.name.toString();
        for (c = c.owner.enclClass(); c != null; c = c.owner.enclClass()) {
            n = c.name.toString() + "." + n;
        }
        return n;
    }

    /**
      * Get the name.
      *
      * @return the qualified name of the member.
      */
    public String qualifiedName() {
        return sym.enclClass().fullName().toString();
    }
}

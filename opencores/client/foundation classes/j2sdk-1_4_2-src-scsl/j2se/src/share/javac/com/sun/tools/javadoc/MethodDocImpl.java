/**
 * @(#)MethodDocImpl.java	1.24 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.tools.javadoc;
import com.sun.javadoc.*;

import com.sun.tools.javac.v8.code.Flags;

import com.sun.tools.javac.v8.code.Scope;

import com.sun.tools.javac.v8.code.Type;

import com.sun.tools.javac.v8.code.Type.MethodType;

import com.sun.tools.javac.v8.code.TypeTags;

import com.sun.tools.javac.v8.code.Symbol.TypeSymbol;

import com.sun.tools.javac.v8.code.Symbol.ClassSymbol;

import com.sun.tools.javac.v8.code.Symbol.MethodSymbol;

import com.sun.tools.javac.v8.util.List;

import com.sun.tools.javac.v8.util.Name;

import java.lang.reflect.Modifier;


/**
 * Represents a method of a java class.
 *
 * @since JDK1.2
 * @author Robert Field
 * @author Neal Gafter (rewrite)
 */
public class MethodDocImpl extends ExecutableMemberDocImpl implements MethodDoc {

    /**
     * constructor.
     */
    public MethodDocImpl(DocEnv env, MethodSymbol sym) {
        super(env, sym);
    }

    /**
      * constructor.
      */
    public MethodDocImpl(DocEnv env, MethodSymbol sym, String docComment,
            List argNames) {
        super(env, sym, docComment, argNames);
    }

    /**
      *
      * Return true if it is a method, which it is.
      * Note: constructors are not methods.
      *
      * @return tree
      */
    public boolean isMethod() {
        return true;
    }

    /**
      *
      * Return true if this method is abstract
      */
    public boolean isAbstract() {
        if (containingClass().isInterface()) {
            return false;
        }
        return Modifier.isAbstract(getModifiers());
    }

    /**
      * Get return type.
      *
      * @return the return type of this method, null if it
      * is a constructor.
      */
    public com.sun.javadoc.Type returnType() {
                                          return TypeMaker.getType(env,
                                                  sym.type.restype());
                                      }

                                      /**
                                        *
                                        * Return the class that originally defined the method that
                                        * is overridden by the current definition, or null if no
                                        * such class exists.
                                        *
                                        * @return a ClassDocImpl representing the superclass that
                                        * originally defined this method, null if this method does
                                        * not override a definition in a superclass.
                                        */
                                      public ClassDoc overriddenClass() {
                                          MethodDoc meth = overriddenMethod();
                                          return (meth == null) ? null :
                                                  meth.containingClass();
                                      }

                                      /**
                                        * Return the method that this method overrides.
                                        *
                                        * @return a MethodDoc representing a method definition
                                        * in a superclass this method overrides, null if
                                        * this method does not override.
                                        */
                                      public MethodDoc overriddenMethod() {
                                      if ((sym.flags() & Flags.STATIC) != 0) {
                                              return null;
                                          }
                                          ClassSymbol origin =
                                                  (ClassSymbol) sym.owner;
                                      for (Type t = origin.type.supertype();
                                              t.tag == TypeTags.CLASS;
                                              t = t.supertype()) {
                                              ClassSymbol c = (ClassSymbol) t.tsym;
                                          for (Scope.Entry e =
                                                  c.members().lookup(sym.name);
                                                  e.scope != null; e = e.next()) {
                                              if (sym.overrides(e.sym, origin)) {
                                                      return env.getMethodDoc(
                                                              (MethodSymbol) e.sym);
                                                  }
                                              }
                                          }
                                          return null;
                                      }

                                      private String genericParameters() {
                                          return (sym.type.typarams().isEmpty())
                                                  ? "" : "<" +
                                                  sym.type.typarams() + ">";
                                      }

                                      public String name() {
                                          return genericParameters() +
                                                  sym.name.toString();
                                      }

                                      public String qualifiedName() {
                                          return genericParameters() +
                                                  sym.enclClass().fullName().
                                                  toString() + '.' +
                                                  sym.name.toString();
                                      }
                                  }

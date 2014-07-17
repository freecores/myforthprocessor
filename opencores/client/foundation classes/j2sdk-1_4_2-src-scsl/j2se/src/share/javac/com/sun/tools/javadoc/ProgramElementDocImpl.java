/**
 * @(#)ProgramElementDocImpl.java	1.16 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.tools.javadoc;
import com.sun.javadoc.*;

import com.sun.tools.javac.v8.code.Flags;

import com.sun.tools.javac.v8.code.Symbol.ClassSymbol;

import java.lang.reflect.Modifier;

import java.text.CollationKey;


/**
 * Represents a java program element: class, interface, field,
 * constructor, or method.
 * This is an abstract class dealing with information common to
 * these elements.
 *
 * @see MemberDocImpl
 * @see ClassDocImpl
 *
 * @author Robert Field
 * @author Neal Gafter (rewrite)
 */
public abstract class ProgramElementDocImpl extends DocImpl implements ProgramElementDoc {

    protected ProgramElementDocImpl(DocEnv env, String doc) {
        super(env, doc);
    }

    /**
      * Subclasses override to identify the containing class
      */
    protected abstract ClassSymbol getContainingClass();

    /**
     * Returns the flags in terms of javac's flags
     */
    protected abstract long getFlags();

    /**
     * Returns the modifier flags in terms of java.lang.reflect.Modifier.
     */
    protected int getModifiers() {
        return DocEnv.translateModifiers(getFlags());
    }

    /**
      * Get the containing class of this program element.
      *
      * @return a ClassDocImpl for this element's containing class.
      * If this is a class with no outer class, return null.
      */
    public ClassDoc containingClass() {
        if (getContainingClass() == null) {
            return null;
        }
        return env.getClassDoc(getContainingClass());
    }

    /**
      * Return the package that this member is contained in.
      * Return "" if in empty package.
      */
    public PackageDoc containingPackage() {
        return env.getPackageDoc(getContainingClass().packge());
    }

    /**
      * Get the modifier specifier integer.
      *
      * @see java.lang.reflect.Modifier
      */
    public int modifierSpecifier() {
        int modifiers = getModifiers();
        if (isMethod() && containingClass().isInterface())
            return modifiers & ~Modifier.ABSTRACT;
        return modifiers;
    }

    /**
      * Get modifiers string.
      * <pre>
      * Example, for:
      *   public abstract int foo() { ... }
      * modifiers() would return:
      *   'public abstract'
      * </pre>
      */
    public String modifiers() {
        int modifiers = getModifiers();
        if (isMethod() && containingClass().isInterface()) {
            return Modifier.toString(modifiers & ~Modifier.ABSTRACT);
        } else {
            return Modifier.toString(modifiers);
        }
    }

    /**
      * Return true if this program element is public
      */
    public boolean isPublic() {
        int modifiers = getModifiers();
        return Modifier.isPublic(modifiers);
    }

    /**
      * Return true if this program element is protected
      */
    public boolean isProtected() {
        int modifiers = getModifiers();
        return Modifier.isProtected(modifiers);
    }

    /**
      * Return true if this program element is private
      */
    public boolean isPrivate() {
        int modifiers = getModifiers();
        return Modifier.isPrivate(modifiers);
    }

    /**
      * Return true if this program element is package private
      */
    public boolean isPackagePrivate() {
        return !(isPublic() || isPrivate() || isProtected());
    }

    /**
      * Return true if this program element is static
      */
    public boolean isStatic() {
        int modifiers = getModifiers();
        return Modifier.isStatic(modifiers);
    }

    /**
      * Return true if this program element is final
      */
    public boolean isFinal() {
        int modifiers = getModifiers();
        return Modifier.isFinal(modifiers);
    }

    /**
      * Generate a key for sorting.
      */
    CollationKey generateKey() {
        String k = name();
        return env.doclocale.collator.getCollationKey(k);
    }
}

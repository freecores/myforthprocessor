/**
 * @(#)ExecutableMemberDocImpl.java	1.29 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.tools.javadoc;
import com.sun.javadoc.*;

import com.sun.tools.javac.v8.util.List;

import com.sun.tools.javac.v8.util.ListBuffer;

import com.sun.tools.javac.v8.util.Name;

import com.sun.tools.javac.v8.code.Flags;

import com.sun.tools.javac.v8.code.Type;

import com.sun.tools.javac.v8.code.Type.MethodType;

import com.sun.tools.javac.v8.code.Symbol;

import com.sun.tools.javac.v8.code.Symbol.ClassSymbol;

import com.sun.tools.javac.v8.code.Symbol.MethodSymbol;

import com.sun.tools.javac.v8.tree.Tree.MethodDef;

import java.text.CollationKey;

import java.lang.reflect.Modifier;


/**
 * Represents a method or constructor of a java class.
 *
 * @since JDK1.2
 * @author Robert Field
 * @author Neal Gafter (rewrite)
 */
public abstract class ExecutableMemberDocImpl extends MemberDocImpl implements ExecutableMemberDoc {
    protected final MethodSymbol sym;
    private List argNames;
    MethodDef tree = null;

    /**
     * Constructor.
     */
    public ExecutableMemberDocImpl(DocEnv env, MethodSymbol sym, String rawDocs,
            List argNames) {
        super(env, rawDocs);
        this.sym = sym;
        this.argNames = argNames;
    }

    /**
      * Constructor.
      */
    public ExecutableMemberDocImpl(DocEnv env, MethodSymbol sym) {
        this(env, sym, null, new List());
    }

    /**
      * Returns the flags in terms of javac's flags
      */
    protected long getFlags() {
        return sym.flags();
    }

    /**
      * Identify the containing class
      */
    protected ClassSymbol getContainingClass() {
        return sym.enclClass();
    }

    /**
      *
      * Return true if this method is native
      */
    public boolean isNative() {
        return Modifier.isNative(getModifiers());
    }

    /**
      *
      * Return true if this method is synchronized
      */
    public boolean isSynchronized() {
        return Modifier.isSynchronized(getModifiers());
    }

    /**
      * Set the arg names
      */
    void setArgNames(List argNames) {
        this.argNames = argNames;
    }

    /**
      *
      * Returns true if this field was synthesized by the compiler.
      */
    public boolean isSynthetic() {
        return ((sym.flags() & Flags.SYNTHETIC) != 0);
    }

    /**
      * Return the throws tags in this method.
      *
      * @return an array of ThrowTagImpl containing all <code>&#64exception</code>
      * and <code>&#64throws</code> tags.
      */
    public ThrowsTag[] throwsTags() {
        return comment().throwsTags();
    }

    /**
      * Return the param tags in this method.
      *
      * @return an array of ParamTagImpl containing all <code>&#64param</code> tags.
      */
    public ParamTag[] paramTags() {
        return comment().paramTags();
    }

    /**
      * Return exceptions this method or constructor throws.
      *
      * @return an array of ClassDoc[] representing the exceptions
      * thrown by this method.
      */
    public ClassDoc[] thrownExceptions() {
        ListBuffer l = new ListBuffer();
        for (List thrown = sym.type.thrown(); thrown.nonEmpty();
                thrown = thrown.tail) {
            ClassDocImpl cdi =
                    env.getClassDoc((ClassSymbol)((Type) thrown.head).tsym);
            if (cdi != null)
                l.append(cdi);
        }
        return (ClassDoc[]) l.toArray(new ClassDocImpl[l.length()]);
    }

    /**
      * Get argument information.
      *
      * @see ParameterImpl
      *
      * @return an array of ParameterImpl, one element per argument
      * in the order the arguments are present.
      */
    public Parameter[] parameters() {
        ListBuffer paramList = new ListBuffer();
        List names = argNames;
        if (names.nonEmpty()) {
            for (List types = sym.type.argtypes(); types.nonEmpty();
                    types = types.tail) {
                String name = ((Name) names.head).toString();
                paramList.append(new ParameterImpl(env, (Type) types.head, name));
                names = names.tail;
            }
        } else {
            for (List types = sym.type.argtypes(); types.nonEmpty();
                    types = types.tail) {
                paramList.append(new ParameterImpl(env, (Type) types.head, ""));
            }
        }
        return (Parameter[]) paramList.toArray(new Parameter[paramList.length()]);
    }

    /**
      * Get the signature. It is the parameter list, type is qualified.
      * For instance, for a method <code>mymethod(String x, int y)</code>,
      * it will return <code>(java.lang.String,int)</code>.
      */
    public String signature() {
        return makeSignature(true);
    }

    private String makeSignature(boolean full) {
        StringBuffer result = new StringBuffer();
        result.append("(");
        for (List types = sym.type.argtypes(); types.nonEmpty();) {
            Type t = (Type) types.head;
            result.append(TypeMaker.getTypeName(t, full));
            types = types.tail;
            if (types.nonEmpty()) {
                result.append(", ");
            }
        }
        result.append(")");
        return result.toString();
    }

    /**
      * Get flat signature.  All types are not qualified.
      * Return a String, which is the flat signiture of this member.
      * It is the parameter list, type is not qualified.
      * For instance, for a method <code>mymethod(String x, int y)</code>,
      * it will return <code>(String, int)</code>.
      */
    public String flatSignature() {
        return makeSignature(false);
    }

    public boolean isIncluded() {
        return containingClass().isIncluded() && env.shouldDocument(sym);
    }

    /**
      * Returns a string representation of the member.
      *
      * @return  a fully-qualified signature
      */
    public String toString() {
        return qualifiedName() + signature();
    }

    /**
      * Generate a key for sorting.
      */
    CollationKey generateKey() {
        String signature = flatSignature();
        if (signature.indexOf(",") > 0) {
            signature = signature.replace(',', ' ');
        }
        String k = name() + signature;
        return env.doclocale.collator.getCollationKey(k);
    }

    /**
      * Return the qualified name appended with the signature.
      */
    public String toQualifiedString() {
        return qualifiedName() + flatSignature();
    }

    /**
      * Return the source position of the entity, or null if
      * no position is available.
      */
    public SourcePosition position() {
        if (sym.enclClass().sourcefile == null)
            return null;
        return SourcePositionImpl.make(sym.enclClass().sourcefile.toString(),
                (tree == null) ? 0 : tree.pos);
    }
}

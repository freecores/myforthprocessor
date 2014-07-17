/**
 * @(#)MemberDocImpl.java	1.38 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.tools.javadoc;
import com.sun.javadoc.*;

import com.sun.tools.javac.v8.code.Symbol.ClassSymbol;


/**
 * Represents a member of a java class: field, constructor, or method.
 * This is an abstract class dealing with information common to
 * method, constructor and field members. Class members of a class
 * (nested classes) are represented instead by ClassDocImpl.
 *
 * @see MethodDocImpl
 * @see FieldDocImpl
 * @see ClassDocImpl
 *
 * @author Robert Field
 * @author Neal Gafter
 */
public abstract class MemberDocImpl extends ProgramElementDocImpl implements MemberDoc {

    /**
     * constructor.
     */
    public MemberDocImpl(DocEnv env, String doc) {
        super(env, doc);
    }

    /**
      *
      * Returns true if this field was synthesized by the compiler.
      */
    public abstract boolean isSynthetic();

    String toQualifiedString() {
        return qualifiedName();
    }
}

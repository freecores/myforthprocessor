/**
 * @(#)AttrContext.java	1.12 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.tools.javac.v8.comp;
import com.sun.tools.javac.v8.util.*;

import com.sun.tools.javac.v8.code.*;

import com.sun.tools.javac.v8.tree.*;

import com.sun.tools.javac.v8.code.Symbol.*;


/**
 * Contains information specific to the attribute and enter
 *  passes, to be used in place of the generic field in environments.
 */
public class AttrContext {

    public AttrContext() {
        super();
    }

    /**
      * The scope of local symbols.
      */
    Scope scope = null;

    /**
     * The number of enclosing `static' modifiers.
     */
    int staticLevel = 0;

    /**
     * Is this an environment for a this(...) or super(...) call?
     */
    boolean isSelfCall = false;

    /**
     * Are we evaluating the selector of a `super' or type name?
     */
    boolean selectSuper = false;

    /**
     * Do arguments to current function applications have raw types?
     */
    boolean rawArgs = false;

    /**
     * A list of type variables that are all-quantifed in current context.
     */
    List tvars = Type.emptyList;

    /**
     * Duplicate this context, replacing scope field and copying all others.
     */
    AttrContext dup(Scope scope) {
        AttrContext info = new AttrContext();
        info.scope = scope;
        info.staticLevel = staticLevel;
        info.isSelfCall = isSelfCall;
        info.selectSuper = selectSuper;
        info.rawArgs = rawArgs;
        info.tvars = tvars;
        return info;
    }

    /**
      * Duplicate this context, copying all fields.
      */
    AttrContext dup() {
        return dup(scope);
    }
}

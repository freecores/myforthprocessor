/**
 * @(#)Infer.java	1.24 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.tools.javac.v8.comp;
import com.sun.tools.javac.v8.util.*;

import com.sun.tools.javac.v8.code.*;

import com.sun.tools.javac.v8.tree.*;

import com.sun.tools.javac.v8.tree.Tree.*;

import com.sun.tools.javac.v8.code.Type.*;

import com.sun.tools.javac.v8.code.Symbol.*;


/**
 * Helper class for type parameter inference, used by the attribution phase.
 */
public class Infer implements Kinds, Flags, TypeTags {
    private static final Context.Key inferKey = new Context.Key();

    /**
     * A value for prototypes that admit any type, including polymorphic ones.
     */
    public static final Type anyPoly = new Type(NONE, null);
    private Symtab syms;

    public static Infer instance(Context context) {
        Infer instance = (Infer) context.get(inferKey);
        if (instance == null)
            instance = new Infer(context);
        return instance;
    }

    private Infer(Context context) {
        super();
        context.put(inferKey, this);
        syms = Symtab.instance(context);
    }

    public static class NoInstanceException extends RuntimeException {
        boolean isAmbiguous;

        NoInstanceException(boolean isAmbiguous) {
            super();
            this.isAmbiguous = isAmbiguous;
        }
    }
    private static final NoInstanceException ambiguousNoInstanceException =
            new NoInstanceException(true);
    private static final NoInstanceException unambiguousNoInstanceException =
            new NoInstanceException(false);
}

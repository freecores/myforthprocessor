/**
 * @(#)AttrContextEnv.java	1.5 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.tools.javac.v8.comp;
import com.sun.tools.javac.v8.tree.Tree;


/**
 * Env<A> specialized as Env<AttrContext>
 */
public class AttrContextEnv extends Env {

    /**
     * Create an outermost environment for a given (toplevel)tree,
     *  with a given info field.
     */
    public AttrContextEnv(Tree tree, AttrContext info) {
        super(tree, info);
    }

    /*synthetic*/ public Env dup(Tree x0, Object x1) {
        return super.dup(x0, (AttrContext) x1);
    }
}

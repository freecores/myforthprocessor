/**
 * @(#)JavadocEnter.java	1.7 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.tools.javadoc;
import com.sun.tools.javac.v8.util.Context;

import com.sun.tools.javac.v8.util.List;

import com.sun.tools.javac.v8.util.ListBuffer;

import com.sun.tools.javac.v8.util.Name;

import com.sun.tools.javac.v8.code.Kinds;

import com.sun.tools.javac.v8.code.Symbol;

import com.sun.tools.javac.v8.code.Symbol.*;

import com.sun.tools.javac.v8.tree.Tree;

import com.sun.tools.javac.v8.tree.Tree.*;


/**
 *  Javadoc's own enter phase does a few things above and beyond that
 *  done by javac.
 *  @author Neal Gafter
 */
public class JavadocEnter extends com.sun.tools.javac.v8.comp.Enter {
    private static final Context.Key javadocEnterKey = new Context.Key();

    public static JavadocEnter instance0(Context context) {
        JavadocEnter instance = (JavadocEnter) context.get(javadocEnterKey);
        if (instance == null)
            instance = new JavadocEnter(context);
        return instance;
    }
    private final Messager messager;
    private final DocEnv docenv;

    public void main(List trees) {
        int nerrors = messager.nerrors;
        super.main(trees);
        messager.nwarnings += (messager.nerrors - nerrors);
        messager.nerrors = nerrors;
    }

    public void visitClassDef(ClassDef tree) {
        super.visitClassDef(tree);
        if (tree.sym != null && tree.sym.kind == Kinds.TYP) {
            if (tree.sym == null)
                return;
            String comment = (String) env.toplevel.docComments.get(tree);
            ClassSymbol c = (ClassSymbol) tree.sym;
            docenv.makeClassDoc(c, comment, tree);
        }
    }

    /**
      * Don't complain about a duplicate class.
      */
    protected void duplicateClass(int pos, ClassSymbol c) {
    }

    class JavadocMemberEnter extends MemberEnter {

        JavadocMemberEnter() {
            super();
        }

        public void visitMethodDef(MethodDef tree) {
            super.visitMethodDef(tree);
            if (tree.sym == null || tree.sym.kind != Kinds.MTH)
                return;
            String docComment = (String) env.toplevel.docComments.get(tree);
            ListBuffer argNames = new ListBuffer();
            for (List l = tree.params; l.nonEmpty(); l = l.tail) {
                argNames.append(((VarDef) l.head).name);
            }
            if (tree.sym.isConstructor())
                docenv.makeConstructorDoc((MethodSymbol) tree.sym, docComment,
                        argNames.toList(), tree);
            else
                docenv.makeMethodDoc((MethodSymbol) tree.sym, docComment,
                        argNames.toList(), tree);
        }

        public void visitVarDef(VarDef tree) {
            super.visitVarDef(tree);
            if (tree.sym == null || tree.sym.kind != Kinds.VAR)
                return;
            String docComment = (String) env.toplevel.docComments.get(tree);
            docenv.makeFieldDoc((VarSymbol) tree.sym, docComment, tree);
        }
    }

    private JavadocEnter(Context context) {
        super(context);
        context.put(javadocEnterKey, this);
        messager = Messager.instance0(context);
        super.phase2 = new JavadocMemberEnter();
        docenv = DocEnv.instance(context);
    }
}

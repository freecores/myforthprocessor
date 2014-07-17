/**
 * @(#)TransTypes.java	1.39 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.tools.javac.v8.comp;
import com.sun.tools.javac.v8.util.*;

import com.sun.tools.javac.v8.code.*;

import com.sun.tools.javac.v8.tree.*;

import com.sun.tools.javac.v8.code.Symbol.*;

import com.sun.tools.javac.v8.tree.Tree.*;

import com.sun.tools.javac.v8.code.Type.*;


/**
 * This pass translates Generic Java to conventional Java.
 */
public class TransTypes extends TreeTranslator implements Flags, Kinds, TypeTags {

    /**
     * The context key for the TransTypes phase.
     */
    private static final Context.Key transTypesKey = new Context.Key();

    /**
     * Get the instance for this context.
     */
    public static TransTypes instance(Context context) {
        TransTypes instance = (TransTypes) context.get(transTypesKey);
        if (instance == null)
            instance = new TransTypes(context);
        return instance;
    }
    private Name.Table names;
    private Log log;
    private Symtab syms;
    private TreeMaker make;

    private TransTypes(Context context) {
        super();
        context.put(transTypesKey, this);
        names = Name.Table.instance(context);
        log = Log.instance(context);
        syms = Symtab.instance(context);
    }

    /**
      * A hashtable mapping bridge methods to the methods they override after
      *  type erasure.
      */
    Hashtable overridden;

    /**
     * Construct an attributed tree for a cast of expression to target type,
     *  unless it already has precisely that type.
     *  @param tree    The expression tree.
     *  @param target  The target type.
     */
    Tree cast(Tree tree, Type target) {
        int oldpos = make.pos;
        make.at(tree.pos);
        if (!tree.type.isSameType(target)) {
            tree = make.TypeCast(make.Type(target), tree).setType(target);
        }
        make.pos = oldpos;
        return tree;
    }

    /**
      * Construct an attributed tree to coerce an expression to some erased
      *  target type, unless the expression is already assignable to that type.
      *  If target type is a constant type, use its base type instead.
      *  @param tree    The expression tree.
      *  @param target  The target type.
      */
    Tree coerce(Tree tree, Type target) {
        Type btarget = target.baseType();
        return tree.type.isAssignable(btarget) ? tree : cast(tree, btarget);
    }

    /**
      * Given an erased reference type, assume this type as the tree's type.
      *  Then, coerce to some given target type unless target type is null.
      *  This operation is used in situations like the following:
      *
      *  class Cell<A> { A value; }
      *  ...
      *  Cell<Integer> cell;
      *  Integer x = cell.value;
      *
      *  Since the erasure of Cell.value is Object, but the type
      *  of cell.value in the assignment is Integer, we need to
      *  adjust the original type of cell.value to Object, and insert
      *  a cast to Integer. That is, the last assignment becomes:
      *
      *  Integer x = (Integer)cell.value;
      *
      *  @param tree       The expression tree whose type might need adjustment.
      *  @param erasedType The expression's type after erasure.
      *  @param target     The target type, which is usually the erasure of the
      *                    expression's original type.
      */
    Tree retype(Tree tree, Type erasedType, Type target) {
        if (erasedType.tag > lastBaseTag) {
            tree.type = erasedType;
            if (target != null)
                return coerce(tree, target);
        }
        return tree;
    }

    /**
      * Translate method argument list, casting each argument
      *  to its corresponding type in a list of target types.
      *  @param trees     The method argument list.
      *  @param targets   The list of target types.
      */
    List translateArgs(List trees, List targets) {
        for (List l = trees; l.nonEmpty(); l = l.tail) {
            l.head = translate((Tree) l.head, (Type) targets.head);
            targets = targets.tail;
        }
        return trees;
    }

    /**
      * Visitor argument: proto-type.
      */
    private Type pt;

    /**
     * Visitor method: perform a type translation on tree.
     */
    public Tree translate(Tree tree, Type pt) {
        Type prevPt = this.pt;
        try {
            this.pt = pt;
            if (tree == null)
                result = null;
            else
                tree.accept(this);
        }
        finally { this.pt = prevPt;
                } return result;
    }

    /**
      * Visitor method: perform a type translation on list of trees.
      */
    public List translate(List trees, Type pt) {
        Type prevPt = this.pt;
        List res;
        try {
            this.pt = pt;
            res = translate(trees);
        }
        finally { this.pt = prevPt;
                } return res;
    }

    public void visitClassDef(ClassDef tree) {
        tree.typarams = TypeParameter.emptyList;
        super.visitClassDef(tree);
        make.at(tree.pos);
        tree.type = tree.type.erasure();
        result = tree;
    }

    public void visitMethodDef(MethodDef tree) {
        tree.restype = translate(tree.restype, null);
        tree.typarams = TypeParameter.emptyList;
        tree.params = translateVarDefs(tree.params);
        tree.thrown = translate(tree.thrown, null);
        tree.body = (Block) translate(tree.body, tree.sym.erasure().restype());
        tree.type = tree.type.erasure();
        result = tree;
        for (Scope.Entry e = tree.sym.owner.members().lookup(tree.name);
                e.sym != null; e = e.next()) {
            if (e.sym != tree.sym && e.sym.type.erasure().isSameType(tree.type)) {
                log.error(tree.pos, "name.clash.same.erasure", tree.sym.toJava(),
                        e.sym.toJava());
                return;
            }
        }
    }

    public void visitVarDef(VarDef tree) {
        tree.vartype = translate(tree.vartype, null);
        tree.init = translate(tree.init, tree.sym.erasure());
        tree.type = tree.type.erasure();
        result = tree;
    }

    public void visitDoLoop(DoLoop tree) {
        tree.body = translate(tree.body);
        tree.cond = translate(tree.cond, null);
        result = tree;
    }

    public void visitWhileLoop(WhileLoop tree) {
        tree.cond = translate(tree.cond, null);
        tree.body = translate(tree.body);
        result = tree;
    }

    public void visitForLoop(ForLoop tree) {
        tree.init = translate(tree.init, null);
        tree.cond = translate(tree.cond, null);
        tree.step = translate(tree.step, null);
        tree.body = translate(tree.body);
        result = tree;
    }

    public void visitSwitch(Switch tree) {
        tree.selector = translate(tree.selector, null);
        tree.cases = translateCases(tree.cases);
        result = tree;
    }

    public void visitCase(Case tree) {
        tree.pat = translate(tree.pat, null);
        tree.stats = translate(tree.stats);
        result = tree;
    }

    public void visitSynchronized(Synchronized tree) {
        tree.lock = translate(tree.lock, null);
        tree.body = translate(tree.body);
        result = tree;
    }

    public void visitConditional(Conditional tree) {
        tree.cond = translate(tree.cond, null);
        tree.truepart = translate(tree.truepart, tree.type.erasure());
        tree.falsepart = translate(tree.falsepart, tree.type.erasure());
        result = tree;
    }

    public void visitIf(If tree) {
        tree.cond = translate(tree.cond, null);
        tree.thenpart = translate(tree.thenpart);
        tree.elsepart = translate(tree.elsepart);
        result = tree;
    }

    public void visitExec(Exec tree) {
        tree.expr = translate(tree.expr, null);
        result = tree;
    }

    public void visitReturn(Return tree) {
        tree.expr = translate(tree.expr);
        result = tree;
    }

    public void visitThrow(Throw tree) {
        tree.expr = translate(tree.expr, tree.expr.type.erasure());
        result = tree;
    }

    public void visitAssert(Assert tree) {
        tree.cond = translate(tree.cond, null);
        if (tree.detail != null)
            tree.detail = translate(tree.detail, null);
        result = tree;
    }

    public void visitApply(Apply tree) {
        tree.meth = translate(tree.meth, null);
        Type mt = TreeInfo.symbol(tree.meth).erasure();
        tree.args = translateArgs(tree.args, mt.argtypes());
        result = retype(tree, mt.restype(), pt);
    }

    public void visitNewClass(NewClass tree) {
        if (tree.encl != null)
            tree.encl = translate(tree.encl, tree.encl.type.erasure());
        tree.clazz = translate(tree.clazz, null);
        tree.args = translateArgs(tree.args, tree.constructor.erasure().argtypes());
        tree.def = (ClassDef) translate(tree.def, null);
        tree.type = tree.type.erasure();
        result = tree;
    }

    public void visitNewArray(NewArray tree) {
        tree.elemtype = translate(tree.elemtype, null);
        tree.dims = translate(tree.dims, null);
        tree.elems = translate(tree.elems, tree.type.elemtype().erasure());
        tree.type = tree.type.erasure();
        result = tree;
    }

    public void visitParens(Parens tree) {
        tree.expr = translate(tree.expr, null);
        tree.type = tree.type.erasure();
        result = tree;
    }

    public void visitAssign(Assign tree) {
        tree.lhs = translate(tree.lhs, null);
        tree.rhs = translate(tree.rhs, tree.lhs.type.erasure());
        tree.type = tree.type.erasure();
        result = tree;
    }

    public void visitAssignop(Assignop tree) {
        tree.lhs = translate(tree.lhs, null);
        tree.rhs = translate(tree.rhs, null);
        tree.type = tree.type.erasure();
        result = tree;
    }

    public void visitUnary(Unary tree) {
        tree.arg = translate(tree.arg, (Type) tree.operator.type.argtypes().head);
        result = tree;
    }

    public void visitBinary(Binary tree) {
        tree.lhs = translate(tree.lhs, (Type) tree.operator.type.argtypes().head);
        tree.rhs =
                translate(tree.rhs, (Type) tree.operator.type.argtypes().tail.head);
        result = tree;
    }

    public void visitTypeCast(TypeCast tree) {
        tree.clazz = translate(tree.clazz, null);
        tree.expr = translate(tree.expr, null);
        tree.type = tree.type.erasure();
        result = tree;
    }

    public void visitTypeTest(TypeTest tree) {
        tree.expr = translate(tree.expr, null);
        tree.clazz = translate(tree.clazz, null);
        result = tree;
    }

    public void visitIndexed(Indexed tree) {
        tree.indexed = translate(tree.indexed, tree.indexed.type.erasure());
        tree.index = translate(tree.index, null);
        result = retype(tree, tree.indexed.type.elemtype(), pt);
    }

    public void visitIdent(Ident tree) {
        Type et = tree.sym.erasure();
        if (tree.type.constValue != null) {
            result = tree;
        } else if (tree.sym.kind == VAR) {
            result = retype(tree, et, pt);
        } else {
            tree.type = tree.type.erasure();
            result = tree;
        }
    }

    public void visitSelect(Select tree) {
        Type t = tree.selected.type;
        tree.selected = translate(tree.selected, t.erasure());
        if (tree.type.constValue != null) {
            result = tree;
        } else if (tree.sym.kind == VAR) {
            result = retype(tree, tree.sym.erasure(), pt);
        } else {
            tree.type = tree.type.erasure();
            result = tree;
        }
    }

    public void visitTypeArray(TypeArray tree) {
        tree.elemtype = translate(tree.elemtype, null);
        tree.type = tree.type.erasure();
        result = tree;
    }

    /**
      * Translate a toplevel class definition.
      *  @param cdef    The definition to be translated.
      */
    public Tree translateTopLevelClass(Tree cdef, TreeMaker make) {
        try {
            this.make = make;
            overridden = Hashtable.make();
            pt = null;
            return translate(cdef, null);
        }
        finally { this.make = null;
                  overridden = null;
                  pt = null;
                } }
}

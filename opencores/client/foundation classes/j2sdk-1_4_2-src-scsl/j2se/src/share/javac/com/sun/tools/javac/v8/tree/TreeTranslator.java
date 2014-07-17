/**
 * @(#)TreeTranslator.java	1.21 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.tools.javac.v8.tree;
import com.sun.tools.javac.v8.util.*;

import com.sun.tools.javac.v8.tree.Tree.*;


/**
 * A subclass of Tree.Visitor, this class defines
 *  a general tree translator pattern. Translation proceeds recursively in
 *  left-to-right order down a tree, constructing translated nodes by
 *  overwriting existing ones. There is one visitor method in this class
 *  for every possible kind of tree node.  To obtain a specific
 *  translator, it suffices to override those visitor methods which
 *  do some interesting work. The translator class itself takes care of all
 *  navigational aspects.
 */
public class TreeTranslator extends Tree.Visitor {

    public TreeTranslator() {
        super();
    }

    /**
      * Visitor result field: a tree
      */
    protected Tree result;

    /**
     * Visitor method: Translate a single node.
     */
    public Tree translate(Tree tree) {
        if (tree == null) {
            return null;
        } else {
            tree.accept(this);
            Tree result = this.result;
            this.result = null;
            return result;
        }
    }

    /**
      * Visitor method: translate a list of nodes.
      */
    public List translate(List trees) {
        if (trees == null)
            return null;
        for (List l = trees; l.nonEmpty(); l = l.tail)
            l.head = translate((Tree) l.head);
        return trees;
    }

    /**
      * Visitor method: translate a list of variable definitions.
      */
    public List translateVarDefs(List trees) {
        for (List l = trees; l.nonEmpty(); l = l.tail)
            l.head = (VarDef) translate((Tree) l.head);
        return trees;
    }

    /**
      * Visitor method: translate a list of case parts of switch statements.
      */
    public List translateCases(List trees) {
        for (List l = trees; l.nonEmpty(); l = l.tail)
            l.head = (Case) translate((Tree) l.head);
        return trees;
    }

    /**
      * Visitor method: translate a list of catch clauses in try statements.
      */
    public List translateCatchers(List trees) {
        for (List l = trees; l.nonEmpty(); l = l.tail)
            l.head = (Catch) translate((Tree) l.head);
        return trees;
    }

    public void visitTopLevel(TopLevel tree) {
        tree.pid = translate(tree.pid);
        tree.defs = translate(tree.defs);
        result = tree;
    }

    public void visitImport(Import tree) {
        tree.qualid = translate(tree.qualid);
        result = tree;
    }

    public void visitClassDef(ClassDef tree) {
        tree.extending = translate(tree.extending);
        tree.implementing = translate(tree.implementing);
        tree.defs = translate(tree.defs);
        result = tree;
    }

    public void visitMethodDef(MethodDef tree) {
        tree.restype = translate(tree.restype);
        tree.params = translateVarDefs(tree.params);
        tree.thrown = translate(tree.thrown);
        tree.body = (Block) translate(tree.body);
        result = tree;
    }

    public void visitVarDef(VarDef tree) {
        tree.vartype = translate(tree.vartype);
        tree.init = translate(tree.init);
        result = tree;
    }

    public void visitSkip(Skip tree) {
        result = tree;
    }

    public void visitBlock(Block tree) {
        tree.stats = translate(tree.stats);
        result = tree;
    }

    public void visitDoLoop(DoLoop tree) {
        tree.body = translate(tree.body);
        tree.cond = translate(tree.cond);
        result = tree;
    }

    public void visitWhileLoop(WhileLoop tree) {
        tree.cond = translate(tree.cond);
        tree.body = translate(tree.body);
        result = tree;
    }

    public void visitForLoop(ForLoop tree) {
        tree.init = translate(tree.init);
        tree.cond = translate(tree.cond);
        tree.step = translate(tree.step);
        tree.body = translate(tree.body);
        result = tree;
    }

    public void visitLabelled(Labelled tree) {
        tree.body = translate(tree.body);
        result = tree;
    }

    public void visitSwitch(Switch tree) {
        tree.selector = translate(tree.selector);
        tree.cases = translateCases(tree.cases);
        result = tree;
    }

    public void visitCase(Case tree) {
        tree.pat = translate(tree.pat);
        tree.stats = translate(tree.stats);
        result = tree;
    }

    public void visitSynchronized(Synchronized tree) {
        tree.lock = translate(tree.lock);
        tree.body = translate(tree.body);
        result = tree;
    }

    public void visitTry(Try tree) {
        tree.body = translate(tree.body);
        tree.catchers = translateCatchers(tree.catchers);
        tree.finalizer = translate(tree.finalizer);
        result = tree;
    }

    public void visitCatch(Catch tree) {
        tree.param = (VarDef) translate(tree.param);
        tree.body = translate(tree.body);
        result = tree;
    }

    public void visitConditional(Conditional tree) {
        tree.cond = translate(tree.cond);
        tree.truepart = translate(tree.truepart);
        tree.falsepart = translate(tree.falsepart);
        result = tree;
    }

    public void visitIf(If tree) {
        tree.cond = translate(tree.cond);
        tree.thenpart = translate(tree.thenpart);
        tree.elsepart = translate(tree.elsepart);
        result = tree;
    }

    public void visitExec(Exec tree) {
        tree.expr = translate(tree.expr);
        result = tree;
    }

    public void visitBreak(Break tree) {
        result = tree;
    }

    public void visitContinue(Continue tree) {
        result = tree;
    }

    public void visitReturn(Return tree) {
        tree.expr = translate(tree.expr);
        result = tree;
    }

    public void visitThrow(Throw tree) {
        tree.expr = translate(tree.expr);
        result = tree;
    }

    public void visitAssert(Assert tree) {
        tree.cond = translate(tree.cond);
        tree.detail = translate(tree.detail);
        result = tree;
    }

    public void visitApply(Apply tree) {
        tree.meth = translate(tree.meth);
        tree.args = translate(tree.args);
        result = tree;
    }

    public void visitNewClass(NewClass tree) {
        tree.encl = translate(tree.encl);
        tree.clazz = translate(tree.clazz);
        tree.args = translate(tree.args);
        tree.def = (ClassDef) translate(tree.def);
        result = tree;
    }

    public void visitNewArray(NewArray tree) {
        tree.elemtype = translate(tree.elemtype);
        tree.dims = translate(tree.dims);
        tree.elems = translate(tree.elems);
        result = tree;
    }

    public void visitParens(Parens tree) {
        tree.expr = translate(tree.expr);
        result = tree;
    }

    public void visitAssign(Assign tree) {
        tree.lhs = translate(tree.lhs);
        tree.rhs = translate(tree.rhs);
        result = tree;
    }

    public void visitAssignop(Assignop tree) {
        tree.lhs = translate(tree.lhs);
        tree.rhs = translate(tree.rhs);
        result = tree;
    }

    public void visitUnary(Unary tree) {
        tree.arg = translate(tree.arg);
        result = tree;
    }

    public void visitBinary(Binary tree) {
        tree.lhs = translate(tree.lhs);
        tree.rhs = translate(tree.rhs);
        result = tree;
    }

    public void visitTypeCast(TypeCast tree) {
        tree.clazz = translate(tree.clazz);
        tree.expr = translate(tree.expr);
        result = tree;
    }

    public void visitTypeTest(TypeTest tree) {
        tree.expr = translate(tree.expr);
        tree.clazz = translate(tree.clazz);
        result = tree;
    }

    public void visitIndexed(Indexed tree) {
        tree.indexed = translate(tree.indexed);
        tree.index = translate(tree.index);
        result = tree;
    }

    public void visitSelect(Select tree) {
        tree.selected = translate(tree.selected);
        result = tree;
    }

    public void visitIdent(Ident tree) {
        result = tree;
    }

    public void visitLiteral(Literal tree) {
        result = tree;
    }

    public void visitTypeIdent(TypeIdent tree) {
        result = tree;
    }

    public void visitTypeArray(TypeArray tree) {
        tree.elemtype = translate(tree.elemtype);
        result = tree;
    }

    public void visitErroneous(Erroneous tree) {
        result = tree;
    }

    public void visitTree(Tree tree) {
        assert false;
    }
}

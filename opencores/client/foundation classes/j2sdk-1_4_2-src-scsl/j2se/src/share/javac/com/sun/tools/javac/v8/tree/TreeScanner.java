/**
 * @(#)TreeScanner.java	1.9 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.tools.javac.v8.tree;
import com.sun.tools.javac.v8.util.*;

import com.sun.tools.javac.v8.tree.Tree.*;


/**
 * A subclass of Tree.Visitor, this class defines
 *  a general tree scanner pattern. Translation proceeds recursively in
 *  left-to-right order down a tree. There is one visitor method in this class
 *  for every possible kind of tree node.  To obtain a specific
 *  scanner, it suffices to override those visitor methods which
 *  do some interesting work. The scanner class itself takes care of all
 *  navigational aspects.
 */
public class TreeScanner extends Visitor {

    public TreeScanner() {
        super();
    }

    /**
      * Visitor method: Scan a single node.
      */
    public void scan(Tree tree) {
        if (tree != null)
            tree.accept(this);
    }

    /**
      * Visitor method: scan a list of nodes.
      */
    public void scan(List trees) {
        if (trees != null)
            for (List l = trees; l.nonEmpty(); l = l.tail)
                scan((Tree) l.head);
    }

    /**
      * Visitor method: scan a list of variable definitions.
      */
    public void scanVarDefs(List trees) {
        for (List l = trees; l.nonEmpty(); l = l.tail)
            scan((Tree) l.head);
    }

    /**
      * Visitor method: scan a list of case parts of switch statements.
      */
    public void scanCases(List trees) {
        for (List l = trees; l.nonEmpty(); l = l.tail)
            scan((Tree) l.head);
    }

    /**
      * Visitor method: scan a list of catch clauses in try statements.
      */
    public void scanCatchers(List trees) {
        for (List l = trees; l.nonEmpty(); l = l.tail)
            scan((Tree) l.head);
    }

    public void visitTopLevel(TopLevel tree) {
        scan(tree.pid);
        scan(tree.defs);
    }

    public void visitImport(Import tree) {
        scan(tree.qualid);
    }

    public void visitClassDef(ClassDef tree) {
        scan(tree.extending);
        scan(tree.implementing);
        scan(tree.defs);
    }

    public void visitMethodDef(MethodDef tree) {
        scan(tree.restype);
        scanVarDefs(tree.params);
        scan(tree.thrown);
        scan(tree.body);
    }

    public void visitVarDef(VarDef tree) {
        scan(tree.vartype);
        scan(tree.init);
    }

    public void visitSkip(Skip tree) {
    }

    public void visitBlock(Block tree) {
        scan(tree.stats);
    }

    public void visitDoLoop(DoLoop tree) {
        scan(tree.body);
        scan(tree.cond);
    }

    public void visitWhileLoop(WhileLoop tree) {
        scan(tree.cond);
        scan(tree.body);
    }

    public void visitForLoop(ForLoop tree) {
        scan(tree.init);
        scan(tree.cond);
        scan(tree.step);
        scan(tree.body);
    }

    public void visitLabelled(Labelled tree) {
        scan(tree.body);
    }

    public void visitSwitch(Switch tree) {
        scan(tree.selector);
        scanCases(tree.cases);
    }

    public void visitCase(Case tree) {
        scan(tree.pat);
        scan(tree.stats);
    }

    public void visitSynchronized(Synchronized tree) {
        scan(tree.lock);
        scan(tree.body);
    }

    public void visitTry(Try tree) {
        scan(tree.body);
        scanCatchers(tree.catchers);
        scan(tree.finalizer);
    }

    public void visitCatch(Catch tree) {
        scan(tree.param);
        scan(tree.body);
    }

    public void visitConditional(Conditional tree) {
        scan(tree.cond);
        scan(tree.truepart);
        scan(tree.falsepart);
    }

    public void visitIf(If tree) {
        scan(tree.cond);
        scan(tree.thenpart);
        scan(tree.elsepart);
    }

    public void visitExec(Exec tree) {
        scan(tree.expr);
    }

    public void visitBreak(Break tree) {
    }

    public void visitContinue(Continue tree) {
    }

    public void visitReturn(Return tree) {
        scan(tree.expr);
    }

    public void visitThrow(Throw tree) {
        scan(tree.expr);
    }

    public void visitAssert(Assert tree) {
        scan(tree.cond);
        scan(tree.detail);
    }

    public void visitApply(Apply tree) {
        scan(tree.meth);
        scan(tree.args);
    }

    public void visitNewClass(NewClass tree) {
        scan(tree.encl);
        scan(tree.clazz);
        scan(tree.args);
        scan(tree.def);
    }

    public void visitNewArray(NewArray tree) {
        scan(tree.elemtype);
        scan(tree.dims);
        scan(tree.elems);
    }

    public void visitParens(Parens tree) {
        scan(tree.expr);
    }

    public void visitAssign(Assign tree) {
        scan(tree.lhs);
        scan(tree.rhs);
    }

    public void visitAssignop(Assignop tree) {
        scan(tree.lhs);
        scan(tree.rhs);
    }

    public void visitUnary(Unary tree) {
        scan(tree.arg);
    }

    public void visitBinary(Binary tree) {
        scan(tree.lhs);
        scan(tree.rhs);
    }

    public void visitTypeCast(TypeCast tree) {
        scan(tree.clazz);
        scan(tree.expr);
    }

    public void visitTypeTest(TypeTest tree) {
        scan(tree.expr);
        scan(tree.clazz);
    }

    public void visitIndexed(Indexed tree) {
        scan(tree.indexed);
        scan(tree.index);
    }

    public void visitSelect(Select tree) {
        scan(tree.selected);
    }

    public void visitIdent(Ident tree) {
    }

    public void visitLiteral(Literal tree) {
    }

    public void visitTypeIdent(TypeIdent tree) {
    }

    public void visitTypeArray(TypeArray tree) {
        scan(tree.elemtype);
    }

    public void visitErroneous(Erroneous tree) {
    }

    public void visitTree(Tree tree) {
        assert false;
    }
}

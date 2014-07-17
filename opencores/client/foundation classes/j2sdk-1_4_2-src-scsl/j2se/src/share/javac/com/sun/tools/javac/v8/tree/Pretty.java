/**
 * @(#)Pretty.java	1.33 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.tools.javac.v8.tree;
import java.io.*;

import com.sun.tools.javac.v8.util.*;

import com.sun.tools.javac.v8.code.*;

import com.sun.tools.javac.v8.code.Symbol.*;

import com.sun.tools.javac.v8.tree.Tree.*;


/**
 * Prints out a tree as an indented Java source program.
 */
public class Pretty extends Tree.Visitor implements Flags {

    public Pretty(PrintWriter out, boolean sourceOutput) {
        super();
        this.out = out;
        this.sourceOutput = sourceOutput;
    }

    /**
      * Set when we are producing source output.  If we're not
      *  producing source output, we can sometimes give more detail in
      *  the output even though that detail would not be valid java
      *  soruce.
      */
    private final boolean sourceOutput;

    /**
     * The output stream on which trees are printed.
     */
    PrintWriter out;

    /**
     * Indentation width (can be reassigned from outside).
     */
    public int width = 4;

    /**
     * The current left margin.
     */
    int lmargin = 0;

    /**
     * The enclosing class name.
     */
    Name enclClassName;

    /**
     * A hashtable mapping trees to their documentation comments
     *  (can be null)
     */
    Hashtable docComments = null;

    /**
     * Align code to be indented to left margin.
     */
    void align() {
        for (int i = 0; i < lmargin; i++)
            out.print(" ");
    }

    /**
      * Increase left margin by indentation width.
      */
    void indent() {
        lmargin = lmargin + width;
    }

    /**
      * Decrease left margin by indentation width.
      */
    void undent() {
        lmargin = lmargin - width;
    }

    /**
      * Enter a new precedence level. Emit a `(' if new precedence level
      *  is less than precedence level so far.
      *  @param contextPrec    The precedence level in force so far.
      *  @param ownPrec        The new precedence level.
      */
    void open(int contextPrec, int ownPrec) {
        if (ownPrec < contextPrec)
            out.print("(");
    }

    /**
      * Leave precedence level. Emit a `(' if inner precedence level
      *  is less than precedence level we revert to.
      *  @param contextPrec    The precedence level we revert to.
      *  @param ownPrec        The inner precedence level.
      */
    void close(int contextPrec, int ownPrec) {
        if (ownPrec < contextPrec)
            out.print(")");
    }

    /**
      * Print string, replacing all non-ascii character with unicode escapes.
      */
    public void print(String s) {
        out.print(Convert.escapeUnicode(s));
    }

    /**
      * Print new line.
      */
    public void println() {
        out.println();
    }

    /**
      * Visitor argument: the current precedence level.
      */
    int prec;

    /**
     * Visitor method: print expression tree.
     *  @param prec  The current precedence level.
     */
    public void printExpr(Tree tree, int prec) {
        int prevPrec = this.prec;
        try {
            this.prec = prec;
            if (tree == null)
                print("/*missing*/");
            else
                tree.accept(this);
        }
        finally { this.prec = prevPrec;
                } }

    /**
      * Derived visitor method: print expression tree at minimum precedence level
      *  for expression.
      */
    public void printExpr(Tree tree) {
        printExpr(tree, TreeInfo.noPrec);
    }

    /**
      * Derived visitor method: print statement tree.
      */
    public void printStat(Tree tree) {
        printExpr(tree, TreeInfo.notExpression);
    }

    /**
      * Derived visitor method: print list of expression trees, separated by given string.
      *  @param sep the separator string
      */
    public void printExprs(List trees, String sep) {
        if (trees.nonEmpty()) {
            printExpr((Tree) trees.head);
            for (List l = trees.tail; l.nonEmpty(); l = l.tail) {
                print(sep);
                printExpr((Tree) l.head);
            }
        }
    }

    /**
      * Derived visitor method: print list of expression trees, separated by commas.
      */
    public void printExprs(List trees) {
        printExprs(trees, ", ");
    }

    /**
      * Derived visitor method: print list of statements, each on a separate line.
      */
    public void printStats(List trees) {
        for (List l = trees; l.nonEmpty(); l = l.tail) {
            align();
            printStat((Tree) l.head);
            println();
        }
    }

    /**
      * Print a set of modifiers.
      */
    public void printFlags(long flags) {
        if ((flags & SYNTHETIC) != 0)
            print("/*synthetic*/ ");
        print(TreeInfo.flagNames(flags));
        if ((flags & StandardFlags) != 0)
            print(" ");
    }

    /**
      * Print documentation comment, if it exists
      *  @param tree    The tree for which a documentation comment should be printed.
      */
    public void printDocComment(Tree tree) {
        if (docComments != null) {
            String dc = (String) docComments.get(tree);
            if (dc != null) {
                print("/**");
                println();
                int pos = 0;
                int endpos = lineEndPos(dc, pos);
                while (pos < dc.length()) {
                    align();
                    print(" *");
                    if (pos < dc.length() && dc.charAt(pos) > ' ')
                        print(" ");
                    print(dc.substring(pos, endpos));
                    println();
                    pos = endpos + 1;
                    endpos = lineEndPos(dc, pos);
                }
                align();
                print(" */");
                println();
                align();
            }
        }
    }

    static int lineEndPos(String s, int start) {
        int pos = s.indexOf('\n', start);
        if (pos < 0)
            pos = s.length();
        return pos;
    }

    /**
      * Print a block.
      */
    public void printBlock(List stats) {
        print("{");
        println();
        indent();
        printStats(stats);
        undent();
        align();
        print("}");
    }

    /**
      * Print unit consisting of package clause and import statements in toplevel,
      *  followed by class definition. if class definition == null,
      *  print all definitions in toplevel.
      *  @param tree     The toplevel tree
      *  @param cdef     The class definition, which is assumed to be part of the
      *                  toplevel tree.
      */
    public void printUnit(TopLevel tree, ClassDef cdef) {
        docComments = tree.docComments;
        printDocComment(tree);
        if (tree.pid != null) {
            print("package ");
            printExpr(tree.pid);
            print(";");
            println();
        }
        for (List l = tree.defs; l.nonEmpty() &&
                (cdef == null || ((Tree) l.head).tag == Tree.IMPORT); l = l.tail) {
            printStat((Tree) l.head);
            println();
        }
        if (cdef != null) {
            printStat(cdef);
            println();
        }
    }

    /**
      * Visitor methods
      */
    public void visitTopLevel(TopLevel tree) {
        printUnit(tree, null);
    }

    public void visitImport(Import tree) {
        print("import ");
        printExpr(tree.qualid);
        print(";");
        println();
    }

    public void visitClassDef(ClassDef tree) {
        println();
        align();
        printDocComment(tree);
        printFlags(tree.flags & ~INTERFACE);
        Name enclClassNamePrev = enclClassName;
        enclClassName = tree.name;
        if ((tree.flags & INTERFACE) != 0) {
            print("interface " + tree.name);
            if (tree.implementing.nonEmpty()) {
                print(" extends ");
                printExprs(tree.implementing);
            }
        } else {
            print("class " + tree.name);
            if (tree.extending != null) {
                print(" extends ");
                printExpr(tree.extending);
            }
            if (tree.implementing.nonEmpty()) {
                print(" implements ");
                printExprs(tree.implementing);
            }
        }
        print(" ");
        printBlock(tree.defs);
        enclClassName = enclClassNamePrev;
    }

    public void visitMethodDef(MethodDef tree) {
        if (tree.name == tree.name.table.init && enclClassName == null &&
                sourceOutput)
            return;
        println();
        align();
        printDocComment(tree);
        printFlags(tree.flags);
        if (tree.name == tree.name.table.init) {
            print(enclClassName != null ? enclClassName.toString() :
                    tree.name.toString());
        } else {
            printExpr(tree.restype);
            print(" " + tree.name);
        }
        print("(");
        printExprs(tree.params);
        print(")");
        if (tree.thrown.nonEmpty()) {
            print(" throws ");
            printExprs(tree.thrown);
        }
        if (tree.body != null) {
            print(" ");
            printStat(tree.body);
        } else {
            print(";");
        }
    }

    public void visitVarDef(VarDef tree) {
        if (docComments != null && docComments.get(tree) != null) {
            println();
            align();
        }
        printDocComment(tree);
        printFlags(tree.flags);
        printExpr(tree.vartype);
        print(" " + tree.name);
        if (tree.init != null) {
            print(" = ");
            printExpr(tree.init);
        }
        if (prec == TreeInfo.notExpression)
            print(";");
    }

    public void visitSkip(Skip tree) {
        print(";");
    }

    public void visitBlock(Block tree) {
        printFlags(tree.flags);
        printBlock(tree.stats);
    }

    public void visitDoLoop(DoLoop tree) {
        print("do ");
        printStat(tree.body);
        align();
        print(" while ");
        if (tree.cond.tag == Tree.PARENS) {
            printExpr(tree.cond);
        } else {
            print("(");
            printExpr(tree.cond);
            print(")");
        }
        print(";");
    }

    public void visitWhileLoop(WhileLoop tree) {
        print("while ");
        if (tree.cond.tag == Tree.PARENS) {
            printExpr(tree.cond);
        } else {
            print("(");
            printExpr(tree.cond);
            print(")");
        }
        print(" ");
        printStat(tree.body);
    }

    public void visitForLoop(ForLoop tree) {
        print("for (");
        if (tree.init.nonEmpty()) {
            if (((Tree) tree.init.head).tag == Tree.VARDEF) {
                printExpr((Tree) tree.init.head);
                for (List l = tree.init.tail; l.nonEmpty(); l = l.tail) {
                    VarDef vdef = (VarDef) l.head;
                    print(", " + vdef.name + " = ");
                    printExpr(vdef.init);
                }
            } else {
                printExprs(tree.init);
            }
        }
        print("; ");
        if (tree.cond != null)
            printExpr(tree.cond);
        print("; ");
        printExprs(tree.step);
        print(") ");
        printStat(tree.body);
    }

    public void visitLabelled(Labelled tree) {
        print(tree.label + ": ");
        printStat(tree.body);
    }

    public void visitSwitch(Switch tree) {
        print("switch ");
        if (tree.selector.tag == Tree.PARENS) {
            printExpr(tree.selector);
        } else {
            print("(");
            printExpr(tree.selector);
            print(")");
        }
        print(" {");
        println();
        printStats(tree.cases);
        align();
        print("}");
    }

    public void visitCase(Case tree) {
        if (tree.pat == null) {
            print("default");
        } else {
            print("case ");
            printExpr(tree.pat);
        }
        print(": ");
        println();
        indent();
        printStats(tree.stats);
        undent();
        align();
    }

    public void visitSynchronized(Synchronized tree) {
        print("synchronized ");
        if (tree.lock.tag == Tree.PARENS) {
            printExpr(tree.lock);
        } else {
            print("(");
            printExpr(tree.lock);
            print(")");
        }
        print(" ");
        printStat(tree.body);
    }

    public void visitTry(Try tree) {
        print("try ");
        printStat(tree.body);
        for (List l = tree.catchers; l.nonEmpty(); l = l.tail) {
            printStat((Tree) l.head);
        }
        if (tree.finalizer != null) {
            print(" finally ");
            printStat(tree.finalizer);
        }
    }

    public void visitCatch(Catch tree) {
        print(" catch (");
        printExpr(tree.param);
        print(") ");
        printStat(tree.body);
    }

    public void visitConditional(Conditional tree) {
        open(prec, TreeInfo.condPrec);
        printExpr(tree.cond, TreeInfo.condPrec);
        print(" ? ");
        printExpr(tree.truepart, TreeInfo.condPrec);
        print(" : ");
        printExpr(tree.falsepart, TreeInfo.condPrec);
        close(prec, TreeInfo.condPrec);
    }

    public void visitIf(If tree) {
        print("if ");
        if (tree.cond.tag == Tree.PARENS) {
            printExpr(tree.cond);
        } else {
            print("(");
            printExpr(tree.cond);
            print(")");
        }
        print(" ");
        printStat(tree.thenpart);
        if (tree.elsepart != null) {
            print(" else ");
            printStat(tree.elsepart);
        }
    }

    public void visitExec(Exec tree) {
        printExpr(tree.expr);
        if (prec == TreeInfo.notExpression)
            print(";");
    }

    public void visitBreak(Break tree) {
        print("break");
        if (tree.label != null)
            print(" " + tree.label);
        print(";");
    }

    public void visitContinue(Continue tree) {
        print("continue");
        if (tree.label != null)
            print(" " + tree.label);
        print(";");
    }

    public void visitReturn(Return tree) {
        print("return");
        if (tree.expr != null) {
            print(" ");
            printExpr(tree.expr);
        }
        print(";");
    }

    public void visitThrow(Throw tree) {
        print("throw ");
        printExpr(tree.expr);
        print(";");
    }

    public void visitAssert(Assert tree) {
        print("assert ");
        printExpr(tree.cond);
        if (tree.detail != null) {
            print(" : ");
            printExpr(tree.detail);
        }
        print(";");
    }

    public void visitApply(Apply tree) {
        printExpr(tree.meth);
        print("(");
        printExprs(tree.args);
        print(")");
    }

    public void visitNewClass(NewClass tree) {
        if (tree.encl != null) {
            printExpr(tree.encl);
            print(".");
        }
        print("new ");
        printExpr(tree.clazz);
        print("(");
        printExprs(tree.args);
        print(")");
        if (tree.def != null) {
            Name enclClassNamePrev = enclClassName;
            enclClassName = null;
            printBlock(((ClassDef) tree.def).defs);
            enclClassName = enclClassNamePrev;
        }
    }

    public void visitNewArray(NewArray tree) {
        if (tree.elemtype != null) {
            print("new ");
            int n = 0;
            Tree elemtype = tree.elemtype;
            while (elemtype.tag == Tree.TYPEARRAY) {
                n++;
                elemtype = ((TypeArray) elemtype).elemtype;
            }
            printExpr(elemtype);
            for (List l = tree.dims; l.nonEmpty(); l = l.tail) {
                print("[");
                printExpr((Tree) l.head);
                print("]");
            }
            for (int i = 0; i < n; i++) {
                print("[]");
            }
            if (tree.elems != null) {
                print("[]");
            }
        }
        if (tree.elems != null) {
            print("{");
            printExprs(tree.elems);
            print("}");
        }
    }

    public void visitParens(Parens tree) {
        print("(");
        printExpr(tree.expr);
        print(")");
    }

    public void visitAssign(Assign tree) {
        open(prec, TreeInfo.assignPrec);
        printExpr(tree.lhs, TreeInfo.assignPrec + 1);
        print(" = ");
        printExpr(tree.rhs, TreeInfo.assignPrec);
        close(prec, TreeInfo.assignPrec);
    }

    public String operatorName(int tag) {
        switch (tag) {
        case Tree.POS:
            return "+";

        case Tree.NEG:
            return "-";

        case Tree.NOT:
            return "!";

        case Tree.COMPL:
            return "~";

        case Tree.PREINC:
            return "++";

        case Tree.PREDEC:
            return "--";

        case Tree.POSTINC:
            return "++";

        case Tree.POSTDEC:
            return "--";

        case Tree.NULLCHK:
            return "<*nullchk*>";

        case Tree.OR:
            return "||";

        case Tree.AND:
            return "&&";

        case Tree.EQ:
            return "==";

        case Tree.NE:
            return "!=";

        case Tree.LT:
            return "<";

        case Tree.GT:
            return ">";

        case Tree.LE:
            return "<=";

        case Tree.GE:
            return ">=";

        case Tree.BITOR:
            return "|";

        case Tree.BITXOR:
            return "^";

        case Tree.BITAND:
            return "&";

        case Tree.SL:
            return "<<";

        case Tree.SR:
            return ">>";

        case Tree.USR:
            return ">>>";

        case Tree.PLUS:
            return "+";

        case Tree.MINUS:
            return "-";

        case Tree.MUL:
            return "*";

        case Tree.DIV:
            return "/";

        case Tree.MOD:
            return "%";

        default:
            throw new Error();

        }
    }

    public void visitAssignop(Assignop tree) {
        open(prec, TreeInfo.assignopPrec);
        printExpr(tree.lhs, TreeInfo.assignopPrec + 1);
        print(" " + operatorName(tree.tag - Tree.ASGOffset) + "= ");
        printExpr(tree.rhs, TreeInfo.assignopPrec);
        close(prec, TreeInfo.assignopPrec);
    }

    public void visitUnary(Unary tree) {
        int ownprec = TreeInfo.opPrec(tree.tag);
        String opname = operatorName(tree.tag).toString();
        open(prec, ownprec);
        if (tree.tag <= Tree.PREDEC) {
            print(opname);
            printExpr(tree.arg, ownprec);
        } else {
            printExpr(tree.arg, ownprec);
            print(opname);
        }
        close(prec, ownprec);
    }

    public void visitBinary(Binary tree) {
        int ownprec = TreeInfo.opPrec(tree.tag);
        String opname = operatorName(tree.tag).toString();
        open(prec, ownprec);
        printExpr(tree.lhs, ownprec);
        print(" " + opname + " ");
        printExpr(tree.rhs, ownprec + 1);
        close(prec, ownprec);
    }

    public void visitTypeCast(TypeCast tree) {
        open(prec, TreeInfo.prefixPrec);
        print("(");
        printExpr(tree.clazz);
        print(")");
        printExpr(tree.expr, TreeInfo.prefixPrec);
        close(prec, TreeInfo.prefixPrec);
    }

    public void visitTypeTest(TypeTest tree) {
        open(prec, TreeInfo.ordPrec);
        printExpr(tree.expr, TreeInfo.ordPrec);
        print(" instanceof ");
        printExpr(tree.clazz, TreeInfo.ordPrec + 1);
        close(prec, TreeInfo.ordPrec);
    }

    public void visitIndexed(Indexed tree) {
        printExpr(tree.indexed, TreeInfo.postfixPrec);
        print("[");
        printExpr(tree.index);
        print("]");
    }

    public void visitSelect(Select tree) {
        printExpr(tree.selected, TreeInfo.postfixPrec);
        print("." + tree.name);
    }

    public void visitIdent(Ident tree) {
        print(tree.name.toString());
    }

    public void visitLiteral(Literal tree) {
        switch (tree.typetag) {
        case Type.INT:
            print(tree.value.toString());
            break;

        case Type.LONG:
            print(tree.value.toString() + "L");
            break;

        case Type.FLOAT:
            print(tree.value.toString() + "F");
            break;

        case Type.DOUBLE:
            print(tree.value.toString());
            break;

        case Type.CHAR:
            print("\'" + Convert.quote(
                    String.valueOf((char)((Number) tree.value).intValue())) + "\'");
            break;

        case Type.CLASS:
            print("\"" + Convert.quote((String) tree.value) + "\"");
            break;

        default:
            print(tree.value.toString());

        }
    }

    public void visitTypeIdent(TypeIdent tree) {
        switch (tree.typetag) {
        case TypeTags.BYTE:
            print("byte");
            break;

        case TypeTags.CHAR:
            print("char");
            break;

        case TypeTags.SHORT:
            print("short");
            break;

        case TypeTags.INT:
            print("int");
            break;

        case TypeTags.LONG:
            print("long");
            break;

        case TypeTags.FLOAT:
            print("float");
            break;

        case TypeTags.DOUBLE:
            print("double");
            break;

        case TypeTags.BOOLEAN:
            print("boolean");
            break;

        case TypeTags.VOID:
            print("void");
            break;

        default:
            print("error");
            break;

        }
    }

    public void visitTypeArray(TypeArray tree) {
        printExpr(tree.elemtype);
        print("[]");
    }

    public void visitErroneous(Erroneous tree) {
        print("(ERROR)");
    }

    public void visitTree(Tree tree) {
        print("(UNKNOWN: " + tree + ")");
        println();
    }
}

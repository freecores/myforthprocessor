/**
 * @(#)TreeMaker.java	1.32 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.tools.javac.v8.tree;
import com.sun.tools.javac.v8.util.*;

import com.sun.tools.javac.v8.code.*;

import com.sun.tools.javac.v8.tree.Tree.*;

import com.sun.tools.javac.v8.code.Symbol.*;

import com.sun.tools.javac.v8.code.Type.*;


/**
 * Factory class for trees
 */
public class TreeMaker implements Tree.Factory, Kinds, Flags, TypeTags {

    /**
     * The context key for the tree factory.
     */
    private static final Context.Key treeMakerKey = new Context.Key();

    /**
     * Get the TreeMaker instance.
     */
    public static TreeMaker instance(Context context) {
        TreeMaker instance = (TreeMaker) context.get(treeMakerKey);
        if (instance == null)
            instance = new TreeMaker(context);
        return instance;
    }

    /**
      * The position at which subsequent trees will be created.
      */
    public int pos = Position.NOPOS;

    /**
     * The toplevel tree to which created trees belong.
     */
    public TopLevel toplevel;

    /**
     * The current name table.
     */
    private Name.Table names;

    /**
     * Create a tree maker with null toplevel and NOPOS as initial position.
     */
    private TreeMaker(Context context) {
        super();
        context.put(treeMakerKey, this);
        this.pos = Position.NOPOS;
        this.toplevel = null;
        this.names = Name.Table.instance(context);
    }

    /**
      * Create a tree maker with null toplevel and NOPOS as initial position.
      */
    public TreeMaker(TopLevel toplevel) {
        super();
        this.pos = Position.FIRSTPOS;
        this.toplevel = toplevel;
        this.names = toplevel.sourcefile.table;
    }

    /**
      * Reassign current position.
      */
    public TreeMaker at(int pos) {
        this.pos = pos;
        return this;
    }

    /**
      * Create given tree node at current position.
      */
    public TopLevel TopLevel(Tree pid, List defs) {
        TopLevel tree = new TopLevel(pid, defs, null, null, null, null);
        tree.pos = pos;
        return tree;
    }

    public Import Import(Tree qualid) {
        Import tree = new Import(qualid);
        tree.pos = pos;
        return tree;
    }

    public ClassDef ClassDef(long flags, Name name, List typarams,
            Tree extending, List implementing, List defs) {
        ClassDef tree = new ClassDef(flags, name, typarams, extending, implementing,
                defs, null);
        tree.pos = pos;
        return tree;
    }

    public MethodDef MethodDef(long flags, Name name, Tree restype,
            List typarams, List params, List thrown, Block body) {
        MethodDef tree = new MethodDef(flags, name, restype, typarams, params, thrown,
                body, null);
        tree.pos = pos;
        return tree;
    }

    public VarDef VarDef(long flags, Name name, Tree vartype, Tree init) {
        VarDef tree = new VarDef(flags, name, vartype, init, null);
        tree.pos = pos;
        return tree;
    }

    public Skip Skip() {
        Skip tree = new Skip();
        tree.pos = pos;
        return tree;
    }

    public Block Block(long flags, List stats) {
        Block tree = new Block(flags, stats);
        tree.pos = pos;
        return tree;
    }

    public DoLoop DoLoop(Tree body, Tree cond) {
        DoLoop tree = new DoLoop(body, cond);
        tree.pos = pos;
        return tree;
    }

    public WhileLoop WhileLoop(Tree cond, Tree body) {
        WhileLoop tree = new WhileLoop(cond, body);
        tree.pos = pos;
        return tree;
    }

    public ForLoop ForLoop(List init, Tree cond, List step, Tree body) {
        ForLoop tree = new ForLoop(init, cond, step, body);
        tree.pos = pos;
        return tree;
    }

    public Labelled Labelled(Name label, Tree body) {
        Labelled tree = new Labelled(label, body);
        tree.pos = pos;
        return tree;
    }

    public Switch Switch(Tree selector, List cases) {
        Switch tree = new Switch(selector, cases);
        tree.pos = pos;
        return tree;
    }

    public Case Case(Tree pat, List stats) {
        Case tree = new Case(pat, stats);
        tree.pos = pos;
        return tree;
    }

    public Synchronized Synchronized(Tree lock, Tree body) {
        Synchronized tree = new Synchronized(lock, body);
        tree.pos = pos;
        return tree;
    }

    public Try Try(Tree body, List catchers, Tree finalizer) {
        Try tree = new Try(body, catchers, finalizer);
        tree.pos = pos;
        return tree;
    }

    public Catch Catch(VarDef param, Tree body) {
        Catch tree = new Catch(param, body);
        tree.pos = pos;
        return tree;
    }

    public Conditional Conditional(Tree cond, Tree thenpart, Tree elsepart) {
        Conditional tree = new Conditional(cond, thenpart, elsepart);
        tree.pos = pos;
        return tree;
    }

    public If If(Tree cond, Tree thenpart, Tree elsepart) {
        If tree = new If(cond, thenpart, elsepart);
        tree.pos = pos;
        return tree;
    }

    public Exec Exec(Tree expr) {
        Exec tree = new Exec(expr);
        tree.pos = pos;
        return tree;
    }

    public Break Break(Name label) {
        Break tree = new Break(label, null);
        tree.pos = pos;
        return tree;
    }

    public Continue Continue(Name label) {
        Continue tree = new Continue(label, null);
        tree.pos = pos;
        return tree;
    }

    public Return Return(Tree expr) {
        Return tree = new Return(expr);
        tree.pos = pos;
        return tree;
    }

    public Throw Throw(Tree expr) {
        Throw tree = new Throw(expr);
        tree.pos = pos;
        return tree;
    }

    public Assert Assert(Tree cond, Tree detail) {
        Assert tree = new Assert(cond, detail);
        tree.pos = pos;
        return tree;
    }

    public Apply Apply(Tree fn, List args) {
        Apply tree = new Apply(fn, args);
        tree.pos = pos;
        return tree;
    }

    public NewClass NewClass(Tree encl, Tree clazz, List args, ClassDef def) {
        NewClass tree = new NewClass(encl, clazz, args, def, null);
        tree.pos = pos;
        return tree;
    }

    public NewArray NewArray(Tree elemtype, List dims, List elems) {
        NewArray tree = new NewArray(elemtype, dims, elems);
        tree.pos = pos;
        return tree;
    }

    public Parens Parens(Tree expr) {
        Parens tree = new Parens(expr);
        tree.pos = pos;
        return tree;
    }

    public Assign Assign(Tree lhs, Tree rhs) {
        Assign tree = new Assign(lhs, rhs);
        tree.pos = pos;
        return tree;
    }

    public Assignop Assignop(int opcode, Tree lhs, Tree rhs) {
        Assignop tree = new Assignop(opcode, lhs, rhs, null);
        tree.pos = pos;
        return tree;
    }

    public Unary Unary(int opcode, Tree arg) {
        Unary tree = new Unary(opcode, arg, null);
        tree.pos = pos;
        return tree;
    }

    public Binary Binary(int opcode, Tree lhs, Tree rhs) {
        Binary tree = new Binary(opcode, lhs, rhs, null);
        tree.pos = pos;
        return tree;
    }

    public TypeCast TypeCast(Tree clazz, Tree expr) {
        TypeCast tree = new TypeCast(clazz, expr);
        tree.pos = pos;
        return tree;
    }

    public TypeTest TypeTest(Tree expr, Tree clazz) {
        TypeTest tree = new TypeTest(expr, clazz);
        tree.pos = pos;
        return tree;
    }

    public Indexed Indexed(Tree indexed, Tree index) {
        Indexed tree = new Indexed(indexed, index);
        tree.pos = pos;
        return tree;
    }

    public Select Select(Tree selected, Name selector) {
        Select tree = new Select(selected, selector, null);
        tree.pos = pos;
        return tree;
    }

    public Ident Ident(Name name) {
        Ident tree = new Ident(name, null);
        tree.pos = pos;
        return tree;
    }

    public Literal Literal(int tag, Object value) {
        Literal tree = new Literal(tag, value);
        tree.pos = pos;
        return tree;
    }

    public TypeIdent TypeIdent(int typetag) {
        TypeIdent tree = new TypeIdent(typetag);
        tree.pos = pos;
        return tree;
    }

    public TypeArray TypeArray(Tree elemtype) {
        TypeArray tree = new TypeArray(elemtype);
        tree.pos = pos;
        return tree;
    }

    public Erroneous Erroneous() {
        Erroneous tree = new Erroneous();
        tree.pos = pos;
        return tree;
    }

    /**
      * Create an identifier from a symbol.
      */
    public Tree Ident(Symbol sym) {
        return new Ident(sym.name, sym).setPos(pos).setType(sym.type);
    }

    /**
      * Create a selection node from a qualifier tree and a symbol.
      *  @param base   The qualifier tree.
      */
    public Tree Select(Tree base, Symbol sym) {
        return new Select(base, sym.name, sym).setPos(pos).setType(sym.type);
    }

    /**
      * Create a qualified identifier from a symbol, adding enough qualifications
      *  to make the reference unique.
      */
    public Tree QualIdent(Symbol sym) {
        return isUnqualifiable(sym) ? Ident(sym) : Select(QualIdent(sym.owner), sym);
    }

    /**
      * Create an identifier that refers to the variable declared in given variable
      *  declaration.
      */
    public Tree Ident(VarDef param) {
        return Ident(param.sym);
    }

    /**
      * Create a list of identifiers referring to the variables declared
      *  in given list of variable declarations.
      */
    public List Idents(List params) {
        ListBuffer ids = new ListBuffer();
        for (List l = params; l.nonEmpty(); l = l.tail)
            ids.append(Ident((VarDef) l.head));
        return ids.toList();
    }

    /**
      * Create a tree representing `this', given its type.
      */
    public Tree This(Type t) {
        return Ident(new VarSymbol(FINAL, names._this, t, t.tsym));
    }

    /**
      * Create a tree representing `super', given its type and owner.
      */
    public Tree Super(Type t, TypeSymbol owner) {
        return Ident(new VarSymbol(FINAL, names._super, t, owner));
    }

    /**
      * Create a method invocation from a method tree and a list of argument trees.
      */
    public Tree App(Tree meth, List args) {
        return Apply(meth, args).setType(meth.type.restype());
    }

    /**
      * Create a tree representing given type.
      */
    public Tree Type(Type t) {
        if (t == null)
            return null;
        Tree tp;
        switch (t.tag) {
        case BYTE:

        case CHAR:

        case SHORT:

        case INT:

        case LONG:

        case FLOAT:

        case DOUBLE:

        case BOOLEAN:

        case VOID:
            tp = TypeIdent(t.tag);
            break;

        case CLASS:
            Type outer = t.outer();
            Tree clazz = outer.tag == CLASS && t.tsym.owner.kind == TYP ?
                    Select(Type(outer), t.tsym) : QualIdent(t.tsym);
            tp = clazz;
            break;

        case ARRAY:
            tp = TypeArray(Type(t.elemtype()));
            break;

        case ERROR:
            tp = TypeIdent(ERROR);
            break;

        default:
            throw new AssertionError("unexpected type: " + t);

        }
        return tp.setType(t);
    }

    private Tree Selectors(Tree base, Symbol sym, Symbol limit) {
        if (sym == limit)
            return base;
        else
            return Select(Selectors(base, sym.owner, limit), sym);
    }

    /**
      * Create a list of trees representing given list of types.
      */
    public List Types(List ts) {
        ListBuffer types = new ListBuffer();
        for (List l = ts; l.nonEmpty(); l = l.tail)
            types.append(Type((Type) l.head));
        return types.toList();
    }

    /**
      * Create a variable definition from a variable symbol and an initializer
      *  expression.
      */
    public VarDef VarDef(VarSymbol v, Tree init) {
        return (VarDef) new VarDef(v.flags(), v.name, Type(v.type), init,
                v).setPos(pos).setType(v.type);
    }

    /**
      * Create a method definition from a method symbol and a method body.
      */
    public MethodDef MethodDef(MethodSymbol m, Block body) {
        return MethodDef(m, m.type, body);
    }

    /**
      * Create a method definition from a method symbol, method type
      *  and a method body.
      */
    public MethodDef MethodDef(MethodSymbol m, Type mtype, Block body) {
        return (MethodDef) new MethodDef(m.flags(), m.name,
                Type(mtype.restype()), TypeParameter.emptyList,
                Params(mtype.argtypes(), m), Types(mtype.thrown()), body,
                m).setPos(pos).setType(mtype);
    }

    /**
      * Create a value parameter tree from its name, type, and owner.
      */
    public VarDef Param(Name name, Type argtype, Symbol owner) {
        return VarDef(new VarSymbol(0, name, argtype, owner), null);
    }

    /**
      * Create a a list of value parameter trees x0, ..., xn from a list of
      *  their types and an their owner.
      */
    public List Params(List argtypes, Symbol owner) {
        ListBuffer params = new ListBuffer();
        int i = 0;
        for (List l = argtypes; l.nonEmpty(); l = l.tail)
            params.append(Param(paramName(i++), (Type) l.head, owner));
        return params.toList();
    }

    /**
      * Wrap a method invocation in an expression statement or return statement,
      *  depending on whether the method invocation expression's type is void.
      */
    public Tree Call(Tree apply) {
        return apply.type.tag == VOID ? (Tree) Exec(apply) : (Tree) Return(apply);
    }

    /**
      * Construct an assignment from a variable symbol and a right hand side.
      */
    public Tree Assignment(Symbol v, Tree rhs) {
        return Exec(Assign(Ident(v), rhs).setType(v.type));
    }

    /**
      * Can given symbol be referred to in unqualified form?
      */
    boolean isUnqualifiable(Symbol sym) {
        if (sym.owner == null || sym.owner.kind == MTH || sym.owner.kind == VAR ||
                sym.owner.name == names.empty) {
            return true;
        } else if (sym.kind == TYP && toplevel != null) {
            Scope.Entry e;
            e = toplevel.namedImportScope.lookup(sym.name);
            if (e.scope != null) {
                return e.scope.owner == e.sym.owner && e.sym == sym &&
                        e.next().scope == null;
            }
            e = toplevel.packge.members().lookup(sym.name);
            if (e.scope != null) {
                return e.scope.owner == e.sym.owner && e.sym == sym &&
                        e.next().scope == null;
            }
            e = toplevel.starImportScope.lookup(sym.name);
            if (e.scope != null) {
                return e.scope.owner == e.sym.owner && e.sym == sym &&
                        e.next().scope == null;
            }
        }
        return false;
    }

    /**
      * The name of synthetic parameter number `i'.
      */
    public Name paramName(int i) {
        return names.fromString("x" + i);
    }

    /**
      * The name of synthetic type parameter number `i'.
      */
    public Name typaramName(int i) {
        return names.fromString("A" + i);
    }
}

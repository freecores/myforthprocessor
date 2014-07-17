/**
 * @(#)Enter.java	1.83 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.tools.javac.v8.comp;
import java.io.File;

import com.sun.tools.javac.v8.util.*;

import com.sun.tools.javac.v8.code.*;

import com.sun.tools.javac.v8.tree.*;

import com.sun.tools.javac.v8.code.Type.*;

import com.sun.tools.javac.v8.code.Symbol.*;

import com.sun.tools.javac.v8.tree.Tree.*;


/**
 * This class enters symbols for all encountered definitions into
 *  the symbol table. The pass consists of two phases, organized as
 *  follows:
 *
 *  In the first phase, all class symbols are intered into their enclosing scope,
 *  descending recursively down the tree for classes which are members of other
 *  classes. The class symbols are given a CompleteEnter object as completer.
 *
 *  In the second phase classes are completed using CompleteEnter.complete().
 *  Completion might occur on demand, but any classes that are not completed that
 *  way will be eventually completed by processing the `uncompleted' queue.
 *  Completion entails (1) determination of a classe's parameters, supertype and
 *  interfaces, as well as (2) entering all symbols defined in the class into its
 *  scope,  with the exception of class symbols which have been entered in phase 1.
 *  (2) depends on (1) having been completed for a class and all its superclasses
 *  and enclosing classes. That's why, after doing (1), we put classes in a
 *  `halfcompleted' queue. Only when we have performed (1) for a class and all it's
 *  superclasses and enclosing classes, we proceed to (2).
 *
 *  Whereas the first phase is organized as a sweep through all compiled
 *  syntax trees,  the second phase is demand. Members of a class are entered
 *  when the contents of a class are first accessed. This is accomplished
 *  by installing completer objects in class symbols for compiled classes
 *  which invoke the member-enter phase for the corresponding class tree.
 *
 *  Classes migrate from one phase to the next via queues:
 *
 *  class enter -> (uncompleted)   --> member enter (1)
 *		-> (halfcompleted) --> member enter (2)
 *		-> (todo)	   --> attribute // only for toplevel classes
 */
public class Enter extends Tree.Visitor implements Flags, Kinds, TypeTags {
    private static final Context.Key enterKey = new Context.Key();
    private Log log;
    private Symtab syms;
    private Resolve rs;
    private Check chk;
    private TreeMaker make;
    private Attr attr;
    private ClassReader reader;
    private Name.Table names;
    protected MemberEnter phase2;

    /**
     * A queue of all as yet unattributed toplevel classes
     */
    Todo todo;

    public static Enter instance(Context context) {
        Enter instance = (Enter) context.get(enterKey);
        if (instance == null)
            instance = new Enter(context);
        return instance;
    }

    protected Enter(Context context) {
        super();
        context.put(enterKey, this);
        names = Name.Table.instance(context);
        log = Log.instance(context);
        reader = ClassReader.instance(context);
        make = TreeMaker.instance(context);
        todo = Todo.instance(context);
        syms = Symtab.instance(context);
        rs = Resolve.instance(context);
        chk = Check.instance(context);
        attr = Attr.instance(context);
        phase2 = new MemberEnter();
        predefClassDef = new ClassDef(PUBLIC, syms.predefClass.name, null, null, null,
                null, syms.predefClass);
    }

    /**
      * A switch to determine whether we check for package/class conflicts
      */
    static final boolean checkClash = true;

    /**
     * A hashtable mapping classes to the environments current at the point
     *	of the class declaration's local definitions.
     */
    Hashtable classEnvs = Hashtable.make();

    /**
     * Accessor for classEnvs
     */
    public Env getEnv(ClassSymbol c) {
        return (Env) classEnvs.get(c);
    }

    /**
      * The queue of all classes that might still need to be completed;
      *	saved and initialized by main().
      */
    ListBuffer uncompleted;

    /**
     * A queue for classes whose members still need to be entered into the
     *	symbol table.
     */
    ListBuffer halfcompleted = new ListBuffer();

    /**
     * A dummy class to serve as enclClass for toplevel environments.
     */
    private ClassDef predefClassDef;

    /**
     * A flag to enable/disable class completion. This is
     *	necessary to prevent false cyclic dependencies involving imports.
     *	Example (due to Todd Turnbridge): Consider the following three files:
     *
     *	A.java: public class A extends B {}
     *	B.java: public class B extends C {}
     *	C.java: import A; public class C {}
     *
     *	Now compile B.java. The (import A;) is not allowed to go beyond enter
     *	phase, or a false cycle will occur.
     */
    private boolean completionEnabled = true;

    /**
     * Generate call to superclass constructor. This is:
     *
     *	  super(id_0, ..., id_n)
     *
     * or, if based == true
     *
     *	  id_0.super(id_1,...,id_n)
     *
     *	where id_0, ..., id_n are the names of the given parameters.
     *
     *	@param make    The tree factory
     *	@param params  The parameters that need to be passed to super
     *	@param based   Is first parameter a this$n?
     */
    Tree SuperCall(TreeMaker make, List params, boolean based) {
        Tree meth;
        if (based) {
            meth = make.Select(make.Ident((Tree.VarDef) params.head), names._super);
            params = params.tail;
        } else {
            meth = make.Ident(names._super);
        }
        return make.Exec(make.Apply(meth, make.Idents(params)));
    }

    /**
      * Generate default constructor for given class. For classes different
      *	from java.lang.Object, this is:
      *
      *	  c(argtype_0 x_0, ..., argtype_n x_n) throws thrown {
      *	    super(x_0, ..., x_n)
      *	  }
      *
      *	or, if based == true:
      *
      *	  c(argtype_0 x_0, ..., argtype_n x_n) throws thrown {
      *	    x_0.super(x_1, ..., x_n)
      *	  }
      *
      *	@param make	The tree factory.
      *	@param c	The class owning the default constructor.
      *	@param argtypes The parameter types of the constructor.
      *	@param thrown	The thrown exceptions of the constructor.
      *	@param based	Is first parameter a this$n?
      */
    Tree DefaultConstructor(TreeMaker make, ClassSymbol c, List argtypes,
            List thrown, boolean based) {
        List params = make.Params(argtypes, syms.noSymbol);
        List stats = Tree.emptyList;
        if (c.type != syms.objectType)
            stats = stats.prepend(SuperCall(make, params, based));
        long flags = c.flags() & Check.AccessFlags;
        if (c.name.len == 0)
            flags |= ANONCONSTR;
        Tree result = make.MethodDef(flags, names.init, null, TypeParameter.emptyList,
                params, make.Types(thrown), make.Block(0, stats));
        return result;
    }

    /**
      * Report duplicate declaration error.
      */
    private void duplicateError(int pos, Symbol sym) {
        if (!sym.type.isErroneous()) {
            log.error(pos, "already.defined", sym.toJava(), sym.javaLocation());
        }
    }

    /**
      * Check that symbol is unique in given scope.
      *	@param pos	     Position for error reporting.
      *	@param sym	     The symbol.
      *	@param s	     The scope.
      */
    private boolean checkUnique(int pos, Symbol sym, Scope s) {
        if (sym.owner.name == names.any)
            return false;
        for (Scope.Entry e = s.lookup(sym.name); e.scope == s; e = e.next()) {
            if (sym != e.sym && sym.kind == e.sym.kind && sym.name != names.error &&
                    (sym.kind != MTH || sym.type.hasSameArgs(e.sym.type))) {
                duplicateError(pos, e.sym);
                return false;
            }
        }
        return true;
    }

    /**
      * Check that single-type import is not already imported or top-level defined,
      *	but make an exception for two single-type imports which denote the same type.
      *	@param pos	     Position for error reporting.
      *	@param sym	     The symbol.
      *	@param env	     The current environment.
      */
    private boolean checkUniqueImport(int pos, Symbol sym, Env env) {
        Scope s = env.toplevel.namedImportScope;
        for (Scope.Entry e = s.lookup(sym.name); e.scope != null; e = e.next()) {
            boolean isClassDecl = e.scope == s;
            if ((isClassDecl || sym != e.sym) && sym.kind == e.sym.kind &&
                    sym.name != names.error) {
                if (!e.sym.type.isErroneous()) {
                    String what = e.sym.toJava();
                    if (!isClassDecl)
                        log.error(pos, "already.defined.single.import", what);
                    else if (sym != e.sym)
                        log.error(pos, "already.defined.this.unit", what);
                }
                return false;
            }
        }
        return true;
    }

    /**
      * Check that variable does not hide variable with same name in
      *	immediately enclosing local scope.
      *	@param pos	     Position for error reporting.
      *	@param sym	     The symbol.
      *	@param s	     The scope.
      */
    private void checkTransparentVar(int pos, VarSymbol v, Scope s) {
        if (s.next != null) {
            for (Scope.Entry e = s.next.lookup(v.name);
                    e.scope != null && e.sym.owner == v.owner; e = e.next()) {
                if (e.sym.kind == VAR && (e.sym.owner.kind & (VAR | MTH)) != 0 &&
                        v.name != names.error) {
                    duplicateError(pos, e.sym);
                    return;
                }
            }
        }
    }

    /**
      * Check that a class or interface does not hide a class or
      *	interface with same name in immediately enclosing local scope.
      *	@param pos	     Position for error reporting.
      *	@param sym	     The symbol.
      *	@param s	     The scope.
      */
    private void checkTransparentClass(int pos, ClassSymbol c, Scope s) {
        if (s.next != null) {
            for (Scope.Entry e = s.next.lookup(c.name);
                    e.scope != null && e.sym.owner == c.owner; e = e.next()) {
                if (e.sym.kind == TYP && (e.sym.owner.kind & (VAR | MTH)) != 0 &&
                        c.name != names.error) {
                    duplicateError(pos, e.sym);
                    return;
                }
            }
        }
    }

    /**
      * Check that class does not have the same name as one of
      *	its enclosing classes, or as a class defined in its enclosing scope.
      *	return true if class is unique in its enclosing scope.
      *	@param pos	     Position for error reporting.
      *	@param name	     The class name.
      *	@param s	     The enclosing scope.
      */
    private boolean checkUniqueClassName(int pos, Name name, Scope s) {
        for (Scope.Entry e = s.lookup(name); e.scope == s; e = e.next()) {
            if (e.sym.kind == TYP && e.sym.name != names.error) {
                duplicateError(pos, e.sym);
                return false;
            }
        }
        for (Symbol sym = s.owner; sym != null; sym = sym.owner) {
            if (sym.kind == TYP && sym.name == name && sym.name != names.error) {
                duplicateError(pos, sym);
                return true;
            }
        }
        return true;
    }

    /**
      * Check that name does not clash with internal names used in the
      *	translation process.
      */
    private void checkNotReserved(int pos, Name name) {
        if (name == names.classDollar || name.startsWith(names.thisDollar)) {
            log.error(pos, "name.reserved.for.internal.use", name.toJava());
        }
    }

    /**
      * Create a fresh environment for method bodies.
      *	@param tree	The method definition.
      *	@param env	The environment current outside of the method definition.
      */
    Env methodEnv(MethodDef tree, Env env) {
        Env localEnv = env.dup(tree,
                ((AttrContext) env.info).dup(
                ((AttrContext) env.info).scope.dupUnshared()));
        localEnv.enclMethod = tree;
        ((AttrContext) localEnv.info).scope.owner = tree.sym;
        if ((tree.flags & STATIC) != 0)
            ((AttrContext) localEnv.info).staticLevel++;
        return localEnv;
    }

    /**
      * Create a fresh environment for class bodies.
      *	This will create a fresh scope for local symbols of a class, referred
      *	to by the environments info.scope field.
      *	This scope will contain
      *	  - symbols for this and super
      *	  - symbols for any type parameters
      *	In addition, it serves as an anchor for scopes of methods and initializers
      *	which are nested in this scope via Scope.dup().
      *	This scope should not be confused with the members scope of a class.
      *
      *	@param tree	The class definition.
      *	@param env	The environment current outside of the class definition.
      */
    public Env classEnv(ClassDef tree, Env env) {
        Env localEnv =
                env.dup(tree, ((AttrContext) env.info).dup(new Scope(tree.sym)));
        localEnv.enclClass = tree;
        localEnv.outer = env;
        ((AttrContext) localEnv.info).isSelfCall = false;
        return localEnv;
    }

    /**
      * Create a fresh environment for toplevels.
      *	@param tree	The toplevel tree.
      */
    Env topLevelEnv(TopLevel tree) {
        Env localEnv = new Env(tree, new AttrContext());
        localEnv.toplevel = tree;
        localEnv.enclClass = predefClassDef;
        tree.namedImportScope = new Scope(tree.packge);
        tree.starImportScope = new Scope(tree.packge);
        ((AttrContext) localEnv.info).scope = tree.namedImportScope;
        return localEnv;
    }

    /**
      * Create a fresh environment for a variable's initializer.
      *	If the variable is a field, the owner of the environment's scope
      *	is be the variable itself, otherwise the owner is the method
      *	enclosing the variable definition.
      *
      *	@param tree	The variable definition.
      *	@param env	The environment current outside of the variable definition.
      */
    Env initEnv(VarDef tree, Env env) {
        Env localEnv =
                env.dupto(new AttrContextEnv(tree, ((AttrContext) env.info).dup()));
        if (tree.sym.owner.kind == TYP) {
            ((AttrContext) localEnv.info).scope =
                    ((AttrContext) env.info).scope.dup();
            ((AttrContext) localEnv.info).scope.owner = tree.sym;
        }
        if ((tree.flags & STATIC) != 0 ||
                (env.enclClass.sym.flags() & INTERFACE) != 0)
            ((AttrContext) localEnv.info).staticLevel++;
        return localEnv;
    }

    /**
      * The scope in which a member definition in environment env is to be entered
      *	This is usually the environment's scope, except for class environments,
      *	where the local scope is for type variables, and the this and super symbol
      *	only, and members go into the class member scope.
      */
    Scope enterScope(Env env) {
        return (env.tree.tag == Tree.CLASSDEF) ?
                ((ClassDef) env.tree).sym.members_field :
                ((AttrContext) env.info).scope;
    }

    /**
      * Visitor argument: the current environment.
      */
    protected Env env;

    /**
     * Visitor result: the computed type.
     */
    Type result;

    /**
     * Visitor method: enter all classes in given tree, catching any
     *	completion failure exceptions. Return the tree's type.
     *
     *	@param tree    The tree to be visited.
     *	@param env     The environment visitor argument.
     */
    Type classEnter(Tree tree, Env env) {
        Env prevEnv = this.env;
        try {
            this.env = env;
            tree.accept(this);
            return result;
        } catch (CompletionFailure ex) {
            return chk.completionError(tree.pos, ex);
        }
        finally { this.env = prevEnv;
                } }

    /**
      * Visitor method: enter classes of a list of trees, returning a list of types.
      */
    List classEnter(List trees, Env env) {
        ListBuffer ts = new ListBuffer();
        for (List l = trees; l.nonEmpty(); l = l.tail)
            ts.append(classEnter((Tree) l.head, env));
        return ts.toList();
    }

    public void visitTopLevel(TopLevel tree) {
        Name prev = log.useSource(tree.sourcefile);
        if (tree.pid != null) {
            tree.packge = reader.enterPackage(TreeInfo.fullName(tree.pid));
        } else {
            tree.packge = syms.emptyPackage;
        }
        tree.packge.complete();
        classEnter(tree.defs, topLevelEnv(tree));
        log.useSource(prev);
        result = null;
    }

    public void visitClassDef(ClassDef tree) {
        Symbol owner = ((AttrContext) env.info).scope.owner;
        Scope enclScope = enterScope(env);
        ClassSymbol c;
        if (owner.kind == PCK) {
            PackageSymbol packge = (PackageSymbol) owner;
            for (Symbol q = packge; q != null && q.kind == PCK; q = q.owner)
                q.flags_field |= EXISTS;
            c = reader.enterClass(tree.name, packge);
            packge.members().enterIfAbsent(c);
            if ((tree.flags & PUBLIC) != 0 && !classNameMatchesFileName(c, env)) {
                log.error(tree.pos, "class.public.should.be.in.file",
                        tree.name.toJava());
            }
        } else {
            if (tree.name.len != 0 &&
                    !checkUniqueClassName(tree.pos, tree.name, enclScope)) {
                result = null;
                return;
            }
            if (owner.kind == TYP) {
                c = reader.enterClass(tree.name, (TypeSymbol) owner);
                if ((owner.flags_field & INTERFACE) != 0) {
                    tree.flags |= PUBLIC | STATIC;
                }
            } else {
                c = reader.defineClass(tree.name, owner);
                c.flatname = chk.localClassName(c);
                if (c.name.len != 0)
                    checkTransparentClass(tree.pos, c,
                            ((AttrContext) env.info).scope);
            }
        }
        tree.sym = c;
        if (chk.compiled.get(c.flatname) != null) {
            duplicateClass(tree.pos, c);
            result = new ErrorType(tree.name, (TypeSymbol) owner);
            tree.sym = (ClassSymbol) result.tsym;
            return;
        }
        chk.compiled.put(c.flatname, c);
        enclScope.enter(c);
        Env localEnv = classEnv(tree, env);
        classEnvs.put(c, localEnv);
        c.completer = new CompleteEnter();
        c.flags_field = chk.checkFlags(tree.pos, tree.flags, c);
        c.sourcefile = env.toplevel.sourcefile;
        c.members_field = new Scope(c);
        ClassType ct = (ClassType) c.type;
        if (owner.kind != PCK && (c.flags_field & STATIC) == 0) {
            Symbol owner1 = owner;
            while ((owner1.kind & (VAR | MTH)) != 0 &&
                    (owner1.flags_field & STATIC) == 0) {
                owner1 = owner1.owner;
            }
            if (owner1.kind == TYP) {
                ct.outer_field = owner1.type;
            }
        }
        ct.typarams_field = classEnter(tree.typarams, localEnv);
        if (!c.isLocal() && uncompleted != null)
            uncompleted.append(c);
        classEnter(tree.defs, localEnv);
        result = c.type;
    }

    /**
      * Does class have the same name as the file it appears in?
      */
    private static boolean classNameMatchesFileName(ClassSymbol c, Env env) {
        String fname = env.toplevel.sourcefile.toString();
        String cname = c.name + ".java";
        try {
            return endsWith(fname, cname) ||
                    endsWith(new File(fname).getCanonicalPath(), cname);
        } catch (java.io.IOException ex) {
            return false;
        }
    }

    /**
      * Does path name have file name as last component?
      */
    private static boolean endsWith(String pathname, String filename) {
        return pathname.endsWith(filename) &&
                (pathname.length() == filename.length() ||
                pathname.charAt(pathname.length() - filename.length() - 1) ==
                File.separatorChar);
    }

    /**
      * Complain about a duplicate class.
      */
    protected void duplicateClass(int pos, ClassSymbol c) {
        log.error(pos, "duplicate.class", c.fullname.toJava());
    }

    /**
      * Default class enter visitor method: do nothing.
      */
    public void visitTree(Tree tree) {
        result = null;
    }

    /**
      * Main method: enter all classes in a list of toplevel trees.
      *	@param trees	  The list of trees to be processed.
      */
    public void main(List trees) {
        complete(trees, null);
    }

    /**
      * Main method: enter one class from a list of toplevel trees and
      *  place the rest on uncompleted for later processing.
      *  @param trees      The list of trees to be processed.
      *  @param c          The class symbol to be processed.
      */
    public void complete(List trees, ClassSymbol c) {
        ListBuffer prevUncompleted = uncompleted;
        if (completionEnabled)
            uncompleted = new ListBuffer();
        try {
            classEnter(trees, null);
            if (completionEnabled) {
                while (uncompleted.nonEmpty()) {
                    ClassSymbol clazz = (Symbol.ClassSymbol) uncompleted.next();
                    if (c == null || c == clazz || prevUncompleted == null)
                        clazz.complete();
                    else
                        prevUncompleted.append(clazz);
                }
                for (List l = trees; l.nonEmpty(); l = l.tail) {
                    TopLevel tree = (TopLevel) l.head;
                    if (tree.starImportScope.elems == null) {
                        Name prev = log.useSource(tree.sourcefile);
                        phase2.memberEnter(tree, topLevelEnv(tree));
                        log.useSource(prev);
                    }
                }
            }
        }
        finally { uncompleted = prevUncompleted;
                } }

    /**
      * Enter all members of a class. This is done in a second phase
      *	after the classes themselves have been entered.
      */
    protected class MemberEnter extends Tree.Visitor {

        protected MemberEnter() {
            super();
        }

        /**
          * Given a class, and an (import) scope, is there already a
          *  class with same fully qualified name in this scope?
          */
        private boolean isIncluded(Symbol c, Scope scope) {
            for (Scope.Entry e = scope.lookup(c.name); e.scope == scope;
                    e = e.next()) {
                if (e.sym.kind == c.kind && e.sym.fullName() == c.fullName())
                    return true;
            }
            return false;
        }

        /**
          * Import all classes of a class or package on demand.
          *  @param pos	     Position to be used for error reporting.
          *  @param tsym	     The class or package the members of which are imported.
          *  @param toScope   The (import) scope in which imported classes
          *		     are entered.
          */
        private void importAll(int pos, TypeSymbol tsym, Env env) {
            if (tsym.kind == PCK && tsym.members().elems == null && !tsym.exists()) {
                if (((PackageSymbol) tsym).fullname.equals(names.java_lang)) {
                    String msg = Log.getLocalizedString("fatal.err.no.java.lang");
                    throw new FatalError(msg);
                } else {
                    log.error(pos, "doesnt.exist", tsym.toJava());
                }
            }
            Scope fromScope = tsym.members();
            Scope toScope = env.toplevel.starImportScope;
            for (Scope.Entry e = fromScope.elems; e != null; e = e.sibling)
                if (e.sym.kind == TYP && !isIncluded(e.sym, toScope))
                    toScope.enter(e.sym, fromScope);
        }

        /**
          * Import given class.
          *  @param pos	     Position to be used for error reporting.
          *  @param tsym	     The class to be imported.
          *  @param toScope   The (import) scope in which the imported class
          *		     is entered.
          */
        private void importNamed(int pos, Symbol tsym, Env env) {
            if (tsym.kind == TYP && checkUniqueImport(pos, tsym, env)) {
                env.toplevel.namedImportScope.enter(tsym, tsym.owner.members());
            }
        }

        /**
          * Construct method type from method signature.
          *  @param typarams    The method's type parameters.
          *  @param params      The method's value parameters.
          *  @param res	       The method's result type,
          *		       null if it is a constructor.
          *  @param thrown      The method's thrown exceptions.
          *  @param env	       The method's (local) environment.
          */
        Type signature(List typarams, List params, Tree res, List thrown, Env env) {
            ListBuffer argbuf = new ListBuffer();
            for (List l = params; l.nonEmpty(); l = l.tail)
                argbuf.append(attr.attribType(((Tree.VarDef) l.head).vartype, env));
            Type restype = res == null ? syms.voidType : attr.attribType(res, env);
            ListBuffer thrownbuf = new ListBuffer();
            for (List l = thrown; l.nonEmpty(); l = l.tail) {
                Type exc = attr.attribType((Tree) l.head, env);
                exc = chk.checkClassType(((Tree) l.head).pos, exc);
                thrownbuf.append(exc);
            }
            Type mtype = new MethodType(argbuf.toList(), restype, thrownbuf.toList(),
                    syms.methodClass);
            return mtype;
        }

        /**
          * Visitor argument: the current environment
          */
        protected Env env;

        /**
         * Visitor method: enter field and method definitions and process import
         *  clauses, catching any completion failure exceptions.
         */
        protected void memberEnter(Tree tree, Env env) {
            Env prevEnv = this.env;
            try {
                this.env = env;
                tree.accept(this);
            } catch (CompletionFailure ex) {
                chk.completionError(tree.pos, ex);
            }
            finally { this.env = prevEnv;
                    } }

        /**
          * Visitor method: enter members in a list of trees.
          */
        void memberEnter(List trees, Env env) {
            for (List l = trees; l.nonEmpty(); l = l.tail)
                memberEnter((Tree) l.head, env);
        }

        public void visitTopLevel(TopLevel tree) {
            if (tree.starImportScope.elems != null) {
                return;
            }
            if (checkClash && tree.pid != null) {
                Symbol p = tree.packge;
                while (p.owner != syms.rootPackage) {
                    p.owner.complete();
                    if (syms.classes.get(p.fullName()) != null) {
                        log.error(tree.pos, "pkg.clashes.with.class.of.same.name",
                                p.toJava());
                    }
                    p = p.owner;
                }
            }
            importAll(tree.pos, reader.enterPackage(names.java_lang), env);
            memberEnter(tree.defs, env);
        }

        public void visitImport(Import tree) {
            Tree imp = tree.qualid;
            Name name = TreeInfo.name(imp);
            TypeSymbol p;
            Env localEnv = env.dup(tree);
            assert completionEnabled;
            completionEnabled = false;
            if (imp.tag == Tree.SELECT) {
                Select s = (Select) imp;
                p = attr.attribTree(s.selected, localEnv, TYP | PCK, Type.noType).tsym;
                if (name == names.asterisk) {
                    chk.checkCanonical(s.selected);
                    importAll(tree.pos, p, env);
                } else {
                    TypeSymbol c = attr.attribType(imp, localEnv).tsym;
                    chk.checkCanonical(imp);
                    importNamed(tree.pos, c, env);
                }
            } else {
                assert false :
                "malformed import clause";
            }
            completionEnabled = true;
        }

        public void visitMethodDef(MethodDef tree) {
            Scope enclScope = enterScope(env);
            MethodSymbol m = new MethodSymbol(0, tree.name, null, enclScope.owner);
            m.flags_field = chk.checkFlags(tree.pos, tree.flags, m);
            tree.sym = m;
            Env localEnv = methodEnv(tree, env);
            m.type = signature(tree.typarams, tree.params, tree.restype,
                    tree.thrown, localEnv);
            ((AttrContext) localEnv.info).scope.leave();
            if (checkUnique(tree.pos, m, enclScope)) {
                checkNotReserved(tree.pos, tree.name);
                enclScope.enter(m);
            }
        }

        public void visitVarDef(VarDef tree) {
            Env localEnv = env;
            if ((tree.flags & STATIC) != 0) {
                localEnv = env.dup(tree, ((AttrContext) env.info).dup());
                ((AttrContext) localEnv.info).staticLevel++;
            }
            attr.attribType(tree.vartype, localEnv);
            Scope enclScope = enterScope(env);
            VarSymbol v =
                    new VarSymbol(0, tree.name, tree.vartype.type, enclScope.owner);
            v.flags_field = chk.checkFlags(tree.pos, tree.flags, v);
            tree.sym = v;
            if (tree.init != null) {
                v.flags_field |= HASINIT;
                if ((v.flags_field & FINAL) != 0)
                    v.constValue = initEnv(tree, env);
            }
            if (checkUnique(tree.pos, v, enclScope)) {
                checkNotReserved(tree.pos, tree.name);
                checkTransparentVar(tree.pos, v, enclScope);
                enclScope.enter(v);
            }
            v.pos = tree.pos;
        }

        /**
          * Default member enter visitor method: do nothing
          */
        public void visitTree(Tree tree) {
        }
    }

    /**
      * A completer class for source classes
      */
    class CompleteEnter implements Completer {

        CompleteEnter() {
            super();
        }

        /**
          * Complete entering a class.
          *  @param sym	   The symbol of the class to be completed.
          */
        public void complete(Symbol sym) throws CompletionFailure {
            if (!completionEnabled) {
                sym.completer = this;
                return;
            }
            ClassSymbol c = (ClassSymbol) sym;
            ClassType ct = (ClassType) c.type;
            Env env = (Env) classEnvs.get(c);
            ClassDef tree = (ClassDef) env.tree;
            Name prev = log.useSource(env.toplevel.sourcefile);
            boolean isFirst = halfcompleted.isEmpty();
            try {
                halfcompleted.append(env);
                if (c.owner.kind == PCK) {
                    phase2.memberEnter(env.toplevel, env.enclosing(Tree.TOPLEVEL));
                    todo.append(env);
                }
                try {
                    c.flags_field |= (LOCKED | UNATTRIBUTED);
                    if (c.owner.kind == TYP) {
                        c.owner = chk.checkNonCyclic(tree.pos, c.owner.type).tsym;
                        c.owner.complete();
                    }
                    Type supertype = (tree.extending != null) ?
                            attribBase(tree.extending, env, true, false) :
                            (c.fullname == names.java_lang_Object) ?
                            Type.noType : syms.objectType;
                    ListBuffer interfaces = new ListBuffer();
                    Set interfaceSet = Set.make();
                    for (List l = tree.implementing; l.nonEmpty(); l = l.tail) {
                        Type i = attribBase((Tree) l.head, env, false, true);
                        if (i.tag == CLASS) {
                            interfaces.append(i);
                            chk.checkNotRepeated(((Tree) l.head).pos, i,
                                    interfaceSet);
                        }
                    }
                    ct.supertype_field = supertype;
                    ct.interfaces_field = interfaces.toList();
                    if (c.fullname == names.java_lang_Object) {
                        if (tree.extending != null) {
                            chk.checkNonCyclic(tree.extending.pos, supertype);
                            ct.supertype_field = Type.noType;
                        } else if (tree.implementing.nonEmpty()) {
                            chk.checkNonCyclic( ((Tree) tree.implementing.head).pos,
                                    (Type) ct.interfaces_field.head);
                            ct.interfaces_field = Type.emptyList;
                        }
                    }
                }
                finally { c.flags_field &= ~LOCKED;
                        } attr.attribStats(tree.typarams, env);
                if ((c.flags() & INTERFACE) == 0 &&
                        !TreeInfo.hasConstructors(tree.defs)) {
                    List argtypes = Type.emptyList;
                    List thrown = Type.emptyList;
                    boolean based = false;
                    if (c.name.len == 0) {
                        NewClass nc = (NewClass) env.next.tree;
                        if (nc.constructor != null) {
                            Type superConstrType = c.type.memberType(nc.constructor);
                            argtypes = superConstrType.argtypes();
                            if (nc.encl != null) {
                                argtypes = argtypes.prepend(nc.encl.type);
                                based = true;
                            }
                            thrown = superConstrType.thrown();
                        }
                    }
                    Tree constrDef =
                            DefaultConstructor(make.at(tree.pos), c, argtypes,
                            thrown, based);
                    tree.defs = tree.defs.prepend(constrDef);
                }
                if ((c.flags_field & INTERFACE) == 0) {
                    VarSymbol thisSym =
                            new VarSymbol(FINAL | HASINIT, names._this, c.type, c);
                    thisSym.pos = Position.FIRSTPOS;
                    ((AttrContext) env.info).scope.enter(thisSym);
                    if (ct.supertype_field.tag == CLASS) {
                        VarSymbol superSym =
                                new VarSymbol(FINAL | HASINIT, names._super,
                                ct.supertype_field, c);
                        superSym.pos = Position.FIRSTPOS;
                        ((AttrContext) env.info).scope.enter(superSym);
                    }
                }
            } catch (CompletionFailure ex) {
                chk.completionError(tree.pos, ex);
            }
            if (checkClash && c.owner.kind == PCK && c.owner != syms.emptyPackage &&
                    reader.packageExists(c.fullname)) {
                log.error(tree.pos, "clash.with.pkg.of.same.name", c.toJava());
            }
            log.useSource(prev);
            if (isFirst) {
                while (halfcompleted.nonEmpty()) {
                    finish((Env) halfcompleted.next());
                }
            }
        }

        /**
          * Enter member fields and methods of a class
          *  @param env	  the environment current for the class block.
          */
        private void finish(Env env) {
            Name prev = log.useSource(env.toplevel.sourcefile);
            ClassDef tree = (ClassDef) env.tree;
            ClassSymbol c = tree.sym;
            phase2.memberEnter(tree.defs, env);
            log.useSource(prev);
        }

        /**
          * Attribute extended or implemented type reference, and check
          *  for cyclic references.
          *  @param tree		 The tree expressing the type reference.At
          *  @param env		 The environment current at the type reference.
          *  @param classExpected     true if only a class is expected here.
          *  @param interfaceExpected true if only an interface is expected here.
          */
        private Type attribBase(Tree tree, Env env, boolean classExpected,
                boolean interfaceExpected) {
            Type t = attr.attribBase(tree, env, classExpected, interfaceExpected);
            return chk.checkNonCyclic(tree.pos, t);
        }
    }
}

/**
 * @(#)TransInner.java	1.92 03/04/10
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
 * This pass maps Java with inner classes to flat Java without
 *  inner classes.
 */
public class TransInner extends TreeTranslator implements Flags, Kinds, TypeTags,
ByteCodes {
    private static final Context.Key transInnerKey = new Context.Key();

    public static TransInner instance(Context context) {
        TransInner instance = (TransInner) context.get(transInnerKey);
        if (instance == null)
            instance = new TransInner(context);
        return instance;
    }
    private Name.Table names;
    private Log log;
    private Symtab syms;
    private Resolve rs;
    private Check chk;
    private Attr attr;
    private TreeMaker make;
    private ClassWriter writer;
    private ClassReader reader;
    private ConstFold cfolder;
    private Target target;

    private TransInner(Context context) {
        super();
        context.put(transInnerKey, this);
        names = Name.Table.instance(context);
        log = Log.instance(context);
        syms = Symtab.instance(context);
        rs = Resolve.instance(context);
        chk = Check.instance(context);
        attr = Attr.instance(context);
        make = TreeMaker.instance(context);
        writer = ClassWriter.instance(context);
        reader = ClassReader.instance(context);
        cfolder = ConstFold.instance(context);
        target = Target.instance(context);
    }

    /**
      * The currently enclosing class.
      */
    ClassSymbol currentClass;

    /**
     * A queue of all translated classes.
     */
    ListBuffer translated;

    /**
     * Environment for symbol lookup, set by translateTopLevelClass.
     */
    Env attrEnv;

    /**
     * A hash table mapping syntax trees to their ending source positions.
     */
    Hashtable endPositions;

    /**
     * A hash table mapping local classes to their definitions.
     */
    Hashtable classdefs;

    /**
     * A hash table mapping virtual accessed symbols in outer subclasses
     *  to the actually referred symbol in superclasses.
     */
    Hashtable actualSymbols;

    /**
     * The currently enclosing outermost class definition.
     */
    ClassDef outermostClassDef;

    /**
     * The currently enclosing outermost member definition.
     */
    Tree outermostMemberDef;

    /**
     * A navigator class for assembling a mapping from local class symbols
     *  to class definition trees.
     *  There is only one case; all other cases simply traverse down the tree.
     */
    class ClassMap extends TreeScanner {

        ClassMap() {
            super();
        }

        /**
          * All encountered class defs are entered into classdefs table.
          */
        public void visitClassDef(ClassDef tree) {
            classdefs.put(tree.sym, tree);
            super.visitClassDef(tree);
        }
    }

    /**
      * Map a class symbol to its definition.
      *  @param c    The class symbol of which we want to determine the definition.
      */
    ClassDef classDef(ClassSymbol c) {
        ClassDef def = (Tree.ClassDef) classdefs.get(c);
        if (def == null && outermostMemberDef != null) {
            new ClassMap().scan(outermostMemberDef);
            def = (Tree.ClassDef) classdefs.get(c);
        }
        if (def == null) {
            new ClassMap().scan(outermostClassDef);
            def = (Tree.ClassDef) classdefs.get(c);
        }
        return def;
    }

    /**
      * A hash table mapping class symbols to lists of free variables.
      *  accessed by them. Only free variables of the method immediately containing
      *  a class are associated with that class.
      */
    Hashtable freevarCache;

    /**
     * A navigator class for collecting the free variables accessed
     *  from a local class.
     *  There is only one case; all other cases simply traverse down the tree.
     */
    class FreeVarCollector extends TreeScanner {

        /**
         * The owner of the local class.
         */
        Symbol owner;

        /**
         * The current class.
         */
        ClassSymbol clazz;

        /**
         * The list of owner's variables accessed from within the local class,
         *  without any duplicates.
         */
        List fvs;

        FreeVarCollector(ClassSymbol clazz) {
            super();
            this.clazz = clazz;
            this.owner = clazz.owner;
            this.fvs = VarSymbol.emptyList;
        }

        /**
          * Add free variable to fvs list unless it is already there.
          */
        private void addFreeVar(VarSymbol v) {
            for (List l = fvs; l.nonEmpty(); l = l.tail)
                if (l.head == v)
                    return;
            fvs = fvs.prepend(v);
        }

        /**
          * Add all free variables of class c to fvs list
          *  unless they are already there.
          */
        private void addFreeVars(ClassSymbol c) {
            List fvs = (List) freevarCache.get(c);
            if (fvs != null) {
                for (List l = fvs; l.nonEmpty(); l = l.tail) {
                    addFreeVar((Symbol.VarSymbol) l.head);
                }
            }
        }

        /**
          * If tree refers to a variable in owner of local class, add it to
          *  free variables list.
          */
        public void visitIdent(Ident tree) {
            result = tree;
            visitSymbol(tree.sym);
        }

        private void visitSymbol(Symbol sym) {
            if (sym.kind == VAR) {
                while (sym != null && sym.owner != owner)
                    sym = proxies.lookup(proxyName(sym.name)).sym;
                if (sym != null && sym.owner == owner) {
                    VarSymbol v = (VarSymbol) sym;
                    if (v.constValue == null) {
                        addFreeVar(v);
                    }
                }
            }
        }

        /**
          * If tree refers to a class instance creation expression
          *  add all free variables of the freshly created class.
          */
        public void visitNewClass(NewClass tree) {
            ClassSymbol c = (ClassSymbol) tree.constructor.owner;
            addFreeVars(c);
            if (tree.encl == null && c.hasOuterInstance() &&
                    outerThisStack.head != null)
                visitSymbol((Symbol) outerThisStack.head);
            super.visitNewClass(tree);
        }

        /**
          * If tree refers to a qualified this or super expression
          *  for anything but the current class, add the outer this
          *  stack as a free variable.
          */
        public void visitSelect(Select tree) {
            if ((tree.name == names._this || tree.name == names._super) &&
                    tree.selected.type.tsym != clazz && outerThisStack.head != null)
                visitSymbol((Symbol) outerThisStack.head);
            super.visitSelect(tree);
        }

        /**
          * If tree refers to a superclass constructor call,
          *  add all free variables of the superclass.
          */
        public void visitApply(Apply tree) {
            if (TreeInfo.name(tree.meth) == names._super) {
                addFreeVars((ClassSymbol) TreeInfo.symbol(tree.meth).owner);
                Symbol constructor = TreeInfo.symbol(tree.meth);
                ClassSymbol c = (ClassSymbol) constructor.owner;
                if (c.hasOuterInstance() && tree.meth.tag != Tree.SELECT &&
                        outerThisStack.head != null)
                    visitSymbol((Symbol) outerThisStack.head);
            }
            super.visitApply(tree);
        }
    }

    /**
      * Return the variables accessed from within a local class, which
      *  are declared in the local class' owner.
      *  (in reverse order of first access).
      */
    List freevars(ClassSymbol c) {
        if ((c.owner.kind & (VAR | MTH)) != 0) {
            List fvs = (List) freevarCache.get(c);
            if (fvs == null) {
                FreeVarCollector collector = new FreeVarCollector(c);
                collector.scan(classDef(c));
                fvs = collector.fvs;
                freevarCache.put(c, fvs);
            }
            return fvs;
        } else {
            return VarSymbol.emptyList;
        }
    }

    /**
      * Make an attributed tree representing a literal. This will be an
      *  Ident node in the case of boolean literals, a Literal node in all
      *  other cases.
      *  @param type       The literal's type.
      *  @param value      The literal's value.
      */
    Tree makeLit(Type type, Object value) {
        if (type.tag == BOOLEAN) {
            return make.Ident(((Integer) value).intValue() == 0 ?
                    syms.falseConst : syms.trueConst);
        } else {
            return make.Literal(type.tag, value).setType(type.constType(value));
        }
    }

    /**
      * Make an attributed class instance creation expression.
      *  @param ctype    The class type.
      *  @param args     The constructor arguments.
      */
    NewClass makeNewClass(Type ctype, List args) {
        NewClass tree = make.NewClass(null, make.QualIdent(ctype.tsym), args, null);
        tree.constructor = rs.resolveConstructor(make.pos, attrEnv, ctype,
                TreeInfo.types(args));
        tree.type = ctype;
        return tree;
    }

    /**
      * Make an attributed unary expression.
      *  @param optag    The operators tree tag.
      *  @param arg      The operator's argument.
      */
    Tree makeUnary(int optag, Tree arg) {
        Unary tree = make.Unary(optag, arg);
        tree.operator = rs.resolveUnaryOperator(make.pos, optag, attrEnv, arg.type);
        tree.type = tree.operator.type.restype();
        return tree;
    }

    /**
      * Make an attributed binary expression.
      *  @param optag    The operators tree tag.
      *  @param lhs      The operator's left argument.
      *  @param rhs      The operator's right argument.
      */
    Binary makeBinary(int optag, Tree lhs, Tree rhs) {
        Binary tree = make.Binary(optag, lhs, rhs);
        tree.operator = rs.resolveBinaryOperator(make.pos, optag, attrEnv, lhs.type,
                rhs.type);
        tree.type = tree.operator.type.restype();
        return tree;
    }

    /**
      * Convert tree into string object, unless it has already a
      *  reference type..
      */
    Tree makeString(Tree tree) {
        if (tree.type.tag >= CLASS) {
            return tree;
        } else {
            Symbol valueOfSym =
                    rs.resolveInternalMethod(tree.pos, attrEnv, syms.stringType,
                    names.valueOf, Type.emptyList.prepend(tree.type));
            return make.App(make.QualIdent(valueOfSym), List.make(tree));
        }
    }

    /**
      * Create an empty anonymous class definition and enter and complete
      *  its symbol. Return the class definition's symbol.
      *  and create
      *  @param flags    The class symbol's flags
      *  @param owner    The class symbol's owner
      */
    ClassSymbol makeEmptyClass(long flags, ClassSymbol owner) {
        ClassSymbol c = reader.defineClass(names.empty, owner);
        c.flatname = chk.localClassName(c);
        c.sourcefile = owner.sourcefile;
        c.completer = null;
        c.members_field = new Scope(c);
        c.flags_field = flags;
        ClassType ctype = (ClassType) c.type;
        ctype.supertype_field = syms.objectType;
        ctype.interfaces_field = Type.emptyList;
        ClassDef odef = classDef(owner);
        enterSynthetic(odef.pos, c, owner.members());
        chk.compiled.put(c.flatname, c);
        ClassDef cdef =
                make.ClassDef(flags, names.empty, TypeParameter.emptyList, null,
                Tree.emptyList, Tree.emptyList);
        cdef.sym = c;
        cdef.type = c.type;
        odef.defs = odef.defs.prepend(cdef);
        return c;
    }

    /**
      * Report a conflict between a user symbol and a synthetic symbol.
      */
    private void duplicateError(int pos, Symbol sym) {
        if (!sym.type.isErroneous()) {
            log.error(pos, "synthetic.name.conflict", sym.toJava(),
                    sym.javaLocation());
        }
    }

    /**
      * Enter a synthetic symbol in a given scope, but complain if there was already one there.
      *  @param pos           Position for error reporting.
      *  @param sym           The symbol.
      *  @param s             The scope.
      */
    private void enterSynthetic(int pos, Symbol sym, Scope s) {
        if (sym.name != names.error && sym.name != names.empty) {
            for (Scope.Entry e = s.lookup(sym.name); e.scope == s; e = e.next()) {
                if (sym != e.sym && sym.kind == e.sym.kind) {
                    if ((sym.kind & (MTH | VAR)) != 0 &&
                            !sym.type.erasure().equals(e.sym.type.erasure()))
                        continue;
                    duplicateError(pos, e.sym);
                    break;
                }
            }
        }
        s.enter(sym);
    }

    /**
      * Look up a synthetic name in a given scope.
      *  @param scope	    The scope.
      *  @param name	    The name.
      */
    private Symbol lookupSynthetic(Name name, Scope s) {
        Symbol sym = s.lookup(name).sym;
        return (sym == null || (sym.flags() & SYNTHETIC) == 0) ? null : sym;
    }

    /**
      * Access codes for dereferencing, assignment,
      *  and pre/post increment/decrement.
      *  Access codes for assignment operations are determined by method accessCode
      *  below.
      *
      *  All access codes for accesses to the current class are even.
      *  If a member of the superclass should be accessed instead (because
      *  access was via a qualified super), add one to the corresponding code
      *  for the current class, making the number odd.
      *  This numbering scheme is used by the backend to decide whether
      *  to issue an invokevirtual or invokespecial call.
      *
      *  @see Gen.visitSelect(Select tree)
      */
    private static final int DEREFcode = 0;

    /**
     * Access codes for dereferencing, assignment,
     *  and pre/post increment/decrement.
     *  Access codes for assignment operations are determined by method accessCode
     *  below.
     *
     *  All access codes for accesses to the current class are even.
     *  If a member of the superclass should be accessed instead (because
     *  access was via a qualified super), add one to the corresponding code
     *  for the current class, making the number odd.
     *  This numbering scheme is used by the backend to decide whether
     *  to issue an invokevirtual or invokespecial call.
     *
     *  @see Gen.visitSelect(Select tree)
     */
    private static final int ASSIGNcode = 2;

    /**
     * Access codes for dereferencing, assignment,
     *  and pre/post increment/decrement.
     *  Access codes for assignment operations are determined by method accessCode
     *  below.
     *
     *  All access codes for accesses to the current class are even.
     *  If a member of the superclass should be accessed instead (because
     *  access was via a qualified super), add one to the corresponding code
     *  for the current class, making the number odd.
     *  This numbering scheme is used by the backend to decide whether
     *  to issue an invokevirtual or invokespecial call.
     *
     *  @see Gen.visitSelect(Select tree)
     */
    private static final int PREINCcode = 4;

    /**
     * Access codes for dereferencing, assignment,
     *  and pre/post increment/decrement.
     *  Access codes for assignment operations are determined by method accessCode
     *  below.
     *
     *  All access codes for accesses to the current class are even.
     *  If a member of the superclass should be accessed instead (because
     *  access was via a qualified super), add one to the corresponding code
     *  for the current class, making the number odd.
     *  This numbering scheme is used by the backend to decide whether
     *  to issue an invokevirtual or invokespecial call.
     *
     *  @see Gen.visitSelect(Select tree)
     */
    private static final int PREDECcode = 6;

    /**
     * Access codes for dereferencing, assignment,
     *  and pre/post increment/decrement.
     *  Access codes for assignment operations are determined by method accessCode
     *  below.
     *
     *  All access codes for accesses to the current class are even.
     *  If a member of the superclass should be accessed instead (because
     *  access was via a qualified super), add one to the corresponding code
     *  for the current class, making the number odd.
     *  This numbering scheme is used by the backend to decide whether
     *  to issue an invokevirtual or invokespecial call.
     *
     *  @see Gen.visitSelect(Select tree)
     */
    private static final int POSTINCcode = 8;

    /**
     * Access codes for dereferencing, assignment,
     *  and pre/post increment/decrement.
     *  Access codes for assignment operations are determined by method accessCode
     *  below.
     *
     *  All access codes for accesses to the current class are even.
     *  If a member of the superclass should be accessed instead (because
     *  access was via a qualified super), add one to the corresponding code
     *  for the current class, making the number odd.
     *  This numbering scheme is used by the backend to decide whether
     *  to issue an invokevirtual or invokespecial call.
     *
     *  @see Gen.visitSelect(Select tree)
     */
    private static final int POSTDECcode = 10;

    /**
     * Access codes for dereferencing, assignment,
     *  and pre/post increment/decrement.
     *  Access codes for assignment operations are determined by method accessCode
     *  below.
     *
     *  All access codes for accesses to the current class are even.
     *  If a member of the superclass should be accessed instead (because
     *  access was via a qualified super), add one to the corresponding code
     *  for the current class, making the number odd.
     *  This numbering scheme is used by the backend to decide whether
     *  to issue an invokevirtual or invokespecial call.
     *
     *  @see Gen.visitSelect(Select tree)
     */
    private static final int FIRSTASGOPcode = 12;

    /**
     * Number of access codes
     */
    private static final int NCODES = accessCode(ByteCodes.lushrl) + 2;

    /**
     * A mapping from symbols to their access numbers.
     */
    private Hashtable accessNums;

    /**
     * A mapping from symbols to an array of access symbols, indexed by
     *  access code.
     */
    private Hashtable accessSyms;

    /**
     * A mapping from (constructor) symbols to access constructor symbols.
     */
    private Hashtable accessConstrs;

    /**
     * A queue for all accessed symbols.
     */
    private ListBuffer accessed;

    /**
     * Map bytecode of binary operation to access code of corresponding
     *  assignment operation. This is always an even number.
     */
    private static int accessCode(int bytecode) {
        if (ByteCodes.iadd <= bytecode && bytecode <= ByteCodes.lxor)
            return (bytecode - iadd) * 2 + FIRSTASGOPcode;
        else if (bytecode == ByteCodes.string_add)
            return (ByteCodes.lxor + 1 - iadd) * 2 + FIRSTASGOPcode;
        else if (ByteCodes.ishll <= bytecode && bytecode <= ByteCodes.lushrl)
            return (bytecode - ishll + ByteCodes.lxor + 2 - iadd) * 2 +
                    FIRSTASGOPcode;
        else
            return -1;
    }

    /**
      * return access code for identifier,
      *  @param tree     The tree representing the identifier use.
      *  @param enclOp   The closest enclosing operation node of tree,
      *                  null if tree is not a subtree of an operation.
      */
    private static int accessCode(Tree tree, Tree enclOp) {
        if (enclOp == null)
            return DEREFcode;
        else if (enclOp.tag == Tree.ASSIGN &&
                tree == TreeInfo.skipParens(((Assign) enclOp).lhs))
            return ASSIGNcode;
        else if (Tree.PREINC <= enclOp.tag && enclOp.tag <= Tree.POSTDEC &&
                tree == TreeInfo.skipParens(((Unary) enclOp).arg))
            return (enclOp.tag - Tree.PREINC) * 2 + PREINCcode;
        else if (Tree.BITOR_ASG <= enclOp.tag && enclOp.tag <= Tree.MOD_ASG &&
                tree == TreeInfo.skipParens(((Assignop) enclOp).lhs))
            return accessCode(((OperatorSymbol)((Assignop) enclOp).operator).opcode);
        else
            return DEREFcode;
    }

    /**
      * Return binary operator that corresponds to given access code.
      */
    private OperatorSymbol binaryAccessOperator(int acode) {
        for (Scope.Entry e = syms.predefClass.members().elems; e != null;
                e = e.sibling) {
            if (e.sym instanceof OperatorSymbol) {
                OperatorSymbol op = (OperatorSymbol) e.sym;
                if (accessCode(op.opcode) == acode)
                    return op;
            }
        }
        return null;
    }

    /**
      * Return tree tag for assignment operation corresponding
      *  to given binary operator.
      */
    private static int treeTag(OperatorSymbol operator) {
        switch (operator.opcode) {
        case ByteCodes.ior:

        case ByteCodes.lor:
            return Tree.BITOR_ASG;

        case ByteCodes.ixor:

        case ByteCodes.lxor:
            return Tree.BITXOR_ASG;

        case ByteCodes.iand:

        case ByteCodes.land:
            return Tree.BITAND_ASG;

        case ByteCodes.ishl:

        case ByteCodes.lshl:

        case ByteCodes.ishll:

        case ByteCodes.lshll:
            return Tree.SL_ASG;

        case ByteCodes.ishr:

        case ByteCodes.lshr:

        case ByteCodes.ishrl:

        case ByteCodes.lshrl:
            return Tree.SR_ASG;

        case ByteCodes.iushr:

        case ByteCodes.lushr:

        case ByteCodes.iushrl:

        case ByteCodes.lushrl:
            return Tree.USR_ASG;

        case ByteCodes.iadd:

        case ByteCodes.ladd:

        case ByteCodes.fadd:

        case ByteCodes.dadd:

        case ByteCodes.string_add:
            return Tree.PLUS_ASG;

        case ByteCodes.isub:

        case ByteCodes.lsub:

        case ByteCodes.fsub:

        case ByteCodes.dsub:
            return Tree.MINUS_ASG;

        case ByteCodes.imul:

        case ByteCodes.lmul:

        case ByteCodes.fmul:

        case ByteCodes.dmul:
            return Tree.MUL_ASG;

        case ByteCodes.idiv:

        case ByteCodes.ldiv:

        case ByteCodes.fdiv:

        case ByteCodes.ddiv:
            return Tree.DIV_ASG;

        case ByteCodes.imod:

        case ByteCodes.lmod:

        case ByteCodes.fmod:

        case ByteCodes.dmod:
            return Tree.MOD_ASG;

        default:
            throw new AssertionError();

        }
    }

    /**
      * The name of the access method with number `anum' and access code `acode'.
      */
    Name accessName(int anum, int acode) {
        return names.fromString("access$" + anum + acode / 10 + acode % 10);
    }

    /**
      * Return access symbol for a private or protected symbol from an inner class.
      *  @param sym        The accessed private symbol.
      *  @param tree       The accessing tree.
      *  @param enclOp     The closest enclosing operation node of tree,
      *                    null if tree is not a subtree of an operation.
      *  @param protAccess Is access to a protected symbol in another
      *                    package?
      *  @param refSuper   Is access via a (qualified) C.super?
      */
    MethodSymbol accessSymbol(Symbol sym, Tree tree, Tree enclOp,
            boolean protAccess, boolean refSuper) {
        ClassSymbol accOwner = refSuper && protAccess ?
                (ClassSymbol)((Select) tree).selected.type.tsym :
                accessClass(sym, protAccess);
        Symbol vsym = sym;
        if (sym.owner != accOwner) {
            vsym = sym.clone(accOwner);
            actualSymbols.put(vsym, sym);
        }
        Integer anum = (Integer) accessNums.get(vsym);
        if (anum == null) {
            anum = new Integer(accessed.length());
            accessNums.put(vsym, anum);
            accessSyms.put(vsym, new MethodSymbol[NCODES]);
            accessed.append(vsym);
        }
        int acode;
        List argtypes;
        Type restype;
        List thrown;
        switch (vsym.kind) {
        case VAR:
            acode = accessCode(tree, enclOp);
            if (acode >= FIRSTASGOPcode) {
                OperatorSymbol operator = binaryAccessOperator(acode);
                if (operator.opcode == string_add)
                    argtypes = List.make(syms.objectType);
                else
                    argtypes = operator.type.argtypes().tail;
            } else if (acode == ASSIGNcode)
                argtypes = Type.emptyList.prepend(vsym.erasure());
            else
                argtypes = Type.emptyList;
            restype = vsym.erasure();
            thrown = Type.emptyList;
            break;

        case MTH:
            acode = DEREFcode;
            argtypes = vsym.erasure().argtypes();
            restype = vsym.erasure().restype();
            thrown = vsym.type.thrown();
            break;

        default:
            throw new AssertionError();

        }
        if (protAccess && refSuper)
            acode++;
        if ((vsym.flags() & STATIC) == 0) {
            argtypes = argtypes.prepend(vsym.owner.erasure());
        }
        MethodSymbol[] accessors = (Symbol.MethodSymbol[]) accessSyms.get(vsym);
        MethodSymbol accessor = accessors[acode];
        if (accessor == null) {
            accessor = new MethodSymbol(STATIC | SYNTHETIC,
                    accessName(anum.intValue(), acode),
                    new MethodType(argtypes, restype, thrown, syms.methodClass),
                    accOwner);
            enterSynthetic(tree.pos, accessor, accOwner.members());
            accessors[acode] = accessor;
        }
        return accessor;
    }

    /**
      * The qualifier to be used for accessing a symbol in an outer class.
      *  This is either C.sym or C.this.sym, depending on whether or not
      *  sym is static.
      *  @param sym   The accessed symbol.
      */
    Tree accessBase(int pos, Symbol sym) {
        return (sym.flags() & STATIC) != 0 ?
                access(make.at(pos).QualIdent(sym.owner)) :
                makeOwnerThis(pos, sym, true);
    }

    /**
      * Do we need an access method to reference private symbol?
      */
    boolean needsPrivateAccess(Symbol sym) {
        if ((sym.flags() & PRIVATE) == 0 || sym.owner == currentClass) {
            return false;
        } else if (sym.name == names.init &&
                (sym.owner.owner.kind & (VAR | MTH)) != 0) {
            sym.flags_field &= ~PRIVATE;
            return false;
        } else {
            return true;
        }
    }

    /**
      * Do we need an access method to reference symbol in other package?
      */
    boolean needsProtectedAccess(Symbol sym) {
        return (sym.flags() & PROTECTED) != 0 &&
                sym.owner.owner != currentClass.owner &&
                sym.packge() != currentClass.packge() &&
                !currentClass.isSubClass(sym.owner);
    }

    /**
      * The class in which an access method for given symbol goes.
      *  @param sym        The access symbol
      *  @param protAccess Is access to a protected symbol in another
      *                    package?
      */
    ClassSymbol accessClass(Symbol sym, boolean protAccess) {
        if (protAccess) {
            ClassSymbol c = currentClass;
            while (!c.isSubClass(sym.owner)) {
                c = c.owner.enclClass();
            }
            return c;
        } else {
            return sym.owner.enclClass();
        }
    }

    /**
      * Ensure that identifier is accessible, return tree accessing the identifier.
      *  @param sym      The accessed symbol.
      *  @param tree     The tree referring to the symbol.
      *  @param enclOp   The closest enclosing operation node of tree,
      *                  null if tree is not a subtree of an operation.
      *  @param refSuper Is access via a (qualified) C.super?
      */
    Tree access(Symbol sym, Tree tree, Tree enclOp, boolean refSuper) {
        while (sym.kind == VAR && sym.owner.kind == MTH &&
                sym.owner.enclClass() != currentClass) {
            Object cv = ((VarSymbol) sym).constValue;
            if (cv != null) {
                make.at(tree.pos);
                return makeLit(sym.type, cv);
            }
            sym = proxies.lookup(proxyName(sym.name)).sym;
            assert sym != null && (sym.flags_field & FINAL) != 0;
            tree = make.at(tree.pos).Ident(sym);
        }
        Tree base = (tree.tag == Tree.SELECT) ? ((Select) tree).selected : null;
        switch (sym.kind) {
        case TYP:
            if (sym.owner.kind != PCK) {
                Name flatname = Convert.shortName(sym.flatName());
                while (base != null && TreeInfo.symbol(base) != null &&
                        TreeInfo.symbol(base).kind != PCK) {
                    base = (base.tag == Tree.SELECT) ? ((Select) base).selected :
                            null;
                }
                if (tree.tag == Tree.IDENT) {
                    ((Ident) tree).name = flatname;
                } else if (base == null) {
                    tree = make.at(tree.pos).Ident(sym);
                    ((Ident) tree).name = flatname;
                } else {
                    ((Select) tree).selected = base;
                    ((Select) tree).name = flatname;
                }
            }
            break;

        case MTH:

        case VAR:
            if (sym.owner.kind == TYP) {
                boolean protAccess = refSuper && !needsPrivateAccess(sym) ||
                        needsProtectedAccess(sym);
                boolean accReq = protAccess || needsPrivateAccess(sym);
                boolean baseReq = base == null && sym.owner != syms.predefClass &&
                        !sym.isMemberOf(currentClass);
                if (accReq || baseReq) {
                    make.at(tree.pos);
                    if (sym.kind == VAR) {
                        Object cv = ((VarSymbol) sym).constValue;
                        if (cv != null)
                            return makeLit(sym.type, cv);
                    }
                    if (accReq) {
                        List args = Tree.emptyList;
                        if ((sym.flags() & STATIC) == 0) {
                            if (base == null)
                                base = makeOwnerThis(tree.pos, sym, true);
                            args = args.prepend(base);
                            base = null;
                        }
                        Symbol access = accessSymbol(sym, tree, enclOp, protAccess,
                                refSuper);
                        Tree receiver = make.Select(base != null ? base :
                                make.QualIdent(access.owner), access);
                        return make.App(receiver, args);
                    } else if (baseReq) {
                        return make.at(tree.pos).Select(
                                accessBase(tree.pos, sym), sym);
                    }
                }
            }

        }
        return tree;
    }

    /**
      * Ensure that identifier is accessible, return tree accessing the identifier.
      *  @param tree     The identifier tree.
      */
    Tree access(Tree tree) {
        Symbol sym = TreeInfo.symbol(tree);
        return sym == null ? tree : access(sym, tree, null, false);
    }

    /**
      * Return access constructor for a private constructor,
      *  or the constructor itself, if no access constructor is needed.
      *  @param pos	 The position to report diagnostics, if any.
      *  @param constr    The private constructor.
      */
    Symbol accessConstructor(int pos, Symbol constr) {
        if (needsPrivateAccess(constr)) {
            ClassSymbol accOwner = accessClass(constr, false);
            MethodSymbol aconstr = (Symbol.MethodSymbol) accessConstrs.get(constr);
            if (aconstr == null) {
                aconstr = new MethodSymbol(SYNTHETIC, names.init,
                        new MethodType( constr.type.argtypes().append(
                        accessConstructorTag().type), constr.type.restype(),
                        constr.type.thrown(), syms.methodClass), accOwner);
                enterSynthetic(pos, aconstr, accOwner.members());
                accessConstrs.put(constr, aconstr);
                accessed.append(constr);
            }
            return aconstr;
        } else {
            return constr;
        }
    }

    /**
      * Return an anonymous class nested in this toplevel class.
      */
    ClassSymbol accessConstructorTag() {
        ClassSymbol topClass = currentClass.outermostClass();
        Name flatname = names.fromString(topClass.fullName() + "$1");
        ClassSymbol ctag = (Symbol.ClassSymbol) chk.compiled.get(flatname);
        if (ctag == null)
            ctag = makeEmptyClass(STATIC | SYNTHETIC, topClass);
        return ctag;
    }

    /**
      * Add all required access methods for a private symbol to enclosing class.
      *  @param sym       The symbol.
      */
    void makeAccessible(Symbol sym) {
        ClassDef cdef = classDef(sym.owner.enclClass());
        assert cdef != null :
        "class def not found: " + sym + " in " + sym.owner;
        if (sym.name == names.init) {
            cdef.defs = cdef.defs.prepend( accessConstructorDef(cdef.pos, sym,
                    (Symbol.MethodSymbol) accessConstrs.get(sym)));
        } else {
            MethodSymbol[] accessors = (Symbol.MethodSymbol[]) accessSyms.get(sym);
            for (int i = 0; i < NCODES; i++) {
                if (accessors[i] != null)
                    cdef.defs = cdef.defs.prepend(
                            accessDef(cdef.pos, sym, accessors[i], i));
            }
        }
    }

    /**
      * Construct definition of an access method.
      *  @param pos        The source code position of the definition.
      *  @param sym        The private or protected symbol.
      *  @param accessor   The access method for the symbol.
      *  @param acode      The access code.
      */
    Tree accessDef(int pos, Symbol vsym, MethodSymbol accessor, int acode) {
        currentClass = vsym.owner.enclClass();
        make.at(pos);
        MethodDef md = make.MethodDef(accessor, null);
        Symbol sym = (Symbol) actualSymbols.get(vsym);
        if (sym == null)
            sym = vsym;
        Tree ref;
        List args;
        if ((sym.flags() & STATIC) != 0) {
            ref = make.Ident(sym);
            args = make.Idents(md.params);
        } else {
            ref = make.Select(make.Ident((Tree.VarDef) md.params.head), sym);
            args = make.Idents(md.params.tail);
        }
        Tree stat;
        if (sym.kind == VAR) {
            int acode1 = acode - (acode & 1);
            Tree expr;
            switch (acode1) {
            case DEREFcode:
                expr = ref;
                break;

            case ASSIGNcode:
                expr = make.Assign(ref, (Tree) args.head);
                break;

            case PREINCcode:

            case POSTINCcode:

            case PREDECcode:

            case POSTDECcode:
                expr = makeUnary(((acode1 - PREINCcode)>> 1) + Tree.PREINC, ref);
                break;

            default:
                expr = make.Assignop(treeTag(binaryAccessOperator(acode1)), ref,
                        (Tree) args.head);
                ((Assignop) expr).operator = binaryAccessOperator(acode1);

            }
            stat = make.Return(expr.setType(sym.type));
        } else {
            stat = make.Call(make.App(ref, args));
        }
        md.body = make.Block(0, List.make(stat));
        for (List l = md.params; l.nonEmpty(); l = l.tail)
            ((Tree.VarDef) l.head).vartype = access(((Tree.VarDef) l.head).vartype);
        md.restype = access(md.restype);
        for (List l = md.thrown; l.nonEmpty(); l = l.tail)
            l.head = access((Tree) l.head);
        return md;
    }

    /**
      * Construct definition of an access constructor.
      *  @param pos        The source code position of the definition.
      *  @param constr     The private constructor.
      *  @param accessor   The access method for the constructor.
      */
    Tree accessConstructorDef(int pos, Symbol constr, MethodSymbol accessor) {
        make.at(pos);
        MethodDef md = make.MethodDef(accessor, accessor.externalType(), null);
        Ident callee = make.Ident(names._this);
        callee.sym = constr;
        callee.type = constr.type;
        md.body = make.Block(0,
                List.make( make.Call(
                make.App(callee, make.Idents(md.params.reverse().tail.reverse())))));
        return md;
    }

    /**
      * A scope containing all free variable proxies for currently translated
      *  class, as well as its this$n symbol (if needed).
      *  Proxy scopes are nested in the same way classes are.
      *  Inside a constructor, proxies and any this$n symbol are duplicated
      *  in an additional innermost scope, where they represent the constructor
      *  parameters.
      */
    Scope proxies;

    /**
     * A stack containing the this$n field of the currently translated
     *  classes (if needed) in innermost first order.
     *  Inside a constructor, proxies and any this$n symbol are duplicated
     *  in an additional innermost scope, where they represent the constructor
     *  parameters.
     */
    List outerThisStack;

    /**
     * The name of a free variable proxy.
     */
    Name proxyName(Name name) {
        return names.fromString("val$" + name);
    }

    /**
      * Proxy definitions for all free variables in given list, in reverse order.
      *  @param pos        The source code position of the definition.
      *  @param freevars   The free variables.
      *  @param owner      The class in which the definitions go.
      */
    List freevarDefs(int pos, List freevars, Symbol owner) {
        long flags = FINAL | SYNTHETIC;
        if (owner.kind == TYP && target.ordinal < Target.JDK1_4_2.ordinal)
            flags |= PRIVATE;
        List defs = VarDef.emptyList;
        for (List l = freevars; l.nonEmpty(); l = l.tail) {
            VarSymbol v = (Symbol.VarSymbol) l.head;
            VarSymbol proxy =
                    new VarSymbol(flags, proxyName(v.name), v.erasure(), owner);
            proxies.enter(proxy);
            VarDef vd = make.at(pos).VarDef(proxy, null);
            vd.vartype = access(vd.vartype);
            defs = defs.prepend(vd);
        }
        return defs;
    }

    /**
      * The name of a this$n field
      *  @param target   The class referenced by the this$n field
      */
    Name outerThisName(Type target) {
        Type t = target.outer();
        int nestingLevel = 0;
        while (t.tag == CLASS) {
            t = t.outer();
            nestingLevel++;
        }
        return names.fromString("this$" + nestingLevel);
    }

    /**
      * Definition for this$n field.
      *  @param pos        The source code position of the definition.
      *  @param owner      The class in which the definition goes.
      */
    VarDef outerThisDef(int pos, Symbol owner) {
        long flags = FINAL | SYNTHETIC;
        if (owner.kind == TYP && target.ordinal < Target.JDK1_4_2.ordinal)
            flags |= PRIVATE;
        Type target = owner.enclClass().type.outer().erasure();
        VarSymbol outerThis =
                new VarSymbol(flags, outerThisName(target), target, owner);
        outerThisStack = outerThisStack.prepend(outerThis);
        VarDef vd = make.at(pos).VarDef(outerThis, null);
        vd.vartype = access(vd.vartype);
        return vd;
    }

    /**
      * Return a list of trees that load the free variables in given list,
      *  in reverse order.
      *  @param pos          The source code position to be used for the trees.
      *  @param freevars     The list of free variables.
      */
    List loadFreevars(int pos, List freevars) {
        List args = Tree.emptyList;
        for (List l = freevars; l.nonEmpty(); l = l.tail)
            args = args.prepend(loadFreevar(pos, (Symbol.VarSymbol) l.head));
        return args;
    }

    Tree loadFreevar(int pos, VarSymbol v) {
        return access(v, make.at(pos).Ident(v), null, false);
    }

    /**
      * Construct a tree simulating the expression <C.this>.
      *  @param pos           The source code position to be used for the tree.
      *  @param c             The qualifier class.
      */
    Tree makeThis(int pos, TypeSymbol c) {
        if (currentClass == c) {
            return make.at(pos).This(c.erasure());
        } else {
            return makeOuterThis(pos, c);
        }
    }

    /**
      * Construct a tree that represents the outer instance
      *  <C.this>. Never pick the current `this'.
      *  @param pos           The source code position to be used for the tree.
      *  @param c             The qualifier class.
      */
    Tree makeOuterThis(int pos, TypeSymbol c) {
        List ots = outerThisStack;
        if (ots.isEmpty()) {
            log.error(pos, "no.encl.instance.of.type.in.scope", c.toJava());
            assert false;
            return make.Ident(syms.nullConst);
        }
        VarSymbol ot = (Symbol.VarSymbol) ots.head;
        Tree tree = access(make.at(pos).Ident(ot));
        TypeSymbol otc = ot.type.tsym;
        while (otc != c) {
            do {
                ots = ots.tail;
                if (ots.isEmpty()) {
                    log.error(pos, "no.encl.instance.of.type.in.scope", c.toJava());
                    assert false;
                    return tree;
                }
                ot = (Symbol.VarSymbol) ots.head;
            } while (ot.owner != otc)
                ;
            if (otc.owner.kind != PCK && !otc.hasOuterInstance()) {
                chk.earlyRefError(pos, c);
                assert false;
                return make.Ident(syms.nullConst);
            }
            tree = access(make.at(pos).Select(tree, ot));
            otc = ot.type.tsym;
        }
        return tree;
    }

    /**
      * Construct a tree that represents the closest outer instance
      *  <C.this> such that the given symbol is a member of C.
      *  @param pos           The source code position to be used for the tree.
      *  @param sym           The accessed symbol.
      *  @param preciseMatch  should we accept a type that is a subtype of
      *                       sym's owner, even if it doesn't contain sym
      *                       due to hiding, overriding, or non-inheritance
      *                       due to protection?
      */
    Tree makeOwnerThis(int pos, Symbol sym, boolean preciseMatch) {
        Symbol c = sym.owner;
        if (preciseMatch ? sym.isMemberOf(currentClass) :
                currentClass.isSubClass(sym.owner)) {
            return make.at(pos).This(c.erasure());
        } else {
            List ots = outerThisStack;
            if (ots.isEmpty()) {
                log.error(pos, "no.encl.instance.of.type.in.scope", c.toJava());
                assert false;
                return make.Ident(syms.nullConst);
            }
            VarSymbol ot = (Symbol.VarSymbol) ots.head;
            Tree tree = access(make.at(pos).Ident(ot));
            TypeSymbol otc = ot.type.tsym;
            while (!(preciseMatch ? sym.isMemberOf(otc) :
                    otc.isSubClass(sym.owner))) {
                do {
                    ots = ots.tail;
                    if (ots.isEmpty()) {
                        log.error(pos, "no.encl.instance.of.type.in.scope",
                                c.toJava());
                        assert false;
                        return tree;
                    }
                    ot = (Symbol.VarSymbol) ots.head;
                } while (ot.owner != otc)
                    ;
                tree = access(make.at(pos).Select(tree, ot));
                otc = ot.type.tsym;
            }
            return tree;
        }
    }

    /**
      * Return tree simulating the the assignment <this.name = name>, where
      *  name is the name of a free variable.
      */
    Tree initField(int pos, Name name) {
        Scope.Entry e = proxies.lookup(name);
        Symbol rhs = e.sym;
        assert rhs.owner.kind == MTH;
        Symbol lhs = e.next().sym;
        assert rhs.owner.owner == lhs.owner;
        make.at(pos);
        return make.Exec(
                make.Assign(make.Select(make.This(lhs.owner.erasure()), lhs),
                make.Ident(rhs)).setType(lhs.erasure()));
    }

    /**
      * Return tree simulating the the assignment <this.this$n = this$n>.
      */
    Tree initOuterThis(int pos) {
        VarSymbol rhs = (Symbol.VarSymbol) outerThisStack.head;
        assert rhs.owner.kind == MTH;
        VarSymbol lhs = (Symbol.VarSymbol) outerThisStack.tail.head;
        assert rhs.owner.owner == lhs.owner;
        make.at(pos);
        return make.Exec(
                make.Assign(make.Select(make.This(lhs.owner.erasure()), lhs),
                make.Ident(rhs)).setType(lhs.erasure()));
    }

    /**
      * Return the symbol of a class to contain a cache of
      *  compiler-generated statics such as class$ and the
      *  $assertionsDisabled flag.  We create an anonymous nested class
      *  unless one exists and return its symbol.  However, for
      *  backward compatibility in 1.4 and earlier we use the
      *  top-level class.
      */
    private ClassSymbol outerCacheClass() {
        ClassSymbol clazz = outermostClassDef.sym;
        if ((clazz.flags() & INTERFACE) == 0 &&
                target.ordinal < Target.JDK1_4_2.ordinal)
            return clazz;
        Scope s = clazz.members();
        for (Scope.Entry e = s.elems; e != null; e = e.sibling)
            if (e.sym.kind == TYP && e.sym.name == names.empty &&
                    (e.sym.flags() & INTERFACE) == 0)
                return (ClassSymbol) e.sym;
        return makeEmptyClass(STATIC | SYNTHETIC, clazz);
    }

    /**
      * Return symbol for "class$" method. If there is no method definition
      *  for class$, construct one as follows:
      *
      *    class class$(String x0) {
      *      try {
      *        return Class.forName(x0);
      *      } catch (ClassNotFoundException x1) {
      *        throw new NoClassDefFoundError(x1.getMessage());
      *      }
      *    }
      */
    private MethodSymbol classDollarSym(int pos) {
        ClassSymbol outerCacheClass = outerCacheClass();
        MethodSymbol classDollarSym =
                (MethodSymbol) lookupSynthetic(names.classDollar,
                outerCacheClass.members());
        if (classDollarSym == null) {
            classDollarSym = new MethodSymbol(STATIC | SYNTHETIC, names.classDollar,
                    new MethodType(Type.emptyList.prepend(syms.stringType),
                    syms.classType, Type.emptyList, syms.methodClass),
                    outerCacheClass);
            enterSynthetic(pos, classDollarSym, outerCacheClass.members());
            MethodDef md = make.MethodDef(classDollarSym, null);
            try {
                md.body = classDollarSymBody(pos, md);
            } catch (CompletionFailure ex) {
                md.body = make.Block(0, Tree.emptyList);
                chk.completionError(pos, ex);
            }
            ClassDef outerCacheClassDef = classDef(outerCacheClass);
            outerCacheClassDef.defs = outerCacheClassDef.defs.prepend(md);
        }
        return classDollarSym;
    }

    /**
      * Generate code for class$(String name).
      */
    Block classDollarSymBody(int pos, MethodDef md) {
        MethodSymbol classDollarSym = md.sym;
        ClassSymbol outerCacheClass = (ClassSymbol) classDollarSym.owner;
        Tree returnResult;
        if (target.ordinal >= Target.JDK1_4_2.ordinal) {
            VarSymbol clsym =
                    new VarSymbol(STATIC | SYNTHETIC, names.fromString("cl$"),
                    syms.classLoaderType, outerCacheClass);
            enterSynthetic(pos, clsym, outerCacheClass.members());
            VarDef cldef = make.VarDef(clsym, null);
            ClassDef outerCacheClassDef = classDef(outerCacheClass);
            outerCacheClassDef.defs = outerCacheClassDef.defs.prepend(cldef);
            Tree newcache = make.NewArray(make.Type(outerCacheClass.type),
                    Tree.emptyList.prepend(
                    make.Literal(Type.INT, new Integer(0)).setType(syms.intType))
                    , null);
            newcache.type = new ArrayType(outerCacheClass.type, syms.arrayClass);
            Symbol forNameSym =
                    rs.resolveInternalMethod(make.pos, attrEnv, syms.classType,
                    names.forName,
                    Type.emptyList.prepend(syms.classLoaderType).prepend(
                    syms.booleanType).prepend(syms.stringType));
            Tree clvalue = make.Conditional( makeBinary(Tree.EQ, make.Ident(clsym),
                    make.Ident(syms.nullConst)),
                    make.Assign(make.Ident(clsym),
                    makeCall(
                    makeCall(makeCall(newcache, names.getClass, Tree.emptyList),
                    names.getComponentType, Tree.emptyList),
                    names.getClassLoader, Tree.emptyList)).setType(
                    syms.classLoaderType),
                    make.Ident(clsym)).setType(syms.classLoaderType);
            List args = Tree.emptyList.prepend(clvalue).prepend(
                    make.Ident(syms.falseConst)).prepend(
                    make.Ident(((Tree.VarDef) md.params.head).sym));
            returnResult = make.Block(0,
                    Tree.emptyList.prepend(
                    make.Call(make.App(make.Ident(forNameSym), args))));
        } else {
            Symbol forNameSym =
                    rs.resolveInternalMethod(make.pos, attrEnv, syms.classType,
                    names.forName, Type.emptyList.prepend(syms.stringType));
            returnResult = make.Block(0,
                    Tree.emptyList.prepend( make.Call(
                    make.App(make.QualIdent(forNameSym),
                    List.make(make.Ident(((Tree.VarDef) md.params.head).sym))))));
        }
        VarSymbol catchParam = new VarSymbol(0, make.paramName(1),
                syms.classNotFoundExceptionType, classDollarSym);
        Tree rethrow;
        if (target.ordinal >= Target.JDK1_4.ordinal) {
            Tree throwExpr = makeCall(
                    makeNewClass(syms.noClassDefFoundErrorType, Tree.emptyList),
                    names.initCause, Tree.emptyList.prepend(make.Ident(catchParam)));
            rethrow = make.Throw(throwExpr);
        } else {
            Symbol getMessageSym = rs.resolveInternalMethod(make.pos, attrEnv,
                    syms.classNotFoundExceptionType, names.getMessage,
                    Type.emptyList);
            rethrow = make.Throw( makeNewClass(syms.noClassDefFoundErrorType,
                    List.make(
                    make.App(make.Select(make.Ident(catchParam), getMessageSym),
                    Tree.emptyList))));
        }
        Tree rethrowStmt = make.Block(0, Tree.emptyList.prepend(rethrow));
        Catch catchBlock = make.Catch(make.VarDef(catchParam, null), rethrowStmt);
        Tree tryCatch =
                make.Try(returnResult, Catch.emptyList.prepend(catchBlock), null);
        return make.Block(0, Tree.emptyList.prepend(tryCatch));
    }

    /**
      * Create an attributed tree of the form left.name().
      */
    private Tree makeCall(Tree left, Name name, List args) {
        assert left.type != null;
        List types = Type.emptyList;
        Symbol funcsym = rs.resolveInternalMethod(make.pos, attrEnv, left.type, name,
                TreeInfo.types(args));
        return make.App(make.Select(left, funcsym), args);
    }

    /**
      * The Name Of The variable to cache T.class values.
      *  @param sig      The signature of type T.
      */
    private Name cacheName(String sig) {
        StringBuffer buf = new StringBuffer();
        if (sig.startsWith("[")) {
            buf = buf.append("array");
            while (sig.startsWith("[")) {
                buf = buf.append("$");
                sig = sig.substring(1);
            }
            if (sig.startsWith("L")) {
                sig = sig.substring(0, sig.length() - 1);
            }
        } else {
            buf = buf.append("class$");
        }
        buf = buf.append(sig.replace('.', '$'));
        return names.fromString(buf.toString());
    }

    /**
      * The variable symbol that caches T.class values.
      *  If none exists yet, create a definition.
      *  @param sig      The signature of type T.
      *  @param pos	The position to report diagnostics, if any.
      */
    private VarSymbol cacheSym(int pos, String sig) {
        ClassSymbol outerCacheClass = outerCacheClass();
        Name cname = cacheName(sig);
        VarSymbol cacheSym =
                (VarSymbol) lookupSynthetic(cname, outerCacheClass.members());
        if (cacheSym == null) {
            cacheSym = new VarSymbol(STATIC | SYNTHETIC, cname, syms.classType,
                    outerCacheClass);
            enterSynthetic(pos, cacheSym, outerCacheClass.members());
            VarDef cacheDef = make.VarDef(cacheSym, null);
            ClassDef outerCacheClassDef = classDef(outerCacheClass);
            outerCacheClassDef.defs = outerCacheClassDef.defs.prepend(cacheDef);
        }
        return cacheSym;
    }

    /**
      * The tree simulating a T.class expression.
      *  @param clazz      The tree identifying type T.
      */
    private Tree classOf(Tree clazz) {
        return classOfType(clazz.type, clazz.pos);
    }

    private Tree classOfType(Type type, int pos) {
        switch (type.tag) {
        case BYTE:

        case SHORT:

        case CHAR:

        case INT:

        case LONG:

        case FLOAT:

        case DOUBLE:

        case BOOLEAN:

        case VOID:
            Name bname = syms.boxedName[type.tag];
            ClassSymbol c = reader.enterClass(bname);
            Symbol typeSym = rs.access(
                    rs.findIdentInType(attrEnv, c.type, names.TYPE, VAR), pos,
                    c.type, names.TYPE, true);
            if (typeSym.kind == VAR)
                attr.evalInit((VarSymbol) typeSym);
            return make.QualIdent(typeSym);

        case CLASS:

        case ARRAY:
            String sig = writer.xClassName(type).toString().replace('/', '.');
            Symbol cs = cacheSym(pos, sig);
            return make.at(pos).Conditional( makeBinary(Tree.EQ, make.Ident(cs),
                    make.Ident(syms.nullConst)),
                    make.Assign(make.Ident(cs),
                    make.App(make.Ident(classDollarSym(pos)),
                    List.make(
                    make.Literal(Type.CLASS, sig).setType(syms.stringType)))).
                    setType(syms.classType), make.Ident(cs)).setType(syms.classType);

        default:
            throw new AssertionError();

        }
    }

    /**
      * Code for enabling/disabling assertions.
      */
    private Tree assertFlagTest(int pos) {
        ClassSymbol outermostClass = outermostClassDef.sym;
        ClassSymbol container = currentClass;
        VarSymbol assertDisabledSym =
                (VarSymbol) lookupSynthetic(names.dollarAssertionsDisabled,
                container.members());
        if (assertDisabledSym == null) {
            assertDisabledSym = new VarSymbol(STATIC | FINAL | SYNTHETIC,
                    names.dollarAssertionsDisabled, syms.booleanType, container);
            enterSynthetic(pos, assertDisabledSym, container.members());
            Symbol desiredAssertionStatusSym =
                    rs.resolveInternalMethod(pos, attrEnv, syms.classType,
                    names.desiredAssertionStatus, Type.emptyList);
            ClassDef containerDef = classDef(container);
            make.at(containerDef.pos);
            Tree notStatus = makeUnary(Tree.NOT,
                    make.App( make.Select(
                    classOfType(outermostClass.type, containerDef.pos),
                    desiredAssertionStatusSym), Tree.emptyList));
            VarDef assertDisabledDef = make.VarDef(assertDisabledSym, notStatus);
            containerDef.defs = containerDef.defs.prepend(assertDisabledDef);
        }
        make.at(pos);
        return makeUnary(Tree.NOT, make.Ident(assertDisabledSym));
    }

    /**
      * Visitor argument: enclosing operator node.
      */
    private Tree enclOp;

    /**
     * Visitor method: Translate a single node.
     *  Attach the source position from the old tree to its replacement tree.
     */
    public Tree translate(Tree tree) {
        if (tree == null) {
            return null;
        } else {
            make.at(tree.pos);
            tree.accept(this);
            if (endPositions != null && result != tree) {
                Integer endPos = (Integer) endPositions.remove(tree);
                if (endPos != null)
                    endPositions.put(result, endPos);
            }
            return result;
        }
    }

    /**
      * Visitor method: Translate tree.
      */
    public Tree translate(Tree tree, Tree enclOp) {
        Tree prevEnclOp = this.enclOp;
        this.enclOp = enclOp;
        Tree res = translate(tree);
        this.enclOp = prevEnclOp;
        return res;
    }

    /**
      * Visitor method: Translate list of trees.
      */
    public List translate(List trees, Tree enclOp) {
        Tree prevEnclOp = this.enclOp;
        this.enclOp = enclOp;
        List res = translate(trees);
        this.enclOp = prevEnclOp;
        return res;
    }

    public void visitClassDef(ClassDef tree) {
        ClassSymbol currentClassPrev = currentClass;
        currentClass = tree.sym;
        classdefs.put(currentClass, tree);
        proxies = proxies.dup();
        List prevOuterThisStack = outerThisStack;
        VarDef otdef = null;
        if (currentClass.hasOuterInstance())
            otdef = outerThisDef(tree.pos, currentClass);
        List fvdefs = freevarDefs(tree.pos, freevars(currentClass), currentClass);
        tree.extending = translate(tree.extending);
        tree.implementing = translate(tree.implementing);
        List seen = Tree.emptyList;
        while (tree.defs != seen) {
            List unseen = tree.defs;
            for (List l = unseen; l.nonEmpty() && l != seen; l = l.tail) {
                Tree outermostMemberDefPrev = outermostMemberDef;
                if (outermostMemberDefPrev == null)
                    outermostMemberDef = (Tree) l.head;
                l.head = translate((Tree) l.head);
                outermostMemberDef = outermostMemberDefPrev;
            }
            seen = unseen;
        }
        if ((tree.flags & PROTECTED) != 0)
            tree.flags |= PUBLIC;
        tree.flags &= ClassFlags;
        tree.name = Convert.shortName(currentClass.flatName());
        for (List l = fvdefs; l.nonEmpty(); l = l.tail) {
            tree.defs = tree.defs.prepend(l.head);
            enterSynthetic(tree.pos, ((Tree.VarDef) l.head).sym,
                    currentClass.members());
        }
        if (currentClass.hasOuterInstance()) {
            tree.defs = tree.defs.prepend(otdef);
            enterSynthetic(tree.pos, otdef.sym, currentClass.members());
        }
        proxies = proxies.leave();
        outerThisStack = prevOuterThisStack;
        translated.append(tree);
        currentClass = currentClassPrev;
        result = make.at(tree.pos).Block(0, Tree.emptyList);
    }

    public void visitMethodDef(MethodDef tree) {
        if (tree.name == names.init && (currentClass.isInner() ||
                (currentClass.owner.kind & (VAR | MTH)) != 0)) {
            MethodSymbol m = tree.sym;
            proxies = proxies.dup();
            List prevOuterThisStack = outerThisStack;
            List fvs = freevars(currentClass);
            VarDef otdef = null;
            if (currentClass.hasOuterInstance())
                otdef = outerThisDef(tree.pos, m);
            List fvdefs = freevarDefs(tree.pos, fvs, m);
            tree.restype = translate(tree.restype);
            tree.params = translateVarDefs(tree.params);
            tree.thrown = translate(tree.thrown);
            tree.params = tree.params.appendList(fvdefs);
            if (currentClass.hasOuterInstance())
                tree.params = tree.params.prepend(otdef);
            Tree selfCall = translate((Tree) tree.body.stats.head);
            List added = Tree.emptyList;
            if (fvs.nonEmpty()) {
                List addedargtypes = new List();
                for (List l = fvs; l.nonEmpty(); l = l.tail) {
                    if (TreeInfo.isInitialConstructor(tree))
                        added = added.prepend( initField(tree.body.pos,
                                proxyName(((Symbol.VarSymbol) l.head).name)));
                    addedargtypes = addedargtypes.prepend(
                            ((Symbol.VarSymbol) l.head).erasure());
                }
                Type olderasure = m.erasure();
                m.erasure_field = new MethodType(
                        olderasure.argtypes().appendList(addedargtypes),
                        olderasure.restype(), olderasure.thrown(), syms.methodClass);
            }
            if (currentClass.hasOuterInstance() &&
                    TreeInfo.isInitialConstructor(tree)) {
                added = added.prepend(initOuterThis(tree.body.pos));
            }
            proxies = proxies.leave();
            List stats = translate(tree.body.stats.tail);
            if (target.initializeFieldsBeforeSuper())
                tree.body.stats = stats.prepend(selfCall).prependList(added);
            else
                tree.body.stats = stats.prependList(added).prepend(selfCall);
            outerThisStack = prevOuterThisStack;
        } else {
            super.visitMethodDef(tree);
        }
        result = tree;
    }

    public void visitNewClass(NewClass tree) {
        ClassSymbol c = (ClassSymbol) tree.constructor.owner;
        tree.args = translate(tree.args);
        if ((c.owner.kind & (VAR | MTH)) != 0) {
            tree.args = tree.args.appendList(loadFreevars(tree.pos, freevars(c)));
        }
        Symbol constructor = accessConstructor(tree.pos, tree.constructor);
        if (constructor != tree.constructor) {
            tree.args = tree.args.append(make.Ident(syms.nullConst));
            tree.constructor = constructor;
        }
        if (c.hasOuterInstance()) {
            Tree thisArg;
            if (tree.encl != null) {
                thisArg = attr.makeNullCheck(translate(tree.encl));
                thisArg.type = tree.encl.type;
            } else if ((c.owner.kind & (MTH | VAR)) != 0) {
                thisArg = makeThis(tree.pos, c.type.outer().tsym);
            } else {
                thisArg = makeOwnerThis(tree.pos, c, false);
            }
            tree.args = tree.args.prepend(thisArg);
        }
        tree.encl = null;
        if (tree.def != null) {
            translate(tree.def);
            tree.clazz = access(make.at(tree.clazz.pos).Ident(tree.def.sym));
            tree.def = null;
        } else {
            tree.clazz = access(c, tree.clazz, enclOp, false);
        }
        result = tree;
    }

    /**
      * Visitor method for conditional expressions.
      */
    public void visitConditional(Conditional tree) {
        Tree cond = tree.cond = translate(tree.cond);
        if (cond.type.isTrue()) {
            result = convert(translate(tree.truepart), tree.type);
        } else if (cond.type.isFalse()) {
            result = convert(translate(tree.falsepart), tree.type);
        } else {
            tree.truepart = translate(tree.truepart);
            tree.falsepart = translate(tree.falsepart);
            result = tree;
        }
    }

    private Tree convert(Tree tree, Type pt) {
        if (tree.type == pt)
            return tree;
        Tree result = make.at(tree.pos).TypeCast(make.Type(pt), tree);
        result.type =
                (tree.type.constValue != null) ? cfolder.coerce(tree.type, pt) : pt;
        return result;
    }

    /**
      * Visitor method for if statements.
      */
    public void visitIf(If tree) {
        Tree cond = tree.cond = translate(tree.cond);
        if (cond.type.isTrue()) {
            result = translate(tree.thenpart);
        } else if (cond.type.isFalse()) {
            if (tree.elsepart != null) {
                result = translate(tree.elsepart);
            } else {
                result = make.Skip();
            }
        } else {
            tree.thenpart = translate(tree.thenpart);
            tree.elsepart = translate(tree.elsepart);
            result = tree;
        }
    }

    /**
      * Visitor method for assert statements. Translate them away.
      */
    public void visitAssert(Assert tree) {
        int detailPos = (tree.detail == null) ? tree.pos : tree.detail.pos;
        tree.cond = translate(tree.cond);
        if (!tree.cond.type.isTrue()) {
            Tree cond = assertFlagTest(tree.pos);
            List exnArgs = (tree.detail == null) ? Tree.emptyList :
                    List.make(translate(tree.detail));
            if (!tree.cond.type.isFalse()) {
                cond = makeBinary(Tree.AND, cond, makeUnary(Tree.NOT, tree.cond));
            }
            result = make.If(cond,
                    make.at(detailPos).Throw(
                    makeNewClass(syms.assertionErrorType, exnArgs)), null);
        } else {
            result = make.Skip();
        }
    }

    public void visitApply(Apply tree) {
        tree.args = translate(tree.args);
        Name methName = TreeInfo.name(tree.meth);
        if (methName == names._this || methName == names._super) {
            Symbol constructor =
                    accessConstructor(tree.pos, TreeInfo.symbol(tree.meth));
            if (constructor != TreeInfo.symbol(tree.meth)) {
                tree.args = tree.args.append(make.Ident(syms.nullConst));
                TreeInfo.setSymbol(tree.meth, constructor);
            }
            ClassSymbol c = (ClassSymbol) constructor.owner;
            if ((c.owner.kind & (VAR | MTH)) != 0) {
                tree.args = tree.args.appendList(loadFreevars(tree.pos, freevars(c)));
            }
            if (c.hasOuterInstance()) {
                Tree thisArg;
                if (tree.meth.tag == Tree.SELECT) {
                    thisArg = attr.makeNullCheck(
                            translate(((Select) tree.meth).selected));
                    tree.meth = make.Ident(constructor);
                    ((Ident) tree.meth).name = methName;
                } else if ((c.owner.kind & (MTH | VAR)) != 0) {
                    thisArg = makeThis(tree.meth.pos, c.type.outer().tsym);
                } else {
                    thisArg = makeOwnerThis(tree.meth.pos, c, false);
                }
                tree.args = tree.args.prepend(thisArg);
            }
        } else {
            tree.meth = translate(tree.meth);
            if (tree.meth.tag == Tree.APPLY) {
                Apply app = (Apply) tree.meth;
                app.args = tree.args.prependList(app.args);
                result = app;
                return;
            }
        }
        result = tree;
    }

    /**
      * Visitor method for parenthesized expressions.
      *  If the subexpression has changed, omit the parens.
      */
    public void visitParens(Parens tree) {
        Tree expr = translate(tree.expr);
        result = ((expr == tree.expr) ? tree : expr);
    }

    public void visitAssign(Assign tree) {
        tree.lhs = translate(tree.lhs, tree);
        tree.rhs = translate(tree.rhs);
        if (tree.lhs.tag == Tree.APPLY) {
            Apply app = (Apply) tree.lhs;
            app.args = List.make(tree.rhs).prependList(app.args);
            result = app;
        } else {
            result = tree;
        }
    }

    public void visitAssignop(Assignop tree) {
        tree.lhs = translate(tree.lhs, tree);
        tree.rhs = translate(tree.rhs);
        if (tree.lhs.tag == Tree.APPLY) {
            Apply app = (Apply) tree.lhs;
            Tree rhs = (((OperatorSymbol) tree.operator).opcode == string_add) ?
                    makeString(tree.rhs) : tree.rhs;
            app.args = List.make(rhs).prependList(app.args);
            result = app;
        } else {
            result = tree;
        }
    }

    public void visitUnary(Unary tree) {
        tree.arg = translate(tree.arg, tree);
        if (tree.tag == Tree.NOT && tree.arg.type.constValue != null) {
            tree.type = cfolder.fold1(bool_not, tree.arg.type);
        }
        if (Tree.PREINC <= tree.tag && tree.tag <= Tree.POSTDEC &&
                tree.arg.tag == Tree.APPLY) {
            result = tree.arg;
        } else {
            result = tree;
        }
    }

    public void visitBinary(Binary tree) {
        Tree lhs = tree.lhs = translate(tree.lhs);
        switch (tree.tag) {
        case Tree.OR:
            if (lhs.type.isTrue()) {
                result = lhs;
                return;
            }
            if (lhs.type.isFalse()) {
                result = translate(tree.rhs);
                return;
            }
            break;

        case Tree.AND:
            if (lhs.type.isFalse()) {
                result = lhs;
                return;
            }
            if (lhs.type.isTrue()) {
                result = translate(tree.rhs);
                return;
            }
            break;

        }
        tree.rhs = translate(tree.rhs);
        result = tree;
    }

    public void visitIdent(Ident tree) {
        result = access(tree.sym, tree, enclOp, false);
    }

    public void visitSelect(Select tree) {
        boolean qualifiedSuperAccess = tree.selected.tag == Tree.SELECT &&
                TreeInfo.name(tree.selected) == names._super;
        tree.selected = translate(tree.selected);
        if (tree.name == names._class)
            result = classOf(tree.selected);
        else if (tree.name == names._this || tree.name == names._super)
            result = makeThis(tree.pos, tree.selected.type.tsym);
        else
            result = access(tree.sym, tree, enclOp, qualifiedSuperAccess);
    }

    /**
      * Translate a toplevel class and return a list consisting of
      *  the translated class and translated versions of all inner classes.
      *  @param env   The attribution environment current at the class definition.
      *               We need this for resolving some additional symbols.
      *  @param cdef  The tree representing the class definition.
      */
    public List translateTopLevelClass(Env env, Tree cdef, TreeMaker make) {
        ListBuffer translated = null;
        try {
            attrEnv = env;
            this.make = make;
            endPositions = env.toplevel.endPositions;
            currentClass = null;
            outermostClassDef = (ClassDef) cdef;
            outermostMemberDef = null;
            this.translated = new ListBuffer();
            classdefs = Hashtable.make();
            actualSymbols = Hashtable.make();
            freevarCache = Hashtable.make();
            proxies = new Scope(null);
            outerThisStack = VarSymbol.emptyList;
            accessNums = Hashtable.make();
            accessSyms = Hashtable.make();
            accessConstrs = Hashtable.make();
            accessed = new ListBuffer();
            translate(cdef, null);
            for (List l = accessed.toList(); l.nonEmpty(); l = l.tail)
                makeAccessible((Symbol) l.head);
            translated = this.translated;
        }
        finally { attrEnv = null;
                  this.make = null;
                  endPositions = null;
                  currentClass = null;
                  outermostClassDef = null;
                  outermostMemberDef = null;
                  this.translated = null;
                  classdefs = null;
                  actualSymbols = null;
                  freevarCache = null;
                  proxies = null;
                  outerThisStack = null;
                  accessNums = null;
                  accessSyms = null;
                  accessConstrs = null;
                  accessed = null;
                } return translated.toList();
    }
}

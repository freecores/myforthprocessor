/**
 * @(#)Attr.java	1.93 03/01/23
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
 * This is the main context-dependent analysis phase in GJC. It
 *  encompasses name resolution, type checking and constant folding as
 *  subtasks. Some subtasks involve auxiliary classes.
 *  @see Check
 *  @see Resolve
 *  @see ConstFold
 *  @see Infer
 */
public class Attr extends Tree.Visitor implements Flags, Kinds, TypeTags {
    private static final Context.Key attrKey = new Context.Key();
    private Name.Table names;
    private Log log;
    private Symtab syms;
    private Resolve rs;
    private Check chk;
    private TreeMaker make;
    private ConstFold cfolder;
    private Enter enter;
    private TreeInfo treeinfo;
    private Target target;

    public static Attr instance(Context context) {
        Attr instance = (Attr) context.get(attrKey);
        if (instance == null)
            instance = new Attr(context);
        return instance;
    }

    private Attr(Context context) {
        super();
        context.put(attrKey, this);
        names = Name.Table.instance(context);
        log = Log.instance(context);
        syms = Symtab.instance(context);
        rs = Resolve.instance(context);
        chk = Check.instance(context);
        make = TreeMaker.instance(context);
        enter = Enter.instance(context);
        cfolder = ConstFold.instance(context);
        treeinfo = TreeInfo.instance(context);
        target = Target.instance(context);
        Options options = Options.instance(context);
        retrofit = options.get("-retrofit") != null;
    }

    /**
      * Switch: retrofit mode?
      */
    boolean retrofit;

    /**
     * Check kind and type of given tree against protokind and prototype.
     *  If check succeeds, store type in tree and return it.
     *  If check fails, store errType in tree and return it.
     *  No checks are performed if the prototype is a method type.
     *  Its not necessary in this case since we know that kind and type
     *  are correct.
     *
     *  @param tree     The tree whose kind and type is checked
     *  @param owntype  The computed type of the tree
     *  @param ownkind  The computed kind of the tree
     *  @param pkind    The expected kind (or: protokind) of the tree
     *  @param pt       The expected type (or: prototype) of the tree
     */
    Type check(Tree tree, Type owntype, int ownkind, int pkind, Type pt) {
        if (owntype.tag != ERROR && pt.tag != METHOD) {
            if ((ownkind & ~pkind) == 0) {
                owntype = chk.checkType(tree.pos, owntype, pt);
            } else {
                log.error(tree.pos, "unexpected.type", Resolve.kindNames(pkind),
                        Resolve.kindName(ownkind));
                owntype = syms.errType;
            }
        }
        tree.type = owntype;
        return owntype;
    }

    /**
      * Is given blank final variable assignable, i.e. in a scope where it
      *  may be assigned to even though it is final?
      *  @param v      The blank final variable.
      *  @param env    The current environment.
      */
    boolean isAssignableAsBlankFinal(VarSymbol v, Env env) {
        Symbol owner = ((AttrContext) env.info).scope.owner;
        return v.owner == owner || ((owner.name == names.init || owner.kind == VAR ||
                (owner.flags() & BLOCK) != 0) && v.owner == owner.owner &&
                ((v.flags() & STATIC) != 0) == Resolve.isStatic(env));
    }

    /**
      * Check that variable can be assigned to.
      *  @param pos    The current source code position.
      *  @param v      The assigned varaible
      *  @param base   If the variable is referred to in a Select, the part
      *                to the left of the `.', null otherwise.
      *  @env          The current environment.
      */
    void checkAssignable(int pos, VarSymbol v, Tree base, Env env) {
        if ((v.flags() & FINAL) != 0 && ((v.flags() & HASINIT) != 0 ||
                !((base == null ||
                (base.tag == Tree.IDENT && TreeInfo.name(base) == names._this))
                && isAssignableAsBlankFinal(v, env)))) {
            log.error(pos, "cant.assign.val.to.final.var", v.toJava());
        }
    }

    /**
      * Does tree represent a static reference to an identifier?
      *  It is assumed that tree is either a SELECT or an IDENT.
      *  We have to weed out selects from non-type names here.
      *  @param tree    The candidate tree.
      */
    boolean isStaticReference(Tree tree) {
        if (tree.tag == Tree.SELECT) {
            Symbol lsym = TreeInfo.symbol(((Select) tree).selected);
            if (lsym == null || lsym.kind != TYP) {
                return false;
            }
        }
        return true;
    }

    /**
      * Is this symbol a type?
      */
    static boolean isType(Symbol sym) {
        return sym != null && sym.kind == TYP;
    }

    /**
      * The current `this' symbol.
      *  @param env    The current environment.
      */
    Symbol thisSym(Env env) {
        return rs.resolveSelf(Position.NOPOS, env, env.enclClass.sym, names._this);
    }

    /**
      * Visitor argument: the current environment.
      */
    Env env;

    /**
     * Visitor argument: the currently expected proto-kind.
     */
    int pkind;

    /**
     * Visitor argument: the currently expected proto-type.
     */
    Type pt;

    /**
     * Visitor result: the computed type.
     */
    Type result;

    /**
     * Visitor method: attribute a tree, catching any completion failure
     *  exceptions. Return the tree's type.
     *
     *  @param tree    The tree to be visited.
     *  @param env     The environment visitor argument.
     *  @param pkind   The protokind visitor argument.
     *  @param pt      The prototype visitor argument.
     */
    Type attribTree(Tree tree, Env env, int pkind, Type pt) {
        Env prevEnv = this.env;
        int prevPkind = this.pkind;
        Type prevPt = this.pt;
        try {
            this.env = env;
            this.pkind = pkind;
            this.pt = pt;
            tree.accept(this);
            return result;
        } catch (CompletionFailure ex) {
            tree.type = syms.errType;
            return chk.completionError(tree.pos, ex);
        }
        finally { this.env = prevEnv;
                  this.pkind = prevPkind;
                  this.pt = prevPt;
                } }

    /**
      * Derived visitor method: attribute an expression tree.
      */
    Type attribExpr(Tree tree, Env env, Type pt) {
        return attribTree(tree, env, VAL, pt);
    }

    /**
      * Derived visitor method: attribute an expression tree with
      *  no constraints on the computed type.
      */
    Type attribExpr(Tree tree, Env env) {
        return attribTree(tree, env, VAL, Type.noType);
    }

    /**
      * Derived visitor method: attribute a type tree.
      */
    Type attribType(Tree tree, Env env) {
        return attribTree(tree, env, TYP, Type.noType);
    }

    /**
      * Derived visitor method: attribute a statement or definition tree.
      */
    Type attribStat(Tree tree, Env env) {
        return attribTree(tree, env, NIL, Type.noType);
    }

    /**
      * Attribute a list of expressions, returning a list of types.
      */
    List attribExprs(List trees, Env env, Type pt) {
        ListBuffer ts = new ListBuffer();
        for (List l = trees; l.nonEmpty(); l = l.tail)
            ts.append(attribExpr((Tree) l.head, env, pt));
        return ts.toList();
    }

    /**
      * Attribute a list of statements, returning nothing.
      */
    void attribStats(List trees, Env env) {
        for (List l = trees; l.nonEmpty(); l = l.tail)
            attribStat((Tree) l.head, env);
    }

    /**
      * Attribute the arguments in a method call, returning a list of types.
      */
    List attribArgs(List trees, Env env) {
        ListBuffer argtypes = new ListBuffer();
        for (List l = trees; l.nonEmpty(); l = l.tail)
            argtypes.append( chk.checkNonVoid(((Tree) l.head).pos,
                    attribTree((Tree) l.head, env, VAL, Infer.anyPoly)));
        return argtypes.toList();
    }

    /**
      * Attribute type reference in an `extends' or `implements' clause.
      *
      *  @param tree              The tree making up the type reference.
      *  @param env               The environment current at the reference.
      *  @param classExpected     true if only a class is expected here.
      *  @param interfaceExpected true if only an interface is expected here.
      */
    Type attribBase(Tree tree, Env env, boolean classExpected,
            boolean interfaceExpected) {
        Type t = attribType(tree, env);
        t = chk.checkClassType(tree.pos, t);
        if (interfaceExpected & (t.tsym.flags() & INTERFACE) == 0) {
            log.error(tree.pos, "intf.expected.here");
            return syms.errType;
        } else if (classExpected & (t.tsym.flags() & INTERFACE) != 0) {
            log.error(tree.pos, "no.intf.expected.here");
            return syms.errType;
        }
        if ((t.tsym.flags() & FINAL) != 0) {
            log.error(tree.pos, "cant.inherit.from.final", t.tsym.toJava());
        }
        return t;
    }

    public void visitClassDef(ClassDef tree) {
        if ((((AttrContext) env.info).scope.owner.kind & (VAR | MTH)) != 0)
            enter.classEnter(tree, env);
        ClassSymbol c = tree.sym;
        if (c == null) {
            result = null;
        } else {
            c.complete();
            if (((AttrContext) env.info).isSelfCall &&
                    env.tree.tag == Tree.NEWCLASS &&
                    ((NewClass) env.tree).encl == null) {
                c.flags_field |= NOOUTERTHIS;
            }
            attribClass(tree.pos, c);
            result = tree.type = c.type;
        }
    }

    public void visitMethodDef(MethodDef tree) {
        MethodSymbol m = tree.sym;
        chk.checkOverride(tree, m);
        Env localEnv = enter.methodEnv((MethodDef) tree, env);
        for (List l = tree.params; l.nonEmpty(); l = l.tail)
            attribStat((Tree) l.head, localEnv);
        chk.validate(tree.restype);
        for (List l = tree.thrown; l.nonEmpty(); l = l.tail)
            chk.checkType(((Tree) l.head).pos, ((Tree) l.head).type,
                    syms.throwableType);
        ClassSymbol owner = env.enclClass.sym;
        if (tree.body == null) {
            if ((owner.flags() & INTERFACE) == 0 &&
                    (tree.flags & (ABSTRACT | NATIVE)) == 0 && !retrofit)
                log.error(tree.pos, "missing.meth.body.or.decl.abstract");
        } else if ((owner.flags() & INTERFACE) != 0) {
            log.error(tree.body.pos, "intf.meth.cant.have.body");
        } else if ((tree.flags & ABSTRACT) != 0) {
            log.error(tree.pos, "abstract.meth.cant.have.body");
        } else if ((tree.flags & NATIVE) != 0) {
            log.error(tree.pos, "native.meth.cant.have.body");
        } else {
            if (tree.name == names.init && owner.type != syms.objectType) {
                Block body = tree.body;
                if (body.stats.isEmpty() ||
                        !TreeInfo.isSelfCall((Tree) body.stats.head)) {
                    body.stats = body.stats.prepend(
                            enter.SuperCall(make.at(body.pos), VarDef.emptyList,
                            false));
                }
            }
            attribStat(tree.body, localEnv);
        }
        ((AttrContext) localEnv.info).scope.leave();
        result = tree.type = m.type;
    }

    public void visitVarDef(VarDef tree) {
        if (((AttrContext) env.info).scope.owner.kind == MTH)
            enter.phase2.memberEnter(tree, env);
        chk.validate(tree.vartype);
        VarSymbol v = tree.sym;
        if (tree.init != null) {
            v.pos = Position.MAXPOS;
            if ((v.flags_field & FINAL) != 0) {
                evalInit(v);
            } else {
                Type itype = attribExpr(tree.init, enter.initEnv((VarDef) tree, env),
                        v.type);
            }
            v.pos = tree.pos;
        }
        result = tree.type = v.type;
    }

    public void visitSkip(Skip tree) {
        result = null;
    }

    public void visitBlock(Block tree) {
        if (((AttrContext) env.info).scope.owner.kind == TYP) {
            Env localEnv = env.dup(tree,
                    ((AttrContext) env.info).dup(
                    ((AttrContext) env.info).scope.dupUnshared()));
            ((AttrContext) localEnv.info).scope.owner =
                    new MethodSymbol(tree.flags | BLOCK, names.empty, null,
                    ((AttrContext) env.info).scope.owner);
            if ((tree.flags & STATIC) != 0)
                ((AttrContext) localEnv.info).staticLevel++;
            attribStats(tree.stats, localEnv);
        } else {
            Env localEnv = env.dup(tree,
                    ((AttrContext) env.info).dup(
                    ((AttrContext) env.info).scope.dup()));
            attribStats(tree.stats, localEnv);
            ((AttrContext) localEnv.info).scope.leave();
        }
        result = null;
    }

    public void visitDoLoop(DoLoop tree) {
        attribStat(tree.body, env.dup(tree));
        attribExpr(tree.cond, env, syms.booleanType);
        result = null;
    }

    public void visitWhileLoop(WhileLoop tree) {
        attribExpr(tree.cond, env, syms.booleanType);
        attribStat(tree.body, env.dup(tree));
        result = null;
    }

    public void visitForLoop(ForLoop tree) {
        Env loopEnv = env.dup(env.tree,
                ((AttrContext) env.info).dup(((AttrContext) env.info).scope.dup()));
        attribStats(tree.init, loopEnv);
        if (tree.cond != null)
            attribExpr(tree.cond, loopEnv, syms.booleanType);
        loopEnv.tree = tree;
        attribStats(tree.step, loopEnv);
        attribStat(tree.body, loopEnv);
        ((AttrContext) loopEnv.info).scope.leave();
        result = null;
    }

    public void visitLabelled(Labelled tree) {
        Env env1 = env;
        while (env1 != null && env1.tree.tag != Tree.CLASSDEF) {
            if (env1.tree.tag == Tree.LABELLED &&
                    ((Labelled) env1.tree).label == tree.label) {
                log.error(tree.pos, "label.already.in.use", tree.label.toJava());
                break;
            }
            env1 = env1.next;
        }
        attribStat(tree.body, env.dup(tree));
        result = null;
    }

    public void visitSwitch(Switch tree) {
        Type seltype = attribExpr(tree.selector, env, syms.intType);
        Env switchEnv = env.dup(tree,
                ((AttrContext) env.info).dup(((AttrContext) env.info).scope.dup()));
        Set labels = Set.make();
        boolean hasDefault = false;
        for (List l = tree.cases; l.nonEmpty(); l = l.tail) {
            Case c = (Tree.Case) l.head;
            Env caseEnv = switchEnv.dup(c,
                    ((AttrContext) env.info).dup(
                    ((AttrContext) switchEnv.info).scope.dup()));
            if (c.pat != null) {
                Type pattype = attribExpr(c.pat, switchEnv, syms.intType);
                if (pattype.tag != ERROR) {
                    if (pattype.constValue == null) {
                        log.error(c.pat.pos, "const.expr.req");
                    } else if (labels.contains(pattype.constValue)) {
                        log.error(c.pos, "duplicate.case.label");
                    } else {
                        chk.checkType(c.pat.pos, pattype, seltype);
                        labels.put(pattype.constValue);
                    }
                }
            } else if (hasDefault) {
                log.error(c.pos, "duplicate.default.label");
            } else {
                hasDefault = true;
            }
            attribStats(c.stats, caseEnv);
            ((AttrContext) caseEnv.info).scope.leave();
            addVars(c.stats, ((AttrContext) switchEnv.info).scope);
        }
        ((AttrContext) switchEnv.info).scope.leave();
        result = null;
    }

    /**
      * Add any variables defined in stats to the switch scope.
      */
    private static void addVars(List stats, Scope switchScope) {
        for (; stats.nonEmpty(); stats = stats.tail) {
            Tree stat = (Tree) stats.head;
            if (stat.tag == Tree.VARDEF)
                switchScope.enter(((VarDef) stat).sym);
        }
    }

    public void visitSynchronized(Synchronized tree) {
        chk.checkRefType(tree.pos, attribExpr(tree.lock, env, syms.objectType));
        attribStat(tree.body, env);
        result = null;
    }

    public void visitTry(Try tree) {
        attribStat(tree.body, env.dup(tree, ((AttrContext) env.info).dup()));
        for (List l = tree.catchers; l.nonEmpty(); l = l.tail) {
            Catch c = (Tree.Catch) l.head;
            Env catchEnv = env.dup(c,
                    ((AttrContext) env.info).dup(
                    ((AttrContext) env.info).scope.dup()));
            Type ctype = attribStat(c.param, catchEnv);
            chk.checkType(c.param.vartype.pos, ctype, syms.throwableType);
            attribStat(c.body, catchEnv);
            ((AttrContext) catchEnv.info).scope.leave();
        }
        if (tree.finalizer != null)
            attribStat(tree.finalizer, env);
        result = null;
    }

    public void visitConditional(Conditional tree) {
        attribExpr(tree.cond, env, syms.booleanType);
        attribExpr(tree.truepart, env, pt);
        attribExpr(tree.falsepart, env, pt);
        result = check(tree,
                condType(tree.pos, tree.cond.type, tree.truepart.type,
                tree.falsepart.type), VAL, pkind, pt);
    }

    /**
      * Compute the type of a conditional expression, after
      *  checking that it exists. See Spec 15.25.
      *
      *  @param pos      The source position to be used for error diagnostics.
      *  @param condtype The type of the expression's condition.
      *  @param thentype The type of the expression's then-part.
      *  @param elsetype The type of the expression's else-part.
      */
    private Type condType(int pos, Type condtype, Type thentype, Type elsetype) {
        Type ctype = condType1(pos, condtype, thentype, elsetype);
        return ((condtype.constValue != null) && (thentype.constValue != null) &&
                (elsetype.constValue != null)) ? cfolder.coerce(
                (((Number) condtype.constValue).intValue() != 0) ? thentype :
                elsetype, ctype):
               ctype;
    }

    /**
      * Compute the type of a conditional expression, after
      *  checking that it exists.  Does not take into
      *  account the special case where condition and both arms are constants.
      *
      *  @param pos      The source position to be used for error diagnostics.
      *  @param condtype The type of the expression's condition.
      *  @param thentype The type of the expression's then-part.
      *  @param elsetype The type of the expression's else-part.
      */
    private Type condType1(int pos, Type condtype, Type thentype, Type elsetype) {
        if (thentype.tag < INT && elsetype.tag == INT &&
                elsetype.isAssignable(thentype))
            return thentype.baseType();
        else if (elsetype.tag < INT && thentype.tag == INT &&
                thentype.isAssignable(elsetype))
            return elsetype.baseType();
        else if (thentype.tag <= DOUBLE && elsetype.tag <= DOUBLE)
            for (int i = BYTE; i <= DOUBLE; i++) {
                Type candidate = syms.typeOfTag[i];
                if (thentype.isSubType(candidate) && elsetype.isSubType(candidate))
                    return candidate;
            }
        if (thentype.tsym == syms.stringType.tsym &&
                elsetype.tsym == syms.stringType.tsym)
            return syms.stringType;
        else if (thentype.isSubType(elsetype))
            return elsetype.baseType();
        else if (elsetype.isSubType(thentype))
            return thentype.baseType();
        else {
            log.error(pos, "neither.conditional.subtype", thentype.toJava(),
                    elsetype.toJava());
            return thentype.baseType();
        }
    }

    public void visitIf(If tree) {
        attribExpr(tree.cond, env, syms.booleanType);
        attribExpr(tree.thenpart, env, pt);
        if (tree.elsepart != null)
            attribExpr(tree.elsepart, env, pt);
        result = null;
    }

    public void visitExec(Exec tree) {
        attribExpr(tree.expr, env);
        result = null;
    }

    public void visitBreak(Break tree) {
        tree.target = findJumpTarget(tree.pos, tree.tag, tree.label, env);
        result = null;
    }

    public void visitContinue(Continue tree) {
        tree.target = findJumpTarget(tree.pos, tree.tag, tree.label, env);
        result = null;
    }

    /**
      * Return the target of a break or continue statement, if it exists,
      *  report an error if not.
      *  Note: The target of a labelled break or continue is the
      *  (non-labelled) statement tree referred to by the label,
      *  not the tree representing the labelled statement itself.
      *
      *  @param pos     The position to be used for error diagnostics
      *  @param tag     The tag of the jump statement. This is either
      *                 Tree.BREAK or Tree.CONTINUE.
      *  @param label   The label of the jump statement, or null if no
      *                 label is given.
      *  @param env     The environment current at the jump statement.
      */
    private Tree findJumpTarget(int pos, int tag, Name label, Env env) {
        Env env1 = env;
        LOOP:
        while (env1 != null) {
            switch (env1.tree.tag) {
            case Tree.LABELLED:
                Labelled labelled = (Labelled) env1.tree;
                if (label == labelled.label) {
                    if (tag == Tree.CONTINUE) {
                        if (labelled.body.tag != Tree.DOLOOP &&
                                labelled.body.tag != Tree.WHILELOOP &&
                                labelled.body.tag != Tree.FORLOOP)
                            log.error(pos, "not.loop.label", label.toJava());
                        return TreeInfo.referencedStatement(labelled);
                    } else {
                        return labelled;
                    }
                }
                break;

            case Tree.DOLOOP:

            case Tree.WHILELOOP:

            case Tree.FORLOOP:
                if (label == null)
                    return env1.tree;
                break;

            case Tree.SWITCH:
                if (label == null && tag == Tree.BREAK)
                    return env1.tree;
                break;

            case Tree.METHODDEF:

            case Tree.CLASSDEF:
                break LOOP;

            default:

            }
            env1 = env1.next;
        }
        if (label != null)
            log.error(pos, "undef.label", label.toJava());
        else if (tag == Tree.CONTINUE)
            log.error(pos, "cont.outside.loop");
        else
            log.error(pos, "break.outside.switch.loop");
        return null;
    }

    public void visitReturn(Return tree) {
        if (env.enclMethod == null || env.enclMethod.sym.owner != env.enclClass.sym) {
            log.error(tree.pos, "ret.outside.meth");
        } else {
            Symbol m = env.enclMethod.sym;
            if (m.type.restype().tag == VOID) {
                if (tree.expr != null)
                    log.error(tree.expr.pos, "cant.ret.val.from.meth.decl.void");
            } else if (tree.expr == null) {
                log.error(tree.pos, "missing.ret.val");
            } else {
                attribExpr(tree.expr, env, m.type.restype());
            }
        }
        result = null;
    }

    public void visitThrow(Throw tree) {
        Type t = attribExpr(tree.expr, env, syms.throwableType);
        result = null;
    }

    public void visitAssert(Assert tree) {
        Type ct = attribExpr(tree.cond, env, syms.booleanType);
        if (tree.detail != null) {
            chk.checkNonVoid(tree.detail.pos, attribExpr(tree.detail, env));
        }
        result = null;
    }

    /**
      * Visitor method for method invocations.
      *  NOTE: The method part of an application will have in its type field
      *        the return type of the method, not the method's type itself!
      */
    public void visitApply(Apply tree) {
        Env localEnv = env;
        List argtypes;
        Name methName = TreeInfo.name(tree.meth);
        boolean isConstructorCall =
                methName == names._this || methName == names._super;
        if (isConstructorCall) {
            if (checkFirstConstructorStat(tree, env)) {
                localEnv = env.dup(env.tree, ((AttrContext) env.info).dup());
                ((AttrContext) localEnv.info).isSelfCall = true;
                argtypes = attribArgs(tree.args, localEnv);
                Type site = env.enclClass.sym.type;
                if (methName == names._super)
                    site = site.supertype();
                if (site.tag == CLASS) {
                    if (site.outer().tag == CLASS) {
                        if (tree.meth.tag == Tree.SELECT) {
                            Tree qualifier = ((Select) tree.meth).selected;
                            chk.checkRefType(qualifier.pos,
                                    attribExpr(qualifier, localEnv, site.outer()));
                        } else if (methName == names._super) {
                            rs.resolveImplicitThis(tree.meth.pos, localEnv, site);
                        }
                    } else if (tree.meth.tag == Tree.SELECT) {
                        log.error(tree.meth.pos, "illegal.qual.not.icls",
                                site.tsym.toJava());
                    }
                    boolean selectSuperPrev =
                            ((AttrContext) localEnv.info).selectSuper;
                    ((AttrContext) localEnv.info).selectSuper = true;
                    Symbol sym = rs.resolveConstructor(tree.meth.pos, localEnv, site,
                            argtypes);
                    ((AttrContext) localEnv.info).selectSuper = selectSuperPrev;
                    TreeInfo.setSymbol(tree.meth, sym);
                    List saved = methTemplateSupply.elems;
                    Type mpt = newMethTemplate(argtypes);
                    checkId(tree.meth, site, sym, env, MTH, mpt);
                    methTemplateSupply.elems = saved;
                }
            }
            result = syms.voidType;
        } else {
            argtypes = attribArgs(tree.args, localEnv);
            List saved = methTemplateSupply.elems;
            Type mpt = newMethTemplate(argtypes);
            Type mtype = attribExpr(tree.meth, localEnv, mpt);
            methTemplateSupply.elems = saved;
            result = check(tree, mtype.restype(), VAL, pkind, pt);
        }
    }

    /**
      * Check that given application node appears as first statement
      *  in a constructor call.
      *  @param tree   The application node
      *  @param env    The environment current at the application.
      */
    boolean checkFirstConstructorStat(Apply tree, Env env) {
        MethodDef enclMethod = env.enclMethod;
        if (enclMethod != null && enclMethod.name == names.init) {
            Block body = (Block) enclMethod.body;
            if (((Tree) body.stats.head).tag == Tree.EXEC &&
                    ((Exec) body.stats.head).expr == tree)
                return true;
        }
        log.error(tree.pos, "call.must.be.first.stmt.in.ctor",
                TreeInfo.name(tree.meth).toJava());
        return false;
    }

    /**
      * Optimization: To avoid allocating a new methodtype for every
      *  attribution of an Apply node, we use a reservoir.
      */
    ListBuffer methTemplateSupply = new ListBuffer();

    /**
     * Obtain an otherwise unused method type with given argument types.
     *  Take it from the reservoir if non-empty, or create a new one.
     */
    Type newMethTemplate(List argtypes) {
        if (methTemplateSupply.elems == methTemplateSupply.last)
            methTemplateSupply.append(
                    new MethodType(null, null, null, syms.methodClass));
        MethodType mt = (Type.MethodType) methTemplateSupply.elems.head;
        methTemplateSupply.elems = methTemplateSupply.elems.tail;
        mt.argtypes = argtypes;
        return mt;
    }

    public void visitNewClass(NewClass tree) {
        Type owntype = syms.errType;
        ClassDef cdef = tree.def;
        Tree clazz = tree.clazz;
        Tree clazzid = clazz;
        Tree clazzid1 = clazzid;
        if (tree.encl != null) {
            Type encltype =
                    chk.checkRefType(tree.encl.pos, attribExpr(tree.encl, env));
            clazzid1 = make.at(clazz.pos).Select(make.Type(encltype),
                    ((Ident) clazzid).name);
            clazz = clazzid1;
        }
        Type clazztype = chk.checkClassType(tree.clazz.pos, attribType(clazz, env));
        chk.validate(clazz);
        if (tree.encl != null) {
            tree.clazz.type = clazztype;
            TreeInfo.setSymbol(clazzid, TreeInfo.symbol(clazzid1));
            clazzid.type = ((Ident) clazzid).sym.type;
            if ((clazztype.tsym.flags() & STATIC) != 0 && !clazztype.isErroneous()) {
                log.error(tree.pos, "qualified.new.of.static.class",
                        clazztype.tsym.toJava());
            }
        } else if ((clazztype.tsym.flags() & INTERFACE) == 0 &&
                clazztype.outer().tag == CLASS) {
            rs.resolveImplicitThis(tree.pos, env, clazztype);
        }
        List argtypes = attribArgs(tree.args, env);
        if (clazztype.tag == CLASS) {
            if (cdef == null &&
                    (clazztype.tsym.flags() & (ABSTRACT | INTERFACE)) != 0) {
                log.error(tree.pos, "abstract.cant.be.instantiated",
                        clazztype.tsym.toJava());
            } else if (cdef != null && (clazztype.tsym.flags() & INTERFACE) != 0) {
                if (argtypes.nonEmpty()) {
                    log.error(tree.pos, "anon.class.impl.intf.no.args");
                    argtypes = Type.emptyList;
                } else if (tree.encl != null) {
                    log.error(tree.pos, "anon.class.impl.intf.no.qual.for.new");
                }
            } else {
                boolean selectSuperPrev = ((AttrContext) env.info).selectSuper;
                if (cdef != null)
                    ((AttrContext) env.info).selectSuper = true;
                tree.constructor =
                        rs.resolveConstructor(tree.pos, env, clazztype, argtypes);
                ((AttrContext) env.info).selectSuper = selectSuperPrev;
            }
            if (cdef != null) {
                if (Resolve.isStatic(env))
                    cdef.flags |= STATIC;
                if ((clazztype.tsym.flags() & INTERFACE) != 0) {
                    cdef.implementing = List.make(clazz);
                } else {
                    cdef.extending = clazz;
                }
                attribStat(cdef, env.dup(tree));
                if (tree.encl != null) {
                    tree.args = tree.args.prepend(makeNullCheck(tree.encl));
                    argtypes = argtypes.prepend(tree.encl.type);
                    tree.encl = null;
                }
                clazztype = cdef.sym.type;
                tree.constructor =
                        rs.resolveConstructor(tree.pos, env, clazztype, argtypes);
            }
            if (tree.constructor != null && tree.constructor.kind == MTH) {
                owntype = clazztype;
            }
        }
        result = check(tree, owntype, VAL, pkind, pt);
    }

    /**
      * Make an attributed null check tree.
      */
    public Tree makeNullCheck(Tree arg) {
        Name name = TreeInfo.name(arg);
        if (name == names._this || name == names._super)
            return arg;
        int optag = Tree.NULLCHK;
        Unary tree = make.at(arg.pos).Unary(optag, arg);
        tree.operator = syms.nullcheck;
        tree.type = arg.type;
        return tree;
    }

    public void visitNewArray(NewArray tree) {
        Type owntype = syms.errType;
        Type elemtype;
        if (tree.elemtype != null) {
            elemtype = attribType(tree.elemtype, env);
            chk.validate(tree.elemtype);
            owntype = elemtype;
            for (List l = tree.dims; l.nonEmpty(); l = l.tail) {
                attribExpr((Tree) l.head, env, syms.intType);
                owntype = new ArrayType(owntype, syms.arrayClass);
            }
        } else {
            if (pt.tag == ARRAY) {
                elemtype = pt.elemtype();
            } else {
                if (pt.tag != ERROR) {
                    log.error(tree.pos, "illegal.initializer.for.type", pt.toJava());
                }
                elemtype = syms.errType;
            }
        }
        if (tree.elems != null) {
            attribExprs(tree.elems, env, elemtype);
            owntype = new ArrayType(elemtype, syms.arrayClass);
        }
        result = check(tree, owntype, VAL, pkind, pt);
    }

    public void visitParens(Parens tree) {
        Type owntype = attribTree(tree.expr, env, pkind, pt);
        result = check(tree, owntype, pkind, pkind, pt);
        Symbol sym = TreeInfo.symbol(tree);
        if (isType(sym)) {
            log.error(tree.pos, "illegal.start.of.expr");
        }
    }

    public void visitAssign(Assign tree) {
        Type owntype = attribTree(tree.lhs, env.dup(tree), VAR, pt);
        attribExpr(tree.rhs, env, owntype);
        result = check(tree, owntype, VAL, pkind, pt);
    }

    public void visitAssignop(Assignop tree) {
        List argtypes = List.make(attribTree(tree.lhs, env, VAR, Type.noType),
                attribExpr(tree.rhs, env));
        Symbol operator = tree.operator =
                rs.resolveOperator(tree.pos, tree.tag - Tree.ASGOffset, env,
                argtypes);
        Type owntype = (Type) argtypes.head;
        if (operator.kind == MTH) {
            if (owntype.tag <= DOUBLE)
                chk.checkCastable(tree.rhs.pos, operator.type.restype(), owntype);
            else if (owntype.tag == CLASS)
                chk.checkType(tree.lhs.pos, owntype, syms.stringType);
            else
                chk.checkType(tree.rhs.pos, operator.type.restype(), owntype);
        }
        result = check(tree, owntype, VAL, pkind, pt);
    }

    public void visitUnary(Unary tree) {
        Type argtype = (Tree.PREINC <= tree.tag && tree.tag <= Tree.POSTDEC) ?
                attribTree(tree.arg, env, VAR, Type.noType) :
                chk.checkNonVoid(tree.arg.pos, attribExpr(tree.arg, env));
        Symbol operator = tree.operator =
                rs.resolveUnaryOperator(tree.pos, tree.tag, env, argtype);
        Type owntype = syms.errType;
        if (operator.kind == MTH) {
            owntype = operator.type.restype();
            int opc = ((OperatorSymbol) operator).opcode;
            if (argtype.constValue != null) {
                Type ctype = cfolder.fold1(opc, argtype);
                if (ctype != null) {
                    owntype = cfolder.coerce(ctype, owntype);
                    if (tree.arg.type.tsym == syms.stringType.tsym) {
                        tree.arg.type = syms.stringType;
                    }
                }
            }
        }
        result = check(tree, owntype, VAL, pkind, pt);
    }

    public void visitBinary(Binary tree) {
        Type left = chk.checkNonVoid(tree.lhs.pos, attribExpr(tree.lhs, env));
        Type right = chk.checkNonVoid(tree.lhs.pos, attribExpr(tree.rhs, env));
        Symbol operator = tree.operator =
                rs.resolveBinaryOperator(tree.pos, tree.tag, env, left, right);
        Type owntype = syms.errType;
        if (operator.kind == MTH) {
            owntype = operator.type.restype();
            int opc = ((OperatorSymbol) operator).opcode;
            if (opc == ByteCodes.error) {
                log.error(tree.lhs.pos, "operator.cant.be.applied",
                        treeinfo.operatorName(tree.tag).toJava(),
                        left.toJava() + "," + right.toJava());
            }
            if (left.constValue != null && right.constValue != null) {
                Type ctype = cfolder.fold2(opc, left, right);
                if (ctype != null) {
                    owntype = cfolder.coerce(ctype, owntype);
                    if (tree.lhs.type.tsym == syms.stringType.tsym) {
                        tree.lhs.type = syms.stringType;
                    }
                    if (tree.rhs.type.tsym == syms.stringType.tsym) {
                        tree.rhs.type = syms.stringType;
                    }
                }
            }
            if (opc == ByteCodes.if_acmpeq || opc == ByteCodes.if_acmpne) {
                if (!left.isCastable(right.erasure()) &&
                        !right.isCastable(left.erasure())) {
                    log.error(tree.pos, "incomparable.types", left.toJava(),
                            right.toJava());
                } else {
                    chk.checkCompatible(tree.pos, left, right);
                }
            }
        }
        result = check(tree, owntype, VAL, pkind, pt);
    }

    public void visitTypeCast(TypeCast tree) {
        Type clazztype = attribType(tree.clazz, env);
        Type exprtype = attribExpr(tree.expr, env, Type.noType);
        Type owntype = chk.checkCastable(tree.expr.pos, exprtype, clazztype);
        if (exprtype.constValue != null)
            owntype = cfolder.coerce(exprtype, owntype);
        result = check(tree, owntype, VAL, pkind, pt);
    }

    public void visitTypeTest(TypeTest tree) {
        Type exprtype = attribExpr(tree.expr, env);
        Type clazztype = chk.checkClassOrArrayType(tree.clazz.pos,
                attribType(tree.clazz, env));
        chk.checkCastable(tree.expr.pos, exprtype, clazztype);
        result = check(tree, syms.booleanType, VAL, pkind, pt);
    }

    public void visitIndexed(Indexed tree) {
        Type owntype = syms.errType;
        Type atype = attribExpr(tree.indexed, env);
        attribExpr(tree.index, env, syms.intType);
        if (atype.tag == ARRAY)
            owntype = atype.elemtype();
        else if (atype.tag != ERROR)
            log.error(tree.pos, "array.req.but.found", atype.toJava());
        result = check(tree, owntype, VAR, pkind, pt);
    }

    public void visitIdent(Ident tree) {
        Symbol sym;
        if (pt.tag == METHOD) {
            sym = rs.resolveMethod(tree.pos, env, tree.name, pt.argtypes());
        } else if (tree.sym != null && tree.sym.kind != VAR) {
            sym = tree.sym;
        } else {
            sym = rs.resolveIdent(tree.pos, env, tree.name, pkind);
        }
        tree.sym = sym;
        Env symEnv = env;
        boolean noOuterThisPath = false;
        if (env.enclClass.sym.owner.kind != PCK &&
                (sym.kind & (VAR | MTH | TYP)) != 0 && sym.owner.kind == TYP &&
                tree.name != names._this && tree.name != names._super) {
            while (symEnv.outer != null && !sym.isMemberOf(symEnv.enclClass.sym)) {
                if ((symEnv.enclClass.sym.flags() & NOOUTERTHIS) != 0)
                    noOuterThisPath = true;
                symEnv = symEnv.outer;
            }
        }
        if (sym.kind == VAR) {
            VarSymbol v = (VarSymbol) sym;
            checkInit(tree, env, v);
            if (v.owner.kind == MTH &&
                    v.owner != ((AttrContext) env.info).scope.owner) {
                if ((v.flags_field & FINAL) == 0) {
                    log.error(tree.pos, "local.var.accessed.from.icls.needs.final",
                            v.toJava());
                }
            }
            if (pkind == VAR)
                checkAssignable(tree.pos, v, null, env);
        }
        if ((((AttrContext) symEnv.info).isSelfCall || noOuterThisPath) &&
                (sym.kind & (VAR | MTH)) != 0 && sym.owner.kind == TYP &&
                (sym.flags() & STATIC) == 0) {
            chk.earlyRefError(tree.pos, sym.kind == VAR ? sym : thisSym(env));
        }
        result = checkId(tree, env.enclClass.sym.type, sym, env, pkind, pt);
    }

    public void visitSelect(Select tree) {
        int skind = 0;
        if (tree.name == names._this || tree.name == names._super ||
                tree.name == names._class) {
            skind = TYP;
        } else {
            if ((pkind & PCK) != 0)
                skind = skind | PCK;
            if ((pkind & TYP) != 0)
                skind = skind | TYP | PCK;
            if ((pkind & (VAL | MTH)) != 0)
                skind = skind | VAL | TYP;
        }
        Type site = attribTree(tree.selected, env, skind, Infer.anyPoly);
        Symbol sitesym = TreeInfo.symbol(tree.selected);
        boolean selectSuperPrev = ((AttrContext) env.info).selectSuper;
        ((AttrContext) env.info).selectSuper = sitesym != null &&
                (sitesym.name == names._super || sitesym.kind == TYP);
        Symbol sym = selectSym(tree, site, env, pt, pkind);
        tree.sym = sym;
        if (sym.kind == VAR) {
            VarSymbol v = (VarSymbol) sym;
            evalInit(v);
            if (pkind == VAR)
                checkAssignable(tree.pos, v, tree.selected, env);
        }
        if (isType(sym) && (sitesym == null || (sitesym.kind & (TYP | PCK)) == 0)) {
            tree.type = check(tree.selected, pt,
                    sitesym == null ? VAL : sitesym.kind, TYP | PCK, pt);
        }
        if (((AttrContext) env.info).selectSuper) {
            if ((sym.flags() & STATIC) == 0 && sym.name != names._this &&
                    sym.name != names._super) {
                if (sitesym.name == names._super) {
                    rs.checkNonAbstract(tree.pos, sym);
                } else if (sym.kind == VAR || sym.kind == MTH) {
                    rs.access(new Resolve.StaticError(sym), tree.pos, site,
                            sym.name, true);
                }
                if (site.isRaw()) {
                    Type site1 = env.enclClass.sym.type.asSuper(site.tsym);
                    if (site1 != null)
                        site = site1;
                }
            }
            if (((AttrContext) env.info).isSelfCall && tree.name == names._this &&
                    site.tsym == env.enclClass.sym) {
                chk.earlyRefError(tree.pos, sym);
            }
        }
        ((AttrContext) env.info).selectSuper = selectSuperPrev;
        result = checkId(tree, site, sym, env, pkind, pt);
        ((AttrContext) env.info).tvars = Type.emptyList;
    }

    /**
      * Determine symbol referenced by a Select expression,
      *
      *  @param tree   The select tree.
      *  @param site   The type of the selected expression,
      *  @param env    The current environment.
      *  @param pt     The current prototype.
      *  @param pkind  The expected kind(s) of the Select expression.
      */
    private Symbol selectSym(Select tree, Type site, Env env, Type pt, int pkind) {
        int pos = tree.pos;
        Name name = tree.name;
        switch (site.tag) {
        case PACKAGE:
            return rs.access(rs.findIdentInPackage(env, site.tsym, name, pkind),
                    pos, site, name, true);

        case ARRAY:

        case CLASS:
            if (pt.tag == METHOD) {
                return rs.resolveQualifiedMethod(pos, env, site, name, pt.argtypes());
            } else if (name == names._this || name == names._super) {
                return rs.resolveSelf(pos, env, site.tsym, name);
            } else if (name == names._class) {
                return new VarSymbol(STATIC | PUBLIC | FINAL, names._class,
                        syms.classType, site.tsym);
            } else {
                return rs.access(rs.findIdentInType(env, site, name, pkind), pos,
                        site, name, true);
            }

        case ERROR:
            return new ErrorType(name, site.tsym).tsym;

        default:
            if (name == names._class) {
                return new VarSymbol(STATIC | PUBLIC | FINAL, names._class,
                        syms.classType, site.tsym);
            } else {
                log.error(pos, "cant.deref", site.toJava());
                return syms.errSymbol;
            }

        }
    }

    /**
      * Determine type of identifier or select expression and check that
      *  (1) the referenced symbol is not deprecated
      *  (2) the symbol's type is safe (@see checkSafe)
      *  (3) if symbol is a variable, check that its type and kind are
      *      compatible with the prototype and protokind.
      *  (4) if symbol is an instance field of a raw type,
      *      which is being assigned to, issue an unchecked warning if its
      *      type changes under erasure.
      *  (5) if symbol is an instance method of a raw type, issue an
      *      unchecked warning if its argument types change under erasure.
      *  If checks succeed:
      *    If symbol is a constant, return its constant type
      *    else if symbol is a method, return its result type
      *    otherwise return its type.
      *  Otherwise return errType.
      *
      *  @param tree       The syntax tree representing the identifier
      *  @param site       If this is a select, the type of the selected
      *                    expression, otherwise the type of the current class.
      *  @param sym        The symbol representing the identifier.
      *  @param env        The current environment.
      *  @param pkind      The set of expected kinds.
      *  @param pt         The expected type.
      */
    Type checkId(Tree tree, Type site, Symbol sym, Env env, int pkind, Type pt) {
        Type owntype;
        switch (sym.kind) {
        case TYP:
            owntype = sym.type;
            if (owntype.tag == CLASS) {
                Type ownOuter = owntype.outer();
                if (ownOuter.tag == CLASS && site != ownOuter) {
                    Type normSite = site;
                    if (normSite.tag == CLASS)
                        normSite = site.asOuterSuper(ownOuter.tsym);
                    if (normSite != ownOuter)
                        owntype = new ClassType(normSite, Type.emptyList,
                                owntype.tsym);
                }
            }
            break;

        case VAR:
            VarSymbol v = (VarSymbol) sym;
            owntype = (sym.owner.kind == TYP && sym.name != names._this &&
                    sym.name != names._super) ? site.memberType(sym) : sym.type;
            if (v.constValue != null && isStaticReference(tree))
                owntype = owntype.constType(v.constValue);
            break;

        case MTH:
            owntype = rs.instantiate(env, site, sym, pt.argtypes());
            if (owntype == null && Type.isDerivedRaw(pt.argtypes()) &&
                    !((AttrContext) env.info).rawArgs) {
                ((AttrContext) env.info).rawArgs = true;
                owntype = rs.instantiate(env, site, sym, pt.argtypes());
                ((AttrContext) env.info).rawArgs = false;
            }
            if (owntype == null) {
                log.error(tree.pos, "internal.error.cant.instantiate",
                        sym.toJava(), site.toJava(), Type.toJavaList(pt.argtypes()));
            }
            break;

        case PCK:

        case ERR:
            owntype = sym.type;
            break;

        default:
            throw new AssertionError("unexpected kind: " + sym.kind +
                    " in tree " + tree);

        }
        if (sym.name != names.init && (sym.flags() & DEPRECATED) != 0 &&
                (((AttrContext) env.info).scope.owner.flags() & DEPRECATED) ==
                0 && sym.outermostClass() !=
                ((AttrContext) env.info).scope.owner.outermostClass())
            chk.warnDeprecated(tree.pos, sym);
        return check(tree, owntype, sym.kind, pkind, pt);
    }

    /**
      * Check that variable is initialized and evaluate the variable's
      *  initializer, if not yet done. Also check that variable is not
      *  referenced before it is defined.
      *  @param tree    The tree making up the variable reference.
      *  @param env     The current environment.
      *  @param v       The variable's symbol.
      */
    private void checkInit(Ident tree, Env env, VarSymbol v) {
        if (v.pos > tree.pos && v.owner.kind == TYP &&
                canOwnInitializer(((AttrContext) env.info).scope.owner) &&
                v.owner == ((AttrContext) env.info).scope.owner.enclClass() &&
                ((v.flags() & STATIC) != 0) == Resolve.isStatic(env) &&
                (env.tree.tag != Tree.ASSIGN ||
                TreeInfo.skipParens(((Assign) env.tree).lhs) != tree))
            log.error(tree.pos, "illegal.forward.ref");
        evalInit(v);
    }

    /**
      * Can the given symbol be the owner of code which forms part
      *  if class initialization? This is the case if the symbol is
      *  a type or field, or if the symbol is the synthetic method.
      *  owning a block.
      */
    private boolean canOwnInitializer(Symbol sym) {
        return (sym.kind & (VAR | TYP)) != 0 ||
                (sym.kind == MTH && (sym.flags() & BLOCK) != 0);
    }

    /**
      * Evaluate a final variable's initializer, unless this has already been
      *  done, and set variable's constant value, if the initializer is
      *  constant.
      */
    public void evalInit(VarSymbol v) {
        if (v.constValue instanceof AttrContextEnv) {
            AttrContextEnv evalEnv = (AttrContextEnv) v.constValue;
            Name prev = log.useSource(evalEnv.toplevel.sourcefile);
            v.constValue = null;
            Type itype = attribExpr(((VarDef) evalEnv.tree).init, evalEnv, v.type);
            if (itype.constValue != null)
                v.constValue = cfolder.coerce(itype, v.type).constValue;
            log.useSource(prev);
        }
    }

    public void visitLiteral(Literal tree) {
        result = check(tree, litType(tree.typetag).constType(tree.value), VAL,
                pkind, pt);
    }

    /**
      * Return the type of a literal with given type tag.
      */
    Type litType(int tag) {
        return (tag == CLASS) ? syms.stringType : syms.typeOfTag[tag];
    }

    public void visitTypeIdent(TypeIdent tree) {
        result = check(tree, syms.typeOfTag[tree.typetag], TYP, pkind, pt);
    }

    public void visitTypeArray(TypeArray tree) {
        Type etype = attribType(tree.elemtype, env);
        result = check(tree, new ArrayType(etype, syms.arrayClass), TYP, pkind, pt);
    }

    public void visitErroneous(Erroneous tree) {
        result = tree.type = syms.errType;
    }

    /**
      * Default visitor method for all other trees.
      */
    public void visitTree(Tree tree) {
        throw new AssertionError();
    }

    /**
      * Main method: attribute class definition associated with given class symbol.
      *  reporting completion failures at the given position.
      *  @param pos The source position at which completion errors are to be
      *             reported.
      *  @param c   The class symbol whose definition will be attributed.
      */
    public void attribClass(int pos, ClassSymbol c) {
        try {
            attribClass(c);
        } catch (CompletionFailure ex) {
            chk.completionError(pos, ex);
        }
    }

    /**
      * Attribute class definition associated with given class symbol.
      *  @param c   The class symbol whose definition will be attributed.
      */
    void attribClass(ClassSymbol c) throws CompletionFailure {
        if (c.type.tag == ERROR)
            return;
        chk.checkNonCyclic(Position.NOPOS, c.type);
        Type st = c.type.supertype();
        if (st.tag == CLASS)
            attribClass((ClassSymbol) st.tsym);
        if (c.owner.kind == TYP && c.owner.type.tag == CLASS)
            attribClass((ClassSymbol) c.owner);
        if ((c.flags_field & UNATTRIBUTED) != 0) {
            c.flags_field &= ~UNATTRIBUTED;
            Env env = (Env) enter.classEnvs.remove(c);
            Name prev = log.useSource(c.sourcefile);
            try {
                attribClassBody(env, c);
            }
            finally { log.useSource(prev);
                    } }
    }

    /**
      * Finish the attribution of a class.
      */
    private void attribClassBody(Env env, ClassSymbol c) {
        ClassDef tree = (ClassDef) env.tree;
        assert c == tree.sym;
        chk.validate(tree.extending);
        chk.validate(tree.implementing);
        if ((c.flags() & (ABSTRACT | INTERFACE)) == 0) {
            if (!retrofit)
                chk.checkAllDefined(tree.pos, c);
        } else
            chk.checkCompatibleSupertypes(tree.pos, c.type);
        tree.type = c.type;
        chk.checkImplementations(tree);
        for (List l = tree.defs; l.nonEmpty(); l = l.tail) {
            attribStat((Tree) l.head, env);
            if (c.owner.kind != PCK &&
                    ((c.flags() & STATIC) == 0 || c.name == names.empty) &&
                    (TreeInfo.flags((Tree) l.head) & (STATIC | INTERFACE)) != 0) {
                Symbol sym = null;
                if (((Tree) l.head).tag == Tree.VARDEF)
                    sym = ((VarDef) l.head).sym;
                if (sym == null || sym.kind != VAR ||
                        ((VarSymbol) sym).constValue == null)
                    log.error(((Tree) l.head).pos, "icls.cant.have.static.decl");
            }
        }
        chk.checkCyclicConstructors(tree);
    }
}

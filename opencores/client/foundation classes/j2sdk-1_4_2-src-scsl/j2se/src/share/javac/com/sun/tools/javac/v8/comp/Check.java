/**
 * @(#)Check.java	1.70 03/02/26
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.tools.javac.v8.comp;
import com.sun.tools.javac.v8.util.*;

import com.sun.tools.javac.v8.code.*;

import com.sun.tools.javac.v8.tree.*;

import com.sun.tools.javac.v8.tree.Tree.*;

import com.sun.tools.javac.v8.code.Type.*;

import com.sun.tools.javac.v8.code.Symbol.*;


/**
 * Type checking helper class for the attribution phase.
 */
public class Check implements Kinds, Flags, TypeTags {
    private static final Context.Key checkKey = new Context.Key();
    private Name.Table names;
    private Log log;
    private Symtab syms;
    private Infer infer;

    public static Check instance(Context context) {
        Check instance = (Check) context.get(checkKey);
        if (instance == null)
            instance = new Check(context);
        return instance;
    }

    private Check(Context context) {
        super();
        context.put(checkKey, this);
        this.names = Name.Table.instance(context);
        this.log = Log.instance(context);
        this.syms = Symtab.instance(context);
        this.infer = Infer.instance(context);
        Options options = Options.instance(context);
        warnunchecked = options.get("-warnunchecked") != null;
        deprecation = options.get("-deprecation") != null;
    }

    /**
      * Switch: -warnunchecked option set?
      */
    boolean warnunchecked;

    /**
     * Switch: -deprecation option set?
     */
    boolean deprecation;

    /**
     * A table mapping flat names of all compiled classes in this run to their
     *  symbols; maintained from outside.
     */
    public Hashtable compiled = Hashtable.make();

    /**
     * Output variable: the source file where deprecated symbols were
     *  encountered. If deprecated symbols were encountered in more than one
     *  input file, the variable is set to "*".
     */
    public Name deprecatedSource;

    /**
     * Output variable: the source file where deprecated symbols were
     *  encountered. If deprecated symbols were encountered in more than one
     *  input file, the variable is set to "*".
     */
    public Name uncheckedSource;

    /**
     * Warn about deprecated symbol.
     *  @param pos        Position to be used for error reporting.
     *  @param sym        The deprecated symbol.
     */
    void warnDeprecated(int pos, Symbol sym) {
        if (deprecatedSource == null)
            deprecatedSource = log.currentSource();
        else if (deprecatedSource != log.currentSource())
            deprecatedSource = names.asterisk;
        if (deprecation)
            log.warning(pos, "has.been.deprecated", sym.toJava(), sym.javaLocation());
    }

    /**
      * Warn about unchecked operation.
      *  @param pos        Position to be used for error reporting.
      *  @param msg        A string describing the problem.
      */
    void warnUnchecked(int pos, String msg) {
        warnUnchecked(pos, msg, null, null, null, null);
    }

    void warnUnchecked(int pos, String msg, String arg0) {
        warnUnchecked(pos, msg, arg0, null, null, null);
    }

    void warnUnchecked(int pos, String msg, String arg0, String arg1) {
        warnUnchecked(pos, msg, arg0, arg1, null, null);
    }

    void warnUnchecked(int pos, String msg, String arg0, String arg1, String arg2) {
        warnUnchecked(pos, msg, arg0, arg1, arg2, null);
    }

    void warnUnchecked(int pos, String msg, String arg0, String arg1,
            String arg2, String arg3) {
        if (uncheckedSource == null)
            uncheckedSource = log.currentSource();
        else if (uncheckedSource != log.currentSource())
            uncheckedSource = names.asterisk;
        if (warnunchecked)
            log.warning(pos, msg, arg0, arg1, arg2, arg3);
    }

    /**
      * Report a failure to complete a class.
      *  @param pos        Position to be used for error reporting.
      *  @param ex         The failure to report.
      */
    public Type completionError(int pos, CompletionFailure ex) {
        log.error(pos, "cant.access", ex.sym.toJava(), ex.errmsg);
        if (ex instanceof ClassReader.BadClassFile)
            throw new Abort();
        else
            return syms.errType;
    }

    /**
      * Report a type error.
      *  @param pos        Position to be used for error reporting.
      *  @param problem    A string describing the error.
      *  @param found      The type that was found.
      *  @param req        The type that was required.
      */
    Type typeError(int pos, String problem, Type found, Type req) {
        log.error(pos, "prob.found.req", problem, found.toJava(), req.toJava());
        return syms.errType;
    }

    /**
      * Report an error that wrong type tag was found.
      *  @param pos        Position to be used for error reporting.
      *  @param required   An internationalized string describing the type tag
      *                    required.
      *  @param found      The type that was found.
      */
    Type typeTagError(int pos, String required, Type found) {
        log.error(pos, "type.found.req", found.toJava(), required);
        return syms.errType;
    }

    /**
      * Report an error that symbol cannot be referenced before super
      *  has been called.
      *  @param pos        Position to be used for error reporting.
      *  @param sym        The referenced symbol.
      */
    void earlyRefError(int pos, Symbol sym) {
        log.error(pos, "cant.ref.before.ctor.called", sym.toJava());
    }

    /**
      * Report an error that type parameter is not within its bound.
      *  @param pos                Position to be used for error reporting.
      *  @param t                  The type parameter.
      *  @param bound              Its bound.
      *  @param explanation        An internationalized string giving an
      *                            additional explanation.
      */
    void boundError(int pos, Type t, Type bound, String explanation) {
        if (explanation.equals(""))
            log.error(pos, "not.within.bounds", t.toJava());
        else
            log.error(pos, "not.within.bounds.explain", t.toJava(), explanation);
    }

    /**
      * Return name of local class.
      *  This is of the form    <topname> $ n $ <classname>
      *  where
      *    topname is the fully qualified name of the enclosing toplevel class,
      *    classname is the simple name of the local class
      */
    Name localClassName(ClassSymbol c) {
        ClassSymbol topClass = c.outermostClass();
        for (int i = 1; ; i++) {
            Name flatname = names.fromString(topClass.flatname + "$" + i + c.name);
            if (compiled.get(flatname) == null)
                return flatname;
        }
    }

    /**
      * Check that a given type is assignable to a given proto-type.
      *  If it is, return the type, otherwise return errType.
      *  @param pos        Position to be used for error reporting.
      *  @param found      The type that was found.
      *  @param req        The type that was required.
      */
    Type checkType(int pos, Type found, Type req) {
        if (req.tag == ERROR) {
            return req;
        } else if (req.tag == NONE) {
            return found;
        } else if (found.isAssignable(req)) {
            return found;
        } else {
            String problem;
            if (found.tag <= DOUBLE && req.tag <= DOUBLE)
                problem = log.getLocalizedString("possible.loss.of.precision");
            else
                problem = log.getLocalizedString("incompatible.types");
            return typeError(pos, problem, found, req);
        }
    }

    /**
      * Check that a given type can be cast to a given target type.
      *  Return the result of the cast.
      *  @param pos        Position to be used for error reporting.
      *  @param found      The type that is being cast.
      *  @param req        The target type of the cast.
      */
    Type checkCastable(int pos, Type found, Type req) {
        if (found.isCastable(req)) {
            checkCompatible(pos, found, req);
            return req;
        } else {
            return typeError(pos, log.getLocalizedString("inconvertible.types"),
                    found, req);
        }
    }

    /**
      * Check that type is different from 'void'.
      *  @param pos           Position to be used for error reporting.
      *  @param t             The type to be checked.
      */
    Type checkNonVoid(int pos, Type t) {
        if (t.tag == VOID) {
            log.error(pos, "void.not.allowed.here");
            return syms.errType;
        } else {
            return t;
        }
    }

    /**
      * Check that type is a class or interface type.
      *  @param pos           Position to be used for error reporting.
      *  @param t             The type to be checked.
      */
    Type checkClassType(int pos, Type t) {
        if (t.tag != CLASS && t.tag != ERROR)
            return typeTagError(pos, log.getLocalizedString("type.req.class"), t);
        else
            return t;
    }

    /**
      * Check that type is a class, interface or array type.
      *  @param pos           Position to be used for error reporting.
      *  @param t             The type to be checked.
      */
    Type checkClassOrArrayType(int pos, Type t) {
        if (t.tag != CLASS && t.tag != ARRAY && t.tag != ERROR)
            return typeTagError(pos,
                    log.getLocalizedString("type.req.class.array"), t);
        else
            return t;
    }

    /**
      * Check that type is a reference type, i.e. a class, interface or array type
      *  or a type variable.
      *  @param pos           Position to be used for error reporting.
      *  @param t             The type to be checked.
      */
    Type checkRefType(int pos, Type t) {
        if (t.tag != CLASS && t.tag != ARRAY && t.tag != ERROR)
            return typeTagError(pos, log.getLocalizedString("type.req.ref"), t);
        else
            return t;
    }

    /**
      * Check that flag set does not contain elements of two conflicting sets. s
      *  Return true if it doesn't.
      *  @param pos           Position to be used for error reporting.
      *  @param flags         The set of flags to be checked.
      *  @param set1          Conflicting flags set #1.
      *  @param set2          Conflicting flags set #2.
      */
    boolean checkDisjoint(int pos, long flags, long set1, long set2) {
        if ((flags & set1) != 0 && (flags & set2) != 0) {
            log.error(pos, "illegal.combination.of.modifiers",
                    TreeInfo.flagNames(TreeInfo.firstFlag(flags & set1)),
                    TreeInfo.flagNames(TreeInfo.firstFlag(flags & set2)));
            return false;
        } else
            return true;
    }

    /**
      * Check that given modifiers are legal for given symbol and
      *  return modifiers together with any implicit modififiers for that symbol.
      *  Warning: we can't use flags() here since this method
      *  is called during class enter, when flags() would cause a premature
      *  completion.
      *  @param pos           Position to be used for error reporting.
      *  @param flags         The set of modifiers given in a definition.
      *  @param sym           The defined symbol.
      */
    long checkFlags(int pos, long flags, Symbol sym) {
        long mask;
        long implicit = 0;
        switch (sym.kind) {
        case VAR:
            if (sym.owner.kind != TYP)
                mask = LocalVarFlags;
            else if ((sym.owner.flags_field & INTERFACE) != 0)
                mask = implicit = InterfaceVarFlags;
            else
                mask = VarFlags;
            break;

        case MTH:
            if (sym.name == names.init)
                mask = ConstructorFlags;
            else if ((sym.owner.flags_field & INTERFACE) != 0)
                mask = implicit = InterfaceMethodFlags;
            else {
                mask = MethodFlags;
            }
            if (((flags | implicit) & Flags.ABSTRACT) == 0)
                implicit |= sym.owner.flags_field & STRICTFP;
            break;

        case TYP:
            if (sym.isLocal()) {
                mask = LocalClassFlags;
                if (sym.name.len == 0)
                    mask |= STATIC;
            } else if (sym.owner.kind == TYP) {
                mask = MemberClassFlags;
                if (sym.owner.owner.kind == PCK ||
                        (sym.owner.flags_field & STATIC) != 0)
                    mask |= STATIC;
                if ((flags & INTERFACE) != 0)
                    implicit = STATIC;
            } else {
                mask = ClassFlags;
            }
            if ((flags & INTERFACE) != 0)
                implicit |= ABSTRACT;
            implicit |= sym.owner.flags_field & STRICTFP;
            break;

        default:
            throw new AssertionError();

        }
        long illegal = flags & StandardFlags & ~mask;
        if (illegal != 0)
            log.error(pos, "mod.not.allowed.here", TreeInfo.flagNames(illegal));
        else if ( (sym.kind == TYP ||
                checkDisjoint(pos, flags, ABSTRACT, PRIVATE | STATIC)) &&
                checkDisjoint(pos, flags, ABSTRACT | INTERFACE,
                FINAL | NATIVE | SYNCHRONIZED) &&
                checkDisjoint(pos, flags, PUBLIC, PRIVATE | PROTECTED) &&
                checkDisjoint(pos, flags, PRIVATE, PUBLIC | PROTECTED) &&
                checkDisjoint(pos, flags, FINAL, VOLATILE) && (sym.kind == TYP ||
                checkDisjoint(pos, flags, ABSTRACT | NATIVE, STRICTFP))) {
        }
        return flags & (mask | ~StandardFlags) | implicit;
    }

    /**
      * Validate a type expression. That is,
      *  check that all type arguments of a parametric type are within
      *  their bounds. This must be done in a second phase after type attributon
      *  since a class might have a subclass as type parameter bound. E.g:
      *
      *  class B<A extends C> { ... }
      *  class C extends B<C> { ... }
      *
      *  and we can't make sure that the bound is already attributed because
      *  of possible cycles.
      */
    private Validator validator = new Validator();

    /**
     * Visitor method: Validate a type expression, if it is not null, catching
     *  and reporting any completion failures.
     */
    void validate(Tree tree) {
        try {
            if (tree != null)
                tree.accept(validator);
        } catch (CompletionFailure ex) {
            completionError(tree.pos, ex);
        }
    }

    /**
      * Visitor method: Validate a list of type expressions.
      */
    void validate(List trees) {
        for (List l = trees; l.nonEmpty(); l = l.tail)
            validate((Tree) l.head);
    }

    /**
      * A visitor class for type validation.
      */
    class Validator extends Visitor {

        Validator() {
            super();
        }

        public void visitTypeArray(TypeArray tree) {
            validate(tree.elemtype);
        }

        public void visitSelect(Select tree) {
            if (tree.type.tag == CLASS) {
                if (tree.type.outer().tag == CLASS)
                    validate(tree.selected);
                else if (tree.selected.type.isParameterized())
                    log.error(tree.pos, "cant.select.static.class.from.param.type");
                if (tree.type.isRaw() && tree.type.allparams().nonEmpty())
                    log.error(tree.pos, "improperly.formed.type.param.missing");
            }
        }

        /**
          * Default visitor method: do nothing.
          */
        public void visitTree(Tree tree) {
        }
    }

    /**
      * Is given type a subtype of some of the types in given list?
      */
    static boolean subset(Type t, List ts) {
        for (List l = ts; l.nonEmpty(); l = l.tail)
            if (t.isSubType((Type) l.head))
                return true;
        return false;
    }

    /**
      * Is given type a subtype or supertype of
      *  some of the types in given list?
      */
    static boolean intersects(Type t, List ts) {
        for (List l = ts; l.nonEmpty(); l = l.tail)
            if (t.isSubType((Type) l.head) || ((Type) l.head).isSubType(t))
                return true;
        return false;
    }

    /**
      * Add type set to given type list, unless it is a subclass of some class
      *  in the list.
      */
    static List incl(Type t, List ts) {
        return subset(t, ts) ? ts : excl(t, ts).prepend(t);
    }

    /**
      * Remove type set from type set list.
      */
    static List excl(Type t, List ts) {
        if (ts.isEmpty()) {
            return ts;
        } else {
            List ts1 = excl(t, ts.tail);
            if (((Type) ts.head).isSubType(t))
                return ts1;
            else if (ts1 == ts.tail)
                return ts;
            else
                return ts1.prepend(ts.head);
        }
    }

    /**
      * Form the union of two type set lists.
      */
    static List union(List ts1, List ts2) {
        List ts = ts1;
        for (List l = ts2; l.nonEmpty(); l = l.tail)
            ts = incl((Type) l.head, ts);
        return ts;
    }

    /**
      * Form the difference of two type lists.
      */
    static List diff(List ts1, List ts2) {
        List ts = ts1;
        for (List l = ts2; l.nonEmpty(); l = l.tail)
            ts = excl((Type) l.head, ts);
        return ts;
    }

    /**
      * Form the intersection of two type lists.
      */
    static List intersect(List ts1, List ts2) {
        List ts = Type.emptyList;
        for (List l = ts1; l.nonEmpty(); l = l.tail)
            if (subset((Type) l.head, ts2))
                ts = incl((Type) l.head, ts);
        for (List l = ts2; l.nonEmpty(); l = l.tail)
            if (subset((Type) l.head, ts1))
                ts = incl((Type) l.head, ts);
        return ts;
    }

    /**
      * Is exc an exception symbol that need not be declared?
      */
    boolean isUnchecked(ClassSymbol exc) {
        return exc.kind == ERR || exc.isSubClass(syms.errorType.tsym) ||
                exc.isSubClass(syms.runtimeExceptionType.tsym);
    }

    /**
      * Is exc an exception type that need not be declared?
      */
    boolean isUnchecked(Type exc) {
        return (exc.tag == CLASS) ? isUnchecked((ClassSymbol) exc.tsym) : false;
    }

    /**
      * Same, but handling completion failures.
      */
    boolean isUnchecked(int pos, Type exc) {
        try {
            return isUnchecked(exc);
        } catch (CompletionFailure ex) {
            completionError(pos, ex);
            return true;
        }
    }

    /**
      * Is exc handled by given exception list?
      */
    boolean isHandled(Type exc, List handled) {
        return isUnchecked(exc) || subset(exc, handled);
    }

    /**
      * Return all exceptions in thrown list that are not in handled list.
      *  @param thrown     The list of thrown exceptions.
      *  @param handled    The list of handled exceptions.
      */
    List unHandled(List thrown, List handled) {
        List unhandled = Type.emptyList;
        for (List l = thrown; l.nonEmpty(); l = l.tail)
            if (!isHandled((Type) l.head, handled))
                unhandled = unhandled.prepend(l.head);
        return unhandled;
    }

    /**
      * The level of access protection given by a flag set,
      *  where PRIVATE is highest and PUBLIC is lowest.
      */
    static int protection(long flags) {
        switch ((short)(flags & AccessFlags)) {
        case PRIVATE:
            return 3;

        case PROTECTED:
            return 1;

        case PUBLIC:
            return 0;

        default:
            return 2;

        }
    }

    /**
      * A string describing the access permission given by a flag set.
      *  This always returns a space-separated list of Java Keywords.
      */
    private static String protectionString(long flags) {
        long flags1 = flags & AccessFlags;
        return (flags1 == 0) ? "package" : TreeInfo.flagNames(flags1);
    }

    /**
      * A customized "cannot override" error message.
      *  @param m      The overriding method.
      *  @param other  The overridden method.
      *  @return       An internationalized string.
      */
    static String cannotOverride(MethodSymbol m, MethodSymbol other) {
        String msg;
        if ((other.owner.flags() & INTERFACE) == 0)
            msg = Log.getLocalizedString("cant.override", m.toJava(),
                    m.javaLocation(), other.toJava(), other.javaLocation());
        else if ((m.owner.flags() & INTERFACE) == 0)
            msg = Log.getLocalizedString("cant.implement", m.toJava(),
                    m.javaLocation(), other.toJava(), other.javaLocation());
        else
            msg = Log.getLocalizedString("clashes.with", m.toJava(),
                    m.javaLocation(), other.toJava(), other.javaLocation());
        return msg;
    }

    /**
      * Check that this method conforms with overridden method 'other'.
      *  where `origin' is the class where checking started.
      *  Complications:
      *  (1) Do not check overriding of synthetic methods
      *      (reason: they might be final).
      *      todo: check whether this is still necessary.
      *  (2) Admit the case where an interface proxy throws fewer exceptions
      *      than the method it implements. Augment the proxy methods with the
      *      undeclared exceptions in this case.
      *  (3) In GJ, admit the case where an interface proxy has a result type
      *      extended by the result type of the method it implements.
      *      Change the proxies result type to the smaller type in this case.
      *
      *  @param tree         The tree from which positions
      *			    are extracted for errors.
      *  @param m            The overriding method.
      *  @param other        The overridden method.
      *  @param origin       The class of which the overriding method
      *			    is a member.
      */
    void checkOverride(Tree tree, MethodSymbol m, MethodSymbol other,
            ClassSymbol origin) {
        if ((other.flags() & SYNTHETIC) != 0) {
        } else if ((m.flags() & STATIC) != 0 && (other.flags() & STATIC) == 0) {
            log.error(TreeInfo.positionFor(m, tree), "override.static",
                    cannotOverride(m, other));
        } else if ((other.flags() & FINAL) != 0 || (m.flags() & STATIC) == 0 &&
                (other.flags() & STATIC) != 0) {
            log.error(TreeInfo.positionFor(m, tree), "override.meth",
                    cannotOverride(m, other),
                    TreeInfo.flagNames(other.flags() & (FINAL | STATIC)));
        } else if ((origin.flags() & INTERFACE) == 0 &&
                protection(m.flags()) > protection(other.flags())) {
            log.error(TreeInfo.positionFor(m, tree), "override.weaker.access",
                    cannotOverride(m, other), protectionString(other.flags()));
        } else {
            Type mt = origin.type.memberType(m);
            Type ot = origin.type.memberType(other);
            List mtvars = mt.typarams();
            List otvars = ot.typarams();
            Type otres = ot.restype().subst(otvars, mtvars);
            boolean resultTypesOK = mt.restype().isSameType(otres);
            if (!resultTypesOK) {
                typeError(TreeInfo.positionFor(m, tree),
                        log.getLocalizedString("override.incompatible.ret",
                        cannotOverride(m, other)), mt.restype(),
                        ot.restype().subst(otvars, mtvars));
            } else {
                List otthrown = Type.subst(ot.thrown(), otvars, mtvars);
                List unhandled = unHandled(mt.thrown(), otthrown);
                if (unhandled.nonEmpty()) {
                    log.error(TreeInfo.positionFor(m, tree), "override.meth.doesnt.throw",
                            cannotOverride(m, other),
                            ((Type) unhandled.head).toJava());
                }
            }
            if ((other.flags() & DEPRECATED) != 0 && (m.flags() & DEPRECATED) == 0 &&
                    m.outermostClass() != other.outermostClass() &&
                    m.enclClass().type.supertype().tsym == other.enclClass())
                warnDeprecated(TreeInfo.positionFor(m, tree), other);
        }
    }

    /**
      * Check that a given method conforms with any method it overrides.
      *  @param tree         The tree from which positions are extracted
      *			    for errors.
      *  @param m            The overriding method.
      */
    void checkOverride(Tree tree, MethodSymbol m) {
        ClassSymbol origin = (ClassSymbol) m.owner;
        for (Type t = origin.type.supertype(); t.tag == CLASS; t = t.supertype()) {
            TypeSymbol c = t.tsym;
            Scope.Entry e = c.members().lookup(m.name);
            while (e.scope != null) {
                if (m.overrides(e.sym, origin))
                    checkOverride(tree, m, (MethodSymbol) e.sym, origin);
                e = e.next();
            }
        }
    }

    /**
      * Return the first method which is defined with same args
      *  but different return types in two given interfaces, or null if none
      *  exists.
      *  @param t1     The first interface.
      *  @param t2     The second interface.
      *  @param site   The most derived type.
      */
    public Symbol firstIncompatibility(Type t1, Type t2, Type site) {
        return firstIncompatibility(t1, t2, t2, site);
    }

    /**
      * Return the first method which is defined with same args
      *  but different return types in a type `t1' and a supertype
      *  `t2super' of a type `t2', or null if none exists.
      *  @param t1       The first type.
      *  @param t2       The second type.
      *  @param t2super  The second's supertype.
      *  @param site     The most derived type.
      */
    public Symbol firstIncompatibility(Type t1, Type t2, Type t2super, Type site) {
        if ((t2super.tsym.flags() & ABSTRACT) == 0)
            return null;
        for (Scope.Entry e = t2super.tsym.members().elems; e != null; e = e.sibling) {
            if (e.sym.kind == MTH && (e.sym.flags() & ABSTRACT) != 0 &&
                    (site == null || e.sym.isInheritedIn(site.tsym)) &&
                    !isCompatible(t1, t1, t2, e.sym))
                return e.sym;
        }
        for (List is = t2super.interfaces(); is.nonEmpty(); is = is.tail) {
            Symbol sym = firstIncompatibility(t1, t2, (Type) is.head, site);
            if (sym != null)
                return sym;
        }
        return (t2super.supertype().tag == CLASS) ?
                firstIncompatibility(t1, t2, t2super.supertype(), site) : null;
    }

    /**
      * Check that supertype `t1super' of type `t1' does not define
      *  an abstract method with with same name and argument types but
      *  a different return type than its definition in type `t2'.
      *  @param t1       The first type.
      *  @param t1super  The first type's supertype.
      *  @param t2       The second type.
      *  @param sym      The method symbol.
      */
    private boolean isCompatible(Type t1, Type t1super, Type t2, Symbol sym) {
        for (Scope.Entry e = t1super.tsym.members().lookup(sym.name);
                e.scope != null; e = e.next()) {
            if (e.sym.kind == MTH && (e.sym.flags() & ABSTRACT) != 0) {
                Type symt1 = t1.memberType(e.sym);
                Type symt2 = t2.memberType(sym);
                if (symt1.hasSameArgs(symt2)) {
                    List tvars1 = symt1.typarams();
                    List tvars2 = symt2.typarams();
                    Type rt1 = symt1.restype();
                    Type rt2 = symt2.restype().subst(tvars2, tvars1);
                    return rt1.isSameType(rt2);
                }
            }
        }
        for (List is = t1super.interfaces(); is.nonEmpty(); is = is.tail) {
            if (!isCompatible(t1, (Type) is.head, t2, sym))
                return false;
        }
        return (t1super.supertype().tag == CLASS &&
                (t1super.supertype().tsym.flags() & ABSTRACT) != 0) ?
                isCompatible(t1, t1super.supertype(), t2, sym) : true;
    }

    /**
      * Check that (arrays of) interfaces do not each define a method
      *  with same name and arguments but different return types.
      *  If either argument type is not an (array of) interface type, do
      *  nothing.
      *  @param pos          Position to be used for error reporting.
      *  @param t1           The first argument type.
      *  @param t2           The second argument type.
      */
    public boolean checkCompatible(int pos, Type t1, Type t2) {
        if (t1.tag == ARRAY && t2.tag == ARRAY) {
            checkCompatible(pos, t1.elemtype(), t2.elemtype());
        } else if (t1.tag == CLASS && (t1.tsym.flags() & INTERFACE) != 0 &&
                t2.tag == CLASS && (t2.tsym.flags() & INTERFACE) != 0) {
            return checkCompatibleAbstracts(pos, t1, t2, null);
        }
        return true;
    }

    /**
      * Check that classes (or interfaces) do not each define an abstract
      *  method with same name and arguments but different return types.
      *  @param pos          Position to be used for error reporting.
      *  @param t1           The first argument type.
      *  @param t2           The second argument type.
      */
    public boolean checkCompatibleAbstracts(int pos, Type t1, Type t2, Type site) {
        Symbol sym = firstIncompatibility(t1, t2, site);
        if (sym != null) {
            log.error(pos, "types.incompatible.diff.ret", t1.toJava(), t2.toJava(),
                    sym.name + "(" + t2.memberType(sym).argtypes() + ")");
            return false;
        }
        return true;
    }

    /**
      * Check that all abstract members of given class have definitions.
      *  @param pos          Position to be used for error reporting.
      *  @param c            The class.
      */
    void checkAllDefined(int pos, ClassSymbol c) {
        try {
            MethodSymbol undef = firstUndef(c, c);
            if (undef != null) {
                MethodSymbol undef1 = new MethodSymbol(undef.flags(), undef.name,
                        c.type.memberType(undef), undef.owner);
                log.error(pos, "does.not.override.abstract", c.toJava(),
                        undef1.toJava(), undef1.javaLocation());
            }
        } catch (CompletionFailure ex) {
            completionError(pos, ex);
        }
    }

    /**
      * Return first abstract member of class `c' that is not defined
      *  in `impl', null if there is none.
      */
    private MethodSymbol firstUndef(ClassSymbol impl, ClassSymbol c) {
        MethodSymbol undef = null;
        if (c == impl || (c.flags() & (ABSTRACT | INTERFACE)) != 0) {
            Scope s = c.members();
            for (Scope.Entry e = s.elems; undef == null && e != null; e = e.sibling) {
                if (e.sym.kind == MTH &&
                        (e.sym.flags() & (ABSTRACT | IPROXY)) == ABSTRACT) {
                    MethodSymbol absmeth = (MethodSymbol) e.sym;
                    MethodSymbol implmeth = absmeth.implementation(impl);
                    if (implmeth == null || implmeth == absmeth)
                        undef = absmeth;
                }
            }
            if (undef == null) {
                Type st = c.type.supertype();
                if (st.tag == CLASS)
                    undef = firstUndef(impl, (ClassSymbol) st.tsym);
            }
            for (List l = c.type.interfaces(); undef == null && l.nonEmpty();
                    l = l.tail) {
                undef = firstUndef(impl, (ClassSymbol)((Type) l.head).tsym);
            }
        }
        return undef;
    }

    /**
      * Check for cyclic references. Issue an error if the
      *  symbol of the type referred to has a LOCKED flag set.
      *
      *  @param pos      Position to be used for error reporting.
      *  @param t        The type referred to.
      */
    Type checkNonCyclic(int pos, Type t) {
        Symbol c = t.tsym;
        if ((c.flags_field & LOCKED) != 0) {
            log.error(pos, "cyclic.inheritance", c.toJava());
            t = new ErrorType((ClassSymbol) c);
        } else if ((c.flags_field & ACYCLIC) != 0) {
            return t;
        } else if (!c.type.isErroneous()) {
            try {
                c.flags_field |= LOCKED;
                for (List l = c.type.interfaces(); l.nonEmpty(); l = l.tail)
                    l.head = checkNonCyclic(pos, (Type) l.head);
                Type st = c.type.supertype();
                if (st != null && st.tag == CLASS)
                    ((ClassType) c.type).supertype_field = checkNonCyclic(pos, st);
            }
            finally { c.flags_field &= ~LOCKED;
                    } }
        c.flags_field |= ACYCLIC;
        return t;
    }

    /**
      * Check that all methods which implement some
      *  method conform to the method they implement.
      *  @param tree         The class definition whose members are checked.
      */
    void checkImplementations(ClassDef tree) {
        checkImplementations(tree, tree.sym);
    }

    /**
      * Check that all methods which implement some
      *  method in `ic' conform to the method they implement.
      */
    void checkImplementations(ClassDef tree, ClassSymbol ic) {
        ClassSymbol origin = tree.sym;
        if ((origin != ic) && (ic.flags() & ABSTRACT) != 0) {
            for (Scope.Entry e = ic.members().elems; e != null; e = e.sibling) {
                if (e.sym.kind == MTH &&
                        (e.sym.flags() & (STATIC | ABSTRACT)) == ABSTRACT) {
                    MethodSymbol absmeth = (MethodSymbol) e.sym;
                    MethodSymbol implmeth = absmeth.implementation(origin);
                    if (implmeth != null && implmeth != absmeth &&
                            (implmeth.owner.flags() & INTERFACE) ==
                            (origin.flags() & INTERFACE)) {
                        checkOverride(tree, implmeth, absmeth, origin);
                    }
                }
            }
        }
        Type st = ic.type.supertype();
        if (st.tag == CLASS)
            checkImplementations(tree, (ClassSymbol) st.tsym);
        for (List l = ic.type.interfaces(); l.nonEmpty(); l = l.tail)
            checkImplementations(tree, (ClassSymbol)((Type) l.head).tsym);
    }

    /**
      * Check that all abstract methods implemented by a class are
      *  mutually compatible.
      *  @param pos          Position to be used for error reporting.
      *  @param c            The class whose interfaces are checked.
      */
    void checkCompatibleSupertypes(int pos, Type c) {
        List supertypes = c.interfaces();
        Type supertype = c.supertype();
        if (supertype.tag == CLASS && (supertype.tsym.flags() & ABSTRACT) != 0)
            supertypes = supertypes.prepend(supertype);
        for (List l = supertypes; l.nonEmpty(); l = l.tail) {
            for (List m = supertypes; m != l; m = m.tail)
                if (!checkCompatibleAbstracts(pos, (Type) l.head, (Type) m.head, c))
                    return;
        }
    }

    /**
      * Enter interface into into set.
      *  If it existed already, issue a "repeated interface" error.
      */
    void checkNotRepeated(int pos, Type it, Set its) {
        if (its.contains(it))
            log.error(pos, "repeated.interface");
        else {
            its.put(it);
        }
    }

    /**
      * Check for cycles in the graph of constructors calling other
      *  constructors.
      */
    void checkCyclicConstructors(ClassDef tree) {
        Hashtable callMap = new Hashtable();
        for (List l = tree.defs; l.nonEmpty(); l = l.tail) {
            Apply app = TreeInfo.firstConstructorCall((Tree) l.head);
            if (app == null)
                continue;
            MethodDef meth = (MethodDef) l.head;
            if (TreeInfo.name(app.meth) == names._this) {
                callMap.put(meth.sym, TreeInfo.symbol(app.meth));
            } else {
                meth.sym.flags_field |= ACYCLIC;
            }
        }
        for (List callers = callMap.keys(); callers.nonEmpty();
                callers = callers.tail) {
            checkCyclicConstructor(tree, (Symbol) callers.head, callMap);
        }
    }

    /**
      * Look in the map to see if the given constructor is part of a
      *  call cycle.
      */
    private void checkCyclicConstructor(ClassDef tree, Symbol ctor,
            Hashtable callMap) {
        if (ctor != null && (ctor.flags_field & ACYCLIC) == 0) {
            if ((ctor.flags_field & LOCKED) != 0) {
                log.error(TreeInfo.positionFor(ctor, tree), "recursive.ctor.invocation");
            } else {
                ctor.flags_field |= LOCKED;
                checkCyclicConstructor(tree, (Symbol) callMap.remove(ctor), callMap);
                ctor.flags_field &= ~LOCKED;
            }
            ctor.flags_field |= ACYCLIC;
        }
    }

    /**
      * Check that a qualified name is in canonical form (for import decls).
      */
    public void checkCanonical(Tree tree) {
        if (!isCanonical(tree))
            log.error(tree.pos, "import.requires.canonical",
                    TreeInfo.symbol(tree).toJava());
    }

    private boolean isCanonical(Tree tree) {
        while (tree.tag == Tree.SELECT) {
            Select s = (Select) tree;
            if (s.sym.owner != TreeInfo.symbol(s.selected))
                return false;
            tree = s.selected;
        }
        return true;
    }
}

/**
 * @(#)Resolve.java	1.67 03/05/06
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.tools.javac.v8.comp;
import com.sun.tools.javac.v8.util.*;

import com.sun.tools.javac.v8.code.*;

import com.sun.tools.javac.v8.tree.*;

import com.sun.tools.javac.v8.code.Type.*;

import com.sun.tools.javac.v8.code.Symbol.*;

import com.sun.tools.javac.v8.tree.Tree.*;


/**
 * Helper class for name resolution, used mostly by the attribution phase.
 */
public class Resolve implements Flags, Kinds, TypeTags {
    private static final Context.Key resolveKey = new Context.Key();
    private Name.Table names;
    private Log log;
    private Symtab syms;
    private Check chk;
    private Infer infer;
    private ClassReader reader;
    private TreeInfo treeinfo;

    public static Resolve instance(Context context) {
        Resolve instance = (Resolve) context.get(resolveKey);
        if (instance == null)
            instance = new Resolve(context);
        return instance;
    }

    private Resolve(Context context) {
        super();
        context.put(resolveKey, this);
        syms = Symtab.instance(context);
        varNotFound = new ResolveError(ABSENT_VAR, syms.errSymbol, "variable not found");
        wrongMethod = new ResolveError(WRONG_MTH, syms.errSymbol, "method not found");
        wrongMethods = new ResolveError(WRONG_MTHS, syms.errSymbol, "wrong methods");
        methodNotFound = new ResolveError(ABSENT_MTH, syms.errSymbol, "method not found");
        typeNotFound = new ResolveError(ABSENT_TYP, syms.errSymbol, "type not found");
        names = Name.Table.instance(context);
        log = Log.instance(context);
        chk = Check.instance(context);
        infer = Infer.instance(context);
        reader = ClassReader.instance(context);
        treeinfo = TreeInfo.instance(context);
    }

    /**
      * Error kinds that complement the constants in Kinds
      */
    static final int AMBIGUOUS = 256;
    static final int HIDDEN = 257;
    static final int ABSENT_VAR = 258;
    static final int WRONG_MTHS = 259;
    static final int WRONG_MTH = 260;
    static final int ABSENT_MTH = 261;
    static final int ABSENT_TYP = 262;

    /**
     * error symbols, which are returned when resolution fails
     */
    final ResolveError varNotFound;
    final ResolveError wrongMethod;
    final ResolveError wrongMethods;
    final ResolveError methodNotFound;
    final ResolveError typeNotFound;

    /**
     * An environment is "static" if its static level is greater than
     *  the one of its outer environment
     */
    static boolean isStatic(Env env) {
        return ((AttrContext) env.info).staticLevel >
                ((AttrContext) env.outer.info).staticLevel;
    }

    /**
      * The static level of a symbol is the number of prefixing static flags
      *  for the symbol and any enclosing class or member declarations.
      */
    static int staticLevel(Symbol sym) {
        int level = 0;
        do {
            if ((sym.flags() & STATIC) != 0)
                level++;
            sym = sym.owner;
        } while (sym.kind != PCK)
            ;
        return level;
    }

    /**
      * Is class accessible in given evironment?
      *  @param env    The current environment.
      *  @param c      The class whose accessibility is checked.
      */
    boolean isAccessible(Env env, TypeSymbol c) {
        switch ((short)(c.flags() & AccessFlags)) {
        case PRIVATE:
            return env.enclClass.sym.outermostClass() == c.owner.outermostClass();

        case 0:
            return env.toplevel.packge == c.owner ||
                    env.toplevel.packge == c.packge() || env.enclMethod != null &&
                    (env.enclMethod.flags & ANONCONSTR) != 0;

        case PUBLIC:
            return true;

        case PROTECTED:
            return env.toplevel.packge == c.owner ||
                    env.toplevel.packge == c.packge() ||
                    isInnerSubClass(env.enclClass.sym, c.owner);

        default:
            return true;

        }
    }

    /**
      * Is given class a subclass of given base class, or an inner class
      *  of a subclass?
      *  Return null if no such class exists.
      *  @param c     The class which is the subclass or is contained in it.
      *  @param base  The base class
      */
    private boolean isInnerSubClass(ClassSymbol c, Symbol base) {
        while (c != null && !c.isSubClass(base)) {
            c = c.owner.enclClass();
        }
        return c != null;
    }

    boolean isAccessible(Env env, Type t) {
        return (t.tag == ARRAY) ? isAccessible(env, t.elemtype()) :
                isAccessible(env, t.tsym);
    }

    /**
      * Is symbol accessible as a member of given type in given evironment?
      *  @param env    The current environment.
      *  @param site   The type of which the tested symbol is regarded
      *                as a member.
      *  @param sym    The symbol.
      */
    boolean isAccessible(Env env, Type site, Symbol sym) {
        if (sym.name == names.init && sym.owner != site.tsym)
            return false;
        ClassSymbol sub;
        switch ((short)(sym.flags() & AccessFlags)) {
        case PRIVATE:
            return (env.enclClass.sym == sym.owner ||
                    env.enclClass.sym.outermostClass() ==
                    sym.owner.outermostClass()) && sym.isInheritedIn(site.tsym);

        case 0:
            return (env.toplevel.packge == sym.owner.owner ||
                    env.toplevel.packge == sym.packge()) &&
                    sym.isInheritedIn(site.tsym);

        case PROTECTED:
            return (env.toplevel.packge == sym.owner.owner ||
                    env.toplevel.packge == sym.packge() ||
                    isProtectedAccessible(sym, env.enclClass.sym, site) ||
                    ((AttrContext) env.info).selectSuper &&
                    (sym.flags() & STATIC) == 0 && sym.kind != TYP) &&
                    (sym.kind != MTH || sym.isConstructor() ||
                    ((MethodSymbol) sym).implementation(site.tsym) == sym);

        default:
            return isAccessible(env, site);

        }
    }

    /**
      * Is given protected symbol accessible if it is selected from given site
      *  and the selection takes place in given class?
      *  @param sym     The symbol with protected access
      *  @param c       The class where the access takes place
      *  @site          The type of the qualifier
      */
    private boolean isProtectedAccessible(Symbol sym, ClassSymbol c, Type site) {
        while (c != null &&
                !(c.isSubClass(sym.owner) && (c.flags() & INTERFACE) == 0 &&
                ((sym.flags() & STATIC) != 0 || sym.kind == TYP ||
                site.tsym.isSubClass(c))))
            c = c.owner.enclClass();
        return c != null;
    }

    /**
      * Try to instantiate the type of a method so that it fits
      *  given type parameters and argument types. If succesful, return
      *  the method's instantiated type, else return null.
      *  The instantiation will take into account an additional leading
      *  formal parameter if the method is an instance method seen as a member
      *  of un underdetermined site In this case, we treat site as an additional
      *  parameter and the parameters of the class containing the method as
      *  additional type variables that get instantiated.
      *
      *  @param env         The current environment
      *  @param site        The type of which the method is a member.
      *  @param m           The method symbol.
      *  @param argtypes    The invocation's given value parameters.
      */
    Type instantiate(Env env, Type site, Symbol m, List argtypes) {
        Type mt = site.memberType(m);
        List formals = mt.argtypes();
        return (Type.isSubTypes(argtypes, formals)) ? mt : null;
    }

    /**
      * Find field. Synthetic fields are always skipped.
      *  @param env     The current environment.
      *  @param site    The original type from where the selection takes place.
      *  @param name    The name of the field.
      *  @param c       The class to search for the field. This is always
      *                 a superclass or implemented interface of site's class.
      */
    Symbol findField(Env env, Type site, Name name, TypeSymbol c) {
        Symbol bestSoFar = varNotFound;
        Symbol sym;
        Scope.Entry e = c.members().lookup(name);
        while (e.scope != null) {
            if (e.sym.kind == VAR && (e.sym.flags_field & SYNTHETIC) == 0) {
                if (isAccessible(env, site, e.sym))
                    return e.sym;
                else
                    return new AccessError(e.sym);
            }
            e = e.next();
        }
        Type st = c.type.supertype();
        if (st != null && st.tag == CLASS) {
            sym = findField(env, site, name, st.tsym);
            if (sym.kind < bestSoFar.kind)
                bestSoFar = sym;
        }
        for (List l = c.type.interfaces();
                bestSoFar.kind != AMBIGUOUS && l.nonEmpty(); l = l.tail) {
            sym = findField(env, site, name, ((Type) l.head).tsym);
            if (bestSoFar.kind < AMBIGUOUS && sym.kind < AMBIGUOUS &&
                    sym.owner != bestSoFar.owner)
                bestSoFar = new AmbiguityError(bestSoFar, sym);
            else if (sym.kind < bestSoFar.kind)
                bestSoFar = sym;
        }
        return bestSoFar;
    }

    /**
      * Find unqualified variable or field with given name.
      *  Synthetic fields always skipped.
      *  @param env     The current environment.
      *  @param name    The name of the variable or field.
      */
    Symbol findVar(Env env, Name name) {
        Symbol bestSoFar = varNotFound;
        Symbol sym;
        Env env1 = env;
        boolean staticOnly = false;
        while (env1.outer != null) {
            if (isStatic(env1))
                staticOnly = true;
            Scope.Entry e = ((AttrContext) env1.info).scope.lookup(name);
            while (e.scope != null &&
                    (e.sym.kind != VAR || (e.sym.flags_field & SYNTHETIC) != 0))
                e = e.next();
            sym = (e.scope != null) ? e.sym :
                    findField(env1, env1.enclClass.sym.type, name,
                    env1.enclClass.sym);
            if (sym.kind <= AMBIGUOUS) {
                if (staticOnly && sym.kind == VAR && sym.owner.kind == TYP &&
                        (sym.flags() & STATIC) == 0)
                    return new StaticError(sym);
                else
                    return sym;
            } else if (sym.kind < bestSoFar.kind) {
                bestSoFar = sym;
            }
            if ((env1.enclClass.sym.flags() & STATIC) != 0)
                staticOnly = true;
            env1 = env1.outer;
        }
        sym = findField(env, syms.predefClass.type, name, syms.predefClass);
        if (sym.kind <= AMBIGUOUS)
            return sym;
        else
            return bestSoFar;
    }

    /**
      * Select the best method for a call site among two choices.
      *  @param env		The current environment.
      *  @param site		The original type from where the selection takes place.
      *  @param argtypes		The invocation's value parameters,
      *  @param sym		Proposed new best match.
      *  @param bestSoFar	Previously found best match.
      */
    Symbol selectBest(Env env, Type site, List argtypes, Symbol sym,
            Symbol bestSoFar) {
        if (sym.kind == ERR)
            return bestSoFar;
        assert sym.kind < AMBIGUOUS;
        if (instantiate(env, site, sym, argtypes) == null) {
            switch (bestSoFar.kind) {
            case ABSENT_MTH:
                return wrongMethod.setWrongSym(sym);

            case WRONG_MTH:
                return wrongMethods;

            default:
                return bestSoFar;

            }
        }
        if (!isAccessible(env, site, sym)) {
            return (bestSoFar.kind == ABSENT_MTH) ? new AccessError(sym) : bestSoFar;
        }
        return (bestSoFar.kind > AMBIGUOUS) ? sym :
                mostSpecific(sym, bestSoFar, env, site);
    }

    Symbol mostSpecific(Symbol m1, Symbol m2, Env env, Type site) {
        switch (m2.kind) {
        case MTH:
            if (m1 == m2)
                return m1;
            Type mt1 = site.memberType(m1);
            boolean m1SignatureMoreSpecific =
                    instantiate(env, site, m2, mt1.argtypes()) != null;
            Type mt2 = site.memberType(m2);
            boolean m2SignatureMoreSpecific =
                    instantiate(env, site, m1, mt2.argtypes()) != null;
            if (m1SignatureMoreSpecific && m2SignatureMoreSpecific) {
                if (methodsInherited(m1.owner, m2.owner))
                    return m1;
                if (methodsInherited(m2.owner, m1.owner))
                    return m2;
                boolean m1Abstract = (m1.flags() & ABSTRACT) != 0;
                if (!m1Abstract)
                    return m1;
                boolean m2Abstract = (m2.flags() & ABSTRACT) != 0;
                if (!m2Abstract)
                    return m2;
                Symbol result = mt1.restype().isSubType(mt2.restype()) ? m1 : m2;
                result = result.clone(result.owner);
                result.type = (Type) result.type.clone();
                result.type.setThrown(Check.intersect(mt1.thrown(), mt2.thrown()));
                return result;
            }
            if (m1SignatureMoreSpecific)
                return m1;
            if (m2SignatureMoreSpecific)
                return m2;
            return new AmbiguityError(m1, m2);

        case AMBIGUOUS:
            AmbiguityError e = (AmbiguityError) m2;
            Symbol err1 = mostSpecific(m1, e.sym1, env, site);
            Symbol err2 = mostSpecific(m1, e.sym2, env, site);
            if (err1 == err2)
                return err1;
            if (err1 == e.sym1 && err2 == e.sym2)
                return m2;
            return new AmbiguityError(err1, err2);

        default:
            throw new AssertionError();

        }
    }

    private boolean methodsInherited(Symbol subclass, Symbol superclass) {
        return subclass.isSubClass(superclass) &&
                !((subclass.flags() & INTERFACE) != 0 &&
                superclass.type == syms.objectType);
    }

    /**
      * Find best qualified method matching given name, type and value parameters.
      *  @param env       The current environment.
      *  @param site      The original type from where the selection takes place.
      *  @param name      The method's name.
      *  @param argtypes  The method's value parameters.
      */
    Symbol findMethod(Env env, Type site, Name name, List argtypes) {
        return findMethod(env, site, name, argtypes, site.tsym.type, true,
                methodNotFound);
    }

    private Symbol findMethod(Env env, Type site, Name name, List argtypes,
            Type intype, boolean abstractok, Symbol bestSoFar) {
        for (Type ct = intype; ct.tag == CLASS; ct = ct.supertype()) {
            ClassSymbol c = (ClassSymbol) ct.tsym;
            if ((c.flags() & (ABSTRACT | INTERFACE)) == 0)
                abstractok = false;
            for (Scope.Entry e = c.members().lookup(name); e.scope != null;
                    e = e.next()) {
                if (e.sym.kind == MTH && (e.sym.flags_field & SYNTHETIC) == 0) {
                    bestSoFar = selectBest(env, site, argtypes, e.sym, bestSoFar);
                }
            }
            if (abstractok) {
                for (List l = c.type.interfaces(); l.nonEmpty(); l = l.tail) {
                    bestSoFar = findMethod(env, site, name, argtypes, (Type) l.head,
                            abstractok, bestSoFar);
                }
            }
        }
        return bestSoFar;
    }

    /**
      * Find unqualified method matching given name, type and value parameters.
      *  @param env       The current environment.
      *  @param name      The method's name.
      *  @param argtypes  The method's value parameters.
      */
    Symbol findFun(Env env, Name name, List argtypes) {
        Symbol bestSoFar = methodNotFound;
        Symbol sym;
        Env env1 = env;
        boolean staticOnly = false;
        while (env1.outer != null) {
            if (isStatic(env1))
                staticOnly = true;
            sym = findMethod(env1, env1.enclClass.sym.type, name, argtypes);
            if (sym.kind <= AMBIGUOUS || sym.kind == WRONG_MTHS ||
                    sym.kind == WRONG_MTH) {
                if (staticOnly && sym.kind == MTH && sym.owner.kind == TYP &&
                        (sym.flags() & STATIC) == 0)
                    return new StaticError(sym);
                else
                    return sym;
            } else if (sym.kind < bestSoFar.kind) {
                bestSoFar = sym;
            }
            if ((env1.enclClass.sym.flags() & STATIC) != 0)
                staticOnly = true;
            env1 = env1.outer;
        }
        sym = findMethod(env, syms.predefClass.type, name, argtypes);
        if (sym.kind <= AMBIGUOUS)
            return sym;
        else
            return bestSoFar;
    }

    /**
      * Load toplevel or member class with given fully qualified name and
      *  verify that it is accessible.
      *  @param env       The current environment.
      *  @param name      The fully qualified name of the class to be loaded.
      */
    Symbol loadClass(Env env, Name name) {
        try {
            ClassSymbol c = reader.loadClass(name);
            if (isAccessible(env, c))
                return c;
            else
                return new AccessError(c);
        } catch (ClassReader.BadClassFile err) {
            throw err;
        }
        catch (CompletionFailure ex) {
            return typeNotFound;
        }
    }

    /**
      * Find qualified member type.
      *  @param env       The current environment.
      *  @param site      The original type from where the selection takes place.
      *  @param name      The type's name.
      *  @param c         The class to search for the member type. This is always
      *                   a superclass or implemented interface of site's class.
      */
    Symbol findMemberType(Env env, Type site, Name name, TypeSymbol c) {
        Symbol bestSoFar = typeNotFound;
        Symbol sym;
        Scope.Entry e = c.members().lookup(name);
        while (e.scope != null) {
            if (e.sym.kind == TYP) {
                return isAccessible(env, site, e.sym) ? e.sym :
                        new AccessError(e.sym);
            }
            e = e.next();
        }
        Type st = c.type.supertype();
        if (st != null && st.tag == CLASS) {
            sym = findMemberType(env, site, name, st.tsym);
            if (sym.kind < bestSoFar.kind)
                bestSoFar = sym;
        }
        for (List l = c.type.interfaces();
                bestSoFar.kind != AMBIGUOUS && l.nonEmpty(); l = l.tail) {
            sym = findMemberType(env, site, name, ((Type) l.head).tsym);
            if (bestSoFar.kind < AMBIGUOUS && sym.kind < AMBIGUOUS &&
                    sym.owner != bestSoFar.owner)
                bestSoFar = new AmbiguityError(bestSoFar, sym);
            else if (sym.kind < bestSoFar.kind)
                bestSoFar = sym;
        }
        return bestSoFar;
    }

    /**
      * Find a global type in given scope and load corresponding class.
      *  @param env       The current environment.
      *  @param scope     The scope in which to look for the type.
      *  @param name      The type's name.
      */
    Symbol findGlobalType(Env env, Scope scope, Name name) {
        Symbol bestSoFar = typeNotFound;
        Scope.Entry e = scope.lookup(name);
        while (e.scope != null) {
            if (e.scope.owner == e.sym.owner) {
                Symbol sym = loadClass(env, e.sym.flatName());
                if (bestSoFar.kind == TYP && sym.kind == TYP && bestSoFar != sym)
                    return new AmbiguityError(bestSoFar, sym);
                else if (sym.kind < bestSoFar.kind)
                    bestSoFar = sym;
            }
            e = e.next();
        }
        return bestSoFar;
    }

    /**
      * Find an unqualified type symbol.
      *  @param env       The current environment.
      *  @param name      The type's name.
      */
    Symbol findType(Env env, Name name) {
        Symbol bestSoFar = typeNotFound;
        Symbol sym;
        Env env1 = env;
        boolean staticOnly = false;
        while (env1.outer != null) {
            if (isStatic(env1))
                staticOnly = true;
            Scope.Entry e = ((AttrContext) env1.info).scope.lookup(name);
            while (e.scope != null) {
                if (e.sym.kind == TYP) {
                    return e.sym;
                }
                e = e.next();
            }
            sym = findMemberType(env1, env1.enclClass.sym.type, name,
                    env1.enclClass.sym);
            if (staticOnly && sym.kind == TYP && sym.type.tag == CLASS &&
                    sym.type.outer().isParameterized())
                return new StaticError(sym);
            else if (sym.kind <= AMBIGUOUS)
                return sym;
            else if (sym.kind < bestSoFar.kind)
                bestSoFar = sym;
            long flags = env1.enclClass.sym.flags();
            if ((flags & STATIC) != 0)
                staticOnly = true;
            env1 = env1.outer;
        }
        if (env.tree.tag != Tree.IMPORT) {
            sym = findGlobalType(env, env.toplevel.namedImportScope, name);
            if (sym.kind <= AMBIGUOUS)
                return sym;
            else if (sym.kind < bestSoFar.kind)
                bestSoFar = sym;
            sym = findGlobalType(env, env.toplevel.packge.members(), name);
            if (sym.kind <= AMBIGUOUS)
                return sym;
            else if (sym.kind < bestSoFar.kind)
                bestSoFar = sym;
            sym = findGlobalType(env, env.toplevel.starImportScope, name);
            if (sym.kind <= AMBIGUOUS)
                return sym;
            else if (sym.kind < bestSoFar.kind)
                bestSoFar = sym;
        }
        return bestSoFar;
    }

    /**
      * Find an unqualified identifier which matches a specified kind set.
      *  @param env       The current environment.
      *  @param name      The indentifier's name.
      *  @param kind      Indicates the possible symbol kinds
      *                   (a subset of VAL, TYP, PCK).
      */
    Symbol findIdent(Env env, Name name, int kind) {
        Symbol bestSoFar = typeNotFound;
        Symbol sym;
        if ((kind & VAR) != 0) {
            sym = findVar(env, name);
            if (sym.kind <= AMBIGUOUS)
                return sym;
            else if (sym.kind < bestSoFar.kind)
                bestSoFar = sym;
        }
        if ((kind & TYP) != 0) {
            sym = findType(env, name);
            if (sym.kind <= AMBIGUOUS)
                return sym;
            else if (sym.kind < bestSoFar.kind)
                bestSoFar = sym;
        }
        if ((kind & PCK) != 0)
            return reader.enterPackage(name);
        else
            return bestSoFar;
    }

    /**
      * Find an identifier in a package which matches a specified kind set.
      *  @param env       The current environment.
      *  @param name      The identifier's name.
      *  @param kind      Indicates the possible symbol kinds
      *                   (a nonempty subset of TYP, PCK).
      */
    Symbol findIdentInPackage(Env env, TypeSymbol pck, Name name, int kind) {
        Name fullname = TypeSymbol.formFullName(name, pck);
        Symbol bestSoFar = typeNotFound;
        PackageSymbol pack = null;
        if ((kind & PCK) != 0) {
            pack = reader.enterPackage(fullname);
            if (pack.exists())
                return pack;
        }
        if ((kind & TYP) != 0) {
            Symbol sym = loadClass(env, fullname);
            if (sym.kind <= AMBIGUOUS) {
                if (name == sym.name)
                    return sym;
            } else if (sym.kind < bestSoFar.kind)
                bestSoFar = sym;
        }
        return (pack != null) ? pack : bestSoFar;
    }

    /**
      * Find an identifier among the members of a given type `site'.
      *  @param env       The current environment.
      *  @param site      The type containing the symbol to be found.
      *  @param name      The identifier's name.
      *  @param kind      Indicates the possible symbol kinds
      *                   (a subset of VAL, TYP).
      */
    Symbol findIdentInType(Env env, Type site, Name name, int kind) {
        Symbol bestSoFar = typeNotFound;
        Symbol sym;
        if ((kind & VAR) != 0) {
            sym = findField(env, site, name, site.tsym);
            if (sym.kind <= AMBIGUOUS)
                return sym;
            else if (sym.kind < bestSoFar.kind)
                bestSoFar = sym;
        }
        if ((kind & TYP) != 0) {
            sym = findMemberType(env, site, name, site.tsym);
            if (sym.kind <= AMBIGUOUS)
                return sym;
            else if (sym.kind < bestSoFar.kind)
                bestSoFar = sym;
        }
        return bestSoFar;
    }

    /**
      * If `sym' is a bad symbol: report error and return errSymbol
      *  else pass through unchanged,
      *  additional arguments duplicate what has been used in trying to find the
      *  symbol (--> flyweight pattern). This improves performance since we expect
      *  misses to happen frequently.
      *
      *  @param sym       The symbol that was found, or a ResolveError.
      *  @param pos       The position to use for error reporting.
      *  @param site      The original type from where the selection took place.
      *  @param name      The symbol's name.
      *  @param argtypes  The invocation's value parameters,
      *                   if we looked for a method.
      */
    Symbol access(Symbol sym, int pos, Type site, Name name, boolean qualified,
            List argtypes) {
        if (sym.kind >= AMBIGUOUS) {
            if (!site.isErroneous() && !Type.isErroneous(argtypes))
                ((ResolveError) sym).report(log, pos, site, name, argtypes);
            do {
                sym = ((ResolveError) sym).sym;
            } while (sym.kind >= AMBIGUOUS)
                ;
            if (sym == syms.errSymbol)
                sym = new ErrorType(name, qualified ? site.tsym : syms.noSymbol).tsym;
        }
        return sym;
    }

    /**
      * Same as above, but without type parameters and arguments.
      */
    Symbol access(Symbol sym, int pos, Type site, Name name, boolean qualified) {
        if (sym.kind >= AMBIGUOUS)
            return access(sym, pos, site, name, qualified, Type.emptyList);
        else
            return sym;
    }

    /**
      * Check that sym is not an abstract method.
      */
    void checkNonAbstract(int pos, Symbol sym) {
        if ((sym.flags() & ABSTRACT) != 0)
            log.error(pos, "abstract.cant.be.accessed.directly",
                    kindName(sym.kind), sym.toJava());
    }

    /**
      * print all scopes starting with scope s and proceeding outwards.
      *  used for debugging.
      */
    public static void printscopes(Scope s) {
        while (s != null) {
            if (s.owner != null)
                System.err.print(s.owner + ": ");
            for (Scope.Entry e = s.elems; e != null; e = e.sibling) {
                if ((e.sym.flags() & ABSTRACT) != 0)
                    System.err.print("abstract ");
                System.err.print(e.sym + " ");
            }
            System.err.println();
            s = s.next;
        }
    }

    static void printscopes(Env env) {
        while (env.outer != null) {
            System.err.println("------------------------------");
            printscopes(((AttrContext) env.info).scope);
            env = env.outer;
        }
    }

    public static void printscopes(Type t) {
        while (t.tag == CLASS) {
            printscopes(t.tsym.members());
            t = t.supertype();
        }
    }

    /**
      * Warn about an unchecked method invocation.
      */
    void warnUncheckedInvocation(int pos, Symbol sym, List argtypes) {
        if (sym.kind < AMBIGUOUS) {
            chk.warnUnchecked(pos, "unchecked.meth.invocation.applied",
                    sym.toJava(), sym.javaLocation(), Type.toJavaList(argtypes));
        }
    }

    /**
      * Resolve an unqualified (non-method) identifier.
      *  @param pos       The position to use for error reporting.
      *  @param env       The environment current at the identifier use.
      *  @param name      The identifier's name.
      *  @param kind      The set of admissible symbol kinds for the identifier.
      */
    Symbol resolveIdent(int pos, Env env, Name name, int kind) {
        return access(findIdent(env, name, kind), pos, env.enclClass.sym.type,
                name, false);
    }

    /**
      * Resolve an unqualified method identifier.
      *  @param pos       The position to use for error reporting.
      *  @param env       The environment current at the method invocation.
      *  @param name      The identifier's name.
      *  @param argtypes  The types of the invocation's value parameters.
      */
    Symbol resolveMethod(int pos, Env env, Name name, List argtypes) {
        Symbol sym = findFun(env, name, argtypes);
        if (sym.kind >= WRONG_MTHS && Type.isDerivedRaw(argtypes) &&
                !((AttrContext) env.info).rawArgs) {
            ((AttrContext) env.info).rawArgs = true;
            sym = findFun(env, name, argtypes);
            ((AttrContext) env.info).rawArgs = false;
            warnUncheckedInvocation(pos, sym, argtypes);
        }
        if (sym.kind >= AMBIGUOUS) {
            sym = access(sym, pos, env.enclClass.sym.type, name, false, argtypes);
        }
        return sym;
    }

    /**
      * Resolve a qualified method identifier
      *  @param pos       The position to use for error reporting.
      *  @param env       The environment current at the method invocation.
      *  @param site      The type of the qualifying expression, in which
      *                   identifier is searched.
      *  @param name      The identifier's name.
      *  @param argtypes  The types of the invocation's value parameters.
      */
    Symbol resolveQualifiedMethod(int pos, Env env, Type site, Name name,
            List argtypes) {
        Symbol sym = findMethod(env, site, name, argtypes);
        if (sym.kind >= WRONG_MTHS && Type.isDerivedRaw(argtypes) &&
                !((AttrContext) env.info).rawArgs) {
            ((AttrContext) env.info).rawArgs = true;
            sym = findMethod(env, site, name, argtypes);
            ((AttrContext) env.info).rawArgs = false;
            warnUncheckedInvocation(pos, sym, argtypes);
        }
        if (sym.kind >= AMBIGUOUS) {
            sym = access(sym, pos, site, name, true, argtypes);
        }
        return sym;
    }

    /**
      * Resolve a qualified method identifier, throw a fatal error if not
      *  found.
      *  @param pos       The position to use for error reporting.
      *  @param env       The environment current at the method invocation.
      *  @param site      The type of the qualifying expression, in which
      *                   identifier is searched.
      *  @param name      The identifier's name.
      *  @param argtypes  The types of the invocation's value parameters.
      */
    Symbol resolveInternalMethod(int pos, Env env, Type site, Name name,
            List argtypes) {
        Symbol sym = resolveQualifiedMethod(pos, env, site, name, argtypes);
        if (sym.kind == MTH)
            return sym;
        else
            throw new FatalError( Log.getLocalizedString("fatal.err.cant.locate.meth",
                    name.toJava()));
    }

    /**
      * Resolve constructor.
      *  @param pos       The position to use for error reporting.
      *  @param env       The environment current at the constructor invocation.
      *  @param site      The type of class for which a constructor is searched.
      *  @param argtypes  The types of the constructor invocation's value
      *			 parameters.
      */
    Symbol resolveConstructor(int pos, Env env, Type site, List argtypes) {
        Symbol sym = resolveQualifiedMethod(pos, env, site, names.init, argtypes);
        if ((sym.flags() & DEPRECATED) != 0 &&
                (((AttrContext) env.info).scope.owner.flags() & DEPRECATED) ==
                0 && ((AttrContext) env.info).scope.owner.outermostClass() !=
                sym.outermostClass())
            chk.warnDeprecated(pos, sym);
        return sym;
    }

    /**
      * Resolve operator.
      *  @param pos       The position to use for error reporting.
      *  @param optag     The tag of the operation tree.
      *  @param env       The environment current at the operation.
      *  @param argtypes  The types of the operands.
      */
    Symbol resolveOperator(int pos, int optag, Env env, List argtypes) {
        Name name = treeinfo.operatorName(optag);
        return access(findMethod(env, syms.predefClass.type, name, argtypes),
                pos, env.enclClass.sym.type, name, false, argtypes);
    }

    /**
      * Resolve operator.
      *  @param pos       The position to use for error reporting.
      *  @param optag     The tag of the operation tree.
      *  @param env       The environment current at the operation.
      *  @param argtypes  The types of the operands.
      */
    Symbol resolveUnaryOperator(int pos, int optag, Env env, Type arg) {
        List argtypes = Type.emptyList.prepend(arg);
        return resolveOperator(pos, optag, env, argtypes);
    }

    /**
      * Resolve binary operator.
      *  @param pos       The position to use for error reporting.
      *  @param optag     The tag of the operation tree.
      *  @param env       The environment current at the operation.
      *  @param left      The types of the left operand.
      *  @param right     The types of the right operand.
      */
    Symbol resolveBinaryOperator(int pos, int optag, Env env, Type left, Type right) {
        List argtypes = Type.emptyList.prepend(right).prepend(left);
        return resolveOperator(pos, optag, env, argtypes);
    }

    /**
      * Resolve `c.name' where name == this or name == super.
      *  @param pos           The position to use for error reporting.
      *  @param env           The environment current at the expression.
      *  @param c             The qualifier.
      *  @param name          The identifier's name.
      */
    Symbol resolveSelf(int pos, Env env, TypeSymbol c, Name name) {
        Env env1 = env;
        boolean staticOnly = false;
        while (env1.outer != null) {
            if (isStatic(env1))
                staticOnly = true;
            if (env1.enclClass.sym == c) {
                Symbol sym = ((AttrContext) env1.info).scope.lookup(name).sym;
                if (sym != null) {
                    if (staticOnly)
                        sym = new StaticError(sym);
                    return access(sym, pos, env.enclClass.sym.type, name, true);
                }
            }
            if ((env1.enclClass.sym.flags() & STATIC) != 0)
                staticOnly = true;
            env1 = env1.outer;
        }
        log.error(pos, "not.encl.class", c.toJava());
        return syms.errSymbol;
    }

    /**
      * Resolve `c.this' for an enclosing class c that contains
      *  the named member.
      *  @param pos           The position to use for error reporting.
      *  @param env           The environment current at the expression.
      *  @param member        The member that must be contained in the result.
      */
    Symbol resolveSelfContaining(int pos, Env env, Symbol member) {
        Name name = names._this;
        Env env1 = env;
        boolean staticOnly = false;
        while (env1.outer != null) {
            if (isStatic(env1))
                staticOnly = true;
            if (env1.enclClass.sym.isSubClass(member.owner)) {
                Symbol sym = ((AttrContext) env1.info).scope.lookup(name).sym;
                if (sym != null) {
                    if (staticOnly)
                        sym = new StaticError(sym);
                    return access(sym, pos, env.enclClass.sym.type, name, true);
                }
            }
            if ((env1.enclClass.sym.flags() & STATIC) != 0)
                staticOnly = true;
            env1 = env1.outer;
        }
        log.error(pos, "encl.class.required", member.toJava());
        return syms.errSymbol;
    }

    /**
      * Resolve an appropriate implicit this instance for t's container.
      *  JLS2 8.8.5.1 and 15.9.2
      */
    Type resolveImplicitThis(int pos, Env env, Type t) {
        Type thisType = (((t.tsym.owner.kind & (MTH | VAR)) != 0) ?
                resolveSelf(pos, env, t.outer().tsym, names._this) :
                resolveSelfContaining(pos, env, t.tsym)).type;
        if (((AttrContext) env.info).isSelfCall && thisType.tsym == env.enclClass.sym)
            log.error(pos, "cant.ref.before.ctor.called", "this");
        return thisType;
    }

    /**
      * A localized string describing a given kind.
      */
    static String kindName(int kind) {
        switch (kind) {
        case PCK:
            return Log.getLocalizedString("kindname.package");

        case TYP:
            return Log.getLocalizedString("kindname.class");

        case VAR:
            return Log.getLocalizedString("kindname.variable");

        case VAL:
            return Log.getLocalizedString("kindname.value");

        case MTH:
            return Log.getLocalizedString("kindname.method");

        default:
            return Log.getLocalizedString("kindname.default", Integer.toString(kind));

        }
    }

    /**
      * A localized string describing a given set of kinds.
      */
    static String kindNames(int kind) {
        String[] s = new String[4];
        int i = 0;
        if ((kind & VAL) != 0)
            s[i++] = ((kind & VAL) == VAR) ?
                    Log.getLocalizedString("kindname.variable") :
                    Log.getLocalizedString("kindname.value");
        if ((kind & MTH) != 0)
            s[i++] = Log.getLocalizedString("kindname.method");
        if ((kind & TYP) != 0)
            s[i++] = Log.getLocalizedString("kindname.class");
        if ((kind & PCK) != 0)
            s[i++] = Log.getLocalizedString("kindname.package");
        String names = "";
        for (int j = 0; j < i - 1; j++)
            names = names + s[j] + ", ";
        if (i >= 1)
            names = names + s[i - 1];
        else
            names = Log.getLocalizedString("kindname.default",
                    Integer.toString(kind));
        return names;
    }

    /**
      * A localized string describing the kind -- either class or interface --
      *  of a given type.
      */
    static String typeKindName(Type t) {
        if (t.tag == CLASS && (t.tsym.flags() & COMPOUND) != 0)
            return Log.getLocalizedString("kindname.type.variable");
        else if (t.tag == PACKAGE)
            return Log.getLocalizedString("kindname.package");
        else if ((t.tsym.flags_field & INTERFACE) != 0)
            return Log.getLocalizedString("kindname.interface");
        else
            return Log.getLocalizedString("kindname.class");
    }

    /**
      * A localized string describing the kind of a missing symbol, given an
      *  error kind.
      */
    static String absentKindName(int kind) {
        switch (kind) {
        case ABSENT_VAR:
            return Log.getLocalizedString("kindname.variable");

        case WRONG_MTHS:

        case WRONG_MTH:

        case ABSENT_MTH:
            return Log.getLocalizedString("kindname.method");

        case ABSENT_TYP:
            return Log.getLocalizedString("kindname.class");

        default:
            return Log.getLocalizedString("kindname.identifier");

        }
    }

    /**
      * Root class for resolve errors.
      *  Instances of this class indicate "Symbol not found".
      *  Instances of subclass indicate other errors.
      */
    private static class ResolveError extends Symbol implements TypeTags {

        ResolveError(int kind, Symbol sym, String debugName) {
            super(kind, 0, null, null, null);
            this.debugName = debugName;
            this.sym = sym;
        }

        /**
          * The name of the kind of error, for debugging only.
          */
        final String debugName;

        /**
         * The symbol that was determined by resolution, or errSymbol if none
         *  was found.
         */
        final Symbol sym;

        /**
         * The symbol that was a close mismatch, or null if none was found.
         *  wrongSym is currently set if a simgle method with the correct name, but
         *  the wrong parameters was found.
         */
        Symbol wrongSym;

        /**
         * Print the (debug only) name of the kind of error.
         */
        public String toString() {
            return debugName + " wrongSym=" + wrongSym;
        }

        /**
          * Update wrongSym and return this
          */
        ResolveError setWrongSym(Symbol sym) {
            this.wrongSym = sym;
            return this;
        }

        /**
          * Report error.
          *  @param log       The error log to be used for error reporting.
          *  @param pos       The position to be used for error reporting.
          *  @param site      The original type from where the selection took place.
          *  @param name      The name of the symbol to be resolved.
          *  @param argtypes  The invocation's value parameters,
          *                   if we looked for a method.
          */
        void report(Log log, int pos, Type site, Name name, List argtypes) {
            if (name != name.table.error) {
                String kindname = absentKindName(kind);
                String idname = name.toJava();
                String args = "";
                if (kind >= WRONG_MTHS && kind <= ABSENT_MTH) {
                    if (isOperator(name)) {
                        log.error(pos, "operator.cant.be.applied", name.toJava(),
                                Type.toJavaList(argtypes));
                        return;
                    }
                    if (name == name.table.init) {
                        kindname = log.getLocalizedString("kindname.constructor");
                        idname = site.tsym.name.toJava();
                    }
                    args = "(" + Type.toJavaList(argtypes) + ")";
                }
                if (kind == WRONG_MTH) {
                    log.error(pos, "cant.apply.symbol",
                            wrongSym.asMemberOf(site).toJava(),
                            wrongSym.javaLocation(site), Type.toJavaList(argtypes));
                } else if (site.tsym.name.len != 0) {
                    if (site.tsym.kind == PCK && !site.tsym.exists())
                        log.error(pos, "doesnt.exist", site.tsym.toJava());
                    else if (idname.indexOf('$') != -1)
                        log.error(pos, "cant.resolve.location.perchance",
                                kindname, idname, args, typeKindName(site),
                                site.toJava(), idname.replace('$', '.'));
                    else
                        log.error(pos, "cant.resolve.location", kindname, idname,
                                args, typeKindName(site), site.toJava());
                } else
                    log.error(pos, "cant.resolve", kindname, idname, args);
            }
        }

        /**
          * A name designates an operator if it consists
          *  of a non-empty sequence of operator symbols +-~!/*%&|^<>=
          */
        boolean isOperator(Name name) {
            int i = 0;
            while (i < name.len && "+-~!*/%&|^<>=".indexOf(name.byteAt(i)) >= 0)
                i++;
            return i > 0 && i == name.len;
        }
    }

    /**
      * Resolve error class indicating that a symbol is not accessible.
      */
    static class AccessError extends ResolveError {

        AccessError(Symbol sym) {
            super(HIDDEN, sym, "access error");
        }

        /**
          * Report error.
          *  @param log       The error log to be used for error reporting.
          *  @param pos       The position to be used for error reporting.
          *  @param site      The original type from where the selection took place.
          *  @param name      The name of the symbol to be resolved.
          *  @param argtypes  The invocation's value parameters,
          *                   if we looked for a method.
          */
        void report(Log log, int pos, Type site, Name name, List argtypes) {
            if (sym.owner.type.tag != ERROR) {
                if (sym.name == sym.name.table.init && sym.owner != site.tsym)
                    new ResolveError(ABSENT_MTH, sym.owner,
                            "absent method " + sym).report(log, pos, site, name,
                            argtypes);
                if ((sym.flags() & PUBLIC) != 0)
                    log.error(pos, "not.def.public.class.intf.cant.access",
                            sym.toJava(), sym.javaLocation());
                else if ((sym.flags() & (PRIVATE | PROTECTED)) != 0)
                    log.error(pos, "report.access", sym.toJava(),
                            TreeInfo.flagNames(sym.flags() &
                            (PRIVATE | PROTECTED)), sym.javaLocation());
                else
                    log.error(pos, "not.def.public.cant.access", sym.toJava(),
                            sym.javaLocation());
            }
        }
    }

    /**
      * Resolve error class indicating that an instance member was accessed
      *  from a static context.
      */
    static class StaticError extends ResolveError {

        StaticError(Symbol sym) {
            super(HIDDEN, sym, "static error");
        }

        /**
          * Report error.
          *  @param log       The error log to be used for error reporting.
          *  @param pos       The position to be used for error reporting.
          *  @param site      The original type from where the selection took place.
          *  @param name      The name of the symbol to be resolved.
          *  @param argtypes  The invocation's value parameters,
          *                   if we looked for a method.
          */
        void report(Log log, int pos, Type site, Name name, List argtypes) {
            String symstr = sym.kind == TYP & sym.type.tag == CLASS ?
                    sym.type.toJava() : sym.toJava();
            log.error(pos, "non-static.cant.be.ref", kindName(sym.kind), symstr);
        }
    }

    /**
      * Resolve error class indicating an ambiguous reference.
      */
    static class AmbiguityError extends ResolveError {
        Symbol sym1;
        Symbol sym2;

        AmbiguityError(Symbol sym1, Symbol sym2) {
            super(AMBIGUOUS, sym1, "ambiguity error");
            this.sym1 = sym1;
            this.sym2 = sym2;
        }

        /**
          * Report error.
          *  @param log       The error log to be used for error reporting.
          *  @param pos       The position to be used for error reporting.
          *  @param site      The original type from where the selection took place.
          *  @param name      The name of the symbol to be resolved.
          *  @param argtypes  The invocation's value parameters,
          *                   if we looked for a method.
          */
        void report(Log log, int pos, Type site, Name name, List argtypes) {
            AmbiguityError pair = this;
            while (true) {
                if (pair.sym1.kind == AMBIGUOUS)
                    pair = (AmbiguityError) pair.sym1;
                else if (pair.sym2.kind == AMBIGUOUS)
                    pair = (AmbiguityError) pair.sym2;
                else
                    break;
            }
            Name sname = pair.sym1.name;
            if (sname == sname.table.init)
                sname = pair.sym1.owner.name;
            log.error(pos, "ref.ambiguous", sname.toJava(),
                    kindName(pair.sym1.kind), pair.sym1.toJava(),
                    pair.sym1.javaLocation(site), kindName(pair.sym2.kind),
                    pair.sym2.toJava(), pair.sym2.javaLocation(site));
        }
    }
}

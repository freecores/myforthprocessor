/**
 * @(#)Symbol.java	1.45 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.tools.javac.v8.code;
import com.sun.tools.javac.v8.util.*;

import com.sun.tools.javac.v8.code.Type.*;


/**
 * Root class for Java symbols. It contains subclasses
 *  for specific sorts of symbols, such as variables, methods and operators,
 *  types, packages. Each subclass is represented as a static inner class
 *  inside Symbol.
 */
public class Symbol implements Flags, Kinds, TypeTags {

    /**
     * The kind of this symbol.
     *  @see Kinds.
     */
    public int kind;

    /**
     * The flags of this symbol.
     */
    public long flags_field;

    /**
     * An accessor method for the flags of this symbol.
     *  Flags of class symbols should be accessed through the accessor
     *  method to make sure that the class symbol is loaded.
     */
    public long flags() {
        return flags_field;
    }

    /**
      * The name of this symbol in Utf8 representation.
      */
    public Name name;

    /**
     * The type of this symbol.
     */
    public Type type;

    /**
     * The owner of this symbol.
     */
    public Symbol owner;

    /**
     * The completer of this symbol.
     */
    public Completer completer;

    /**
     * A cache for the type erasure of this symbol.
     */
    public Type erasure_field;

    /**
     * Construct a symbol with given kind, flags, name, type and owner.
     */
    public Symbol(int kind, long flags, Name name, Type type, Symbol owner) {
        super();
        this.kind = kind;
        this.flags_field = flags;
        this.type = type;
        this.owner = owner;
        this.completer = null;
        this.erasure_field = null;
        this.name = name;
    }

    /**
      * Clone this symbol with new owner.
      *  Legal only for fields and methods.
      */
    public Symbol clone(Symbol newOwner) {
        throw new AssertionError();
    }

    /**
      * A description of this symbol; overrides Object.
      */
    public String toString() {
        return name.toString();
    }

    /**
      * The Java source which this symbol represents.  Use of this method will
      *  result in a loss of the plain-language description for the symbol.
      */
    public String toJava() {
        return name.toString();
    }

    /**
      * A description of the location of this symbol; used for
      *  error reporting.
      *
      *  XXX 06/09/99 iris
      *  This method appears to be redundant and should probably be
      *  unified with javaLocation();
      */
    public String location() {
        if (owner.name == null || owner.name.len == 0)
            return "";
        else
            return " in " + owner;
    }

    public String location(Type site) {
        if (owner.name == null || owner.name.len == 0) {
            return "";
        }
        if (owner.type.tag == CLASS) {
            Type ownertype = site.asOuterSuper(owner);
            if (ownertype != null)
                return " in class " + ownertype;
        }
        return " in " + owner;
    }

    /**
      * A Java source description of the location of this symbol; used for
      *  error reporting.  Use of this method may result in the loss of the
      *  symbol's description.
      */
    public String javaLocation() {
        if (owner.name == null || owner.name.len == 0)
            return "";
        else
            return owner.toJava();
    }

    public String javaLocation(Type site) {
        if (owner.name == null || owner.name.len == 0) {
            return "";
        }
        if (owner.type.tag == CLASS) {
            Type ownertype = site.asOuterSuper(owner);
            if (ownertype != null)
                return ownertype.toJava();
        }
        return owner.toJava();
    }

    /**
      * The symbol's erased type.
      */
    public Type erasure() {
        if (erasure_field == null)
            erasure_field = type.erasure();
        return erasure_field;
    }

    /**
      * The external type of a symbol. This is the symbol's erased type
      *  except for constructors of inner classes which get the enclosing
      *  instance class added as first argument.
      */
    public Type externalType() {
        Type t = erasure();
        if (name == name.table.init && owner.hasOuterInstance()) {
            Type outerThisType = owner.type.outer().erasure();
            return new MethodType(t.argtypes().prepend(outerThisType),
                    t.restype(), t.thrown(), t.tsym);
        } else {
            return t;
        }
    }

    /**
      * Is this symbol declared (directly or indirectly) local
      *  to a method or variable initializer?
      *  Also includes fields of inner classes which are in
      *  turn local to a method or variable initializer.
      */
    public boolean isLocal() {
        return (owner.kind & (VAR | MTH)) != 0 ||
                (owner.kind == TYP && owner.isLocal());
    }

    /**
      * Is this symbol a constructor?
      */
    public boolean isConstructor() {
        return name == name.table.init;
    }

    /**
      * The fully qualified name of this symbol.
      *  This is the same as the symbol's name except for class symbols,
      *  which are handled separately.
      */
    public Name fullName() {
        return name;
    }

    /**
      * The fully qualified name of this symbol after converting to flat
      *  representation. This is the same as the symbol's name except for
      *  class symbols, which are handled separately.
      */
    public Name flatName() {
        return fullName();
    }

    /**
      * If this is a class or package, its members, otherwise null.
      */
    public Scope members() {
        return null;
    }

    /**
      * A class is an inner class if it it has an enclosing instance class.
      */
    public boolean isInner() {
        return type.outer().tag == CLASS;
    }

    /**
      * An inner class has an outer instance if it is not an interface
      *  it has an enclosing instance class which might be referenced from the class.
      *  Nested classes can see instance members of their enclosing class.
      *  Their constructors carry an additional this$n parameter, inserted
      *  implicitly by the compiler.
      *
      *  @see isInner
      */
    public boolean hasOuterInstance() {
        return type.outer().tag == CLASS &&
                (flags() & (INTERFACE | NOOUTERTHIS)) == 0;
    }

    /**
      * The closest enclosing class of this symbol's declaration.
      */
    public ClassSymbol enclClass() {
        Symbol c = this;
        while (c != null && (c.kind & TYP) == 0)
            c = c.owner;
        return (ClassSymbol) c;
    }

    /**
      * The outermost class which indirectly owns this symbol.
      */
    public ClassSymbol outermostClass() {
        Symbol sym = this;
        Symbol prev = null;
        while (sym.kind != PCK) {
            prev = sym;
            sym = sym.owner;
        }
        return (ClassSymbol) prev;
    }

    /**
      * The package which indirectly owns this symbol.
      */
    public PackageSymbol packge() {
        Symbol sym = this;
        while (sym.kind != PCK) {
            sym = sym.owner;
        }
        return (PackageSymbol) sym;
    }

    /**
      * Is this symbol a subclass of `base'? Only defined for ClassSymbols.
      */
    public boolean isSubClass(Symbol base) {
        throw new AssertionError("isSubClass " + this);
    }

    /**
      * Fully check membership: hierarchy, protection, and hiding.
      *  Does not exclude methods not inherited due to overriding.
      */
    public boolean isMemberOf(TypeSymbol clazz) {
        return owner == clazz || clazz.isSubClass(owner) && isInheritedIn(clazz) &&
                (kind == MTH || !hiddenIn((ClassSymbol) clazz));
    }

    /**
      * Is this symbol the same as or enclosed by the given class?
      */
    public boolean isEnclosedBy(ClassSymbol clazz) {
        for (Symbol sym = this; sym.kind != PCK; sym = sym.owner)
            if (sym == clazz)
                return true;
        return false;
    }

    /**
      * Check for hiding.  Note that this doesn't handle multiple
      *  (interface) inheritance.
      */
    private boolean hiddenIn(ClassSymbol clazz) {
        while (true) {
            if (owner == clazz)
                return false;
            Scope.Entry e = clazz.members().lookup(name);
            while (e.scope != null) {
                if (e.sym.kind == kind)
                    return e.sym != this;
                e = e.next();
            }
            Type superType = clazz.type.supertype();
            if (superType.tag != TypeTags.CLASS)
                return false;
            clazz = (ClassSymbol) superType.tsym;
        }
    }

    /**
      * Is this symbol inherited into a given class?
      *  PRE: If symbol's owner is a interface,
      *       it is already assumed that the interface is a superinterface
      *       of given class.
      *  @param clazz  The class for which we want to establish membership.
      *                This must be a subclass of the member's owner.
      */
    public boolean isInheritedIn(Symbol clazz) {
        switch ((int)(flags_field & Flags.AccessFlags)) {
        case PUBLIC:
            return true;

        case PRIVATE:
            return this.owner == clazz;

        case PROTECTED:
            return (clazz.flags() & INTERFACE) == 0;

        case 0:
            PackageSymbol thisPackage = this.packge();
            for (Symbol sup = clazz; sup != null && sup != this.owner;
                    sup = sup.type.supertype().tsym)
                if (sup.packge() != thisPackage)
                    return false;
            return (clazz.flags() & INTERFACE) == 0;

        default:
            throw new AssertionError();

        }
    }

    /**
      * The (variable or method) symbol seen as a member of given
      *  class type`site' (this might change the symbol's type).
      */
    public Symbol asMemberOf(Type site) {
        throw new AssertionError();
    }

    /**
      * Complete the elaboration of this symbol's definition.
      */
    public void complete() throws CompletionFailure {
        if (completer != null) {
            Completer c = completer;
            completer = null;
            c.complete(this);
        }
    }

    /**
      * A class for type symbols. Type variables are represented by instances
      *  of this class, classes and packages by instances of subclasses.
      */
    public static class TypeSymbol extends Symbol {

        public TypeSymbol(long flags, Name name, Type type, Symbol owner) {
            super(TYP, flags, name, type, owner);
        }

        public String toString() {
            return "type variable " + name;
        }

        /**
          * form a fully qualified name from a name and an owner
          */
        public static Name formFullName(Name name, Symbol owner) {
            if (owner == null)
                return name;
            if (((owner.kind != ERR)) && ((owner.kind & (VAR | MTH)) != 0))
                return name;
            Name prefix = owner.fullName();
            if (prefix == null || prefix == prefix.table.empty ||
                    prefix == prefix.table.emptyPackage)
                return name;
            else
                return prefix.append('.', name);
        }

        /**
          * form a fully qualified name from a name and an owner, after
          *  converting to flat representation
          */
        public static Name formFlatName(Name name, Symbol owner) {
            if (owner == null || (owner.kind & (VAR | MTH)) != 0)
                return name;
            char sep = owner.kind == TYP ? '$' : '.';
            Name prefix = owner.flatName();
            if (prefix == null || prefix == prefix.table.empty ||
                    prefix == prefix.table.emptyPackage)
                return name;
            else
                return prefix.append(sep, name);
        }

        /**
          * The rank of a class is the length of the longest path
          *  between the class and java.lang.Object in the class inheritance
          *  graph. Undefined for all other type symbols.
          */
        public int rank() {
            throw new AssertionError();
        }

        /**
          * A total ordering between type symbols that refines the class
          *  inheritance graph. Typevariables always precede other type symbols.
          */
        public boolean precedes(TypeSymbol that) {
            return this != that;
        }

        /**
          * If symbol is a package, does it exist?
          *  Otherwise always true.
          */
        public boolean exists() {
            return true;
        }
    }

    /**
      * A class for package symbols
      */
    public static class PackageSymbol extends TypeSymbol {
        public Scope members_field;
        public Name fullname;

        public PackageSymbol(Name name, Type type, Symbol owner) {
            super(0, name, type, owner);
            this.kind = PCK;
            this.members_field = null;
            this.fullname = formFullName(name, owner);
        }

        public PackageSymbol(Name name, Symbol owner) {
            this(name, null, owner);
            this.type = new PackageType(this);
        }

        public String toString() {
            return "package " + fullname;
        }

        /**
          * The Java source which this symbol represents.  Use of this method
          *  will result in the loss of the plain-language description for
          *  the symbol.
          */
        public String toJava() {
            return fullname.toString();
        }

        public Name fullName() {
            return fullname;
        }

        public Scope members() {
            if (completer != null)
                complete();
            return members_field;
        }

        public long flags() {
            if (completer != null)
                complete();
            return flags_field;
        }

        /**
          * A package "exists" if a type or package that exists has
          *  been seen within it.
          */
        public boolean exists() {
            return (flags_field & EXISTS) != 0;
        }
    }

    /**
      * A class for class symbols
      */
    public static class ClassSymbol extends TypeSymbol {

        /**
         * a scope for all class members; variables, methods and inner classes
         *  type parameters are not part of this scope
         */
        public Scope members_field;

        /**
         * the fully qualified name of the class, i.e. pck.outer.inner.
         *  null for anonymous classes
         */
        public Name fullname;

        /**
         * the fully qualified name of the class after converting to flat
         *  representation, i.e. pck.outer$inner,
         *  set externally for local and anonymous classes
         */
        public Name flatname;

        /**
         * the sourcefile where the class came from
         */
        public Name sourcefile;

        /**
         * the classfile from where to load this class
         *  this will have extension .class or .java
         */
        public FileEntry classfile;

        /**
         * the constant pool of the class
         */
        public Pool pool;

        /**
         * a cache for the rank of the class. @see rank()
         */
        private int rank_field = -1;

        public ClassSymbol(long flags, Name name, Type type, Symbol owner) {
            super(flags, name, type, owner);
            this.members_field = null;
            this.fullname = formFullName(name, owner);
            this.flatname = formFlatName(name, owner);
            this.sourcefile = null;
            this.classfile = null;
            this.pool = null;
        }

        public ClassSymbol(long flags, Name name, Symbol owner) {
            this(flags, name, new ClassType(Type.noType, Type.emptyList, null),
                    owner);
            this.type.tsym = this;
        }
        public static final List emptyList = new List();

        public String toString() {
            if ((flags_field & COMPOUND) != 0)
                return "type variable " + name;
            else if ((flags_field & INTERFACE) != 0)
                return "interface " + className();
            else
                return "class " + className();
        }

        /**
          * The Java source which this symbol represents.  Use of this method
          *  will result in the loss of the plain-language description for
          *  the symbol.
          */
        public String toJava() {
            return className();
        }

        public long flags() {
            if (completer != null)
                complete();
            return flags_field;
        }

        public Scope members() {
            if (completer != null)
                complete();
            return members_field;
        }

        public Type erasure() {
            if (erasure_field == null)
                if (type.isParameterized())
                    erasure_field = new ClassType(type.outer().erasure(),
                            Type.emptyList, this);
                else
                    erasure_field = type;
            return erasure_field;
        }

        public String className() {
            if (name.len == 0)
                return Log.getLocalizedString("anonymous.class", flatname.toString());
            else
                return fullname.toString();
        }

        public Name fullName() {
            return fullname;
        }

        public Name flatName() {
            return flatname;
        }

        public boolean isSubClass(Symbol base) {
            if (this == base) {
                return true;
            } else if ((base.flags() & INTERFACE) != 0) {
                for (Type t = type; t.tag == CLASS; t = t.supertype())
                    for (List is = t.interfaces(); is.nonEmpty(); is = is.tail)
                        if (((Type) is.head).tsym.isSubClass(base))
                            return true;
            } else {
                for (Type t = type; t.tag == CLASS; t = t.supertype())
                    if (t.tsym == base)
                        return true;
            }
            return false;
        }

        /**
          * The rank of a class is the length of the longest path to Object
          *  in the class inheritance graph.
          */
        public int rank() {
            if (rank_field < 0) {
                Name fiulname = fullName();
                if (fullname == fullname.table.java_lang_Object || type.tag == ERROR)
                    rank_field = 0;
                else {
                    int r = type.supertype().tsym.rank();
                    for (List l = type.interfaces(); l.nonEmpty(); l = l.tail) {
                        if (((Type) l.head).tsym.rank() > r)
                            r = ((Type) l.head).tsym.rank();
                    }
                    rank_field = r + 1;
                }
            }
            return rank_field;
        }

        /**
          * A total ordering between class symbols that refines the class
          *  inheritance graph.
          */
        public boolean precedes(TypeSymbol that) {
            return that.rank() < this.rank() || that.rank() == this.rank() &&
                    isLess(that.fullName(), this.fullname);
        }

        private boolean isLess(Name n1, Name n2) {
            if (n1 == n2)
                return false;
            int i = n1.index + n1.len - 1;
            int j = n2.index + n2.len - 1;
            while (i != n1.index && j != n2.index &&
                    n1.table.names[i] == n2.table.names[j]) {
                i--;
                j--;
            }
            if (j == n2.index)
                return false;
            else if (i == n1.index)
                return true;
            else
                return n1.table.names[i] < n2.table.names[j];
        }

        /**
          * Complete the elaboration of this symbol's definition.
          */
        public void complete() throws CompletionFailure {
            try {
                super.complete();
            } catch (CompletionFailure ex) {
                flags_field |= (PUBLIC | STATIC);
                this.type = new ErrorType(this);
                throw ex;
            }
        }
    }

    /**
      * A class for variable symbols
      */
    public static class VarSymbol extends Symbol {

        /**
         * The variable's declaration position.
         */
        public int pos = Position.NOPOS;

        /**
         * The variable's address. Used for different purposes during
         *  flow analysis, translation and code generation.
         *  Flow analysis:
         *    If this is a blank final or local variable, its sequence number.
         *  Translation:
         *    If this is a private field, its access number.
         *  Code generation:
         *    If this is a local variable, its logical slot number.
         */
        public int adr = -1;

        /**
         * The variable's constant value, if this is a constant.
         *  Before the constant value is evaluated, it points to
         *  an initalizer environment.
         */
        public Object constValue;

        /**
         * Construct a variable symbol, given its flags, name, type and owner.
         */
        public VarSymbol(long flags, Name name, Type type, Symbol owner) {
            super(VAR, flags, name, type, owner);
        }

        /**
          * Clone this symbol with new owner.
          */
        public Symbol clone(Symbol newOwner) {
            VarSymbol v = new VarSymbol(flags_field, name, type, newOwner);
            v.pos = pos;
            v.adr = adr;
            v.constValue = constValue;
            return v;
        }
        public static final List emptyList = new List();

        public String toString() {
            return "variable " + name;
        }

        public Symbol asMemberOf(Type site) {
            return new VarSymbol(flags_field, name, site.memberType(this), owner);
        }
    }

    /**
      * A class for method symbols.
      */
    public static class MethodSymbol extends Symbol {
        public Code code = null;

        /**
         * Construct a method symbol, given its flags, name, type and owner.
         */
        public MethodSymbol(long flags, Name name, Type type, Symbol owner) {
            super(MTH, flags, name, type, owner);
        }
        public static final List emptyList = new List();

        /**
         * Clone this symbol with new owner.
         */
        public Symbol clone(Symbol newOwner) {
            MethodSymbol m = new MethodSymbol(flags_field, name, type, newOwner);
            m.code = code;
            return m;
        }

        public String toString() {
            if ((flags() & BLOCK) != 0) {
                return "body of " + owner;
            } else {
                String s;
                if (name == name.table.init)
                    s = "constructor " + owner.name;
                else
                    s = "method " + name;
                if (type != null) {
                    s += "(" + type.argtypes().toString() + ")";
                }
                return s;
            }
        }

        /**
          * The Java source which this symbol represents.  Use of this method
          *  will result in the loss of the plain-language description for
          *  the symbol.
          */
        public String toJava() {
            String s;
            if ((flags() & BLOCK) != 0) {
                s = owner.name.toString();
            } else {
                if (name == name.table.init)
                    s = owner.name.toString();
                else
                    s = name.toString();
                if (type != null) {
                    s += "(" + type.argtypes().toString() + ")";
                }
            }
            return s;
        }

        /**
          * find a symbol that this (proxy method) symbol implements.
          *  @param    c       The class whose members are searched for
          *                    implementations
          */
        public Symbol implemented(TypeSymbol c) {
            Symbol impl = null;
            for (List is = c.type.interfaces(); impl == null && is.nonEmpty();
                    is = is.tail) {
                TypeSymbol i = ((Type) is.head).tsym;
                for (Scope.Entry e = i.members().lookup(name);
                        impl == null && e.scope != null; e = e.next()) {
                    if (this.overrides(e.sym, (TypeSymbol) owner) &&
                            type.restype().isSameType(
                            owner.type.memberType(e.sym).restype())) {
                        impl = e.sym;
                    }
                    if (impl == null)
                        impl = implemented(i);
                }
            }
            return impl;
        }

        /**
          * Will the erasure of this method be considered by the VM to
          *  override the erasure of the other when seen from class `origin'?
          */
        public boolean binaryOverrides(Symbol _other, TypeSymbol origin) {
            if (isConstructor() || _other.kind != MTH)
                return false;
            if (this == _other)
                return true;
            MethodSymbol other = (MethodSymbol)_other;
            if (other.isOverridableIn((TypeSymbol) owner) &&
                    owner.type.asSuper(other.owner) != null &&
                    erasure().isSameType(other.type.erasure()))
                return true;
            return (flags() & ABSTRACT) == 0 && other.isOverridableIn(origin) &&
                    this.isMemberOf(origin) &&
                    erasure().isSameType(other.type.erasure());
        }

        /**
          * The implementation of this (abstract) symbol in class origin,
          *  from the VM's point of view, null if method does not have an
          *  implementation in class.
          *  @param origin   The class of which the implementation is a member.
          */
        public MethodSymbol binaryImplementation(ClassSymbol origin) {
            for (TypeSymbol c = origin; c != null; c = c.type.supertype().tsym) {
                for (Scope.Entry e = c.members().lookup(name); e.scope != null;
                        e = e.next()) {
                    if (e.sym.kind == MTH &&
                            ((MethodSymbol) e.sym).binaryOverrides(this, origin))
                        return (MethodSymbol) e.sym;
                }
            }
            return null;
        }

        /**
          * Does this symbol override `other' symbol, when both are seen as
          *  members of class `origin'?
          *  It is assumed that both symbols have the same name.
          *  See JLS 8.4.6.1 (without transitivity) and 8.4.6.4
          */
        public boolean overrides(Symbol _other, TypeSymbol origin) {
            if (isConstructor() || _other.kind != MTH)
                return false;
            MethodSymbol other = (MethodSymbol)_other;
            if (other.isOverridableIn((TypeSymbol) owner) &&
                    owner.type.asSuper(other.owner) != null &&
                    owner.type.memberType(this).hasSameArgs(
                    owner.type.memberType(other)))
                return true;
            return (flags() & ABSTRACT) == 0 && other.isOverridableIn(origin) &&
                    this.isMemberOf(origin) &&
                    origin.type.memberType(this).hasSameArgs(
                    origin.type.memberType(other));
        }

        private boolean isOverridableIn(TypeSymbol origin) {
            switch ((int)(flags_field & Flags.AccessFlags)) {
            case Flags.PRIVATE:
                return false;

            case Flags.PUBLIC:
                return true;

            case Flags.PROTECTED:
                return (origin.flags() & INTERFACE) == 0;

            case 0:
                return this.packge() == origin.packge() &&
                        (origin.flags() & INTERFACE) == 0;

            default:
                throw new AssertionError();

            }
        }

        /**
          * The implementation of this (abstract) symbol in class origin;
          *  null if none exists. Synthetic methods are not considered
          *  as possible implementations.
          */
        public MethodSymbol implementation(TypeSymbol origin) {
            for (Type t = origin.type; t.tag == CLASS; t = t.supertype()) {
                TypeSymbol c = t.tsym;
                for (Scope.Entry e = c.members().lookup(name); e.scope != null;
                        e = e.next()) {
                    if (e.sym.kind == MTH) {
                        MethodSymbol m = (MethodSymbol) e.sym;
                        if (m.overrides(this, origin) && (m.flags() & SYNTHETIC) == 0)
                            return m;
                    }
                }
            }
            if (Type.isDerivedRaw(origin.type))
                return implementation(origin.type.supertype().tsym);
            else
                return null;
        }

        public Symbol asMemberOf(Type site) {
            return new MethodSymbol(flags_field, name, site.memberType(this), owner);
        }
    }

    /**
      * A class for predefined operators.
      */
    public static class OperatorSymbol extends MethodSymbol {
        public int opcode;

        public OperatorSymbol(Name name, Type type, int opcode, Symbol owner) {
            super(PUBLIC | STATIC, name, type, owner);
            this.opcode = opcode;
        }
    }

    /**
      * Symbol completer interface.
      */
    public static interface Completer {

        void complete(Symbol sym) throws CompletionFailure;
            }

    public static class CompletionFailure extends RuntimeException {
        public Symbol sym;

        /**
         * A localized string describing the failure.
         */
        public String errmsg;

        public CompletionFailure(Symbol sym, String errmsg) {
            super();
            this.sym = sym;
            this.errmsg = errmsg;
        }

        public String getMessage() {
            return errmsg;
        }
    }
}

/**
 * @(#)Type.java	1.51 03/04/15
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.tools.javac.v8.code;
import com.sun.tools.javac.v8.util.*;

import com.sun.tools.javac.v8.code.Symbol.*;


/**
 * This class represents GJ types. The class itself defines the behavior of
 *  the following types:
 *
 *  base types (tags: BYTE, CHAR, SHORT, INT, LONG, FLOAT, DOUBLE, BOOLEAN),
 *  type `void' (tag: VOID),
 *  the bottom type (tag: BOT),
 *  the missing type (tag: NONE).
 *
 *  The behavior of the following types is defined in subclasses, which are
 *  all static inner classes of this class:
 *
 *  class types (tag: CLASS, class: ClassType),
 *  array types (tag: ARRAY, class: ArrayType),
 *  method types (tag: METHOD, class: MethodType),
 *  package types (tag: PACKAGE, class: PackageType),
 *  type variables (tag: TYPEVAR, class: TypeVar),
 *  polymorphic types (tag: FORALL, class: ForAll),
 *  the error type (tag: ERROR, class: ErrorType).
 *
 *  @see TypeTags
 */
public class Type implements Flags, Kinds, TypeTags {

    /**
     * Constant type: no type at all.
     */
    public static final Type noType = new Type(NONE, null);

    /**
     * If this switch is turned on, the names of type variables
     *  and anonymous classes are printed with hashcodes appended.
     */
    public static boolean moreInfo = false;

    /**
     * The tag of this type.
     *
     *  @see TypeTags
     */
    public int tag;

    /**
     * The defining class / interface / package / type variable
     */
    public TypeSymbol tsym;

    /**
     * The constant value of this type, null if this type does not have
     *  a constant value attribute. Constant values can be set only for
     *  base types (numbers, booleans) and class types (strings).
     */
    public Object constValue = null;

    /**
     * Define a type given its tag and type symbol
     */
    public Type(int tag, TypeSymbol tsym) {
        super();
        this.tag = tag;
        this.tsym = tsym;
    }

    /**
      * An abstract class for mappings from types to types
      */
    public static abstract class Mapping {

        public Mapping() {
            super();
        }

        public abstract Type apply(Type t);
    }

    /**
      * map a type function over all immediate descendants of this type
      */
    public Type map(Mapping f) {
        return this;
    }

    /**
      * map a type function over a list of types
      */
    public static List map(List these, Mapping f) {
        if (these.nonEmpty()) {
            List tail1 = map(these.tail, f);
            Type head1 = f.apply((Type) these.head);
            if (tail1 != these.tail || head1 != these.head)
                return tail1.prepend(head1);
        }
        return these;
    }

    /**
      * Define a constant type, of the same kind as this type
      *  and with given constant value
      */
    public Type constType(Object constValue) {
        assert tag <= BOOLEAN;
        Type t = new Type(tag, tsym);
        t.constValue = constValue;
        return t;
    }

    /**
      * If this is a constant type, return its underlying type.
      *  Otherwise, return the type itself.
      */
    public Type baseType() {
        if (constValue == null)
            return this;
        else
            return tsym.type;
    }

    /**
      * Return the base types of a list of types.
      */
    public static List baseTypes(List types) {
        if (types.nonEmpty()) {
            Type t = ((Type) types.head).baseType();
            List ts = baseTypes(types.tail);
            if (t != types.head || ts != types.tail)
                return new List(t, ts);
        }
        return types;
    }

    /**
      * Convert to string.
      */
    public String toString() {
        String s =
                (tsym == null || tsym.name == null) ? "null" : tsym.name.toString();
        return s;
    }

    /**
      * The Java source which this type represents.  Use of this method
      *  will result in the loss of the plain-language description for
      *  the type.
      */
    public String toJava() {
        String s = (tsym == null || tsym.name == null) ? "null" : tsym.name.toJava();
        return s;
    }

    /**
      * The Java source which this type represents.  A List will always
      *  be represented as a comma-spearated listing of the elements in
      *  that list.  Use of this method will result in the loss of the
      *  plain-language description for the type.
      */
    public static String toJavaList(List list) {
        if (list.isEmpty()) {
            return "";
        } else {
            StringBuffer buf = new StringBuffer();
            buf.append(((Type) list.head).toJava());
            for (List l = list.tail; l.nonEmpty(); l = l.tail) {
                buf.append(",");
                buf.append(((Type) l.head).toJava());
            }
            return buf.toString();
        }
    }

    /**
      * The constant value of this type, converted to String
      *  PRE: Type has a non-null constValue field.
      */
    public String stringValue() {
        if (tag == BOOLEAN)
            return ((Integer) constValue).intValue() == 0 ? "false" : "true";
        else if (tag == CHAR)
            return String.valueOf((char)((Integer) constValue).intValue());
        else
            return constValue.toString();
    }

    /**
      * Is this a constant type whose value is false?
      */
    public boolean isFalse() {
        return tag == BOOLEAN && constValue != null &&
                ((Integer) constValue).intValue() == 0;
    }

    /**
      * Is this a constant type whose value is true?
      */
    public boolean isTrue() {
        return tag == BOOLEAN && constValue != null &&
                ((Integer) constValue).intValue() != 0;
    }

    /**
      * Access methods.
      */
    public List typarams() {
        return emptyList;
    }

    public Type outer() {
        return null;
    }

    public Type elemtype() {
        return null;
    }

    public int dimensions() {
        return 0;
    }

    public List argtypes() {
        return emptyList;
    }

    public Type restype() {
        return null;
    }

    public List thrown() {
        return Type.emptyList;
    }

    public Type bound() {
        return null;
    }

    /**
      * Set the thrown exceptions for a method type.
      */
    public void setThrown(List t) {
        throw new AssertionError();
    }

    /**
      * Navigation methods, these will work for classes, type variables,
      *  foralls, but will return null for arrays and methods.
      */
    public Type supertype() {
        return null;
    }

    public List interfaces() {
        return emptyList;
    }

    /**
      * Return all parameters of this type and all its outer types in order
      *  outer (first) to inner (last).
      */
    public List allparams() {
        return emptyList;
    }

    /**
      * Return the (most specific) base type of this type that starts
      *  with symbol sym.  If none exists, return null.
      */
    public Type asSuper(Symbol sym) {
        return null;
    }

    /**
      * Return the base type of this type or any
      *  of its outer types that starts with symbol sym.
      *  If none exists, return null.
      */
    public Type asOuterSuper(Symbol sym) {
        return null;
    }

    /**
      * If this type is a (possibly selected) type variable,
      *  return the bounding class of this type, otherwise
      *  return the type itself
      */
    public Type classBound() {
        return this;
    }

    /**
      * Return the least specific subtype of this type that starts
      *  with symbol sym. if none exists, return null.
      *  The least specific subtype is determined as follows:
      *
      *  If there is exactly one parameterized instance of sym
      *  that is a subtype of this type, that parameterized instance is returned.
      *  Otherwise, if the plain type or raw type `sym' is a subtype
      *  of this type, the type `sym' itself is returned.
      *  Otherwise, null is returned.
      */
    Type asSub(Symbol sym) {
        return null;
    }

    /**
      * The type of given symbol, seen as a member of this type
      */
    public Type memberType(Symbol sym) {
        return sym.type;
    }

    /**
      * Substitute all occurrences of a type in `from' with the
      *  corresponding type in `to'. Match lists `from' and `to' from the right:
      *  If lists have different length, discard leading elements of the longer list.
      */
    public Type subst(List from, List to) {
        return this;
    }

    public static List subst(List these, List from, List to) {
        if (these.tail != null && from.tail != null) {
            Type head1 = ((Type) these.head).subst(from, to);
            List tail1 = subst(these.tail, from, to);
            if (head1 != these.head || tail1 != these.tail)
                return tail1.prepend(head1);
        }
        return these;
    }

    /**
      * Does this type contain "error" elements?
      */
    public boolean isErroneous() {
        return false;
    }

    public static boolean isErroneous(List ts) {
        for (List l = ts; l.nonEmpty(); l = l.tail)
            if (((Type) l.head).isErroneous())
                return true;
        return false;
    }

    /**
      * Is this type parameterized?
      *  A class type is parameterized if it has some parameters.
      *  An array type is parameterized if its element type is parameterized.
      *  All other types are not parameterized.
      */
    public boolean isParameterized() {
        return false;
    }

    /**
      * Is this type a raw type?
      *  A class type is a raw type if it misses some of its parameters.
      *  An array type is a raw type if its element type is raw.
      *  All other types are not raw.
      *  Type validation will ensure that the only raw types
      *  in a program are types that miss all their type variables.
      */
    public boolean isRaw() {
        return false;
    }

    public static boolean isRaw(List ts) {
        return false;
    }

    public static boolean isDerivedRaw(Type t) {
        return t.isRaw() || t.supertype() != null &&
                isDerivedRaw(t.supertype()) || isDerivedRaw(t.interfaces());
    }

    public static boolean isDerivedRaw(List ts) {
        List l = ts;
        while (l.nonEmpty() && !isDerivedRaw((Type) l.head))
            l = l.tail;
        return l.nonEmpty();
    }

    /**
      * The erasure of this type -- the type that results when
      *  all type parameters in this type are deleted.
      */
    public Type erasure() {
        if (tag <= lastBaseTag)
            return this;
        else
            return map(erasureFun);
    }

    public static List erasure(List these) {
        return map(these, erasureFun);
    }
    static Mapping erasureFun = new Mapping() {


                                public Type apply(Type t) {
                                    return t.erasure();
                                }
                            };

    /**
     * Does this type contain occurrences of type `elem'?
     */
    public boolean contains(Type elem) {
        return elem == this;
    }

    public static boolean contains(List these, Type elem) {
        for (List l = these; l.tail != null; l = l.tail)
            if (((Type) l.head).contains(elem))
                return true;
        return false;
    }

    /**
      * Does this type contain an occurrence of some type in `elems'?
      */
    public boolean containsSome(List elems) {
        for (List l = elems; l.nonEmpty(); l = l.tail)
            if (this.contains((Type) elems.head))
                return true;
        return false;
    }

    /**
      * Is this type the same as that type?
      */
    public boolean isSameType(Type that) {
        if (this == that)
            return true;
        if (that.tag >= firstPartialTag)
            return that.isSameType(this);
        switch (this.tag) {
        case BYTE:

        case CHAR:

        case SHORT:

        case INT:

        case LONG:

        case FLOAT:

        case DOUBLE:

        case BOOLEAN:

        case VOID:

        case BOT:

        case NONE:
            return this.tag == that.tag;

        default:
            throw new AssertionError("isSameType " + this.tag);

        }
    }

    /**
      * Same for lists of types, if lists are of different length, return false.
      */
    public static boolean isSameTypes(List these, List those) {
        while (these.tail != null && those.tail != null &&
                ((Type) these.head).isSameType((Type) those.head)) {
            these = these.tail;
            those = those.tail;
        }
        return these.tail == null && those.tail == null;
    }

    /**
      * Is this type a subtype of that type?
      *  (not defined for Method and ForAll types)
      */
    public boolean isSubType(Type that) {
        if (this == that)
            return true;
        if (that.tag >= firstPartialTag)
            return that.isSuperType(this);
        switch (this.tag) {
        case BYTE:

        case CHAR:
            return (this.tag == that.tag || this.tag + 2 <= that.tag &&
                    that.tag <= DOUBLE);

        case SHORT:

        case INT:

        case LONG:

        case FLOAT:

        case DOUBLE:
            return this.tag <= that.tag && that.tag <= DOUBLE;

        case BOOLEAN:

        case VOID:
            return this.tag == that.tag;

        case BOT:
            return (that.tag == BOT || that.tag == CLASS || that.tag == ARRAY);

        default:
            throw new AssertionError("isSubType " + this.tag);

        }
    }

    /**
      * Is this type a supertype of that type?
      *  overridden for partial types.
      */
    public boolean isSuperType(Type that) {
        return that.isSubType(this);
    }

    /**
      * Is this type a subtype of every type in given list `those'?
      *  (not defined for Method and ForAll types)
      */
    public boolean isSubType(List those) {
        for (List l = those; l.nonEmpty(); l = l.tail) {
            if (!this.isSubType((Type) l.head))
                return false;
        }
        return true;
    }

    /**
      * Same for lists of types, if lists are of different length, return false.
      */
    public static boolean isSubTypes(List these, List those) {
        while (these.tail != null && those.tail != null &&
                ((Type) these.head).isSubType((Type) those.head)) {
            these = these.tail;
            those = those.tail;
        }
        return these.tail == null && those.tail == null;
    }

    /**
      * If this switch is true, we allow assigning character constants to byte
      *  provided they fit in the range.
      */
    private static final boolean looseInterpretation = true;

    /**
     * Is this type assignable to that type? Equivalent to subtype
     *  except for constant values. (not defined for Method and ForAll types)
     */
    public boolean isAssignable(Type that) {
        if (this.tag <= INT && this.constValue != null) {
            int value = ((Number) this.constValue).intValue();
            switch (that.tag) {
            case BYTE:
                if ((looseInterpretation || this.tag != CHAR) &&
                        Byte.MIN_VALUE <= value && value <= Byte.MAX_VALUE)
                    return true;
                break;

            case CHAR:
                if (Character.MIN_VALUE <= value && value <= Character.MAX_VALUE)
                    return true;
                break;

            case SHORT:
                if (Short.MIN_VALUE <= value && value <= Short.MAX_VALUE)
                    return true;
                break;

            case INT:
                return true;

            }
        }
        return this.isSubType(that);
    }

    /**
      * If this type is castable to that type, return the result of the cast
      *  otherwise return null;
      *  that type is assumed to be an erased type.
      *  (not defined for Method and ForAll types).
      */
    public boolean isCastable(Type that) {
        if (that.tag == ERROR)
            return true;
        switch (this.tag) {
        case BYTE:

        case CHAR:

        case SHORT:

        case INT:

        case LONG:

        case FLOAT:

        case DOUBLE:
            return that.tag <= DOUBLE;

        case BOOLEAN:
            return that.tag == BOOLEAN;

        case VOID:
            return false;

        case BOT:
            return this.isSubType(that);

        default:
            throw new AssertionError();

        }
    }

    /**
      * The underlying method type of this type.
      */
    public MethodType asMethodType() {
        throw new AssertionError();
    }

    /**
      * Does this type have the same arguments as that type?
      *  It is assumed that both types are (possibly polymorphic) method types.
      *  Monomorphic method types "have the same arguments",
      *  if their argument lists are equal.
      *  Polymorphic method types "have the same arguments",
      *  if they have the same arguments after renaming all type variables
      *  of one to corresponding type variables in the other, where
      *  correspondence is by position in the type parameter list.
      */
    public boolean hasSameArgs(Type that) {
        throw new AssertionError();
    }

    /**
      * Complete loading all classes in this type.
      */
    public void complete() {
    }

    public Object clone() {
        try {
            return super.clone();
        } catch (CloneNotSupportedException e) {
            throw new AssertionError(e);
        }
    }

    /**
      * An empty list of types.
      */
    public static final List emptyList = new List();

    public static class ClassType extends Type {

        /**
         * The enclosing type of this type. If this is the type of an inner
         *  class, outer_field refers to the type of its enclosing
         *  instance class, in all other cases it referes to noType.
         */
        public Type outer_field;

        /**
         * The type parameters of this type (to be set once class is loaded).
         */
        public List typarams_field;

        /**
         * A cache variable for the type parameters of this type,
         *  appended to all parameters of its enclosing class.
         *  @see allparams()
         */
        public List allparams_field;

        /**
         * The supertype of this class (to be set once class is loaded).
         */
        public Type supertype_field;

        /**
         * The interfaces of this class (to be set once class is loaded).
         */
        public List interfaces_field;

        public ClassType(Type outer, List typarams, TypeSymbol tsym) {
            super(CLASS, tsym);
            this.outer_field = outer;
            this.typarams_field = typarams;
            this.allparams_field = null;
            this.supertype_field = null;
            this.interfaces_field = null;
        }

        /**
          * make a compound type from non-empty list of types
          * @param bounds            the types from which the compound type is formed
          * @param tsym              the compound type's type symbol
          * @param supertype         is objectType if all bounds are interfaces,
          *                          null otherwise.
          */
        static Type makeCompoundType(List bounds, TypeSymbol tsym, Type supertype) {
            ClassSymbol bc = new ClassSymbol(ABSTRACT | PUBLIC | SYNTHETIC | COMPOUND,
                    tsym.name, tsym);
            bc.erasure_field = ((Type) bounds.head).erasure();
            bc.members_field = new Scope(bc);
            ClassType bt = (ClassType) bc.type;
            bt.allparams_field = Type.emptyList;
            if (supertype != null) {
                bt.supertype_field = supertype;
                bt.interfaces_field = bounds;
            } else {
                bt.supertype_field = (Type) bounds.head;
                bt.interfaces_field = bounds.tail;
            }
            return bt;
        }

        /**
          * Same as previous function, except that third parameter is
          *  computed directly. Note that this test might cause a symbol completion.
          *  Hence, this version of makeCompoundType may not be called during
          *  a classfile read.
          */
        static Type makeCompoundType(List bounds, TypeSymbol tsym) {
            Type supertype = (((Type) bounds.head).tsym.flags() & INTERFACE) != 0 ?
                    ((Type) bounds.head).supertype() : null;
            return makeCompoundType(bounds, tsym, supertype);
        }

        public Type constType(Object constValue) {
            Type t = new ClassType(outer_field, typarams_field, tsym);
            t.constValue = constValue;
            return t;
        }

        public String toString() {
            StringBuffer buf = new StringBuffer();
            if (outer().tag == CLASS && tsym.owner.kind == TYP) {
                buf.append(outer().toString());
                buf.append(".");
                buf.append(className(tsym, false));
            } else {
                buf.append(className(tsym, true));
            }
            if (typarams().nonEmpty()) {
                buf.append('<');
                buf.append(typarams().toString());
                buf.append(">");
            }
            return buf.toString();
        }

        private String className(Symbol sym, boolean longform) {
            if (sym.name.len == 0)
                return "<anonymous " + (sym.type.interfaces().nonEmpty() ?
                        (Type) sym.type.interfaces().head :
                        sym.type.supertype()) + ">" +
                        (moreInfo ? String.valueOf(sym.hashCode()) : "");
            else if (longform)
                return sym.fullName().toString();
            else
                return sym.name.toString();
        }

        /**
          * The Java source which this type represents.  Use of this method
          *  will result in the loss of the plain-language description for
          *  the type.
          */
        public String toJava() {
            StringBuffer buf = new StringBuffer();
            if (outer().tag == CLASS && tsym.owner.kind == TYP) {
                buf.append(outer().toJava());
                buf.append(".");
                buf.append(javaClassName(tsym, false));
            } else {
                buf.append(javaClassName(tsym, true));
            }
            if (typarams().nonEmpty()) {
                buf.append('<');
                buf.append(typarams().toString());
                buf.append(">");
            }
            return buf.toString();
        }

        private String javaClassName(Symbol sym, boolean longform) {
            if (sym.name.len == 0) {
                String s;
                if (sym.type.interfaces().nonEmpty()) {
                    s = Log.getLocalizedString("anonymous.class",
                            ((Type) sym.type.interfaces().head).toJava());
                } else {
                    s = Log.getLocalizedString("anonymous.class",
                            sym.type.supertype().toJava());
                }
                if (moreInfo)
                    s += String.valueOf(sym.hashCode());
                return s;
            } else if (longform) {
                return sym.fullName().toString();
            } else {
                return sym.name.toString();
            }
        }

        public List typarams() {
            if (typarams_field == null) {
                complete();
                typarams_field = tsym.type.typarams();
            }
            return typarams_field;
        }

        public Type outer() {
            if (outer_field == null) {
                complete();
                outer_field = tsym.type.outer();
            }
            return outer_field;
        }

        public Type supertype() {
            if (supertype_field == null) {
                complete();
                Type st = ((ClassType) tsym.type).supertype_field;
                if (st == null) {
                    supertype_field = noType;
                } else if (this == tsym.type) {
                    supertype_field = st;
                } else {
                    List ownparams = classBound().allparams();
                    List symparams = tsym.type.allparams();
                    supertype_field = (ownparams.isEmpty()) ? st.erasure() :
                            st.subst(symparams, ownparams);
                }
            }
            return supertype_field;
        }

        public List interfaces() {
            if (interfaces_field == null) {
                complete();
                List is = ((ClassType) tsym.type).interfaces_field;
                if (is == null) {
                    interfaces_field = Type.emptyList;
                } else if (this == tsym.type) {
                    interfaces_field = is;
                } else {
                    List ownparams = allparams();
                    List symparams = tsym.type.allparams();
                    if (ownparams.isEmpty())
                        interfaces_field = erasure(is);
                    else
                        interfaces_field = subst(is, symparams, ownparams);
                }
            }
            return interfaces_field;
        }

        public List allparams() {
            if (allparams_field == null) {
                allparams_field = typarams().prependList(outer().allparams());
            }
            return allparams_field;
        }

        public Type asSuper(Symbol sym) {
            if (tsym == sym) {
                return this;
            } else {
                Type st = supertype();
                if (st.tag == CLASS || st.tag == ERROR) {
                    Type t = st.asSuper(sym);
                    if (t != null)
                        return t;
                }
                if ((sym.flags() & INTERFACE) != 0) {
                    for (List l = interfaces(); l.nonEmpty(); l = l.tail) {
                        Type t = ((Type) l.head).asSuper(sym);
                        if (t != null)
                            return t;
                    }
                }
                return null;
            }
        }

        public Type asOuterSuper(Symbol sym) {
            Type t = this;
            do {
                Type s = t.asSuper(sym);
                if (s != null)
                    return s;
                t = t.outer();
            } while (t.tag == CLASS)
                ;
            return null;
        }

        public Type classBound() {
            Type outer1 = outer().classBound();
            if (outer1 != outer_field)
                return new ClassType(outer1, typarams(), tsym);
            else
                return this;
        }

        public Type asSub(Symbol sym) {
            if (tsym == sym) {
                return this;
            } else {
                Type base = sym.type.asSuper(tsym);
                if (base == null)
                    return null;
                ListBuffer from = new ListBuffer();
                ListBuffer to = new ListBuffer();
                adapt(base, this, from, to);
                Type res = sym.type.subst(from.toList(), to.toList());
                if (!res.isSubType(this))
                    return null;
                for (List l = sym.type.allparams(); l.nonEmpty(); l = l.tail)
                    if (res.contains((Type) l.head) && !this.contains((Type) l.head))
                        return res.erasure();
                return res;
            }
        }

        /**
          * Adapt a type by computing a substitution which
          *  maps a source type to a target type.
          *  @param from      the source type
          *  @param target    the target type
          *  @param from      the type variables of the computed substitution
          *  @param to        the types of the computed substitution.
          */
        private static void adapt(Type source, Type target, ListBuffer from,
                ListBuffer to) {
            if (source.tag == target.tag) {
                switch (source.tag) {
                case CLASS:
                    adapt(source.allparams(), target.allparams(), from, to);
                    break;

                case ARRAY:
                    adapt(source.elemtype(), target.elemtype(), from, to);

                }
            }
        }

        /**
          * Adapt a type by computing a substitution which
          *  maps a list of source types to a list of target types.
          *  @param source    the source type
          *  @param target    the target type
          *  @param from      the type variables of the computed substitution
          *  @param to        the types of the computed substitution.
          */
        private static void adapt(List source, List target, ListBuffer from,
                ListBuffer to) {
            if (source.length() == target.length()) {
                while (source.nonEmpty()) {
                    adapt((Type) source.head, (Type) target.head, from, to);
                    source = source.tail;
                    target = target.tail;
                }
            }
        }

        public Type memberType(Symbol sym) {
            Symbol owner = sym.owner;
            long flags = sym.flags();
            if (((flags & STATIC) == 0) && owner.type.isParameterized()) {
                Type base = this.asOuterSuper(owner);
                if (base != null) {
                    List ownerParams = owner.type.allparams();
                    List baseParams = base.allparams();
                    if (ownerParams.nonEmpty()) {
                        if (baseParams.isEmpty()) {
                            return sym.type.erasure();
                        } else {
                            return sym.type.subst(ownerParams, baseParams);
                        }
                    }
                }
            }
            return sym.type;
        }

        public Type subst(List from, List to) {
            if (from.tail == null) {
                return this;
            } else if ((tsym.flags() & COMPOUND) == 0) {
                Type outer = outer();
                List typarams = typarams();
                List typarams1 = subst(typarams, from, to);
                Type outer1 = outer.subst(from, to);
                if (typarams1 == typarams && outer1 == outer)
                    return this;
                else
                    return new ClassType(outer1, typarams1, tsym);
            } else {
                Type st = supertype().subst(from, to);
                List is = subst(interfaces(), from, to);
                if (st == supertype() && is == interfaces())
                    return this;
                else
                    return makeCompoundType(is.prepend(st), tsym);
            }
        }

        public boolean isErroneous() {
            return outer().isErroneous() || isErroneous(typarams()) ||
                    this != tsym.type && tsym.type.isErroneous();
        }

        public boolean isParameterized() {
            return allparams().tail != null;
        }

        public Type erasure() {
            return tsym.erasure();
        }

        public Type map(Mapping f) {
            Type outer = outer();
            Type outer1 = f.apply(outer);
            List typarams = typarams();
            List typarams1 = map(typarams, f);
            if (outer1 == outer && typarams1 == typarams)
                return this;
            else
                return new ClassType(outer1, typarams1, tsym);
        }

        public boolean contains(Type elem) {
            return elem == this || (isParameterized() &&
                    (outer().contains(elem) || contains(typarams(), elem)));
        }

        public boolean isSameType(Type that) {
            if (this == that)
                return true;
            if (that.tag >= firstPartialTag)
                return that.isSameType(this);
            return this.tsym == that.tsym && this.outer().isSameType(that.outer()) &&
                    isSameTypes(this.typarams(), that.typarams());
        }

        public boolean isSubType(Type that) {
            if (this == that)
                return true;
            if (that.tag >= firstPartialTag)
                return that.isSuperType(this);
            if (this.tsym == that.tsym)
                return (!that.isParameterized() ||
                        isSameTypes(this.typarams(), that.typarams())) &&
                        this.outer().isSubType(that.outer());
            if ((that.tsym.flags() & INTERFACE) != 0)
                for (List is = this.interfaces(); is.nonEmpty(); is = is.tail)
                    if (((Type) is.head).isSubType(that))
                        return true;
            Type st = this.supertype();
            if (st.tag == CLASS && st.isSubType(that))
                return true;
            return st.isErroneous();
        }

        /**
          * The rules for castability are extended to parameterized types
          *  as follows:
          *  (1) One may always cast to a supertype
          *  (2) One may cast to a subtype C<...> provided there is only
          *      one type with the subtype's class part, C, that is a subtype
          *      of this type. (This is equivalent to: C<...> = this.asSub(C).
          *  (3) One may cast an interface to an unparameterized class.
          *  (4) One may cast a non-final class to an unparameterized interface.
          */
        public boolean isCastable(Type that) {
            return that.tag == ERROR || (that.tag == CLASS || that.tag == ARRAY) &&
                    (this.isSubType(that) || (that.isSubType(this) &&
                    (that.tag == ARRAY || that.tsym.type == that ||
                    (this.asSub(that.tsym) != null &&
                    that.isSameType(this.asSub(that.tsym))))) ||
                    that.tag == CLASS && that.allparams().isEmpty() &&
                    ((that.tsym.flags() & INTERFACE) != 0 &&
                    (this.tsym.flags() & FINAL) == 0 ||
                    (this.tsym.flags() & INTERFACE) != 0 &&
                    (that.tsym.flags() & FINAL) == 0));
        }

        public void complete() {
            if (tsym.completer != null)
                tsym.complete();
        }
    }

    public static class ArrayType extends Type {
        public Type elemtype;

        public ArrayType(Type elemtype, TypeSymbol arrayClass) {
            super(ARRAY, arrayClass);
            this.elemtype = elemtype;
        }

        public String toString() {
            return elemtype + "[]";
        }

        public String toJava() {
            return toString();
        }

        public boolean equals(Object that) {
            return this == that || (that instanceof ArrayType &&
                    this.elemtype.equals(((ArrayType) that).elemtype));
        }

        public int hashCode() {
            return (ARRAY << 5) + elemtype.hashCode();
        }

        public Type elemtype() {
            return elemtype;
        }

        public int dimensions() {
            int result = 0;
            for (Type t = this; t.tag == ARRAY; t = t.elemtype()) {
                result++;
            }
            return result;
        }

        public List allparams() {
            return elemtype.allparams();
        }

        public Type subst(List from, List to) {
            if (from.tail == null) {
                return this;
            } else {
                Type elemtype1 = elemtype.subst(from, to);
                if (elemtype1 == elemtype)
                    return this;
                else
                    return new ArrayType(elemtype1, tsym);
            }
        }

        public boolean isErroneous() {
            return elemtype.isErroneous();
        }

        public boolean isParameterized() {
            return elemtype.isParameterized();
        }

        public Type map(Mapping f) {
            Type elemtype1 = f.apply(elemtype);
            if (elemtype1 == elemtype)
                return this;
            else
                return new ArrayType(elemtype1, tsym);
        }

        public boolean contains(Type elem) {
            return elem == this || elemtype.contains(elem);
        }

        public Type asSuper(Symbol sym) {
            return (this.isSubType(sym.type)) ? sym.type : null;
        }

        public Type asOuterSuper(Symbol sym) {
            return (this.isSubType(sym.type)) ? sym.type : null;
        }

        public boolean isSameType(Type that) {
            if (this == that)
                return true;
            if (that.tag >= firstPartialTag)
                return that.isSameType(this);
            return that.tag == ARRAY && this.elemtype.isSameType(that.elemtype());
        }

        public boolean isSubType(Type that) {
            if (this == that)
                return true;
            if (that.tag >= firstPartialTag)
                return that.isSuperType(this);
            if (that.tag == ARRAY) {
                if (this.elemtype.tag <= lastBaseTag)
                    return this.elemtype.isSameType(that.elemtype());
                else
                    return this.elemtype.isSubType(that.elemtype());
            } else if (that.tag == CLASS) {
                Name thatname = that.tsym.fullName();
                Name.Table names = thatname.table;
                return (thatname == names.java_lang_Object ||
                        thatname == names.java_lang_Cloneable ||
                        thatname == names.java_io_Serializable);
            } else {
                return false;
            }
        }

        public boolean isCastable(Type that) {
            return that.tag == ERROR || (that.tag == CLASS && this.isSubType(that)) ||
                    that.tag == ARRAY && (this.elemtype().tag <= lastBaseTag ?
                    this.elemtype().tag == that.elemtype().tag :
                    this.elemtype().isCastable(that.elemtype()));
        }

        public void complete() {
            elemtype.complete();
        }
    }

    public static class MethodType extends Type implements Cloneable {
        public List argtypes;
        public Type restype;
        public List thrown;

        public MethodType(List argtypes, Type restype, List thrown,
                TypeSymbol methodClass) {
            super(METHOD, methodClass);
            this.argtypes = argtypes;
            this.restype = restype;
            this.thrown = thrown;
        }

        public String toString() {
            return "(" + argtypes.toString() + ")" + restype;
        }

        /**
          * The Java source which this type represents.  Use of this method
          *  will result in the loss of the plain-language description for
          *  the type.
          *
          *  XXX 06/09/99 iris This isn't correct Java syntax, but it probably
          *  should be.
          */
        public String toJava() {
            return "(" + argtypes.toString() + ")" + restype.toString();
        }

        public boolean equals(Object that) {
            if (this == that)
                return true;
            if (!(that instanceof MethodType))
                return false;
            MethodType mthat = (MethodType) that;
            List thisargs = this.argtypes;
            List thatargs = mthat.argtypes;
            while (thisargs.tail != null && thatargs.tail != null &&
                    ((Type) thisargs.head).equals(thatargs.head)) {
                thisargs = thisargs.tail;
                thatargs = thatargs.tail;
            }
            if (thisargs.tail != null || thatargs.tail != null)
                return false;
            return this.restype.equals(mthat.restype);
        }

        public int hashCode() {
            int h = METHOD;
            for (List thisargs = this.argtypes; thisargs.tail != null;
                    thisargs = thisargs.tail)
                h = (h << 5) + ((Type) thisargs.head).hashCode();
            return (h << 5) + this.restype.hashCode();
        }

        public List argtypes() {
            return argtypes;
        }

        public Type restype() {
            return restype;
        }

        public List thrown() {
            return thrown;
        }

        public void setThrown(List t) {
            thrown = t;
        }

        public Type subst(List from, List to) {
            if (from.tail == null) {
                return this;
            } else {
                List argtypes1 = subst(argtypes, from, to);
                Type restype1 = restype.subst(from, to);
                List thrown1 = subst(thrown, from, to);
                if (argtypes1 == argtypes && restype1 == restype && thrown1 == thrown)
                    return this;
                else
                    return new MethodType(argtypes1, restype1, thrown1, tsym);
            }
        }

        public boolean isErroneous() {
            return isErroneous(argtypes) || restype.isErroneous();
        }

        public Type map(Mapping f) {
            List argtypes1 = map(argtypes, f);
            Type restype1 = f.apply(restype);
            List thrown1 = map(thrown, f);
            if (argtypes1 == argtypes && restype1 == restype && thrown1 == thrown)
                return this;
            else
                return new MethodType(argtypes1, restype1, thrown1, tsym);
        }

        public boolean contains(Type elem) {
            return elem == this || contains(argtypes, elem) || restype.contains(elem);
        }

        public MethodType asMethodType() {
            return this;
        }

        public boolean hasSameArgs(Type that) {
            return that.tag == METHOD && isSameTypes(this.argtypes, that.argtypes());
        }

        /**
          * isSameType for methods does not take thrown exceptions into account!
          */
        public boolean isSameType(Type that) {
            return hasSameArgs(that) && restype.isSameType(that.restype());
        }

        public void complete() {
            for (List l = argtypes; l.nonEmpty(); l = l.tail)
                ((Type) l.head).complete();
            restype.complete();
            for (List l = thrown; l.nonEmpty(); l = l.tail)
                ((Type) l.head).complete();
        }
    }

    public static class PackageType extends Type {

        PackageType(TypeSymbol tsym) {
            super(PACKAGE, tsym);
        }

        public String toString() {
            return tsym.fullName().toString();
        }

        public boolean isSameType(Type that) {
            return this == that;
        }
    }

    public static class ErrorType extends ClassType {

        public ErrorType() {
            super(noType, emptyList, null);
            tag = ERROR;
        }

        public ErrorType(ClassSymbol c) {
            this();
            tsym = c;
            c.type = this;
            c.kind = ERR;
            c.members_field = new Scope.ErrorScope(c);
        }

        public ErrorType(Name name, TypeSymbol container) {
            this(new ClassSymbol(PUBLIC | STATIC, name, null, container));
        }

        public Type constType(Object constValue) {
            return this;
        }

        public Type outer() {
            return this;
        }

        public Type elemtype() {
            return this;
        }

        public Type restype() {
            return this;
        }

        public Type asSuper(Symbol sym) {
            return this;
        }

        public Type asOuterSuper(Symbol sym) {
            return this;
        }

        public Type asSub(Symbol sym) {
            return this;
        }

        public Type memberType(Symbol sym) {
            return this;
        }

        public Type classBound() {
            return this;
        }

        public Type subst(List from, List to) {
            return this;
        }

        public Type erasure() {
            return this;
        }

        public Type unerasure() {
            return this;
        }

        public boolean isGenType(Type that) {
            return true;
        }

        public boolean isErroneous() {
            return true;
        }

        public boolean isSameType(Type that) {
            return true;
        }

        public boolean isSubType(Type that) {
            return true;
        }

        public boolean isSuperType(Type that) {
            return true;
        }

        public boolean isCastable(Type that) {
            return true;
        }

        public boolean hasSameArgs(Type that) {
            return false;
        }

        public boolean isAssignable(Type that) {
            return true;
        }

        public List allparams() {
            return emptyList;
        }

        public List typarams() {
            return emptyList;
        }
    }
}

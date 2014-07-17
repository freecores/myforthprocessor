/**
 * @(#)ClassDocImpl.java	1.56 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.tools.javadoc;
import com.sun.javadoc.*;

import com.sun.tools.javac.v8.util.Hashtable;

import com.sun.tools.javac.v8.util.List;

import com.sun.tools.javac.v8.util.ListBuffer;

import com.sun.tools.javac.v8.util.Name;

import com.sun.tools.javac.v8.util.Position;

import com.sun.tools.javac.v8.code.Flags;

import com.sun.tools.javac.v8.code.Kinds;

import com.sun.tools.javac.v8.code.TypeTags;

import com.sun.tools.javac.v8.code.Type;

import com.sun.tools.javac.v8.code.Type.ClassType;

import com.sun.tools.javac.v8.code.Type.MethodType;

import com.sun.tools.javac.v8.code.Scope;

import com.sun.tools.javac.v8.code.Symbol;

import com.sun.tools.javac.v8.code.Symbol.VarSymbol;

import com.sun.tools.javac.v8.code.Symbol.ClassSymbol;

import com.sun.tools.javac.v8.code.Symbol.CompletionFailure;

import com.sun.tools.javac.v8.code.Symbol.MethodSymbol;

import com.sun.tools.javac.v8.code.Symbol.PackageSymbol;

import com.sun.tools.javac.v8.comp.AttrContext;

import com.sun.tools.javac.v8.comp.Env;

import com.sun.tools.javac.v8.tree.Tree;

import com.sun.tools.javac.v8.tree.Tree.Select;

import com.sun.tools.javac.v8.tree.Tree.Import;

import com.sun.tools.javac.v8.tree.Tree.ClassDef;

import com.sun.tools.javac.v8.tree.TreeInfo;

import java.io.File;

import java.util.Set;

import java.util.HashSet;

import java.lang.reflect.Modifier;


/**
 * Represents a java class and provides access to information
 * about the class, the class' comment and tags, and the
 * members of the class.  A ClassDocImpl only exists if it was
 * processed in this run of javadoc.  References to classes
 * which may or may not have been processed in this run are
 * referred to using Type (which can be converted to ClassDocImpl,
 * if possible).
 *
 * @see Type
 *
 * @since JDK1.2
 * @author Robert Field
 * @author Neal Gafter (rewrite)
 */
public class ClassDocImpl extends ProgramElementDocImpl implements ClassDoc {
    protected final ClassType type;
    protected final ClassSymbol tsym;
    ClassDef tree = null;
    boolean isIncluded = false;
    private SerializedForm serializedForm;

    /**
     * Constructor
     */
    public ClassDocImpl(DocEnv env, ClassType type) {
        super(env, null);
        this.type = type;
        this.tsym = (ClassSymbol) type.tsym;
    }

    /**
      * Constructor
      */
    public ClassDocImpl(DocEnv env, ClassSymbol sym, String documentation,
            Tree.ClassDef tree) {
        super(env, documentation);
        this.type = (ClassType) sym.type;
        this.tsym = sym;
        this.tree = tree;
    }

    /**
      * Returns the flags in terms of javac's flags
      */
    protected long getFlags() {
        while (true) {
            try {
                return tsym.flags();
            } catch (CompletionFailure ex) {
            }
        }
    }

    /**
      * Identify the containing class
      */
    protected ClassSymbol getContainingClass() {
        return tsym.owner.enclClass();
    }

    /**
      * Constructor
      */
    public ClassDocImpl(DocEnv env, ClassSymbol sym) {
        this(env, sym, null, null);
    }

    /**
      * Return true if this is a class, not an interface.
      */
    public boolean isClass() {
        return !Modifier.isInterface(getModifiers());
    }

    /**
      * Return true if this is a ordinary class,
      * not an exception, an error, or an interface.
      */
    public boolean isOrdinaryClass() {
        if (Modifier.isInterface(getModifiers())) {
            return false;
        }
        for (Type t = type; t.tag == TypeTags.CLASS; t = t.supertype()) {
            if (t.tsym == env.syms.errorType.tsym ||
                    t.tsym == env.syms.exceptionType.tsym) {
                return false;
            }
        }
        return true;
    }

    /**
      * Return true if this is an interface.
      */
    public boolean isInterface() {
        return Modifier.isInterface(getModifiers());
    }

    /**
      * Return true if this is an exception class
      */
    public boolean isException() {
        if (Modifier.isInterface(getModifiers())) {
            return false;
        }
        for (Type t = type; t.tag == TypeTags.CLASS; t = t.supertype()) {
            if (t.tsym == env.syms.exceptionType.tsym) {
                return true;
            }
        }
        return false;
    }

    /**
      * Return true if this is an error class
      */
    public boolean isError() {
        if (Modifier.isInterface(getModifiers())) {
            return false;
        }
        for (Type t = type; t.tag == TypeTags.CLASS; t = t.supertype()) {
            if (t.tsym == env.syms.errorType.tsym) {
                return true;
            }
        }
        return false;
    }

    /**
      * Return true if this is a throwable class
      */
    public boolean isThrowable() {
        if (Modifier.isInterface(getModifiers())) {
            return false;
        }
        for (Type t = type; t.tag == TypeTags.CLASS; t = t.supertype()) {
            if (t.tsym == env.syms.throwableType.tsym) {
                return true;
            }
        }
        return false;
    }

    /**
      * Return true if this class is abstract
      */
    public boolean isAbstract() {
        return Modifier.isAbstract(getModifiers());
    }

    /**
      * Returns true if this class was synthesized by the compiler.
      */
    public boolean isSynthetic() {
        return (getFlags() & Flags.SYNTHETIC) != 0;
    }

    /**
      * Return true if this class is included in the active set.
      * A ClassDoc is included iff either it is specified on the
      * commandline, or if it's containing package is specified
      * on the command line, or if it is a member class of an
      * included class.
      */
    public boolean isIncluded() {
        if (isIncluded) {
            return true;
        }
        if (tsym.type != type)
            return false;
        if (env.shouldDocument(tsym)) {
            if (containingPackage().isIncluded()) {
                return isIncluded = true;
            }
            ClassDoc outer = containingClass();
            if (outer != null && outer.isIncluded()) {
                return isIncluded = true;
            }
        }
        return false;
    }

    /**
      * Return the package that this class is contained in.
      */
    public PackageDoc containingPackage() {
        PackageDocImpl p = env.getPackageDoc(tsym.packge());
        SourcePosition po = position();
        if (po != null && p.docPath == null && p.zipDocPath == null) {
            File packageDir = po.file().getParentFile();
            if (packageDir != null &&
                    (new File(packageDir, "package.html")).exists()) {
                p.setDocPath(packageDir.getPath());
            }
        }
        return p;
    }

    /**
      * Return the class name without package qualifier - but with
      * enclosing class qualifier - as a String.
      * <pre>
      * Example:
      *  for java.util.Hashtable
      *  return Hashtable
      * </pre>
      */
    public String name() {
        String n = tsym.name.toString();
        for (ClassSymbol c = tsym.owner.enclClass(); c != null;
                c = c.owner.enclClass()) {
            n = c.name.toString() + "." + n;
        }
        return n + genericArgs();
    }

    /**
      * Return the generic args for a class.
      */
    private String genericArgs() {
        return (type.typarams().nonEmpty()) ? "<" + type.typarams() + ">" : "";
    }

    /**
      * Return unqualified name of type excluding any dimension information.
      * <p>
      * For example, a two dimensional array of String returns 'String'.
      */
    public String typeName() {
        return name();
    }

    /**
      * Return the qualified class name as a String.
      * <pre>
      * Example:
      *  for java.util.Hashtable
      *  return java.util.Hashtable
      *  if no qualifier, just return flat name
      * </pre>
      */
    public String qualifiedName() {
        return tsym.fullName().toString() + genericArgs();
    }

    /**
      * Return qualified name of type excluding any dimension information.
      * <p>
      * For example, a two dimensional array of String
      * returns 'java.lang.String'.
      */
    public String qualifiedTypeName() {
        return qualifiedName();
    }

    /**
      * Return the modifier string for this class. If it's an interface
      * exclude 'abstract' keyword from the modifer string
      */
    public String modifiers() {
        return Modifier.toString(modifierSpecifier());
    }

    public int modifierSpecifier() {
        int modifiers = getModifiers();
        return (isInterface() ? modifiers & ~Modifier.ABSTRACT : modifiers);
    }

    /**
      * Return the superclass of this class
      *
      * @return the ClassDocImpl for the superclass of this class, null
      * if there is no superclass.
      */
    public ClassDoc superclass() {
        if (isInterface())
            return null;
        if (tsym == env.syms.objectType.tsym)
            return null;
        ClassSymbol c = (ClassSymbol) type.supertype().tsym;
        if (c == null)
            c = (ClassSymbol) env.syms.objectType.tsym;
        return env.getClassDoc(c);
    }

    /**
      * Test whether this class is a subclass of the specified class.
      *
      * @param cd the candidate superclass.
      * @return true if cd is a superclass of this class.
      */
    public boolean subclassOf(ClassDoc cd) {
        return tsym.isSubClass(((ClassDocImpl) cd).tsym);
    }

    /**
      * Return interfaces implemented by this class or interfaces
      * extended by this interface.
      *
      * @return An array of ClassDocImpl representing the interfaces.
      * Return an empty array if there are no interfaces.
      */
    public ClassDoc[] interfaces() {
        ListBuffer ta = new ListBuffer();
        for (List l = type.interfaces(); l.nonEmpty(); l = l.tail) {
            ta.append(env.getClassDoc((ClassSymbol)((ClassType) l.head).tsym));
        }
        return (ClassDoc[]) ta.toArray(new ClassDocImpl[ta.length()]);
    }

    /**
      * Return fields in class.
      *
      * @param filter include only the included fields if filter==true
      * @return an array of FieldDocImpl for representing the visible
      * fields in this class.
      */
    public FieldDoc[] fields(boolean filter) {
        List fields = new List();
        for (Scope.Entry e = tsym.members().elems; e != null; e = e.sibling) {
            if (e.sym != null && e.sym.kind == Symbol.VAR) {
                VarSymbol s = (VarSymbol) e.sym;
                if (!filter || env.shouldDocument(s)) {
                    fields = fields.prepend(env.getFieldDoc(s));
                }
            }
        }
        return (FieldDoc[]) fields.toArray(new FieldDocImpl[fields.length()]);
    }

    /**
      * Return included fields in class.
      *
      * @return an array of FieldDocImpl for representing the visible
      * fields in this class.
      */
    public FieldDoc[] fields() {
        return fields(true);
    }

    /**
      * Return methods in class.
      *
      * @param filter include only the included methods if filter==true
      * @return an array of MethodDocImpl for representing the visible
      * methods in this class.  Does not include constructors.
      */
    public MethodDoc[] methods(boolean filter) {
        Name.Table names = tsym.name.table;
        List methods = new List();
        for (Scope.Entry e = tsym.members().elems; e != null; e = e.sibling) {
            if (e.sym != null && e.sym.kind == Kinds.MTH &&
                    e.sym.name != names.init) {
                MethodSymbol s = (MethodSymbol) e.sym;
                if (!filter || env.shouldDocument(s)) {
                    methods = methods.prepend(env.getMethodDoc(s));
                }
            }
        }
        return (MethodDoc[]) methods.toArray(new MethodDocImpl[methods.length()]);
    }

    /**
      * Return included methods in class.
      *
      * @return an array of MethodDocImpl for representing the visible
      * methods in this class.  Does not include constructors.
      */
    public MethodDoc[] methods() {
        return methods(true);
    }

    /**
      * Return constructors in class.
      *
      * @param filter include only the included constructors if filter==true
      * @return an array of ConstructorDocImpl for representing the visible
      * constructors in this class.
      */
    public ConstructorDoc[] constructors(boolean filter) {
        Name.Table names = tsym.name.table;
        List constructors = new List();
        for (Scope.Entry e = tsym.members().elems; e != null; e = e.sibling) {
            if (e.sym != null && e.sym.kind == Kinds.MTH &&
                    e.sym.name == names.init) {
                MethodSymbol s = (MethodSymbol) e.sym;
                if (!filter || env.shouldDocument(s)) {
                    constructors = constructors.prepend(env.getConstructorDoc(s));
                }
            }
        }
        return (ConstructorDoc[]) constructors.toArray(
                new ConstructorDocImpl[constructors.length()]);
    }

    /**
      * Return included constructors in class.
      *
      * @return an array of ConstructorDocImpl for representing the visible
      * constructors in this class.
      */
    public ConstructorDoc[] constructors() {
        return constructors(true);
    }

    /**
      * Adds all inner classes of this class, and their
      * inner classes recursively, to the list l.
      */
    void addAllClasses(ListBuffer l, boolean filtered) {
        if (tsym.type != type)
            return;
        try {
            if (isSynthetic())
                return;
            if (filtered && !env.shouldDocument(tsym))
                return;
            if (l.contains(this))
                return;
            l.append(this);
            List more = new List();
            for (Scope.Entry e = tsym.members().elems; e != null; e = e.sibling) {
                if (e.sym != null && e.sym.kind == Symbol.TYP) {
                    ClassSymbol s = (ClassSymbol) e.sym;
                    ClassDocImpl c = env.getClassDoc(s);
                    if (c.isSynthetic())
                        continue;
                    if (c != null)
                        more = more.prepend(c);
                }
            }
            for (; more.nonEmpty(); more = more.tail) {
                ((ClassDocImpl) more.head).addAllClasses(l, filtered);
            }
        } catch (CompletionFailure e) {
        }
    }

    /**
      * Return inner classes within this class.
      *
      * @param filter include only the included inner classes if filter==true.
      * @return an array of ClassDocImpl for representing the visible
      * classes defined in this class. Anonymous and local classes
      * are not included.
      */
    public ClassDoc[] innerClasses(boolean filter) {
        ListBuffer innerClasses = new ListBuffer();
        for (Scope.Entry e = tsym.members().elems; e != null; e = e.sibling) {
            if (e.sym != null && e.sym.kind == Symbol.TYP) {
                ClassSymbol s = (ClassSymbol) e.sym;
                if ((s.flags_field & Flags.SYNTHETIC) != 0)
                    continue;
                if (!filter || env.isVisible(s)) {
                    innerClasses.prepend(env.getClassDoc(s));
                }
            }
        }
        return (ClassDoc[]) innerClasses.toArray(
                new ClassDocImpl[innerClasses.length()]);
    }

    /**
      * Return included inner classes within this class.
      *
      * @return an array of ClassDocImpl for representing the visible
      * classes defined in this class. Anonymous and local classes
      * are not included.
      */
    public ClassDoc[] innerClasses() {
        return innerClasses(true);
    }

    /**
      * Find a class within the context of this class.
      * Search order: qualified name, in this class (inner),
      * in this package, in the class imports, in the package
      * imports.
      * Return the ClassDocImpl if found, null if not found.
      */
    public ClassDoc findClass(String className) {
        Name.Table names = tsym.name.table;
        ClassDoc cd = env.lookupClass(className);
        if (cd != null) {
            return cd;
        }
        ClassDoc[] innerClasses = innerClasses();
        for (int i = 0; i < innerClasses.length; i++) {
            if (innerClasses[i].name().equals(className) ||
                    innerClasses[i].name().endsWith(className)) {
                return innerClasses[i];
            } else {
                ClassDoc innercd = innerClasses[i].findClass(className);
                if (innercd != null) {
                    return innercd;
                }
            }
        }
        cd = containingPackage().findClass(className);
        if (cd != null) {
            return cd;
        }
        if (tsym.completer != null) {
            tsym.complete();
        }
        if (tsym.sourcefile != null) {
            Env compenv = env.enter.getEnv(tsym);
            if (compenv == null)
                return null;
            Scope s = compenv.toplevel.namedImportScope;
            for (Scope.Entry e = s.lookup(names.fromString(className));
                    e != null; e = e.shadowed) {
                if (e.sym != null && e.sym.kind == Symbol.TYP) {
                    ClassDoc c = env.getClassDoc((ClassSymbol) e.sym);
                    if (c.name().equals(className)) {
                        return c;
                    }
                }
            }
            s = compenv.toplevel.starImportScope;
            for (Scope.Entry e = s.lookup(names.fromString(className));
                    e != null; e = e.shadowed) {
                if (e.sym != null && e.sym.kind == Symbol.TYP) {
                    ClassDoc c = env.getClassDoc((ClassSymbol) e.sym);
                    if (c.name().equals(className)) {
                        return c;
                    }
                }
            }
        }
        return null;
    }

    private boolean hasParameterTypes(MethodSymbol method, String[] argTypes) {
        if (argTypes == null) {
            return true;
        }
        int i = 0;
        List types = method.type.argtypes();
        if (argTypes.length != types.length()) {
            return false;
        }
        for (; types.nonEmpty(); types = types.tail) {
            if (!hasTypeName((Type) types.head, argTypes[i])) {
                return false;
            }
            i++;
        }
        return true;
    }

    private static boolean hasTypeName(Type t, String name) {
        return name.equals(TypeMaker.getTypeName(t, true)) ||
                name.equals(TypeMaker.getTypeName(t, false));
    }

    /**
      * Find a method in this class scope.
      * Search order: this class, interfaces, superclasses, outerclasses.
      * Note that this is not necessarily what the compiler would do!
      *
      * @param methodName the unqualified name to search for.
      * @param paramTypeArray the array of Strings for method parameter types.
      * @return the first MethodDocImpl which matches, null if not found.
      */
    public MethodDocImpl findMethod(String methodName, String[] paramTypes) {
        return searchMethod(methodName, paramTypes, new HashSet());
    }

    private MethodDocImpl searchMethod(String methodName, String[] paramTypes,
            Set searched) {
        ClassDocImpl cdi;
        MethodDocImpl mdi;
        if (searched.contains(this)) {
            return null;
        }
        searched.add(this);
        Name.Table names = tsym.name.table;
        Scope.Entry e = tsym.members().lookup(names.fromString(methodName));
        if (paramTypes == null) {
            MethodSymbol lastFound = null;
            for (; e != null; e = e.shadowed) {
                if (e.sym != null && e.sym.kind == Kinds.MTH &&
                        e.sym.name != names.init) {
                    if (e.sym.name.toString().equals(methodName) &&
                            env.shouldDocument((MethodSymbol) e.sym)) {
                        lastFound = (MethodSymbol) e.sym;
                    }
                }
            }
            if (lastFound != null) {
                return env.getMethodDoc(lastFound);
            }
        } else {
            for (; e != null; e = e.shadowed) {
                if (e.sym != null && e.sym.kind == Kinds.MTH &&
                        e.sym.name != names.init) {
                    if (e.sym.name.toString().equals(methodName) &&
                            hasParameterTypes((MethodSymbol) e.sym, paramTypes) &&
                            env.shouldDocument((MethodSymbol) e.sym)) {
                        return env.getMethodDoc((MethodSymbol) e.sym);
                    }
                }
            }
        }
        cdi = (ClassDocImpl) superclass();
        if (cdi != null) {
            mdi = cdi.searchMethod(methodName, paramTypes, searched);
            if (mdi != null) {
                return mdi;
            }
        }
        ClassDoc[] intf = interfaces();
        for (int i = 0; i < intf.length; i++) {
            cdi = (ClassDocImpl) intf[i];
            mdi = cdi.searchMethod(methodName, paramTypes, searched);
            if (mdi != null) {
                return mdi;
            }
        }
        cdi = (ClassDocImpl) containingClass();
        if (cdi != null) {
            mdi = cdi.searchMethod(methodName, paramTypes, searched);
            if (mdi != null) {
                return mdi;
            }
        }
        return null;
    }

    /**
      * Find constructor in this class.
      *
      * @param constrName the unqualified name to search for.
      * @param paramTypeArray the array of Strings for constructor parameters.
      * @return the first ConstructorDocImpl which matches, null if not found.
      */
    public ConstructorDoc findConstructor(String constrName, String[] paramTypes) {
        Name.Table names = tsym.name.table;
        for (Scope.Entry e = tsym.members().lookup(names.fromString("<init>"));
                e != null; e = e.shadowed) {
            if (e.sym != null && e.sym.kind == Kinds.MTH &&
                    e.sym.name == names.init) {
                if (hasParameterTypes((MethodSymbol) e.sym, paramTypes) &&
                        env.shouldDocument((MethodSymbol) e.sym)) {
                    return env.getConstructorDoc((MethodSymbol) e.sym);
                }
            }
        }
        return null;
    }

    /**
      * Find a field in this class scope.
      * Search order: this class, outerclasses, interfaces,
      * superclasses. IMP: If see tag is defined in an inner class,
      * which extends a super class and if outerclass and the super
      * class have a visible field in common then Java compiler cribs
      * about the ambiguity, but the following code will search in the
      * above given search order.
      *
      * @param fieldName the unqualified name to search for.
      * @return the first FieldDocImpl which matches, null if not found.
      */
    public FieldDoc findField(String fieldName) {
        return searchField(fieldName, new HashSet());
    }

    private FieldDocImpl searchField(String fieldName, Set searched) {
        Name.Table names = tsym.name.table;
        if (searched.contains(this)) {
            return null;
        }
        searched.add(this);
        for (Scope.Entry e = tsym.members().lookup(names.fromString(fieldName));
                e != null; e = e.shadowed) {
            if (e.sym != null && e.sym.kind == Kinds.VAR) {
                if (e.sym.name.toString().equals(fieldName) &&
                        env.shouldDocument((VarSymbol) e.sym)) {
                    return env.getFieldDoc((VarSymbol) e.sym);
                }
            }
        }
        ClassDocImpl cdi = (ClassDocImpl) containingClass();
        if (cdi != null) {
            FieldDocImpl fdi = cdi.searchField(fieldName, searched);
            if (fdi != null) {
                return fdi;
            }
        }
        cdi = (ClassDocImpl) superclass();
        if (cdi != null) {
            FieldDocImpl fdi = cdi.searchField(fieldName, searched);
            if (fdi != null) {
                return fdi;
            }
        }
        ClassDoc[] intf = interfaces();
        for (int i = 0; i < intf.length; i++) {
            cdi = (ClassDocImpl) intf[i];
            FieldDocImpl fdi = cdi.searchField(fieldName, searched);
            if (fdi != null) {
                return fdi;
            }
        }
        return null;
    }

    /**
      * Get the list of classes declared as imported.
      * These are called "single-type-import declarations" in the JLS.
      *
      * @return an array of ClassDocImpl representing the imported classes.
      */
    public ClassDoc[] importedClasses() {
        if (tsym.sourcefile == null)
            return new ClassDoc[0];
        ListBuffer importedClasses = new ListBuffer();
        Env compenv = env.enter.getEnv(tsym);
        if (compenv == null)
            return new ClassDocImpl[0];
        Name asterisk = tsym.name.table.asterisk;
        for (List defs = compenv.toplevel.defs; defs.nonEmpty(); defs = defs.tail) {
            Tree t = (Tree) defs.head;
            if (t.tag == Tree.IMPORT) {
                Tree imp = ((Import) t).qualid;
                if (TreeInfo.name(imp) != asterisk) {
                    importedClasses.append(
                            env.getClassDoc((ClassSymbol) imp.type.tsym));
                }
            }
        }
        return (ClassDoc[]) importedClasses.toArray(
                new ClassDocImpl[importedClasses.length()]);
    }

    /**
      * Get the list of packages declared as imported.
      * These are called "type-import-on-demand declarations" in the JLS.
      *
      * @return an array of PackageDocImpl representing the imported packages.
      * ###NOTE: the syntax supports importing all inner classes from a class as well.
      */
    public PackageDoc[] importedPackages() {
        if (tsym.sourcefile == null)
            return new PackageDoc[0];
        ListBuffer importedPackages = new ListBuffer();
        Name.Table names = tsym.name.table;
        importedPackages.append(
                env.getPackageDoc(env.reader.enterPackage(names.java_lang)));
        Env compenv = env.enter.getEnv(tsym);
        if (compenv == null)
            return new PackageDocImpl[0];
        for (List defs = compenv.toplevel.defs; defs.nonEmpty(); defs = defs.tail) {
            Tree t = (Tree) defs.head;
            if (t.tag == Tree.IMPORT) {
                Tree imp = ((Import) t).qualid;
                if (TreeInfo.name(imp) == names.asterisk) {
                    Select sel = (Select) imp;
                    Symbol s = sel.selected.type.tsym;
                    PackageDocImpl pdoc = env.getPackageDoc(s.packge());
                    if (!importedPackages.contains(pdoc))
                        importedPackages.append(pdoc);
                }
            }
        }
        return (PackageDoc[]) importedPackages.toArray(
                new PackageDocImpl[importedPackages.length()]);
    }

    /**
      * Return the type's dimension information, as a string.
      * <p>
      * For example, a two dimensional array of String returns '[][]'.
      * <p>
      * In ClassDocImpl so that it implements TypeDoc interface, always
      * returns empty string.              List<Name> argnames,
      *
      */
    public String dimension() {
        return "";
    }

    /**
      * Return this type as a class.  Array dimensions are ignored.
      * <p>
      * In ClassDocImpl so that it implements TypeDoc interface, always
      * returns this.
      *
      * @return a ClassDocImpl if the type is a Class.
      * Return null if it is a primitive type..
      */
    public ClassDoc asClassDoc() {
        return this;
    }

    /**
      * Return true if this is a primitive Java type. (i.e. if this
      * is not a class type or array type)
      */
    public boolean isPrimitiveJavaType() {
        return false;
    }

    /**
      * Return true if this is an array type
      */
    public boolean isArrayType() {
        return false;
    }

    /**
      * Return true if this class implements <code>java.io.Serializable</code>.
      *
      * Since <code>java.io.Externalizable</code> extends
      * <code>java.io.Serializable</code>,
      * Externalizable objects are also Serializable.
      */
    public boolean isSerializable() {
        try {
            return type.isSubType(env.syms.serializableType);
        } catch (CompletionFailure ex) {
            return false;
        }
    }

    /**
      * Return true if this class implements
      * <code>java.io.Externalizable</code>.
      */
    public boolean isExternalizable() {
        try {
            return type.isSubType(env.externalizableSym.type);
        } catch (CompletionFailure ex) {
            return false;
        }
    }

    /**
      * Return the serialization methods for this class.
      *
      * @return an array of <code>MethodDocImpl</code> that represents
      * the serialization methods for this class.
      */
    public MethodDoc[] serializationMethods() {
        if (serializedForm == null) {
            serializedForm = new SerializedForm(env, tsym, this);
        }
        return serializedForm.methods();
    }

    /**
      * Return the Serializable fields of class.<p>
      *
      * Return either a list of default fields documented by
      * <code>serial</code> tag<br>
      * or return a single <code>FieldDoc</code> for
      * <code>serialPersistentField</code> member.
      * There should be a <code>serialField</code> tag for
      * each Serializable field defined by an <code>ObjectStreamField</code>
      * array component of <code>serialPersistentField</code>.
      *
      * @returns an array of <code>FieldDoc</code> for the Serializable fields
      * of this class.
      *
      * @see #definesSerializableFields()
      * @see SerialFieldTagImpl
      */
    public FieldDoc[] serializableFields() {
        if (serializedForm == null) {
            serializedForm = new SerializedForm(env, tsym, this);
        }
        return serializedForm.fields();
    }

    /**
      * Return true if Serializable fields are explicitly defined with
      * the special class member <code>serialPersistentFields</code>.
      *
      * @see #serializableFields()
      * @see SerialFieldTagImpl
      */
    public boolean definesSerializableFields() {
        if (!isSerializable() || isExternalizable()) {
            return false;
        } else {
            if (serializedForm == null) {
                serializedForm = new SerializedForm(env, tsym, this);
            }
            return serializedForm.definesSerializableFields();
        }
    }

    /**
      * Determine if a class is a RuntimeException.
      * <p>
      * Used only by ThrowsTagImpl.
      */
    boolean isRuntimeException() {
        return tsym.isSubClass(env.syms.runtimeExceptionType.tsym);
    }

    /**
      * Return the source position of the entity, or null if
      * no position is available.
      */
    public SourcePosition position() {
        if (tsym.sourcefile == null)
            return null;
        return SourcePositionImpl.make(tsym.sourcefile.toString(),
                (tree == null) ? Position.NOPOS : tree.pos);
    }
}

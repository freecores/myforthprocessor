/**
 * @(#)DocEnv.java	1.30 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.tools.javadoc;
import com.sun.javadoc.*;

import java.lang.reflect.Modifier;

import com.sun.tools.javac.v8.code.Flags;

import com.sun.tools.javac.v8.code.Scope;

import com.sun.tools.javac.v8.code.Symbol.ClassSymbol;

import com.sun.tools.javac.v8.code.Symbol.CompletionFailure;

import com.sun.tools.javac.v8.code.Symbol.MethodSymbol;

import com.sun.tools.javac.v8.code.Symbol.PackageSymbol;

import com.sun.tools.javac.v8.code.Symbol.VarSymbol;

import com.sun.tools.javac.v8.code.Symbol;

import com.sun.tools.javac.v8.code.Symtab;

import com.sun.tools.javac.v8.code.Type.ClassType;

import com.sun.tools.javac.v8.comp.Attr;

import com.sun.tools.javac.v8.comp.Check;

import com.sun.tools.javac.v8.tree.Tree.ClassDef;

import com.sun.tools.javac.v8.tree.Tree.MethodDef;

import com.sun.tools.javac.v8.tree.Tree.VarDef;

import com.sun.tools.javac.v8.tree.Tree;

import com.sun.tools.javac.v8.util.Context;

import com.sun.tools.javac.v8.util.Hashtable;

import com.sun.tools.javac.v8.util.List;

import com.sun.tools.javac.v8.util.Name;

import com.sun.tools.javac.v8.util.Position;


/**
 * Holds the environment for a run of javadoc.
 * Holds only the information needed throughout the
 * run and not the compiler info that could be GC'ed
 * or ported.
 *
 * @since JDK1.4
 * @author Robert Field
 * @author Neal Gafter (rewrite)
 */
public class DocEnv {
    private static final Context.Key docEnvKey = new Context.Key();

    public static DocEnv instance(Context context) {
        DocEnv instance = (DocEnv) context.get(docEnvKey);
        if (instance == null)
            instance = new DocEnv(context);
        return instance;
    }
    Messager messager;
    DocLocale doclocale;

    /**
     * Predefined symbols known to the compiler.
     */
    Symtab syms;

    /**
     * Referenced directly in RootDocImpl.
     */
    JavadocClassReader reader;

    /**
     * The compiler's attribution phase (needed to evaluate
     *  constant initializers).
     */
    Attr attr;

    /**
     * Javadoc's own version of the compiler's enter phase.
     */
    JavadocEnter enter;

    /**
     * The name table.
     */
    Name.Table names;

    /**
     * The encoding name.
     */
    private String encoding;
    final Symbol externalizableSym;

    /**
     * Access filter (public, protected, ...).
     */
    ModifierFilter showAccess;
    private ClassDocImpl runtimeException;

    /**
     * True if we are using a sentence BreakIterator.
     */
    boolean breakiterator;
    Check chk;

    /**
     * Allow documenting from class files?
     */
    boolean docClasses = false;

    /**
     * Constructor
     *
     * @param context      Context for this javadoc instance.
     * @param showAccess   Access modifier filter for classes to document.
     * @param breakiterator True if we are using a sentence BreakIterator.
     */
    private DocEnv(Context context) {
        super();
        context.put(docEnvKey, this);
        messager = Messager.instance0(context);
        syms = Symtab.instance(context);
        reader = JavadocClassReader.instance0(context);
        enter = JavadocEnter.instance0(context);
        attr = Attr.instance(context);
        names = Name.Table.instance(context);
        externalizableSym = reader.enterClass(names.fromString("java.io.Externalizable"));
        chk = Check.instance(context);
        this.doclocale = new DocLocale(this, "", breakiterator);
    }

    /**
      *
      * Look up ClassDoc by qualified name.
      */
    public ClassDocImpl lookupClass(String name) {
        ClassSymbol c = getClassSymbol(name);
        if (c != null) {
            return getClassDoc(c);
        } else {
            return null;
        }
    }

    /**
      *
      * Load ClassDoc by qualified name.
      */
    public ClassDocImpl loadClass(String name) {
        try {
            ClassSymbol c = reader.loadClass(names.fromString(name));
            return getClassDoc(c);
        } catch (CompletionFailure ex) {
            chk.completionError(Position.NOPOS, ex);
            return null;
        }
    }

    /**
      * Look up PackageDoc by qualified name.
      */
    public PackageDocImpl lookupPackage(String name) {
        PackageSymbol p = (PackageSymbol) syms.packages.get(names.fromString(name));
        ClassSymbol c = getClassSymbol(name);
        if (p != null && c == null) {
            return getPackageDoc(p);
        } else {
            return null;
        }
    }

    /**
      * Retrieve class symbol by fully-qualified name.
      */
    ClassSymbol getClassSymbol(String name) {
        int nameLen = name.length();
        char[] nameChars = name.toCharArray();
        int idx = name.length();
        for (; ;) {
            ClassSymbol s = (ClassSymbol) syms.classes.get(
                    names.fromChars(nameChars, 0, nameLen));
            if (s != null)
                return s;
            idx = name.substring(0, idx).lastIndexOf('.');
            if (idx < 0)
                break;
            nameChars[idx] = '$';
        }
        return null;
    }

    /**
      * Set the locale.
      */
    public void setLocale(String localeName) {
        doclocale = new DocLocale(this, localeName, breakiterator);
        messager.reset();
    }

    /**
      * Check whether this member should be documented.
      */
    public boolean shouldDocument(VarSymbol sym) {
        long mod = sym.flags();
        if ((mod & Flags.SYNTHETIC) != 0) {
            return false;
        }
        return showAccess.checkModifier(translateModifiers(mod));
    }

    /**
      * Check whether this member should be documented.
      */
    public boolean shouldDocument(MethodSymbol sym) {
        long mod = sym.flags();
        if ((mod & Flags.SYNTHETIC) != 0) {
            return false;
        }
        return showAccess.checkModifier(translateModifiers(mod));
    }

    /**
      * check whether this class should be documented.
      */
    public boolean shouldDocument(ClassSymbol sym) {
        return (sym.flags_field & Flags.SYNTHETIC) == 0 &&
                (docClasses || getClassDoc(sym).tree != null) && isVisible(sym);
    }

    /**
      * Check the visibility if this is an nested class.
      * if this is not a nested class, return true.
      * if this is an static visible nested class,
      *    return true.
      * if this is an visible nested class
      *    if the outer class is visible return true.
      *    else return false.
      * IMPORTANT: This also allows, static nested classes
      * to be defined inside an nested class, which is not
      * allowed by the compiler. So such an test case will
      * not reach upto this method itself, but if compiler
      * allows it, then that will go through.
      */
    protected boolean isVisible(ClassSymbol sym) {
        long mod = sym.flags_field;
        if (!showAccess.checkModifier(translateModifiers(mod))) {
            return false;
        }
        ClassSymbol encl = sym.owner.enclClass();
        return (encl == null || (mod & Flags.STATIC) != 0 || isVisible(encl));
    }

    /**
      * Print error message, increment error count.
      *
      * @param key selects message from resource
      */
    public void error(DocImpl doc, String key) {
        messager.error(doc == null ? null : doc.position(), key);
    }

    /**
      * Print error message, increment error count.
      *
      * @param key selects message from resource
      * @param a1 first argument
      */
    public void error(DocImpl doc, String key, String a1) {
        messager.error(doc == null ? null : doc.position(), key, a1);
    }

    /**
      * Print error message, increment error count.
      *
      * @param key selects message from resource
      * @param a1 first argument
      * @param a2 second argument
      */
    public void error(DocImpl doc, String key, String a1, String a2) {
        messager.error(doc == null ? null : doc.position(), key, a1, a2);
    }

    /**
      * Print error message, increment error count.
      *
      * @param key selects message from resource
      * @param a1 first argument
      * @param a2 second argument
      * @param a3 third argument
      */
    public void error(DocImpl doc, String key, String a1, String a2, String a3) {
        messager.error(doc == null ? null : doc.position(), key, a1, a2, a3);
    }

    /**
      * Print warning message, increment warning count.
      *
      * @param key selects message from resource
      */
    public void warning(DocImpl doc, String key) {
        messager.warning(doc == null ? null : doc.position(), key);
    }

    /**
      * Print warning message, increment warning count.
      *
      * @param key selects message from resource
      * @param a1 first argument
      */
    public void warning(DocImpl doc, String key, String a1) {
        messager.warning(doc == null ? null : doc.position(), key, a1);
    }

    /**
      * Print warning message, increment warning count.
      *
      * @param key selects message from resource
      * @param a1 first argument
      * @param a2 second argument
      */
    public void warning(DocImpl doc, String key, String a1, String a2) {
        messager.warning(doc == null ? null : doc.position(), key, a1, a2);
    }

    /**
      * Print warning message, increment warning count.
      *
      * @param key selects message from resource
      * @param a1 first argument
      * @param a2 second argument
      * @param a3 third argument
      */
    public void warning(DocImpl doc, String key, String a1, String a2, String a3) {
        messager.warning(doc == null ? null : doc.position(), key, a1, a2, a3);
    }

    /**
      * Print warning message, increment warning count.
      *
      * @param key selects message from resource
      * @param a1 first argument
      * @param a2 second argument
      * @param a3 third argument
      */
    public void warning(DocImpl doc, String key, String a1, String a2, String a3,
            String a4) {
        messager.warning(doc == null ? null : doc.position(), key, a1, a2, a3, a4);
    }

    /**
      * Print a message.
      *
      * @param key selects message from resource
      */
    public void notice(String key) {
        messager.notice(key);
    }

    /**
      * Print a message.
      *
      * @param key selects message from resource
      * @param a1 first argument
      */
    public void notice(String key, String a1) {
        messager.notice(key, a1);
    }

    /**
      * Print a message.
      *
      * @param key selects message from resource
      * @param a1 first argument
      * @param a2 second argument
      */
    public void notice(String key, String a1, String a2) {
        messager.notice(key, a1, a2);
    }

    /**
      * Print a message.
      *
      * @param key selects message from resource
      * @param a1 first argument
      * @param a2 second argument
      * @param a3 third argument
      */
    public void notice(String key, String a1, String a2, String a3) {
        messager.notice(key, a1, a2, a3);
    }

    /**
      * Exit, reporting errors and warnings.
      */
    public void exit() {
        messager.exit();
    }
    private Hashtable packageMap = new Hashtable();

    /**
     * Return the PackageDoc of this package symbol.
     */
    public PackageDocImpl getPackageDoc(PackageSymbol pack) {
        PackageDocImpl result = (PackageDocImpl) packageMap.get(pack);
        if (result != null)
            return result;
        result = new PackageDocImpl(this, pack);
        packageMap.put(pack, result);
        return result;
    }
    private Hashtable classMap = new Hashtable();

    /**
     * Return the ClassDoc of this class symbol.
     */
    ClassDocImpl getClassDoc(ClassSymbol clazz) {
        ClassDocImpl result = (ClassDocImpl) classMap.get(clazz);
        if (result != null)
            return result;
        result = new ClassDocImpl(this, clazz);
        classMap.put(clazz, result);
        return result;
    }

    /**
      * Return the ClassDoc of this class type.
      */
    ClassDocImpl getClassDoc(ClassType clazzt) {
        return getClassDoc((ClassSymbol) clazzt.tsym);
    }

    /**
      * Create the ClassDoc for a class symbol.
      */
    void makeClassDoc(ClassSymbol clazz, String docComment, ClassDef tree) {
        ClassDocImpl result = (ClassDocImpl) classMap.get(clazz);
        if (result != null) {
            if (docComment != null)
                result.setRawCommentText(docComment);
        } else {
            result = new ClassDocImpl(this, clazz, docComment, tree);
            classMap.put(clazz, result);
        }
        if (tree != null)
            result.tree = tree;
    }
    private Hashtable fieldMap = new Hashtable();

    /**
     * Return the FieldDoc of this var symbol.
     */
    FieldDocImpl getFieldDoc(VarSymbol var) {
        FieldDocImpl result = (FieldDocImpl) fieldMap.get(var);
        if (result != null)
            return result;
        result = new FieldDocImpl(this, var);
        fieldMap.put(var, result);
        return result;
    }

    /**
      * Create a FieldDoc for a var symbol.
      */
    void makeFieldDoc(VarSymbol var, String docComment, VarDef tree) {
        FieldDocImpl result = (FieldDocImpl) fieldMap.get(var);
        if (result != null) {
            if (docComment != null)
                result.setRawCommentText(docComment);
        } else {
            result = new FieldDocImpl(this, var, docComment);
            fieldMap.put(var, result);
        }
        if (tree != null)
            result.tree = tree;
    }
    private Hashtable methodMap = new Hashtable();

    private ExecutableMemberDocImpl getExecutableMemberDoc(MethodSymbol meth) {
        ExecutableMemberDocImpl result =
                (ExecutableMemberDocImpl) methodMap.get(meth);
        return result;
    }

    /**
      * Create a MethodDoc for this MethodSymbol.
      * Should be called only on symbols representing methods.
      */
    void makeMethodDoc(MethodSymbol meth, String docComment, List argNames,
            MethodDef tree) {
        ExecutableMemberDocImpl r = getExecutableMemberDoc(meth);
        if (r != null) {
            if (docComment != null)
                r.setRawCommentText(docComment);
            if (argNames != null)
                r.setArgNames(argNames);
            if (tree != null)
                r.tree = tree;
        } else {
            MethodDocImpl result =
                    new MethodDocImpl(this, meth, docComment, argNames);
            methodMap.put(meth, result);
            if (tree != null)
                result.tree = tree;
        }
    }

    /**
      * Return the MethodDoc for a MethodSymbol.
      * Should be called only on symbols representing methods.
      */
    public MethodDocImpl getMethodDoc(MethodSymbol meth) {
        ExecutableMemberDocImpl r = getExecutableMemberDoc(meth);
        if (r != null)
            return (MethodDocImpl) r;
        MethodDocImpl result = new MethodDocImpl(this, meth);
        methodMap.put(meth, result);
        return result;
    }

    /**
      * Create the ConstructorDoc for a MethodSymbol.
      * Should be called only on symbols representing constructors.
      */
    void makeConstructorDoc(MethodSymbol meth, String docComment, List argNames,
            MethodDef tree) {
        ExecutableMemberDocImpl r = getExecutableMemberDoc(meth);
        if (r != null) {
            if (docComment != null)
                r.setRawCommentText(docComment);
            if (argNames != null)
                r.setArgNames(argNames);
            if (tree != null)
                r.tree = tree;
        } else {
            ConstructorDocImpl result =
                    new ConstructorDocImpl(this, meth, docComment, argNames);
            methodMap.put(meth, result);
            if (tree != null)
                result.tree = tree;
        }
    }

    /**
      * Return the ConstructorDoc for a MethodSymbol.
      * Should be called only on symbols representing constructors.
      */
    public ConstructorDocImpl getConstructorDoc(MethodSymbol meth) {
        ExecutableMemberDocImpl r = getExecutableMemberDoc(meth);
        if (r != null)
            return (ConstructorDocImpl) r;
        ConstructorDocImpl result = new ConstructorDocImpl(this, meth);
        methodMap.put(meth, result);
        return result;
    }

    /**
      * Set the encoding.
      */
    public void setEncoding(String encoding) {
        this.encoding = encoding;
    }

    /**
      * Get the encoding.
      */
    public String getEncoding() {
        return encoding;
    }

    /**
      * Convert modifier bits from private coding used by
      * the compiler to that of java.lang.reflect.Modifier.
      */
    static int translateModifiers(long flags) {
        int result = 0;
        if ((flags & Flags.ABSTRACT) != 0)
            result |= Modifier.ABSTRACT;
        if ((flags & Flags.FINAL) != 0)
            result |= Modifier.FINAL;
        if ((flags & Flags.INTERFACE) != 0)
            result |= Modifier.INTERFACE;
        if ((flags & Flags.NATIVE) != 0)
            result |= Modifier.NATIVE;
        if ((flags & Flags.PRIVATE) != 0)
            result |= Modifier.PRIVATE;
        if ((flags & Flags.PROTECTED) != 0)
            result |= Modifier.PROTECTED;
        if ((flags & Flags.PUBLIC) != 0)
            result |= Modifier.PUBLIC;
        if ((flags & Flags.STATIC) != 0)
            result |= Modifier.STATIC;
        if ((flags & Flags.SYNCHRONIZED) != 0)
            result |= Modifier.SYNCHRONIZED;
        if ((flags & Flags.TRANSIENT) != 0)
            result |= Modifier.TRANSIENT;
        if ((flags & Flags.VOLATILE) != 0)
            result |= Modifier.VOLATILE;
        return result;
    }
}

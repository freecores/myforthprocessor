/**
 * @(#)JavadocTool.java	1.26 03/04/08
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.tools.javadoc;
import java.io.*;

import com.sun.tools.javac.v8.code.ClassReader;

import com.sun.tools.javac.v8.code.ClassWriter;

import com.sun.tools.javac.v8.code.Kinds;

import com.sun.tools.javac.v8.code.Symbol.ClassSymbol;

import com.sun.tools.javac.v8.code.Symbol.CompletionFailure;

import com.sun.tools.javac.v8.code.Symbol.MethodSymbol;

import com.sun.tools.javac.v8.code.Symbol.VarSymbol;

import com.sun.tools.javac.v8.code.Symbol;

import com.sun.tools.javac.v8.code.Symtab;

import com.sun.tools.javac.v8.comp.Check;

import com.sun.tools.javac.v8.comp.Enter;

import com.sun.tools.javac.v8.comp.Env;

import com.sun.tools.javac.v8.comp.Resolve;

import com.sun.tools.javac.v8.tree.Tree.ClassDef;

import com.sun.tools.javac.v8.tree.Tree.MethodDef;

import com.sun.tools.javac.v8.tree.Tree.TopLevel;

import com.sun.tools.javac.v8.tree.Tree.VarDef;

import com.sun.tools.javac.v8.tree.Tree;

import com.sun.tools.javac.v8.tree.TreeMaker;

import com.sun.tools.javac.v8.util.Abort;

import com.sun.tools.javac.v8.util.Context;

import com.sun.tools.javac.v8.util.Hashtable;

import com.sun.tools.javac.v8.util.List;

import com.sun.tools.javac.v8.util.ListBuffer;

import com.sun.tools.javac.v8.util.Log;

import com.sun.tools.javac.v8.util.Name;

import com.sun.tools.javac.v8.util.Position;


/**
 *  This class could be the main entry point for Javadoc when Javadoc is used as a
 *  component in a larger software system. It provides operations to
 *  construct a new javadoc processor, and to run it on a set of source
 *  files.
 *  @author Neal Gafter
 */
public class JavadocTool extends com.sun.tools.javac.v8.JavaCompiler {
    DocEnv docenv;
    private final Context context;
    private final Messager messager;
    private final JavadocClassReader reader;
    private final JavadocEnter enter;

    /**
     * Construct a new JavaCompiler processor from a log, a symbol table, and an options table
     */
    private JavadocTool(Context context) {
        super(context);
        this.context = context;
        messager = Messager.instance0(context);
        reader = JavadocClassReader.instance0(context);
        enter = JavadocEnter.instance0(context);
    }

    /**
      * For javadoc, the parser needs to keep comments. Overrides method from JavaCompiler.
      */
    protected boolean keepComments() {
        return true;
    }

    /**
      *  Construct a new javadoc tool from a log and an options table.
      *  This will create a new symbol table module.
      *  Note we use an extended ClassReader that scans for package.html files.
      */
    public static JavadocTool make0(Context context) {
        Messager messager = null;
        try {
            messager = Messager.instance0(context);
            JavadocClassReader reader = JavadocClassReader.instance0(context);
            JavadocEnter enter = JavadocEnter.instance0(context);
            return new JavadocTool(context);
        } catch (CompletionFailure ex) {
            messager.error(Position.NOPOS, ex.getMessage());
            return null;
        }
    }

    public RootDocImpl getRootDocImpl(String doclocale, String encoding,
            ModifierFilter filter, List javaNames, List options,
            boolean breakiterator, List subPackages, List excludedPackages,
            boolean docClasses) {
        docenv = DocEnv.instance(context);
        docenv.showAccess = filter;
        docenv.breakiterator = breakiterator;
        docenv.setLocale(doclocale);
        docenv.setEncoding(encoding);
        docenv.docClasses = docClasses;
        reader.sourceCompleter = docClasses ? null : this;
        ListBuffer names = new ListBuffer();
        ListBuffer classTrees = new ListBuffer();
        ListBuffer packTrees = new ListBuffer();
        try {
            for (List it = javaNames; it.nonEmpty(); it = it.tail) {
                String name = (String) it.head;
                if (!docClasses && name.endsWith(".java") &&
                        new File(name).exists()) {
                    messager.notice("main.Loading_source_file", name);
                    Tree tree = parse(name);
                    classTrees.append(tree);
                } else if (isValidPackageName(name)) {
                    names = names.append(name);
                } else if (name.endsWith(".java")) {
                    messager.error(null, "main.file_not_found", name);
                    ;
                } else {
                    messager.error(null, "main.illegal_package_name", name);
                }
            }
            if (!docClasses) {
                searchSubPackages(subPackages, names, excludedPackages);
                for (List packs = names.toList(); packs.nonEmpty();
                        packs = packs.tail) {
                    parsePackageClasses((String) packs.head, packTrees,
                            excludedPackages);
                }
                if (messager.nerrors() != 0)
                    return null;
                messager.notice("main.Building_tree");
                enter.main(classTrees.toList().appendList(packTrees.toList()));
            }
        } catch (Abort ex) {
        }
        if (messager.nerrors() != 0)
            return null;
        if (docClasses)
            return new RootDocImpl(docenv, javaNames, options);
        else
            return new RootDocImpl(docenv, listClasses(classTrees.toList()),
                    names.toList(), options);
    }

    /**
      * Is the given string a valid package name?
      */
    boolean isValidPackageName(String s) {
        int index;
        while ((index = s.indexOf('.')) != -1) {
            if (!isValidClassName(s.substring(0, index)))
                return false;
            s = s.substring(index + 1);
        }
        return isValidClassName(s);
    }
    private static final char pathSep = File.pathSeparatorChar;

    /**
     * search all directories in path for subdirectory name. Add all
     * .java files found in such a directory to args.
     */
    private void parsePackageClasses(String name, ListBuffer trees,
            List excludedPackages) {
        if (excludedPackages.contains(name)) {
            return;
        }
        boolean hasFiles = false;
        String path = reader.sourceClassPath;
        if (path == null)
            path = reader.classPath;
        int plen = path.length();
        int i = 0;
        messager.notice("main.Loading_source_files_for_package", name);
        name = name.replace('.', File.separatorChar);
        while (i < plen) {
            int end = path.indexOf(pathSep, i);
            String pathname = path.substring(i, end);
            File f = new File(pathname, name);
            String[] names = f.list();
            if (names != null) {
                String dir = f.getAbsolutePath();
                if (!dir.endsWith(File.separator))
                    dir = dir + File.separator;
                for (int j = 0; j < names.length; j++) {
                    if (isValidJavaSourceFile(names[j])) {
                        String fn = dir + names[j];
                        trees.append(parse(fn));
                        hasFiles = true;
                    }
                }
            }
            i = end + 1;
        }
        if (!hasFiles)
            messager.warning(null, "main.no_source_files_for_package",
                    name.replace(File.separatorChar, '.'));
    }

    /**
      * Recursively search all directories in path for subdirectory name.
      * Add all packages found in such a directory to packages list.
      */
    private void searchSubPackages(List subPackages, ListBuffer packages,
            List excludedPackages) {
        for (List subpacks = subPackages; subpacks.nonEmpty();
                subpacks = subpacks.tail) {
            searchSubPackages((String) subpacks.head, packages, excludedPackages);
        }
    }

    /**
      * Recursively search all directories in path for subdirectory name.
      * Add all packages found in such a directory to packages list.
      */
    private void searchSubPackages(String name, ListBuffer packages,
            List excludedPackages) {
        if (excludedPackages.contains(name)) {
            return;
        }
        String path = reader.sourceClassPath + pathSep + reader.classPath;
        int plen = path.length();
        int i = 0;
        String packageName = name.replace('.', File.separatorChar);
        boolean addedPackage = false;
        while (i < plen) {
            int end = path.indexOf(pathSep, i);
            String pathname = path.substring(i, end);
            File f = new File(pathname, packageName);
            String[] names = f.list();
            if (names != null) {
                for (int j = 0; j < names.length; j++) {
                    if (!addedPackage && (isValidJavaSourceFile(names[j]) ||
                            isValidJavaClassFile(names[j])) &&
                            !packages.contains(name)) {
                        packages.append(name);
                        addedPackage = true;
                    } else if ((new File(f.getPath(), names[j])).isDirectory()) {
                        searchSubPackages(name + "." + names[j], packages,
                                excludedPackages);
                    }
                }
            }
            i = end + 1;
        }
    }

    /**
      * Return true if given file name is a valid class file name.
      * @param file the name of the file to check.
      * @return true if given file name is a valid class file name
      * and false otherwise.
      */
    private static boolean isValidJavaClassFile(String file) {
        if (!file.endsWith(".class"))
            return false;
        String clazzName = file.substring(0, file.length() - ".class".length());
        return isValidClassName(clazzName);
    }

    /**
      * Return true if given file name is a valid Java source file name.
      * @param file the name of the file to check.
      * @return true if given file name is a valid Java source file name
      * and false otherwise.
      */
    private static boolean isValidJavaSourceFile(String file) {
        if (!file.endsWith(".java"))
            return false;
        String clazzName = file.substring(0, file.length() - ".java".length());
        return isValidClassName(clazzName);
    }

    /**
      * Return true if given file name is a valid class name.
      * @param clazzname the name of the class to check.
      * @return true if given class name is a valid class name
      * and false otherwise.
      */
    private static boolean isValidClassName(String clazzName) {
        if (clazzName.length() < 1)
            return false;
        if (!Character.isJavaIdentifierStart(clazzName.charAt(0)))
            return false;
        for (int i = 1; i < clazzName.length(); i++)
            if (!Character.isJavaIdentifierPart(clazzName.charAt(i)))
                return false;
        return true;
    }

    /**
      * From a list of top level trees, return the list of contained class definitions
      */
    List listClasses(List trees) {
        ListBuffer result = new ListBuffer();
        for (List i = trees; i.nonEmpty(); i = i.tail) {
            Tree t = (Tree) i.head;
            if (t.tag == Tree.TOPLEVEL) {
                for (List j = ((TopLevel) t).defs; j.nonEmpty(); j = j.tail) {
                    Tree def = (Tree) j.head;
                    if (def.tag == Tree.CLASSDEF)
                        result.append((ClassDef) def);
                }
            }
        }
        return result.toList();
    }
}

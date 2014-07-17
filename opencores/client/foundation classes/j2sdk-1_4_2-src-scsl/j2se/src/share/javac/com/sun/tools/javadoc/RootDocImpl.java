/**
 * @(#)RootDocImpl.java	1.45 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.tools.javadoc;
import java.io.IOException;

import java.io.FileInputStream;

import java.io.File;

import com.sun.javadoc.*;

import com.sun.tools.javac.v8.tree.Tree;

import com.sun.tools.javac.v8.tree.Tree.ClassDef;

import com.sun.tools.javac.v8.code.Symbol;

import com.sun.tools.javac.v8.util.List;

import com.sun.tools.javac.v8.util.ListBuffer;

import com.sun.tools.javac.v8.util.Position;


/**
 * This class holds the information from one run of javadoc.
 * Particularly the packages, classes and options specified
 * by the user..
 *
 * @since JDK1.2
 * @author Robert Field
 * @author Atul M Dambalkar
 * @author Neal Gafter (rewrite)
 */
public class RootDocImpl extends DocImpl implements RootDoc {

    /**
     * list of classes specified on the command line.
     */
    private List cmdLineClasses;

    /**
     * list of packages specified on the command line.
     */
    private List cmdLinePackages;

    /**
     * a collection of all options.
     */
    private List options;

    /**
     * Constructor
     *
     * @param env the documentation environment, state for this javadoc run
     * @param classes list of classes (ClassSymbol) specified on the commandline
     * @param packages list of package names specified on the commandline
     * @param options list of options
     */
    public RootDocImpl(DocEnv env, List classes, List packages, List options) {
        super(env, null);
        this.options = options;
        setPackages(env, packages);
        setClasses(env, classes);
    }

    /**
      * Constructor
      *
      * @param env the documentation environment, state for this javadoc run
      * @param classes list of class names specified on the commandline
      * @param options list of options
      */
    public RootDocImpl(DocEnv env, List classes, List options) {
        super(env, null);
        this.options = options;
        cmdLinePackages = List.make();
        ListBuffer classList = new ListBuffer();
        for (List l = classes; l.nonEmpty(); l = l.tail) {
            String className = (String) l.head;
            ClassDocImpl c = env.loadClass(className);
            if (c == null)
                env.error(null, "javadoc.class_not_found", className);
            else
                classList = classList.append(c);
        }
        cmdLineClasses = classList.toList();
    }

    /**
      * Initialize classes information. Those classes are input from
      * command line.
      *
      * @param env the compilation environment
      * @param classes a list of ClassDeclaration
      */
    private void setClasses(DocEnv env, List classes) {
        ListBuffer result = new ListBuffer();
        for (List i = classes; i.nonEmpty(); i = i.tail) {
            ClassDef def = (ClassDef) i.head;
            if (env.shouldDocument(def.sym)) {
                ClassDocImpl cd = env.getClassDoc(def.sym);
                if (cd != null) {
                    cd.isIncluded = true;
                    result.append(cd);
                }
            }
        }
        cmdLineClasses = result.toList();
    }

    /**
      * Initialize packages information.
      *
      * @param env the compilation environment
      * @param packages a list of package names (String)
      */
    private void setPackages(DocEnv env, List packages) {
        ListBuffer packlist = new ListBuffer();
        for (List i = packages; i.nonEmpty(); i = i.tail) {
            String name = (String) i.head;
            PackageDocImpl pkg = env.lookupPackage(name);
            if (pkg != null) {
                pkg.isIncluded = true;
                packlist.append(pkg);
            } else {
                env.warning(null, "main.no_source_files_for_package", name);
            }
        }
        cmdLinePackages = packlist.toList();
    }

    /**
      * Command line options.
      *
      * <pre>
      * For example, given:
      *     javadoc -foo this that -bar other ...
      *
      * This method will return:
      *      options()[0][0] = "-foo"
      *      options()[0][1] = "this"
      *      options()[0][2] = "that"
      *      options()[1][0] = "-bar"
      *      options()[1][1] = "other"
      * </pre>
      *
      * @return an array of arrays of String.
      */
    public String[][] options() {
        return (String[][]) options.toArray(new String[options.length()][]);
    }

    /**
      * Packages specified on the command line.
      */
    public PackageDoc[] specifiedPackages() {
        return (PackageDoc[]) cmdLinePackages.toArray(
                new PackageDocImpl[cmdLinePackages.length()]);
    }

    /**
      * Classes and interfaces specified on the command line.
      */
    public ClassDoc[] specifiedClasses() {
        ListBuffer classesToDocument = new ListBuffer();
        for (List it = cmdLineClasses; it.nonEmpty(); it = it.tail) {
            ((ClassDocImpl) it.head).addAllClasses(classesToDocument, true);
        }
        return (ClassDoc[]) classesToDocument.toArray(
                new ClassDocImpl[classesToDocument.length()]);
    }

    /**
      * Return all classes and interfaces (including those inside
      * packages) to be documented.
      */
    public ClassDoc[] classes() {
        ListBuffer classesToDocument = new ListBuffer();
        for (List it = cmdLineClasses; it.nonEmpty(); it = it.tail) {
            ((ClassDocImpl) it.head).addAllClasses(classesToDocument, true);
        }
        for (List it = cmdLinePackages; it.nonEmpty(); it = it.tail) {
            ((PackageDocImpl) it.head).addAllClassesTo(classesToDocument);
        }
        return (ClassDoc[]) classesToDocument.toArray(
                new ClassDocImpl[classesToDocument.length()]);
    }

    /**
      * Return a ClassDoc for the specified class/interface name
      *
      * @param qualifiedName qualified class name
      *                        (i.e. includes package name).
      *
      * @return a ClassDocImpl holding the specified class, null if
      * this class is not referenced.
      */
    public ClassDoc classNamed(String qualifiedName) {
        return env.lookupClass(qualifiedName);
    }

    /**
      * Return a PackageDoc for the specified package name
      *
      * @param name package name
      *
      * @return a PackageDoc holding the specified package, null if
      * this package is not referenced.
      */
    public PackageDoc packageNamed(String name) {
        return env.lookupPackage(name);
    }

    /**
      * Return the name of this Doc item.
      *
      * @return the string <code>"*RootDocImpl*"</code>.
      */
    public String name() {
        return "*RootDocImpl*";
    }

    /**
      * Return the name of this Doc item.
      *
      * @return the string <code>"*RootDocImpl*"</code>.
      */
    public String qualifiedName() {
        return "*RootDocImpl*";
    }

    /**
      * Return true if this Doc is include in the active set.
      * RootDocImpl isn't even a program entity so it is always false.
      */
    public boolean isIncluded() {
        return false;
    }

    /**
      * Print error message, increment error count.
      *
      * @param msg message to print
      */
    public void printError(String msg) {
        env.messager.printError(msg);
    }

    /**
      * Print error message, increment error count.
      *
      * @param msg message to print
      */
    public void printError(SourcePosition pos, String msg) {
        env.messager.printError(pos, msg);
    }

    /**
      * Print warning message, increment warning count.
      *
      * @param msg message to print
      */
    public void printWarning(String msg) {
        env.messager.printWarning(msg);
    }

    /**
      * Print warning message, increment warning count.
      *
      * @param msg message to print
      */
    public void printWarning(SourcePosition pos, String msg) {
        env.messager.printWarning(pos, msg);
    }

    /**
      * Print a message.
      *
      * @param msg message to print
      */
    public void printNotice(String msg) {
        env.messager.printNotice(msg);
    }

    /**
      * Print a message.
      *
      * @param msg message to print
      */
    public void printNotice(SourcePosition pos, String msg) {
        env.messager.printNotice(pos, msg);
    }

    /**
      * Return the path of the overview file and null if it does not exist.
      * @return the path of the overview file and null if it does not exist.
      */
    private String getOverviewPath() {
        for (List i = options; i.nonEmpty(); i = i.tail) {
            String[] one = (String[]) i.head;
            if (one[0].equals("-overview")) {
                return one[1];
            }
        }
        return null;
    }

    /**
      * Do lazy initialization of "documentation" string.
      */
    protected String documentation() {
        if (documentation == null) {
            int cnt = options.length();
            String overviewPath = getOverviewPath();
            if (overviewPath == null) {
                documentation = "";
            } else {
                try {
                    documentation = readHTMLDocumentation(
                            new FileInputStream(overviewPath), overviewPath);
                } catch (IOException exc) {
                    documentation = "";
                    env.error(null, "javadoc.File_Read_Error", overviewPath);
                }
            }
        }
        return documentation;
    }

    /**
      * Return the source position of the entity, or null if
      * no position is available.
      */
    public SourcePosition position() {
        String path;
        return ((path = getOverviewPath()) == null) ? null :
                SourcePositionImpl.make(path, Position.NOPOS);
    }
}

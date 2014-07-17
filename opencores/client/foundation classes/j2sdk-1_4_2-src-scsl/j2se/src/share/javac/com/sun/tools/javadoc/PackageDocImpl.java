/**
 * @(#)PackageDocImpl.java	1.39 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.tools.javadoc;
import com.sun.javadoc.*;

import java.io.File;

import java.io.InputStream;

import java.io.FileInputStream;

import java.io.IOException;

import java.util.zip.ZipFile;

import java.util.zip.ZipEntry;

import com.sun.tools.javac.v8.code.Scope;

import com.sun.tools.javac.v8.code.Symbol.ClassSymbol;

import com.sun.tools.javac.v8.code.Symbol.PackageSymbol;

import com.sun.tools.javac.v8.comp.AttrContext;

import com.sun.tools.javac.v8.comp.Env;

import com.sun.tools.javac.v8.util.List;

import com.sun.tools.javac.v8.util.ListBuffer;

import com.sun.tools.javac.v8.util.Position;


/**
 * Represents a java package.  Provides access to information
 * about the package, the package's comment and tags, and the
 * classes in the package.
 *
 * @since JDK1.2
 * @author Kaiyang Liu (original)
 * @author Robert Field (rewrite)
 * @author Neal Gafter (rewrite)
 */
public class PackageDocImpl extends DocImpl implements PackageDoc {
    private static final String PACKAGE_FILE_NAME = "package.html";
    protected PackageSymbol sym;
    public String docPath = null;
    public String zipDocPath = null;
    public String zipDocEntry = null;
    boolean isIncluded = false;

    /**
     * Constructor
     */
    public PackageDocImpl(DocEnv env, PackageSymbol sym) {
        super(env, null);
        this.sym = sym;
    }

    /**
      * Do lazy initialization of "documentation" string.
      */
    String documentation() {
        if (documentation != null)
            return documentation;
        if (zipDocPath != null) {
            try {
                ZipFile f = new ZipFile(zipDocPath);
                ZipEntry entry = f.getEntry(zipDocEntry);
                if (entry != null) {
                    InputStream s = f.getInputStream(entry);
                    return (documentation = readHTMLDocumentation(s,
                            zipDocPath + File.separatorChar + zipDocEntry));
                }
            } catch (IOException exc) {
                documentation = "";
                env.error(null, "javadoc.File_Read_Error",
                        zipDocPath + File.separatorChar + zipDocEntry);
            }
        }
        if (docPath != null) {
            try {
                InputStream s = new FileInputStream(docPath);
                documentation = readHTMLDocumentation(s, docPath);
            } catch (IOException exc) {
                documentation = "";
                env.error(null, "javadoc.File_Read_Error", docPath);
            }
        } else {
            documentation = "";
        }
        return documentation;
    }

    /**
      * Cache of all classes contained in this package, including
      * member classes of those classes, and their member classes, etc.
      * Includes only those classes at the specified protection level
      * and weaker.
      */
    private List allClassesFiltered = null;

    /**
     * Cache of all classes contained in this package, including
     * member classes of those classes, and their member classes, etc.
     */
    private List allClasses = null;

    /**
     * Return a list of all classes contained in this package, including
     * member classes of those classes, and their member classes, etc.
     */
    private List getClasses(boolean filtered) {
        if (allClasses != null && !filtered) {
            return allClasses;
        }
        if (allClassesFiltered != null && filtered) {
            return allClassesFiltered;
        }
        ListBuffer classes = new ListBuffer();
        for (Scope.Entry e = sym.members().elems; e != null; e = e.sibling) {
            if (e.sym != null) {
                ClassSymbol s = (ClassSymbol) e.sym;
                ClassDocImpl c = env.getClassDoc(s);
                if (c != null && !c.isSynthetic())
                    c.addAllClasses(classes, filtered);
            }
        }
        if (filtered)
            return allClassesFiltered = classes.toList();
        else
            return allClasses = classes.toList();
    }

    /**
      * Add all included classes (including Exceptions and Errors)
      * and interfaces.
      */
    public void addAllClassesTo(ListBuffer list) {
        list.appendList(getClasses(true));
    }

    /**
      * Get all classes (including Exceptions and Errors)
      * and interfaces.
      * @since J2SE1.4.
      *
      * @return all classes and interfaces in this package, filtered to include
      * only the included classes if filter==true.
      */
    public ClassDoc[] allClasses(boolean filter) {
        List classes = getClasses(filter);
        return (ClassDoc[]) classes.toArray(new ClassDocImpl[classes.length()]);
    }

    /**
      * Get all included classes (including Exceptions and Errors)
      * and interfaces.  Same as allClasses(true).
      *
      * @return all included classes and interfaces in this package.
      */
    public ClassDoc[] allClasses() {
        return allClasses(true);
    }

    /**
      * Get ordinary classes (excluding Exceptions and Errors)
      * in this package.
      *
      * @return included classes in this package.
      */
    public ClassDoc[] ordinaryClasses() {
        ListBuffer ret = new ListBuffer();
        for (List it = getClasses(true); it.nonEmpty(); it = it.tail) {
            ClassDocImpl c = (ClassDocImpl) it.head;
            if (c.isOrdinaryClass()) {
                ret.append(c);
            }
        }
        return (ClassDoc[]) ret.toArray(new ClassDocImpl[ret.length()]);
    }

    /**
      * Get Exception classes in this package.
      *
      * @return included Exceptions in this package.
      */
    public ClassDoc[] exceptions() {
        ListBuffer ret = new ListBuffer();
        for (List it = getClasses(true); it.nonEmpty(); it = it.tail) {
            ClassDocImpl c = (ClassDocImpl) it.head;
            if (c.isException()) {
                ret.append(c);
            }
        }
        return (ClassDoc[]) ret.toArray(new ClassDocImpl[ret.length()]);
    }

    /**
      * Get Error classes in this package.
      *
      * @return included Errors in this package.
      */
    public ClassDoc[] errors() {
        ListBuffer ret = new ListBuffer();
        for (List it = getClasses(true); it.nonEmpty(); it = it.tail) {
            ClassDocImpl c = (ClassDocImpl) it.head;
            if (c.isError()) {
                ret.append(c);
            }
        }
        return (ClassDoc[]) ret.toArray(new ClassDocImpl[ret.length()]);
    }

    /**
      * Get Interfaces in this package.
      *
      * @return included interfaces in this package.
      */
    public ClassDoc[] interfaces() {
        ListBuffer ret = new ListBuffer();
        for (List it = getClasses(true); it.nonEmpty(); it = it.tail) {
            ClassDocImpl c = (ClassDocImpl) it.head;
            if (c.isInterface()) {
                ret.append(c);
            }
        }
        return (ClassDoc[]) ret.toArray(new ClassDocImpl[ret.length()]);
    }

    /**
      * Lookup for a class within this package.
      *
      * @return ClassDocImpl of found class, or null if not found.
      */
    public ClassDoc findClass(String className) {
        final boolean filtered = true;
        for (List it = getClasses(filtered); it.nonEmpty(); it = it.tail) {
            ClassDocImpl c = (ClassDocImpl) it.head;
            if (c.name().equals(className)) {
                return c;
            }
        }
        return null;
    }

    /**
      * Return true if this package is included in the active set.
      */
    public boolean isIncluded() {
        return isIncluded;
    }

    /**
      * Get package name.
      *
      * Note that we do not provide a means of obtaining the simple
      * name of a package -- package names are always returned in their
      * uniquely qualified form.
      */
    public String name() {
        String result = sym.fullName().toString();
        if (result.equals("unnamed package"))
            result = "";
        return result;
    }

    /**
      * Get package name.
      *
      * Note that we do not provide a means of obtaining the simple
      * name of a package -- package names are always returned in their
      * unique qualified form.
      */
    public String qualifiedName() {
        return name();
    }

    /**
      * set doc path for an unzipped directory
      */
    public void setDocPath(String path) {
        docPath = path + File.separatorChar + PACKAGE_FILE_NAME;
    }

    /**
      * set the doc path for zipped directory
      */
    public void setDocPath(String path, String entry) {
        zipDocPath = path;
        zipDocEntry = entry + PACKAGE_FILE_NAME;
    }

    /**
      * Return the source position of the entity, or null if
      * no position is available.
      */
    public SourcePosition position() {
        return (docPath == null) ? null :
                SourcePositionImpl.make(docPath, Position.NOPOS);
    }
}

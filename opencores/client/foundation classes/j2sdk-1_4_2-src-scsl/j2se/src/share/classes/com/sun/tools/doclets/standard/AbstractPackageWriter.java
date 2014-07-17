/*
 * @(#)AbstractPackageWriter.java	1.24 02/03/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.doclets.standard;

import com.sun.tools.doclets.*;
import com.sun.javadoc.*;
import java.io.*;
import java.lang.*;
import java.util.*;

/**
 * Abstract class to generate file for each package contents. Sub-classed to
 * generate specific formats Frame and Non-Frame Output by 
 * {@link PackageIndexFrameWriter} and {@link PackageIndexFrameWriter} 
 * respectively.
 *
 * @author Atul M Dambalkar
 */
public abstract class AbstractPackageWriter extends HtmlStandardWriter {

    /**
     * The classes to be documented.  Use this to filter out classes
     * that will not be documented.
     */
    protected Set documentedClasses;

    /**
     * The package under consideration.
     */
    PackageDoc packagedoc;

    /**
     * Create appropriate directory for the package and also initilise the 
     * relative path from this generated file to the current or
     * the destination directory.
     *
     * @param path Directories in this path will be created if they are not 
     * already there.
     * @param filename Name of the package summary file to be generated.
     * @param packagedoc PackageDoc under consideration.
     * @throws DocletAbortException
     */
    public AbstractPackageWriter(ConfigurationStandard configuration,
                                 String path, String filename, 
                                 PackageDoc packagedoc) 
                                 throws IOException {
        super(configuration, path, filename,
              DirectoryManager.getRelativePath(packagedoc.name()));
        this.packagedoc = packagedoc;
        if (configuration.root.specifiedPackages().length == 0) {
            ClassDoc[] classes = configuration.root.classes();
            documentedClasses = new HashSet();
            for (int i = 0; i < classes.length; i++) {
                documentedClasses.add(classes[i]);
            }
        }
    }

    protected abstract void generateClassListing();

    protected abstract void printPackageDescription() throws IOException;

    protected abstract void printPackageHeader(String head);

    protected abstract void printPackageFooter();

    /**
     * Generate Individual Package File with Class/Interface/Exceptions and
     * Error Listing with the appropriate links. Calls the methods from the
     * sub-classes to generate the file contents.
     * @param includeScript boolean true when including windowtitle script
     */
    protected void generatePackageFile(boolean includeScript) throws IOException {
        String pkgName = packagedoc.name();
        String[] metakeywords = { pkgName + " " + "package" };

        printHtmlHeader(pkgName, metakeywords, includeScript);
        printPackageHeader(pkgName);

        generateClassListing();
        printPackageDescription();

        printPackageFooter();
        printBodyHtmlEnd();
    }
   
    /**
     * Highlight "Package" in the navigation bar, as this is the package page.
     */
    protected void navLinkPackage() {
        navCellRevStart();
        fontStyle("NavBarFont1Rev");
        boldText("doclet.Package");
        fontEnd();
        navCellEnd();
    }
}




/*
 * @(#)ConstantsSummaryWriter.java  1.5 01/12/03
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
 * Generate the Constants Summary Page.
 *
 * @author Jamie Ho
 * @since 1.4
 */
public class ConstantsSummaryWriter extends SubWriterHolderWriter {
    
    /**
     * The configuration used in this run of the standard doclet.
     */
    ConfigurationStandard configuration;
    
    /**
     * The set of ClassDocs that have constant fields.
     */
    private Set classDocsWithConstFields;
    
    /**
     * The set of printed package headers.
     */
    private Set printedPackageHeaders;
    
    /**
     * The anchor for the unnamed package.
     */
    private String UNNAMED_PACKAGE_ANCHOR = "unnamed_package";
    
    /**
     * The heading for the unnamed package.
     */
    private String UNNAMED_PACKAGE_HEADING = "Unnamed Package";
    
    /**
     * Construct a ConstantsSummaryWriter.
     * @param configuration the configuration used in this run
     *        of the standard doclet.
     * @param filename the name of the output file.
     */
    public ConstantsSummaryWriter(ConfigurationStandard configuration,
                                  String filename) throws IOException {
        super(configuration, filename);
        this.configuration = configuration;
        classDocsWithConstFields = new HashSet();
    }
    
    /**
     * Create an instance of <code>ConstantsSummaryWriter</code> and
     * generate the constants summary.
     * @param configuration the configuration used in this run
     *        of the standard doclet.
     */
    public static void generate(ConfigurationStandard configuration) {
        ConstantsSummaryWriter constGen;
        String filename = configuration.CONSTANTS_FILE_NAME;
        try {
            constGen = new ConstantsSummaryWriter(configuration, filename);
            constGen.generateConstantsFile(configuration.root);
            constGen.close();
        } catch (IOException exc) {
            configuration.standardmessage.error(
                "doclet.exception_encountered",
                exc.toString(), filename);
            throw new DocletAbortException();
        }
    }
    
    /**
     * Generate the contants summary.
     */
    public void generateConstantsFile(RootDoc root) {
        printHtmlHeader(getText("doclet.Constants_Summary"));
        navLinks(true);
        hr();
        
        center();
        h1(); printText("doclet.Constants_Summary"); h1End();
        centerEnd();
        
        hr(4, "noshade");
        generateIndex(root);
        generateContents(root);
        
        hr();
        navLinks(false);
        printBottom();
        printBodyHtmlEnd();
    }
    
    /**
     * Generate the contants file index.
     */
    protected void generateIndex(RootDoc root) {
        PackageDoc[] packages = configuration.packages;
        printedPackageHeaders = new HashSet();
        bold(getText("doclet.Contents"));
        ul();
        String name;
        for (int i = 0; i < packages.length; i++) {
            name = packages[i].name();
            if (hasConstantField(packages[i]) && ! hasPrintedPackage(name)) {
                //add link to summary
                li();
                if (name.length() == 0) {
                    printHyperLink("#" + UNNAMED_PACKAGE_ANCHOR, UNNAMED_PACKAGE_HEADING);
                } else {
                    name = parsePackageName(name);
                    printHyperLink("#" + name, name + ".*");
                    printedPackageHeaders.add(name);
                }
                print("\n");
            }
        }
        ulEnd();
        print("\n");
    }
    
    /**
     * Generate the contants tables.
     */
    protected void generateContents(RootDoc root) {
        PackageDoc[] packages = configuration.packages;
        printedPackageHeaders = new HashSet();
        boolean first = true;
        ClassDoc[] classes;
        for (int i = 0; i < packages.length; i++) {
            if (packages[i].name().length() > 0) {
                classes = packages[i].allClasses();
            } else {
                classes = configuration.classDocCatalog.allClasses("");
            }
            boolean printPackageName = true;
            Arrays.sort(classes);
            for (int j = 0; j < classes.length; j++) {
                ClassDoc classdoc = classes[j];
                if (! classDocsWithConstFields.contains(classdoc) ||
                    ! classdoc.isIncluded()) {
                    continue;
                }
                if (printPackageName) {
                    printPackageName(packages[i].name());
                    printPackageName = false;
                }
                first = false;
                printConstantMemberInfo(classdoc);
            }
        }
    }
    
    private boolean hasConstantField(PackageDoc pkg) {
        ClassDoc[] classes;
        if (pkg.name().length() > 0) {
            classes = pkg.allClasses();
        } else {
            classes = configuration.classDocCatalog.allClasses("");
        }
        boolean found = false;
        for (int j = 0; j < classes.length; j++){
            if (classes[j].isIncluded() && hasConstantField(classes[j])) {
                found = true;
            }
        }
        return found;
    }
    
    private boolean hasConstantField (ClassDoc cd) {
        FieldDoc[] fields = cd.fields();
        for (int i = 0; i < fields.length; i++) {
            if (fields[i].constantValueExpression() != null) {
                classDocsWithConstFields.add(cd);
                return true;
            }
        }
        return false;
    }
    
    
    /**
     * Print all the constant member information.
     */
    protected void printConstantMemberInfo(ClassDoc cd) {
        //generate links backward only to public classes.
        String classlink = (cd.isPublic() || cd.isProtected())?
            getClassLink(cd):
            cd.qualifiedName();
        String name = cd.containingPackage().name();
        if (name.length() > 0) {
            printClassName(name + "." + classlink);
        } else {
            printClassName(classlink);
        }
        printConstantMembers(cd);
        tableFooter();
        p();
    }
    
    /**
     * Print summary of constant members in the
     * class.
     */
    protected void printConstantMembers(ClassDoc cd) {
        new ConstantFieldSubWriter(this, cd).printMembersSummary();
    }
    
    /**
     * Print the package name in the table format.
     */
    protected void printPackageName(String pkgname) {
        if (pkgname.length() == 0) {
            anchor(UNNAMED_PACKAGE_ANCHOR);
            pkgname = UNNAMED_PACKAGE_HEADING;
        } else {
            if (hasPrintedPackage(pkgname)) {
                return;
            }
            pkgname = parsePackageName(pkgname);
            anchor(pkgname);
        }
        table(1, "100%", 3, 0);
        trBgcolorStyle("#CCCCFF", "TableHeadingColor");
        tdAlign("left");
        font("+2");
        printedPackageHeaders.add(pkgname);
        bold(pkgname + ".*");
        tableFooter();
    }
    
    /**
     * Parse the package name.  We only want to display package name up to
     * 2 levels.
     */
    private String parsePackageName(String pkgname) {
        int index = -1;
        for (int j = 0; j < 2; j++) {
            index = pkgname.indexOf(".", index + 1);
        }
        if (index != -1) {
            pkgname = pkgname.substring(0, index);
        }
        return pkgname;
    }
    
    /**
     * Return true if the given package name has been printed.  Also
     * return true if the root of this package has been printed.
     * @param pkgname the name of the package to check.
     */
    protected boolean hasPrintedPackage(String pkgname) {
        String[] list = (String[])printedPackageHeaders.toArray(new String[] {});
        for (int i = 0; i < list.length; i++) {
            if (pkgname.startsWith(list[i])) {
                return true;
            }
        }
        return false;
    }
    
    /**
     * Print the class name in the table format.
     */
    protected void printClassName(String classstr) {
        table(0, 3, 0);
        trBgcolorStyle("#EEEEFF", "TableSubHeadingColor");
        tdColspan(3);
        bold(classstr);
        tdEnd();
        trEnd();
    }
    
    private void tableFooter() {
        fontEnd();
        tdEnd(); trEnd(); tableEnd();
        p();
    }
}




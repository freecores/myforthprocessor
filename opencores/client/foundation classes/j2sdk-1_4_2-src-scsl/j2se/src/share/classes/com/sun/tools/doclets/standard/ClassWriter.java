/*
 * @(#)ClassWriter.java	1.74 03/01/23
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
 * Generate the Class Information Page.
 * @see com.sun.javadoc.ClassDoc
 * @see java.util.Collections
 * @see java.util.List
 * @see java.util.ArrayList
 * @see java.util.HashMap
 *
 * @author Atul M Dambalkar
 * @author Robert Field
 */
public class ClassWriter extends SubWriterHolderWriter {
    
    protected ClassDoc classdoc;
    
    protected ClassTree classtree;
    
    protected ClassDoc prev;
    
    protected ClassDoc next;
    
    protected MethodSubWriter methodSubWriter;
    
    protected ConstructorSubWriter constrSubWriter;
    
    protected FieldSubWriter fieldSubWriter;
    
    protected ClassSubWriter nestedSubWriter;
    
    protected static Set containingPackagesSeen;
    
    /**
     * @throws IOException
     * @throws DocletAbortException
     */
    public ClassWriter(ConfigurationStandard configuration,
                       String path, String filename, ClassDoc classdoc,
                       ClassDoc prev, ClassDoc next, ClassTree classtree) throws IOException {
        super(configuration, path, filename,
              DirectoryManager.getRelativePath(classdoc.containingPackage().name()));
        this.classdoc = classdoc;
        configuration.currentcd = classdoc;
        this.classtree = classtree;
        this.prev = prev;
        this.next = next;
        methodSubWriter = new MethodSubWriter(this, classdoc, configuration);
        constrSubWriter = new ConstructorSubWriter(this, classdoc);
        fieldSubWriter = new FieldSubWriter(this, classdoc);
        nestedSubWriter = new ClassSubWriter(this, classdoc);
                           
        if(containingPackagesSeen == null){
           containingPackagesSeen = new HashSet();
        }
    }
    
    /**
     * Generate a class page.
     *
     * @param prev the previous class to generated, or null if no previous.
     * @param classdoc the class to generate.
     * @param next the next class to be generated, or null if no next.
     * @throws DocletAbortException
     */
    public static void generate(ConfigurationStandard configuration,
                                ClassDoc classdoc, ClassDoc prev,
                                ClassDoc next, ClassTree classtree) {
        ClassWriter clsgen;
        String pkgpath =
            DirectoryManager.getDirectoryPath(classdoc.containingPackage());
        String filename = classdoc.name() + ".html";
        try {
            clsgen = new ClassWriter(configuration,
                                     pkgpath, filename, classdoc,
                                     prev, next, classtree);
            clsgen.generateClassFile();
            PackageDoc cp = classdoc.containingPackage();
            
            if((configuration.packages == null ||
                    Arrays.binarySearch(configuration.packages, cp) < 0) &&
                ! containingPackagesSeen.contains(cp.name())){
                   
                //Only copy doc files dir if the containing package is not documented
                //AND if we have not documented a class from the same package already.
                //Otherwise, we are making duplicate copies.
                clsgen.copyDocFiles(configuration, getSourcePath(configuration, classdoc.containingPackage()),
                            pkgpath + fileseparator + DOC_FILES_DIR_NAME, true);
                containingPackagesSeen.add(cp.name());
            }
            
            clsgen.close();
        } catch (IOException exc) {
            configuration.standardmessage.error(
                "doclet.exception_encountered",
                exc.toString(), filename);
            throw new DocletAbortException();
        }
    }
    
    /**
     * Print this package link
     */
    protected void navLinkPackage() {
        navCellStart();
        PackageDoc pkg = classdoc.containingPackage();
        if (pkg != null && pkg.name().length() > 0) {
            printHyperLink("package-summary.html", "", getText("doclet.Package"),
                       true, "NavBarFont1");
        } else {
           super.navLinkPackage();
        }
        navCellEnd();
    }
    
    /**
     * Print class page indicator
     */
    protected void navLinkClass() {
        navCellRevStart();
        fontStyle("NavBarFont1Rev");
        boldText("doclet.Class");
        fontEnd();
        navCellEnd();
    }
    
    /**
     * Print class use link
     */
    protected void navLinkClassUse() {
        navCellStart();
        printHyperLink("class-use/" + filename, "",
                       getText("doclet.navClassUse"), true, "NavBarFont1");
        navCellEnd();
    }
    
    /**
     * Print previous package link
     */
    protected void navLinkPrevious() {
        if (prev == null) {
            printText("doclet.Prev_Class");
        } else {
            printClassLink(prev, getText("doclet.Prev_Class"), true);
        }
    }
    
    /**
     * Print next package link
     */
    protected void navLinkNext() {
        if (next == null) {
            printText("doclet.Next_Class");
        } else {
            printClassLink(next, getText("doclet.Next_Class"), true);
        }
    }
    
    /**
     * Generate the class file contents.
     */
    public void generateClassFile() {
        String cltype = getText(classdoc.isInterface()?
                                    "doclet.Interface":
                                    "doclet.Class");
        PackageDoc pkg = classdoc.containingPackage();
        String pkgname = (pkg != null)? pkg.name(): "";
        String clname = classdoc.name();
        String label = cltype + " " + clname;

        printHtmlHeader(clname, 
            configuration.metakeywords.getMetaKeywords(classdoc));
        navLinks(true);
        hr();
        println("<!-- ======== START OF CLASS DATA ======== -->");
        h2();
        if (pkgname.length() > 0) {
            font("-1"); print(pkgname); fontEnd(); br();
        }
        print(label);
        h2End();
        
        // if this is a class (not an interface) then generate
        // the super class tree.
        if (classdoc.isClass()) {
            pre();
            printTreeForClass(classdoc);
            preEnd();
        }
        
        printSuperImplementedInterfacesInfo();
        
        printSubClassInterfaceInfo();
        
        if (classdoc.isInterface()) {
            printImplementingClasses();
        }
        
        printEnclosingInfo();
        
        hr();
        
        printDeprecated();
        
        printClassDescription();
        p();
        if(!configuration.nocomment) {
            // generate documentation for the class.
            if (classdoc.inlineTags().length > 0) {
                printInlineComment(classdoc);
                p();
            }
            // Print Information about all the tags here
            generateTagInfo(classdoc);
            hr();
            p();
        } else {
            hr();
        }
        
        printAllMembers();
        
        println("<!-- ========= END OF CLASS DATA ========= -->");
        hr();
        navLinks(false);
        printBottom();
        printBodyHtmlEnd();
    }


    /**
     * Print summary and detail information for the specified members in the
     * class.
     */
    protected void printAllMembers() {
        if(! configuration.nocomment){
            println("<!-- ======== NESTED CLASS SUMMARY ======== -->"); println();
            nestedSubWriter.printMembersSummary();
            nestedSubWriter.printInheritedMembersSummary();
            println();
            println("<!-- =========== FIELD SUMMARY =========== -->"); println();
            fieldSubWriter.printMembersSummary();
            fieldSubWriter.printInheritedMembersSummary();
            println();
            println("<!-- ======== CONSTRUCTOR SUMMARY ======== -->"); println();
            constrSubWriter.printMembersSummary();
            println();
            println("<!-- ========== METHOD SUMMARY =========== -->"); println();
            methodSubWriter.printMembersSummary();
            methodSubWriter.printInheritedMembersSummary();
        
            p();
        }
        println();
        println("<!-- ============ FIELD DETAIL =========== -->"); println();
        fieldSubWriter.printMembers();
        println();
        println("<!-- ========= CONSTRUCTOR DETAIL ======== -->"); println();
        constrSubWriter.printMembers();
        println();
        println("<!-- ============ METHOD DETAIL ========== -->"); println();
        methodSubWriter.printMembers();
    }
        
    /**
     * Print the class description regarding iterfaces implemented, classes
     * inheritted.
     */
    protected void printClassDescription() {
        boolean isInterface = classdoc.isInterface();
        dl();
        dt();
        print(classdoc.modifiers() + " ");
        if (!isInterface) {
            print("class ");
        }
        if (configuration().genSrc) {
            printSrcLink(classdoc, classdoc.name());
        } else {
            bold(classdoc.name());
        }
        if (!isInterface) {
            ClassDoc superclass = classdoc.superclass();
            if (superclass != null) {
                dt();
                print("extends ");
                printClassLink(superclass);
            }
        }
        ClassDoc[] implIntfacs = classdoc.interfaces();
        if (implIntfacs != null && implIntfacs.length > 0) {
            dt();
            print(isInterface? "extends " : "implements ");
            printClassLink(implIntfacs[0]);
            for (int i = 1; i < implIntfacs.length; i++) {
                print(", ");
                printClassLink(implIntfacs[i]);
            }
        }
        dlEnd();
    }
    
    /**
     * Mark the class as deprecated if it is.
     */
    protected void printDeprecated() {
        Tag[] deprs = classdoc.tags("deprecated");
        if (deprs.length > 0) {
            Tag[] commentTags = deprs[0].inlineTags();
            if (commentTags.length > 0) {
                boldText("doclet.Deprecated");
                space();
                printInlineDeprecatedComment(classdoc, deprs[0]);
            }
            p();
        }
    }
    
    /**
     * Generate the indent and get the line image for the class tree.
     * For user accessibility, the image includes the alt attribute
     * "extended by".  (This method is not intended for a class
     * implementing an interface, where "implemented by" would be required.)
     *
     * indent  integer indicating the number of spaces to indent
     */
    protected void printStep(int indent) {
        print(spaces(4 * indent - 2));
        String alttext = getText("doclet.extended_by");
        print("<IMG SRC=\"" + relativepathNoSlash + "/resources/inherit.gif\" " +
              "ALT=\"" + alttext + "\">");
    }
    
    /**
     * Print the class hierarchy tree for this class only.
     */
    protected int printTreeForClass(ClassDoc cd) {
        ClassDoc sup = cd.superclass();
        int indent = 0;
        if (sup != null) {
            indent = printTreeForClass(sup);
            printStep(indent);
        }
        if (cd.equals(classdoc)) {
            if (configuration.shouldExcludeQualifier(
                    classdoc.containingPackage().name())) {
                bold(cd.name());
            } else {
                bold(cd.qualifiedName());
            }
        } else {
            printQualifiedClassLink(cd);
        }
        println();
        return indent + 1;
    }
    
    /**
     * Which are the sub-classes or sub-interfaces for this class?
     */
    protected void printSubClassInterfaceInfo() {
        // Before using TreeBuilder.getSubClassList
        // make sure that tree.html is generated prior.
        if (classdoc.qualifiedName().equals("java.lang.Object") ||
            classdoc.qualifiedName().equals("org.omg.CORBA.Object")) {
            return;    // Don't generate the list, too huge
        }
        List subclasses = classdoc.isClass()?
            classtree.subs(classdoc): // it's a class
            classtree.allSubs(classdoc); // it's an interface
        if (subclasses.size() > 0) {
            printInfoHeader();
            if (classdoc.isClass()) {
                boldText("doclet.Subclasses");
            } else { // this is an interface
                boldText("doclet.Subinterfaces");
            }
            printSubClassLinkInfo(subclasses);
        }
    }
    
    /**
     * If this is the interface which are the classes, that implement this?
     */
    protected void printImplementingClasses() {
        if (classdoc.qualifiedName().equals("java.lang.Cloneable") ||
            classdoc.qualifiedName().equals("java.io.Serializable")) {
            return;   // Don't generate the list, too big
        }
        List implcl = classtree.implementingclasses(classdoc);
        if (implcl.size() > 0) {
            printInfoHeader();
            boldText("doclet.Implementing_Classes");
            printSubClassLinkInfo(implcl);
        }
    }
    
    protected void printSuperImplementedInterfacesInfo() {
        ProgramElementDoc[] intarr = Util.getAllInterfaces(classdoc);
        Arrays.sort(intarr);
        if (intarr.length > 0) {
            printInfoHeader();
            boldText(classdoc.isClass()?
                         "doclet.All_Implemented_Interfaces":
                         "doclet.All_Superinterfaces");
            printSubClassLinkInfo(Arrays.asList(intarr));
        }
    }
    
    protected void printInfoHeader() {
        dl();
        dt();
    }
    
    /**
     * Generate a link for the sub-classes.
     */
    protected void printSubClassLinkInfo(List list) {
        int i = 0;
        Object[] classDocList = list.toArray();
        //Sort the list to be printed.
        Arrays.sort(classDocList);
        print(' ');
        dd();
        for (; i < list.size() - 1; i++) {
            printClassLink((ClassDoc)(classDocList[i]));
            print(", ");
        }
        printClassLink((ClassDoc)(classDocList[i]));
        ddEnd();
        dlEnd();
    }
    
    protected void navLinkTree() {
        navCellStart();
        if (classdoc.containingPackage().name().length() == 0) {
            printHyperLink(relativepath + "overview-tree.html", "",
                           getText("doclet.Tree"), true, "NavBarFont1");
        } else {
            printHyperLink("package-tree.html", "", getText("doclet.Tree"),
                           true, "NavBarFont1");
        }
        navCellEnd();
    }
    
    protected void printSummaryDetailLinks() {
        tr();
        tdVAlignClass("top", "NavBarCell3");
        font("-2");
        print("  ");
        navSummaryLinks();
        fontEnd();
        tdEnd();
        
        tdVAlignClass("top", "NavBarCell3");
        font("-2");
        navDetailLinks();
        fontEnd();
        tdEnd();
        trEnd();
    }
    
    protected void navSummaryLinks() {
        printText("doclet.Summary");
        print("&nbsp;");
        nestedSubWriter.navSummaryLink();
        navGap();
        fieldSubWriter.navSummaryLink();
        navGap();
        constrSubWriter.navSummaryLink();
        navGap();
        methodSubWriter.navSummaryLink();
    }
    
    protected void navDetailLinks() {
        printText("doclet.Detail");
        print("&nbsp;");
        fieldSubWriter.navDetailLink();
        navGap();
        constrSubWriter.navDetailLink();
        navGap();
        methodSubWriter.navDetailLink();
    }
    
    protected void navGap() {
        space();
        print('|');
        space();
    }
    
    protected void printEnclosingInfo() {
        ClassDoc outerClass = classdoc.containingClass();
        if (outerClass != null) {
            printInfoHeader();
            
            if (classdoc.isInterface()) {
                boldText("doclet.Enclosing_Interface");
            } else {
                boldText("doclet.Enclosing_Class");
            }
            dd();
            printClassLink(outerClass);
            ddEnd();
            dlEnd();
        }
    }
}






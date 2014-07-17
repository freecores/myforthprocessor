/*
 * @(#)SerializedFormWriter.java	1.23 03/01/23
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
 * Generate the Serialized Form Information Page.
 *
 * @author Atul M Dambalkar
 */
public class SerializedFormWriter extends SubWriterHolderWriter {

    ConfigurationStandard configuration;

    /**
     * The header for the serial version UID.  Save the string
     * here instead of the properties file because we do not want
     * this string to be localized.
     */
    private static final String SERIAL_VERSION_UID_HEADER = "serialVersionUID:";
    
    /**
     * @throws IOException
     * @throws DocletAbortException
     */
    public SerializedFormWriter(ConfigurationStandard configuration,
                                String filename) throws IOException {
        super(configuration, filename);
        this.configuration = configuration;
    }
    
    /**
     * Generate a serialized form page.
     * @throws DocletAbortException
     */
    public static void generate(ConfigurationStandard configuration) {
        SerializedFormWriter serialgen;
        String filename = "serialized-form.html";
        try {
            serialgen = new SerializedFormWriter(configuration, filename);
            serialgen.generateSerializedFormFile(configuration.root);
            serialgen.close();
        } catch (IOException exc) {
            configuration.standardmessage.error(
                "doclet.exception_encountered",
                exc.toString(), filename);
            throw new DocletAbortException();
        }
    }
    
    /**
     * Generate the serialized form file.
     */
    public void generateSerializedFormFile(RootDoc root) {
        printHtmlHeader(getText("doclet.Serialized_Form"));
        navLinks(true);
        hr();
        
        center();
        h1(); printText("doclet.Serialized_Form"); h1End();
        centerEnd();
        
        generateContents(root);
        
        hr();
        navLinks(false);
        printBottom();
        printBodyHtmlEnd();
    }
    
    /**
     * Generate the serialized form file contents.
     */
    protected void generateContents(RootDoc root) {
        PackageDoc[] packages = root.specifiedPackages();
        ClassDoc[] cmdlineClasses = root.specifiedClasses();
        boolean first = true;
        for (int i = 0; i < packages.length; i++) {
            if (!serialInclude(packages[i])) {
                continue;
            }
            ClassDoc[] classes = packages[i].allClasses(false);
            boolean printPackageName = true;
            if (!serialClassFoundToDocument(classes)) {
                continue;
            }
            Arrays.sort(classes);
            for (int j = 0; j < classes.length; j++) {
                ClassDoc classdoc = classes[j];
                if(classdoc.isClass() && classdoc.isSerializable()) {
                    if(!serialClassInclude(classdoc)) {
                        continue;
                    }
                    if (printPackageName) {
                        hr(4, "noshade");
                        printPackageName(packages[i].name());
                        printPackageName = false;
                    }
                    first = false;
                    printSerialMemberInfo(classdoc);
                }
            }
        }
        if (cmdlineClasses.length > 0) {
            Arrays.sort(cmdlineClasses);
            for (int i = 0; i < cmdlineClasses.length; i++) {
                ClassDoc classdoc = cmdlineClasses[i];
                if(classdoc.isClass() && classdoc.isSerializable()) {
                    if (!first) {
                        hr(4, "noshade");
                    }
                    first = false;
                    printSerialMemberInfo(classdoc);
                }
            }
        }
    }
    
    /**
     * Return true if a serialized class is found with @serialinclude tag,
     * false otherwise.
     */
    protected boolean serialClassFoundToDocument(ClassDoc[] classes) {
        for (int i = 0; i < classes.length; i++) {
            if (serialClassInclude(classes[i])) {
                return true;
            }
        }
        return false;
    }
    
    
    /**
     * Print all the serializable member information.
     */
    protected void printSerialMemberInfo(ClassDoc cd) {
        //generate links backward only to public classes.
        String classLink = (cd.isPublic() || cd.isProtected())?
            getQualifiedClassLink(cd):
            cd.qualifiedName();
        anchor(cd.qualifiedName());
        String superClassLink = cd.superclass() != null ?
            getClassLink(cd.superclass()) : null;
        
        printClassName(superClassLink == null ?
            getText("doclet.Class_0_implements_serializable", classLink) :
            getText("doclet.Class_0_extends_implements_serializable", classLink, superClassLink));
        printSerialUID(cd);
        printSerialMembers(cd);
        p();
    }

    /**
     * Print the serial UID.
     * @param cd the serializable class to print the serial UID for.
     */
     protected void printSerialUID(ClassDoc cd) {
         FieldDoc[] fields = cd.fields(false);
         for (int i = 0; i < fields.length; i++) {
             if (fields[i].name().equals("serialVersionUID") &&
                 fields[i].constantValueExpression() != null) {
                 bold(SERIAL_VERSION_UID_HEADER +
                     "&nbsp;");
                 println(fields[i].constantValueExpression());
                 p();
                 return;
             }
         }
     }
    
    /**
     * Print summary and detail information for the serial members in the
     * class.
     */
    protected void printSerialMembers(ClassDoc cd) {
        new SerialMethodSubWriter(this, cd, configuration).printMembers();
        new SerialFieldSubWriter(this, cd).printMembers();
    }
    
    /**
     * Print the package name in the table format.
     */
    protected void printPackageName(String pkgname) {
        tableHeader();
        tdAlign("center");
        font("+2");
        boldText("doclet.Package");
        print(' ');
        bold(pkgname);
        tableFooter();
    }
    
    protected void printClassName(String classstr) {
        tableHeader();
        tdColspan(2);
        font("+2");
        bold(classstr);
        tableFooter();
    }
    
    protected void tableHeader() {
        tableIndexSummary();
        trBgcolorStyle("#CCCCFF", "TableSubHeadingColor");
    }
    
    protected void tableFooter() {
        fontEnd();
        tdEnd(); trEnd(); tableEnd();
        p();
    }
}



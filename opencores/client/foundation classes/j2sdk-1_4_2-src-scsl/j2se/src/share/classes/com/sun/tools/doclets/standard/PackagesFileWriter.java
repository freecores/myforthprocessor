/*
 * @(#)PackagesFileWriter.java	1.10 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.doclets.standard;

import com.sun.tools.doclets.*;
import java.io.*;
import java.lang.*;
import java.util.*;

/**
 * Generate the "packages.html" file for the backward compatibility.
 *
 * @author Atul M Dambalkar
 */
public class PackagesFileWriter extends HtmlStandardWriter {

    /**
     * Constructor.
     */
    public PackagesFileWriter(ConfigurationStandard configuration,
                              String filename) throws IOException {
        super(configuration, filename);
    }

    /**
     * Generate the file.
     * @throws DocletAbortException
     */
    public static void generate(ConfigurationStandard configuration) {
        PackagesFileWriter packgen;
        String filename = "";
        try {
            filename = "packages.html";
            packgen = new PackagesFileWriter(configuration, filename);
            packgen.generatePackagesFile();
            packgen.close();
        } catch (IOException exc) {
            configuration.standardmessage.error(
                        "doclet.exception_encountered",
                        exc.toString(), filename);
            throw new DocletAbortException();
        }
    }

    /**
     * Generate the packages file.
     */
    protected void generatePackagesFile() {
        printHtmlHeader("");

        printPackagesFileContents();

        printBodyHtmlEnd();
    }

    /**
     * Print the pacakges file contents.
     */
    protected void printPackagesFileContents() {
        br(); br(); br();
        center(); 
        printText("doclet.Packages_File_line_1"); 
        printText("doclet.Packages_File_line_2");
        br(); printNbsps();
        printHyperLink("index.html", getText("doclet.Frame_Version"));
        br(); printNbsps();
        printHyperLink(configuration.topFile,
                       getText("doclet.Non_Frame_Version"));
        centerEnd();
    }

}


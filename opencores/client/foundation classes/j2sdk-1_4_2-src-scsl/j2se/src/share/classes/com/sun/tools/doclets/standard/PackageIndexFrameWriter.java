/*
 * @(#)PackageIndexFrameWriter.java	1.21 02/10/21
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
 * Generate the package index for the left-hand frame in the generated output.
 * A click on the package name in this frame will update the page in the bottom
 * left hand frame with the listing of contents of the clicked package.
 *
 * @author Atul M Dambalkar
 */
public class PackageIndexFrameWriter extends AbstractPackageIndexWriter {

    /**
     * Construct the PackageIndexFrameWriter object.
     *
     * @param filename Name of the package index file to be generated.
     */
    public PackageIndexFrameWriter(ConfigurationStandard configuration,
                                   String filename) throws IOException {
        super(configuration, filename);
    }

    /**
     * Generate the package index file named "overview-frame.html".
     * @throws DocletAbortException
     */
    public static void generate(ConfigurationStandard configuration) {
        PackageIndexFrameWriter packgen;
        String filename = "overview-frame.html";
        try {
            packgen = new PackageIndexFrameWriter(configuration, filename);
            packgen.generatePackageIndexFile(false);
            packgen.close();
        } catch (IOException exc) {
            configuration.standardmessage.error(
                        "doclet.exception_encountered",
                        exc.toString(), filename);
            throw new DocletAbortException();
        }
    }

    /**
     * Print each package name on separate rows.
     *
     * @param pd PackageDoc
     */
    protected void printIndexRow(PackageDoc pd) {
        fontStyle("FrameItemFont");
        if (pd.name().length() > 0) {
            printTargetHyperLink(pathString(pd, "package-frame.html"),
                             "packageFrame", pd.name());
        } else {
            printTargetHyperLink("package-frame.html",
                             "packageFrame", "&lt;unnamed package>");
        }
        fontEnd();
        br();
    }

    /**
     * Print the "-packagesheader" string in bold format, at top of the page,
     * if it is not the empty string.  Otherwise print the "-header" string.
     * Despite the name, there is actually no navigation bar for this page.
     */
    protected void printNavigationBarHeader() {
        printTableHeader();
        fontSizeStyle("+1", "FrameTitleFont");
        if (configuration.packagesheader.length() > 0) {
            bold(replaceDocRootDir(configuration.packagesheader));
        } else {
            bold(replaceDocRootDir(configuration.header));
        }
        fontEnd();
        printTableFooter();
    }

    /**
     * Do nothing as there is no overview information in this page.
     */
    protected void printOverviewHeader() {
    }

    /**
     * Print Html "table" tag for the package index format.
     *
     * @param text Text string will not be used in this method.
     */
    protected void printIndexHeader(String text) {
        printTableHeader();
    }

    /**
     * Print Html closing "table" tag at the end of the package index.
     */
    protected void printIndexFooter() {
        printTableFooter();
    }

    /**
     * Print "All Classes" link at the top of the left-hand frame page.
     */
    protected void printAllClassesPackagesLink() {
        fontStyle("FrameItemFont");
        printTargetHyperLink("allclasses-frame.html", "packageFrame",
                              getText("doclet.All_Classes"));
        fontEnd();
        p();
        fontSizeStyle("+1", "FrameHeadingFont");
        printText("doclet.Packages");
        fontEnd();
        br();
    }

    /**
     * Just print some space, since there is no navigation bar for this page.
     */
    protected void printNavigationBarFooter() {
        p();
        space();
    }

    /**
     * Print Html closing tags for the table for package index.
     */
    protected void printTableFooter() {
        tdEnd();
        trEnd();
        tableEnd();
    }

    /**
     * Print Html tags for the table for package index.
     */
    protected void printTableHeader() {
        table();
        tr();
        tdNowrap();
    }
}





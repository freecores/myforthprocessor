/*
 * @(#)PackageListWriter.java	1.10 03/01/23
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
 * Write out the package index.
 *
 * @see com.sun.javadoc.PackageDoc
 * @see com.sun.tools.doclets.HtmlDocWriter
 * @author Atul M Dambalkar
 */
public class PackageListWriter extends HtmlStandardWriter {

    /**
     * Constructor.
     */
    public PackageListWriter(ConfigurationStandard configuration,
                             String filename) throws IOException {
        super(configuration, filename);
    }

    /**
     * Generate the package index.
     *
     * @param root the root of the doc tree.
     * @throws DocletAbortException
     */
    public static void generate(ConfigurationStandard configuration) {
        PackageListWriter packgen;
        String filename = "package-list";
        try {
            packgen = new PackageListWriter(configuration, filename);
            packgen.generatePackageListFile(configuration.root);
            packgen.close();
        } catch (IOException exc) {
            configuration.standardmessage.error(
                        "doclet.exception_encountered",
                        exc.toString(), filename);
            throw new DocletAbortException();
        }
    }

    protected void generatePackageListFile(RootDoc root) {
        PackageDoc[] packages = configuration.packages;
        String[] names = new String[packages.length];
        for (int i = 0; i < packages.length; i++) {
            names[i] = packages[i].name();
        }
        Arrays.sort(names);
        for (int i = 0; i < packages.length; i++) {
            println(names[i]);
        }
    }
}




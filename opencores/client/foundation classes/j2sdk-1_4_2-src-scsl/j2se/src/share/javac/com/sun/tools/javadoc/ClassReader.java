/**
 * @(#)ClassReader.java	1.2 01/05/21
 *
 * Copyright 2003 Sun Microsystems, Inc. All Rights Reserved.
 *
 * This software is the proprietary information of Sun Microsystems, Inc.
 * Use is subject to license terms.
 *
 */
package com.sun.tools.javadoc;
import com.sun.tools.javac.v8.code.Symbol.PackageSymbol;

import com.sun.tools.javac.v8.util.Hashtable;

import java.io.File;


/**
 * Javadoc uses an extended class reader that records package.html entries
 *  @author Neal Gafter
 */
class ClassReader extends com.sun.tools.javac.v8.code.ClassReader {
    DocEnv docenv;

    ClassReader(Hashtable options) {
        super(options);
    }

    /**
      * Override extraZipFileActions to check for package documentation
      */
    protected void extraZipFileActions(PackageSymbol pack, String zipEntryName,
            String classPathName, String zipName) {
        if (docenv != null && zipEntryName.endsWith("package.html"))
            docenv.getPackageDoc(pack).setDocPath(zipName, classPathName);
    }

    /**
      * Override extraFileActions to check for package documentation
      */
    protected void extraFileActions(PackageSymbol pack, String fileName,
            File fileDir) {
        if (docenv != null && fileName.equals("package.html"))
            docenv.getPackageDoc(pack).setDocPath(fileDir.getAbsolutePath());
    }
}

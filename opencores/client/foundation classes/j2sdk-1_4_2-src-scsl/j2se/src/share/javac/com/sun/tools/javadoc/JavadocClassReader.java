/**
 * @(#)JavadocClassReader.java	1.8 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.tools.javadoc;
import com.sun.tools.javac.v8.code.Symbol.PackageSymbol;

import com.sun.tools.javac.v8.util.Context;

import java.io.File;


/**
 * Javadoc uses an extended class reader that records package.html entries
 *  @author Neal Gafter
 */
class JavadocClassReader extends com.sun.tools.javac.v8.code.ClassReader {
    private static final Context.Key javadocClassReaderKey = new Context.Key();

    public static JavadocClassReader instance0(Context context) {
        JavadocClassReader instance =
                (JavadocClassReader) context.get(javadocClassReaderKey);
        if (instance == null)
            instance = new JavadocClassReader(context);
        return instance;
    }
    private DocEnv docenv;

    private JavadocClassReader(Context context) {
        super(context, true);
        context.put(javadocClassReaderKey, this);
        docenv = DocEnv.instance(context);
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

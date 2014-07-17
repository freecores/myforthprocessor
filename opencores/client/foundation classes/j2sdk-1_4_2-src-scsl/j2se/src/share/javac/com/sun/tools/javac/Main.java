/**
 * @(#)Main.java	1.16 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.tools.javac;
import java.io.PrintWriter;


public class Main {

    public Main() {
        super();
    }
    static {
        ClassLoader loader = Main.class.getClassLoader();
        if (loader != null)
            loader.setPackageAssertionStatus("com.sun.tools.javac", true);
    }

    /**
      * Command line interface.
      * @param args   The command line parameters.
      */
    public static void main(String[] args) {
        System.exit(compile(args));
    }

    /**
      * Programmatic interface.
      * @param args   The command line parameters.
      */
    public static int compile(String[] args) {
        com.sun.tools.javac.v8.Main compiler = new com.sun.tools.javac.v8.Main("javac");
        return compiler.compile(args);
    }

    /**
      * Programmatic interface.
      * @param args   The command line parameters.
      * @param out    Where the compiler's output is directed.
      */
    public static int compile(String[] args, PrintWriter out) {
        com.sun.tools.javac.v8.Main compiler =
                new com.sun.tools.javac.v8.Main("javac", out);
        return compiler.compile(args);
    }
}

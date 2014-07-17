/*
 * @(#)Printer.java	1.23 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.media.sound;

/**
 * Printer allows you to set up global debugging status and print
 * messages accordingly.
 *
 * @version 1.23 03/01/23
 * @author David Rivas
 * @author Kara Kytle
 */
class Printer {

    static final boolean err = false;
    static final boolean debug = false;
    static final boolean trace = false;
    static final boolean verbose = false;
    static final boolean release = false;

    /*static void setErrorPrint(boolean on) {

      err = on;
      }

      static void setDebugPrint(boolean on) {

      debug = on;
      }

      static void setTracePrint(boolean on) {

      trace = on;
      }

      static void setVerbosePrint(boolean on) {

      verbose = on;
      }

      static void setReleasePrint(boolean on) {

      release = on;
      }*/

    public static void err(String str) {

	if (err)
	    System.out.println(str);
    }

    public static void debug(String str) {

	if (debug)
	    System.out.println(str);
    }

    public static void trace(String str) {

	if (trace)
	    System.out.println(str);
    }

    public static void verbose(String str) {

	if (verbose)
	    System.out.println(str);
    }

    public static void release(String str) {

	if (release)
	    System.out.println(str);
    }
}

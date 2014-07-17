/*
 * @(#)Doclet.java	1.13 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javadoc;

/**
 * This is an example of a starting class for a doclet,
 * showing the entry-point methods.  A starting class must 
 * import com.sun.javadoc.* and implement the 
 * <code>start(RootDoc)</code> method, as described in the 
 * <a href="package-summary.html#package_description">package 
 * description</a>.  If the doclet takes command line options, 
 * it must also implement <code>optionLength</code> and 
 * <code>validOptions</code>.  To start the doclet, you pass
 * <code>-doclet</code> followed by the fully-qualified
 * name of the starting class on the javadoc tool command line.
 */
public abstract class Doclet {

    /**
     * Generate documentation here.
     * This method is required for all doclets.
     *
     * @return true on success.
     */
    public static boolean start(RootDoc root) {
        return true;
    }

    /**
     * Check for doclet-added options.  Returns the number of 
     * arguments you must specify on the command line for the
     * given option.  For example, "-d docs" would return 2.
     * <P>
     * This method is required if the doclet contains any options.
     * If this method is missing, Javadoc will print an invalid flag 
     * error for every option.
     *
     * @return number of arguments on the command line for an option
     *         including the option name itself.  Zero return means
     *         option not known.  Negative value means error occurred.
     */
    public static int optionLength(String option) {
        return 0;  // default is option unknown
    }

    /**
     * Check that options have the correct arguments.
     * <P>
     * This method is not required, but is recommended,
     * as every option will be considered valid if this method
     * is not present.  It will default gracefully (to true) 
     * if absent.
     * <P>
     * Printing option related error messages (using the provided
     * DocErrorReporter) is the responsibility of this method.
     *
     * @return true if the options are valid.
     */
    public static boolean validOptions(String options[][],
                                       DocErrorReporter reporter) {
        return true;  // default is options are valid
    }

}


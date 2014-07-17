/*
 * @(#)ExtensionInstallationException.java	1.6 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.misc;

/*
 * Exception when installation of an extension has failed for 
 * any reason
 *
 * @author  Jerome Dochez
 * @version 1.6, 01/23/03
 */

public class ExtensionInstallationException extends Exception {

    /*
     * <p>
     * Construct a new exception with an exception reason
     * </p>
     */
    public ExtensionInstallationException(String s) {
	super(s);
    }
}

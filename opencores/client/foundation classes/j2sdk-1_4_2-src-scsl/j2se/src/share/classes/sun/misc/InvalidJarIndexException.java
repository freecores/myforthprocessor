/*
 * @(#)InvalidJarIndexException.java	1.5 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.misc;

import java.lang.LinkageError;

/**
 * Thrown if the URLClassLoader finds the INDEX.LIST file of
 * a jar file contains incorrect information.
 *
 * @author   Zhenghua Li
 * @version  1.5, 01/23/03
 * @since   1.3
 */

public
class InvalidJarIndexException extends RuntimeException {

    /**
     * Constructs an <code>InvalidJarIndexException</code> with no 
     * detail message.
     */
    public InvalidJarIndexException() {
	super();
    }

    /**
     * Constructs an <code>InvalidJarIndexException</code> with the 
     * specified detail message. 
     *
     * @param   s   the detail message.
     */
    public InvalidJarIndexException(String s) {
	super(s);
    }
}

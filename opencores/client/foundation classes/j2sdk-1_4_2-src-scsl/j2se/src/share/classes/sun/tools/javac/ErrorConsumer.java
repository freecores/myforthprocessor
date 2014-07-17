/*
 * @(#)ErrorConsumer.java	1.10 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.javac;

/**
 * Allows for easier parsing of errors and warnings from the compiler
 */
 
public
interface ErrorConsumer {
	public void pushError(String errorFileName, 
							int line, 
							String message,
							String referenceText, String referenceTextPointer);
};

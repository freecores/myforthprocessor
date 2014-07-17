/*
 * @(#)ErrorMessage.java	1.17 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.javac;

/**
 * A sorted list of error messages
 */
final
class ErrorMessage {
    long where;
    String message;
    ErrorMessage next;

    /**
     * Constructor
     */
    ErrorMessage(long where, String message) {
	this.where = where;
	this.message = message;
    }
}

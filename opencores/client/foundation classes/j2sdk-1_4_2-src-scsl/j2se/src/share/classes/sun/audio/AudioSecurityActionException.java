/*
 * @(#)AudioSecurityActionException.java	1.7 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.audio;

class AudioSecurityActionException extends Exception {
    private Exception exception;
    AudioSecurityActionException(Exception e) {
	exception = e;
    }

    Exception getException() {
	return exception;
    }
}

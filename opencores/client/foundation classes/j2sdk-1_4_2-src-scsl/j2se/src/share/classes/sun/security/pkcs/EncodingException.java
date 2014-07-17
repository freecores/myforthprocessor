/*
 * @(#)EncodingException.java	1.9 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/**
 * Generic PKCS Encoding exception.
 * 
 * @version 1.3	96/09/15
 * @author Benjamin Renaud 
 */

package sun.security.pkcs;

public class EncodingException extends Exception {
    public EncodingException() {
	super();
    }

    public EncodingException(String s) {
	super(s);
    }
}

/*
 * @(#)ParsingException.java	1.13 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/**
 * Generic PKCS Parsing exception. 
 * 
 * @version 1.13	01/23/03
 * @author Benjamin Renaud 
 */

package sun.security.pkcs;

import java.io.IOException;

public class ParsingException extends IOException {
    public ParsingException() {
	super();
    }

    public ParsingException(String s) {
	super(s);
    }
}

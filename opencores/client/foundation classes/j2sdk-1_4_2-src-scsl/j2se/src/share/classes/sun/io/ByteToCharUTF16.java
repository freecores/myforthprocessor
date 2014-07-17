/*
 * @(#)ByteToCharUTF16.java	1.5 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.io;


/**
 * Convert byte arrays containing Unicode characters into arrays of actual
 * Unicode characters, assuming an initial byte-order mark.
 */

public class ByteToCharUTF16 extends ByteToCharUnicode {

    public ByteToCharUTF16() {
	super(AUTO, true);
    }

    public String getCharacterEncoding() {
	return "UTF-16";
    }

}

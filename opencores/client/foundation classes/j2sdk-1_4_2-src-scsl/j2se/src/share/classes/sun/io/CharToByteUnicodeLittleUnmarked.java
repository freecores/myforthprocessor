/*
 * @(#)CharToByteUnicodeLittleUnmarked.java	1.10 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.io;


/**
 * Convert arrays containing Unicode characters into arrays of bytes, using
 * little-endian byte order; do not write a byte-order mark before the first
 * converted character.
 *
 * @version 	1.10, 03/01/23
 * @author	Mark Reinhold
 */

public class CharToByteUnicodeLittleUnmarked extends CharToByteUnicode {

    public CharToByteUnicodeLittleUnmarked () {
	byteOrder = LITTLE;
	usesMark = false;
    }

}

/*
 * @(#)ByteToCharUnicodeBigUnmarked.java	1.5 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.io;
import java.io.*;


/**
 * Convert byte arrays containing Unicode characters into arrays of actual
 * Unicode characters, assuming a big-endian byte order and requiring no
 * byte-order mark.
 *
 * @version 	1.5, 03/01/23
 * @author	Mark Reinhold
 */

public class ByteToCharUnicodeBigUnmarked extends ByteToCharUnicode {

    public ByteToCharUnicodeBigUnmarked() {
	super(BIG, false);
    }

}

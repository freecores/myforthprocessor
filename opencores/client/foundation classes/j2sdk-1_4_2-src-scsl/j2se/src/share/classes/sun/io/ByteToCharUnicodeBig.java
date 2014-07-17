/*
 * @(#)ByteToCharUnicodeBig.java	1.10 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.io;
import java.io.*;


/**
 * Convert byte arrays containing Unicode characters into arrays of actual
 * Unicode characters, assuming a big-endian byte order.
 *
 * @version 	1.10, 03/01/23
 * @author	Mark Reinhold
 */

public class ByteToCharUnicodeBig extends ByteToCharUnicode {

    public ByteToCharUnicodeBig() {
	super(BIG, true);
    }

}

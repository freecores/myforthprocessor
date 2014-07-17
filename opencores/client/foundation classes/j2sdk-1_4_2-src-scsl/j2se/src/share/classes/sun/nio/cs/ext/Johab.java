/*
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)Johab.java	1.4 03/01/23
 */

package sun.nio.cs.ext;

import java.nio.charset.Charset;
import java.nio.charset.CharsetDecoder;
import java.nio.charset.CharsetEncoder;
import java.nio.charset.CharacterCodingException;
import sun.io.ByteToCharJohab;
import sun.io.CharToByteJohab;

public class Johab
    extends Charset
{

    public Johab() {
	super("x-Johab", ExtendedCharsets.aliasesFor("x-Johab"));
    }

    public boolean contains(Charset cs) {
	return ((cs.name().equals("US-ASCII"))
		|| (cs instanceof Johab));
    }

    public CharsetDecoder newDecoder() {
	return new Decoder(this);
    }

    public CharsetEncoder newEncoder() {
	return new Encoder(this);
    }

    private static class Decoder extends DoubleByteDecoder {

	private static ByteToCharJohab b2c = new ByteToCharJohab();

	public Decoder(Charset cs) {
	    super(cs,
		  b2c.getIndex1(),
		  b2c.getIndex2(),
		  0x20,
		  0xFE);
	}
    }

    private static class Encoder extends DoubleByteEncoder {

	private static CharToByteJohab c2b = new CharToByteJohab();;

	public Encoder(Charset cs) {
	    super(cs,
		  c2b.getIndex1(),
		  c2b.getIndex2());
	}
    }
}

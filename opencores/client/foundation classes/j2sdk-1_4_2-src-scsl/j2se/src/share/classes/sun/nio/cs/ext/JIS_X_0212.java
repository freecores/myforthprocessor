/*
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)JIS_X_0212.java	1.4 03/01/23
 */

package sun.nio.cs.ext;

import java.nio.charset.Charset;
import java.nio.charset.CharsetDecoder;
import java.nio.charset.CharsetEncoder;
import sun.io.ByteToCharJIS0212;
import sun.io.CharToByteJIS0212;

public class JIS_X_0212
    extends Charset
{
    public JIS_X_0212() {
	super("JIS_X0212-1990", ExtendedCharsets.aliasesFor("JIS_X0212-1990"));
    }

    public boolean contains(Charset cs) {
	return (cs instanceof JIS_X_0212);
    }

    public CharsetDecoder newDecoder() {
	return new Decoder(this);
    }

    public CharsetEncoder newEncoder() {
	return new Encoder(this);
    }

    protected static class Decoder extends DoubleByteDecoder {

        private static ByteToCharJIS0212 b2c = new ByteToCharJIS0212();

	    public Decoder(Charset cs) {
		super(cs,
		      b2c.getIndex1(),
		      b2c.getIndex2(),
		      0x21,
		      0x7E);
	    }

	protected char convSingleByte(int b) {
	    return REPLACE_CHAR;
	}

    }

    protected static class Encoder extends DoubleByteEncoder {

	private static CharToByteJIS0212 c2b = new CharToByteJIS0212();

	public Encoder(Charset cs) {
	    super(cs,
		  c2b.getIndex1(),
		  c2b.getIndex2());
	}
    }
}

/*
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)Big5.java	1.3 03/01/23
 */

package sun.nio.cs.ext;

import java.nio.charset.Charset;
import java.nio.charset.CharsetDecoder;
import java.nio.charset.CharsetEncoder;
import sun.io.ByteToCharBig5;
import sun.io.CharToByteBig5;

public class Big5
    extends Charset
{


    public Big5() {
	super("Big5", ExtendedCharsets.aliasesFor("Big5"));
    }

    public boolean contains(Charset cs) {
	return ((cs.name().equals("US-ASCII"))
		|| (cs instanceof Big5));
    }

    public CharsetDecoder newDecoder() {
	return new Decoder(this);
    }

    public CharsetEncoder newEncoder() {
	return new Encoder(this);
    }

    protected static class Decoder extends DoubleByteDecoder {

	private static ByteToCharBig5 b2c = new ByteToCharBig5();

	public Decoder(Charset cs) {
	    super(cs,
		  b2c.getIndex1(),
		  b2c.getIndex2(),
		  0x40,
		  0xFE);
	}
    }

    protected static class Encoder extends DoubleByteEncoder {

	private static CharToByteBig5 c2b = new CharToByteBig5();;

	public Encoder(Charset cs) {
	    super(cs,
		  c2b.getIndex1(),
		  c2b.getIndex2());
	}
    }
}

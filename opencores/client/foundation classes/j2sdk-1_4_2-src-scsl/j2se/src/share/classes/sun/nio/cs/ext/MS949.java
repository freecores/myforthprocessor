/*
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)MS949.java	1.4 03/01/23
 */

package sun.nio.cs.ext;

import java.nio.charset.Charset;
import java.nio.charset.CharsetDecoder;
import java.nio.charset.CharsetEncoder;
import sun.nio.cs.HistoricallyNamedCharset;
import sun.io.ByteToCharMS949;
import sun.io.CharToByteMS949;
import sun.nio.cs.HistoricallyNamedCharset;

public class MS949
    extends Charset implements HistoricallyNamedCharset
{

    public MS949() {
	super("x-windows-949", ExtendedCharsets.aliasesFor("x-windows-949"));
    }

    public String historicalName() {
	return "MS949";
    }

    public boolean contains(Charset cs) {
	return ((cs.name().equals("US-ASCII"))
		|| (cs instanceof MS949));
    }

    public CharsetDecoder newDecoder() {
	return new Decoder(this);
    }

    public CharsetEncoder newEncoder() {
	return new Encoder(this);
    }

    private static class Decoder extends DoubleByteDecoder {

	private static ByteToCharMS949 b2c = new ByteToCharMS949();

	public Decoder(Charset cs) {
	    super(cs,
		  b2c.getIndex1(),
		  b2c.getIndex2(),
		  0x41,
		  0xFE);
	}
    }

    private static class Encoder extends DoubleByteEncoder {

	private static CharToByteMS949 c2b = new CharToByteMS949();

	public Encoder(Charset cs) {
	    super(cs,
	          c2b.getIndex1(),
		  c2b.getIndex2());
	}
    }
}

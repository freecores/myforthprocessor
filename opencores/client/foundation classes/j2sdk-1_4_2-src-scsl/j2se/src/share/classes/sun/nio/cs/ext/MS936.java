/*
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)MS936.java	1.4 03/01/23
 */

package sun.nio.cs.ext;

import java.nio.charset.Charset;
import java.nio.charset.CharsetDecoder;
import java.nio.charset.CharsetEncoder;
import sun.io.ByteToCharMS936;
import sun.io.CharToByteMS936;
import sun.nio.cs.HistoricallyNamedCharset;

public class MS936
    extends Charset
    implements HistoricallyNamedCharset
{

    public MS936() {
	super("x-mswin-936", ExtendedCharsets.aliasesFor("x-mswin-936"));
    }

    public String historicalName() {
	return "MS936";
    }

    public boolean contains(Charset cs) {
	return ((cs.name().equals("US-ASCII"))
		|| (cs instanceof MS936));
    }

    public CharsetDecoder newDecoder() {
	return new Decoder(this);
    }

    public CharsetEncoder newEncoder() {
	return new Encoder(this);
    }

    private static class Decoder extends DoubleByteDecoder {

	private static ByteToCharMS936 b2c = new ByteToCharMS936();

	public Decoder(Charset cs) {
	    super(cs,
		  b2c.getIndex1(),
		  b2c.getIndex2(),
		  0x40,
		  0xFE);
	}
    }

    private static class Encoder extends DoubleByteEncoder {

	private static CharToByteMS936 c2b = new CharToByteMS936();

	public Encoder(Charset cs) {
	    super(cs,
		  c2b.getIndex1(),
		  c2b.getIndex2());
	}
    }
}

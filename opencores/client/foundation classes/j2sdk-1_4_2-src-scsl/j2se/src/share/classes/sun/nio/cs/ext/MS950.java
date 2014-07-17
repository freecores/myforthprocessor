/*
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)MS950.java	1.5 03/01/23
 */

package sun.nio.cs.ext;

import java.nio.charset.Charset;
import java.nio.charset.CharsetDecoder;
import java.nio.charset.CharsetEncoder;
import sun.io.ByteToCharMS950;
import sun.io.CharToByteMS950;
import sun.nio.cs.HistoricallyNamedCharset;

public class MS950
    extends Charset
    implements HistoricallyNamedCharset
{

    public MS950() {
	super("x-windows-950", ExtendedCharsets.aliasesFor("x-windows-950"));
    }

    public String historicalName() {
	return "MS950";
    }

    public boolean contains(Charset cs) {
	return ((cs.name().equals("US-ASCII"))
		|| (cs instanceof MS950));
    }

    public CharsetDecoder newDecoder() {
	return new Decoder(this);
    }

    public CharsetEncoder newEncoder() {
	return new Encoder(this);
    }

    protected static class Decoder extends DoubleByteDecoder {

	private static ByteToCharMS950 b2c = new ByteToCharMS950();

	public Decoder(Charset cs) {
	    super(cs,
		  b2c.getIndex1(),
		  b2c.getIndex2(),
		  0x40,
		  0xFE);
	}
    }

    protected static class Encoder extends DoubleByteEncoder {

	private static CharToByteMS950 c2b = new CharToByteMS950();

	public Encoder(Charset cs) {
	    super(cs,
		  c2b.getIndex1(),
		  c2b.getIndex2());
	}
    }
}

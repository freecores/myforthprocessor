/*
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)EUC_CN.java	1.4 03/01/23
 */

package sun.nio.cs.ext;

import java.nio.charset.Charset;
import java.nio.charset.CharsetDecoder;
import java.nio.charset.CharsetEncoder;
import sun.io.ByteToCharEUC_CN;
import sun.io.CharToByteEUC_CN;
import sun.nio.cs.HistoricallyNamedCharset;

public class EUC_CN
    extends Charset
    implements HistoricallyNamedCharset
{

    public EUC_CN() {
	super("x-EUC-CN", ExtendedCharsets.aliasesFor("x-EUC-CN"));
    }

    public String historicalName() {
	return "EUC_CN";
    }

    public boolean contains(Charset cs) {
	return ((cs instanceof EUC_CN)
	        || (cs.name().equals("US-ASCII")));
    }

    public CharsetDecoder newDecoder() {
	return new Decoder(this);
    }

    public CharsetEncoder newEncoder() {
	return new Encoder(this);
    }

    private static class Decoder extends DoubleByteDecoder {

	private static ByteToCharEUC_CN b2c = new ByteToCharEUC_CN();

	public Decoder(Charset cs) {
	    super(cs,
		  b2c.getIndex1(),
		  b2c.getIndex2(),
		  0xA1,
		  0xFE);
	}
    }

    private static class Encoder extends DoubleByteEncoder {

	private static CharToByteEUC_CN c2b = new CharToByteEUC_CN();

	public Encoder(Charset cs) {
	    super(cs,
		  c2b.getIndex1(),	
		  c2b.getIndex2());
	}
    }
}

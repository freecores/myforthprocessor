/*
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)EUC_KR.java	1.3 03/01/23
 */

package sun.nio.cs.ext;

import java.nio.charset.Charset;
import java.nio.charset.CharsetDecoder;
import java.nio.charset.CharsetEncoder;
import java.nio.charset.CharacterCodingException;
import sun.io.ByteToCharEUC_KR;
import sun.io.CharToByteEUC_KR;
import sun.nio.cs.HistoricallyNamedCharset;

public class EUC_KR
    extends Charset
    implements HistoricallyNamedCharset
{
    public EUC_KR() {
	super("EUC-KR", ExtendedCharsets.aliasesFor("EUC-KR"));
    }

    public String historicalName() {
	return "EUC_KR";
    }

    public boolean contains(Charset cs) {
	return ((cs instanceof EUC_KR)
	        || (cs.name().equals("US-ASCII")));
    }

    public CharsetDecoder newDecoder() {
	return new Decoder(this);
    }

    public CharsetEncoder newEncoder() {
	return new Encoder(this);
    }

    private static class Decoder extends DoubleByteDecoder {

	private static ByteToCharEUC_KR b2c = new ByteToCharEUC_KR();

	public Decoder(Charset cs) {
	    super(cs,
		  b2c.getIndex1(),
		  b2c.getIndex2(),	
		  0xA1,
		  0xFE);
	}

    }

    private static class Encoder extends DoubleByteEncoder {

	private static CharToByteEUC_KR c2b = new CharToByteEUC_KR();

	public Encoder(Charset cs) {
	    super(cs,
		  c2b.getIndex1(),
		  c2b.getIndex2());
	}
    }
}

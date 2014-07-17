/*
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)SJIS.java	1.3 03/01/23
 */

package sun.nio.cs.ext;

import java.nio.charset.Charset;
import java.nio.charset.CharsetDecoder;
import java.nio.charset.CharsetEncoder;
import java.nio.charset.CoderResult;
import sun.nio.cs.HistoricallyNamedCharset;

public class SJIS
    extends Charset
    implements HistoricallyNamedCharset
{

    public SJIS() {
	super("Shift_JIS", ExtendedCharsets.aliasesFor("Shift_JIS"));
    }

    public String historicalName() {
	return "SJIS";
    }

    public boolean contains(Charset cs) {
	return ((cs.name().equals("US-ASCII"))
		|| (cs instanceof JIS_X_0201)
		|| (cs instanceof SJIS)
		|| (cs instanceof JIS_X_0208));
    }

    public CharsetDecoder newDecoder() {
	return new Decoder(this);
    }

    public CharsetEncoder newEncoder() {
	return new Encoder(this);
    }

    private static class Decoder extends JIS_X_0208.Decoder {

	JIS_X_0201.Decoder jis0201;

	private Decoder(Charset cs) {
	    super(cs);
	    jis0201 = new JIS_X_0201.Decoder(cs);
	}

	protected char decodeSingle(int b) {
	    // If the high bits are all off, it's ASCII == Unicode
	    if ((b & 0xFF80) == 0) {
		return (char)b;
	    }
	    return jis0201.decode(b);
	}

	protected char decodeDouble(int c1, int c2) {
	    int adjust = c2 < 0x9F ? 1 : 0;
	    int rowOffset = c1 < 0xA0 ? 0x70 : 0xB0;
	    int cellOffset = (adjust == 1) ? (c2 > 0x7F ? 0x20 : 0x1F) : 0x7E;
	    int b1 = ((c1 - rowOffset) << 1) - adjust;
	    int b2 = c2 - cellOffset;
	    return super.decodeDouble(b1, b2);
	}
    }

    private static class Encoder extends JIS_X_0208.Encoder {

	private JIS_X_0201.Encoder jis0201; 

	short[] j0208Index1;	
	String[] j0208Index2;	

	private Encoder(Charset cs) {
	    super(cs);
	    jis0201 = new JIS_X_0201.Encoder(cs);
	    j0208Index1 = super.getIndex1();
	    j0208Index2 = super.getIndex2();
	}

	public boolean canEncode(char c) {
	    return true;
	}

	protected int encodeSingle(char inputChar) {
	    byte b;

	    // \u0000 - \u007F map straight through
	    if ((inputChar & 0xFF80) == 0)
		return (byte)inputChar;

	    if ((b = jis0201.encode(inputChar)) == 0)
		return -1;
	    else
	        return b;
	}

	protected int encodeDouble(char ch) {
	    int offset = j0208Index1[ch >> 8] << 8;
	    int pos = j0208Index2[offset >> 12].charAt((offset & 0xfff) + (ch & 0xff));
	    if (pos == 0) {
		/* Zero value indicates this Unicode has no mapping to
	         * JIS0208.
		 * We bail here because the JIS -> SJIS algorithm produces
		 * bogus SJIS values for invalid JIS input.  Zero should be
		 * the only invalid JIS value in our table.
		 */
		return 0;
	    }
	    /*
	     * This algorithm for converting from JIS to SJIS comes from		     * Ken Lunde's "Understanding Japanese Information Processing",
	     * pg 163.
	     */
	    int c1 = (pos >> 8) & 0xff;
	    int c2 = pos & 0xff;
	    int rowOffset = c1 < 0x5F ? 0x70 : 0xB0;
	    int cellOffset = (c1 % 2 == 1) ? (c2 > 0x5F ? 0x20 : 0x1F) : 0x7E;
	    return ((((c1 + 1 ) >> 1) + rowOffset) << 8) | (c2 + cellOffset);
	}
    }
}

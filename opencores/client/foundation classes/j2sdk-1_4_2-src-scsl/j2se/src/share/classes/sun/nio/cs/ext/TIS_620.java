/*
 * @(#)TIS_620.java	1.2	03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.nio.cs.ext;

import java.nio.ByteBuffer;
import java.nio.CharBuffer;
import java.nio.charset.Charset;
import java.nio.charset.CharsetDecoder;
import java.nio.charset.CharsetEncoder;
import java.nio.charset.CharacterCodingException;
import java.nio.charset.MalformedInputException;
import java.nio.charset.UnmappableCharacterException;
import sun.nio.cs.StandardCharsets;
import sun.nio.cs.SingleByteDecoder;
import sun.nio.cs.SingleByteEncoder;
import sun.nio.cs.HistoricallyNamedCharset;
import sun.io.ByteToCharTIS620;
import sun.io.CharToByteTIS620;

public class TIS_620
    extends Charset
    implements HistoricallyNamedCharset
{

    public TIS_620() {
	super("TIS-620", StandardCharsets.aliasesFor("TIS-620"));
    }

    public String historicalName() {
	return "TIS620";
    }

    public boolean contains(Charset cs) {
	return ((cs.name().equals("US-ASCII"))
		|| (cs instanceof TIS_620));
    }

    public CharsetDecoder newDecoder() {
	return new Decoder(this);
    }

    public CharsetEncoder newEncoder() {
	return new Encoder(this);
    }

    private static class Decoder extends SingleByteDecoder {
	private static ByteToCharTIS620 b2c = new ByteToCharTIS620();
	    public Decoder(Charset cs) {
		super(cs, b2c.getByteToCharTable());
	}
    }

    private static class Encoder extends SingleByteEncoder {
	private static CharToByteTIS620 c2b = new CharToByteTIS620();
	    public Encoder(Charset cs) {
		super(cs, c2b.getIndex1(), c2b.getIndex2(), 0xFF00, 0x00FF, 8);
	}
    }
}

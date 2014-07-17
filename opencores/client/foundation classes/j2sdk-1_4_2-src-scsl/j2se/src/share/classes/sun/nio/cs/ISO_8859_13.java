/*
 * @(#)ISO_8859_13.java	1.3	03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.nio.cs;

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
import sun.io.ByteToCharISO8859_13;
import sun.io.CharToByteISO8859_13;


public class ISO_8859_13
    extends Charset
    implements HistoricallyNamedCharset
{

    public ISO_8859_13() {
	super("ISO-8859-13", StandardCharsets.aliasesFor("ISO-8859-13"));
    }

    public String historicalName() {
	return "ISO8859_13";
    }

    public boolean contains(Charset cs) {
	return ((cs.name().equals("US-ASCII"))
		|| (cs instanceof ISO_8859_13));
    }

    public CharsetDecoder newDecoder() {
	return new Decoder(this);
    }

    public CharsetEncoder newEncoder() {
	return new Encoder(this);
    }


    private static class Decoder extends SingleByteDecoder {
	private static ByteToCharISO8859_13 b2c = new ByteToCharISO8859_13();
	public Decoder(Charset cs) {
	    super(cs, b2c.getByteToCharTable());
	}
    }

    private static class Encoder extends SingleByteEncoder {
	private static CharToByteISO8859_13 c2b = new CharToByteISO8859_13();
	    public Encoder(Charset cs) {
	    super(cs, c2b.getIndex1(), c2b.getIndex2(), 0xFF00, 0x00FF, 8);
	}
    }
}

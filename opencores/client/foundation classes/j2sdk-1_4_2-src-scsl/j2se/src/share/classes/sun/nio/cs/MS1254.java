/*
 * @(#)MS1254.java	1.3	03/01/23
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
import sun.io.ByteToCharCp1254;
import sun.io.CharToByteCp1254;


public class MS1254
    extends Charset
    implements HistoricallyNamedCharset
{

    public String historicalName() {
	return "Cp1254";
    }

    public MS1254() {
	super("windows-1254", StandardCharsets.aliasesFor("windows-1254"));
    }

    public boolean contains(Charset cs) {
	return ((cs.name().equals("US-ASCII"))
		|| (cs instanceof MS1254));
    }

    public CharsetDecoder newDecoder() {
	return new Decoder(this);
    }

    public CharsetEncoder newEncoder() {
	return new Encoder(this);
    }

    private static class Decoder extends SingleByteDecoder {
	private static ByteToCharCp1254 b2c = new ByteToCharCp1254();
	    public Decoder(Charset cs) {
		super(cs, b2c.getByteToCharTable());
	}
    }

    private static class Encoder extends SingleByteEncoder {
	private static CharToByteCp1254 c2b = new CharToByteCp1254();
	    public Encoder(Charset cs) {
		super(cs, c2b.getIndex1(), c2b.getIndex2(), 0xFF00, 0x00FF, 8);
	}
    }
}

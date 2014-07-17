/*
 * @(#)MS1258.java	1.2	03/01/23
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
import sun.io.ByteToCharCp1258;
import sun.io.CharToByteCp1258;


public class MS1258
    extends Charset
    implements HistoricallyNamedCharset
{

    public String historicalName() {
	return "Cp1258";
    }

    public MS1258() {
	super("windows-1258", StandardCharsets.aliasesFor("windows-1258"));
    }

    public boolean contains(Charset cs) {
	return ((cs.name().equals("US-ASCII"))
		|| (cs instanceof MS1258));
    }

    public CharsetDecoder newDecoder() {
	return new Decoder(this);
    }

    public CharsetEncoder newEncoder() {
	return new Encoder(this);
    }

    private static class Decoder extends SingleByteDecoder {
	private static ByteToCharCp1258 b2c = new ByteToCharCp1258();
	    public Decoder(Charset cs) {
		super(cs, b2c.getByteToCharTable());
	}
    }

    private static class Encoder extends SingleByteEncoder {
	private static CharToByteCp1258 c2b = new CharToByteCp1258();
	    public Encoder(Charset cs) {
		super(cs, c2b.getIndex1(), c2b.getIndex2(), 0xFF00, 0x00FF, 8);
	}
    }
}

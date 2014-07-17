/*
 * @(#)MS932.java	1.4	03/01/23
 *
 * Copyright 2003 by Sun Microsystems, Inc.  All Rights Reserved.
 * 
 * This software is the proprietary information of Sun Microsystems, Inc.
 * Use is subject to license terms.
 */


package sun.nio.cs.ext;

import java.nio.charset.Charset;
import java.nio.charset.CharsetDecoder;
import java.nio.charset.CharsetEncoder;
import java.nio.charset.CoderResult;

public class MS932
    extends Charset
{
    public MS932() {
	super("windows-31j", ExtendedCharsets.aliasesFor("windows-31j"));
    }

    public boolean contains(Charset cs) {
	return ((cs.name().equals("US-ASCII"))
		|| (cs instanceof JIS_X_0201)
		|| (cs instanceof MS932));
    }

    public CharsetDecoder newDecoder() {
	return new Decoder(this);
    }

    public CharsetEncoder newEncoder() {
	return new Encoder(this);
    }

    private static class Decoder extends MS932DB.Decoder {

	JIS_X_0201.Decoder jisDec0201;

	private Decoder(Charset cs) {
	    super(cs);
	    jisDec0201 = new JIS_X_0201.Decoder(cs);
	}

	protected char decodeSingle(int b) {
	    // If the high bits are all off, it's ASCII == Unicode
	    if ((b & 0xFF80) == 0) {
		return (char)b;
	    }
	    return jisDec0201.decode(b);
	}
    }

    private static class Encoder extends MS932DB.Encoder {

	private JIS_X_0201.Encoder jisEnc0201; 

	
	private Encoder(Charset cs) {
	    super(cs);
	    jisEnc0201 = new JIS_X_0201.Encoder(cs);
	}

	protected int encodeSingle(char inputChar) {

	    byte b;
	    // \u0000 - \u007F map straight through
	    if ((inputChar & 0xFF80) == 0) {
		return ((byte)inputChar);
	    }

	    if ((b = jisEnc0201.encode(inputChar)) == 0)
		return -1;
	    else
		return b;
	}
    }
}

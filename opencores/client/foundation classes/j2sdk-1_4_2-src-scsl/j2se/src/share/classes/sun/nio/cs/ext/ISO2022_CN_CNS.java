/*
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)ISO2022_CN_CNS.java	1.4 03/01/23
 */

package sun.nio.cs.ext;

import java.nio.charset.Charset;
import java.nio.ByteBuffer;
import java.nio.CharBuffer;
import java.nio.charset.CharsetDecoder;
import java.nio.charset.CharsetEncoder;
import java.nio.charset.CoderResult;
import sun.nio.cs.HistoricallyNamedCharset;

public class ISO2022_CN_CNS extends ISO2022 
implements HistoricallyNamedCharset
{
    public ISO2022_CN_CNS() {
	super("x-ISO-2022-CN-CNS", ExtendedCharsets.aliasesFor("x-ISO-2022-CN-CNS"));
    }

    public boolean contains(Charset cs) {
	// overlapping repertoire of EUC_TW, CNS11643
	return ((cs instanceof EUC_TW) ||
		(cs.name().equals("US-ASCII")) ||
	     (cs instanceof ISO2022_CN_CNS));
    }

    public String historicalName() {
	return "ISO2022CN_CNS";
    }

    // Decoder unsupported
    public CharsetDecoder newDecoder() {
	return null;
    }

    public CharsetEncoder newEncoder() {
	return new Encoder(this);
    }

    private static class Encoder extends ISO2022.Encoder {

	public Encoder(Charset cs)
	{
	    super(cs);
	    SODesig = "$)G";
	    SS2Desig = "$*H";
	    SS3Desig = "$+I";

	    try {
		Charset cset = Charset.forName("EUC_TW"); // CNS11643
		ISOEncoder = cset.newEncoder();
	    } catch (Exception e) { }
	}

	public boolean canEncode(char c) {
	    return (c != '\uFFFD');
	}
	
	/*
	 * Since ISO2022-CN-CNS possesses a CharsetEncoder
	 * without the corresponding CharsetDecoder half the
	 * default replacement check needs to be overridden
	 * since the parent class version attempts to
	 * decode 0x3f (?).  
	 */

	public boolean isLegalReplacement(byte[] repl) {
	    // 0x3f is OK as the replacement byte
	    return (repl.length == 1 && repl[0] == (byte) 0x3f);
	}
    }
}

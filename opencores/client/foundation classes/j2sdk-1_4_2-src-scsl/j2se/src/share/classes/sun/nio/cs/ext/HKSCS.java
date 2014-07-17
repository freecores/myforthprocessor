/*
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)HKSCS.java	1.3 03/01/23
 */

package sun.nio.cs.ext;

import java.nio.charset.Charset;
import java.nio.charset.CharsetDecoder;
import java.nio.charset.CharsetEncoder;
import sun.io.ByteToCharHKSCS;
import sun.io.CharToByteHKSCS;

abstract class HKSCS
    extends Charset
{

    public HKSCS() {
	super("HKSCS", ExtendedCharsets.aliasesFor("HKSCS"));
    }

    public CharsetDecoder newDecoder() {
	return new Decoder(this);
    }

    public CharsetEncoder newEncoder() {
	return new Encoder(this);
    }

    protected static class Decoder extends DoubleByteDecoder {

        private static ByteToCharHKSCS b2c = new ByteToCharHKSCS();

	public Decoder(Charset cs) {
	    super(cs,
	          b2c.getIndex1(),
		  b2c.getIndex2(),
		  0x40,
		  0xFE);
	}
    }

    protected static class Encoder extends DoubleByteEncoder {
        private static CharToByteHKSCS c2b = new CharToByteHKSCS();

	public Encoder(Charset cs) {
	    super(cs,
		  c2b.getIndex1(),
		  c2b.getIndex2());
	}
    }
}

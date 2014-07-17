/*
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)JIS_X_0208.java	1.4 03/01/23
 */

package sun.nio.cs.ext;

import java.nio.charset.Charset;
import java.nio.charset.CharsetDecoder;
import java.nio.charset.CharsetEncoder;
import sun.io.ByteToCharJIS0208;
import sun.io.CharToByteJIS0208;

public class JIS_X_0208
    extends Charset
{

    public JIS_X_0208() {
	super("x-JIS0208", ExtendedCharsets.aliasesFor("x-JIS0208"));
    }

    public boolean contains(Charset cs) {
	return (cs instanceof JIS_X_0208);
    }

    public CharsetDecoder newDecoder() {
	return new Decoder(this);
    }

    public CharsetEncoder newEncoder() {
	return new Encoder(this);
    }

    protected static class Decoder extends DoubleByteDecoder {

	private static ByteToCharJIS0208 b2c = new ByteToCharJIS0208();

	public Decoder(Charset cs) {
	    super(cs,
		  b2c.getIndex1(),
		  b2c.getIndex2(),
		  0x21,
		  0x7E);
	}

	protected char convSingleByte(int b) {
	    return REPLACE_CHAR;
	}
	public short[] getIndex1() {
	   return b2c.getIndex1();
	}

	public String[] getIndex2() {
	   return b2c.getIndex2();
	}
    }

    protected static class Encoder extends DoubleByteEncoder {

	private static CharToByteJIS0208 c2b = new CharToByteJIS0208();

	// Default replacement is 0x21, 0x29 (jisx0208 fullwidth question mark)
        protected static byte[] repl = { (byte)0x21, (byte)0x29 }; 

	public Encoder(Charset cs) {
	    super(cs,
		  c2b.getIndex1(),
		  c2b.getIndex2()
		 );
	}

	protected Encoder(Charset cs, float avg, float max) {
	    super(cs,
		 c2b.getIndex1(),
		 c2b.getIndex2(),
		 avg,
		 max);
	}

	protected Encoder(Charset cs, byte[] repl, float avg, float max) {
	    super(cs, c2b.getIndex1(), c2b.getIndex2(), repl , avg, max);
	}


	public short[] getIndex1() {
	   return c2b.getIndex1();
	}

	public String[] getIndex2() {
	   return c2b.getIndex2();
	}
    }
}

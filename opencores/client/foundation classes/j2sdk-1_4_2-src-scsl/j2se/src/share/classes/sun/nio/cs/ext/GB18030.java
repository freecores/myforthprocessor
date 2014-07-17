/*
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)GB18030.java	1.6 03/01/23
 */

package sun.nio.cs.ext;

import java.nio.ByteBuffer;
import java.nio.CharBuffer;
import java.nio.charset.Charset;
import java.nio.charset.CharsetDecoder;
import java.nio.charset.CharsetEncoder;
import java.nio.charset.CoderResult;
import sun.nio.cs.Surrogate;
import sun.io.ByteToCharGB18030;
import sun.io.CharToByteGB18030;

public class GB18030
    extends Charset
{
    private static final int GB18030_SINGLE_BYTE = 1;
    private static final int GB18030_DOUBLE_BYTE = 2;
    private static final int GB18030_FOUR_BYTE = 3;
    static ByteToCharGB18030 decoderGB18K;
    static CharToByteGB18030 encoderGB18K;

    public GB18030() {
	super("GB18030", ExtendedCharsets.aliasesFor("GB18030"));
	decoderGB18K = new ByteToCharGB18030();
	encoderGB18K = new CharToByteGB18030();
    }

    public boolean contains(Charset cs) {
	return ((cs.name().equals("US-ASCII"))
	        || (cs.name().equals("GBK"))
	        || (cs.name().equals("ISO-8859-1"))
	        || (cs.name().equals("ISO-8859-2"))
	        || (cs.name().equals("ISO-8859-3"))
	        || (cs.name().equals("ISO-8859-4"))
	        || (cs.name().equals("ISO-8859-5"))
	        || (cs.name().equals("ISO-8859-6"))
	        || (cs.name().equals("ISO-8859-7"))
	        || (cs.name().equals("ISO-8859-8"))
	        || (cs.name().equals("ISO-8859-9"))
	        || (cs.name().equals("ISO-8859-13"))
	        || (cs.name().equals("UTF-8"))
	        || (cs.name().equals("UTF-16"))
	        || (cs.name().equals("UTF-16LE"))
	        || (cs.name().equals("UTF-16BE"))
	        || (cs.name().equals("ISO-8859-15"))
	        || (cs.name().equals("windows-1251"))
	        || (cs.name().equals("windows-1252"))
	        || (cs.name().equals("windows-1253"))
	        || (cs.name().equals("windows-1254"))
	        || (cs.name().equals("windows-1255"))
	        || (cs.name().equals("windows-1256"))
	        || (cs.name().equals("windows-1257"))
	        || (cs.name().equals("windows-1258"))
	        || (cs.name().equals("windows-932"))
	        || (cs.name().equals("x-mswin-936"))
	        || (cs.name().equals("x-windows-949"))
	        || (cs.name().equals("x-windows-950"))
	        || (cs.name().equals("windows-31j"))
	        || (cs.name().equals("JIS_X0201"))
	        || (cs.name().equals("JIS_X0208-1990"))
	        || (cs.name().equals("JIS_X0212"))
	        || (cs.name().equals("Shift_JIS"))
	        || (cs.name().equals("x-EUC-CN"))
	        || (cs.name().equals("EUC-KR"))
	        || (cs.name().equals("x-EUC-TW"))
	        || (cs.name().equals("EUC-JP"))
	        || (cs.name().equals("euc-jp-linux"))
	        || (cs.name().equals("KOI8-R"))
	        || (cs.name().equals("TIS-620"))
	        || (cs.name().equals("x-ISCII91"))
	        || (cs.name().equals("Big5"))
	        || (cs.name().equals("Big5-HKSCS"))
	        || (cs.name().equals("x-MS950-HKSCS"))
	        || (cs.name().equals("ISO-2022-JP"))
	        || (cs.name().equals("ISO-2022-KR"))
	        || (cs.name().equals("x-ISO-2022-CN-CNS"))
	        || (cs.name().equals("x-ISO-2022-CN-GB"))
	        || (cs.name().equals("x-Johab"))
		|| (cs instanceof GB18030));
    }

    public CharsetDecoder newDecoder() {
	return new Decoder(this);
    }

    public CharsetEncoder newEncoder() {
	return new Encoder(this);
    }

    private static class Decoder extends CharsetDecoder {

	private static final char REPLACE_CHAR = '\uFFFD';
        private int currentState = GB18030_DOUBLE_BYTE;

	short[] b2cOuter;
	String[] b2cInner;
	short[] index1;
	String[] index2;

	private Decoder(Charset cs) {
	    super(cs, 1.0f, 2.0f);
	    b2cOuter = decoderGB18K.getOuter();
	    b2cInner = decoderGB18K.getInner();
	    index1 = decoderGB18K.getDBIndex1(); 
	    index2 = decoderGB18K.getDBIndex2(); 
	}

	private char getChar(int offset) {
	    int byte1 = (offset >>8) & 0xFF;
	    int byte2 = (offset & 0xFF);
	    int start = 0, end = 0xFF;

	    if (((byte1 < 0) || (byte1 > b2cOuter.length))
		 || ((byte2 < start) || (byte2 > end))) {
		       return REPLACE_CHAR;
	    }

	    int n = (b2cOuter[byte1] & 0xf) * (end - start + 1) + (byte2 - start);
	    return b2cInner[b2cOuter[byte1] >> 4].charAt(n);
	}

	protected char decodeDouble(int byte1, int byte2) {
	    int start = 0x40, end = 0xFE;
	    if (((byte1 < 0) || (byte1 > index1.length))
		|| ((byte2 < start) || (byte2 > end)))
		return (char)'\uFFFD';

	    int n = (index1[byte1] & 0xf) * (end - start + 1) + (byte2 - start);
	    return index2[index1[byte1] >> 4].charAt(n);
	}

	protected void implReset() {
	    currentState = GB18030_DOUBLE_BYTE;
	}

	private CoderResult decodeArrayLoop(ByteBuffer src,
					    CharBuffer dst)
	{
	    byte[] sa = src.array();
	    int sp = src.arrayOffset() + src.position();
	    int sl = src.arrayOffset() + src.limit();
	    assert (sp <= sl);
	    sp = (sp <= sl ? sp : sl);

	    char[] da = dst.array();
	    int dp = dst.arrayOffset() + dst.position();
	    int dl = dst.arrayOffset() + dst.limit();
	    assert (dp <= dl);
	    dp = (dp <= dl ? dp : dl);

	    int inputSize = 1;

	    try {
		while (sp < sl) {
		    int byte1 = 0 , byte2 = 0, byte3 = 0, byte4 = 0;
		    // Get the input byte
		    byte1 = sa[sp] & 0xFF;
		    inputSize = 1;

		    if ((byte1 & (byte)0x80) == 0){ // US-ASCII range
			currentState = GB18030_SINGLE_BYTE;
		    }
		    else if (byte1 < 0x81 || byte1 > 0xfe) {
			return CoderResult.malformedForLength(1);
		    }
		    else { // Either 2 or 4 byte sequence follows
		        if ( sl - sp < 2 )
			    return CoderResult.UNDERFLOW;
			byte2 = sa[sp + 1] & 0xFF;
			inputSize = 2;

			if (byte2 < 0x30)
				return CoderResult.malformedForLength(1);
			else if (byte2 >= 0x30 && byte2 <= 0x39) {
			    currentState = GB18030_FOUR_BYTE;

			    if (sl - sp < 4)
				return CoderResult.UNDERFLOW;

			    byte3 = sa[sp + 2] & 0xFF;
			    if (byte3 < 0x81 || byte3 > 0xfe)
				return CoderResult.malformedForLength(3);

			    byte4 = sa[sp + 3] & 0xFF; 
			    inputSize = 4;

			    if (byte4 < 0x30 || byte4 > 0x39)
				return CoderResult.malformedForLength(4);
			}
			else if (byte2 == 0x7f || byte2 == 0xff ||
				(byte2 < 0x40 )) {
			   return CoderResult.malformedForLength(2);
			}
			else 
			    currentState = GB18030_DOUBLE_BYTE;
		    }

		    if (dl - dp < 1)
			return CoderResult.OVERFLOW;
		    switch (currentState){
			case GB18030_SINGLE_BYTE:
			    da[dp++] = (char)byte1;
			    break;
			case GB18030_DOUBLE_BYTE:
			    da[dp++] = decodeDouble(byte1, byte2);
			    break;
			case GB18030_FOUR_BYTE:
			    int offset = (((byte1 - 0x81) * 10 +
					   (byte2 - 0x30)) * 126 +
					    byte3 - 0x81) * 10 + byte4 - 0x30;		
			    int hiByte = (offset >>8) & 0xFF;
			    int lowByte = (offset & 0xFF);

			// Mixture of table lookups and algorithmic calculation
			// of character values.

			// BMP Ranges
			if (offset <= 0x4A62)
			    da[dp++] = getChar(offset);
			else if (offset > 0x4A62 && offset <= 0x82BC)
			    da[dp++] = (char)(offset + 0x5543);
			else if (offset >= 0x82BD && offset <= 0x830D)
			    da[dp++] = getChar(offset);
			else if (offset >= 0x830D && offset <= 0x93A8)
			    da[dp++] = (char)(offset + 0x6557);
			else if (offset >= 0x93A9 && offset <= 0x99FB)
			    da[dp++] = getChar(offset);
			// Supplemental UCS planes handled via surrogates
			else if (offset >= 0x2E248 && offset < 0x12E248) {
			    if (offset >= 0x12E248)
				return CoderResult.malformedForLength(4);
			    offset -= 0x1e248;
			    if ( dl - dp < 2)
				return CoderResult.OVERFLOW;
			    // emit high + low surrogate
			    da[dp++] = (char)((offset - 0x10000) / 0x400 + 0xD800);
			    da[dp++] = (char)((offset - 0x10000) % 0x400 + 0xDC00);
			}
			else
			    return CoderResult.malformedForLength(inputSize);
			break;
		      }
		      sp += inputSize;
		}
		return CoderResult.UNDERFLOW;
	    } finally {
		src.position(sp - src.arrayOffset());
		dst.position(dp - dst.arrayOffset());
	    }
	}

	private CoderResult decodeBufferLoop(ByteBuffer src,
					    CharBuffer dst)
	{
	    int mark = src.position();
	    int inputSize = 1;

	    try {
		while (src.hasRemaining()) {
		    int byte1 = 0, byte2 = 0, byte3 = 0, byte4 = 0;
		    byte1 = src.get() & 0xFF;
		    inputSize = 1;

		    if ((byte1 & (byte)0x80) == 0){ // US-ASCII range
			currentState = GB18030_SINGLE_BYTE;
		    }
		    else { // Either 2 or 4 byte sequence follows
		        if ( src.remaining() < 1 )
			    return CoderResult.UNDERFLOW;
			byte2 = src.get() & 0xFF;
			inputSize = 2;

			if (byte2 < 0x30)
				return CoderResult.malformedForLength(1);
			else if (byte2 >= 0x30 && byte2 <= 0x39) {
			    currentState = GB18030_FOUR_BYTE;

			    if (src.remaining() < 2)
				return CoderResult.UNDERFLOW;

			    byte3 = src.get() & 0xFF;
			    if (byte3 < 0x81 || byte3 > 0xfe)
				return CoderResult.malformedForLength(3);

			    byte4 = src.get() & 0xFF;
			    inputSize = 4;

			    if (byte4 < 0x30 || byte4 > 0x39)
				return CoderResult.malformedForLength(4);
			}
			else if (byte2 == 0x7f || byte2 == 0xff ||
				(byte2 < 0x40 )) {
			   return CoderResult.malformedForLength(2);
			}
			else 
			    currentState = GB18030_DOUBLE_BYTE;
		    }

		    if (dst.remaining() < 1)
			return CoderResult.OVERFLOW;
		    switch (currentState){
			case GB18030_SINGLE_BYTE:
			    dst.put((char)byte1);
			    break;
			case GB18030_DOUBLE_BYTE:
			    dst.put(decodeDouble(byte1, byte2));
			    break;
			case GB18030_FOUR_BYTE:
			    int offset = (((byte1 - 0x81) * 10 +
					   (byte2 - 0x30)) * 126 +
					    byte3 - 0x81) * 10 + byte4 - 0x30;		
			    int hiByte = (offset >>8) & 0xFF;
			    int lowByte = (offset & 0xFF);

			// Mixture of table lookups and algorithmic calculation
			// of character values.

			// BMP Ranges
			if (offset <= 0x4A62)
			    dst.put(getChar(offset));
			else if (offset > 0x4A62 && offset <= 0x82BC)
			    dst.put((char)(offset + 0x5543));
			else if (offset >= 0x82BD && offset <= 0x830D)
			    dst.put(getChar(offset));
			else if (offset >= 0x830D && offset <= 0x93A8)
			    dst.put((char)(offset + 0x6557));
			else if (offset >= 0x93A9 && offset <= 0x99F9)
			    dst.put(getChar(offset));
			// Supplemental UCS planes handled via surrogates
			else if (offset >= 0x2E248 && offset < 0x12E248) {
			    if (offset >= 0x12E248)
				return CoderResult.malformedForLength(4);
			    offset -= 0x1e248;
			    if ( dst.remaining() < 2)
				return CoderResult.OVERFLOW;
			    // emit high + low surrogate
			    dst.put((char)((offset - 0x10000) / 0x400 + 0xD800));
			    dst.put((char)((offset - 0x10000) % 0x400 + 0xDC00));
			}
		    }
		    mark += inputSize;
	        }
		return CoderResult.UNDERFLOW;
	    } finally {
		src.position(mark);
	    }
	}


	protected CoderResult decodeLoop(ByteBuffer src,
					 CharBuffer dst)
	{
	    if (src.hasArray() && dst.hasArray())
		return decodeArrayLoop(src, dst);
	    else
		return decodeBufferLoop(src, dst);
	}
    }

    private static class Encoder extends CharsetEncoder {

        private int currentState = GB18030_DOUBLE_BYTE;
	short[] index1;
	String[] index2;

	private Encoder(Charset cs) {
	    super(cs, 4.0f, 4.0f); // max of 4 bytes per char
	    index1 = encoderGB18K.getIndex1();
	    index2 = encoderGB18K.getIndex2();
	}

	public boolean canEncode(char c) {
	    return (c != '\uFFFD');
	}

	private final Surrogate.Parser sgp = new Surrogate.Parser();

	private int getGB18030(short[] outerIndex, String[] innerIndex,
			       char ch) {
	    int offset = outerIndex[((ch & 0xff00) >> 8 )] << 8;
	    return innerIndex[offset >> 12].charAt((offset & 0xfff) +
			(ch & 0xff));
	}

	protected void implReset() {
	    currentState = GB18030_DOUBLE_BYTE;
        }

	private CoderResult encodeArrayLoop(CharBuffer src,
					    ByteBuffer dst)
	{
	    char[] sa = src.array();
	    int sp = src.arrayOffset() + src.position();
	    int sl = src.arrayOffset() + src.limit();
	    assert (sp <= sl);
	    sp = (sp <= sl ? sp : sl);
	    byte[] da = dst.array();
	    int dp = dst.arrayOffset() + dst.position();
	    int dl = dst.arrayOffset() + dst.limit();
	    assert (dp <= dl);
	    dp = (dp <= dl ? dp : dl);

	    int outputSize = 0;
	    int condensedKey = 0;  // expands to a four byte sequence
	    int hiByte = 0, loByte = 0;
	    currentState = GB18030_DOUBLE_BYTE;

	    try {
		while (sp < sl) {
		    char c = sa[sp];

		    if (Surrogate.is(c)) {
			if (sgp.parse(c, sa, sp, sl) < 0)
			    return sgp.error();
			condensedKey = ( sa[sp] - 0xD800) * 0x400 +
					  ( sa[sp+1] - 0xDC00) + 0x2E248;
			currentState = GB18030_FOUR_BYTE;
			sp += sgp.increment();
		    }
		    else if (c >= 0x0000 && c <= 0x007F) {
			currentState = GB18030_SINGLE_BYTE;
			if (dl - dp < 1)
			    return CoderResult.OVERFLOW;
			da[dp++] = (byte)c;
			sp++;
		    }
		    else if (c <= 0xA4C6 || c >= 0xE000) {
			int outByteVal = getGB18030(index1,
						    index2,
						    c);
			if (outByteVal == 0xFFFD )
				return CoderResult.unmappableForLength(1);

			hiByte = (outByteVal & 0xFF00) >> 8;
			loByte = (outByteVal & 0xFF);

			condensedKey = (hiByte - 0x20) * 256 + loByte;

			if (c >= 0xE000 && c < 0xF900)
				condensedKey += 0x82BD;
			else if (c >= 0xF900)
				condensedKey += 0x93A9;

			if (hiByte > 0x80)
			     currentState = GB18030_DOUBLE_BYTE;
			else
			     currentState = GB18030_FOUR_BYTE;
		    }
		    else if (c >= 0xA4C7 && c <= 0xD7FF) {
			condensedKey = c - 0x5543;
			currentState = GB18030_FOUR_BYTE;
		    }

		    if (currentState == GB18030_SINGLE_BYTE)
			continue;

		    if (currentState == GB18030_DOUBLE_BYTE) {
		        if (dl - dp < 2)
			    return CoderResult.OVERFLOW;
			da[dp++] = (byte)hiByte;
			da[dp++] = (byte)loByte;
			sp++;
		    }
		    else { // Four Byte encoding
			byte b1, b2, b3, b4;

			if (dl - dp < 4)
			    return CoderResult.OVERFLOW;
			// Decompose the condensed key into its 4 byte equivalent
			b4 = (byte)((condensedKey % 10) + 0x30);
			condensedKey /= 10;
			b3 = (byte)((condensedKey % 126) + 0x81);
			condensedKey /= 126;
			b2 = (byte)((condensedKey % 10) + 0x30);
			b1 = (byte)((condensedKey / 10) + 0x81);
			da[dp++] = b1;
			da[dp++] = b2;
			da[dp++] = b3;
			da[dp++] = b4;
			sp++;
		    }
		}
	    return CoderResult.UNDERFLOW;
	    } finally {
		src.position(sp - src.arrayOffset());
		dst.position(dp - dst.arrayOffset());
	    }
	}

	private CoderResult encodeBufferLoop(CharBuffer src,
					     ByteBuffer dst)
	{
	    int outputSize = 0;
	    int condensedKey = 0;
	    int hiByte = 0, loByte = 0;
	    currentState = GB18030_DOUBLE_BYTE;

	    int mark = src.position();
	    try {
		while (src.hasRemaining()) {
		    char c = src.get();

		    if (Surrogate.is(c)) {
			if ((condensedKey = sgp.parse(c, src))==0)
			   return sgp.error();
			condensedKey += 0x2e248;
			currentState = GB18030_FOUR_BYTE;
			mark += sgp.increment();
		    }
		    else if (c >= 0x0000 && c <= 0x007F) {
			currentState = GB18030_SINGLE_BYTE;
			if (dst.remaining() < 1)
			    return CoderResult.OVERFLOW;
			dst.put((byte)c);
			mark++;
		    }
		    else if (c <= 0xA4C6 || c >= 0xE000) {
			int outByteVal = getGB18030(index1,
						    index2,
						    c);
			if (outByteVal == 0xFFFD )
			    return CoderResult.unmappableForLength(1);

			hiByte = (outByteVal & 0xFF00) >> 8;
			loByte = (outByteVal & 0xFF);

			condensedKey = (hiByte - 0x20) * 256 + loByte;

			if (c >= 0xE000 && c < 0xF900)
				condensedKey += 0x82BD;
			else if (c >= 0xF900)
				condensedKey += 0x93A9;

			if (hiByte > 0x80)
			     currentState = GB18030_DOUBLE_BYTE;
			else
			     currentState = GB18030_FOUR_BYTE;
		    }
		    else if (c >= 0xA4C7 && c <= 0xD7FF) {
			condensedKey = c - 0x5543;
			currentState = GB18030_FOUR_BYTE;
		    }

		    if (currentState == GB18030_SINGLE_BYTE)
			continue;

		    if (currentState == GB18030_DOUBLE_BYTE) {
		        if (dst.remaining() < 2)
			    return CoderResult.OVERFLOW;
			dst.put((byte)hiByte);
			dst.put((byte)loByte);
			mark++;
		    }
		    else { // Four Byte encoding
			byte b1, b2, b3, b4;

			if (dst.remaining() < 4)
			    return CoderResult.OVERFLOW;
			// Decompose the condensed key into its 4 byte equivalent
			b4 = (byte)((condensedKey % 10) + 0x30);
			condensedKey /= 10;
			b3 = (byte)((condensedKey % 126) + 0x81);
			condensedKey /= 126;
			b2 = (byte)((condensedKey % 10) + 0x30);
			b1 = (byte)((condensedKey / 10) + 0x81);
			dst.put(b1);
			dst.put(b2);
			dst.put(b3);
			dst.put(b4);
			mark++;
		    }
		}
		return CoderResult.UNDERFLOW;
	    } finally {
		src.position(mark);
	    }
	}
	protected CoderResult encodeLoop(CharBuffer src,
					 ByteBuffer dst)
	{
	    if (src.hasArray() && dst.hasArray())
		return encodeArrayLoop(src, dst);
	    else
		return encodeBufferLoop(src, dst);
	}
    }
}

/*
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)ISCII91.java	1.4 03/01/23
 */


package sun.nio.cs.ext;

import java.nio.charset.Charset;
import java.nio.CharBuffer;
import java.nio.ByteBuffer;
import java.nio.charset.CharsetDecoder;
import java.nio.charset.CharsetEncoder;
import java.nio.charset.CoderResult;
import sun.nio.cs.Surrogate;
import sun.io.ByteToCharISCII91;
import sun.io.CharToByteISCII91;

public class ISCII91
    extends Charset
{
    private static final char NUKTA_CHAR = '\u093c';
    private static final char HALANT_CHAR = '\u094d';    

    public ISCII91() {
	super("x-ISCII91", ExtendedCharsets.aliasesFor("x-ISCII91"));
    }

    public boolean contains(Charset cs) {
	return ((cs.name().equals("US-ASCII"))
		|| (cs instanceof ISCII91));
    }

    public CharsetDecoder newDecoder() {
	return new Decoder(this);
    }

    public CharsetEncoder newEncoder() {
	return new Encoder(this);
    }

    private static class Decoder extends CharsetDecoder {

	private static ByteToCharISCII91 b2c = new ByteToCharISCII91();
	private static final char[] directMapTable = b2c.getISCIIDecoderMap();

	private static final char ZWNJ_CHAR = '\u200c';
	private static final char ZWJ_CHAR = '\u200d';
	private static final char INVALID_CHAR = '\uffff';        
	    
	private char contextChar = INVALID_CHAR;
	private boolean needFlushing = false;
    

	private Decoder(Charset cs) {
	    super(cs, 1.0f, 1.0f);
	}

	protected CoderResult implFlush(CharBuffer out) {
	    if(needFlushing) {
		if (out.remaining() < 1) {
		    return CoderResult.OVERFLOW;
		} else {
		    out.put(contextChar);
		}
	    }
	    contextChar = INVALID_CHAR;
	    needFlushing = false;
	    return CoderResult.UNDERFLOW;
	}

        /*Rules:
         * 1)ATR,EXT,following character to be replaced with '\ufffd'
         * 2)Halant + Halant => '\u094d' (Virama) + '\u200c'(ZWNJ)
         * 3)Halant + Nukta => '\u094d' (Virama) + '\u200d'(ZWJ)
	*/
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

	    try {
		while (sp < sl) {
		    int index = sa[sp];
		    index = ( index < 0 )? ( index + 255 ):index;
		    char currentChar = directMapTable[index];

		    // if the contextChar is either ATR || EXT
		    // set the output to '\ufffd'
		    if(contextChar == '\ufffd') {
			if (dl - dp < 1)
			    return CoderResult.OVERFLOW;
			da[dp++] = '\ufffd';
			contextChar = INVALID_CHAR;
			needFlushing = false;
			sp++;
			continue;
		    }

		    switch(currentChar) {
		    case '\u0901':
		    case '\u0907':
		    case '\u0908':        
		    case '\u090b':
		    case '\u093f':
		    case '\u0940':
		    case '\u0943':
		    case '\u0964':
			if(needFlushing) { 
			    if (dl - dp < 1)
				return CoderResult.OVERFLOW;
			    da[dp++] = contextChar;
			    contextChar = currentChar;
			    sp++;
			    continue;
			}
			contextChar = currentChar;
			needFlushing = true;
			sp++;
			continue;    
		    case NUKTA_CHAR:
			switch(contextChar) {
			case '\u0901':
			    da[dp] = '\u0950';
			    break;
			case '\u0907':
			    da[dp] = '\u090c';
			    break;
			case '\u0908':
			    da[dp] = '\u0961';
			    break;
			case '\u090b':
			    da[dp] = '\u0960';
			    break;
			case '\u093f':
			    da[dp] = '\u0962';
			    break;
			case '\u0940':
			    da[dp] = '\u0963';
			    break;
			case '\u0943':
			    da[dp] = '\u0944';
			    break;
			case '\u0964':
			    da[dp] = '\u093d';
			    break;
			case HALANT_CHAR:
			    if(needFlushing) { 
				da[dp] = contextChar;
				contextChar = currentChar;
				continue;
			    }
			    da[dp] = ZWJ_CHAR;
			    break;
			default:
			    if(needFlushing) { 
				da[dp++] = contextChar;
				contextChar = currentChar;
				continue;
			    }
			    da[dp] = NUKTA_CHAR;    
			}
			break;
		    case HALANT_CHAR:
			if(needFlushing) { 
			    if (dl - dp < 1)
				return CoderResult.OVERFLOW;
			    da[dp++] = contextChar;
			    contextChar = currentChar;
			    sp++;
			    continue;
			}
			if(contextChar == HALANT_CHAR) {
			    da[dp] = ZWNJ_CHAR;
			    break;
			}
			da[dp] = HALANT_CHAR;
			break;
		    case INVALID_CHAR:
			if(needFlushing) { 
			    if (dl - dp < 1)
				return CoderResult.OVERFLOW;
			    da[dp++] = contextChar;
			    contextChar = currentChar;
			    sp++;
			    continue;
			}
			return CoderResult.unmappableForLength(1);
		    default:
			if(needFlushing) { 
			    if (dl - dp < 1)
				return CoderResult.OVERFLOW;
			    da[dp++] = contextChar;
			    contextChar = currentChar;
			    sp++;
			    continue;
			}
			da[dp] = currentChar;
			break;
		    }//end switch

		contextChar = currentChar;                 
		needFlushing = false;
		dp++;
		sp++;
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

	    try {
		while (src.hasRemaining()) {
		    int index = src.get();
		    index = ( index < 0 )? ( index + 255 ):index;
		    char currentChar = directMapTable[index];

		    // if the contextChar is either ATR || EXT
		    // set the output to '\ufffd'
		    if(contextChar == '\ufffd') {
			if (dst.remaining() < 1)
			    return CoderResult.OVERFLOW;
			dst.put('\ufffd');
			contextChar = INVALID_CHAR;
			needFlushing = false;
			continue;
		    }

		    switch(currentChar) {
		    case '\u0901':
		    case '\u0907':
		    case '\u0908':        
		    case '\u090b':
		    case '\u093f':
		    case '\u0940':
		    case '\u0943':
		    case '\u0964':
			if(needFlushing) { 
			    if (dst.remaining() < 1)
				return CoderResult.OVERFLOW;
			    dst.put(contextChar);
			    contextChar = currentChar;
			    continue;
			}
			contextChar = currentChar;
			needFlushing = true;
			continue;    
		    case NUKTA_CHAR:
			switch(contextChar) {
			case '\u0901':
			    dst.put('\u0950');
			    break;
			case '\u0907':
			    dst.put('\u090c');
			    break;
			case '\u0908':
			    dst.put('\u0961');
			    break;
			case '\u090b':
			    dst.put('\u0960');
			    break;
			case '\u093f':
			    dst.put('\u0962');
			    break;
			case '\u0940':
			    dst.put('\u0963');
			    break;
			case '\u0943':
			    dst.put('\u0944');
			    break;
			case '\u0964':
			    dst.put('\u093d');
			    break;
			case HALANT_CHAR:
			    if(needFlushing) { 
				dst.put(contextChar);
				contextChar = currentChar;
				continue;
			    }
			    dst.put(ZWJ_CHAR);
			    break;
			default:
			    if(needFlushing) { 
				dst.put(contextChar);
				contextChar = currentChar;
				continue;
			    }
			    dst.put(NUKTA_CHAR);
			}
			break;
		    case HALANT_CHAR:
			if(needFlushing) { 
			    if (dst.remaining() < 1)
				return CoderResult.OVERFLOW;
			    dst.put(contextChar);
			    contextChar = currentChar;
			    continue;
			}
			if(contextChar == HALANT_CHAR) {
			    dst.put(ZWNJ_CHAR);
			    break;
			}
			dst.put(HALANT_CHAR);
			break;
		    case INVALID_CHAR:
			if(needFlushing) { 
			    if (dst.remaining() < 1)
				return CoderResult.OVERFLOW;
			    dst.put(contextChar);
			    contextChar = currentChar;
			    continue;
			}
			return CoderResult.unmappableForLength(1);
		    default:
			if(needFlushing) { 
			    if (dst.remaining() < 1)
				return CoderResult.OVERFLOW;
			    dst.put(contextChar);
			    contextChar = currentChar;
			    continue;
			}
			dst.put(currentChar);
			break;
		    }//end switch
		contextChar = currentChar;                 
		needFlushing = false;
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

        private static final byte NO_CHAR = (byte)255;

	private static CharToByteISCII91 c2b = new CharToByteISCII91();
        private static final byte[] directMapTable = c2b.getISCIIEncoderMap();

	private final Surrogate.Parser sgp = new Surrogate.Parser();

	private Encoder(Charset cs) {
	    super(cs, 2.0f, 2.0f);
	}

	public boolean canEncode(char ch) {
	    //check for Devanagari range,ZWJ,ZWNJ and ASCII range.
	    return ((ch >= 0x0900 && ch <= 0x097f) ||
		    (ch == 0x200d || ch == 0x200c)
		 || (ch >= 0x0000 && ch <= 0x007f));
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

	    try {
		char inputChar;
		while (sp < sl) {
		    int index = Integer.MIN_VALUE;
		    inputChar = sa[sp];

		    if (inputChar >= 0x0000 && inputChar <= 0x007f) {
			if (dl - dp < 1)
			    return CoderResult.OVERFLOW;
			da[dp++] = (byte) inputChar;
			sp++;
			continue;
		    }

		    // if inputChar == ZWJ replace it with halant
		    // if inputChar == ZWNJ replace it with Nukta

		    if (inputChar == 0x200c) {
			inputChar = HALANT_CHAR;
			//sp++;
		    }
		    else if (inputChar == 0x200d) {
			inputChar = NUKTA_CHAR;
			//sp++;
		    }

		    if (inputChar >= 0x0900 && inputChar <= 0x097f) {
			index = ((int)(inputChar) - 0x0900)*2;
			//sp++;
		    }

		    if (inputChar >= 0xd800 && inputChar < 0xdbff) {
			if (sgp.parse(inputChar, sa, sp, sl) < 0)
			    return sgp.error();
			return sgp.unmappableResult();
		    }

		    if (index == Integer.MIN_VALUE ||
			directMapTable[index] == NO_CHAR) {
			return CoderResult.unmappableForLength(1);
		    } else {
			da[dp++] = directMapTable[index++];
			if(directMapTable[index] != NO_CHAR) {
			    if(dl - dp < 1)
				return CoderResult.OVERFLOW;
				    
			}
			da[dp++] = directMapTable[index];
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
	    int mark = src.position();

	    try {
		char inputChar;
		while (src.hasRemaining()) {
		    int index = Integer.MIN_VALUE;
		    inputChar = src.get();

		    if (inputChar >= 0x0000 && inputChar <= 0x007f) {
			if (dst.remaining() < 1)
			    return CoderResult.OVERFLOW;
			dst.put((byte) inputChar);
			continue;
		    }

		    // if inputChar == ZWJ replace it with halant
		    // if inputChar == ZWNJ replace it with Nukta

		    if (inputChar == 0x200c) {
			inputChar = HALANT_CHAR;
		    }
		    else if (inputChar == 0x200d) {
			inputChar = NUKTA_CHAR;
		    }

		    if (inputChar >= 0x0900 && inputChar <= 0x097f) {
			index = ((int)(inputChar) - 0x0900)*2;
		    }

		    if (inputChar >= 0xd800 && inputChar < 0xdbff) {
		        if (sgp.parse(inputChar, src) < 0)
			    return sgp.error();
			return sgp.unmappableResult();
		    }

		    if (index == Integer.MIN_VALUE ||
			directMapTable[index] == NO_CHAR) {
			return CoderResult.unmappableForLength(1);
		    } else {
			dst.put(directMapTable[index++]);
			if(directMapTable[index] != NO_CHAR) {
			    if(dst.remaining() < 1)
				return CoderResult.OVERFLOW;
				    
			}
			dst.put(directMapTable[index]);
		    }
		    mark++;
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

/*
 * @(#)UnicodeDecoder.java	1.10 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.nio.cs;

import java.nio.ByteBuffer;
import java.nio.CharBuffer;
import java.nio.charset.Charset;
import java.nio.charset.CharsetDecoder;
import java.nio.charset.CoderResult;
import java.nio.charset.CharacterCodingException;
import java.nio.charset.MalformedInputException;


abstract class UnicodeDecoder extends CharsetDecoder {

    protected static final char BYTE_ORDER_MARK = (char) 0xfeff;
    protected static final char REVERSED_MARK = (char) 0xfffe;

    protected static final int NONE = 0;
    protected static final int BIG = 1;
    protected static final int LITTLE = 2;

    private final int expectedByteOrder; 
    private int currentByteOrder;

    public UnicodeDecoder(Charset cs, int bo) {
	super(cs, 0.5f, 1.0f); 
	expectedByteOrder = currentByteOrder = bo;
    }

    private final Surrogate.Generator sgg = new Surrogate.Generator();

    protected CoderResult decodeLoop(ByteBuffer src, CharBuffer dst) {
	int mark = src.position();

	try {
	    while (src.remaining() > 1) {
		int b1 = src.get() & 0xff;
		int b2 = src.get() & 0xff;

		// Byte Order Mark interpretation
		if (currentByteOrder == NONE) {
		    char c = (char)((b1 << 8) | b2);
		    if (c == BYTE_ORDER_MARK) {
			currentByteOrder = BIG;
			mark += 2;
			continue;
		    } else if (c == REVERSED_MARK) {
			currentByteOrder = LITTLE;
			mark += 2;
			continue;
		    } else {
			currentByteOrder = BIG;
			// FALL THROUGH to process b1, b2 normally
		    }
		}

		char c;
		if (currentByteOrder == BIG)
		    c = (char)((b1 << 8) | b2);
		else
		    c = (char)((b2 << 8) | b1);

		if (c == REVERSED_MARK) {
		    // A reversed BOM cannot occur within middle of stream
		    return CoderResult.malformedForLength(2);
		}
		if (!dst.hasRemaining())
		    return CoderResult.UNDERFLOW;
		mark += 2;
		dst.put(c);

		// ## Check for, and parse, surrogates
	    }
	    return CoderResult.UNDERFLOW;

	} finally {
	    src.position(mark);
	}
    }

    protected void implReset() {
	currentByteOrder = expectedByteOrder;
    }

}

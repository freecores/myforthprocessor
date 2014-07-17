/*
 * @(#)ByteToCharJISAutoDetect.java	1.16 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.io;

import java.io.UnsupportedEncodingException;

public class ByteToCharJISAutoDetect extends ByteToCharConverter {

    private final static int EUCJP_MASK = 0x01;
    private final static int SJIS2B_MASK = 0x02;
    private final static int SJIS1B_MASK = 0x04;
    private final static int EUCJP_KANA1_MASK = 0x08;
    private final static int EUCJP_KANA2_MASK = 0x10;

    private final static int SS2 = 0x8e;
    private final static int SS3 = 0x8f;

    // SJISName is set to either "SJIS" or "MS932"
    private String SJISName;
    private String EUCJPName;

    private String convName = null;
    private ByteToCharConverter detectedConv = null;
    private ByteToCharConverter defaultConv = null;

    public ByteToCharJISAutoDetect() {
	super();
	SJISName = CharacterEncoding.getSJISName();
	EUCJPName = CharacterEncoding.getEUCJPName();
        defaultConv = new ByteToCharISO8859_1();
        defaultConv.subChars = subChars;
        defaultConv.subMode = subMode;
    }

    public int flush(char [] output, int outStart, int outEnd)
	throws MalformedInputException, ConversionBufferFullException
    {
        badInputLength = 0;
        if(detectedConv != null)
             return detectedConv.flush(output, outStart, outEnd);
        else
             return defaultConv.flush(output, outStart, outEnd);
    }


    /**
     * Character conversion
     */
    public int convert(byte[] input, int inOff, int inEnd,
		       char[] output, int outOff, int outEnd)
        throws UnknownCharacterException, MalformedInputException,
	       ConversionBufferFullException
    {
        int num = 0;

	charOff = outOff;
	byteOff = inOff;

	try {
	    if (detectedConv == null) {
		int euckana = 0;
		int ss2count = 0;
		int firstmask = 0;
		int secondmask = 0;
		int cnt;
		boolean nonAsciiFound = false;

		for (cnt = inOff; cnt < inEnd; cnt++) {
		    firstmask = 0;
		    secondmask = 0;
		    int byte1 = input[cnt]&0xff;
		    int byte2;

		    // TODO: should check valid escape sequences!
		    if (byte1 == 0x1b) {
			convName = "ISO2022JP";
			break;
		    }

		    // Try to convert all leading ASCII characters.
		    if ((nonAsciiFound == false) && (byte1 < 0x80)) {
			if (charOff >= outEnd)
			    throw new ConversionBufferFullException();
			output[charOff++] = (char) byte1;
			byteOff++;
			num++;
			continue;
		    }

		    // We can no longer convert ASCII.
		    nonAsciiFound = true;

		    firstmask = maskTable1[byte1];
		    if (byte1 == SS2)
			ss2count++;

		    if (firstmask != 0) {
			if (cnt+1 < inEnd) {
			    byte2 = input[++cnt] & 0xff;
			    secondmask = maskTable2[byte2];
			    int mask = firstmask & secondmask;
			    if (mask == EUCJP_MASK) {
				convName = EUCJPName;
				break;
			    }
			    if ((mask == SJIS2B_MASK) || (mask == SJIS1B_MASK)
				|| (canBeSJIS1B(firstmask) && secondmask == 0)) {
				convName = SJISName;
				break;
			    }

			    // If the first byte is a SS3 and the third byte
			    // is not an EUC byte, it should be SJIS.
			    // Otherwise, we can't determine it yet, but it's
			    // very likely SJIS. So we don't take the EUCJP CS3
			    // character boundary. If we tried both
			    // possibilities here, it might be able to be
			    // determined correctly.
			    if ((byte1 == SS3) && canBeEUCJP(secondmask)) {
				if (cnt+1 < inEnd) {
				    int nextbyte = input[cnt+1] & 0xff;
				    if (! canBeEUCJP(maskTable2[nextbyte]))
					convName = SJISName;
				} else
				    convName = SJISName;
			    }
			    if (canBeEUCKana(firstmask, secondmask))
				euckana++;
			} else {
			    if ((firstmask & SJIS1B_MASK) != 0) {
				convName = SJISName;
				break;
			    }
			}
		    }
		}

		if (nonAsciiFound && (convName == null)) {
		    if ((euckana > 1) || (ss2count > 1))
			convName = EUCJPName;
		    else
			convName = SJISName;
		}

		if (convName != null) {
		    try {
			detectedConv = ByteToCharConverter.getConverter(convName);
			detectedConv.subChars = subChars;
			detectedConv.subMode = subMode;
		    } catch (UnsupportedEncodingException e){
			detectedConv = null;
			convName = null;
		    }
		}
	    }
	} catch (ConversionBufferFullException bufferFullException) {
		throw bufferFullException;
	} catch (Exception e) {
	    // If we fail to detect the converter needed for any reason,
	    // use the default converter.
	    detectedConv = defaultConv;
	}

	// If we've converted all ASCII characters, then return.
	if (byteOff == inEnd) { 
	    return num;
	}

        if(detectedConv != null) {
	    try {
		num += detectedConv.convert(input, inOff + num, inEnd,
					    output, outOff + num, outEnd);
	    } finally {
		charOff = detectedConv.nextCharIndex();
		byteOff = detectedConv.nextByteIndex();
		badInputLength = detectedConv.badInputLength;
	    }
        } else {
            try {
                num += defaultConv.convert(input, inOff + num, inEnd,
					   output, outOff + num, outEnd);
	    } finally {
                charOff = defaultConv.nextCharIndex();
                byteOff = defaultConv.nextByteIndex();
                badInputLength = defaultConv.badInputLength;
            }
        }
        return num;
    }

    public void reset() {
        if(detectedConv != null) {
	     detectedConv.reset();
	     detectedConv = null;
	     convName = null;
        } else
             defaultConv.reset();
	charOff = byteOff = 0;
    }

    public String getCharacterEncoding() {
	return "JISAutoDetect";
    }

    public String toString() {
	String s = getCharacterEncoding();
	if (detectedConv != null) {
	    s += "[" + detectedConv.getCharacterEncoding() + "]";
	} else {
	    s += "[unknown]";
	}
	return s;
    }

    private final static boolean canBeSJIS1B(int mask) {
	return (mask & SJIS1B_MASK) != 0;
    }

    private final static boolean canBeEUCJP(int mask) {
	return (mask & EUCJP_MASK) != 0;
    }

    private final static boolean canBeEUCKana(int mask1, int mask2) {
	return ((mask1 & EUCJP_KANA1_MASK) != 0)
	       && ((mask2 & EUCJP_KANA2_MASK) != 0);
    }

    // Mask tables - each entry indicates possibility of first or
    // second byte being SJIS or EUC_JP
    private static final byte maskTable1[] = {
	0, 0, 0, 0,	// 0x00 - 0x03
	0, 0, 0, 0,	// 0x04 - 0x07
	0, 0, 0, 0,	// 0x08 - 0x0b
	0, 0, 0, 0,	// 0x0c - 0x0f
	0, 0, 0, 0,	// 0x10 - 0x13
	0, 0, 0, 0,	// 0x14 - 0x17
	0, 0, 0, 0,	// 0x18 - 0x1b
	0, 0, 0, 0,	// 0x1c - 0x1f
	0, 0, 0, 0,	// 0x20 - 0x23
	0, 0, 0, 0,	// 0x24 - 0x27
	0, 0, 0, 0,	// 0x28 - 0x2b
	0, 0, 0, 0,	// 0x2c - 0x2f
	0, 0, 0, 0,	// 0x30 - 0x33
	0, 0, 0, 0,	// 0x34 - 0x37
	0, 0, 0, 0,	// 0x38 - 0x3b
	0, 0, 0, 0,	// 0x3c - 0x3f
	0, 0, 0, 0,	// 0x40 - 0x43
	0, 0, 0, 0,	// 0x44 - 0x47
	0, 0, 0, 0,	// 0x48 - 0x4b
	0, 0, 0, 0,	// 0x4c - 0x4f
	0, 0, 0, 0,	// 0x50 - 0x53
	0, 0, 0, 0,	// 0x54 - 0x57
	0, 0, 0, 0,	// 0x58 - 0x5b
	0, 0, 0, 0,	// 0x5c - 0x5f
	0, 0, 0, 0,	// 0x60 - 0x63
	0, 0, 0, 0,	// 0x64 - 0x67
	0, 0, 0, 0,	// 0x68 - 0x6b
	0, 0, 0, 0,	// 0x6c - 0x6f
	0, 0, 0, 0,	// 0x70 - 0x73
	0, 0, 0, 0,	// 0x74 - 0x77
	0, 0, 0, 0,	// 0x78 - 0x7b
	0, 0, 0, 0,	// 0x7c - 0x7f
	0, SJIS2B_MASK, SJIS2B_MASK, SJIS2B_MASK,	// 0x80 - 0x83
	SJIS2B_MASK, SJIS2B_MASK, SJIS2B_MASK, SJIS2B_MASK,	// 0x84 - 0x87
	SJIS2B_MASK, SJIS2B_MASK, SJIS2B_MASK, SJIS2B_MASK,	// 0x88 - 0x8b
	SJIS2B_MASK, SJIS2B_MASK, SJIS2B_MASK|EUCJP_MASK, SJIS2B_MASK|EUCJP_MASK,	// 0x8c - 0x8f
	SJIS2B_MASK, SJIS2B_MASK, SJIS2B_MASK, SJIS2B_MASK,	// 0x90 - 0x93
	SJIS2B_MASK, SJIS2B_MASK, SJIS2B_MASK, SJIS2B_MASK,	// 0x94 - 0x97
	SJIS2B_MASK, SJIS2B_MASK, SJIS2B_MASK, SJIS2B_MASK,	// 0x98 - 0x9b
	SJIS2B_MASK, SJIS2B_MASK, SJIS2B_MASK, SJIS2B_MASK,	// 0x9c - 0x9f
	0, SJIS1B_MASK|EUCJP_MASK, SJIS1B_MASK|EUCJP_MASK, SJIS1B_MASK|EUCJP_MASK,	// 0xa0 - 0xa3
	SJIS1B_MASK|EUCJP_MASK|EUCJP_KANA1_MASK, SJIS1B_MASK|EUCJP_MASK, SJIS1B_MASK|EUCJP_MASK, SJIS1B_MASK|EUCJP_MASK,	// 0xa4 - 0xa7
	SJIS1B_MASK|EUCJP_MASK, SJIS1B_MASK|EUCJP_MASK, SJIS1B_MASK|EUCJP_MASK, SJIS1B_MASK|EUCJP_MASK,	// 0xa8 - 0xab
	SJIS1B_MASK|EUCJP_MASK, SJIS1B_MASK|EUCJP_MASK, SJIS1B_MASK|EUCJP_MASK, SJIS1B_MASK|EUCJP_MASK,	// 0xac - 0xaf
	SJIS1B_MASK|EUCJP_MASK, SJIS1B_MASK|EUCJP_MASK, SJIS1B_MASK|EUCJP_MASK, SJIS1B_MASK|EUCJP_MASK,	// 0xb0 - 0xb3
	SJIS1B_MASK|EUCJP_MASK, SJIS1B_MASK|EUCJP_MASK, SJIS1B_MASK|EUCJP_MASK, SJIS1B_MASK|EUCJP_MASK,	// 0xb4 - 0xb7
	SJIS1B_MASK|EUCJP_MASK, SJIS1B_MASK|EUCJP_MASK, SJIS1B_MASK|EUCJP_MASK, SJIS1B_MASK|EUCJP_MASK,	// 0xb8 - 0xbb
	SJIS1B_MASK|EUCJP_MASK, SJIS1B_MASK|EUCJP_MASK, SJIS1B_MASK|EUCJP_MASK, SJIS1B_MASK|EUCJP_MASK,	// 0xbc - 0xbf
	SJIS1B_MASK|EUCJP_MASK, SJIS1B_MASK|EUCJP_MASK, SJIS1B_MASK|EUCJP_MASK, SJIS1B_MASK|EUCJP_MASK,	// 0xc0 - 0xc3
	SJIS1B_MASK|EUCJP_MASK, SJIS1B_MASK|EUCJP_MASK, SJIS1B_MASK|EUCJP_MASK, SJIS1B_MASK|EUCJP_MASK,	// 0xc4 - 0xc7
	SJIS1B_MASK|EUCJP_MASK, SJIS1B_MASK|EUCJP_MASK, SJIS1B_MASK|EUCJP_MASK, SJIS1B_MASK|EUCJP_MASK,	// 0xc8 - 0xcb
	SJIS1B_MASK|EUCJP_MASK, SJIS1B_MASK|EUCJP_MASK, SJIS1B_MASK|EUCJP_MASK, SJIS1B_MASK|EUCJP_MASK,	// 0xcc - 0xcf
	SJIS1B_MASK|EUCJP_MASK, SJIS1B_MASK|EUCJP_MASK, SJIS1B_MASK|EUCJP_MASK, SJIS1B_MASK|EUCJP_MASK,	// 0xd0 - 0xd3
	SJIS1B_MASK|EUCJP_MASK, SJIS1B_MASK|EUCJP_MASK, SJIS1B_MASK|EUCJP_MASK, SJIS1B_MASK|EUCJP_MASK,	// 0xd4 - 0xd7
	SJIS1B_MASK|EUCJP_MASK, SJIS1B_MASK|EUCJP_MASK, SJIS1B_MASK|EUCJP_MASK, SJIS1B_MASK|EUCJP_MASK,	// 0xd8 - 0xdb
	SJIS1B_MASK|EUCJP_MASK, SJIS1B_MASK|EUCJP_MASK, SJIS1B_MASK|EUCJP_MASK, SJIS1B_MASK|EUCJP_MASK,	// 0xdc - 0xdf
	SJIS2B_MASK|EUCJP_MASK, SJIS2B_MASK|EUCJP_MASK, SJIS2B_MASK|EUCJP_MASK, SJIS2B_MASK|EUCJP_MASK,	// 0xe0 - 0xe3
	SJIS2B_MASK|EUCJP_MASK, SJIS2B_MASK|EUCJP_MASK, SJIS2B_MASK|EUCJP_MASK, SJIS2B_MASK|EUCJP_MASK,	// 0xe4 - 0xe7
	SJIS2B_MASK|EUCJP_MASK, SJIS2B_MASK|EUCJP_MASK, SJIS2B_MASK|EUCJP_MASK, SJIS2B_MASK|EUCJP_MASK,	// 0xe8 - 0xeb
	SJIS2B_MASK|EUCJP_MASK, SJIS2B_MASK|EUCJP_MASK, SJIS2B_MASK|EUCJP_MASK, SJIS2B_MASK|EUCJP_MASK,	// 0xec - 0xef
	SJIS2B_MASK|EUCJP_MASK, SJIS2B_MASK|EUCJP_MASK, SJIS2B_MASK|EUCJP_MASK, SJIS2B_MASK|EUCJP_MASK,	// 0xf0 - 0xf3
	SJIS2B_MASK|EUCJP_MASK, SJIS2B_MASK|EUCJP_MASK, SJIS2B_MASK|EUCJP_MASK, SJIS2B_MASK|EUCJP_MASK,	// 0xf4 - 0xf7
	SJIS2B_MASK|EUCJP_MASK, SJIS2B_MASK|EUCJP_MASK, SJIS2B_MASK|EUCJP_MASK, SJIS2B_MASK|EUCJP_MASK,	// 0xf8 - 0xfb
	SJIS2B_MASK|EUCJP_MASK, EUCJP_MASK, EUCJP_MASK, 0	// 0xfc - 0xff
    };

    private static final byte maskTable2[] = {
	0, 0, 0, 0,	// 0x00 - 0x03
	0, 0, 0, 0,	// 0x04 - 0x07
	0, 0, 0, 0,	// 0x08 - 0x0b
	0, 0, 0, 0,	// 0x0c - 0x0f
	0, 0, 0, 0,	// 0x10 - 0x13
	0, 0, 0, 0,	// 0x14 - 0x17
	0, 0, 0, 0,	// 0x18 - 0x1b
	0, 0, 0, 0,	// 0x1c - 0x1f
	0, 0, 0, 0,	// 0x20 - 0x23
	0, 0, 0, 0,	// 0x24 - 0x27
	0, 0, 0, 0,	// 0x28 - 0x2b
	0, 0, 0, 0,	// 0x2c - 0x2f
	0, 0, 0, 0,	// 0x30 - 0x33
	0, 0, 0, 0,	// 0x34 - 0x37
	0, 0, 0, 0,	// 0x38 - 0x3b
	0, 0, 0, 0,	// 0x3c - 0x3f
	SJIS2B_MASK, SJIS2B_MASK, SJIS2B_MASK, SJIS2B_MASK,	// 0x40 - 0x43
	SJIS2B_MASK, SJIS2B_MASK, SJIS2B_MASK, SJIS2B_MASK,	// 0x44 - 0x47
	SJIS2B_MASK, SJIS2B_MASK, SJIS2B_MASK, SJIS2B_MASK,	// 0x48 - 0x4b
	SJIS2B_MASK, SJIS2B_MASK, SJIS2B_MASK, SJIS2B_MASK,	// 0x4c - 0x4f
	SJIS2B_MASK, SJIS2B_MASK, SJIS2B_MASK, SJIS2B_MASK,	// 0x50 - 0x53
	SJIS2B_MASK, SJIS2B_MASK, SJIS2B_MASK, SJIS2B_MASK,	// 0x54 - 0x57
	SJIS2B_MASK, SJIS2B_MASK, SJIS2B_MASK, SJIS2B_MASK,	// 0x58 - 0x5b
	SJIS2B_MASK, SJIS2B_MASK, SJIS2B_MASK, SJIS2B_MASK,	// 0x5c - 0x5f
	SJIS2B_MASK, SJIS2B_MASK, SJIS2B_MASK, SJIS2B_MASK,	// 0x60 - 0x63
	SJIS2B_MASK, SJIS2B_MASK, SJIS2B_MASK, SJIS2B_MASK,	// 0x64 - 0x67
	SJIS2B_MASK, SJIS2B_MASK, SJIS2B_MASK, SJIS2B_MASK,	// 0x68 - 0x6b
	SJIS2B_MASK, SJIS2B_MASK, SJIS2B_MASK, SJIS2B_MASK,	// 0x6c - 0x6f
	SJIS2B_MASK, SJIS2B_MASK, SJIS2B_MASK, SJIS2B_MASK,	// 0x70 - 0x73
	SJIS2B_MASK, SJIS2B_MASK, SJIS2B_MASK, SJIS2B_MASK,	// 0x74 - 0x77
	SJIS2B_MASK, SJIS2B_MASK, SJIS2B_MASK, SJIS2B_MASK,	// 0x78 - 0x7b
	SJIS2B_MASK, SJIS2B_MASK, SJIS2B_MASK, 0,	// 0x7c - 0x7f
	SJIS2B_MASK, SJIS2B_MASK, SJIS2B_MASK, SJIS2B_MASK,	// 0x80 - 0x83
	SJIS2B_MASK, SJIS2B_MASK, SJIS2B_MASK, SJIS2B_MASK,	// 0x84 - 0x87
	SJIS2B_MASK, SJIS2B_MASK, SJIS2B_MASK, SJIS2B_MASK,	// 0x88 - 0x8b
	SJIS2B_MASK, SJIS2B_MASK, SJIS2B_MASK, SJIS2B_MASK,	// 0x8c - 0x8f
	SJIS2B_MASK, SJIS2B_MASK, SJIS2B_MASK, SJIS2B_MASK,	// 0x90 - 0x93
	SJIS2B_MASK, SJIS2B_MASK, SJIS2B_MASK, SJIS2B_MASK,	// 0x94 - 0x97
	SJIS2B_MASK, SJIS2B_MASK, SJIS2B_MASK, SJIS2B_MASK,	// 0x98 - 0x9b
	SJIS2B_MASK, SJIS2B_MASK, SJIS2B_MASK, SJIS2B_MASK,	// 0x9c - 0x9f
	SJIS2B_MASK, SJIS1B_MASK|SJIS2B_MASK|EUCJP_MASK|EUCJP_KANA2_MASK, SJIS1B_MASK|SJIS2B_MASK|EUCJP_MASK|EUCJP_KANA2_MASK, SJIS1B_MASK|SJIS2B_MASK|EUCJP_MASK|EUCJP_KANA2_MASK,	// 0xa0 - 0xa3
	SJIS1B_MASK|SJIS2B_MASK|EUCJP_MASK|EUCJP_KANA2_MASK, SJIS1B_MASK|SJIS2B_MASK|EUCJP_MASK|EUCJP_KANA2_MASK, SJIS1B_MASK|SJIS2B_MASK|EUCJP_MASK|EUCJP_KANA2_MASK, SJIS1B_MASK|SJIS2B_MASK|EUCJP_MASK|EUCJP_KANA2_MASK,	// 0xa4 - 0xa7
	SJIS1B_MASK|SJIS2B_MASK|EUCJP_MASK|EUCJP_KANA2_MASK, SJIS1B_MASK|SJIS2B_MASK|EUCJP_MASK|EUCJP_KANA2_MASK, SJIS1B_MASK|SJIS2B_MASK|EUCJP_MASK|EUCJP_KANA2_MASK, SJIS1B_MASK|SJIS2B_MASK|EUCJP_MASK|EUCJP_KANA2_MASK,	// 0xa8 - 0xab
	SJIS1B_MASK|SJIS2B_MASK|EUCJP_MASK|EUCJP_KANA2_MASK, SJIS1B_MASK|SJIS2B_MASK|EUCJP_MASK|EUCJP_KANA2_MASK, SJIS1B_MASK|SJIS2B_MASK|EUCJP_MASK|EUCJP_KANA2_MASK, SJIS1B_MASK|SJIS2B_MASK|EUCJP_MASK|EUCJP_KANA2_MASK,	// 0xac - 0xaf
	SJIS1B_MASK|SJIS2B_MASK|EUCJP_MASK|EUCJP_KANA2_MASK, SJIS1B_MASK|SJIS2B_MASK|EUCJP_MASK|EUCJP_KANA2_MASK, SJIS1B_MASK|SJIS2B_MASK|EUCJP_MASK|EUCJP_KANA2_MASK, SJIS1B_MASK|SJIS2B_MASK|EUCJP_MASK|EUCJP_KANA2_MASK,	// 0xb0 - 0xb3
	SJIS1B_MASK|SJIS2B_MASK|EUCJP_MASK|EUCJP_KANA2_MASK, SJIS1B_MASK|SJIS2B_MASK|EUCJP_MASK|EUCJP_KANA2_MASK, SJIS1B_MASK|SJIS2B_MASK|EUCJP_MASK|EUCJP_KANA2_MASK, SJIS1B_MASK|SJIS2B_MASK|EUCJP_MASK|EUCJP_KANA2_MASK,	// 0xb4 - 0xb7
	SJIS1B_MASK|SJIS2B_MASK|EUCJP_MASK|EUCJP_KANA2_MASK, SJIS1B_MASK|SJIS2B_MASK|EUCJP_MASK|EUCJP_KANA2_MASK, SJIS1B_MASK|SJIS2B_MASK|EUCJP_MASK|EUCJP_KANA2_MASK, SJIS1B_MASK|SJIS2B_MASK|EUCJP_MASK|EUCJP_KANA2_MASK,	// 0xb8 - 0xbb
	SJIS1B_MASK|SJIS2B_MASK|EUCJP_MASK|EUCJP_KANA2_MASK, SJIS1B_MASK|SJIS2B_MASK|EUCJP_MASK|EUCJP_KANA2_MASK, SJIS1B_MASK|SJIS2B_MASK|EUCJP_MASK|EUCJP_KANA2_MASK, SJIS1B_MASK|SJIS2B_MASK|EUCJP_MASK|EUCJP_KANA2_MASK,	// 0xbc - 0xbf
	SJIS1B_MASK|SJIS2B_MASK|EUCJP_MASK|EUCJP_KANA2_MASK, SJIS1B_MASK|SJIS2B_MASK|EUCJP_MASK|EUCJP_KANA2_MASK, SJIS1B_MASK|SJIS2B_MASK|EUCJP_MASK|EUCJP_KANA2_MASK, SJIS1B_MASK|SJIS2B_MASK|EUCJP_MASK|EUCJP_KANA2_MASK,	// 0xc0 - 0xc3
	SJIS1B_MASK|SJIS2B_MASK|EUCJP_MASK|EUCJP_KANA2_MASK, SJIS1B_MASK|SJIS2B_MASK|EUCJP_MASK|EUCJP_KANA2_MASK, SJIS1B_MASK|SJIS2B_MASK|EUCJP_MASK|EUCJP_KANA2_MASK, SJIS1B_MASK|SJIS2B_MASK|EUCJP_MASK|EUCJP_KANA2_MASK,	// 0xc4 - 0xc7
	SJIS1B_MASK|SJIS2B_MASK|EUCJP_MASK|EUCJP_KANA2_MASK, SJIS1B_MASK|SJIS2B_MASK|EUCJP_MASK|EUCJP_KANA2_MASK, SJIS1B_MASK|SJIS2B_MASK|EUCJP_MASK|EUCJP_KANA2_MASK, SJIS1B_MASK|SJIS2B_MASK|EUCJP_MASK|EUCJP_KANA2_MASK,	// 0xc8 - 0xcb
	SJIS1B_MASK|SJIS2B_MASK|EUCJP_MASK|EUCJP_KANA2_MASK, SJIS1B_MASK|SJIS2B_MASK|EUCJP_MASK|EUCJP_KANA2_MASK, SJIS1B_MASK|SJIS2B_MASK|EUCJP_MASK|EUCJP_KANA2_MASK, SJIS1B_MASK|SJIS2B_MASK|EUCJP_MASK|EUCJP_KANA2_MASK,	// 0xcc - 0xcf
	SJIS1B_MASK|SJIS2B_MASK|EUCJP_MASK|EUCJP_KANA2_MASK, SJIS1B_MASK|SJIS2B_MASK|EUCJP_MASK|EUCJP_KANA2_MASK, SJIS1B_MASK|SJIS2B_MASK|EUCJP_MASK|EUCJP_KANA2_MASK, SJIS1B_MASK|SJIS2B_MASK|EUCJP_MASK|EUCJP_KANA2_MASK,	// 0xd0 - 0xd3
	SJIS1B_MASK|SJIS2B_MASK|EUCJP_MASK|EUCJP_KANA2_MASK, SJIS1B_MASK|SJIS2B_MASK|EUCJP_MASK|EUCJP_KANA2_MASK, SJIS1B_MASK|SJIS2B_MASK|EUCJP_MASK|EUCJP_KANA2_MASK, SJIS1B_MASK|SJIS2B_MASK|EUCJP_MASK|EUCJP_KANA2_MASK,	// 0xd4 - 0xd7
	SJIS1B_MASK|SJIS2B_MASK|EUCJP_MASK|EUCJP_KANA2_MASK, SJIS1B_MASK|SJIS2B_MASK|EUCJP_MASK|EUCJP_KANA2_MASK, SJIS1B_MASK|SJIS2B_MASK|EUCJP_MASK|EUCJP_KANA2_MASK, SJIS1B_MASK|SJIS2B_MASK|EUCJP_MASK|EUCJP_KANA2_MASK,	// 0xd8 - 0xdb
	SJIS1B_MASK|SJIS2B_MASK|EUCJP_MASK|EUCJP_KANA2_MASK, SJIS1B_MASK|SJIS2B_MASK|EUCJP_MASK|EUCJP_KANA2_MASK, SJIS1B_MASK|SJIS2B_MASK|EUCJP_MASK|EUCJP_KANA2_MASK, SJIS1B_MASK|SJIS2B_MASK|EUCJP_MASK|EUCJP_KANA2_MASK,	// 0xdc - 0xdf
	SJIS2B_MASK|EUCJP_MASK|EUCJP_KANA2_MASK, SJIS2B_MASK|EUCJP_MASK|EUCJP_KANA2_MASK, SJIS2B_MASK|EUCJP_MASK|EUCJP_KANA2_MASK, SJIS2B_MASK|EUCJP_MASK|EUCJP_KANA2_MASK,	// 0xe0 - 0xe3
	SJIS2B_MASK|EUCJP_MASK|EUCJP_KANA2_MASK, SJIS2B_MASK|EUCJP_MASK|EUCJP_KANA2_MASK, SJIS2B_MASK|EUCJP_MASK|EUCJP_KANA2_MASK, SJIS2B_MASK|EUCJP_MASK|EUCJP_KANA2_MASK,	// 0xe4 - 0xe7
	SJIS2B_MASK|EUCJP_MASK|EUCJP_KANA2_MASK, SJIS2B_MASK|EUCJP_MASK|EUCJP_KANA2_MASK, SJIS2B_MASK|EUCJP_MASK|EUCJP_KANA2_MASK, SJIS2B_MASK|EUCJP_MASK|EUCJP_KANA2_MASK,	// 0xe8 - 0xeb
	SJIS2B_MASK|EUCJP_MASK|EUCJP_KANA2_MASK, SJIS2B_MASK|EUCJP_MASK|EUCJP_KANA2_MASK, SJIS2B_MASK|EUCJP_MASK|EUCJP_KANA2_MASK, SJIS2B_MASK|EUCJP_MASK|EUCJP_KANA2_MASK,	// 0xec - 0xef
	SJIS2B_MASK|EUCJP_MASK|EUCJP_KANA2_MASK, SJIS2B_MASK|EUCJP_MASK|EUCJP_KANA2_MASK, SJIS2B_MASK|EUCJP_MASK|EUCJP_KANA2_MASK, SJIS2B_MASK|EUCJP_MASK|EUCJP_KANA2_MASK,	// 0xf0 - 0xf3
	SJIS2B_MASK|EUCJP_MASK, SJIS2B_MASK|EUCJP_MASK, SJIS2B_MASK|EUCJP_MASK, SJIS2B_MASK|EUCJP_MASK,	// 0xf4 - 0xf7
	SJIS2B_MASK|EUCJP_MASK, SJIS2B_MASK|EUCJP_MASK, SJIS2B_MASK|EUCJP_MASK, SJIS2B_MASK|EUCJP_MASK,	// 0xf8 - 0xfb
	SJIS2B_MASK|EUCJP_MASK, EUCJP_MASK, EUCJP_MASK, 0	// 0xfc - 0xff
    };
}

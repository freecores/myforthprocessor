package sun.io;

/* @(#)CharToByteISCII91.java	1.4 02/04/20
 *
 * Copyright (c) 1998 International Business Machines.
 * All Rights Reserved.
 *
 * Author : Sunanda Bera, C. Thirumalesh
 * Last Modified : 11,December,1998
 *
 * Purpose : Defines class CharToByteISCII91.
 *
 *
 * Revision History
 * ======== =======
 *
 * Date        By            Description
 * ----        --            -----------
 * March 29, 1999 John Raley Removed MalformedInputException; modified substitution logic
 *
 */

/**
 * Converter class. Converts between ISCII91 encoding and Unicode encoding.
 * ISCII91 is the character encoding as defined in Indian Standard document
 * IS 13194:1991 ( Indian Script Code for Information Interchange ).
 *
 * @see sun.io.CharToByteConverter
 */

/*
 * {jbr} I am not sure this class adheres to code converter conventions.
 * Need to investigate.
 * Might should recode as a subclass of CharToByteSingleByte.
 */

public class CharToByteISCII91 extends CharToByteConverter {

        private static final byte NO_CHAR = (byte)255;

	public static byte[] getISCIIEncoderMap() {
	    return directMapTable;
	}

        private static final byte[] directMapTable = {
        NO_CHAR,NO_CHAR, //0900 <reserved>
        (byte)161,NO_CHAR, //0901 -- DEVANAGARI SIGN CANDRABINDU = anunasika
        (byte)162,NO_CHAR, //0902 -- DEVANAGARI SIGN ANUSVARA = bindu
        (byte)163,NO_CHAR, //0903 -- DEVANAGARI SIGN VISARGA
        NO_CHAR,NO_CHAR, //0904 <reserved>
        (byte)164,NO_CHAR, //0905 -- DEVANAGARI LETTER A
        (byte)165,NO_CHAR, //0906 -- DEVANAGARI LETTER AA
        (byte)166,NO_CHAR, //0907 -- DEVANAGARI LETTER I
        (byte)167,NO_CHAR, //0908 -- DEVANAGARI LETTER II
        (byte)168,NO_CHAR, //0909 -- DEVANAGARI LETTER U
        (byte)169,NO_CHAR, //090a -- DEVANAGARI LETTER UU
        (byte)170,NO_CHAR, //090b -- DEVANAGARI LETTER VOCALIC R
        (byte)166,(byte)233, //090c -- DEVANAGARI LETTER VOVALIC L
        (byte)174,NO_CHAR, //090d -- DEVANAGARI LETTER CANDRA E
        (byte)171,NO_CHAR, //090e -- DEVANAGARI LETTER SHORT E
        (byte)172,NO_CHAR, //090f -- DEVANAGARI LETTER E
        (byte)173,NO_CHAR, //0910 -- DEVANAGARI LETTER AI
        (byte)178,NO_CHAR, //0911 -- DEVANAGARI LETTER CANDRA O
        (byte)175,NO_CHAR, //0912 -- DEVANAGARI LETTER SHORT O
        (byte)176,NO_CHAR, //0913 -- DEVANAGARI LETTER O
        (byte)177,NO_CHAR, //0914 -- DEVANAGARI LETTER AU
        (byte)179,NO_CHAR, //0915 -- DEVANAGARI LETTER KA
        (byte)180,NO_CHAR, //0916 -- DEVANAGARI LETTER KHA
        (byte)181,NO_CHAR, //0917 -- DEVANAGARI LETTER GA
        (byte)182,NO_CHAR, //0918 -- DEVANAGARI LETTER GHA
        (byte)183,NO_CHAR, //0919 -- DEVANAGARI LETTER NGA
        (byte)184,NO_CHAR, //091a -- DEVANAGARI LETTER CA
        (byte)185,NO_CHAR, //091b -- DEVANAGARI LETTER CHA
        (byte)186,NO_CHAR, //091c -- DEVANAGARI LETTER JA
        (byte)187,NO_CHAR, //091d -- DEVANAGARI LETTER JHA
        (byte)188,NO_CHAR, //091e -- DEVANAGARI LETTER NYA
        (byte)189,NO_CHAR, //091f -- DEVANAGARI LETTER TTA
        (byte)190,NO_CHAR, //0920 -- DEVANAGARI LETTER TTHA
        (byte)191,NO_CHAR, //0921 -- DEVANAGARI LETTER DDA
        (byte)192,NO_CHAR, //0922 -- DEVANAGARI LETTER DDHA
        (byte)193,NO_CHAR, //0923 -- DEVANAGARI LETTER NNA
        (byte)194,NO_CHAR, //0924 -- DEVANAGARI LETTER TA
        (byte)195,NO_CHAR, //0925 -- DEVANAGARI LETTER THA
        (byte)196,NO_CHAR, //0926 -- DEVANAGARI LETTER DA
        (byte)197,NO_CHAR, //0927 -- DEVANAGARI LETTER DHA
        (byte)198,NO_CHAR, //0928 -- DEVANAGARI LETTER NA
        (byte)199,NO_CHAR, //0929 -- DEVANAGARI LETTER NNNA <=> 0928 + 093C
        (byte)200,NO_CHAR, //092a -- DEVANAGARI LETTER PA
        (byte)201,NO_CHAR, //092b -- DEVANAGARI LETTER PHA
        (byte)202,NO_CHAR, //092c -- DEVANAGARI LETTER BA
        (byte)203,NO_CHAR, //092d -- DEVANAGARI LETTER BHA
        (byte)204,NO_CHAR, //092e -- DEVANAGARI LETTER MA
        (byte)205,NO_CHAR, //092f -- DEVANAGARI LETTER YA
        (byte)207,NO_CHAR, //0930 -- DEVANAGARI LETTER RA
        (byte)208,NO_CHAR, //0931 -- DEVANAGARI LETTER RRA <=> 0930 + 093C
        (byte)209,NO_CHAR, //0932 -- DEVANAGARI LETTER LA
        (byte)210,NO_CHAR, //0933 -- DEVANAGARI LETTER LLA
        (byte)211,NO_CHAR, //0934 -- DEVANAGARI LETTER LLLA <=> 0933 + 093C
        (byte)212,NO_CHAR, //0935 -- DEVANAGARI LETTER VA
        (byte)213,NO_CHAR, //0936 -- DEVANAGARI LETTER SHA
        (byte)214,NO_CHAR, //0937 -- DEVANAGARI LETTER SSA
        (byte)215,NO_CHAR, //0938 -- DEVANAGARI LETTER SA
        (byte)216,NO_CHAR, //0939 -- DEVANAGARI LETTER HA
        NO_CHAR,NO_CHAR, //093a <reserved>
        NO_CHAR,NO_CHAR, //093b <reserved>
        (byte)233,NO_CHAR, //093c -- DEVANAGARI SIGN NUKTA
        (byte)234,(byte)233, //093d -- DEVANAGARI SIGN AVAGRAHA
        (byte)218,NO_CHAR, //093e -- DEVANAGARI VOWEL SIGN AA
        (byte)219,NO_CHAR, //093f -- DEVANAGARI VOWEL SIGN I
        (byte)220,NO_CHAR, //0940 -- DEVANAGARI VOWEL SIGN II
        (byte)221,NO_CHAR, //0941 -- DEVANAGARI VOWEL SIGN U
        (byte)222,NO_CHAR, //0942 -- DEVANAGARI VOWEL SIGN UU
        (byte)223,NO_CHAR, //0943 -- DEVANAGARI VOWEL SIGN VOCALIC R
        (byte)223,(byte)233, //0944 -- DEVANAGARI VOWEL SIGN VOCALIC RR
        (byte)227,NO_CHAR, //0945 -- DEVANAGARI VOWEL SIGN CANDRA E
        (byte)224,NO_CHAR, //0946 -- DEVANAGARI VOWEL SIGN SHORT E
        (byte)225,NO_CHAR, //0947 -- DEVANAGARI VOWEL SIGN E
        (byte)226,NO_CHAR, //0948 -- DEVANAGARI VOWEL SIGN AI
        (byte)231,NO_CHAR, //0949 -- DEVANAGARI VOWEL SIGN CANDRA O
        (byte)228,NO_CHAR, //094a -- DEVANAGARI VOWEL SIGN SHORT O
        (byte)229,NO_CHAR, //094b -- DEVANAGARI VOWEL SIGN O
        (byte)230,NO_CHAR, //094c -- DEVANAGARI VOWEL SIGN AU
        (byte)232,NO_CHAR, //094d -- DEVANAGARI SIGN VIRAMA ( halant )
        NO_CHAR,NO_CHAR, //094e <reserved>
        NO_CHAR,NO_CHAR, //094f <reserved>
        (byte)161,(byte)233, //0950 -- DEVANAGARI OM
        (byte)240,(byte)181, //0951 -- DEVANAGARI STRESS SIGN UDATTA
        (byte)240,(byte)184, //0952 -- DEVANAGARI STRESS SIGN ANUDATTA
        (byte)254,NO_CHAR, //0953 -- DEVANAGARI GRAVE ACCENT || MISSING
        (byte)254,NO_CHAR, //0954 -- DEVANAGARI ACUTE ACCENT || MISSING
        NO_CHAR,NO_CHAR, //0955 <reserved>
        NO_CHAR,NO_CHAR, //0956 <reserved>
        NO_CHAR,NO_CHAR, //0957 <reserved>
        (byte)179,(byte)233, //0958 -- DEVANAGARI LETTER QA <=> 0915 + 093C
        (byte)180,(byte)233, //0959 -- DEVANAGARI LETTER KHHA <=> 0916 + 093C
        (byte)181,(byte)233, //095a -- DEVANAGARI LETTER GHHA <=> 0917 + 093C
        (byte)186,(byte)233, //095b -- DEVANAGARI LETTER ZA <=> 091C + 093C
        (byte)191,(byte)233, //095c -- DEVANAGARI LETTER DDDHA <=> 0921 + 093C
        (byte)192,(byte)233, //095d -- DEVANAGARI LETTER RHA <=> 0922 + 093C
        (byte)201,(byte)233, //095e -- DEVANAGARI LETTER FA <=> 092B + 093C
        (byte)206,NO_CHAR, //095f -- DEVANAGARI LETTER YYA <=> 092F + 093C
        (byte)170,(byte)233, //0960 -- DEVANAGARI LETTER VOCALIC RR
        (byte)167,(byte)233, //0961 -- DEVANAGARI LETTER VOCALIC LL
        (byte)219,(byte)233, //0962 -- DEVANAGARI VOWEL SIGN VOCALIC L
        (byte)220,(byte)233, //0963 -- DEVANAGARI VOWEL SIGN VOCALIC LL
        (byte)234,NO_CHAR, //0964 -- DEVANAGARI DANDA ( phrase separator )
        (byte)234,(byte)234, //0965 -- DEVANAGARI DOUBLE DANDA
        (byte)241,NO_CHAR, //0966 -- DEVANAGARI DIGIT ZERO
        (byte)242,NO_CHAR, //0967 -- DEVANAGARI DIGIT ONE
        (byte)243,NO_CHAR, //0968 -- DEVANAGARI DIGIT TWO
        (byte)244,NO_CHAR, //0969 -- DEVANAGARI DIGIT THREE
        (byte)245,NO_CHAR, //096a -- DEVANAGARI DIGIT FOUR
        (byte)246,NO_CHAR, //096b -- DEVANAGARI DIGIT FIVE
        (byte)247,NO_CHAR, //096c -- DEVANAGARI DIGIT SIX
        (byte)248,NO_CHAR, //096d -- DEVANAGARI DIGIT SEVEN
        (byte)249,NO_CHAR, //096e -- DEVANAGARI DIGIT EIGHT
        (byte)250,NO_CHAR, //096f -- DEVANAGARI DIGIT NINE
        (byte)240,(byte)191,  //0970 -- DEVANAGARI ABBREVIATION SIGN
        NO_CHAR,NO_CHAR, //0971 -- reserved
        NO_CHAR,NO_CHAR, //0972 -- reserved
        NO_CHAR,NO_CHAR, //0973 -- reserved
        NO_CHAR,NO_CHAR, //0974 -- reserved
        NO_CHAR,NO_CHAR, //0975 -- reserved
        NO_CHAR,NO_CHAR, //0976 -- reserved
        NO_CHAR,NO_CHAR, //0977 -- reserved
        NO_CHAR,NO_CHAR, //0978 -- reserved
        NO_CHAR,NO_CHAR, //0979 -- reserved
        NO_CHAR,NO_CHAR, //097a -- reserved
        NO_CHAR,NO_CHAR, //097b -- reserved
        NO_CHAR,NO_CHAR, //097c -- reserved
        NO_CHAR,NO_CHAR, //097d -- reserved
        NO_CHAR,NO_CHAR, //097e -- reserved
        NO_CHAR,NO_CHAR  //097f -- reserved
        }; //end of table definition

        private static final char NUKTA_CHAR = '\u093c';
        private static final char HALANT_CHAR = '\u094d';


/**
 * @return true for Devanagari and ASCII range and for the special characters
 *              Zero Width Joiner and Zero Width Non-Joiner
 * @see sun.io.CharToByteConverter#canConvert
 *
 */
        public boolean canConvert(char ch) {
        //check for Devanagari range,ZWJ,ZWNJ and ASCII range.
        return ((ch >= 0x0900 && ch <= 0x097f) || (ch == 0x200d || ch == 0x200c)
                                || (ch >= 0x0000 && ch <= 0x007f) );
        } //canConvert()
/**
 * Converts both Devanagari and ASCII range of characters.
 * @see sun.io.CharToByteConverter#convert
 */
    public int convert(char[] input, int inStart, int inEnd, byte[] output, int outStart, int outEnd) throws MalformedInputException, UnknownCharacterException, ConversionBufferFullException {

        charOff = inStart;
        byteOff = outStart;

        for (;charOff < inEnd; charOff++) {

            char inputChar = input[charOff];
            int index = Integer.MIN_VALUE;
            boolean isSurrogatePair = false;

            //check if input is in ASCII RANGE
            if (inputChar >= 0x0000 && inputChar <= 0x007f) {
                if (byteOff >= outEnd) {
                        throw new ConversionBufferFullException();
                }
                output[byteOff++] = (byte) inputChar;
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

            // If input char is a high surrogate, ensure that the following
            // char is a low surrogate.  If not, throw a MalformedInputException.
            // Leave index untouched so substitution or an UnknownCharacterException
            // will result.
            else if (inputChar >= 0xd800 && inputChar <= 0xdbff) {
                if (charOff < inEnd-1) {
                    char nextChar = input[charOff];
                    if (nextChar >= 0xdc00 && nextChar <= 0xdfff) {
                        charOff++;
                        isSurrogatePair = true;
                    }
                }
                if (!isSurrogatePair) {
                    badInputLength = 1;
                    throw new MalformedInputException();
                }
            }
            else if (inputChar >= 0xdc00 && inputChar <= 0xdfff) {
                badInputLength = 1;
                throw new MalformedInputException();
            }

            if (index == Integer.MIN_VALUE || directMapTable[index] == NO_CHAR) {
                if (subMode) {
                    if (byteOff + subBytes.length >= outEnd) {
                            throw new ConversionBufferFullException();
                    }
                    System.arraycopy(subBytes, 0, output, byteOff, subBytes.length);
                    byteOff += subBytes.length;
                } else {
                    badInputLength = isSurrogatePair? 2 : 1;
                    throw new UnknownCharacterException();
                }
            }
            else {
                if(byteOff >= outEnd) {
                    throw new ConversionBufferFullException();
                }
                output[byteOff++] = directMapTable[index++];
                if(directMapTable[index] != NO_CHAR) {
                    if(byteOff >= outEnd) {
                            throw new ConversionBufferFullException();
                    }
                    output[byteOff++] = directMapTable[index];
                }
            }

        } //end for

        return byteOff - outStart;
    } //end of routine convert.

/**
* @see sun.io.CharToByteConverter#flush
*/
        public int flush( byte[] output, int outStart, int outEnd )
        throws MalformedInputException, ConversionBufferFullException {
        byteOff = charOff = 0;
        return 0;
        }//flush()
/**
 * @return The character encoding as a String.
 */
        public String getCharacterEncoding() {
        return "ISCII91";
        }//getCharacterEncoding
/**
 * @see sun.io.CharToByteConverter#getMaxBytesPerChar
 */
        public int getMaxBytesPerChar() {
        return 2;
        }//getMaxBytesPerChar()
/**
 * @see sun.io.CharToByteConverter#reset
 */
        public void reset() {
        byteOff = charOff = 0;
        }
} //end of class definition

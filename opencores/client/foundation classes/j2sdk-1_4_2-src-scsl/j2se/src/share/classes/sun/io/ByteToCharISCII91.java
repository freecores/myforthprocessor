package sun.io;

/* @(#)ByteToCharISCII91.java	1.3 02/04/20
 *
 * Copyright (c) 1998 International Business Machines.
 * All Rights Reserved.
 *
 * Author : Sunanda Bera, C. Thirumalesh
 * Last Modified : 23,November,1998
 *
 * Purpose : Defines class ByteToCharISCII91.
 *
 *
 * Revision History 
 * ======== ======= 
 *
 * Date        By            Description
 * ----        --            -----------
 *
 *
 */

/**
 * Converter class. Converts between Unicode encoding and ISCII91 encoding.
 * ISCII91 is the character encoding as defined in Indian Standard document
 * IS 13194:1991 ( Indian Script Code for Information Interchange ).
 *
 * @see sun.io.ByteToCharConverter 
 */
public class ByteToCharISCII91 extends ByteToCharConverter {

    public static char[] getISCIIDecoderMap() {
	return directMapTable;
    }

    private static final char[] directMapTable = {
        '\u0000', // ascii character
        '\u0001', // ascii character
        '\u0002', // ascii character
        '\u0003', // ascii character
        '\u0004', // ascii character
        '\u0005', // ascii character
        '\u0006', // ascii character
        '\u0007', // ascii character
        '\u0008', // ascii character
        '\u0009', // ascii character
        '\012', // ascii character
        '\u000b', // ascii character
        '\u000c', // ascii character
        '\015', // ascii character
        '\u000e', // ascii character
        '\u000f', // ascii character
        '\u0010', // ascii character
        '\u0011', // ascii character
        '\u0012', // ascii character
        '\u0013', // ascii character
        '\u0014', // ascii character
        '\u0015', // ascii character
        '\u0016', // ascii character
        '\u0017', // ascii character
        '\u0018', // ascii character
        '\u0019', // ascii character
        '\u001a', // ascii character
        '\u001b', // ascii character
        '\u001c', // ascii character
        '\u001d', // ascii character
        '\u001e', // ascii character
        '\u001f', // ascii character
        '\u0020', // ascii character
        '\u0021', // ascii character
        '\u0022', // ascii character
        '\u0023', // ascii character
        '\u0024', // ascii character
        '\u0025', // ascii character
        '\u0026', // ascii character
        (char)0x0027, // '\u0027' control -- ascii character
        '\u0028', // ascii character
        '\u0029', // ascii character
        '\u002a', // ascii character
        '\u002b', // ascii character
        '\u002c', // ascii character
        '\u002d', // ascii character
        '\u002e', // ascii character
        '\u002f', // ascii character
        '\u0030', // ascii character
        '\u0031', // ascii character
        '\u0032', // ascii character
        '\u0033', // ascii character
        '\u0034', // ascii character
        '\u0035', // ascii character
        '\u0036', // ascii character
        '\u0037', // ascii character
        '\u0038', // ascii character
        '\u0039', // ascii character
        '\u003a', // ascii character
        '\u003b', // ascii character
        '\u003c', // ascii character
        '\u003d', // ascii character
        '\u003e', // ascii character
        '\u003f', // ascii character
        '\u0040', // ascii character
        '\u0041', // ascii character
        '\u0042', // ascii character
        '\u0043', // ascii character
        '\u0044', // ascii character
        '\u0045', // ascii character
        '\u0046', // ascii character
        '\u0047', // ascii character
        '\u0048', // ascii character
        '\u0049', // ascii character
        '\u004a', // ascii character
        '\u004b', // ascii character
        '\u004c', // ascii character
        '\u004d', // ascii character
        '\u004e', // ascii character
        '\u004f', // ascii character
        '\u0050', // ascii character
        '\u0051', // ascii character
        '\u0052', // ascii character
        '\u0053', // ascii character
        '\u0054', // ascii character
        '\u0055', // ascii character
        '\u0056', // ascii character
        '\u0057', // ascii character
        '\u0058', // ascii character
        '\u0059', // ascii character
        '\u005a', // ascii character
        '\u005b', // ascii character
        '\\',// '\u005c' -- ascii character
        '\u005d', // ascii character
        '\u005e', // ascii character
        '\u005f', // ascii character
        '\u0060', // ascii character
        '\u0061', // ascii character
        '\u0062', // ascii character
        '\u0063', // ascii character
        '\u0064', // ascii character
        '\u0065', // ascii character
        '\u0066', // ascii character
        '\u0067', // ascii character
        '\u0068', // ascii character
        '\u0069', // ascii character
        '\u006a', // ascii character
        '\u006b', // ascii character
        '\u006c', // ascii character
        '\u006d', // ascii character
        '\u006e', // ascii character
        '\u006f', // ascii character
        '\u0070', // ascii character
        '\u0071', // ascii character
        '\u0072', // ascii character
        '\u0073', // ascii character
        '\u0074', // ascii character
        '\u0075', // ascii character
        '\u0076', // ascii character
        '\u0077', // ascii character
        '\u0078', // ascii character
        '\u0079', // ascii character
        '\u007a', // ascii character
        '\u007b', // ascii character
        '\u007c', // ascii character
        '\u007d', // ascii character
        '\u007e', // ascii character
        '\u007f', // ascii character
        '\uffff', // unknown character
        '\uffff', // unknown character
        '\uffff', // unknown character
        '\uffff', // unknown character
        '\uffff', // unknown character
        '\uffff', // unknown character
        '\uffff', // unknown character
        '\uffff', // unknown character
        '\uffff', // unknown character
        '\uffff', // unknown character
        '\uffff', // unknown character
        '\uffff', // unknown character
        '\uffff', // unknown character
        '\uffff', // unknown character
        '\uffff', // unknown character
        '\uffff', // unknown character
        '\uffff', // unknown character
        '\uffff', // unknown character
        '\uffff', // unknown character
        '\uffff', // unknown character
        '\uffff', // unknown character
        '\uffff', // unknown character
        '\uffff', // unknown character
        '\uffff', // unknown character
        '\uffff', // unknown character
        '\uffff', // unknown character
        '\uffff', // unknown character
        '\uffff', // unknown character
        '\uffff', // unknown character
        '\uffff', // unknown character
        '\uffff', // unknown character
        '\uffff', // unknown character
        '\u0901', // a1 -- Vowel-modifier CHANDRABINDU
        '\u0902', // a2 -- Vowel-modifier ANUSWAR
        '\u0903', // a3 -- Vowel-modifier VISARG
        
        '\u0905', // a4 -- Vowel A
        '\u0906', // a5 -- Vowel AA
        '\u0907', // a6 -- Vowel I
        '\u0908', // a7 -- Vowel II
        '\u0909', // a8 -- Vowel U
        '\u090a', // a9 -- Vowel UU
        '\u090b', // aa -- Vowel RI
        '\u090e', // ab -- Vowel E ( Southern Scripts )
        '\u090f', // ac -- Vowel EY
        '\u0910', // ad -- Vowel AI
        '\u090d', // ae -- Vowel AYE ( Devanagari Script )
        '\u0912', // af -- Vowel O ( Southern Scripts )
        '\u0913', // b0 -- Vowel OW 
        '\u0914', // b1 -- Vowel AU
        '\u0911', // b2 -- Vowel AWE ( Devanagari Script )
        '\u0915', // b3 -- Consonant KA
        '\u0916', // b4 -- Consonant KHA
        '\u0917', // b5 -- Consonant GA
        '\u0918', // b6 -- Consonant GHA
        '\u0919', // b7 -- Consonant NGA
        '\u091a', // b8 -- Consonant CHA
        '\u091b', // b9 -- Consonant CHHA
        '\u091c', // ba -- Consonant JA
        '\u091d', // bb -- Consonant JHA
        '\u091e', // bc -- Consonant JNA
        '\u091f', // bd -- Consonant Hard TA
        '\u0920', // be -- Consonant Hard THA
        '\u0921', // bf -- Consonant Hard DA
        '\u0922', // c0 -- Consonant Hard DHA
        '\u0923', // c1 -- Consonant Hard NA
        '\u0924', // c2 -- Consonant Soft TA
        '\u0925', // c3 -- Consonant Soft THA
        '\u0926', // c4 -- Consonant Soft DA
        '\u0927', // c5 -- Consonant Soft DHA
        '\u0928', // c6 -- Consonant Soft NA
        '\u0929', // c7 -- Consonant NA ( Tamil )
        '\u092a', // c8 -- Consonant PA
        '\u092b', // c9 -- Consonant PHA
        '\u092c', // ca -- Consonant BA
        '\u092d', // cb -- Consonant BHA
        '\u092e', // cc -- Consonant MA
        '\u092f', // cd -- Consonant YA
        '\u095f', // ce -- Consonant JYA ( Bengali, Assamese & Oriya )
        '\u0930', // cf -- Consonant RA
        '\u0931', // d0 -- Consonant Hard RA ( Southern Scripts )
        '\u0932', // d1 -- Consonant LA
        '\u0933', // d2 -- Consonant Hard LA
        '\u0934', // d3 -- Consonant ZHA ( Tamil & Malayalam )
        '\u0935', // d4 -- Consonant VA
        '\u0936', // d5 -- Consonant SHA
        '\u0937', // d6 -- Consonant Hard SHA
        '\u0938', // d7 -- Consonant SA
        '\u0939', // d8 -- Consonant HA

        '\u200d', // d9 -- Consonant INVISIBLE
        '\u093e', // da -- Vowel Sign AA

        '\u093f', // db -- Vowel Sign I
        '\u0940', // dc -- Vowel Sign II
        '\u0941', // dd -- Vowel Sign U
        '\u0942', // de -- Vowel Sign UU
        '\u0943', // df -- Vowel Sign RI
        '\u0946', // e0 -- Vowel Sign E ( Southern Scripts )
        '\u0947', // e1 -- Vowel Sign EY
        '\u0948', // e2 -- Vowel Sign AI
        '\u0945', // e3 -- Vowel Sign AYE ( Devanagari Script )
        '\u094a', // e4 -- Vowel Sign O ( Southern Scripts )
        '\u094b', // e5 -- Vowel Sign OW
        '\u094c', // e6 -- Vowel Sign AU
        '\u0949', // e7 -- Vowel Sign AWE ( Devanagari Script )

        '\u094d', // e8 -- Vowel Omission Sign ( Halant )
        '\u093c', // e9 -- Diacritic Sign ( Nukta )
        '\u0964', // ea -- Full Stop ( Viram, Northern Scripts )

        '\uffff', // eb -- This position shall not be used
        '\uffff', // ec -- This position shall not be used
        '\uffff', // ed -- This position shall not be used
        '\uffff', // ee -- This position shall not be used

        '\ufffd', // ef -- Attribute Code ( ATR )
        '\ufffd', // f0 -- Extension Code ( EXT )

        '\u0966', // f1 -- Digit 0
        '\u0967', // f2 -- Digit 1
        '\u0968', // f3 -- Digit 2
        '\u0969', // f4 -- Digit 3
        '\u096a', // f5 -- Digit 4
        '\u096b', // f6 -- Digit 5
        '\u096c', // f7 -- Digit 6
        '\u096d', // f8 -- Digit 7
        '\u096e', // f9 -- Digit 8
        '\u096f', // fa -- Digit 9

        '\uffff', // fb -- This position shall not be used
        '\uffff', // fc -- This position shall not be used
        '\uffff', // fd -- This position shall not be used
        '\uffff', // fe -- This position shall not be used
        '\uffff'  // ff -- This position shall not be used
    }; //end of table definition

    private static final char NUKTA_CHAR = '\u093c';
    private static final char HALANT_CHAR = '\u094d';    
    private static final char ZWNJ_CHAR = '\u200c';
    private static final char ZWJ_CHAR = '\u200d';
    private static final char INVALID_CHAR = '\uffff';        
        
    private char contextChar = INVALID_CHAR;
    private boolean needFlushing = false;
    
/**
 * Converts ISCII91 characters to Unicode.
 * @see sun.io.ByteToCharConverter#convert
 */
    public int convert(byte input[], int inStart, int inEnd, 
                        char output[], int outStart, int outEnd) 
    throws ConversionBufferFullException, UnknownCharacterException {
        /*Rules:
         * 1)ATR,EXT,following character to be replaced with '\ufffd'
         * 2)Halant + Halant => '\u094d' (Virama) + '\u200c'(ZWNJ)
         * 3)Halant + Nukta => '\u094d' (Virama) + '\u200d'(ZWJ)
         */
        charOff = outStart;
        byteOff = inStart;
        while (byteOff < inEnd) {
            if (charOff >= outEnd) {
                throw new ConversionBufferFullException();
            }
            int index = input[byteOff++];
            index = ( index < 0 )? ( index + 255 ):index;
            char currentChar = directMapTable[index];

            // if the contextChar is either ATR || EXT set the output to '\ufffd'
            if(contextChar == '\ufffd') {
                output[charOff++] = '\ufffd';
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
                    output[charOff++] = contextChar;
                    contextChar = currentChar;
                    continue;
                }
                contextChar = currentChar;
                needFlushing = true;
                continue;    
            case NUKTA_CHAR:
                switch(contextChar) {
                case '\u0901':
                    output[charOff] = '\u0950';
                    break;
                case '\u0907':
                    output[charOff] = '\u090c';
                    break;
                case '\u0908':
                    output[charOff] = '\u0961';
                    break;
                case '\u090b':
                    output[charOff] = '\u0960';
                    break;
                case '\u093f':
                    output[charOff] = '\u0962';
                    break;
                case '\u0940':
                    output[charOff] = '\u0963';
                    break;
                case '\u0943':
                    output[charOff] = '\u0944';
                    break;
                case '\u0964':
                    output[charOff] = '\u093d';
                    break;
                case HALANT_CHAR:
                    if(needFlushing) { 
                        output[charOff++] = contextChar;
                        contextChar = currentChar;
                        continue;
                    }
                    output[charOff] = ZWJ_CHAR;
                    break;
                default:
                    if(needFlushing) { 
                        output[charOff++] = contextChar;
                        contextChar = currentChar;
                        continue;
                    }
                    output[charOff] = NUKTA_CHAR;    
                }
                break;
            case HALANT_CHAR:
                if(needFlushing) { 
                    output[charOff++] = contextChar;
                    contextChar = currentChar;
                    continue;
                }
                if(contextChar == HALANT_CHAR) {
                    output[charOff] = ZWNJ_CHAR;
                    break;
                }
                output[charOff] = HALANT_CHAR;
                break;
            case INVALID_CHAR:
                if(needFlushing) { 
                    output[charOff++] = contextChar;
                    contextChar = currentChar;
                    continue;
                }
                if(subMode) {
                    output[charOff] = subChars[0];
                    break;
                } else {
                    contextChar = INVALID_CHAR;
                    throw new UnknownCharacterException();
                }
            default:
                if(needFlushing) { 
                    output[charOff++] = contextChar;
                    contextChar = currentChar;
                    continue;
                }
                output[charOff] = currentChar;
                break;
        }//end switch
        
        contextChar = currentChar;                 
        needFlushing = false;
        charOff++;
        }//end while
        return charOff - outStart;
    } //convert()

/**
 * @see sun.io.ByteToCharConverter#flush
 */
    public  int flush( char[] output, int outStart, int outEnd )
    throws MalformedInputException, ConversionBufferFullException
    {
        int charsWritten = 0;
        //if the last char was not flushed, flush it!
        if(needFlushing) {
            output[outStart] = contextChar;
            charsWritten = 1;
        }
        contextChar = INVALID_CHAR;
        needFlushing = false;
        byteOff = charOff = 0;
        return charsWritten;
    }//flush()
/**
 * Returns the character set id for the conversion.
 */
    public String getCharacterEncoding()
    {
        return "ISCII91";
    }//getCharacterEncoding()
/**
 * @see sun.io.ByteToCharConverter#reset
 */
    public void reset()
    {
        byteOff = charOff = 0;
    }//reset()

}//end of class definition

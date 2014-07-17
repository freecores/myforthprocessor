/*
 * @(#)cmaps.cpp	1.39 03/02/19
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @author Alex D. Gelfenbain
 * @author Xueming Shen
 */

#include "fontObject.h"
#include "cmaps.h"
#include "swaps.h"

#include <stdlib.h>
#include <ctype.h>
#include <jni.h>
#include <jni_util.h>
#include <jvm.h>

#include <stdio.h>

extern "C" {
//- convert a Unicode character to a font-specific glyph
static tt_uint32 ConvertUnicodeToGlyph(sfntFileFontObject& font, byte * &cmap, Unicode theChar);

    static tt_uint16 *GetConverter(tt_uint16 );

    JavaVM *jvm;

    JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved)
    {
        jvm = vm;
        return JNI_VERSION_1_2;
    }

#define HANDLE_EXCEPTION()            \
    if (env->ExceptionCheck()) {   \
        env->ExceptionDescribe();     \
        env->ExceptionClear();        \
        if (retArray)                 \
            free(retArray);           \
        if (tmpBytes)                 \
            free(tmpBytes);           \
        env->PopLocalFrame (0);       \
        return NULL;                  \
    }

    UInt16 *makeNativeToUnicodeArray(int encodingID) {
        jclass      cls = NULL;
        jobject     obj;
        jmethodID   mid;
        jbyteArray  jInputBytes;
        jcharArray  jOutputChars;
        int         dBegin = 0x8000;
        int         dEnd = 0xffff;
        UInt8       *tmpBytes = NULL;
        UInt16      *retArray = NULL;
        jint        ret;
        JNIEnv      *env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);
        int         i,j;

        env->PushLocalFrame (16);

        //speed up the converting by setting the range for double
        //byte characters;
        switch (encodingID) {
            case 0:     // Symbol
                break;
            case 2:      //sjis
                dBegin = 0x8140;
                dEnd   = 0xfcfc;
                cls = env->FindClass("sun/io/ByteToCharSJIS");
                break;
            case 3:     //prc
                dBegin = 0x8140;
                dEnd   = 0xfea0;
                cls = env->FindClass("sun/io/ByteToCharGBK");
                break;
            case 4:      //big5
                dBegin = 0xa140;
                dEnd   = 0xfefe;
                //should use MS950 here?
                //cls = env->FindClass("sun/io/ByteToCharMS950");
                cls = env->FindClass("sun/io/ByteToCharBig5");
                break;
            case 5:    //Wansung
                dBegin = 0xa1a1;
                dEnd   = 0xfefe;
                cls = env->FindClass("sun/io/ByteToCharEUC_KR");
                break;

            case 6:    //Johab
                dBegin = 0x8141;
                dEnd   = 0xfdfe;
                cls = env->FindClass("sun/io/ByteToCharJohab");
                break;

            default:
                if (debugFonts) {
                    fprintf(stderr, "missing encoder [%d]\n", encodingID);
                }
                env->PopLocalFrame (0);
                return NULL;
        }

        if (cls == NULL
            || (mid = env->GetMethodID(cls, "<init>", "()V")) == NULL
            || (obj = env->NewObject(cls, mid)) == NULL) 
        {
	    env->PopLocalFrame (0);
            return NULL;
        }
        HANDLE_EXCEPTION();

        //substitute the uncoverted default char to "0x0"
        mid = env->GetMethodID(cls, "setSubstitutionChars", "([C)V");
        HANDLE_EXCEPTION();
        if (mid != NULL) {
            jcharArray jtmp = env->NewCharArray(1);
            HANDLE_EXCEPTION();
            UInt16 cc[1] = {0};
            env->SetCharArrayRegion(jtmp, 0, 1, (jchar*)cc);
            HANDLE_EXCEPTION();
            env->CallVoidMethod(obj, mid, jtmp);
            HANDLE_EXCEPTION();
        }

        mid = env->GetMethodID(cls, "convert", "([BII[CII)I");
        HANDLE_EXCEPTION();

        if ((retArray = (UInt16*) malloc(sizeof (UInt16) * 65536)) == NULL) {
            JNU_ThrowOutOfMemoryError(env, NULL);
            env->PopLocalFrame (0);
            return NULL;
        }

        for (i=0; i<65536; i++) {
            retArray[i] = 0xFFFD;
        }

        //convert the double byte characters
        jInputBytes = env->NewByteArray((dEnd - dBegin + 1) * 2);
        HANDLE_EXCEPTION();
        jOutputChars = env->NewCharArray(dEnd - dBegin + 1);
        HANDLE_EXCEPTION()
            if (jInputBytes == NULL || jOutputChars == NULL){
                free(retArray);
                env->PopLocalFrame (0);
                return NULL;
            }

        if ((tmpBytes = (UInt8*) malloc(sizeof (UInt8) * 2 * (dEnd - dBegin + 1))) == NULL){
            JNU_ThrowOutOfMemoryError(env, NULL);
            free(retArray);
            env->PopLocalFrame (0);
            return NULL;
        }

        j = 0;
        int firstByte;
        if (encodingID == 2) {
            for (i = dBegin; i <= dEnd; i++){
                firstByte = i >> 8 & 0xff;
                if (firstByte >= 0xa1 && firstByte <= 0xdf){
                    //sjis halfwidth katakana
                    *(tmpBytes + j++) = 0xff;
                    *(tmpBytes + j++) = 0xff;
                } else {
                    *(tmpBytes + j++) = firstByte;
                    *(tmpBytes + j++) = i & 0xff;
                }
            }
        } else
            for (i = dBegin; i <= dEnd; i++) {
                *(tmpBytes + j++) = i >> 8 & 0xff;
                *(tmpBytes + j++) = i & 0xff;
            }
        env->SetByteArrayRegion(jInputBytes,
                                0,
                                env->GetArrayLength(jInputBytes),
                                (jbyte*)tmpBytes);
        HANDLE_EXCEPTION();
        ret = env->CallIntMethod(obj,
                                 mid,
                                 jInputBytes,
                                 0,
                                 env->GetArrayLength(jInputBytes),
                                 jOutputChars,
                                 0,
                                 env->GetArrayLength(jOutputChars));
        HANDLE_EXCEPTION();

        env->GetCharArrayRegion(jOutputChars,
                                0,
                                env->GetArrayLength(jOutputChars),
                                (jchar*)(retArray + dBegin));
        HANDLE_EXCEPTION();

        //single byte ascii
        for (i = 0x20; i <= 0x7e; i++)
            retArray[i] = i;

        //sjis halfwidth katakana
        if (encodingID == 2) {
            for (i = 0xa1; i <= 0xdf; i++){
                retArray[i] = i - 0xa1 + 0xff61;
            }
        }

        free(tmpBytes);
        env->PopLocalFrame (0);
        return retArray;
    }
};

const tt_uint32 cmapFontTableTag =  0x636D6170; // 'cmap'

// cmap converters for tables 0, 2, 4 and 6 are

// Format 0: Byte Encoding table
static tt_uint16 getGlyph0(byte *cmap, tt_uint32 charCode)
{
    tt_uint16 index = 0;
    byte *charP = cmap;
    if (charCode < 256) {
        charP += 6;     /* skip format, length, and version */
                
        index = charP[charCode];
                
    }
    return index; /*****/
}

// Format 2: High-byte mapping through table
static tt_uint16 getGlyph2(byte *cmap, tt_uint32 charCode)
{
    byte               *charP = cmap;
    tt_uint16          index, mapMe;
    tt_uint16          highByte;
    tt_uint16          lowByte;
    tt_uint16          key;
    //sfnt_mappingTable2    *Table2;
    //sfnt_subHeader        *subHeader;
    byte               *subHeaderKeys;
    byte               *subHeaders;
    byte               *subHeader;

    if (charCode >= 0x10000) {
        return 0;
    }
    
    charP += 6;         /* jump over format, length, and version */

    index = 0;          /* Assume the missing glyph */

    /*
      if (wStuff && wStuff->glyph->longCharCode != 0xffff)
          charCode = wStuff->glyph->longCharCode;
    */

    highByte = charCode >> 8;
    lowByte = charCode & 0x00ff;

    //Table2 = (sfnt_mappingTable2 *) charP;
    subHeaderKeys = charP;
    subHeaders    = charP + 512;

    if ((key = GetUInt16(subHeaderKeys, highByte * 2)) != 0) {
        mapMe = lowByte;
    } else {
        mapMe = highByte;
    }

    if (mapMe == 0) {
        mapMe = lowByte;
    }

#if 0
    if ((key = Table2->subHeadersKeys[highByte]) != 0) {
        mapMe = lowByte;        /* We also need the low byte */
        if (wStuff)
            wStuff->glyph->numBytesUsed = 2;
    } else {
        mapMe = highByte;
        if (wStuff)
            wStuff->glyph->numBytesUsed = 1;
    }
    if ((wStuff == 0L) && (mapMe == 0)) {           /* JTS 7-2-93 for Inc-J AFM Files */
        mapMe = lowByte;
    }
#endif

    //  subHeader = (sfnt_subHeader *) ((char  *) &Table2->subHeaders + Table2->subHeadersKeys [highByte]);

    subHeader = subHeaders + key;

    tt_uint16 firstCode  = GetUInt16(subHeader,0);
    tt_uint16 entryCount = GetUInt16(subHeader,2);


    if (mapMe < firstCode)
        return 0;                       /***** Missing *****/

#if 0
    if ((Table2->subHeadersKeys[highByte] == 0) && (wStuff == 0L) && (charCode > 0xff))
        return 0;                       /* single byte character (AFM fix) JTS 7-2-93 */
#endif

    if ((key == 0) && (charCode > 0xFF)) {
        return 0;
    }

    mapMe -= firstCode;                     /* Subtract first code */

    if ( mapMe < entryCount ) {
        tt_uint16 glyph;
        tt_uint16 offset;
        tt_uint16 idRangeOffset = GetUInt16(subHeader, 6);
        tt_uint16 idDelta       = GetUInt16(subHeader, 4);

        //- calculate the offset from the beginning of subHeader !!!
        offset = idRangeOffset + 6 + mapMe + mapMe;
        index = GetUInt16(subHeader, offset);

        /*
            shortP = (tt_uint16 *)&(subHeader->idRangeOffset);
            shortP = (tt_uint16 *)((long)shortP + *shortP + mapMe + mapMe);

            if (*shortP)
                index = *shortP + subHeader->idDelta;
        */

        if (index) {
            index += idDelta;
        }
    }


#ifdef MS_CODE      // ??? Was in Sampo's code
    if (mapMe < subHeader->entryCount ) {
        tt_uint16 glyph;

        if (glyph = * ((tt_uint16 *) ((char *) &subHeader + subHeader->idRangeOffset) + mapMe))
            index = glyph + (tt_uint16) (subHeader->idDelta);
    }
#endif /* MS_CODE */

    return index;
}

#define ANY_NUMBER 1

struct CMAPEncodingSubtable
{
    tt_uint16  format;
    tt_uint16  length;
    tt_uint16  language;
};

struct CMAPFormat4Encoding : CMAPEncodingSubtable
{
    tt_uint16  segCountX2;
    tt_uint16  searchRange;
    tt_uint16  entrySelector;
    tt_uint16  rangeShift;
    tt_uint16  endCodes[ANY_NUMBER];
//  tt_uint16  reservedPad;
//  tt_uint16  startCodes[ANY_NUMBER];
//  tt_uint16  idDelta[ANY_NUMBER];
//  tt_uint16  idRangeOffset[ANY_NUMBER];
//  tt_uint16  glyphIndexArray[ANY_NUMBER];
};

#define BinaryIteration(n) \
    if (SWAPW(startCodes[index + (1 << (n))]) <= charCode) {\
        index += (1 << (n)); \
    };

static tt_uint16 getGlyph4(byte *cmap, tt_uint32 charCode)
{
    tt_uint16 index = 0;
    tt_uint16 glyphCode = 0;

    if (charCode >= 0x10000) {
        return 0;
    }

    CMAPFormat4Encoding *header = (CMAPFormat4Encoding *) cmap;
    tt_uint16 segCount = SWAPW(header->segCountX2) / 2;
    tt_uint16 searchRange = SWAPW(header->searchRange);
    tt_uint16 entrySelector = SWAPW(header->entrySelector);
    tt_uint16 rangeShift = SWAPW(header->rangeShift) / 2;
    tt_uint16 *startCodes = &header->endCodes[segCount + 1]; // + 1 for reservedPad...
    tt_uint16 *idDelta = &startCodes[segCount];
    tt_uint16 *idRangeOffset = &idDelta[segCount];

    if (SWAPW(startCodes[rangeShift]) <= charCode) {
        index = rangeShift;
    }

#ifdef BinaryIteration
    switch (entrySelector) {
    case 15: BinaryIteration(14);
    case 14: BinaryIteration(13);
    case 13: BinaryIteration(12);
    case 12: BinaryIteration(11);
    case 11: BinaryIteration(10);
    case 10: BinaryIteration(9);
    case  9: BinaryIteration(8);
    case  8: BinaryIteration(7);
    case  7: BinaryIteration(6);
    case  6: BinaryIteration(5);
    case  5: BinaryIteration(4);
    case  4: BinaryIteration(3);
    case  3: BinaryIteration(2);
    case  2: BinaryIteration(1);
    case  1: BinaryIteration(0);
    case  0:
    default: break;
    }
#else
    tt_uint16 probe = 1 << entrySelector;

    while (probe > (1 << 0)) {
        probe >>= 1;

        if (SWAPW(startCodes[index + probe]) <= charCode) {
            index += probe;
        }
    }
#endif

    if (charCode >= SWAPW(startCodes[index]) && charCode <= SWAPW(header->endCodes[index])) {
        tt_uint16 rangeOffset = SWAPW(idRangeOffset[index]);

    /*
        // fix for broken stop markers
        if (startCodes[index] == 0xFFFF && header->endCodes[index] == 0xFFFF & rangeOffset == 0xFFFF) {
            rangeOffset = 0;
        }
    */

        if (rangeOffset == 0 /*|| rangeOffset == 0xFFFF*/) {
            glyphCode = charCode;
        } else {
            tt_uint16 offset = charCode - SWAPW(startCodes[index]);
            tt_uint16 *glyphIndexTable = (tt_uint16 *) ((char *) &idRangeOffset[index] + rangeOffset);

            glyphCode = SWAPW(glyphIndexTable[offset]);
        }

        glyphCode += SWAPW(idDelta[index]);
    } else {
        glyphCode = 0;
    }

    return glyphCode;
}

// Format 6: Trimmed table mapping
static tt_uint16 getGlyph6(byte *cmap, tt_uint32 charCode)
{
    tt_uint16 entryCount, index;
    byte *charP = cmap;

    if (charCode >= 0x10000) {
        return 0;
    }

    charP += 6;             /* skip format, length, and version */

    index = *charP++;
    index <<= 8;
    index |= *charP++;
    charCode = (unsigned short)(charCode - index); /* -firstCode */
    entryCount = *charP++;
    entryCount <<= 8;
    entryCount |= *charP++; /* entryCount */
    index = 0;
    if ( charCode < entryCount ) {
        charP += charCode;  /* *2, since word Array */
        charP += charCode;  /* *2, since word Array */
        index = *charP++;
        index <<= 8;
        index |= *charP++;
    }

    return index; /*****/
}

struct CMAPEncodingSubtable32
{
    tt_uint32  format;
    tt_uint32  length;
    tt_uint32  language;
};

struct CMAPGroup
{
    tt_uint32 startCharCode;
    tt_uint32 endCharCode;
    tt_uint32 startGlyphCode;
};

//
// Finds the high bit by binary searching
// through the bits in n.
//
tt_int8 highBit(tt_uint32 value)
{
    tt_uint8 bit = 0;

    if (value >= 1 << 16) {
        value >>= 16;
        bit += 16;
    }

    if (value >= 1 << 8) {
        value >>= 8;
        bit += 8;
    }

    if (value >= 1 << 4) {
        value >>= 4;
        bit += 4;
    }

    if (value >= 1 << 2) {
        value >>= 2;
        bit += 2;
    }

    if (value >= 1 << 1) {
        value >>= 1;
        bit += 1;
    }

    return bit;
}

tt_uint16 getGlyphFromGroup(tt_uint32 charCode, CMAPGroup *groups, tt_uint32 nGroups)
{
    tt_uint8 bit = highBit(nGroups);
    tt_int32 power = 1 << bit;
    tt_int32 extra = nGroups - power;
    tt_int32 probe = power;
    tt_int32 range = 0;

    if (SWAPL(groups[extra].startCharCode) <= charCode) {
        range = extra;
    }

    while (probe > (1 << 0)) {
        probe >>= 1;

        if (SWAPL(groups[range + probe].startCharCode) <= charCode) {
            range += probe;
        }
    }

    if (SWAPL(groups[range].startCharCode) <= charCode && SWAPL(groups[range].endCharCode) >= charCode) {
        return (tt_uint16) (SWAPL(groups[range].startGlyphCode) + charCode - SWAPL(groups[range].startCharCode));
    }

    return 0;
}

struct CMAPFormat8Encoding : CMAPEncodingSubtable32
{
    tt_uint32 is32[65536/32];
    tt_uint32 nGroups;
    CMAPGroup groups[ANY_NUMBER];
};

// Format 8: Mixed 16-bit and 32-bit coverage
static tt_uint16 getGlyph8(byte *cmap, tt_uint32 charCode)
{
    // FIXME: Don't know what to do with is32 since we're
    // only passed one character code. We'll ignore it on
    // the assumption that we'll never see mixed 16/32 bit
    // text anyhow...

    CMAPFormat8Encoding *header = (CMAPFormat8Encoding *) cmap;

    return getGlyphFromGroup(charCode, header->groups, SWAPL(header->nGroups));
}

struct CMAPFormat10Encoding : CMAPEncodingSubtable32
{
    tt_uint32 startCharCode;
    tt_uint32 numCharCodes;
    tt_uint16 glyphs[ANY_NUMBER];
};

// Format 10: Trimmed Array
static tt_uint16 getGlyph10(byte *cmap, tt_uint32 charCode)
{
    CMAPFormat10Encoding *header = (CMAPFormat10Encoding *) cmap;
    tt_uint32 offset = charCode - SWAPL(header->startCharCode);

    if (offset < SWAPL(header->numCharCodes)) {
        return SWAPW(header->glyphs[offset]);
    }

    return 0;
}

struct CMAPFormat12Encoding : CMAPEncodingSubtable32
{
    tt_uint32 nGroups;
    CMAPGroup groups[ANY_NUMBER];
};

// Format 12: Segmented coverage
static tt_uint16 getGlyph12(byte *cmap, tt_uint32 charCode)
{
    CMAPFormat12Encoding *header = (CMAPFormat12Encoding *) cmap;

    return getGlyphFromGroup(charCode, header->groups, SWAPL(header->nGroups));
}

// Format 1: Plain Table (65536 tt_uint16 glyphIDs)

static tt_uint16 getGlyph103(byte *cmap, tt_uint32 charCode)
{
    if (charCode >= 0x10000) {
        return 0;
    }

    return GetUInt16(cmap + 6, charCode * 2);
}

// Implementation of getGlyph104 is below, with the hashtable insert code.
static tt_uint16 getGlyph104(byte *cmap, tt_uint32 charCode);

static inline tt_uint16 getGlyph(byte *cmap, tt_uint32 charCode)
{
    if (charCode == 0xFFFF) {
        return 0;
    }

    switch(GetUInt16(cmap, 0)) {
    case 0: return getGlyph0(cmap, charCode);
    case 2: return getGlyph2(cmap, charCode);
    case 4: return getGlyph4(cmap, charCode);
    case 6: return getGlyph6(cmap, charCode);
    case 8: return getGlyph8(cmap, charCode);
    case 10: return getGlyph10(cmap, charCode);
    case 12: return getGlyph12(cmap, charCode);
    case 103: return getGlyph103(cmap, charCode);
    case 104: return getGlyph104(cmap, charCode);
#ifdef DEBUG
    default:
       printf("ERROR: Unsupported cmap format.\n");
#endif
    }
    return 0;
}

// Returns an array that converts Microsoft encoding to Unicode.
static tt_uint16 *GetConverter(tt_uint16 encodingID)
{
    //  0 - Symbol
    //  2 - ShiftJIS
    //  3 - PRC
    //  4 - BIG5
    //  5 - Wansung
    //  6 - Johab
    static tt_uint16 *carray[7] = {0, 0, 0, 0, 0, 0, 0};
    if (encodingID > 6) {
        return NULL;
    }

    if (carray[encodingID] == 0) {
        carray[encodingID] = makeNativeToUnicodeArray(encodingID);
        if (carray[encodingID] == 0) {
            carray[encodingID] = (tt_uint16 *) -1;
        }
    }

    if (carray[encodingID] == (tt_uint16 *) -1) {
        return NULL;
    }

    return carray[encodingID];
}



/*+----------------------------------------------------------------------
 |
 | Name:            CreateCMAP()
 |
 | Usage:           This functions generates a Unicode cmap
 |
 | Arguments:       cmap - points to a cmap subtable
 |                  xlat - translation array that translates a character code
 |                         used to index the given cmap to a Unicode number.
 |
 | Description:
 |
 | Return value:    pointer to a new cmap subtable. Memory for this table
 |                  is allocated dynamically and should be freed when the
 |                  cmap is no longer needed
 |
 +---------------------------------------------------------------------- */

byte *CreateCMAP(byte *cmap, tt_uint16 *xlat)
{
    byte *res=0, *type4;
    byte *buffer      = new byte [65536 * 2 + 6];
    byte *glyphArray  = buffer + 6;

    if (buffer == 0) return 0;

    memset(glyphArray, 0, 65536 * 2);


    int i, j;

    /* Sadly the callers of this function sometimes pass
     * in NULL "xlat". We need to guard against this.
     */
    if (xlat == NULL) {
        if (debugFonts) {
            fprintf(stderr,"CreateCMAP: null xlat\n");
        }
        delete buffer;
        return NULL;
    }
      
    for (i=0; i<65536; i++) {
        if (xlat[i] != 0xFFFD) {
            PutInt16(getGlyph(cmap, i), glyphArray, xlat[i] * 2);
        }
    }

    PutInt16(103, buffer, 0);                                 //- cmap format 103

    return buffer;
}

/*
 * We handle "symbol" CMAPs specially, in order to save space.  This
 * new format uses a 3 K hashtable to replace a 128 K direct map.
 *
 * This is known internally as our "104" format.  This format is only
 * used for special mappings of Microsoft WingDing and Symbol fonts.  
 * We know WingDings has only 53 relevant characters and Symbol only 94.
 *
 * We create a chained hash table, where there are 128 slots of indexed
 * cells and 128 slots of overflow cells.  Each slot consists of
 * 
 *      16 bit unicode value   (0 => slot is empty)
 *      16 bit glyph value
 *       8 bit index to next bucket in chain (0 => no more)
 *
 * The hash index is simply the low byte of the unicode value.  This
 * works well with the actual values in the fonts.
 *
 *                                                KGH Feb 99
 */

// cmap_has_bucket is used for both basic buckets and overflows.
typedef struct {
    UInt16    unicode;    // a unicode value (zero => not used)
    UInt16    glyph;      // 
    UInt16    next;       // next bucket in chain (zero => not used)
} cmap_hash_bucket;
#define CMAP_BASIC_BUCKETS 128
#define CMAP_OVERFLOW_BUCKETS 128
#define CMAP_TOTAL_BUCKETS (CMAP_BASIC_BUCKETS+CMAP_OVERFLOW_BUCKETS)

static byte *CreateSymbolCMAP(byte *cmap, UInt16* Symbols)
{
    int i;
    byte *buffer = (byte *) new UInt16[((CMAP_TOTAL_BUCKETS*sizeof(cmap_hash_bucket))+8)/2];
    cmap_hash_bucket *table = (cmap_hash_bucket *)(buffer+6);
    cmap_hash_bucket *next_overflow = table + CMAP_BASIC_BUCKETS;
    cmap_hash_bucket *overflow_limit = next_overflow + CMAP_OVERFLOW_BUCKETS;
    if (buffer == 0) {
        return 0;
    }
    memset(table, 0, (CMAP_TOTAL_BUCKETS*sizeof(cmap_hash_bucket)));

    for (i=0; i<256; i++) {
        UInt16 unicode = Symbols[i];
        if (unicode != 0xFFFD) {
            // Add an entry to the hashtable.
            UInt16 glyph = getGlyph(cmap, i + 0xf000);
            UInt8 hash = (UInt8)(unicode % CMAP_BASIC_BUCKETS);

            cmap_hash_bucket *bucket = table + hash;
            if (bucket->unicode == 0) {
                // The basic bucket is free.  Use it.
                bucket->unicode = unicode;
                bucket->glyph = glyph;
            } else {
                // Allocate an overflow bucket
                cmap_hash_bucket *overflow = next_overflow++;
                if (overflow >= overflow_limit) {
                    // This should never happen, as we reserve enough space!
                    printf("ERROR: CreateSymbolCMAP out of hash buckets");
                    continue;
                }
                overflow->unicode = unicode;
                overflow->glyph = glyph;
                overflow->next = bucket->next;
                bucket->next = (overflow - table);
            }
                    
        }
    }

    PutInt16(104, buffer, 0);  
    return buffer;
}

/*
 * Hash table lookup.
 *                                      KGH Feb 99
 */
static tt_uint16 getGlyph104(byte *cmap, tt_uint32 charCode)
{
    UInt8 hash = (UInt8)(charCode % CMAP_BASIC_BUCKETS);
    cmap_hash_bucket *table = (cmap_hash_bucket *)(cmap+6);
    
    cmap_hash_bucket *bucket = table + hash;
    for (;;) {
        if (bucket->unicode == charCode) {
            return bucket->glyph;
        }
        if (bucket->next == 0) {
            return 0;
        }
        bucket = table + bucket->next;
    }
}


static char *upcasestr(char *str) 
{
  char *res=str;
  while (*str) {
    *str = toupper(*str);
    str++;
  }
  return res;
}

// searches for the second string in the first
static char *j_strcasestr(const char *s1, const char *s2)
{
  char *ts1=upcasestr(strdup(s1));
  char *ts2=upcasestr(strdup(s2));

  char *res = strstr(ts1, ts2);

  free(ts1);
  free(ts2);

  return res;
}

  
void InitializeCMAP(sfntFileFontObject& font, byte * &cmap)
{
    int i;

    cmap = NULL;

    tt_uint32 cmapOffset, cmapLength;
    cmapLength = font.FindTableSize(cmapFontTableTag, &cmapOffset);
    byte *buffer = new byte[cmapLength];
    if ( !buffer ) {
       return;
    }
    font.readBlock(cmapOffset, cmapLength, (char *) buffer);
    tt_uint16 numberSubtables = GetUInt16(buffer, 2);
    tt_uint32  ThreeZero  = 0;                   // MS Symbol
    tt_uint32  ThreeOne   = 0;                   // MS Unicode
    tt_uint32  ThreeTwo   = 0;                   // MS ShiftJIS
    tt_uint32  ThreeThree = 0;                   // MS Big5
    tt_uint32  ThreeFour  = 0;                   // MS PRC
    tt_uint32  ThreeFive  = 0;                   // MS Wansung
    tt_uint32  ThreeSix   = 0;                   // MS Johab
    tt_uint32  ThreeTen   = 0;                   // MS Unicode surrogate
    Boolean ThreeStar  = false;               // Any MS encoding

    tt_uint16 platformID, encodingID;

    tt_uint32 firstOffset = GetUInt32(buffer, 8 ); // first offset is at 8 bytes

    for (i=0; i<numberSubtables; i++) {
        tt_uint32 offset;
        platformID = GetUInt16(buffer, 8 * i + 4);
        encodingID = GetUInt16(buffer, 8 * i + 6);
        offset     = GetUInt32(buffer, 8 * i + 8);

        if (platformID == 3) {
            ThreeStar = true;
            switch (encodingID) {
            case 0: ThreeZero  = offset; break;
            case 1: ThreeOne   = offset; break;
            case 2: ThreeTwo   = offset; break;
            case 3: ThreeThree = offset; break;
            case 4: ThreeFour  = offset; break;
            case 5: ThreeFive  = offset; break;
            case 6: ThreeSix   = offset; break;
            case 10: ThreeTen = offset; break;
            }
        }
    }

    if (ThreeStar) {
      
      if (debugFonts) {
        if (ThreeZero)  fprintf(stderr,"%s - CMAP 3,0 found at offset %ld.\n", font.GetFontLocalPlatformName(), ThreeZero);
        if (ThreeOne)   fprintf(stderr,"%s - CMAP 3,1 found at offset %ld.\n", font.GetFontLocalPlatformName(), ThreeOne);
        if (ThreeTwo)   fprintf(stderr,"%s - CMAP 3,2 found at offset %ld.\n", font.GetFontLocalPlatformName(), ThreeTwo);
        if (ThreeThree) fprintf(stderr,"%s - CMAP 3,3 found at offset %ld.\n", font.GetFontLocalPlatformName(), ThreeThree);
        if (ThreeFour)  fprintf(stderr,"%s - CMAP 3,4 found at offset %ld.\n", font.GetFontLocalPlatformName(), ThreeFour);
        if (ThreeFive)  fprintf(stderr,"%s - CMAP 3,5 found at offset %ld.\n", font.GetFontLocalPlatformName(), ThreeFive);
        if (ThreeSix)   fprintf(stderr,"%s - CMAP 3,6 found at offset %ld.\n", font.GetFontLocalPlatformName(), ThreeSix);
        if (ThreeTen)   fprintf(stderr,"%s - CMAP 3,10 found at offset %ld.\n", font.GetFontLocalPlatformName(), ThreeTen);
      }
        if (ThreeTen) {
            tt_uint16 length = GetUInt32(buffer, ThreeTen+4); // length of the subtable
            if (ThreeTen + length > cmapLength) {
			    return; 
            }
            cmap = new byte [length];
            if ( !cmap ) {
                delete [] buffer;
                return;
            }
              
            memcpy(cmap, buffer+ThreeTen, length);
        } else if (ThreeOne) {                 //- MS Unicode cmap found, just use it
            tt_uint16 length = GetUInt16(buffer, ThreeOne+2); // length of the subtable
            //check for broken cmap (see bug 4762039)
            if (ThreeOne + length > cmapLength) {
			    return;
            }

            cmap = new byte [length];
            if ( !cmap ) {
                delete [] buffer;
                return;
            }
              
            memcpy(cmap, buffer+ThreeOne, length);
        } else if (ThreeZero) {         //- The font has a Symbol encoding. Use it.
            // cmap = CreateCMAP(buffer+ThreeZero, GetConverter(0));
            // REMIND: need platform independent determination.
            const char*fontname = font.GetFontLocalPlatformName();
            if (j_strcasestr(fontname, "SYMBOL.TTF") != NULL) {
                cmap = CreateSymbolCMAP(buffer+ThreeZero, Symbols_b2c);
            } else if (j_strcasestr(fontname, "WINGDING.TTF") != NULL) {
                cmap = CreateSymbolCMAP(buffer+ThreeZero, WingDings_b2c);
            } else {
                tt_uint16 length = GetUInt16(buffer, ThreeZero+2); // length of the subtable
                if (ThreeZero + length > cmapLength) {
                    return;
                }
                cmap = new byte [length];
                if ( !cmap ) {
                    delete [] buffer;
                    return;
                }

                memcpy(cmap, buffer+ThreeZero, length);
            }
        } else if (ThreeTwo) {          //- The font has a ShiftJIS cmap. Convert it to Unicode.
            cmap = CreateCMAP(buffer+ThreeTwo, GetConverter(2));
        } else if ( ThreeThree ) {                                    //- GB
            cmap = CreateCMAP ( buffer + ThreeThree, GetConverter(3));
        } else if (ThreeFour) {                                       //- Big5
#ifdef __solaris__
	    //GB2312 TrueType fonts on Solaris have wrong encoding ID for
	    //cmap table, these fonts have EncodingID 4 which is Big5 encoding
	    //according the TrueType spec, but actually the fonts are using 
	    //gb2312 encoding, have to use this workaround to make Solaris
	    //zh_CN locale work.  -sherman
            const char *fname = font.GetFontLocalPlatformName();
            if (fname &&
                (strstr(fname, "/usr/openwin/lib/locale/zh_CN.EUC/X11/fonts/TrueType") ||
                 strstr(fname, "/usr/openwin/lib/locale/zh_CN/X11/fonts/TrueType") ||
                 strstr(fname, "/usr/openwin/lib/locale/zh/X11/fonts/TrueType")))
                cmap = CreateCMAP(buffer+ThreeFour, GetConverter(3));
            else
#endif
            cmap = CreateCMAP(buffer+ThreeFour, GetConverter(4));
        } else if ( ThreeFive ) { // Wansung
           cmap = CreateCMAP ( buffer + ThreeFive, GetConverter(5));
        } else if ( ThreeSix ) { // Johab
           cmap = CreateCMAP ( buffer + ThreeSix, GetConverter(6));
        }
#ifdef DEBUG
        else {
            printf("No known MS cmap subtables were found.\n");
        }
#endif

    } else {
      
      /* This is to handle the case where no useful table was found but atleast we use the
         first one in the font - so later on we do not have problems - the encodings may
         not work - but the font is pretty much guaranteed to have one CMAP encoding */
       tt_uint16 length = GetUInt16(buffer, firstOffset+2); // length of the subtable
       cmap = new byte [length];
       if ( cmap != 0 ) {
           memcpy(cmap, buffer+firstOffset, length);
       }
#ifdef DEBUG
        printf("No usable cmap tables found.\n");
#endif
    }

    // Initialize the character to glyph mapping function
    // Unicode cmap can be format 4, 6, 8, 10, or 12, anything else is an error
    if (cmap != 0) {
        tt_uint16 format = GetUInt16(cmap, 0);
        switch (format) {
        case 0:
        case 4:
        case 6:
        case 8:
        case 10:
        case 12:
        case 103:                     // XXX cmap format 1
        case 104:                     // hash table format
            break;
        default:
          // Can't handle this cmap
            delete [] cmap;
            cmap = 0;
#ifdef DEBUG
            printf("Unicode cmap table is not usable (format: %d.)\n", format);
#endif
        }
    }

    delete [] buffer;
}


#ifdef __solaris__
/* For fix for 4533422 (below) */
#define JA_WAVE_DASH_CHAR 0x301c
#define JA_FULLWIDTH_TILDE_CHAR 0xff5e
#endif /* __solaris__*/

static tt_uint32 ConvertUnicodeToGlyph(sfntFileFontObject& font, byte * &cmap, Unicode theChar)
{
    tt_uint32 res = 0;

    if ( cmap != 0 ) {
        res = getGlyph(cmap, theChar);
#ifdef __solaris__
	if (res > 0 && res < font.fGlyphCharMapLen) {
	    font.fGlyphToCharMap[res] = theChar;
	} else if (res == 0 && theChar == JA_WAVE_DASH_CHAR) {
	  // See bug id 4533422 - special case for Japanese wave dash
	  // missing from Solaris JA TrueType fonts.
	    res = getGlyph(cmap, JA_FULLWIDTH_TILDE_CHAR);
	    if (res > 0 && res < font.fGlyphCharMapLen) {
	      font.fGlyphToCharMap[res] = theChar;
	    }
	}
#endif
    }
    return res;
}


//- Public APIs

void ConvertUnicodeToGlyphs(sfntFileFontObject& font, byte * &cmap, int numChars, const Unicode16 chars[], UInt32 glyphs[])
{
    int i;

    //- Initialize the glyph array with "missing glyph" IDs
    memset(glyphs, 0, numChars * 4);

    if (cmap) {
        for (i=0; i<numChars; i++) {
            Unicode16 high = chars[i];
            Unicode32 code = high;

            if (i < numChars - 1 && high >= 0xD800 && high <= 0xDBFF) {
                Unicode16 low = chars[i + 1];

                if (low >= 0xDC00 && low <= 0xDFFF) {
                    code = (high - 0xD800) * 0x400 + low - 0xDC00 + 0x10000;
                }
            }

            glyphs[i] = getGlyph(cmap, code);
#ifdef __solaris__
	    if (glyphs[i] > 0  &&
		glyphs[i] < font.fGlyphCharMapLen && code < 0x10000) {
		font.fGlyphToCharMap[glyphs[i]] = (Unicode16)code;
	    }  else if (glyphs[i] == 0 && code == JA_WAVE_DASH_CHAR) {
	      // See bug id 4533422 - special case for Japanese wave dash
	      // missing from Solaris JA TrueType fonts.
	      glyphs[i] = getGlyph(cmap, JA_FULLWIDTH_TILDE_CHAR);
	      if (glyphs[i] > 0  &&
		  glyphs[i] < font.fGlyphCharMapLen && code < 0x10000) {
		font.fGlyphToCharMap[glyphs[i]] = (Unicode16)code;
	      }
	    }
#endif
            if (code >= 0x10000) {
                i += 1;
                glyphs[i] = 0xFFFF;
            }
        }
    }
    return;
}

void ConvertUnicodeToGlyphs(sfntFileFontObject& font, byte * &cmap, int numChars, const Unicode32 chars[], UInt32 glyphs[])
{
    int i;

    //- Initialize the glyph array with "missing glyph" IDs
    memset(glyphs, 0, numChars * 4);

    if (cmap) {
        for (i=0; i<numChars; i++) {
            glyphs[i] = getGlyph(cmap, chars[i]);
#ifdef __solaris__
	    if (glyphs[i] > 0 &&
		glyphs[i] < font.fGlyphCharMapLen && chars[i] < 0x10000) {
		font.fGlyphToCharMap[glyphs[i]] = (Unicode16)chars[i];

	    } else if (glyphs[i] == 0 && chars[i] == JA_WAVE_DASH_CHAR) {
	      // See bug id 4533422 - special case for Japanese wave dash
	      // missing from Solaris JA TrueType fonts.
		glyphs[i] = getGlyph(cmap, JA_FULLWIDTH_TILDE_CHAR);
		if (glyphs[i] > 0 &&
		    glyphs[i] < font.fGlyphCharMapLen && chars[i] < 0x10000) {
		  font.fGlyphToCharMap[glyphs[i]] = (Unicode16)chars[i];
		}
	    }
#endif
        }
    }
    return;
}



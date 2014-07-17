/*
 * @(#)nrScalerContext.cpp	1.58 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @author sherman@japan
 * @author Charlton Innovations, Inc.
 * @author Joe Warzecha
 */

/*
 * Important note : All AWTxxx functions are defined in font.h.
 * These were added to remove the dependency of this file on X11.
 * These functions are used to perform X11 operations and should
 * be "stubbed out" in environments that do not support X11.
 * The implementation of these functions has been moved from this file
 * into nrScalerContext_md.c, which is compiled into another library.
 */
#include <stdio.h>

#include "t2kScalerContext.h"
#include "nrScalerContext.h"

#include "fontObject_md.h"
#include "hsTemplates.h"
#include "hsMemory.h"

#include <ctype.h>
#include <sys/utsname.h>

#include <jni.h>
#include <jni_util.h>

#define SHIFTFACTOR 16

#define HANDLE_EXCEPTION()		\
    if (env->ExceptionCheck ()) {	\
	if (debug) {			\
	    env->ExceptionDescribe ();	\
	}				\
	env->ExceptionClear ();		\
	env->PopLocalFrame (NULL);		\
	return;				\
    }

#define HANDLE_EXCEPTION2()		\
    if (env->ExceptionCheck ()) {	\
	if (debug) {			\
	    env->ExceptionDescribe ();	\
	}				\
	env->ExceptionClear ();		\
	free (valsToConvert);		\
	env->PopLocalFrame (NULL);		\
	return;				\
    }

#define MOVETO 1
#define LINETO 2
#define CURVETO 3
#define CLOSEPATH 4

extern "C" {
static AWTChar2b *xChar = 0;
}

class nrCharToGlyphMapper : public CharToGlyphMapper {
private:
    X11CharToGlyphMapper	*xMapper;
    CharToGlyphMapper		*t2kMapper;
    int				 t2kGlyphs;
    UInt32			 xGlyphs;
    int				*xCharCodes;
public:
    nrCharToGlyphMapper (X11CharToGlyphMapper *mapper1, 
			 CharToGlyphMapper *mapper2, int nt2kGlyphs, 
			 int nxGlyphs);
    virtual ~nrCharToGlyphMapper ();
    virtual void CharsToGlyphs (int count, const Unicode16 unicodes [],
				UInt32 glyphs []) const;
    virtual void CharsToGlyphs (int count, const Unicode32 unicodes [],
				UInt32 glyphs []) const;
    virtual Boolean canDisplay (Unicode ch) const;
    char *GetPSEncoding () ;
    int getXCharCode (int glyphID);
};

class X11nrScalerContext : public hsGScalerContext {
private:
    const fontObject    *fFont;
    Boolean             fDoAntiAlias;
    Boolean             fDoFracEnable;
    double              fMatrix[4];
    double		ptSize;
    int			fNumGlyphs;
    int                 encIndex;
    nrCharToGlyphMapper *fMapper;
    Boolean		fDoPaths;
    hsPathSpline	**pathCache;
    hsFixedPoint2	**pathAdvances;
    AWTFont		fXFont;
    t2kScalerContext	*t2k;
    Boolean		uset2k;
public:
    X11nrScalerContext(fontObject* fo,
                         const float matrix[],
                         Boolean   doAntiAlias,
                         Boolean   doFractEnable, int style, int encIndex=0);
    virtual ~X11nrScalerContext();

    virtual UInt32  CountGlyphs();
    virtual void    GenerateMetrics(
                        UInt16 glyphID, hsGGlyph* glyph,
                        hsFixedPoint2* advance);
    virtual void    GenerateMetricsWithImage(
                        UInt16 glyphID, hsGGlyph* glyph,
                        hsFixedPoint2* advance);
    virtual void    GenerateImage(
                        UInt16 glyphID,
                        const hsGGlyph* glyph,
                        void* buffer);
    virtual void    GeneratePath(UInt16 index, hsPathSpline* path,
                        hsFixedPoint2* advance);
    virtual void    GetLineHeight(hsFixedPoint2* ascent,
                        hsFixedPoint2* descent,
                        hsFixedPoint2* baseline,
                        hsFixedPoint2* leading,
                        hsFixedPoint2* maxAdvance);
    virtual void GetCaretAngle(hsFixedPoint2* caret);
    virtual CharToGlyphMapper *getMapper();
    virtual Boolean isStyleSupported(int aStyle) { return false; }
    virtual Boolean canRotate (); 
    virtual Boolean GetGlyphPoint(UInt16 glyphID, Int32 pointNumber, hsFixedPoint2 *point);
    virtual void TransformFunits(Int16 xFunits, Int16 yFunits, hsFixedPoint2 *pixels);
};

//////////////////////////////////////////////////////////////////////////////

#define SINGLE_BYTE 1
#define DOUBLE_BYTE 2

nrCharToGlyphMapper::nrCharToGlyphMapper
	(X11CharToGlyphMapper *mapper1, CharToGlyphMapper *mapper2, 
	 int nt2kGlyphs, int nxGlyphs) : 
	  	xMapper (mapper1), t2kMapper (mapper2), t2kGlyphs (nt2kGlyphs), 
		xGlyphs (nxGlyphs), xCharCodes (NULL) 
{
    if (nt2kGlyphs != 0) {
	xCharCodes = (int *) calloc (1, sizeof (int) * nt2kGlyphs);
	memset (xCharCodes, -1, sizeof (int) * nt2kGlyphs);
    }
}

nrCharToGlyphMapper::~nrCharToGlyphMapper() 
{
    if (xCharCodes != NULL) {
	free (xCharCodes);
    }
}

void
nrCharToGlyphMapper::CharsToGlyphs (int count, const Unicode16 unicodes[],
                                    UInt32 glyphs[]) const 
{
    if (t2kMapper != NULL) {
	t2kMapper->CharsToGlyphs (count, unicodes, glyphs);
	UInt32 *xIndex = new UInt32 [count];
	xMapper->CharsToGlyphs (count, unicodes, xIndex);
	for (int i = 0; i < count; i++) {

	    /* If the xCharCode is greater than the total number of
	     * glyphs, then we're dealing with a CJK font where the
	     * same font is used twice (each time with a different X
	     * encoding).
	     */
	    if ((xIndex [i] == 0) || (xIndex [i] >= xGlyphs)) {
		glyphs [i] = 0;
	    } else {
	        xCharCodes [glyphs [i]] = xIndex [i];
	    }
	}
	delete xIndex;
    } else {
	xMapper->CharsToGlyphs (count, unicodes, glyphs);
    }
}

void
nrCharToGlyphMapper::CharsToGlyphs (int count, const Unicode32 unicodes[],
                                    UInt32 glyphs[]) const 
{
    if (t2kMapper != NULL) {
	t2kMapper->CharsToGlyphs (count, unicodes, glyphs);
	UInt32 *xIndex = new UInt32 [count];
	xMapper->CharsToGlyphs (count, unicodes, xIndex);
	for (int i = 0; i < count; i++) {

	    /* If the xCharCode is greater than the total number of
	     * glyphs, then we're dealing with a CJK font where the
	     * same font is used twice (each time with a different X
	     * encoding).
	     */
	    if ((xIndex [i] == 0) || (xIndex [i] >= xGlyphs)) {
		glyphs [i] = 0;
	    } else {
	        xCharCodes [glyphs [i]] = xIndex [i];
	    }
	}
	delete xIndex;
    } else {
	xMapper->CharsToGlyphs (count, unicodes, glyphs);
    }
}

Boolean 
nrCharToGlyphMapper::canDisplay (Unicode ch) const 
{
    Unicode uVal[1];
    UInt32 xVal [1];
    uVal [0] = ch;
    CharsToGlyphs (1, uVal, xVal);
    if (debugFonts) {
	fprintf(stderr,"nr:canDisplay unicode 0x%x -> xVal=0x%x\n",ch,(int)xVal[0]);
    }
    return (xVal[0] != 0);
}

int 
nrCharToGlyphMapper::getXCharCode (int glyphID)
{
    if ((xCharCodes != NULL) && (glyphID <= t2kGlyphs)) {
	return xCharCodes [glyphID];
    }

    return 0;
}

char *
nrCharToGlyphMapper::GetPSEncoding () 
{
    return xMapper->GetPSEncoding ();
}

X11CharToGlyphMapper::X11CharToGlyphMapper 
  (const char* javaclass, short numBytes, char *psEnc, Unicode maxVal, 
   Unicode minVal) 
    : convertedCount (65536), nBytes (numBytes), psEncoding (psEnc)
{

#ifdef DEBUG
	Boolean debug = true;
#else
	Boolean debug = false;
#endif

    /* If javaclass is NULL, its an iso8859-1 font, with 256 entries.
     * convertedGlyphs array has 256 entries.
     */
    if (javaclass == NULL) {
	convertedGlyphs = (Unicode *) calloc (1, sizeof (Unicode) * 256);
	for (int i = 0; i < 256; i++) {
	    convertedGlyphs [i] = i;
	}
	convertedCount = 256;
 	return;
    }


    /* Create an array to store the unicode->X11 character code values */

    convertedGlyphs = (Unicode *) calloc (1, sizeof (Unicode) * 65536);

    /* Find the correct CharToByte converter class */

    JNIEnv *env = (JNIEnv *) JNU_GetEnv (jvm, JNI_VERSION_1_2);

    /* Need at least 16 local refs */
    if (env->PushLocalFrame (16) < 0) {
	return;
    }
	
    jclass converterClass = env->FindClass (javaclass);
    HANDLE_EXCEPTION();

    if (converterClass == NULL) {
	env->PopLocalFrame (NULL);
	return;
    }

    /* Get a reference to the constructor we need to use */

    jmethodID converterConstruct = env->GetMethodID (converterClass,
						     "<init>", "()V");
    HANDLE_EXCEPTION();

    if (converterConstruct == NULL) {
	env->PopLocalFrame (NULL);
	return;
    }

    /* Create the new CharToByte converter object */

    jobject converterObject = env->NewObject (converterClass, 
					      converterConstruct);

    HANDLE_EXCEPTION();

    if (converterObject == NULL) {
	env->PopLocalFrame (NULL);
        return;
    }

    /* If unicode value can't be converted, set byte to return to 0 */
    /* dlf: note, this becomes the 'missing glyph' id, so the scaler context
     * has to gracefully handle requests to render or get outlines when
     * presented with this value.
     */
    jmethodID mid = env->GetMethodID (converterClass, "setSubstitutionBytes",
				      "([B)V");
    HANDLE_EXCEPTION();
    if (mid != NULL) {
	jbyteArray jtmp = env->NewByteArray (nBytes);
	HANDLE_EXCEPTION();
	byte *bb = new byte [nBytes];
	for (int i = 0; i < nBytes; i++)
	    bb[i] = 0;
	env->SetByteArrayRegion (jtmp, 0, nBytes, (jbyte*) bb);
	delete bb;
	HANDLE_EXCEPTION();
	env->CallVoidMethod (converterObject, mid, jtmp);
	HANDLE_EXCEPTION();
    }

    /* Get a reference to the "convert" method of the CharToByteConverter */

    jmethodID converterMethod = env->GetMethodID (converterClass, 
						"convert", "([CII[BII)I");
    HANDLE_EXCEPTION();

    if (converterMethod == NULL) {
	env->PopLocalFrame (NULL);
	return;
    }
	
    /* Count is total number of unicode values to convert to X11 character
     * code . Used in some cases where the boundaries are well known so 
     * the number of codes converted is lower and faster. Normally though
     * this is the entire unicode range (0 - 0xffff).
     */
    int count = maxVal - minVal + 1;
    Unicode *valsToConvert = (Unicode *) calloc (1, sizeof (Unicode) * count);
    for (int i = 0; i < count; i++) { 
        valsToConvert [i] = minVal + i;
    }

    /* charArray is set to the unicode values to convert,
     * byteArray is the array in which the X11 character codes will be put
     */
    jcharArray charArray = env->NewCharArray (count);
    HANDLE_EXCEPTION2();
    jbyteArray byteArray = env->NewByteArray (count * nBytes);
    HANDLE_EXCEPTION2();

    env->SetCharArrayRegion (charArray, 0, count, valsToConvert);
    HANDLE_EXCEPTION2();

    /* Now loop until all of the unicode values are converted. The
     * 'convert' method may throw an exception if the unicode value
     * can not be converted to X11 character code. If so, we can
     * call 'convert' again, starting with the next unicode value.
     */
    int done = 0;
    int currentChar = -1;
    /* For multi-byte encoding, single byte ascii should be skip. */
    if (nBytes > SINGLE_BYTE) {
	currentChar += 0x0100;
	byte *bb = new byte [0x0100 * nBytes];
	for (int i = 0; i < 0x0100 * nBytes; i++)
	    bb [i] = 0;
	env->SetByteArrayRegion (byteArray, 0, 0x0100 * nBytes, (jbyte*) bb);
	delete bb;
    }
    jmethodID nextCharMethod = NULL;
    jint ret;
    do {
	currentChar++;
        ret = env->CallIntMethod (converterObject, converterMethod,
			          charArray, currentChar, count,
				  byteArray, currentChar * nBytes, 
						count * nBytes);

        if (env->ExceptionCheck ()) { 
	    // env->ExceptionDescribe ();
            env->ExceptionClear ();
	    if (nextCharMethod == NULL) {
    	        nextCharMethod = env->GetMethodID (converterClass, 
						   "nextCharIndex", "()I");
	    }

	    currentChar = env->CallIntMethod (converterObject, nextCharMethod);
	    HANDLE_EXCEPTION2();
        } else {
	    done = 1;
	}

    } while (!done);

    /* Read the X11 character codes and store in the convertedGlyphs array */

    jbyte *charCodes = (jbyte *) calloc (1, sizeof (jbyte) * (count * nBytes));
    env->GetByteArrayRegion (byteArray, 0, count * nBytes, charCodes);
    if (env->ExceptionCheck ()) {
	if (debug) {
	    env->ExceptionDescribe ();
	}
	env->ExceptionClear();
    } else {
        for (int i = 0; i < count; i++) { 
            if (nBytes == 1) {
	        convertedGlyphs [i + minVal] = 0x00ff & charCodes [i];
	    } else {
	        convertedGlyphs [i + minVal] = ((charCodes [i * 2] & 0x00ff) << 8) +
			           	        (charCodes [(i * 2) + 1] & 0x00ff); 
	    }
	}
    }

    free (valsToConvert);
    free (charCodes);
    env->PopLocalFrame (NULL);
}

X11CharToGlyphMapper::~X11CharToGlyphMapper() 
{
    if (convertedGlyphs != NULL) {
        free (convertedGlyphs);
    }
}
 
void 
X11CharToGlyphMapper::AddAsciiChars(Boolean lowPartOnly) 
{
    /* This check is probably not required, but make sure we
     * allocated enough memory for the ascii characters 
     */
    int max = 0x100;
    if (lowPartOnly) {
	max = 0x80;
    } 
    if (convertedCount >= 0xff) {
	for (int i = 0x00; i < max; i++) {
	    // Don't overwrite chars which have a different mapping
	    // already established by the converter.
	    if (convertedGlyphs[i] == 0) {
		convertedGlyphs[i] = i;
	    }
	}
    }
}

void
X11CharToGlyphMapper::CharsToGlyphs(int count, const Unicode16 unicodes[],
                             UInt32 glyphs[]) const 
{
    for (int i = 0; i < count; i++) {
	if (unicodes [i] >= convertedCount) { 
	    glyphs [i] = 0;
	} else {
	    glyphs [i] = convertedGlyphs [unicodes [i]];
	}
    }
}

void
X11CharToGlyphMapper::CharsToGlyphs(int count, const Unicode32 unicodes[],
                             UInt32 glyphs[]) const 
{
    for (int i = 0; i < count; i++) {
	if (unicodes [i] >= (Unicode32)convertedCount) { 
	    glyphs [i] = 0;
	} else {
	  glyphs [i] = convertedGlyphs [(Unicode16)unicodes [i]]; // dlf - fix this
	}
    }
}

Boolean 
X11CharToGlyphMapper::canDisplay (Unicode ch) const 
{
    Unicode uVal[1];
    UInt32 xVal [1];
    uVal [0] = ch;
    CharsToGlyphs (1, uVal, xVal);
    return (xVal[0] != 0);
}


struct MapperPair {
    char *encoding;
    X11CharToGlyphMapper *mapper;

    MapperPair() {}
    MapperPair(const char *enc, X11CharToGlyphMapper *map) {
        encoding = (enc != NULL) ? strdup(enc) : NULL;
	mapper = map;
    }
    MapperPair(const MapperPair &rhs) {
        encoding = (rhs.encoding != NULL) ? strdup(rhs.encoding) : NULL;
	mapper = rhs.mapper;
    }
    ~MapperPair() { free(encoding); }

    MapperPair & operator=(const MapperPair &rhs) {
        encoding = (rhs.encoding != NULL) ? strdup(rhs.encoding) : NULL;
	mapper = rhs.mapper;
	return *this;
    }

    friend int operator==(const MapperPair &a, const MapperPair &b) {
        if (a.encoding != NULL && b.encoding != NULL &&
	        strcmp(a.encoding, b.encoding) == 0) {
	    return 1;
	} else if (a.encoding == NULL && b.encoding == NULL) {
	    return 1;
	} else {
	    return 0;
	}
    }
};

static hsDynamicArray<MapperPair>* x11Mappers = nil;


void MakeNativeMapper (const char *xlfdString) 
{
#ifdef USE_NATIVE_RASTERIZER

    const char *tmpEncoding;
    const char *tmpFamily;
    const char *family = NULL;
    const char *encoding = NULL;

    char *end, *start;
    char xlfd [512];
    int useDefault = 0;

    strcpy (xlfd, xlfdString);
    end = xlfd;

    do {
	NEXT_HYPHEN; /* skip FOUNDRY */
	NEXT_HYPHEN; /* set FAMILY_NAME */
	family = start;
	NEXT_HYPHEN; /* skip STYLE_NAME */
	NEXT_HYPHEN; /* skip SLANT */
	NEXT_HYPHEN; /* skip SETWIDTH_NAME */
	NEXT_HYPHEN; /* skip ADD_STYLE_NAME */
	NEXT_HYPHEN; /* skip PIXEL_SIZE */
	NEXT_HYPHEN; /* skip POINT_SIZE */
	NEXT_HYPHEN; /* skip RESOLUTION_X */
	NEXT_HYPHEN; /* skip RESOLUTION_Y */
 	NEXT_HYPHEN; /* skip SPACING */
	NEXT_HYPHEN; /* skip AVERAGE_WIDTH */
    } while (0);

    end++;

    if (useDefault) {
	encoding = "iso8859-1";
	family = encoding;
    } else {
        encoding = end;
	// Convert the encoding string to lower case.
	// XLFD names should be ASCII, but can be upper or lower case.
	char *p;
	for (p = end; *p != '\0'; ++p) {
	    if (isupper(*p))
		*p = tolower(*p);
	}
    }

    tmpFamily = family;
    tmpEncoding = encoding;

    if (strstr (tmpEncoding, "fontspecific") != 0) {
	if (strstr (tmpFamily, "dingbats") != 0) {
	    tmpEncoding = "dingbats";
	} else if (strstr (tmpFamily, "symbol") != 0) {
	    tmpEncoding = "symbol";
	} else {
	    tmpEncoding = "iso8859-1";
	}
    } else if (strstr (tmpEncoding, "dingbats") != 0) {
	tmpEncoding = "dingbats";
    } else if (strstr (tmpEncoding, "symbol") != 0) {
        tmpEncoding = "symbol";
    }

    X11CharToGlyphMapper *mapper;
    int found = -1;

    if (x11Mappers == nil) {
        x11Mappers = new hsDynamicArray<MapperPair>;
    } else {
	found = x11Mappers->Find (MapperPair (tmpEncoding, nil));
    }

    if (found >= 0) {
	return;
    } else {
	char *psPart = NULL;
        const char *jclass = NULL;
	int nBytes = SINGLE_BYTE;
      	Unicode maxU = 0xffff;
	Unicode minU = 0;
	Boolean addAscii = false;
	Boolean lowPartOnly = false;

    	if (strcmp (tmpEncoding, "iso8859-1") == 0) {
	    maxU = 0xff;
    	} else if (strcmp (tmpEncoding, "iso8859-2") == 0) {
	    jclass = "sun/io/CharToByteISO8859_2";
    	} else if (strcmp (tmpEncoding, "jisx0208.1983-0") == 0) {
	    jclass = "sun/awt/motif/CharToByteX11JIS0208";
	    nBytes = DOUBLE_BYTE;
	    psPart = strdup ("-H");
        } else if (strcmp (tmpEncoding, "jisx0201.1976-0") == 0) {
	    jclass = "sun/awt/motif/CharToByteX11JIS0201";
	    // this is mapping the latin supplement range 128->255 which
	    // doesn't exist in JIS0201. This needs examination.
	    // it was also overwriting a couple of the mappings of
	    // 7E and A5 which in JIS201 are different chars than in
	    // Latin 1. I have revised AddAscii to not overwrite chars
	    // which are already converted.
	    addAscii = true;
	    lowPartOnly = true;
	    psPart = strdup (".Hankaku");
    	} else if (strcmp (tmpEncoding, "jisx0212.1990-0") == 0) {
	    jclass = "sun/awt/motif/CharToByteX11JIS0212";
	    nBytes = DOUBLE_BYTE;
	    psPart = strdup ("-Hojo-H");
  	} else if (strcmp (tmpEncoding, "dingbats") == 0) {
	    jclass = "sun/awt/motif/CharToByteX11Dingbats";
	    minU = 0x2701;
	    maxU = 0x27be;
	} else if (strcmp (tmpEncoding, "symbol") == 0) {
	    jclass = "sun/awt/CharToByteSymbol";
	    minU = 0x0391;
	    maxU = 0x22ef;
        } else if (strcmp (tmpEncoding, "iso8859-4") == 0) {
	    jclass = "sun/io/CharToByteISO8859_4";
    	} else if (strcmp (tmpEncoding, "iso8859-5") == 0) {
	    jclass = "sun/io/CharToByteISO8859_5";
    	} else if (strcmp (tmpEncoding, "koi8-r") == 0) {
	    jclass = "sun/io/CharToByteKOI8_R";
    	} else if (strcmp (tmpEncoding, "ansi-1251") == 0) {
	    jclass = "sun/io/CharToByteCp1251";
    	} else if (strcmp (tmpEncoding, "iso8859-6") == 0) {
	    jclass = "sun/io/CharToByteISO8859_6";
    	} else if (strcmp (tmpEncoding, "iso8859-7") == 0) {
	    jclass = "sun/io/CharToByteISO8859_7";
    	} else if (strcmp (tmpEncoding, "iso8859-8") == 0) {
	    jclass = "sun/io/CharToByteISO8859_8";
        } else if (strcmp (tmpEncoding, "iso8859-9") == 0) {
	    jclass = "sun/io/CharToByteISO8859_9";
    	} else if (strcmp (tmpEncoding, "iso8859-13") == 0) {
	    jclass = "sun/io/CharToByteISO8859_13";
    	} else if (strcmp (tmpEncoding, "iso8859-15") == 0) {
	    jclass = "sun/io/CharToByteISO8859_15";
        } else if (strcmp (tmpEncoding, "ksc5601.1987-0") == 0) {
	    jclass ="sun/awt/motif/CharToByteX11KSC5601";
	    nBytes = DOUBLE_BYTE;
	    psPart = strdup ("-KSC-H");
        } else if (strcmp (tmpEncoding, "ksc5601.1992-3") == 0) {
 	    jclass ="sun/awt/motif/CharToByteX11Johab";
	    nBytes = DOUBLE_BYTE;
	    psPart = strdup ("-KSC-Johab-H");
        } else if (strcmp (tmpEncoding, "ksc5601.1987-1") == 0) {
	    jclass ="sun/io/CharToByteEUC_KR";
	    nBytes = DOUBLE_BYTE;
	    psPart = strdup ("-KSC-EUC-H");
    	} else if (strcmp (tmpEncoding, "cns11643-1") == 0) {
	    jclass = "sun/awt/motif/CharToByteX11CNS11643P1";
	    nBytes = DOUBLE_BYTE;
	    psPart = strdup ("-CNS1-H");
    	} else if (strcmp (tmpEncoding, "cns11643-2") == 0) {
	    jclass = "sun/awt/motif/CharToByteX11CNS11643P2";
	    nBytes = DOUBLE_BYTE;
	    psPart = strdup ("-CNS2-H");
        } else if (strcmp (tmpEncoding, "cns11643-3") == 0) {
	    jclass = "sun/awt/motif/CharToByteX11CNS11643P3";
	    nBytes = DOUBLE_BYTE;
	    psPart = strdup ("-CNS3-H");
    	} else if (strcmp (tmpEncoding, "gb2312.1980-0") == 0) {
	    jclass = "sun/awt/motif/CharToByteX11GB2312";
	    nBytes = DOUBLE_BYTE;
	    psPart = strdup ("-GB-H");
	} else if (strstr (tmpEncoding, "big5") != 0) {
	    jclass = "sun/io/CharToByteBig5";
	    nBytes = DOUBLE_BYTE;
	    psPart = strdup ("-B5-H");
	    addAscii = true;
        } else if (strcmp (tmpEncoding, "tis620.2533-0") == 0) {
	    jclass = "sun/io/CharToByteTIS620";
    	} else if (strcmp (tmpEncoding, "gbk-0") == 0) {
	    jclass = "sun/awt/motif/CharToByteX11GBK";
	    nBytes = DOUBLE_BYTE;
	    psPart = strdup ("-GBK-EUC-H");
        } else if (strstr (tmpEncoding, "sun.unicode-0") != 0) {
	    jclass = "sun/awt/motif/CharToByteX11SunUnicode_0";
	    nBytes = DOUBLE_BYTE;
	} else if (strstr (tmpEncoding, "gb18030.2000-1") != 0) {
	    jclass = "sun/awt/motif/CharToByteX11GB18030_1";
	    nBytes = DOUBLE_BYTE;
	} else if (strstr (tmpEncoding, "gb18030.2000-0") != 0) {
	    jclass = "sun/awt/motif/CharToByteX11GB18030_0";
	    nBytes = DOUBLE_BYTE;
	} else if (strstr (tmpEncoding, "hkscs") != 0) {
	    jclass = "sun/io/CharToByteHKSCS";
	    nBytes = DOUBLE_BYTE;
#ifdef DEBUG
	} else {
	    fprintf(stderr, "Unable to find CharToByte encoder for %s.\n",
		    tmpEncoding);
#endif
	}
    
	mapper = new X11CharToGlyphMapper ((char*)jclass, nBytes, psPart, maxU, minU);
	if (addAscii) {
	    mapper->AddAsciiChars (lowPartOnly);
	}
	x11Mappers->Append (MapperPair (tmpEncoding, mapper));
    }
#endif
}

#define NO_POINTSIZE -1.0

X11nrScalerContext::X11nrScalerContext(
                        fontObject *fo, const float matrix[],
                        Boolean doAntiAlias, Boolean doFractEnable,
			int style, int index)
    : fFont(fo), fDoAntiAlias(doAntiAlias), fDoFracEnable(doFractEnable),
      ptSize (NO_POINTSIZE), fNumGlyphs (-1), encIndex(index), fMapper (0), 
      fDoPaths(true), pathCache (0), pathAdvances (0), fXFont (NULL), t2k (0),
      uset2k (false)
{
    fMatrix[0] = matrix[0];
    fMatrix[1] = matrix[1];
    fMatrix[2] = matrix[2];
    fMatrix[3] = matrix[3];

    hsGScalerContext::fPathType = hsPathSpline::kCubicType;

    /* Determine X11 point size to use for this font. If the matrix
     * is one that X11 can't handle (arbitrary rotation, or different
     * x and y scaling, and the font isn't kNRFontFormat, let the
     * t2k scaler context handle it. If the font is a kNRFontFormat
     * one, we'll do the best we can to figure out some point size
     * to use. Currently, the only case where the nrScalerContext
     * is used for a TrueType font is the case where the matrix
     * is something like [12.0 0.0 0.0 12.0]. Once I add support
     * for simple rotation (90/180/270) and flipping (x or y axis)
     * then this can be expanded.
     */

    float nrPtSize = NO_POINTSIZE;

    if ((fMatrix [1] == 0.0) && (fMatrix [2] == 0.0)) {

        /* First case. Matrix is something like [12.0 0.0 0.0 12.0].
	 * We can handle this. 
	 * NOTE: can change the 'if' to this:
	 *    if (abs (fMatrix [0]) == abs (fMatrix [3])) {
	 * once I can handle flipping of the bitmaps.
	 */

	if ((fMatrix [0] == fMatrix [3]) && (fMatrix [0] > 0)) {
	    ptSize = fabs (fMatrix [0]);

	    if (ptSize > 1.0) {
	        nrPtSize = ptSize;
	    }

	/* Else, matrix is something like [14.0 0.0 0.0 12.0] or
	 * [12.0 0.0 0.0 -12.0]. Use fMatrix [0] for point size right 
         * now for F3 fonts.
	 */

	} else if (fabs (fMatrix [3]) > fabs (fMatrix [0])) {
	    ptSize = fabs (fMatrix [3]);
	} else {
	    ptSize = fabs (fMatrix [0]);
	}

    /* If the matrix is anything like [0.0 12.0 12.0 0.0] or
     * [12.0 12.0 12.0 12.0], let the t2k Scaler handle this.
     * For F3, use the larger of fMatrix [0] and fMatrix [3].
     */
    } else if ((fMatrix [0] == 0.0) && (fMatrix [3] == 0.0)) { 
	if (fabs (fMatrix [1]) > fabs (fMatrix [2])) {
	    ptSize = fabs (fMatrix [1]);
	} else {
	    ptSize = fabs (fMatrix [2]);
	}
    } else {
	ptSize = fabs (fMatrix [0]);
	if (fabs (fMatrix [1]) > ptSize) {
	    ptSize = fabs (fMatrix [1]);
	}
	if (fabs (fMatrix [2]) > ptSize) {
	    ptSize = fabs (fMatrix [2]);
	}
	if (fabs (fMatrix [3]) > ptSize) {
	    ptSize = fabs (fMatrix [3]);
	}
    }
	
    /* If Font Format is kTrueTypeFontFormat or kType1FontFormat
     * this means we have access to the font file and can use t2k.
     * This is a useful backstop in the case that we get here with
     * a string that cannot be handled entirely by the native scaler
     * typically because the char isn't in that X11 encoding even
     * though there's really a suitable glyph in the font.
     * Most cases on Solaris specifically should be caught by CStrike
     * which can direct them to the appropriate native scaler.
     * On Linux we go a step futher and say always use T2K, although it
     * should never get here on Linux now as we don't use NR for fonts of
     * these formats.
     */
    if (debugFonts) {
	fprintf(stderr,"NR scaler initialising a T2K context\n");
    }
    if (fFont->GetFormat () == kTrueTypeFontFormat ||
	fFont->GetFormat () == kType1FontFormat) {
        hsGScalerContext::fPathType = hsPathSpline::kQuadType;
	t2k = new t2kScalerContext (fo, matrix, doAntiAlias, doFractEnable, 
				    style);
#ifdef __linux__
        uset2k = true;
#endif
    }
}

X11nrScalerContext::~X11nrScalerContext()
{
    if (fXFont != NULL) {
        AWTFreeFont (fXFont);
	fXFont = NULL;
    }
    if (fMapper != NULL) {
	delete fMapper;
    }
    if (t2k != NULL) {
	delete t2k;
	t2k = NULL;
    }
    if (pathAdvances != NULL) {
	for (int i = 0; i < fNumGlyphs; i++) {
	    if (pathAdvances [i] != NULL) {
		delete pathAdvances [i];
	    }
	}
	free (pathAdvances);
	pathAdvances = NULL;
    }
    if (pathCache != NULL) {
	for (int i = 0; i < fNumGlyphs; i++) {
	    if (pathCache [i] != NULL) {
		hsPathSpline::Delete (pathCache [i]);
		delete pathCache [i];
	    }
	}
	free (pathCache);
	pathCache = NULL;
    }
}

// CountGlyphs doubles as way of getting a native font reference
// and telling if its valid. So far as I can tell GenerateImage etc
// just return if this "initialisation method" hasn't been called.
// So clients of this class need to call CountGlyphs() right after
// construction to be safe.
UInt32
X11nrScalerContext::CountGlyphs()
{

#ifndef USE_NATIVE_RASTERIZER
    return 0;
#endif

    if (fNumGlyphs == -1) {

   	/* If using the t2kScalerContext, return 0, and let that
	 * figure out the number of glyphs.
	 */
        if (uset2k) {
	    return 0;
	    //fNumGlyphs = t2k->CountGlyphs ();
	    //return fNumGlyphs;
	}

	/* If we couldn't figure out a valid X11 point size, return 0
	 * glyphs so the default font can be used instead.
	 */
	if (ptSize == NO_POINTSIZE) {
	    fNumGlyphs = 0;
	    return 0;
	} 

	/* Get the xlfd string */

	char tmpName [512];	/* Should be large enough */
	char xlfd [512];
	strcpy (xlfd, fFont->GetFontNativeName(encIndex));

	/* If xlfd has a "%d" in it (for point size, then we can
	 * just set it. If not, then we have to replace the '0'
	 * for point size with the correct value. 
	 */

	if (strstr (xlfd, "%d") != 0)
	    sprintf (tmpName, xlfd, (int)(ptSize * 10.0));
	else {
	    char *end, *start;
	    const char *lhs, *rhs;
	    const char *family, *style, *slant, *encoding, *pixelsize = NULL,
	         *setwidth, *addstyle, *pointsize = NULL, *spacing, *foundry;
	    int useDefault = 0;

	    end = xlfd;
	    lhs = xlfd;
	    do {
	        SKIP_HYPHEN;	/* Set FOUNDRY */
	        SKIP_HYPHEN;	/* Set FAMILY_NAME */
		SKIP_HYPHEN;	/* Set Style Name */
		SKIP_HYPHEN;	/* Set Slant */
		SKIP_HYPHEN;	/* Set SETWIDTH_NAME */
		NEXT_HYPHEN;	/* Set ADD_STYLE_NAME */
		NEXT_HYPHEN;	/* Get pixel size */
		pixelsize = start;
		NEXT_HYPHEN;	/* Get point size */
		pointsize = start;
	    } while (0);

	    /* This shouldn't happen, but if useDefault = 1, then 
	     * something went wrong parsing this xlfd string. If this
	     * is not  kNRFontFormat, use the t2kScaler, else  
	     * use default font string.
	     */
	    if (useDefault) {
        	if (fFont->GetFormat () == kTrueTypeFontFormat) {
		    uset2k = true;
		    return 0;
		    //fNumGlyphs = t2k->CountGlyphs ();
		    //return fNumGlyphs;
		}
		lhs = "-*-*-*-*-*-*";
		pointsize = "*";
		pixelsize = "*";
	 	rhs = "0-0-p-0-iso8859-1";
	    } else {
	        rhs = end + 1;
	    }

	    /* See if either point or pixel size was valid */
	    if (((strlen (pointsize) == 1) && 
		 ((pointsize[0] == '*') || (pointsize[0] == '0') ||
		  (pointsize[0] == (char) NULL))) &&
		((strlen (pixelsize) == 1) &&
		 ((pixelsize[0] == '*') || (pixelsize[0] == '0') ||
		  (pixelsize[0] == (char) NULL)))) 
	    {
		char c= pixelsize [0];
		if (c == (char) NULL) {
		    c = '0';
		}
		sprintf (tmpName, "%s-%c-%d-%s", lhs, c, 
			 	((int)(ptSize * 10.0)), rhs);
	    } else {	/* Its ok, use as is */
		strcpy (tmpName, xlfd);
	    }
	}

	/* OK, its a valid X11 font */
	AWTLoadFont (tmpName, &fXFont);

	/* If we didn't get the XFontStruct and we're not dealing
	 * with a kNRFontFormat, then use the t2k ScalerContext
	 * instead. 
	 */
	if (fXFont == NULL) {	/* Couldn't find the font */
       	    if (fFont->GetFormat () == kTrueTypeFontFormat) {
		uset2k = true;
		return 0;
		//fNumGlyphs = t2k->CountGlyphs ();
		//return fNumGlyphs;
	    }
	    fNumGlyphs = 0;
	    return 0;
	}

	fNumGlyphs = (AWTFontMaxByte1(fXFont) << 8) +
            AWTFontMaxCharOrByte2(fXFont) + 1;

    }
    if (t2k != NULL) {
	return t2k->CountGlyphs ();
    }
    
    return fNumGlyphs;
}

Boolean
X11nrScalerContext::canRotate () 
{
    if ((uset2k) && (t2k != NULL)) {
	return t2k->canRotate ();
    }

    return false;
}

void
X11nrScalerContext::GenerateMetrics(
                UInt16 glyphID,
                hsGGlyph* glyph,
                hsFixedPoint2* advance)
{
    UInt16 xGlyphID = glyphID;
    
    /* If t2k is not null, glyphID is valid for the t2kScalerContext. */
    /* To be consistent we really want to always use T2K for metrics if
     * at all possible. There's some small possibility that the T2K metrics
     * aren't 100% exactly the same as the native metrics but if there is
     * a discrepancy we'd be hit by it when we do need to use T2k for the
     * image as well. So our decision is to treat the native scaler solely
     * as the way we get bitmaps, (if we must!) Hence we have commented
     * out the condition that uset2k must be set before we use T2K.
     */
    if (t2k != NULL) {
 	if (uset2k) {
	    t2k->GenerateMetrics (glyphID, glyph, advance);
	    return;
 	}
 	xGlyphID = ((nrCharToGlyphMapper*)getMapper())->getXCharCode (glyphID);
 	if (xGlyphID == 0xffff) {
 	    t2k->GenerateMetrics (glyphID, glyph, advance);
 	    return;
 	}
    } 

    /* IF any of the following conditions are true, we can't retrieve
     * the metrics, so return zeros. Note we don't want to return
     * kNoMetrics_Width in the fWidth field because if we do, we'll
     * keep getting called and since we can't get the metrics, there's
     * no point to that.
     */
    if ((xGlyphID >= fNumGlyphs) || (fXFont == NULL) || (ptSize == NO_POINTSIZE)) {
        if (glyph) {
	    memset(glyph, 0, sizeof(*glyph));
        }
        if (advance) {
            memset(advance, 0, sizeof(*advance));
        }
        return;
    }

    int fMinIndex =  (AWTFontMinByte1(fXFont) << 8) +
        AWTFontMinCharOrByte2(fXFont);

    if (xGlyphID < fMinIndex) {
	/* Sometimes the default_char field of the XFontStruct isn't 
	 * initialized to anything, so it can be a large number. So, I 
	 * check to see if its less than the largest possible value
	 * (which happens to be fNumGlyphs) and if so, then I use it.
	 * Otherwise, I just use the fMinIndex.
	 */

	if ((AWTFontDefaultChar(fXFont) != 0) && 
	    (((int)(AWTFontDefaultChar(fXFont))) < fNumGlyphs))
	{
	    xGlyphID = AWTFontDefaultChar(fXFont);
	} else {
	    xGlyphID = fMinIndex;
	}
    }

    AWTChar xcs;
    jboolean shouldFree = JNI_FALSE;
	
    /* If number of glyphs is 256 or less, the metrics are
     * stored correctly in the XFontStruct for each
     * character. If the # characters is more (double byte
     * case), then these metrics seem flaky and theres no
     * way to determine if they have been set or not. 
     */
    if ((fNumGlyphs <= 256) && (AWTFontPerChar(fXFont, 0) != NULL)) {
        xcs = AWTFontPerChar(fXFont, xGlyphID - AWTFontMinCharOrByte2(fXFont));
    } else {
        int direction, ascent, descent;

    	if (xChar == (AWTChar2b *) NULL) {
            xChar = (AWTChar2b *) calloc (1, sizeof (AWTChar2b));
	    if (xChar == NULL) {
        	if (glyph) {
	    	    memset(glyph, 0, sizeof(*glyph));
        	}
        	if (advance) {
            	    memset(advance, 0, sizeof(*advance));
        	}
		JNIEnv *env = (JNIEnv *) JNU_GetEnv (jvm, JNI_VERSION_1_2);
		JNU_ThrowOutOfMemoryError (env, 
			"Cannot create native data structure.");
		return;
	    }
	}
	
	xChar->byte1 = (unsigned char) (xGlyphID >> 8);
	xChar->byte2 = (unsigned char) xGlyphID;
	AWTFontQueryTextExtents16 (fXFont, xChar, 1, &direction, 
				   &ascent, &descent, &xcs);
        shouldFree = JNI_TRUE;
    }
    advance->fY = 0;
    advance->fX = AWTCharWidth(xcs) << SHIFTFACTOR;
    glyph->fWidth = AWTCharRBearing(xcs) - AWTCharLBearing(xcs);

    glyph->fHeight = AWTCharAscent(xcs) + AWTCharDescent(xcs);
    glyph->fRowBytes = glyph->fWidth;
    glyph->fTopLeft.fX = AWTCharLBearing(xcs) << SHIFTFACTOR;
    glyph->fTopLeft.fY = -AWTCharAscent(xcs) << SHIFTFACTOR;

    if (shouldFree) {
        AWTFreeChar(xcs);
    }
}

void
X11nrScalerContext::GenerateMetricsWithImage(
                UInt16 glyphID,
                hsGGlyph* glyph,
                hsFixedPoint2* advance)
{
    this->GenerateMetrics (glyphID, glyph, advance);
    Int32 imageSize = glyph->fHeight * glyph->fRowBytes;
    void *image = 0;
    if (imageSize >= 0) {
        image = HSMemory::SoftNew(imageSize);
        if (image) {
            this->GenerateImage (glyphID, glyph, image);
        } else {
            HSMemory::Delete(image);
            image = 0;
	}
    } 
    glyph->fImage = image;
}


void
X11nrScalerContext::GenerateImage(
                    UInt16 glyphID,
                    const hsGGlyph* glyph,
                    void* buffer)
{
    UInt16 xGlyphID = glyphID;
    /* If t2k is not null, the glyphID is valid for the t2kScalerContext. */
    if (t2k != NULL) {
	if (uset2k) {
	    t2k->GenerateImage (glyphID, glyph, buffer);
	    return;
	}

	xGlyphID = ((nrCharToGlyphMapper*)getMapper())->getXCharCode (glyphID);
	if (xGlyphID == 0xffff) {
	    t2k->GenerateImage (glyphID, glyph, buffer);
	    return;
	}
    } 

    /* If any of the following conditions are true, then we aren't
     * going to get a valid image, so return an empty one.
     */
    if ((xGlyphID >= fNumGlyphs) || (fXFont == 0) || (ptSize == NO_POINTSIZE) ||
	(glyph->fRowBytes == 0) || (glyph->fHeight == 0)) 
    { 
	if (buffer) {
    	    UInt32 imageSize = glyph->fHeight * glyph->fRowBytes;
	    memset(buffer, 0, imageSize);
        }
        return;
    }

    int fMinIndex =  (AWTFontMinByte1(fXFont) << 8) +
        AWTFontMinCharOrByte2(fXFont);

    if (xGlyphID < fMinIndex) {

	/* Sometimes the default_char field of the XFontStruct isn't 
	 * initialized to anything, so it can be a large number. So, I 
	 * check to see if its less than the largest possible value
	 * (which happens to be fNumGlyphs) and if so, then I use it.
	 * Otherwise, I just use the fMinIndex.
	 */

	if ((AWTFontDefaultChar(fXFont) != 0) && 
	    (AWTFontDefaultChar(fXFont) < (unsigned)fNumGlyphs))
	{
	    xGlyphID = AWTFontDefaultChar(fXFont);
	} else {
	    xGlyphID = fMinIndex;
	}
    }

    int startX = -glyph->fTopLeft.fX >> SHIFTFACTOR;
    int theAscent = -( (glyph->fTopLeft.fY) >> SHIFTFACTOR );
    AWTFontGenerateImage(fXFont, startX, theAscent, glyph->fRowBytes,
        glyph->fWidth, glyph->fHeight, xChar, xGlyphID, buffer);
}

Boolean
CheckFontList (char *name, char *style, char *slant, char *psEnc)
{
    /* This function includes a list of known fonts for which the
     * creation of DPS font name typically does not work. There
     * doesnt seem to be any way around this..
     */  
    if (strcmp (name, "LucidaBright-DemiBold") == 0) {
	strcpy (name, "Lucida-BrightDemiBold");
	return true;
    } else if (strcmp (name, "LucidaBright-DemiBoldItalic") == 0) {
	strcpy (name, "Lucida-BrightDemiBoldItalic");
	return true;
    } else if (strcmp (name, "Times-Bold-Italic") == 0) {
	strcpy (name, "Times-BoldItalic");
	return true;
    } else if (strstr (name, "Gill") != 0) {
        strcpy (name, "GillSans");
    } else if (strstr (name, "NewCenturySchoolbook") != 0) {
        strcpy (name, "NewCenturySchlbk");
    } else if (strstr (name, "Palatino") != 0) {
        strcpy (name, "Palatino-Roman");
    } else if (strstr (name, "chancery") != 0) {
        strcpy (name, "ZapfChancery-MediumItalic");
        return true;
    } else if (strstr (name, "dingbats") != 0) {
	strcpy (name, "ZapfDingbats");
	return true;
    } else if (strstr (name, "Gothic") != 0) {
        strcpy (name, "GothicBBB-Medium");
    } else if (strstr (name, "Ryumin") != 0) {
        strcpy (name, "Ryumin");
    } else if (strstr (name, "Mincho") != 0) {
	strcpy (name, "HG-MinchoL");
    } else if (strstr (name, "Heisei") != 0) {
	strcpy (name, "HeiseiMin-W3H");
	return true;
    } else {
        return false;
    }
 
    if ((style != NULL) || (slant != NULL)) {
        strcat (name, "-");
        if (style != NULL) {
           strcat (name, style);
	}
        if (slant != NULL) {
           strcat (name, slant);
	}
    }
 
    if (psEnc != NULL) {
        strcat (name, psEnc);
    }
 
    return true;
}

void
X11nrScalerContext::GeneratePath(
                    UInt16 index,
                    hsPathSpline* path,
                    hsFixedPoint2* advance)
{
    enum FontFormats format = fFont->GetFormat ();
    UInt16 xIndex = index;

    /* If t2k is not null, the index is for the t2kScalerContext. */
    if (t2k != NULL) {
        /* If this is a TrueType font, then let the
         * t2kScalerContext geneneate the path.
         */
        if (format == kTrueTypeFontFormat) {
	    t2k->GeneratePath (index, path, advance);
	    return;
	}
	xIndex = ((nrCharToGlyphMapper*)getMapper())->getXCharCode (index);
	if (xIndex == 0xffff) {
	    t2k->GeneratePath (index, path, advance);
	    return;
	}
    }

    if (!AWTUseDPS()) {
        return;
    }

    /* NOTE:: This is subject to much error. What we have is
     * xlfd string and what we want is PostScript font name.
     * In some cases, we're lucky and the ADOBE_POSTSCRIPT_NAME
     * was set for the font. If not, we'll try and figure out
     * it out from the FONT_NAME property and do some fun things
     * like remove spaces from font names, try smashing words
     * together, or adding hyphens, capitailizing letters, etc..
     */

    /*
     * 0 is the 'missing glyph' according to the char converter,
     * but we didn't get this from the server.  In order to avoid
     * turning off this context for all glyphs in case of failure,
     * special case this to return a null path.
     */
    if (xIndex == 0 || (!fDoPaths) || (!AWTUseDPS())) {
        if (path) {
	    path->fContourCount = 0;
	    path->fContours = nil;
        }
        if (advance) {
            memset(advance, 0, sizeof(*advance));
	}
	return;
    }

    /* See if the path is already in the cache */

    if (pathCache != NULL) {
	if (pathCache [xIndex] != NULL) {
	    pathCache [xIndex]->Copy (path);
	    if (advance) {
		if (pathAdvances [xIndex] != NULL) {
		    advance->fX = pathAdvances [xIndex]->fX;
		    advance->fY = pathAdvances [xIndex]->fY;
		} else {
		    memset (advance, 0, sizeof (*advance));
		}
	    }
	    return;
	}
    }

    char tempName [512];        /* Should be large enough */

    if (!AWTInitDPS()) {
        /* Throw exception, outline not available */
        fDoPaths = false;
        if (path) {
            path->fContourCount = 0;
            path->fContours = nil;
        }
        if (advance) {
            memset(advance, 0, sizeof(*advance));
        }
        if (AWTDPSError() && format == kNRFontFormat) {
            /* Set PSfontName of X11FontObject since we
             * can't do outlines for this font, so we
             * might as well not try it next time.
             */
            tempName [0] = '\0';
            X11FontObject *xFont = (X11FontObject *) fFont;
            xFont->SetPSName (tempName); 
        }
        return;
    }

    char *psName = NULL;
    
/* 
 * First , check to see if we've already figured out the PostScript
 * font name.
 */

    if (format == kNRFontFormat) {
	X11FontObject *xFont = (X11FontObject *) fFont;
	psName = xFont->GetPSName ();
	if (psName != NULL) {
	    if (psName [0] == '\0') {
	 	fDoPaths = false;
		if (path) {
		    path->fContourCount = 0;
		    path->fContours = nil;
		}
		if (advance) {
		    memset (advance, 0, sizeof (*advance));
		}
		return;
	    } else {
		strcpy (tempName, psName);
	    }
	}
    }
	
    if (psName == NULL) {

    /*
     * Check ADOBE_POSTSCRIPT_NAME font property first. If this is set,
     * then this is the correct PostScript font name to use.. it even
     * has the correct encoding if its a CJK font.
     */
        char *tmpStyle = NULL;
        char *tmpSlant = NULL;

        Boolean found = AWTGetDPSPath(fXFont, tempName);
        Boolean foundWithEncoding = found;
        if (!found) {
	    /* 
 	     * If no property set, then if an F3 font, use the slant and style
 	     * from the XLFD string to try and create a PostScript font name.
 	     */
	    if (format == kNRFontFormat) {
        	X11FontObject *xFont = (X11FontObject *) fFont;
		char *tmpString = xFont->GetFamilyName ();
		if (tmpString != NULL) {
                    found = true;
	            strcpy (tempName, tmpString);
		    tmpStyle = xFont->GetStyle ();
		    tmpSlant = xFont->GetSlant ();
	            if ((tmpStyle != NULL) || (tmpSlant != NULL)) {
		    	strcat (tempName, " ");
		    	if (tmpStyle != NULL) {
		       	    strcat (tempName, tmpStyle);
		    	}
		    	if (tmpSlant != NULL) {
		            strcat (tempName, tmpSlant);
		    	}
		    }
		}
	    }

	    if (!found) {
	        /* Throw exception, outline not available */
	        fDoPaths = false;
    	        if (path) {
		    path->fContourCount = 0;
		    path->fContours = nil;
	        }
    	        if (advance) {
		    memset (advance, 0, sizeof (*advance));
	        }
                return;
	    }
	}
	

	/* Now, remove spaces from font names and replace with
	 * hyphens. PostScript font names don't have spaces in
	 * them, but they do have hyphens.
	 */

	char *a = tempName;
	*a = toupper (*a);
	while (*++a != 0) {
	    if (*a == ' ') {
		*a = '-';
	    }
	}
    
	/* If theres a PS encoding, then we need to append it */
    
	char *psPart = ((nrCharToGlyphMapper*)getMapper())->GetPSEncoding ();
	if (psPart != NULL && !foundWithEncoding) {
	    strcat (tempName, psPart);
	}

        short foundFont;
 	AWTCheckForFont (tempName, &foundFont);
        if (AWTDPSError()) {
	    /* Throw exception, outline not available */
            fDoPaths = false;
            if (path) {
                path->fContourCount = 0;
                path->fContours = nil;
            }
            if (advance) {
                memset(advance, 0, sizeof(*advance));
            }
            if (format == kNRFontFormat) {
                /* Set PSfontName of X11FontObject since we
                 * can't do outlines for this font, so we
                 * might as well not try it next time.
                 */
                tempName [0] = '\0';
                X11FontObject *xFont = (X11FontObject *) fFont;
                xFont->SetPSName (tempName); 
            }
            return;
        }

        if (!foundFont) {

	    /* If not found, see if its one of the font names we have
	     * problems with.
	     */
	    foundFont = CheckFontList (tempName, tmpStyle, tmpSlant, 
					   psPart);

	    if (foundFont)
		AWTCheckForFont (tempName, &foundFont);

	    if ((!foundFont) || (AWTDPSError())) {

	        /* Throw exception ?? */
 	        fDoPaths = false;
    	        if (path) {
		    path->fContourCount = 0;
		    path->fContours = nil;
    	        }
	        if (advance) {
		    memset (advance, 0, sizeof (*advance));
	        }

                if (format == kNRFontFormat) {
                    /* Set PSfontName of X11FontObject since we
                     * can't do outlines for this font, so we
                     * might as well not try it next time.
                     */
                    tempName [0] = '\0';
                    X11FontObject *xFont = (X11FontObject *) fFont;
                    xFont->SetPSName (tempName); 
                }

                return;
	    }
        }
        
        /* Set the PS name of the X11FontObject so we don't have to
         * figure it out again.
         */
        if (format == kNRFontFormat) {
    	    X11FontObject *xFont = (X11FontObject *) fFont;
            xFont->SetPSName (tempName);
	}
    }

    /* OK now generate the outline */

    unsigned char xByte1 = (unsigned char) (xIndex >> 8);
    unsigned char xByte2 = (unsigned char) xIndex;
    int nOps, nPoints, nPaths;   

    float tmpPtSize = ptSize;
/*
    if ((format == kNRFontFormat) && (ptSize < 100.0)) {
	tmpPtSize = 100.0;
    }
*/
    if ((format == kNRFontFormat) && (ptSize < 10.0)) {
	tmpPtSize = 20.0;
    }
    AWTDrawOutline (tempName, tmpPtSize, xByte1, xByte2,
		 &nOps, &nPoints, &nPaths);

    if (AWTDPSError()) {
	/* Throw exception, outline not available */
	fDoPaths = false;
        if (path) {
	    path->fContourCount = 0;
	    path->fContours = nil;
        }
        if (advance) {
            memset(advance, 0, sizeof(*advance));
	}

	if (format == kNRFontFormat) {
	    /* Set PSfontName of X11FontObject since we
             * can't do outlines for this font, so we
	     * might as well not try it next time.
	     */
	    X11FontObject *xFont = (X11FontObject *) fFont;
	    tempName [0] = '\0';
	    xFont->SetPSName (tempName); 
	}
	return;
    }

    int *ops = (int *) calloc (1, sizeof (int) * nOps);
    if (ops == NULL) {
        if (path) {
	    path->fContourCount = 0;
	    path->fContours = nil;
        }
        if (advance) {
            memset(advance, 0, sizeof(*advance));
        }
	JNIEnv *env = (JNIEnv *) JNU_GetEnv (jvm, JNI_VERSION_1_2);
	JNU_ThrowOutOfMemoryError (env, 
			"Cannot create native data structure.");
	return;
    }

    int *pathPoints = (int *) calloc (1, sizeof (int) * nPaths);
    if (pathPoints == NULL) {
        if (path) {
	    path->fContourCount = 0;
	    path->fContours = nil;
        }
        if (advance) {
            memset(advance, 0, sizeof(*advance));
        }
	JNIEnv *env = (JNIEnv *) JNU_GetEnv (jvm, JNI_VERSION_1_2);
	JNU_ThrowOutOfMemoryError (env, 
			"Cannot create native data structure.");
	free (ops);
	return;
    }

    float *points = (float *) calloc (1, sizeof (float) * nPoints);
    if (points == NULL) {
        if (path) {
	    path->fContourCount = 0;
	    path->fContours = nil;
        }
        if (advance) {
            memset(advance, 0, sizeof(*advance));
        }
	JNIEnv *env = (JNIEnv *) JNU_GetEnv (jvm, JNI_VERSION_1_2);
	JNU_ThrowOutOfMemoryError (env, 
			"Cannot create native data structure.");
	free (ops);
	free (pathPoints);
	return;
    }

    AWTGetOutline (nOps, nPoints, nPaths, ops, pathPoints, points);

    if (AWTDPSError()) {
	/* Throw exception, outline not available */
	fDoPaths = false;
        if (path) {
	    path->fContourCount = 0;
	    path->fContours = nil;
        }
        if (advance) {
            memset(advance, 0, sizeof(*advance));
	}

	if (format == kNRFontFormat) {
	    /* Set PSfontName of X11FontObject since we
             * can't do outlines for this font, so we
	     * might as well not try it next time.
	     */
	    X11FontObject *xFont = (X11FontObject *) fFont;
	    tempName [0] = '\0';
	    xFont->SetPSName (tempName); 
	}
	free (ops);
	free (pathPoints);
	free (points);
	return;
    }

    path->fContourCount = nPaths;
    if (nPaths == 0) {
        path->fContours = nil;
        if (advance) {
            memset(advance, 0, sizeof(*advance));
	}
	free (ops);
	free (pathPoints);
	free (points);
        return;
    }
    path->fContours = new hsPathContour[nPaths];

    int i;
    for (i = 0; i < nPaths; i++) {
        int ctrlBitLongs = hsPathContour::ControlBitLongs(pathPoints [i]);
        path->fContours[i].fPointCount = pathPoints [i];
        path->fContours[i].fPoints  = new hsPoint2 [pathPoints [i]];
        path->fContours[i].fControlBits  = new UInt32 [ctrlBitLongs];
 
        HSMemory::Clear(path->fContours[i].fControlBits, 
			ctrlBitLongs * sizeof(UInt32));
    }
 
    if (tmpPtSize != ptSize) {
	float f = ptSize / tmpPtSize;
	for (int i = 0; i < nPoints; i++) {
	    points[i] *= f;
	}
    }

    /* OK, now loop through the ops array. Ops values are:
     *	 1: moveto
     *   2: lineto
     *   3: curveto
     *   4: closepath
     * Associated points are in the points array. Remember that
     * the values are returned in the order they are to be used,
     * ie, points[0] is the X value for the first operation (most
     * likely a moveto) and points[1] is the Y value.
     */

    float curX = 0, curY = 0;
    float newX, newY;
    int iPath = 0;
    int pointCount = 0;
    int pointIndex = 0;
    int ctrlBitIndex;
    int ctrlBitShift;

    for (i = 0; i < nOps; i++) {
	switch (ops [i]) {
	    case MOVETO:
		curX = points [pointIndex++];
		curY = points [pointIndex++];
		break;

	    case LINETO:

		if ((pointCount == 0) || 
		    (path->fContours[iPath].fPoints[pointCount-1].fX != curX) ||
		    (path->fContours[iPath].fPoints[pointCount-1].fY != -curY))
		{
		    path->fContours [iPath].fPoints [pointCount].fX = curX;
		    path->fContours [iPath].fPoints [pointCount].fY = -curY;
		    pointCount++;
		}

		newX = points [pointIndex++];
		newY = points [pointIndex++];

		path->fContours [iPath].fPoints [pointCount].fX = newX;
		path->fContours [iPath].fPoints [pointCount].fY = -newY;
		pointCount++;

		curX = newX;
		curY = newY;
		break;

	    case CURVETO:

		if ((pointCount == 0) || 
		    (path->fContours[iPath].fPoints[pointCount-1].fX != curX) ||
		    (path->fContours[iPath].fPoints[pointCount-1].fY != -curY))
		{
		    path->fContours [iPath].fPoints [pointCount].fX = curX;
		    path->fContours [iPath].fPoints [pointCount].fY = -curY;
		    pointCount++;
		}

		newX = points [pointIndex++];
		newY = points [pointIndex++];

		path->fContours [iPath].fPoints [pointCount].fX = newX;
		path->fContours [iPath].fPoints [pointCount].fY = -newY;
		ctrlBitIndex = pointCount >> 5;
		ctrlBitShift = 31 - (pointCount & 31);
		path->fContours [iPath].fControlBits [ctrlBitIndex] |=
				(1UL << ctrlBitShift);
		pointCount++;

		newX = points [pointIndex++];
		newY = points [pointIndex++];

		path->fContours [iPath].fPoints [pointCount].fX = newX;
		path->fContours [iPath].fPoints [pointCount].fY = -newY;
		ctrlBitIndex = pointCount >> 5;
		ctrlBitShift = 31 - (pointCount & 31);
		path->fContours [iPath].fControlBits [ctrlBitIndex] |=
				(1UL << ctrlBitShift);
		pointCount++;

		newX = points [pointIndex++];
		newY = points [pointIndex++];

		path->fContours [iPath].fPoints [pointCount].fX = newX;
		path->fContours [iPath].fPoints [pointCount].fY = -newY;
		pointCount++;

		curX = newX;
		curY = newY;
		break;

	    case CLOSEPATH:
		/* If pointcount is 0, then this "path" had nothing in it. 
		 * So, decrease the number of paths by one, and delete
		 * the fContour entry.
		 */
		if (pointCount == 0) {
		    if (path->fContourCount > 0) {
			int lastEntry = path->fContourCount - 1;
    		        path->fContourCount = path->fContourCount - 1;
			delete [] path->fContours [lastEntry].fPoints;
			delete [] path->fContours [lastEntry].fControlBits;
			if (path->fContourCount == 0) {
			    delete [] path->fContours;
			    path->fContours = NULL;
			}
		    }
		} else {
		    if ((path->fContours [iPath].fPoints [0].fX ==
		         path->fContours [iPath].fPoints [pointCount -1].fX) &&
		        (path->fContours [iPath].fPoints [0].fY ==
		         path->fContours [iPath].fPoints [pointCount -1].fY)) 
		    {
		        path->fContours [iPath].fPointCount = pointCount - 1;
		    }

		    iPath++;
		}

		pointCount = 0;
		break;
	
	    default:
		break;
	}
    }

    /* At the end, curX, curY is the advance values */
    if (advance) {
	advance->fX = ((int) curX) << SHIFTFACTOR;
	advance->fY = ((int) curY) << SHIFTFACTOR;
    }

    if (pathCache == NULL) {
	pathCache = (hsPathSpline **) calloc (1, sizeof (hsPathSpline *) *
					      fNumGlyphs);
	pathAdvances = (hsFixedPoint2 **) calloc (1, sizeof (hsFixedPoint2 *) *
					      fNumGlyphs);
    }

    pathCache [xIndex] = path->Copy (nil);
    pathAdvances [xIndex] = new hsFixedPoint2;
    pathAdvances [xIndex]->fX = ((int) curX) << SHIFTFACTOR;
    pathAdvances [xIndex]->fY = ((int) curY) << SHIFTFACTOR;
    free (ops);
    free (pathPoints);
    free (points);
}

void
X11nrScalerContext::GetLineHeight(
                    hsFixedPoint2* ascent,
                    hsFixedPoint2* descent,
                    hsFixedPoint2* baseline,
                    hsFixedPoint2* leading,
                    hsFixedPoint2* maxAdvance)
{
    if ((uset2k) && (t2k != NULL)) {
	t2k->GetLineHeight (ascent, descent, baseline,
				leading, maxAdvance);
	return;
    }

    if (fXFont != NULL) {

	if (ascent) {
            ascent->fX = 0;
            ascent->fY = -AWTCharAscent(AWTFontMaxBounds(fXFont))
                << SHIFTFACTOR;
	}
	if (descent) {
            descent->fX = 0;
            descent->fY = AWTCharDescent(AWTFontMaxBounds(fXFont))
                << SHIFTFACTOR;
	}

	/* I have no idea what this should be! */
	if (baseline) {
            baseline->fX = 16384 << SHIFTFACTOR; 
	    baseline->fY = 0;
	}

	if (leading) {
            leading->fX = 0;
            leading->fY = -1 << SHIFTFACTOR;
	}

	if (maxAdvance) {
            maxAdvance->fY = 0;
            maxAdvance->fX = AWTCharWidth(AWTFontMaxBounds(fXFont))
                << SHIFTFACTOR;
	}
	return;
    }

    if (ascent) {
        ascent->fX = 0;
        ascent->fY = 0;
    }

    if (descent) {
        descent->fX = 0;
        descent->fY = 0;
    }

    if (baseline) {
        baseline->fX = 0;
        baseline->fY = 0;
    }

    if (leading) {
        leading->fX = 0;
        leading->fY = 0;
    }

    if (maxAdvance) {
        maxAdvance->fX = 0;
        maxAdvance->fY = 0;
    }
}

void
X11nrScalerContext::GetCaretAngle(hsFixedPoint2* caret)
{
    if (t2k != NULL) {
	t2k->GetCaretAngle (caret);
	return;
    }

    this->GetLineHeight (caret, nil, nil, nil, nil);
    caret->fY = -caret->fY;
}


//////////////////////////////////////////////////////////////////////////////

CharToGlyphMapper*
X11nrScalerContext::getMapper()
{
    if ((t2k != NULL) && (uset2k)) {
	return t2k->getMapper ();
    }
	
    if (fMapper == NULL) {

	char *end, *start, *family = NULL;
	int useDefault = 0;
	char xlfd [512];
	strcpy (xlfd, fFont->GetFontNativeName (encIndex));
	end = xlfd;

	do {
	    NEXT_HYPHEN; /* skip FOUNDRY */
	    NEXT_HYPHEN; /* skip FAMILY_NAME */
	    family = start;
	    NEXT_HYPHEN; /* skip STYLE_NAME */
	    NEXT_HYPHEN; /* skip SLANT */
	    NEXT_HYPHEN; /* skip SETWIDTH_NAME */
	    NEXT_HYPHEN; /* skip ADD_STYLE_NAME */
	    NEXT_HYPHEN; /* skip PIXEL_SIZE */
	    NEXT_HYPHEN; /* skip POINT_SIZE */
	    NEXT_HYPHEN; /* skip RESOLUTION_X */
	    NEXT_HYPHEN; /* skip RESOLUTION_Y */
 	    NEXT_HYPHEN; /* skip SPACING */
	    NEXT_HYPHEN; /* skip AVERAGE_WIDTH */
	} while (0);

	end++;

	const char *encoding;
	if (useDefault) {
	    encoding = "iso8859-1";
	} else {
	    encoding = end;
	}

	/* Special case for dingbats (and symbol when its available) */
	if (strstr (encoding, "fontspecific") != 0) {
	    if (strstr (family, "dingbats") != 0) {
	        encoding = "dingbats";
	    } else if (strstr (family, "symbol") != 0) {
	        encoding = "symbol";
	    } else {
		encoding = "iso8859-1";
	    }
	} else if (strstr (encoding, "dingbats") != 0) {
	    encoding = "dingbats";
	} else if (strstr (encoding, "symbol") != 0) {
	    encoding = "symbol";
	}
	
	int found = -1;
	if (x11Mappers == nil) {
	    x11Mappers = new hsDynamicArray<MapperPair>;
	} else {
	    found = x11Mappers->Find (MapperPair (encoding, nil));
	}

	if (found < 0) {
	    // May be it was never added - try to create it and
	    // check once more.
	    char xlfd2[512];
	    strcpy (xlfd2, fFont->GetFontNativeName (encIndex));
	    MakeNativeMapper(xlfd2);
	    if (debugFonts) {
		fprintf(stderr,"Creating Xmapper for xlfd=%s\n",xlfd2);
	    }
	    found = x11Mappers->Find (MapperPair (encoding, nil));
	}
	X11CharToGlyphMapper *xMapper = NULL;

        if (found >= 0) {
	    xMapper = x11Mappers->Get (found).mapper;
	} else { 	/* Dont know how this could happen */
	    xMapper = x11Mappers->Get (0).mapper;
	}
	
    	int t2kGlyphs = 0;
	CharToGlyphMapper *t2kMapper = NULL;

	if (t2k != NULL) {
	    t2kMapper = t2k->getMapper ();
	    t2kGlyphs = t2k->CountGlyphs ();
	}
        fMapper = new nrCharToGlyphMapper (xMapper, t2kMapper, t2kGlyphs,
					   fNumGlyphs);
    }

    return fMapper;
}

Boolean
X11nrScalerContext::GetGlyphPoint(UInt16 glyphID, Int32 pointNumber, hsFixedPoint2 *point)
{
    if (t2k != NULL) {
        return t2k->GetGlyphPoint(glyphID, pointNumber, point);
    }


    // NOTE: This method will only get called for OpenType or GX
    // fonts, both of which will be TrueType fonts, so I don't
    // expect to ever get here...
    return false;
}

void
X11nrScalerContext::TransformFunits(Int16 xFunits, Int16 yFunits, hsFixedPoint2 *pixels)
{
    if (t2k != NULL) {
        t2k->TransformFunits(xFunits, yFunits, pixels);
    }


    // NOTE: This method will only get called for OpenType or GX
    // fonts, both of which will be TrueType fonts, so I don't
    // expect to ever get here...
    // FIXME: How do we transform the point?
    pixels->fX = hsIntToFixed(xFunits);
    pixels->fY = hsIntToFixed(yFunits);
}

//////////////////////////////////////////////////////////////////////////////

hsGScalerContext*
nrFontScaler::CreateContext(hsConstDescriptor desc)
{
    const FontStrikeDesc *r = (FontStrikeDesc *)hsDescriptor_Find(
                                                        desc,
                                                        kFontStrikeDescTag,
                                                        nil,
                                                        nil);
    return new X11nrScalerContext(
                    r->fFontObj, r->fMatrix,
                    r->fDoAntiAlias, r->fDoFracEnable, r->fStyle, r->fIndex);
}


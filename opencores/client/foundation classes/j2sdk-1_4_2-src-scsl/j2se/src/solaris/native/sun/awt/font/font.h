/*
 * @(#)font.h	1.8 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @author Joe Warzecha
 */

#ifndef _FONT_H_
#define _FONT_H_

#include "gdefs.h"
#ifndef HEADLESS
#include <X11/Xlib.h>
#endif

#define NEXT_HYPHEN\
        start = end + 1;\
        end = strchr(start, '-');\
        if (end == NULL) {\
                              useDefault = 1;\
        break;\
        }\
        *end = '\0'

#define SKIP_HYPHEN\
	start = end + 1;\
	end = strchr(start, '-');\
	if (end == NULL) {\
		useDefault = 1;\
	break;\
	}\

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef HEADLESS

typedef struct {
    unsigned char byte1;
    unsigned char byte2;
} AWTChar2b;

#define Success 1

#else /* !HEADLESS */

extern Display *awt_display;
typedef XChar2b AWTChar2b;

#endif /* !HEADLESS */

typedef void *AWTChar;
typedef void *AWTFont;

extern JavaVM *jvm;

/*
 * Important note : All AWTxxx functions are defined in font.h.
 * These were added to remove the dependency of certain files on X11.
 * These functions are used to perform X11 operations and should
 * be "stubbed out" in environments that do not support X11.
 */
JNIEXPORT void JNICALL AWTLoadFont(char* name, AWTFont* pReturn);
JNIEXPORT void JNICALL AWTFreeFont(AWTFont font);
JNIEXPORT unsigned JNICALL AWTFontMinByte1(AWTFont font);
JNIEXPORT unsigned JNICALL AWTFontMaxByte1(AWTFont font);
JNIEXPORT unsigned JNICALL AWTFontMinCharOrByte2(AWTFont font);
JNIEXPORT unsigned JNICALL AWTFontMaxCharOrByte2(AWTFont font);
JNIEXPORT unsigned JNICALL AWTFontDefaultChar(AWTFont font);
/* Do not call AWTFreeChar() after AWTFontPerChar() or AWTFontMaxBounds() */
JNIEXPORT AWTChar JNICALL AWTFontPerChar(AWTFont font, int index);
JNIEXPORT AWTChar JNICALL AWTFontMaxBounds(AWTFont font);
/* Call AWTFreeChar() on overall after calling AWTFontQueryTextExtents16() */
JNIEXPORT void JNICALL AWTFontQueryTextExtents16(AWTFont font,
    AWTChar2b* xChar, int nchars, int* direction,
    int* ascent, int* descent, AWTChar* overall);
JNIEXPORT void JNICALL AWTFreeChar(AWTChar xChar);
JNIEXPORT void JNICALL AWTFontGenerateImage(AWTFont fXFont,
    int startX, int theAscent, int fRowBytes, int fWidth, int fHeight,
    AWTChar2b* xChar, unsigned short xGlyphID, void* buffer);
JNIEXPORT short JNICALL AWTCharWidth(AWTChar xChar);
JNIEXPORT short JNICALL AWTCharLBearing(AWTChar xChar);
JNIEXPORT short JNICALL AWTCharRBearing(AWTChar xChar);
JNIEXPORT short JNICALL AWTCharAscent(AWTChar xChar);
JNIEXPORT short JNICALL AWTCharDescent(AWTChar xChar);
/* DPS Functions - should be stubbed out when DPS is not used */
JNIEXPORT jboolean JNICALL AWTUseDPS();
JNIEXPORT jboolean JNICALL AWTInitDPS();
JNIEXPORT jboolean JNICALL AWTDPSError();
JNIEXPORT jboolean JNICALL AWTGetDPSPath(AWTFont font, char* path);
JNIEXPORT void JNICALL AWTCheckForFont(const char *fontname, short int *found);
JNIEXPORT void JNICALL AWTDrawOutline(const char *fontname, float size, int upper, int lower, int *numOps, int *numPoints, int *numPaths);
JNIEXPORT void JNICALL AWTGetOutline(int numOps, int numPoints, int numPaths, int opsArray[], int pointsPerPathArray[], float pointsArray[]);


#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif

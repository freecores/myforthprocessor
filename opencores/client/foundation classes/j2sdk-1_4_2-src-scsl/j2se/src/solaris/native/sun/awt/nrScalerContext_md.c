/*
 * @(#)nrScalerContext_md.c	1.9 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <ctype.h>
#include <sys/utsname.h>

#include <jni.h>
#include <jni_util.h>
#include "font.h"

#ifndef HEADLESS

#ifdef HAVE_DPS
#include <DPS/dpsXclient.h>
#include "nrOutlines.h"
#endif /* HAVE_DPS */

#include <X11/Xlib.h>
#include <X11/Xutil.h>
static GC pixmapGC = 0;
static Pixmap pixmap = 0;
static Atom psAtom = 0;
static Atom fullNameAtom = 0;
static int pixmapWidth = 0;
static int pixmapHeight = 0;
#ifdef HAVE_DPS
static DPSContext dpsContext = 0;
static short useDPS = 1;
#endif /* HAVE_DPS */

int CreatePixmapAndGC (int width, int height)
{
    /* REMIND: use the actual screen, not the default screen */
    Window awt_defaultRoot =
        RootWindow(awt_display, DefaultScreen(awt_display));
    
    if (width < 100) {
      width = 100;
    }
    if (height < 100) {
      height = 100;
    }
    pixmapHeight = height;
    pixmapWidth = width;
    if (pixmap != 0) {
      XFreePixmap (awt_display, pixmap);
#ifdef HAVE_DPS
      /* If we created a dps context using this pixmap, we need to destroy
       * it now. */
      if (dpsContext != NULL) {
          DPSDestroySpace (DPSSpaceFromContext (dpsContext));
      }
      dpsContext = NULL;
#endif /* HAVE_DPS */
    }
    if (pixmapGC != NULL) {
      XFreeGC (awt_display, pixmapGC);
    }
    pixmap = XCreatePixmap (awt_display, awt_defaultRoot, pixmapWidth, 
                          pixmapHeight, 1);
    if (pixmap == 0) {
      return BadAlloc;
    }
    pixmapGC = XCreateGC (awt_display, pixmap, 0, 0);
    if (pixmapGC == NULL) {
      return BadAlloc;
    }
    XFillRectangle (awt_display, pixmap, pixmapGC, 0, 0, pixmapWidth, 
                  pixmapHeight);
    XSetForeground (awt_display, pixmapGC, 1);
    return Success;
}

#ifdef DUMP_IMAGES

static void dumpXImage(XImage *ximage)
{
    int height = ximage->height;
    int width = ximage->width;
    int row;
    int column;

    fprintf(stderr, "-------------------------------------------\n");
    for (row = 0; row < height; ++row) {
      for (column = 0; column < width; ++column) {
          int pixel = ximage->f.get_pixel(ximage, column, row);
          fprintf(stderr, (pixel == 0) ? "  " : "XX");
      }
      fprintf(stderr, "\n");
    }
    fprintf(stderr, "-------------------------------------------\n");
}

#endif

#endif /* !HEADLESS */

JNIEXPORT void JNICALL AWTLoadFont(char* name, AWTFont *pReturn) {
    *pReturn = NULL;
#ifndef HEADLESS
    *pReturn = (AWTFont)XLoadQueryFont(awt_display, name);
#endif /* !HEADLESS */
}

JNIEXPORT void JNICALL AWTFreeFont(AWTFont font) {
#ifndef HEADLESS
    XFreeFont(awt_display, (XFontStruct *)font);
#endif /* !HEADLESS */
}

JNIEXPORT unsigned JNICALL AWTFontMinByte1(AWTFont font) {
#ifdef HEADLESS
    return 0;
#else
    return ((XFontStruct *)font)->min_byte1;
#endif /* !HEADLESS */
}

JNIEXPORT unsigned JNICALL AWTFontMaxByte1(AWTFont font) {
#ifdef HEADLESS
    return 0;
#else
    return ((XFontStruct *)font)->max_byte1;
#endif /* !HEADLESS */
}

JNIEXPORT unsigned JNICALL AWTFontMinCharOrByte2(AWTFont font) {
#ifdef HEADLESS
    return 0;
#else
    return ((XFontStruct *)font)->min_char_or_byte2;
#endif /* !HEADLESS */
}

JNIEXPORT unsigned JNICALL AWTFontMaxCharOrByte2(AWTFont font) {
#ifdef HEADLESS
    return 0;
#else
    return ((XFontStruct *)font)->max_char_or_byte2;
#endif /* !HEADLESS */
}

JNIEXPORT unsigned JNICALL AWTFontDefaultChar(AWTFont font) {
#ifdef HEADLESS
    return 0;
#else
    return ((XFontStruct *)font)->default_char;
#endif /* !HEADLESS */
}

JNIEXPORT AWTChar JNICALL AWTFontPerChar(AWTFont font, int index) {
#ifdef HEADLESS
    return NULL;
#else
    XFontStruct *fXFont = (XFontStruct *)font;
    XCharStruct *perChar = fXFont->per_char;
    if (perChar == NULL) {
        return NULL;
    }
    return (AWTChar)&(perChar[index]);
#endif /* !HEADLESS */
}

JNIEXPORT AWTChar JNICALL AWTFontMaxBounds(AWTFont font) {
#ifdef HEADLESS
    return 0;
#else
    return (AWTChar)&((XFontStruct *)font)->max_bounds;
#endif /* !HEADLESS */
}

JNIEXPORT void JNICALL AWTFontQueryTextExtents16(AWTFont font,
    AWTChar2b* xChar, int nchars, int* direction,
    int* ascent, int* descent, AWTChar* overall) {
#ifndef HEADLESS
    XFontStruct* xFont = (XFontStruct*)font;
    XCharStruct* newChar = (XCharStruct*)malloc(sizeof(XCharStruct));
    *overall = (AWTChar)newChar;
    XQueryTextExtents16(awt_display, xFont->fid, xChar, nchars, direction,
        ascent, descent, newChar);
#endif /* !HEADLESS */
}

JNIEXPORT void JNICALL AWTFreeChar(AWTChar xChar) {
#ifndef HEADLESS
    free(xChar);
#endif /* !HEADLESS */
}

JNIEXPORT void JNICALL AWTFontGenerateImage(AWTFont pFont, int startX,
    int theAscent, int fRowBytes, int fWidth,
    int fHeight, AWTChar2b* xChar, unsigned short xGlyphID, void* buffer) {
#ifndef HEADLESS
    XImage *ximage;
    char *imageData;
    char *tmpImage;
    char *tPtr;
    int nbytes;
    int i, j;
    const unsigned char *srcRow;
    const unsigned char *src8;
    unsigned char *dstRow;
    unsigned char *dstByte;
    int wholeByteCount, remainingBitsCount, height;
    unsigned srcValue;
    JNIEnv *env;
    unsigned int imageSize;
    XFontStruct* fXFont = (XFontStruct*)pFont;
	    
    if ((pixmap == 0) || (fRowBytes > (pixmapWidth)) ||
	(fHeight > pixmapHeight)) 
    {
        int retVal = CreatePixmapAndGC (fRowBytes, fHeight);
	if (retVal != Success) {
	    if (buffer) {
    	        unsigned int imageSize = fHeight * fRowBytes;
	        memset(buffer, 0, imageSize);
            }
	    env = (JNIEnv *) JNU_GetEnv (jvm, JNI_VERSION_1_2);
	    JNU_ThrowOutOfMemoryError (env, 
			"Cannot create native data structure.");
	    return;
	}
    }

    XSetFont (awt_display, pixmapGC, fXFont->fid);

    if (xChar == (XChar2b *) NULL) {
        xChar = (XChar2b *) calloc (1, sizeof (XChar2b));
	if (xChar == NULL) {
	    if (buffer) {
    	        unsigned int imageSize = fHeight * fRowBytes;
	        memset(buffer, 0, imageSize);
            }
	    env = (JNIEnv *) JNU_GetEnv (jvm, JNI_VERSION_1_2);
	    JNU_ThrowOutOfMemoryError (env, 
			"Cannot create native data structure.");
	    return;
	}
    }

    xChar->byte1 = (unsigned char) (xGlyphID >> 8);
    xChar->byte2 = (unsigned char) xGlyphID;

    XSetForeground (awt_display, pixmapGC, 0);
    XFillRectangle (awt_display, pixmap, pixmapGC, 0, 0, 
		    pixmapWidth, pixmapHeight);
    XSetForeground (awt_display, pixmapGC, 1);
    XDrawString16 (awt_display, pixmap, pixmapGC, startX, theAscent, xChar, 1);
    ximage = XGetImage (awt_display, pixmap, 0, 0, fRowBytes, 
		        fHeight, AllPlanes, XYPixmap);

    if (ximage == NULL) {
	if (buffer) {
    	    unsigned int imageSize = fHeight * fRowBytes;
	    memset(buffer, 0, imageSize);
        }
	env = (JNIEnv *) JNU_GetEnv (jvm, JNI_VERSION_1_2);
	JNU_ThrowOutOfMemoryError (env, 
			"Cannot create native data structure.");
        return;
    }

#ifdef DUMP_IMAGES
    dumpXImage(ximage);
#endif

    imageData = ximage->data;
    nbytes = fRowBytes / 8;
    if ((fRowBytes % 8) > 0) {
	nbytes++;
    }

    tmpImage = (char *) calloc (1, sizeof (char) * (nbytes * fHeight));
    if (tmpImage == NULL) {
	if (buffer) {
    	    imageSize = fHeight * fRowBytes;
	    memset(buffer, 0, imageSize);
        }
	env = (JNIEnv *) JNU_GetEnv (jvm, JNI_VERSION_1_2);
	JNU_ThrowOutOfMemoryError (env, 
			"Cannot create native data structure.");
	XDestroyImage (ximage);
	return;
    }

    tPtr = tmpImage;
    for (i = 0; i < fHeight; i++) {
	for (j = 0; j < nbytes; j++) {
	    *tPtr++ = *imageData++;
	}
	imageData = ximage->data + ((i + 1) * ximage->bytes_per_line);
    }
	    
    /* This is the CopyBW2ToGrey8 function from t2kScalerContext.cpp */

    srcRow = (unsigned char*)tmpImage;
    dstRow = (unsigned char*)buffer;
    wholeByteCount = fWidth >> 3;
    remainingBitsCount = fWidth & 7;
    height = fHeight;

    while (height--) {
        src8 = srcRow;
        dstByte = dstRow;

        srcRow += nbytes;
        dstRow += fRowBytes;

        for (i = 0; i < wholeByteCount; i++) {
            srcValue = *src8++;
            for (j = 0; j < 8; j++) {
		if (ximage->bitmap_bit_order == LSBFirst) {
                    *dstByte++ = (srcValue & 0x01) ? 0xFF : 0;
                    srcValue >>= 1;
		} else {		/* MSBFirst */
                    *dstByte++ = (srcValue & 0x80) ? 0xFF : 0;
                    srcValue <<= 1;
		}
            }
        }
        if (remainingBitsCount) {
            srcValue = *src8;
            for (j = 0; j < remainingBitsCount; j++) {
		if (ximage->bitmap_bit_order == LSBFirst) {
                    *dstByte++ = (srcValue & 0x01) ? 0xFF : 0;
                    srcValue >>= 1;
		} else {		/* MSBFirst */
                    *dstByte++ = (srcValue & 0x80) ? 0xFF : 0;
                    srcValue <<= 1;
		}
            }
        }
    }

    free (tmpImage);
    XDestroyImage (ximage);
#endif /* !HEADLESS */
}

JNIEXPORT short JNICALL AWTCharWidth(AWTChar xChar) {
#ifdef HEADLESS
    return 0;
#else
    return ((XCharStruct *)xChar)->width;
#endif /* !HEADLESS */
}

JNIEXPORT short JNICALL AWTCharLBearing(AWTChar xChar) {
#ifdef HEADLESS
    return 0;
#else
    return ((XCharStruct *)xChar)->lbearing;
#endif /* !HEADLESS */
}

JNIEXPORT short JNICALL AWTCharRBearing(AWTChar xChar) {
#ifdef HEADLESS
    return 0;
#else
    return ((XCharStruct *)xChar)->rbearing;
#endif /* !HEADLESS */
}

JNIEXPORT short JNICALL AWTCharAscent(AWTChar xChar) {
#ifdef HEADLESS
    return 0;
#else
    return ((XCharStruct *)xChar)->ascent;
#endif /* !HEADLESS */
}

JNIEXPORT short JNICALL AWTCharDescent(AWTChar xChar) {
#ifdef HEADLESS
    return 0;
#else
    return ((XCharStruct *)xChar)->descent;
#endif /* !HEADLESS */
}

JNIEXPORT jboolean JNICALL AWTUseDPS() {
#if defined(HEADLESS) || !defined(HAVE_DPS)
    return JNI_FALSE;
#else
    return useDPS;
#endif /* !HEADLESS && HAVE_DPS */
}

#if !defined(HEADLESS) && defined(HAVE_DPS)

/* Text handler for DPS. We define our own rather than using the
 * default so we don't get annoying messages printed out from DPS
 * (like if a font can't be found). 
 */

static jboolean dpsError = JNI_FALSE;

void
X11nrDPSTextProc (DPSContext ctxt, char *buf, unsigned long count) 
{
#ifdef DPS_DEBUG
    DPSDefaultTextBackstop (ctxt, buf, count);
#endif
}

void 
X11nrDPSErrorProc (DPSContext ctxt, DPSErrorCode errorCode, 
		   long unsigned int arg1, long unsigned int arg2)
{
    dpsError = JNI_TRUE;
    if (dpsContext != NULL) {
	DPSDestroySpace (DPSSpaceFromContext (dpsContext));
        dpsContext = NULL;
    }
#ifdef DPS_DEBUG
    DPSDefaultErrorProc (ctxt, errorCode, arg1, arg2);
#endif
} 
#endif /* !HEADLESS && HAVE_DPS */

JNIEXPORT jboolean JNICALL AWTInitDPS() {
#if defined(HEADLESS) || !defined(HAVE_DPS)
    return JNI_FALSE;
#else /* !HEADLESS && HAVE_DPS */
    XStandardColormap grayMap, rgbMap;
    int retVal;
    JNIEnv *env;

    dpsError = JNI_FALSE;
    if (dpsContext == NULL) {
	/* We're not doing anything exciting.. just use
	 * a black & white colormap. Note that if we
	 * use XDPSCreateSimpleContext or don't specify
	 * a gray or rgb colormap, DPS will allocate one
	 * even though we don't want it.
	 */
	grayMap.colormap = None;
	grayMap.red_max = 1;
	grayMap.red_mult = (unsigned long)-1;
	grayMap.base_pixel = 1;
	rgbMap.colormap = None;
	rgbMap.red_max = 0;
	rgbMap.green_max = 0;
	rgbMap.blue_max = 0;
	rgbMap.red_mult = 0;
	rgbMap.green_mult = 0;
	rgbMap.blue_mult = 0;
	rgbMap.base_pixel = 0;

	if (pixmap == 0) {
	    retVal = CreatePixmapAndGC (100, 100);
	    if (retVal != Success) {
	        useDPS = 0;
	        env = (JNIEnv *) JNU_GetEnv (jvm, JNI_VERSION_1_2);
	        JNU_ThrowOutOfMemoryError (env, 
			"Cannot create native data structure.");
		return JNI_FALSE;
	    }
	}

	dpsContext = XDPSCreateContext (awt_display, pixmap, pixmapGC, 0, 
					pixmapHeight, 0, &grayMap, 
					&rgbMap, 0, X11nrDPSTextProc,
					X11nrDPSErrorProc, (DPSSpace) NULL);

	/* If dpsContext is NULL, then we're probably running on a
	 * server that doesn't support DPS.
	 */
	if (dpsContext == NULL) {
	    useDPS = 0;
	    return JNI_FALSE;
	}

	DPSSetContext (dpsContext);
	OutlineInit ();
        if (dpsError) {
            return JNI_FALSE;
        }
    }
    return JNI_TRUE;
#endif /* !HEADLESS && HAVE_DPS */
}

JNIEXPORT jboolean JNICALL AWTDPSError() {
#if defined(HEADLESS) || !defined(HAVE_DPS)
    return JNI_TRUE;
#else /* !HEADLESS && HAVE_DPS */
    return dpsError;
#endif /* !HEADLESS && HAVE_DPS */
}

JNIEXPORT jboolean JNICALL AWTGetDPSPath(AWTFont fXFont, char* tempName) {
#if defined(HEADLESS) || !defined(HAVE_DPS)
    return JNI_FALSE;
#else /* !HEADLESS && HAVE_DPS */
    unsigned long atomValue;
    Bool found;
    char *fName;

    if (psAtom == 0) {
        psAtom = XInternAtom (awt_display, "_ADOBE_POSTSCRIPT_FONTNAME", 
			      False);
    }

    found = XGetFontProperty ((XFontStruct *)fXFont, psAtom, &atomValue);

    if (found) {
	fName = XGetAtomName (awt_display, atomValue);
	if (fName != NULL) {
            strcpy (tempName, fName);
            XFree (fName);
        } else {
            found = JNI_FALSE;
	}
    }

    if (!found) {

        /*
         * If ADOBE_POSTSCRIPT_NAME not set, then check the "FULL_NAME"
         * property.
         */

	if (fullNameAtom == 0) {
	    fullNameAtom = XInternAtom (awt_display, "FULL_NAME", False);
	}
	found = XGetFontProperty ((XFontStruct *)fXFont, fullNameAtom,
            &atomValue);
	if (found) {
	    fName = XGetAtomName (awt_display, atomValue);
	    if (fName != NULL) {
	        strcpy (tempName, fName);
		XFree (fName);
	    } else {
		found = JNI_FALSE;
	    }
	}
    }
    return found;
#endif /* !HEADLESS && HAVE_DPS */
}

JNIEXPORT void JNICALL AWTCheckForFont(const char *fontname, short int *found) {
#if !defined(HEADLESS) && defined(HAVE_DPS)
    CheckForFont(fontname, found);
#endif /* !HEADLESS && HAVE_DPS */
}

JNIEXPORT void JNICALL AWTDrawOutline(const char *fontname, float size,
    int upper, int lower, int *numOps, int *numPoints, int *numPaths) {
#if !defined(HEADLESS) && defined(HAVE_DPS)
    DrawOutline(fontname, size, upper, lower, numOps, numPoints, numPaths);
#endif /* !HEADLESS && HAVE_DPS */
}

JNIEXPORT void JNICALL AWTGetOutline(int numOps, int numPoints, int numPaths,
    int opsArray[], int pointsPerPathArray[], float pointsArray[]) {
#if !defined(HEADLESS) && defined(HAVE_DPS)
    GetOutline(numOps, numPoints, numPaths, opsArray, pointsPerPathArray,
        pointsArray);
#endif /* !HEADLESS && HAVE_DPS */
}



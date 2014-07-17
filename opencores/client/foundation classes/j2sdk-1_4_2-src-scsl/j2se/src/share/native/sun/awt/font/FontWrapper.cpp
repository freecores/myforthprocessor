/*
 * @(#)FontWrapper.cpp	1.55 03/02/19
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @author Charlton Innovations, Inc.
 */

#ifdef __linux__
#include <stdint.h>
#endif /* __linux__ */
#include "FontGlue.h"
#include "CompositeFont.h"
#include "Strike.h"
#include "GlyphVector.h"
#include "GlyphLayout.h"
#include "cpu.h"
#include "TX.h"
#include "java_awt_Font.h"
#include "DefaultCharMapper.h"

// private field inside of Font object to reference native instance
static jfieldID gFont_pNativeFont = 0;
static jfieldID gFont_style = 0;
static jfieldID gFont_canRotate = 0;

static jobject makeRect(JNIEnv *env, int l, int t, int w, int h) {
  static jclass rectClass = 0;
  static jmethodID rectCC = 0;
  static const char* rectClassName = "java/awt/Rectangle";
  static const char* rectCCName = "<init>";
  static const char* rectCCArgs = "(IIII)V";

  if (!rectClass) {
    rectClass = env->FindClass(rectClassName);
    if (!rectClass) {
        JNU_ThrowClassNotFoundException(env, rectClassName);
        return NULL;
    }
    rectClass = (jclass)env->NewGlobalRef(rectClass);
    if (!rectClass) {
        JNU_ThrowInternalError(env, "could not create global ref");
        return NULL;
    }
    rectCC = env->GetMethodID(rectClass, rectCCName, rectCCArgs);
    if (!rectCC) {
      rectClass = 0;
      JNU_ThrowNoSuchMethodException(env, rectCCName);
      return NULL;
    }
  }

  return env->NewObject(rectClass, rectCC, l, t, w, h);
}

// get native instance from Font object
fontObject *getFontPtr(JNIEnv *env, jobject font)
{
    fontObject *fo = NULL;
    if (JNU_IsNull (env, font)) {
        JNU_ThrowIllegalArgumentException(env, "Font");
        return NULL;
    }
    if (!gFont_pNativeFont || !gFont_style) {
        jclass fontClass = env->GetObjectClass(font);
        if (!fontClass) {
            JNU_ThrowClassNotFoundException(env, "No Font class");
            return NULL;
        }
        if (!gFont_pNativeFont) {
            gFont_pNativeFont = env->GetFieldID(fontClass, "pNativeFont","J");
            if (!gFont_pNativeFont) {
                JNU_ThrowNoSuchFieldException(env, "in Font class");
                return NULL;
            }
        }
        if (!gFont_style) {
            gFont_style = env->GetFieldID(fontClass, "style", "I");
            if (!gFont_style) {
                JNU_ThrowNoSuchFieldException(env, "in Font class");
                return NULL;
            }
        }
    }
    jlong pNativeFont = env->GetLongField(font, gFont_pNativeFont);
    fo = (fontObject *)jlong_to_ptr(pNativeFont);
    jint style = env->GetIntField(font, gFont_style);
    // REMIND: this is yet another thing which makes this code not reentrant.
    fo->m_currentStyle = style;
    return fo;
}

// set native instance into Font object
static void setFontPtr(JNIEnv *env, jobject font, fontObject *foPtr)
{
    if (!gFont_pNativeFont) {
        jclass fontClass = env->GetObjectClass(font);
        if (!fontClass) {
            JNU_ThrowClassNotFoundException(env, "No Font class");
            return;
        }
        gFont_pNativeFont = env->GetFieldID(fontClass, "pNativeFont", "J");
        if (!gFont_pNativeFont) {
            JNU_ThrowNoSuchFieldException(env, "in Font class");
            return;
        }
    }
    env->SetLongField(font, gFont_pNativeFont, ptr_to_jlong(foPtr));
}

// Native implementations of JAVA routines...
extern "C" {
/*
 * Class:     sun_awt_font_NativeFontWrapper
 * Method:    canDisplay
 * Signature: (Ljava/awt/Font;C)Z
 */
JNIEXPORT jboolean JNICALL
Java_sun_awt_font_NativeFontWrapper_canDisplay
    (JNIEnv *env, jclass clsNFW, jobject theFont, jchar c)
{
    jboolean retval = JNI_FALSE;
    fontObject *fo = ::getFontPtr(env, theFont);
    if (fo && fo->isValid) {
        Strike& theStrike = fo->getStrike();
        retval = theStrike.canDisplay(c)? JNI_TRUE: JNI_FALSE;
    }
    return retval;
}

/*   
 * Class:     sun_awt_font_NativeFontWrapper
 * Method:    doesGlyphShaping
 * Signature: (Ljava/awt/Font;)Z
 */
JNIEXPORT jboolean JNICALL
Java_sun_awt_font_NativeFontWrapper_doesGlyphShaping
    (JNIEnv *env, jclass clsNFW, jobject theFont)
{
    jboolean retval = JNI_FALSE;
    fontObject *fo = ::getFontPtr(env, theFont);
    if (fo) {
       Strike& theStrike = fo->getStrike();
        retval = theStrike.doesGlyphShaping()? JNI_TRUE: JNI_FALSE;
    }
    return retval;
}

JNIEXPORT jboolean JNICALL
Java_sun_awt_font_NativeFontWrapper_charsToGlyphs
    (JNIEnv *env, jclass clsNFW, jobject theFont,
     jcharArray chars, jint coffset, jintArray glyphs, jint goffset, jint count)
{
    jboolean needShaping = JNI_FALSE;
    fontObject *fo = ::getFontPtr(env, theFont);
    if (fo && fo->isValid) {
	Strike& theStrike = fo->getStrike();
	
        const jchar *theChars = (const jchar *)env->GetPrimitiveArrayCritical(chars, NULL);
        if (theChars) {
	    UInt32 *theGlyphs = (UInt32 *)env->GetPrimitiveArrayCritical(glyphs, NULL);
	    if (theGlyphs) {
		needShaping = GlyphVector::charsToGlyphs(theStrike, theChars + coffset, theGlyphs + goffset, count);

		env->ReleasePrimitiveArrayCritical(glyphs, (void*)theGlyphs, 0);
	    }
	    env->ReleasePrimitiveArrayCritical(chars, (void*)theChars, JNI_ABORT);
	}
    }
    return needShaping;
}
    
/*  
 * Class:     sun_awt_font_NativeFontWrapper
 * Method:    isStyleSupported
 * Signature: (Ljava/awt/Font;)Z
 */
JNIEXPORT jboolean JNICALL
Java_sun_awt_font_NativeFontWrapper_isStyleSupported
    (JNIEnv *env, jclass clsNFW, jobject theFont,jint theStyle)
{
    jboolean retval = JNI_FALSE;
    fontObject *fo = ::getFontPtr(env, theFont);
    if (fo) {
        Strike& theStrike = fo->getStrike();
        retval = theStrike.isStyleSupported((int)theStyle);
    }
    return retval;
}

/*
 * Class:     sun_awt_font_NativeFontWrapper
 * Method:    fontCanRotate
 * Signature: (Ljava/awt/Font)I
 */
JNIEXPORT jint JNICALL
Java_sun_awt_font_NativeFontWrapper_fontCanRotate
    (JNIEnv *env, jclass clsNFW, jobject theFont)
{
    if (!gFont_canRotate) {
        jclass fontClass = env->GetObjectClass (theFont);
        if (!fontClass) {
            JNU_ThrowClassNotFoundException(env, "No Font class");
            return sun_awt_font_NativeFontWrapper_ROTATE_FALSE;
        }
        gFont_canRotate = env->GetFieldID(fontClass, "canRotate", "I");
        if (!gFont_canRotate) {
            JNU_ThrowNoSuchFieldException(env, "in Font class");
            return sun_awt_font_NativeFontWrapper_ROTATE_FALSE;
        }
    }

    /* First, determine if private field in Font object already set */
    jint canRotate = env->GetIntField (theFont, gFont_canRotate);

    if (canRotate != sun_awt_font_NativeFontWrapper_ROTATE_UNKNOWN) {
        return canRotate;
    }

    canRotate = sun_awt_font_NativeFontWrapper_ROTATE_FALSE;

    fontObject *fo = getFontPtr (env, theFont);
    if (fo && fo->isValid) {
        Strike& theStrike = fo->getStrike ();
        canRotate = theStrike.canRotate ();
    }

    if (canRotate != sun_awt_font_NativeFontWrapper_ROTATE_UNKNOWN) {
        env->SetIntField (theFont, gFont_canRotate, canRotate);
    }

    return canRotate;
}


/*
 * Class:     sun_awt_font_NativeFontWrapper
 * Method:    fontCanRotateText 
 * Signature: (Ljava/awt/Font;Ljava/lang/String;[FZZ)Z
 */
JNIEXPORT jboolean JNICALL
Java_sun_awt_font_NativeFontWrapper_fontCanRotateText
    (JNIEnv *env, jclass clsNFW, jobject theFont, jstring theString,
     jdoubleArray matrix,
     jboolean isAntiAliased, jboolean usesFractionalMetrics)
{
    jboolean retval = JNI_FALSE;
    fontObject *fo = getFontPtr (env, theFont);
    if (fo && fo->isValid) {
        int fNumGlyphs = env->GetStringLength (theString);
        FontTransform tx(env, matrix);
        Strike& theStrike = fo->getStrike(tx, isAntiAliased, 
                                          usesFractionalMetrics);
        const jchar *theChars = env->GetStringCritical (theString, NULL);
        if (theChars) {
            UInt32 *fGlyphs = new UInt32 [fNumGlyphs];
            jchar *tmpChars = new jchar[fNumGlyphs];
            memcpy(tmpChars, theChars, sizeof(jchar)*fNumGlyphs);
            env->ReleaseStringCritical (theString, theChars);
            for (int i = 0; i < fNumGlyphs; i++) {
                fGlyphs[i] = theStrike.CharToGlyph (tmpChars[i]);
            }
            retval = theStrike.canRotateString (fNumGlyphs, fGlyphs);
            delete [] fGlyphs;
            delete [] tmpChars;
        }
    }
    return retval;
}

/*
 * Class:     sun_awt_font_NativeFontWrapper
 * Method:    fontCanRotateGlyphVector 
 * Signature: (Ljava/awt/Font;[I[FZZ)Z
 */
JNIEXPORT jboolean JNICALL
Java_sun_awt_font_NativeFontWrapper_fontCanRotateGlyphVector
    (JNIEnv *env, jclass clsNFW, jobject theFont, jintArray theString,
     jdoubleArray matrix,
     jboolean isAntiAliased, jboolean usesFractionalMetrics)
{
    jboolean retval = JNI_FALSE;
    int fNumGlyphs = env->GetArrayLength (theString);
    fontObject *fo = getFontPtr (env, theFont);
    if (fo && fo->isValid) {
        FontTransform tx(env, matrix);
        Strike& theStrike = fo->getStrike(tx, isAntiAliased, 
                                          usesFractionalMetrics);
        const jint *theGlyphs = (const jint *) env->GetPrimitiveArrayCritical (
                                                        theString, NULL);
        if (theGlyphs) {
            UInt32 *fGlyphs = new UInt32 [fNumGlyphs];
            for (int i = 0; i < fNumGlyphs; i++) {
                fGlyphs[i] = theGlyphs [i];
            }
            env->ReleasePrimitiveArrayCritical (theString, (void *) theGlyphs,
                                                JNI_ABORT);
            retval = theStrike.canRotateString (fNumGlyphs, fGlyphs);
            delete [] fGlyphs;
        }
    }
    return retval;
}

// look up nameID in theFont and return the result as a Java string.
static jstring lookupName
  (JNIEnv *env, jobject theFont, jshort lcid, UInt16 nameID)
{
    UInt16 name[1024];
    UInt16 platformID = 3;
    UInt16 scriptID = 1;
    UInt16 languageID =lcid;

    fontObject *fo = ::getFontPtr(env, theFont);
    if (fo && fo->isValid) {
        // length is the number of bytes in the name, NOT the number of chars
        int length = fo->GetName(platformID, scriptID, languageID, nameID, name);
        if (length) { 
            if ((platformID == 3) || (platformID == 0)) {
                return env->NewString(name, length);
            } else {
              // Note:  this assumes that if the encoding is not
              // Unicode it is UTF.  This may not be true, particularly
              // for some Apple encodings.
                return env->NewStringUTF((char *)name);
            }
        }
    }

    return env->NewStringUTF("");
}

/*
 * Class:     sun_awt_font_NativeFontWrapper
 * Method:    getFamilyName
 * Signature: (Ljava/awt/Font;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL
Java_sun_awt_font_NativeFontWrapper_getFamilyName
  (JNIEnv *env, jclass clsNFW, jobject theFont, jshort lcid)
{
  return lookupName(env, theFont, lcid, kFamilyName);
}


/*
 * Class:     sun_awt_font_NativeFontWrapper
 * Method:    getPostscriptName
 * Signature: (Ljava/awt/Font;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL
Java_sun_awt_font_NativeFontWrapper_getPostscriptName
  (JNIEnv *env, jclass clsNFW, jobject theFont)
{
  // hardwire MS LCID for US English as that is what a PS name must use.
  return lookupName(env, theFont, 0x0409, kPSFontName);
}

/*
 * Class:     sun_awt_font_NativeFontWrapper
 * Method:    getFamilyNameByIndex
 * Signature: (I)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL
Java_sun_awt_font_NativeFontWrapper_getFamilyNameByIndex
    (JNIEnv *env, jclass clsNFW, jint fontIndex, jint localeID)
{
    UInt16 name[256];
    UInt16 platformID = 3;
    UInt16 scriptID = 1;
    UInt16 languageID = localeID;
    UInt16 nameID = kFamilyName;

    fontObject* fo = GetFontObject(fontIndex);
    if (fo && fo->isValid) {
        int length = fo->GetName(platformID, scriptID, languageID, nameID, name);
        if (length) {
            if ((platformID == 3) || (platformID == 0)) {
                return env->NewString(name, length);
            } else {
                return env->NewStringUTF((char *)name);
            }
        }
    }
    return env->NewStringUTF("");
}

/*
 * Class:     sun_awt_font_NativeFontWrapper
 * Method:    getFontMetrics
 * Signature: (Ljava/awt/Font;[FZZ[F)V
 */
JNIEXPORT void JNICALL
Java_sun_awt_font_NativeFontWrapper_getFontMetrics
        (JNIEnv *env, jclass clsNFW, jobject theFont, jdoubleArray transform,
        jboolean isAntiAliased, jboolean usesFractionalMetrics,
        jfloatArray metrics)
{
    fontObject *fo = ::getFontPtr(env, theFont);
    if (fo && fo->isValid) {
        FontTransform tx(env, transform);
        Strike& theStrike = fo->getStrike(tx, isAntiAliased, usesFractionalMetrics);
        hsFixedPoint2 ascent, descent, baseline, leading, maxAdvance;
        jfloat* resultArray;
        theStrike.GetLineHeight(ascent, descent,
                             baseline, leading, maxAdvance);
        resultArray = (jfloat *)env->GetPrimitiveArrayCritical(metrics, 0);
        if (resultArray) {
            resultArray[0] = -hsFixedToFloat(ascent.fY);
            resultArray[1] = hsFixedToFloat(descent.fY);
            resultArray[2] = -hsFixedToFloat(leading.fY);
            resultArray[3] = hsFixedToFloat(maxAdvance.fX);
            env->ReleasePrimitiveArrayCritical(metrics,
                                               resultArray, 0);
        }
    }
}

/*
 * Class:     sun_awt_font_NativeFontWrapper
 * Method:    getFullName
 * Signature: (Ljava/awt/Font;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL
Java_sun_awt_font_NativeFontWrapper_getFullName
        (JNIEnv *env, jclass clsNFW, jobject theFont, jshort lcid)
{
  return lookupName(env, theFont, lcid, kFullName);
}


/*
 * Class:     sun_awt_font_NativeFontWrapper
 * Method:    getFullNameByIndex
 * Signature: (I)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL
Java_sun_awt_font_NativeFontWrapper_getFullNameByIndex
    (JNIEnv *env, jclass clsNFW, jint fontIndex)
{
    UInt16 name[1024];
    UInt16 platformID = 3;
    UInt16 scriptID = 1;
    UInt16 languageID = 0xFFFF;
    UInt16 nameID = kFullName;

    fontObject* fo = GetFontObject(fontIndex);
    if (fo && fo->isValid) {
        int length = fo->GetName(platformID, scriptID, languageID, nameID, name);
        if (length) {
            if ((platformID == 3) || (platformID == 0)) {
                return env->NewString(name, length);
            } else {
                return env->NewStringUTF((char *)name);
            }
        }
    }
    return env->NewStringUTF("");
}

/*
 * Class:     sun_awt_font_NativeFontWrapper
 * Method:    getFullNameByFileName
 * Signature: (I)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL
Java_sun_awt_font_NativeFontWrapper_getFullNameByFileName
    (JNIEnv *env, jclass clsNFW, jstring fontFileName)
{
    UInt16 name[1024];
    UInt16 platformID = 3;
    UInt16 scriptID = 1;
    UInt16 languageID = 0xFFFF;
    UInt16 nameID = kFullName;
    fontObject *fo = NULL;

    {
        JStringBuffer ffName(env, fontFileName);
        fo = GetFontObject(ffName.buffer(), ffName.getLength());
    }
    if (fo) {
        int length = fo->GetName(platformID, scriptID, languageID, nameID, name);
        if (length) {
            if ((platformID == 3) || (platformID == 0)) {
                return env->NewString(name, length);
            } else {
                return env->NewStringUTF((char *)name);
            }
        }
    }
    return NULL;
}


/*
 * Class:     sun_awt_font_NativeFontWrapper
 * Method:    createFont
 * Signature: (Ljava/lang/String;I)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL
Java_sun_awt_font_NativeFontWrapper_createFont
    (JNIEnv *env, jclass clsNFW, jstring fontFileName, jint fontFormat)
{

  const Unicode *theFontFileName = env->GetStringChars(fontFileName, NULL);
  if (!fontFileName) {
     JNU_ThrowIllegalArgumentException(env, "Name");
     return NULL;
  }
  unsigned int fontFileNameLen = env->GetStringLength(fontFileName);

  const char *localName = env->GetStringUTFChars ( fontFileName, 0 );
  int numChars = 0;
  const Unicode *fontName = CreateTrueTypeFont ( (Unicode *) theFontFileName, fontFileNameLen,(char *) localName, &numChars );
  
  // This is required to nmmmbe released or JAVA garbage collection complains
  env->ReleaseStringChars(fontFileName, theFontFileName);
  env->ReleaseStringUTFChars ( fontFileName, localName );


  if ( fontName != NULL ) {
    return env->NewString ( fontName, numChars );
  }
  return NULL;
}


/*
 * Class:     sun_awt_font_NativeFontWrapper
 * Method:    getType1FontVar
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL
Java_sun_awt_font_NativeFontWrapper_getType1FontVar (JNIEnv *env, jclass cl) {

    /*
     * Please do not print out the result of the environmental
     * value.
     */
    char *c = getenv("JAVA2D_NOTYPE1FONT");
    if (c) {
      /* printf("JAVA2D_NOTYPE1FONT set\n"); */
        return JNI_TRUE;
    } else {
      /*  printf("JAVA2D_NOTYPE1FONT not set\n"); */
        return JNI_FALSE;
    }
}

/*
 * Class:     sun_awt_font_NativeFontWrapper
 * Method:    getGlyphJustificationInfo
 * Signature: (Ljava/awt/Font;ILjava/awt/font/GlyphJustificationInfo;)V
 */
JNIEXPORT void JNICALL
Java_sun_awt_font_NativeFontWrapper_getGlyphJustificationInfo
  (JNIEnv *, jclass, jobject, jint, jobject);

// utility used by getGlyphMetrics and getCharMetrics
// should be in GlyphVector

void getMetrics
    (JNIEnv *env, jclass clsNFW, jobject theFont,
     jint code, jboolean isGlyph, jdoubleArray fontTX, jdoubleArray devTX,
     jboolean doAntiAlias, jboolean doFractEnable, jfloatArray results)
{
    fontObject *fo = ::getFontPtr(env, theFont);
    if (fo && fo->isValid) {
        hsGGlyph glyph;
        hsFixedPoint2 glyphAdvance;
        hsRect bounds;
        jfloat ax = 0;
	jfloat ay = 0;
	bool needBounds = env->GetArrayLength(results) > 2;
	jint glyphCode;

        TX ftx(env, fontTX);
        TX dtx(env, devTX);
        TX gtx(dtx);
        gtx.concat(ftx);
	
	jint validGlyph = JNI_TRUE;
        {
            // Get advance under dev tx and then undo devtx
            FontTransform tx(gtx.m00, gtx.m10, gtx.m01, gtx.m11);
            Strike& theStrike = fo->getStrike(tx, doAntiAlias, doFractEnable);

	    // code is either a glyph code or a Unicode32 char
	    if (isGlyph) {
		glyphCode = code ;
	    } else {
		DefaultCharMapper mapper(true, false, true);
		if (mapper.mapChar(code) == 0xFFFF) {
		    validGlyph = JNI_FALSE;
		} else {
		    glyphCode = theStrike.CharToGlyph(code);
		}
	    }

	    if (validGlyph) {
		//ask strike for advance & bounds - but ignore bounds since they are in pixels
		theStrike.getMetrics(glyphCode, glyph, glyphAdvance);

		ax = (jfloat)hsFixedToFloat(glyphAdvance.fX);
		ay = (jfloat)hsFixedToFloat(glyphAdvance.fY);
		if (!dtx.isIdentity()) {
		    if (dtx.invert()) { // ignore error
                        dtx.vectorTransform(ax, ay);
		    }
		}
	    }
        }

        if (needBounds && validGlyph) {
          // get bounds from font tx since it isn't hinted
          FontTransform tx(ftx.m00, ftx.m10, ftx.m01, ftx.m11);
          Strike& theStrike = fo->getStrike(tx, doAntiAlias, doFractEnable);

	  // glyphCode already initialized by previous strike, is resolution independent so ok to reuse
          theStrike.getOutlineBounds(glyphCode, bounds);
        }

	jfloat* cResults = (jfloat*)env->GetPrimitiveArrayCritical(results, NULL);
	if (cResults) {
	    cResults[0] = ax;
            cResults[1] = ay;
	    if (needBounds) {
  	        if (validGlyph) {
		    cResults[2] = (jfloat)hsScalarToFloat(bounds.fLeft);
		    cResults[3] = (jfloat)hsScalarToFloat(bounds.fTop);
		    cResults[4] = (jfloat)hsScalarToFloat(bounds.fRight - bounds.fLeft);
		    cResults[5] = (jfloat)hsScalarToFloat(bounds.fBottom - bounds.fTop);
		} else {
		    cResults[2] = cResults[3] = cResults[4] = cResults[5] = 0;
		}
	    }
            env->ReleasePrimitiveArrayCritical(results, cResults, 0);
        }
    }
}

/*
 * Class:     sun_awt_font_NativeFontWrapper
 * Method:    getGlyphMetrics
 * Signature: (Ljava/awt/Font;I[F[FZZ[F)V
 */
JNIEXPORT void JNICALL
Java_sun_awt_font_NativeFontWrapper_getGlyphMetrics
        (JNIEnv *env, jclass clsNFW, jobject theFont,
        jint glyphCode, jdoubleArray fontTX, jdoubleArray devTX,
        jboolean doAntiAlias, jboolean doFractEnable, jfloatArray results)
{
    getMetrics(env, clsNFW, theFont, glyphCode, JNI_TRUE, 
	       fontTX, devTX, doAntiAlias, doFractEnable, results);
}

JNIEXPORT void JNICALL
Java_sun_awt_font_NativeFontWrapper_getCharMetrics
        (JNIEnv *env, jclass clsNFW, jobject theFont,
        jint charCode, jdoubleArray fontTX, jdoubleArray devTX,
        jboolean doAntiAlias, jboolean doFractEnable, jfloatArray results)
{
    getMetrics(env, clsNFW, theFont, charCode, JNI_FALSE, 
	       fontTX, devTX, doAntiAlias, doFractEnable, results);
}

JNIEXPORT jobject JNICALL
Java_sun_awt_font_NativeFontWrapper_getGlyphLogicalBounds(
   JNIEnv *env,
   jclass clsNFW,
   jobject theFont,
   jint glyphIndex,
   jintArray glyphs,
   jfloatArray positions,
   jdoubleArray transforms,
   jintArray txIndices,
   jdoubleArray fontTX,
   jboolean doAntiAlias,
   jboolean doFractEnable)
{
    fontObject *fo = ::getFontPtr(env, theFont);
    if (fo) {
        GlyphVector gv(env, fontTX, NULL, doAntiAlias, doFractEnable, fo);
        gv.setGlyphCodes(glyphs);
        gv.setPositions(positions);
        gv.setTransforms(transforms, txIndices);

        return gv.getGlyphLogicalBounds(glyphIndex);
    }
    return NULL;
}

JNIEXPORT jobject JNICALL
Java_sun_awt_font_NativeFontWrapper_getVisualBounds(
   JNIEnv *env,
   jclass clsNFW,
   jobject theFont,
   jintArray glyphs,
   jfloatArray positions,
   jdoubleArray transforms,
   jintArray txIndices,
   jdoubleArray fontTX,
   jboolean doAntiAlias,
   jboolean doFractEnable)
{
    fontObject *fo = ::getFontPtr(env, theFont);
    if (fo) {
        GlyphVector gv(env, fontTX, NULL, doAntiAlias, doFractEnable, fo);
        gv.setGlyphCodes(glyphs);
        gv.setPositions(positions);
        gv.setTransforms(transforms, txIndices);

        return gv.getVisualBounds();
    }
    return NULL;
}

JNIEXPORT jobject JNICALL
Java_sun_awt_font_NativeFontWrapper_getGlyphVisualBounds(
   JNIEnv *env,
   jclass clsNFW,
   jobject theFont,
   jint glyphIndex,
   jintArray glyphs,
   jfloatArray positions,
   jdoubleArray transforms,
   jintArray txIndices,
   jdoubleArray fontTX,
   jboolean doAntiAlias,
   jboolean doFractEnable)
{
    fontObject *fo = ::getFontPtr(env, theFont);
    if (fo) {
        GlyphVector gv(env, fontTX, NULL, doAntiAlias, doFractEnable, fo);
        gv.setGlyphCodes(glyphs);
        gv.setPositions(positions);
        gv.setTransforms(transforms, txIndices);

        return gv.getGlyphVisualBounds(glyphIndex);
    }
    return NULL;
}

JNIEXPORT jobject JNICALL
Java_sun_awt_font_NativeFontWrapper_getPixelBounds(
        JNIEnv *env, 
        jclass clsNFW, 
        jobject theFont,
        jfloat gvx,
        jfloat gvy,
        jintArray glyphs,
        jfloatArray positions,
        jdoubleArray transforms,
        jintArray txIndices,
        jdoubleArray fontTX,
        jdoubleArray devTX,
        jboolean doAntiAlias,
        jboolean doFractEnable)
{
    fontObject *fo = ::getFontPtr(env, theFont);
    if (fo) {
        GlyphVector gv(env, fontTX, devTX, doAntiAlias, doFractEnable, fo);
        gv.setGlyphCodes(glyphs);
        gv.setPositions(positions);
        gv.setTransforms(transforms, txIndices);

        return gv.getPixelBounds(gvx, gvy);
    }
    return NULL;
}

JNIEXPORT jobject JNICALL
Java_sun_awt_font_NativeFontWrapper_getGlyphPixelBounds(
        JNIEnv *env, 
        jclass clsNFW, 
        jobject theFont,
        jint glyphIndex,
        jfloat gvx,
        jfloat gvy,
        jintArray glyphs,
        jfloatArray positions,
        jdoubleArray transforms,
        jintArray txIndices,
        jdoubleArray fontTX,
        jdoubleArray devTX,
        jboolean doAntiAlias,
        jboolean doFractEnable)
{
    fontObject *fo = ::getFontPtr(env, theFont);
    if (fo) {
        GlyphVector gv(env, fontTX, devTX, doAntiAlias, doFractEnable, fo);
        gv.setGlyphCodes(glyphs);
        gv.setPositions(positions);
        gv.setTransforms(transforms, txIndices);

        return gv.getGlyphPixelBounds(glyphIndex, gvx, gvy);
    }
    return NULL;
}

JNIEXPORT jfloatArray JNICALL
Java_sun_awt_font_NativeFontWrapper_getGlyphInfo
        (JNIEnv *env, 
         jclass clsNFW, 
         jobject theFont,
         jintArray glyphs, 
         jfloatArray positions, 
         jdoubleArray transforms, 
         jintArray txIndices, 
         jdoubleArray fontTX, 
         jdoubleArray devTX,
         jboolean doAntiAlias, 
         jboolean doFractEnable)
{
  fontObject *fo = ::getFontPtr(env, theFont);
  if (fo) {
        GlyphVector gv(env, fontTX, devTX, doAntiAlias, doFractEnable, fo);
        gv.setGlyphCodes(glyphs);
        gv.setPositions(positions);
        gv.setTransforms(transforms, txIndices);

        return gv.getGlyphInfo();
    }
    return NULL;
}

#if 0
/*
 * Class:     sun_awt_font_NativeFontWrapper
 * Method:    getGlyphInfo
 * Signature: (Ljava/awt/Font;[III[F[FZZ[F)V
 */
JNIEXPORT void JNICALL
Java_sun_awt_font_NativeFontWrapper_getGlyphInfo
        (JNIEnv *env, jclass clsNFW, jobject theFont,
        jintArray glyphs, jfloatArray positions, jint length, jdoubleArray fontTX, jdoubleArray devTX,
        jboolean doAntiAlias, jboolean doFractEnable, jfloatArray results)
{
  fontObject *fo = ::getFontPtr(env, theFont);
  if (fo) {
      TX ftx(env, fontTX);
      TX dtx(env, devTX);
      TX gtx(dtx);
      gtx.concat(ftx);

    FontTransform gft(gtx.m00, gtx.m10, gtx.m01, gtx.m11);
    Strike* gStrike = new Strike(*fo, gft, doAntiAlias, doFractEnable);

    // can't ask fo for more than one strike, it owns and deletes the old one
    // if asked for a new one.  So don't ask, and hope strike doesn't diddle
    // with font.

    FontTransform fft(ftx.m00, ftx.m10, ftx.m01, ftx.m11);
    Strike* sStrikep = new Strike(*fo, fft, doAntiAlias, doFractEnable);
    if (!sStrikep) {
      return;
    }

    UInt32* gpStorage = (UInt32*) 
    env->GetPrimitiveArrayCritical(glyphs, NULL);

    // We modify this later; gpStorage preserves original pointer
    if (gpStorage) {
      UInt32* gp = gpStorage;
      UInt32* ep = gp + length;
      --gp;

      jfloat* ppStorage = 0;
      jfloat* pp = 0;
      if (positions) {
        ppStorage = (jfloat*)env->GetPrimitiveArrayCritical(positions, NULL);
        if (ppStorage) {
            pp = ppStorage - 1;
        }
      }

      jfloat* rpStorage = (jfloat*) env->GetPrimitiveArrayCritical(results, NULL);
      jfloat* rp = rpStorage;
      if (rp) {
        --rp;

        jfloat x = dtx.m02;
        jfloat y = dtx.m12;

        if (ppStorage) {
          x = *++pp;
          y = *++pp;
        }

        dtx.invert(); // ignore failure

        while (++gp != ep) {
          hsGGlyph glyph;
          hsFixedPoint2 advanceXY;
          hsRect bounds;

          *++rp = x;
          *++rp = y;
          if (*gp == 0xffff || !gStrike.getMetrics(*gp, glyph, advanceXY)) {
            *++rp = 0;
            *++rp = 0;
            *++rp = x;
            *++rp = y;
            *++rp = 0;
            *++rp = 0;
          } else {
            // FIXME: should have been done by above call?

            jfloat ax = hsFixedToFloat(advanceXY.fX);
            jfloat ay = hsFixedToFloat(advanceXY.fY);

            if (ppStorage) {
                float axx = pp[1] - pp[-1];
                float ayy = pp[2] - pp[0];

                if (axx == 0) {
                    ax = 0;
                }

                if (ayy == 0) {
                    ay = 0;
                }
            }

            // fprintf(stdout, "advances %g %g", (double)ax, (double)ay);

            dtx.vectorTransform(ax, ay);

            // fprintf(stdout, " --> %g %g\n", (double)ax, (double)ay);

            *++rp = ax;
            *++rp = ay;

            sStrikep->getOutlineBounds(*gp, bounds);

            *++rp = x + (jfloat)hsScalarToFloat(bounds.fLeft);
            *++rp = y + (jfloat)hsScalarToFloat(bounds.fTop);
            *++rp = (jfloat)hsScalarToFloat(bounds.fRight - bounds.fLeft);
            *++rp = (jfloat)hsScalarToFloat(bounds.fBottom - bounds.fTop);
            x += ax;
            y += ay;
          }
          if (ppStorage) {
            x = *++pp;
            y = *++pp;
          }
        }

        env->ReleasePrimitiveArrayCritical(results, rpStorage,  0);
      }
      if (ppStorage) {
        env->ReleasePrimitiveArrayCritical(positions, ppStorage, JNI_ABORT);
      }
      env->ReleasePrimitiveArrayCritical(glyphs, gpStorage, JNI_ABORT);
    }

    // don't forget!
    delete gStrike;
    delete sStrikep;
  }
}
#endif

/*
 * Class:     sun_awt_font_NativeFontWrapper
 * Method:    getItalicAngle
 * Signature: (Ljava/awt/Font;[FZZ)F
 */
JNIEXPORT jfloat JNICALL
Java_sun_awt_font_NativeFontWrapper_getItalicAngle
    (JNIEnv *env, jclass clsNFW, jobject theFont,
    jdoubleArray matrix, jboolean isAntiAliased, jboolean usesFractionalMetrics)
{
    jfloat retVal = 0.0;
    fontObject *fo = ::getFontPtr(env, theFont);
    if (fo && fo->isValid) {
        FontTransform tx(env, matrix);
        Strike& theStrike = fo->getStrike(tx, isAntiAliased, usesFractionalMetrics);
        hsFixedPoint2 italicAngle;
        theStrike.GetItalicAngle(italicAngle);
        if (italicAngle.fY != 0) {
            // REMIND: caller code is expecting inverse slope of caret
            retVal = hsFixedToFloat(italicAngle.fX)
                        / hsFixedToFloat(italicAngle.fY);
        }
    }
    return retVal;
}

/*
 * Class:     sun_awt_font_NativeFontWrapper
 * Method:    getMissingGlyphCode
 * Signature: (Ljava/awt/Font;)I
 */
JNIEXPORT jint JNICALL
Java_sun_awt_font_NativeFontWrapper_getMissingGlyphCode
        (JNIEnv *env, jclass clsNFW, jobject theFont)
{
    jint retval = 0;
    fontObject *fo = ::getFontPtr(env, theFont);
    if (fo && fo->isValid) {
        Strike& theStrike = fo->getStrike();
        retval = theStrike.getMissingGlyphCode();
    }
    return retval;
}

/*
 * Class:     sun_awt_font_NativeFontWrapper
 * Method:    getNumFonts
 * Signature: ()I
 */
JNIEXPORT jint JNICALL
Java_sun_awt_font_NativeFontWrapper_getNumFonts(JNIEnv *env, jclass clsNFW)
{
    return ::CountFontObjects();
}

/*
 * Class:     sun_awt_font_NativeFontWrapper
 * Method:    getNumGlyphs
 * Signature: (Ljava/awt/Font;)I
 */
JNIEXPORT jint JNICALL
Java_sun_awt_font_NativeFontWrapper_getNumGlyphs
        (JNIEnv *env, jclass clsNFW, jobject theFont)
{
    jint retVal = 0;
    fontObject *fo = ::getFontPtr(env, theFont);

    if (fo && fo->isValid) {
        Strike& theStrike = fo->getStrike();
        //return theStrike.GetNumGlyphs();
        retVal = theStrike.GetNumGlyphs();
    }
    //return 0;
    return retVal;
}

/*
 * Class:     sun_awt_font_NativeFontWrapper
 * Method:    initializeFont
 * Signature: (Ljava/awt/Font;Ljava/lang/String;I)V
 */
JNIEXPORT void JNICALL
Java_sun_awt_font_NativeFontWrapper_initializeFont
        (JNIEnv *env, jclass clsNFW, jobject theFont, jstring name, jint style)
{
    fontObject* fo = NULL;

    if (!JNU_IsNull (env, name)) {
        const UInt16 *fontName = NULL;
        unsigned int nameLen = 0;
        fontName = (UInt16 *)env->GetStringChars(name, NULL);
        if (!fontName) {
            JNU_ThrowIllegalArgumentException(env, "Name");
            return;
        }
        nameLen = env->GetStringLength(name);
        fo = ::FindFontObject(fontName, nameLen, style);
        env->ReleaseStringChars(name, fontName);
    } else {
        fo = ::FindFontObject(NULL, 0, style);
    }
    if (fo == NULL) {
        fo = ::DefaultFontObject();
    }
    ::setFontPtr(env, theFont, fo);
}

/*
 * Class:     sun_awt_font_NativeFontWrapper
 * Method:    layoutGlyphVector
 * Signature: (Ljava/awt/Font;[FZZFFLjava/awt/font/GlyphVector;)V
 */
JNIEXPORT void JNICALL Java_sun_awt_font_NativeFontWrapper_layoutGlyphVector
    (JNIEnv *env, jclass clsNFW, jobject theFont,
            jdoubleArray fontTX, jdoubleArray devTX, 
            jboolean isAntiAliased, jboolean usesFractionalMetrics,
            jobject target)
{
    fontObject *fo = ::getFontPtr(env, theFont);
    if (fo && fo->isValid) {
        GlyphVector gv(env, fontTX, devTX, isAntiAliased, usesFractionalMetrics, fo);
        gv.getGlyphCodes(target);
        gv.getTransforms(target);
        gv.positionGlyphs();
        gv.writePositions(target);
    }
}

JNIEXPORT jobject JNICALL Java_sun_awt_font_NativeFontWrapper_getGlyphVectorOutline
    (JNIEnv *env, jclass clsNFW, jobject glyphVector, jobject theFont,
            jdoubleArray fontTX, jdoubleArray devTX,
            jboolean isAntiAliased, jboolean usesFractionalMetrics,
            jfloat x, jfloat y)
{
    fontObject *fo = ::getFontPtr(env, theFont);
    if (fo && fo->isValid) {
        GlyphVector gv(env, fontTX, devTX, isAntiAliased, usesFractionalMetrics, fo);
        gv.getGlyphVector(glyphVector);
        return gv.getOutline(x, y);
    }
    return NULL;
}

JNIEXPORT jobject JNICALL Java_sun_awt_font_NativeFontWrapper_getGlyphOutline
    (JNIEnv *env, jclass clsNFW, jobject glyphVector, jint glyphIndex, jobject theFont,
            jdoubleArray fontTX, jdoubleArray devTX,
            jboolean isAntiAliased, jboolean usesFractionalMetrics,
            jfloat x, jfloat y)
{
    fontObject *fo = ::getFontPtr(env, theFont);
    if (fo && fo->isValid) {
        GlyphVector gv(env, fontTX, devTX, isAntiAliased, usesFractionalMetrics, fo);
        gv.getGlyphVector(glyphVector);
        return gv.getGlyphOutline(glyphIndex, x, y);
    }
    return NULL;
}

/*
 * Class:     sun_awt_font_NativeFontWrapper
 * Method:    populateAndLayoutGlyphVector
 * Signature: (Ljava/awt/Font;[CIIIZ[D[DZZLjava/awt/font/GlyphVector;)V
 */
JNIEXPORT void JNICALL
Java_sun_awt_font_NativeFontWrapper_populateAndLayoutGlyphVector
    (JNIEnv *env, jclass clsNFW, jobject theFont, jcharArray unicodes, jint start, jint length,
            jint flags, jdoubleArray fontTX, jdoubleArray devTX,
            jboolean isAntiAliased, jboolean usesFractionalMetrics,
            jobject target)
{
    fontObject *fo = ::getFontPtr(env, theFont);

    if (fo) {
	    GlyphLayout gl(env, unicodes, start, length, flags, fontTX, devTX,
		       isAntiAliased, usesFractionalMetrics, fo);

	gl.updateGlyphVector(env, target, 0);

    }
}

/*
 * Class:     sun_awt_font_NativeFontWrapper
 * Method:    populateGlyphVector
 * Signature: (Ljava/awt/Font;[CII[DZZLjava/awt/font/GlyphVector;)V
 */
JNIEXPORT void JNICALL
Java_sun_awt_font_NativeFontWrapper_populateGlyphVector
    (JNIEnv *env, jclass clsNFW,
        jobject theFont, jcharArray unicodes, jint offset, jint count,
        jdoubleArray fontTX,
        jboolean isAntiAliased, jboolean usesFractionalMetrics,
        jobject target)
{
    fontObject *fo = ::getFontPtr(env, theFont);
    if (fo) {
        GlyphVector gv(env, fontTX, NULL, isAntiAliased, usesFractionalMetrics, fo);
        gv.setText(unicodes, offset, count);
        gv.writeGlyphCodes(target);
    }
}

/*
 * Class:     sun_awt_font_NativeFontWrapper
 * Method:    shapeGlyphVector
 * Signature: (Ljava/awt/font/GlyphVector;Ljava/awt/Font;[FZZLjava/lang/Object;Ljava/awt/font/GlyphVector;)V
 */
JNIEXPORT void JNICALL Java_sun_awt_font_NativeFontWrapper_shapeGlyphVector
    (JNIEnv *, jclass, jobject, jobject, jdoubleArray, jboolean, jboolean, jobject, jobject)
{
    // REMIND finish me
    // we don't do glyph shaping yet.
}

/*
 * Class:     sun_awt_font_NativeFontWrapper
 * Method:    registerCompositeFont
 * Signature: (Ljava/lang/String;[Ljava/lang/String;[I[I)V
 */
JNIEXPORT void JNICALL
Java_sun_awt_font_NativeFontWrapper_registerCompositeFont
  (JNIEnv *env, jclass clsNFW,
    jstring compositeFontName,
    jobjectArray componentFontNames,
    jintArray exclusionRanges, jintArray exclusionMaxIndex)
{
    if (JNU_IsNull (env, compositeFontName)
            || JNU_IsNull (env, componentFontNames)
            || JNU_IsNull (env, exclusionRanges)
            || JNU_IsNull (env, exclusionMaxIndex)) {
        return;
    }
    jsize numComponents = env->GetArrayLength(componentFontNames);
    if (numComponents <= 0) {
        return;
    }
    CompositeFont *cf = new CompositeFont(env, compositeFontName);
    if (!cf) {
        return;
    }
    cf->addRanges(env, exclusionRanges);
    cf->addMaxIndices(env, exclusionMaxIndex);
    for (int i = 0; i < numComponents; i++) {
        jobject temp = env->GetObjectArrayElement(componentFontNames, i);
        jstring theName = (jstring)temp;
        JStringBuffer componentName(env, theName);

        cf->registerComponentName(i, componentName);
    }
    if (cf->isValid()) {
        registerFont(kCompositeFontFormat, cf);
    }
}

/*
 * Class:     sun_awt_font_NativeFontWrapper
 * Method:    registerFonts
 * Signature: (Ljava/util/Vector;II)V
 */
JNIEXPORT void JNICALL Java_sun_awt_font_NativeFontWrapper_registerFonts
        (JNIEnv *env, jclass obj, jobject fonts, jint size, 
                jobject names, jint format, jboolean useJavaRasterizer);
// REMIND finish me


/*
 * Class:     sun_awt_font_NativeFontWrapper
 * Method:    isFontRegistered
 * Signature: (Ljava/lang/String;I)Z
 */
JNIEXPORT jboolean JNICALL
Java_sun_awt_font_NativeFontWrapper_isFontRegistered
        (JNIEnv *env, jclass clsNFW, jstring name, jint style)
{
    fontObject* fo = NULL;

    if (JNU_IsNull (env, name)) {
	JNU_ThrowNullPointerException(env, "name");
    }
    const UInt16 *fontName = NULL;
    unsigned int nameLen = 0;
    fontName = (UInt16 *)env->GetStringChars(name, NULL);
    if (fontName == NULL) {
        JNU_ThrowIllegalArgumentException(env, "Name");
        return JNI_FALSE;
    }
    nameLen = env->GetStringLength(name);
    fo = ::FindFontObject(fontName, nameLen, style);
    env->ReleaseStringChars(name, fontName);

    return (fo != NULL) ? JNI_TRUE : JNI_FALSE;
}

/*
 * Class:     sun_awt_font_NativeFontWrapper
 * Method:    getPlatformFontVar
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL
Java_sun_awt_font_NativeFontWrapper_getPlatformFontVar(JNIEnv *env, jclass cl) {

    char *c = getenv("JAVA2D_USEPLATFORMFONT");
    if (c) {
        return JNI_TRUE;
    } else {
        return JNI_FALSE;
    }
}


/*
 * Class:     sun_awt_font_NativeFontWrapper
 * Method:    fontSupportsEncoding
 * Signature: (Ljava/awt/Font;Ljava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL
Java_sun_awt_font_NativeFontWrapper_fontSupportsEncoding(JNIEnv *env,
							 jclass cl,
							 jobject theFont,
							 jstring theEncoding) {
    
    return FOFontSupportsEncoding(env, theFont, theEncoding);
}

};  // end of extern "C"

#ifdef NEED_BOGUS_DELETE_OPERATORS
// REMIND: lame.  the C++ compiler/linker complain if there
// is no implementation (Solaris)
void UInt32Buffer::operator delete(void *ptr)
{
}
// REMIND: lame.  the C++ compiler/linker complain if there
// is no implementation (Solaris)
void JFloatBuffer::operator delete(void *ptr)
{
}
// REMIND: lame.  the C++ compiler/linker complain if there
// is no implementation (Solaris)
void JStringBuffer::operator delete(void *ptr)
{
}
// REMIND: lame.  the C++ compiler/linker complains if there
// is no implementation (Solaris)
void JStringBufferCopy::operator delete(void *ptr)
{
}
#endif


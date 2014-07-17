/*
 *  @(#)GlyphLayout.cpp	1.16 03/01/23
 *
 * (C) Copyright IBM Corp. 1999-2001 - All Rights Reserved
 *
 * Portions Copyright 2003 by Sun Microsystems, Inc.,
 * 901 San Antonio Road, Palo Alto, California, 94303, U.S.A.
 * All rights reserved.
 *
 * This software is the confidential and proprietary information
 * of Sun Microsystems, Inc. ("Confidential Information").  You
 * shall not disclose such Confidential Information and shall use
 * it only in accordance with the terms of the license agreement
 * you entered into with Sun.
 *
 * The original version of this source code and documentation is
 * copyrighted and owned by IBM. These materials are provided
 * under terms of a License Agreement between IBM and Sun.
 * This technology is protected by multiple US and International
 * patents. This notice and attribution to IBM may not be removed.
 */

#include "FontGlue.h"
#include "Strike.h"
#include "GlyphLayout.h"
#include "TX.h"

#include "LETypes.h"
#include "LEFontInstance.h"
#include "LayoutEngine.h"
#include "FontInstanceAdapter.h"

#include "ScriptRun.h"

#include "CompositeFont.h"
#include "java_awt_Font.h"

#define ARRAY_SIZE(array) (sizeof array  / sizeof array[0])

static jclass   g_gvClass       = 0;
static jfieldID g_gvPositions   = 0;
static jfieldID g_gvGlyphs      = 0;
static jfieldID g_gvCharIndices = 0;

static Boolean initGVIDs(JNIEnv *env, jobject gv) {
    if (g_gvClass == NULL) {
        g_gvClass = env->GetObjectClass(gv);

        if (g_gvClass == NULL) {
            JNU_ThrowClassNotFoundException(env, "No GlyphVector class");
            return false;
        }
    }

    if (g_gvPositions != NULL && g_gvGlyphs != NULL) {
        return true;
    }

    g_gvPositions   = env->GetFieldID(g_gvClass, "positions", "[F");
    g_gvGlyphs      = env->GetFieldID(g_gvClass, "glyphs", "[I");
    g_gvCharIndices = env->GetFieldID(g_gvClass, "charIndices", "[I");

    if (g_gvPositions == NULL || g_gvGlyphs == NULL || g_gvCharIndices == NULL) {
        JNU_ThrowNoSuchFieldException(env,
                                      "GlyphVector positions, glyphs, or charIndices");
        return false;
    }

    return true;
}

/*
// Hacks for debugging
char *getScriptName(int scriptTag)
{
    static char name[5];
    int i;

    for (i = 0; i < 5; i += 1) {
        name[i] = (scriptTag >> (24 - 8 * i)) & 0xFF;
    }

    name[4] = '\0';

    return name;
}

char *getFontName(fontObject *fo)
{
    int foNameLen;
    const Unicode *foName = fo->GetFontName(foNameLen);
    static char name[100];
    int ii;

    for (ii = 0; ii < foNameLen; ii += 1) {
        name[ii] = (char) foName[ii];
    }
    name[ii] = '\0';

    return name;
}
*/

fontObject *getScriptFont(fontObject *fo, le_int32 scriptSlot)
{
    fontObject *sfo = fo;
    enum FontFormats format = fo->GetFormat();

    if (format == kCompositeFontFormat) {
        CompositeFont *cf = (CompositeFont *) fo;

	    sfo = cf->getSlotFont(scriptSlot);
	    sfo->m_currentStyle = cf->m_currentStyle;

	    if (sfo->m_currentStyle == java_awt_Font_PLAIN) {
	        sfo->m_currentStyle = Strike::algorithmicStyle(*cf, *sfo, scriptSlot);
	    }
    }

    return sfo;
}

void GlyphLayout::growScriptInfo()
{
	ScriptInfo *old = fScriptInfo;
	
	fScriptInfo = new ScriptInfo[fScriptMax + eGrowStorage + 1];

	LE_ARRAY_COPY(fScriptInfo, old, fScriptMax);
	fScriptMax += eGrowStorage;

	if (old != fScriptInfoBase) {
		delete old;
	}
}

void charsToGlyphs(const Unicode16 *chars, Int32 count, Strike *strike, UInt32 *glyphs)
{
    for (Int32 i = 0; i < count; i += 1) {
        Unicode16 high = chars[i];
        Unicode32 code = high;

        if (high >= 0xD800 && high <= 0xDBFF && i < count - 1) {
            Unicode16 low = chars[i + 1];

            if (low >= 0xDC00 && low <= 0xDFFF) {
                code = (high - 0xD800) * 0x400 + low - 0xDC00 + 0x10000;
            }
        }

        glyphs[i] = strike->CharToGlyph(code);

        if (code > 0x10000) {
            UInt32 missingGlyph = (glyphs[i] & 0xFF000000) | 0xFFFF;

            glyphs[++i] = missingGlyph;
        }
    }
}

GlyphLayout::GlyphLayout(JNIEnv *env, jcharArray unicodes, jint offset, jint count,
            jint flags, jdoubleArray fontTX, jdoubleArray devTX, jboolean isAntiAliased,
            jboolean usesFractionalMetrics, fontObject *fo)
  : fNumGlyphs(0), fScriptCount(0), fScriptMax(eDefaultStorage), fRightToLeft(false), fDevTX(env, devTX)
{
    if (JNU_IsNull(env, unicodes) ) {
        JNU_ThrowIllegalArgumentException(env,"Unicode array is NULL!");
        return;
    }

    jint max = env->GetArrayLength(unicodes);

    if (offset + count > max) {
        JNU_ThrowArrayIndexOutOfBoundsException(env,
                                        "chars [offset + count]");
        return;
    }

    // !!! check to see how layout should adapt to devTX.  Should it ignore devTX
    // and then adjust the information by devTX afterwards?  Then it would perhaps
    // get incorrectly hinted advance information by the strike.  But if it uses
    // devTX then the advance and position information needs to be normalized
    // before returning it to native.

    TX gtx(fDevTX);
    TX ftx(env, fontTX);
    gtx.concat(ftx);
    FontTransform tx(gtx.m00, gtx.m10, gtx.m01, gtx.m11);

    jfloat x = (jfloat)gtx.m02;
    jfloat y = (jfloat)gtx.m12;

	const jchar *theChars = (const jchar *) env->GetPrimitiveArrayCritical(unicodes, NULL);

	if (theChars != NULL) {
	    const jchar *oldChars = theChars; // theChars can get changed, save original
        if (flags != 0) {
            if ((flags & 0x0001) != 0) {
                fRightToLeft = true;
            }

            if ((flags & 0x0002) != 0) {
                theChars += offset;
                max -= offset;
                offset = 0;
            }

            if ((flags & 0x0004) != 0) {
                max = offset + count;
            }
        }

        if (fo->GetFormat() == kCompositeFontFormat) {
            Strike *strike = &fo->getStrike(tx, isAntiAliased, usesFractionalMetrics);
            UInt32 *glyphs = new UInt32[count];
            ScriptRun scriptRun(theChars, offset, count);
            le_uint32 slot;

            charsToGlyphs(&theChars[offset], count, strike, glyphs);

            fScriptInfo = fScriptInfoBase;
            slot = glyphs[0] >> 24;

            while (scriptRun.next()) {
                le_int32 scriptStart = scriptRun.getScriptStart();
                le_int32 scriptEnd   = scriptRun.getScriptEnd();
                le_int32 scriptCode  = scriptRun.getScriptCode();

                le_int32 ch = scriptStart;
                while (ch < scriptEnd) {
                    while (ch < scriptEnd && slot == glyphs[ch - offset] >> 24) {
                        ch += 1;
                    }

                    if (fScriptCount >= fScriptMax) {
                        growScriptInfo();
                    }

                    fScriptInfo[fScriptCount].scriptStart = scriptStart;
                    fScriptInfo[fScriptCount].scriptCode  = scriptCode;
                    fScriptInfo[fScriptCount].scriptSlot  = slot;

                    fScriptCount += 1;

                    if (ch < scriptEnd) {
                        slot = glyphs[ch - offset] >> 24;
                        scriptStart = ch;
                    }
                }
            }

            delete[] glyphs;
            fScriptInfo[fScriptCount].scriptStart = scriptRun.getScriptEnd();
        } else {
	        ScriptRun scriptRun(theChars, offset, count);

	        fScriptInfo = fScriptInfoBase;
	        
	        while (scriptRun.next()) {
		        le_int32 scriptStart = scriptRun.getScriptStart();
		        le_int32 scriptCode  = scriptRun.getScriptCode();

		        if (fScriptCount >= fScriptMax) {
			        growScriptInfo();
		        }

		        fScriptInfo[fScriptCount].scriptStart = scriptStart;
		        fScriptInfo[fScriptCount].scriptCode  = scriptCode;
                fScriptInfo[fScriptCount].scriptSlot  = 0;

                fScriptCount += 1;
	        }

	        fScriptInfo[fScriptCount].scriptStart = scriptRun.getScriptEnd();
        }

	    le_int32 script = 0, stop = fScriptCount, dir = 1;
	    le_int32 glyphCount = 0;

	    if (fRightToLeft) {
		    script = fScriptCount - 1;
		    stop = -1;
		    dir = -1;
	    }

	    while (script != stop) {
		    le_int32 scriptStart = fScriptInfo[script].scriptStart;
		    le_int32 scriptCount = fScriptInfo[script + 1].scriptStart - scriptStart;
		    le_int32 scriptCode  = fScriptInfo[script].scriptCode;
		    le_int32 scriptSlot  = fScriptInfo[script].scriptSlot;
		    fontObject *sfo = getScriptFont(fo, scriptSlot);
		    Strike *strike = &sfo->getStrike(tx, isAntiAliased, usesFractionalMetrics);
		    FontInstanceAdapter fontInstance(sfo, strike, &tx, 72, 72);
		    LayoutEngine *engine;
		    LEErrorCode success = LE_NO_ERROR;

			engine = LayoutEngine::layoutEngineFactory(&fontInstance, scriptCode, -1, success);

		    glyphCount = engine->layoutChars(theChars, scriptStart, scriptCount, max, fRightToLeft, x, y, success);
		    
		    fNumGlyphs += glyphCount;
		    engine->getGlyphPosition(glyphCount, x, y, success);
		    fScriptInfo[script].engine = engine;
		    script += dir;
	    }

        env->ReleasePrimitiveArrayCritical(unicodes, (void *)oldChars, JNI_ABORT);
	}
}

GlyphLayout::~GlyphLayout()
{
	for (le_int32 i = 0; i < fScriptCount; i += 1) {
		delete fScriptInfo[i].engine;
	}

    if (fScriptInfo != fScriptInfoBase) {
        delete[] fScriptInfo;
        fScriptInfo = NULL;
    }
}

void GlyphLayout::updateGlyphVector(JNIEnv *env, jobject glyphVector, int extraBits)
{
  if (initGVIDs(env, glyphVector)) {
    le_int32 glyphsSoFar, start = 0, end = fScriptCount, dir = 1;
    jintArray glyphsArray = env->NewIntArray(fNumGlyphs);
    if (glyphsArray == NULL) {
      return;
    }

    if (fRightToLeft) {
      start = fScriptCount - 1;
      end = dir = -1;
    }

    // Note: The extra nesting is needed to make glyphBuffer
    // go out of scope before the SetObjectField call...
    {
      UInt32Buffer glyphBuffer(env, glyphsArray);
      le_uint32 *glyphs = glyphBuffer.buffer();

      if (glyphs != NULL) {
	LEErrorCode success = LE_NO_ERROR;
				
	glyphsSoFar = 0;

	for (le_int32 s = start; s != end; s += dir) {
	  fScriptInfo[s].engine->getGlyphs(&glyphs[glyphsSoFar], fScriptInfo[s].scriptSlot << 24, success);
	  glyphsSoFar += fScriptInfo[s].engine->getGlyphCount();
	}
      }
    }
    env->SetObjectField(glyphVector, g_gvGlyphs, glyphsArray);


    // !!! desire optimization to not initialize index array when
    // it is default (all 1-1, ltr) and perhaps set a flag to
    // indicate when it is rtl default (all 1-1, rtl) as well.

    jintArray charIndicesArray = env->NewIntArray(fNumGlyphs);
    if (charIndicesArray == NULL) {
      return;
    }

    // Note: The extra nesting is needed to make charIndicesBuffer
    // go out of scope before the SetObjectField call...
    // FIXME: really should have an Int32Buffer class for this...
    {
      UInt32Buffer charIndicesBuffer(env, charIndicesArray);
      le_int32 *charIndices = (le_int32 *) charIndicesBuffer.buffer();

      if (charIndices != NULL) {
	LEErrorCode success = LE_NO_ERROR;

	glyphsSoFar = 0;

	for (le_int32 s = start; s != end; s += dir) {
	  le_int32 scriptBase = fScriptInfo[s].scriptStart - fScriptInfo[0].scriptStart;

	  fScriptInfo[s].engine->getCharIndices(&charIndices[glyphsSoFar], scriptBase, success);
	  glyphsSoFar += fScriptInfo[s].engine->getGlyphCount();
	}
      }
    }
    env->SetObjectField(glyphVector, g_gvCharIndices, charIndicesArray);

    // !!! desire optimization to indicate when position data is all
    // default (accumulated advances of glyphs as returned from strike)
    // so that rendering can avoid recomputing device position data
    // from logical position data.

    jfloatArray positions = env->NewFloatArray(fNumGlyphs * 2 + 2);
    if (positions == NULL) {
      return;
    }

    // Note: The extra nesting is needed to make floatBuffer
    // go out of scope before the SetObjectField call...
    {
      JFloatBuffer floatBuffer(env, positions);
      jfloat *floats = floatBuffer.buffer();

      if (floats != NULL) {
	LEErrorCode success = LE_NO_ERROR;
				
	glyphsSoFar = 0;

	for (le_int32 s = start; s != end; s += dir) {
	  fScriptInfo[s].engine->getGlyphPositions(&floats[glyphsSoFar * 2], success);
	  glyphsSoFar += fScriptInfo[s].engine->getGlyphCount();
	}
      }

      if (!fDevTX.isIdentity()) {
	TX inv(fDevTX);
	if (!inv.invert()) {
	  JNU_ThrowInternalError(env, "Could not invert devTX");
	} else {
	  inv.transform(floats, fNumGlyphs + 1);
	}
      }
    }
    env->SetObjectField(glyphVector, g_gvPositions, positions);
  }
}

// REMIND: lame.  the C++ compiler/linker complain if there
// is no implementation
void GlyphLayout::operator delete(void *ptr)
{
}


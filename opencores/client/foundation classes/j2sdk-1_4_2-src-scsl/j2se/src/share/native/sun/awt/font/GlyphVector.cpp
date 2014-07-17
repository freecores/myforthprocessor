/*
 * @(#)GlyphVector.cpp	1.56 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @author Charlton Innovations, Inc.
 */

#include "GlyphVector.h"
#include "Strike.h"
#include "TX.h"
#include "GeneralPath.h"
#include "DefaultCharMapper.h"

// REMIND: speedup if we write a "hsDesc" which can live on the stack

static jfieldID g_gvPositions   = 0;
static jfieldID g_gvGlyphs      = 0;
static jfieldID g_gvCharIndices = 0;
static jfieldID g_gvTransforms  = 0;
static jfieldID g_gvTXIndices   = 0;

// The invisible glyph has no image, no outline, and no advance.
// This is done here rather than at a lower level in order to localize this
// change and keep it consistent across different font types and platforms.
// Use the value ffff instead of 1 (TrueType convention) in order to avoid
// taking over a glyphID that has another use (i.e. in type1 fonts).

#define INVISIBLE_GLYPH_ID 0xffff

// We now have multiple invisible glyphs, one for ligatured-away base
// characters, and one for ligatured-away combining marks.  The first
// is glyph id 0xffff and the second is glyph id 0xfffe.

#define IS_INVISIBLE_GLYPH(gid) ((gid & 0x0fffe) == 0x0fffe)

/*
 StrikeTable provides a way to handle multiple strikes per glyphvector,
 which is the case when setGlyphTransform has been called.

 The transform data comes in as two arrays, one array of jints which 
 has an entry for each glyph, and another array of jdoubles, with
 six doubles for each transform (as in AffineTransform).  The number
 of transforms depends on the number of unique transforms applied to
 glyphs in the glyphvector.

 The index array is held in fTXIndices, and the transforms array in
 fTransforms.  A zero entry in fTXIndices means there is no special
 transform in fTransforms, and the standard font transform should
 be used.  A value of 1 means the first transform (at offset 0 in
 fTransforms) is used, a value of 2 means the second transform 
 (at offset 6 in fTransforms) is used, and so on.

 The striketable builds a list of strikes, one per transform, and
 holds them in the auxStrikes array.  For convenience the font
 strike is aliased and held in the 0th entry-- this means the
 strike table matches the fTXIndices array, in that the value
 in the fTXIndices array can index the strike without adjustments.

 Currently (we must decide before fcs) the glyph transform is
 applied after the font transform.
 */
class StrikeTable {
   const GlyphVector &fGV;
   TX *pdevTX;
   Strike **auxStrikes;
   Strike &theStrike;

public:
   StrikeTable(const GlyphVector &gv, TX* devTX = 0);
   ~StrikeTable();

    inline Strike& getStrike(int index) const {
	if (auxStrikes) {
	    // cast away const
	    return *((StrikeTable*)this)->internalGetStrike(fGV.fTXIndices[index]);
	} else {
	    return theStrike;
	}
    }

    inline Strike& getStrike() const { 
	return theStrike;
    }

    Boolean getStrikeMetrics(int index, int glyphID, hsGGlyph &glyphRef, hsFixedPoint2 &advXY) {
	if (auxStrikes) {
	    return internalGetStrikeMetrics(index, glyphID, glyphRef, advXY);
	} else {
            return theStrike.getMetrics(glyphID, glyphRef, advXY);
	}
    }

    Boolean getStrikeMetricsWithImage(int index, int glyphID, hsGGlyph &glyphRef, hsFixedPoint2 &advXY, Boolean *refCheck = 0) {
	if (auxStrikes) {
	    return internalGetStrikeMetricsWithImage(index, glyphID, glyphRef, advXY, refCheck);
	} else {
	    return theStrike.getMetricsWithImage(glyphID, glyphRef, advXY, refCheck);
	}
    }

    void getStrikeOutlineBounds(int index, int glyphID, hsRect &bounds) {
	if (auxStrikes) {
	    internalGetStrikeOutlineBounds(index, glyphID, bounds);
	} else {
	    theStrike.getOutlineBounds(glyphID, bounds);
	}
    }

    Strike* internalGetStrike(int txIndex);
    Boolean internalGetStrikeMetrics(int index, int glyphID, hsGGlyph &glyphRef, hsFixedPoint2 &advXY);
    Boolean internalGetStrikeMetricsWithImage(int index, int glyphID, hsGGlyph &glyphRef, hsFixedPoint2 &advXY, Boolean *refCheck = 0);
    void    internalGetStrikeOutlineBounds(int index, int glyphID, hsRect &bounds);
    void deltaGlyphInfo(int index, hsFixedPoint2 &origin, hsFixedPoint2 &advance);
};

StrikeTable::StrikeTable(const GlyphVector &gv, TX* devTX)
    : fGV(gv)
    , pdevTX(devTX)
    , auxStrikes(0)
    , theStrike(gv.getGlyphStrike(devTX))
{
    if (fGV.fTransforms) {
	auxStrikes = (Strike**)malloc((fGV.fTXCount + 1) * sizeof(Strike*));
	if (auxStrikes) {
	    auxStrikes[0] = &theStrike;
	    for (int i = 1; i <= fGV.fTXCount; ++i) {
		auxStrikes[i] = 0;
	    }
	}
    }
}

Strike* 
StrikeTable::internalGetStrike(int tx) {
    if (auxStrikes) {
	if (!auxStrikes[tx]) {
	    TX auxTX;
	    if (pdevTX) {
		auxTX.concat(*pdevTX);
	    }

	    TX trans(&fGV.fTransforms[(tx-1) * 6]);
	    auxTX.concat(trans);

	    TX fontTX(fGV.fFontTX);
	    auxTX.concat(fontTX);

	    // !!! adapter
	    FontTransform ftx(auxTX.m00, auxTX.m10, auxTX.m01, auxTX.m11);
	    // !!! cast away const
	    auxStrikes[tx] = new Strike(*(fontObject*)fGV.fFont, ftx, fGV.fIsAntiAliased, fGV.fUsesFractionalMetrics);
	}
	return auxStrikes[tx];
    } else {
	return &theStrike;
    }
}

StrikeTable::~StrikeTable()
{
    if (auxStrikes) {
        for (int i = 1; i <= fGV.fTXCount; ++i) { // note start at 1, strike at 0 is an alias, include index at fTXCount
	    delete auxStrikes[i];
	}
	free(auxStrikes);
	auxStrikes = NULL;
    }
}

/*
 This adjusts the metrics by the translation components of the glyph
 transform.  It is done here since the translation is not known by the
 strike.  
*/
Boolean
StrikeTable::internalGetStrikeMetrics(int index, int glyphID, hsGGlyph &glyphRef, hsFixedPoint2 &advXY) {
    int tx = fGV.fTXIndices[index];
    Strike* s = internalGetStrike(tx);
    if (s->getMetrics(glyphID, glyphRef, advXY)) {
	if (tx) {
	    deltaGlyphInfo(index, glyphRef.fTopLeft, advXY);
	}
	return true;
    }
    return false;
}

/*
 This adjusts the metrics by the translation components of the glyph
 transform.  It is done here since the translation is not known by the
 strike.  
*/
Boolean 
StrikeTable::internalGetStrikeMetricsWithImage(int index, int glyphID, hsGGlyph &glyphRef, hsFixedPoint2 &advXY, Boolean *refCheck) {
    int tx = fGV.fTXIndices[index];
    Strike* s = internalGetStrike(tx);
    if (s->getMetricsWithImage(glyphID, glyphRef, advXY, refCheck)) {
	if (tx) {
	    deltaGlyphInfo(index, glyphRef.fTopLeft, advXY);
	}
	return true;
    }
    return false;
}

/*
 This offsets the bounds of the glyph by the translation components of
 the glyph transform.  It is done here since the translation is not
 known by the strike. Since there is no devTX applied to the bounds
 information, this takes a shortcut and directly applies the glyph
 transform offset to the bounds data. Note that the index has to be
 adjusted to account for the fact that a 1 value in txIndices means to
 use the 0 element in fTransforms.  The translation components are
 elements 4 and 5 of the transform.  
*/
void
StrikeTable::internalGetStrikeOutlineBounds(int index, int glyphID, hsRect &bounds)
{
    int tx = fGV.fTXIndices[index];
    Strike* s = internalGetStrike(tx);
    s->getOutlineBounds(glyphID, bounds);
    if (tx) {
        int offset = (tx - 1) * 6 + 4;
	float dx = fGV.fTransforms[offset];
	float dy = fGV.fTransforms[offset + 1];
	if (dx) {
	    hsScalar sdx = hsFloatToScalar(dx);
	    bounds.fLeft += sdx;
	    bounds.fRight += sdx;
	}
	if (dy) {
	    hsScalar sdy = hsFloatToScalar(dy);
	    bounds.fTop += sdy;
	    bounds.fBottom += sdy;
	}
    }
}

/*
 This adjusts the origin and advance of the glyph.  This data is in
 raster space, so the device transform is applied to the translation
 before it is added to the origin and advance.  
*/
void 
StrikeTable::deltaGlyphInfo(int index, hsFixedPoint2 &origin, hsFixedPoint2 &advance) {
    int tx = fGV.fTXIndices[index];
    if (tx) {
        int offset = (tx - 1) * 6 + 4;
    float dx = fGV.fTransforms[offset];
    float dy = fGV.fTransforms[offset + 1];

    if (pdevTX) {
	pdevTX->vectorTransform(dx, dy);
    }
    if (dx) {
	hsFixed fdx = hsFloatToFixed(dx);
	origin.fX += fdx;
	advance.fX += fdx;
    }
    if (dy) {
	hsFixed fdy = hsFloatToFixed(dy);
	origin.fY += fdy;
	advance.fY += fdy;
    }
}
}

static Boolean initGVIDs(JNIEnv *env, jobject gv) {
    if (!g_gvPositions) {        
	jclass gvClass = env->GetObjectClass(gv);
        if (!gvClass) {
            JNU_ThrowClassNotFoundException(env, "No GlyphVector class");
            return false;
        }

	g_gvPositions = env->GetFieldID(gvClass, "positions", "[F");
	g_gvGlyphs = env->GetFieldID(gvClass, "glyphs", "[I");
	g_gvCharIndices = env->GetFieldID(gvClass, "charIndices", "[I");
	g_gvTransforms = env->GetFieldID(gvClass, "transforms", "[D");
	g_gvTXIndices = env->GetFieldID(gvClass, "txIndices", "[I");
	if (!(g_gvPositions && g_gvGlyphs && g_gvCharIndices && g_gvTransforms && g_gvTXIndices)) {
	    g_gvPositions = 0;
	    g_gvGlyphs = 0;
	    g_gvCharIndices = 0;
	    g_gvTransforms = 0;
	    g_gvTXIndices = 0;
	    JNU_ThrowNoSuchFieldException(env, "Missing required GlyphVector field");
	    return false;
	}
    }
    return true;
}

static jobject makeFloatRect(JNIEnv *env, jfloat l, jfloat t, jfloat w, jfloat h) {
  static jclass rectClass = 0;
  static jmethodID rectCC = 0;
  static const char* rectClassName = "java/awt/geom/Rectangle2D$Float";
  static const char* rectCCName = "<init>";
  static const char* rectCCArgs = "(FFFF)V";

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

static jobject makeRect(JNIEnv *env, jint l, jint t, jint w, jint h) {
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

/**
 * Set the number of glyphs.
 *
 * This will release old storage if it was allocated and either the new
 * numGlyphs > the old numGlyphs or the new numGlyphs is 0 (for use by delete);
 */
void 
GlyphVector::setNumGlyphs(jint numGlyphs)
{
    if (numGlyphs == 0 || (UInt32)numGlyphs > fNumGlyphs) {
	if (fNumGlyphs > eDefaultStorage) {
	    delete [] fGlyphs; fGlyphs = NULL;
	    delete [] fCharIndices; fCharIndices = NULL;
	    delete [] fImageRefs; fImageRefs = NULL;
	}
	if (fNumGlyphs >= eDefaultStorage) {
	    delete [] fPositions; fPositions = NULL;
	}
    }
    fNumGlyphs = numGlyphs;
}

/**
 * Set the fGlyphs array to hold numGlyphs glyphs.  
 * Calls setNumGlyphs to reset all arrays if necessary.
 */
void 
GlyphVector::allocateGlyphs(jint numGlyphs)
{
    setNumGlyphs(numGlyphs);
    if (fGlyphs == NULL) {
	if (fNumGlyphs <= eDefaultStorage) {
	    fGlyphs = fBaseGlyphs;
	} else {
	    fGlyphs = new UInt32[fNumGlyphs];
	}
    }
}

void
GlyphVector::allocateCharIndices() {
    if (fCharIndices == NULL) {
        if (fNumGlyphs <= eDefaultStorage) {
	    fCharIndices = fBaseCharIndices;
        } else {
	    fCharIndices = new UInt32[fNumGlyphs];
        }
    }
}

/**
 * Set the fPositions array to hold numGlyphs * 2 + 2 float values.
 */
void 
GlyphVector::allocatePositions(void)
{
    if (fPositions == NULL) {
	if (fNumGlyphs < eDefaultStorage) { // must be '<'
	    fPositions = fBasePositions;
	} else {
	    fPositions = new float[fNumGlyphs * 2 + 2];
	}
    }
}

/**
 * Set the fImageRefs array to hold numGlyphs ImageRefs.
 */
void
GlyphVector::allocateImageRefs(void)
{
    if (fImageRefs == NULL) {
	if (fNumGlyphs <= eDefaultStorage) {
	    fImageRefs = fBaseImageRefs;
	} else {
	    fImageRefs = new ImageRef[fNumGlyphs];
	}
    }
}

static inline int inRange(int c, int begin, int end) {
    return (c>=begin) && (c<=end);
}

void
GlyphVector::initCharIndices()
{
    allocateCharIndices();
    for (UInt32 i = 0; i < fNumGlyphs; ++i) {
        fCharIndices[i] = i;
    }
}

void 
GlyphVector::initGlyphs(const jchar *theChars, jint count)
{
    Strike &theStrike = getGlyphStrike(NULL);

    allocateGlyphs(count);
    if (fGlyphs) {
	fNeedShaping = charsToGlyphs(theStrike, theChars, fGlyphs, fNumGlyphs);
    }
}

Strike&
GlyphVector::getGlyphStrike(TX* devTX) const
{
    // !!! very dangerous currently
    // the font caches the strike and deletes the old one, so if you
    // call this twice in succession with different devTX args you will
    // delete one of the strikes.

    if (devTX) {
	TX glyphTX(*devTX);
	glyphTX.concat(fFontTX);
	// !!! adapter
	FontTransform ftx(glyphTX.m00, glyphTX.m10, glyphTX.m01, glyphTX.m11);
	return fFont->getStrike(ftx, fIsAntiAliased, fUsesFractionalMetrics);
    } else {
	// !!! adapter
	FontTransform ftx(fFontTX.m00, fFontTX.m10, fFontTX.m01, fFontTX.m11);
	return fFont->getStrike(ftx, fIsAntiAliased, fUsesFractionalMetrics);
    }
}

// ---------------

GlyphVector::GlyphVector(JNIEnv *env,
  jdoubleArray fontTX, jdoubleArray devTX, jboolean isAntiAliased, jboolean usesFractionalMetrics,
  fontObject *fo)
  : fFont(fo),  
    fEnv(env), 
    fNeedShaping(false),
    fNumGlyphs(0), 
    fGlyphs(NULL),
    fCharIndices(NULL),
    fPositions(NULL),
    fImageRefs(NULL),
    fTransforms(NULL),
    fTXIndices(NULL),
    fTXCount(0),
    fFontTX(env, fontTX),
    fDevTX(env, devTX),
    fIsAntiAliased(isAntiAliased),
    fUsesFractionalMetrics(usesFractionalMetrics)
{
}

GlyphVector::~GlyphVector()
{
    delete [] fTransforms; fTransforms = NULL;
    delete [] fTXIndices; fTXIndices = NULL;

    setNumGlyphs(0);
}

void GlyphVector::setString(jstring theString) 
{
    if (JNU_IsNull(fEnv, theString) ) {
        JNU_ThrowNullPointerException(fEnv, "theString is null");
        return;
    }

    jint len = fEnv->GetStringLength(theString);
    jchar *tempChars = new jchar[len];
    if (tempChars) {
	fEnv->GetStringRegion(theString, 0, len, tempChars);

	initGlyphs(tempChars, len);
	
	delete tempChars;
    } else {
	JNU_ThrowInternalError(fEnv, "could not copy string data");
    }
}

void GlyphVector::setText(jcharArray theCharArray, jint offset, jint count)
{
    if (JNU_IsNull(fEnv, theCharArray) ) {
        JNU_ThrowNullPointerException(fEnv, "theCharArray is null");
        return;
    }

    if (offset + count > fEnv->GetArrayLength(theCharArray)) {
        JNU_ThrowArrayIndexOutOfBoundsException(fEnv, "chars [offset + count]");
        return;
    }

    const jchar *theChars = (const jchar *)fEnv->GetPrimitiveArrayCritical(theCharArray, NULL);
    if (theChars) {
#ifdef __linux__
//	this code is needed as memory is not correctly allocated on linux 
        jchar *tempChars = new jchar[count];
        memcpy(tempChars, theChars + offset, sizeof(jchar) * count);
#else
        initGlyphs(theChars + offset, count);
#endif
        fEnv->ReleasePrimitiveArrayCritical(theCharArray, (void *)theChars, JNI_ABORT);
#ifdef __linux__
        initGlyphs(tempChars, count);
        delete tempChars;
#endif
    }
}

void GlyphVector::setText(jbyteArray theByteArray, jint offset, jint count)
{
    if (JNU_IsNull(fEnv, theByteArray) ) {
        JNU_ThrowNullPointerException(fEnv, "theByteArray is null");
        return;
    }

    if (offset + count > fEnv->GetArrayLength(theByteArray)) {
        JNU_ThrowArrayIndexOutOfBoundsException(fEnv, "bytes [offset + count]");
        return;
    }

    // skip shaping tests
    allocateGlyphs(count);
    if (fGlyphs) {
	Strike &theStrike = getGlyphStrike(NULL);
	const jbyte *theBytes = (const jbyte *)fEnv->GetPrimitiveArrayCritical(theByteArray, NULL);
	if (theBytes) {
	    const jbyte *p = theBytes + offset;
	    for (int i = 0; i < count; ++i) {
                fGlyphs[i] = theStrike.CharToGlyph((Unicode16)p[i]);
            }
        }
        fEnv->ReleasePrimitiveArrayCritical(theByteArray, (void *)theBytes, JNI_ABORT);
    }
}

void GlyphVector::setGlyphCodes(jintArray theGlyphArray, jint offset, jint count)
{
    if (JNU_IsNull(fEnv, theGlyphArray)) {
        JNU_ThrowNullPointerException(fEnv, "theGlyphArray is null");
        return;
    }

    if (offset + count > fEnv->GetArrayLength(theGlyphArray)) {
        JNU_ThrowArrayIndexOutOfBoundsException(fEnv, "glyphs [offset + count]");
        return;
    }

    allocateGlyphs(count);
    if (fGlyphs) {
	fEnv->GetIntArrayRegion(theGlyphArray, offset, count, (jint*)fGlyphs);
    }
}

void GlyphVector::setGlyphCodes(jintArray theGlyphArray) {
    if (JNU_IsNull(fEnv, theGlyphArray)) {
        JNU_ThrowNullPointerException(fEnv, "theGlyphArray is null");
        return;
    }

    jsize count = fEnv->GetArrayLength(theGlyphArray);

    allocateGlyphs(count);
    if (fGlyphs) {
	fEnv->GetIntArrayRegion(theGlyphArray, 0, count, (jint*)fGlyphs);
    }
}

void GlyphVector::setPositions(jfloatArray newPos)
{
    if (!JNU_IsNull(fEnv, newPos)) {
	allocatePositions();
	if (fPositions) {
	    UInt32 count = fEnv->GetArrayLength(newPos);
	    if (count == fNumGlyphs * 2 || count == fNumGlyphs * 2 + 2) {
		fEnv->GetFloatArrayRegion(newPos, 0, count, fPositions);
            } else {
                JNU_ThrowIllegalArgumentException(fEnv, "Wrong size positions array");
	    }
        }
    }
}

void GlyphVector::setTransforms(jdoubleArray jTransforms, jintArray jTXIndices) 
{
    if (!JNU_IsNull(fEnv, jTransforms) && !JNU_IsNull(fEnv, jTXIndices)) {
	UInt32 txCount = fEnv->GetArrayLength(jTransforms);
	UInt32 txIxCount = fEnv->GetArrayLength(jTXIndices);
	if (txIxCount != fNumGlyphs) {
	    JNU_ThrowIllegalArgumentException(fEnv, "Wrong size txIndices array");
	} else {
	    int err = 1; // assume memory failure
	    fTransforms = new jdouble[txCount];
	    fTXIndices = new jint[txIxCount];
	    fTXCount = txCount / 6; // six doubles per transform
	    if (fTransforms && fTXIndices) {
		fEnv->GetDoubleArrayRegion(jTransforms, 0, txCount, fTransforms);
		fEnv->GetIntArrayRegion(jTXIndices, 0, txIxCount, fTXIndices);

		// verify fTXIndices array
		err = 0; // assume success
		for (UInt32 i = 0; i < txIxCount; ++i) {
		    jint ix = fTXIndices[i];
		    if (ix < 0 || ix > fTXCount) { // indexes are 1-based, 0 means no transform
			err = 2; // index failure
			break;
		    }
		}
	    }   
	    if (err) {
		delete [] fTXIndices;
		fTXIndices = NULL;
		delete [] fTransforms;
		fTransforms = NULL;
		fTXCount = 0;
		if (err == 1) {
		    JNU_ThrowInternalError(fEnv, "Insufficient Memory");
		} else {
		    JNU_ThrowIllegalArgumentException(fEnv, "transform index out of range");
		}
	    }
	}
    }
}

/**
 * Initialize the raster image data for the glyphs in preparation for rendering.
 *
 * This is used with drawString, drawChars, and drawBytes, to compute the positions
 * of the characters 'on the fly.'
 *
 * x and y are the position of the glyphvector in the device coordinate system.
 *
 * Fetching images can flush other images from the image cache.  If this might have
 * happened then the glyph images pointers are revalidated, and set to null if the
 * glyph has been flushed.  An attempt is made to reserve enough space for the glyphs
 * beforehand to reduce the chances of this occurring.
 */
void GlyphVector::positionAndGetImages(jfloat x, jfloat y)
{
    if (!fImageRefs) {
        allocateImageRefs();
	if (!fImageRefs) {
	    JNU_ThrowInternalError(fEnv, "Insufficient Memory");
	    return;
	}
    }

    Strike& strike = getGlyphStrike(NULL);
    strike.prepareCache(fNumGlyphs);

    Boolean refcheck = false;

    x += .5; // prepare to round pixel locations, must use floor to restore when locations < 0
    y += .5;

    hsGGlyph glyphRef;
    hsFixedPoint2 advanceXY;
    for (UInt32 i=0; i < fNumGlyphs; i++) {
      int glyphID = fGlyphs[i];
      // even with invisible glyphs, we provide full metric information
      if (IS_INVISIBLE_GLYPH(glyphID) ||
	  !strike.getMetricsWithImage(glyphID, glyphRef, advanceXY, &refcheck)) {
        fImageRefs[i].pixels = 0;
        fImageRefs[i].rowBytes = 0;
        fImageRefs[i].width = 0;
        fImageRefs[i].height = 0;
        fImageRefs[i].x = (int)floor(x);
        fImageRefs[i].y = (int)floor(y);
      } else {
        fImageRefs[i].pixels = glyphRef.fImage;
        fImageRefs[i].rowBytes = glyphRef.fRowBytes;
        fImageRefs[i].width = glyphRef.fWidth;
        fImageRefs[i].height = glyphRef.fHeight;
/*
  This is being commented out because it is not the right thing to do
  - although CTE made this change/hack in Ladybird. This may cause
  some other alignment issues. Bug # 414220 is being reopened so that
  Swing can fix the problem in a better way.  I am leaving this in
  case this issue comes back up.

	// if the first glyph has a negative left side bearing, then move the
	// position of x further out by the lsb - this way there would be no clipping.
	// this is a very rare occurence - it happens with 11 point W.
	if ( i==0 && (hsFixedToFloat(glyphRef.fTopLeft.fX) < 0 )) {
	     x += -hsFixedToFloat(glyphRef.fTopLeft.fX);
	}
 */

        fImageRefs[i].x = (int)floor(x + hsFixedToFloat(glyphRef.fTopLeft.fX));
        fImageRefs[i].y = (int)floor(y + hsFixedToFloat(glyphRef.fTopLeft.fY));
	x += hsFixedToFloat(advanceXY.fX);
	y += hsFixedToFloat(advanceXY.fY);
      }
    }

    // cache may have flushed glyph we allocated, so revalidate cache
    if (refcheck) {
      for (UInt32 i=0; i < fNumGlyphs; i++) {
        if (!IS_INVISIBLE_GLYPH(fGlyphs[i])) {
	    fImageRefs[i].pixels = strike.refImage(fGlyphs[i]);
	}
      }
    }
}

/**
 * like positionAndGetImages except this is used with glyphVectors, which
 * might have per-glyph transforms, and may already have position information.
 */
void GlyphVector::getImages(jfloat x, jfloat y)
{
    if (!fImageRefs) {
        allocateImageRefs();
	if (!fImageRefs) {
	    JNU_ThrowInternalError(fEnv, "Insufficient Memory");
	    return;
	}
    }

    StrikeTable strikes(*this, &fDevTX);

    strikes.getStrike().prepareCache(fNumGlyphs);
    Boolean refcheck = false;

    x += .5; // prepare to round pixel locations, must use floor to restore when locations < 0
    y += .5;

    //    fprintf(stdout, "getimages x: %g y: %g p: %x\n", x, y, fPositions);

    hsGGlyph glyphRef;
    hsFixedPoint2 advanceXY;
    float px = 0;
    float py = 0;
    const float *pp = fPositions;
    for (UInt32 i=0; i < fNumGlyphs; i++) {
      int glyphID = fGlyphs[i];
      // even with invisible glyphs, we provide full metric information
      if (IS_INVISIBLE_GLYPH(glyphID) ||
	  !strikes.getStrikeMetricsWithImage(i, glyphID, glyphRef, advanceXY, &refcheck)) {
        fImageRefs[i].pixels = 0;
        fImageRefs[i].rowBytes = 0;
        fImageRefs[i].width = 0;
        fImageRefs[i].height = 0;
	if (pp) {
	    px = *pp++;
	    py = *pp++;
	    fDevTX.vectorTransform(px, py);
	}
        fImageRefs[i].x = (int)floor(x + px);
        fImageRefs[i].y = (int)floor(y + py);

      } else {
        fImageRefs[i].pixels = glyphRef.fImage;
        fImageRefs[i].rowBytes = glyphRef.fRowBytes;
        fImageRefs[i].width = glyphRef.fWidth;
        fImageRefs[i].height = glyphRef.fHeight;
	if (pp) {
	    px = *pp++;
	    py = *pp++;
	    fDevTX.vectorTransform(px, py);
	}
	//	fprintf(stdout, "[%d] gid: %d px: %g py: %g\n", i, glyphID, px, py);
        fImageRefs[i].x = (int)floor(x + px + hsFixedToFloat(glyphRef.fTopLeft.fX));
        fImageRefs[i].y = (int)floor(y + py + hsFixedToFloat(glyphRef.fTopLeft.fY));
	if (!pp) {
	    px += hsFixedToFloat(advanceXY.fX);
	    py += hsFixedToFloat(advanceXY.fY);
	}
      }
      //      fprintf(stdout, "[%d] gid: %d x: %d y: %d\n", i, glyphID, fImageRefs[i].x, fImageRefs[i].y);
    }

    // cache may have flushed glyph we allocated, so revalidate cache
    if (refcheck) {
      for (UInt32 i=0; i < fNumGlyphs; i++) {
        if (!IS_INVISIBLE_GLYPH(fGlyphs[i])) {
	    fImageRefs[i].pixels = strikes.getStrike(i).refImage(fGlyphs[i]);
	}
      }
    }
}

/**
 * This computes glyph positions and leaves them in the positions array.  The
 * positions are in the standard coordinate system.  The positioning is 'dumb'
 * positioning that just offsets each glyph by the advance of the previous
 * glyph.  Real positioning should be done by layout.
 */
void GlyphVector::positionGlyphs()
{
    if (!fPositions) {
        allocatePositions();
	if (!fPositions) {
	    JNU_ThrowInternalError(fEnv, "Insufficient Memory");
	    return;
	}
    }

    // clean up before error throw below just in case
    {
	StrikeTable strikes(*this, &fDevTX);

	hsGGlyph glyphRef;
	hsFixedPoint2 advanceXY;
	float *p = fPositions;
	jfloat x = 0;
	jfloat y = 0;
	for (UInt32 i=0; i < fNumGlyphs; i++) {
	    *p++ = x;
	    *p++ = y;

	    int glyphID = fGlyphs[i];
	    
	    if (!IS_INVISIBLE_GLYPH(glyphID) &&
		strikes.getStrikeMetrics(i, glyphID, glyphRef, advanceXY)) {

		x += hsFixedToFloat(advanceXY.fX);
		y += hsFixedToFloat(advanceXY.fY);
	    }
	}
	*p++ = x;
	*p++ = y;
    }

    if (!fDevTX.isIdentity()) {
	TX inv(fDevTX);
	if (!inv.invert()) {
	    JNU_ThrowInternalError(fEnv, "Could not invert devTX");
	} else {
	    inv.transform(fPositions, fNumGlyphs + 1);
	}
    }
}

void GlyphVector::getGlyphCodes(jobject gvSource)
{
    if (initGVIDs(fEnv, gvSource)) {
        jintArray glyphs = (jintArray)fEnv->GetObjectField(gvSource, g_gvGlyphs);
	setGlyphCodes(glyphs);
    }
}

void GlyphVector::getPositions(jobject gvSource)
{
    if (initGVIDs(fEnv, gvSource)) {
        jfloatArray positions = (jfloatArray)fEnv->GetObjectField(gvSource, g_gvPositions);
        setPositions(positions);
    }
}

void GlyphVector::getTransforms(jobject gvSource)
{ 
    if (initGVIDs(fEnv, gvSource)) {
	jdoubleArray transforms = (jdoubleArray)fEnv->GetObjectField(gvSource, g_gvTransforms);
	jintArray txIndices = (jintArray)fEnv->GetObjectField(gvSource, g_gvTXIndices);
	setTransforms(transforms, txIndices);
    }
}

void GlyphVector::getGlyphVector(jobject gvSource) {
    getGlyphCodes(gvSource);
    getPositions(gvSource);
    getTransforms(gvSource);
}

void GlyphVector::writeGlyphCodes(jobject gvTarget)
{
    if (initGVIDs(fEnv, gvTarget)) {
        jintArray glyphsArray = fEnv->NewIntArray(fNumGlyphs);
        if (glyphsArray != NULL) {
	    if (fGlyphs) {
		fEnv->SetIntArrayRegion(glyphsArray, 0, fNumGlyphs, (jint*)fGlyphs);
	    }
            fEnv->SetObjectField(gvTarget, g_gvGlyphs, glyphsArray);
        }
    }
}

void GlyphVector::writeCharIndices(jobject gvTarget) 
{
    if (initGVIDs(fEnv, gvTarget)) {
	if (fCharIndices) { // only write if not default
	    jintArray charIndicesArray = fEnv->NewIntArray(fNumGlyphs);
	    if (charIndicesArray != NULL) {
		fEnv->SetIntArrayRegion(charIndicesArray, 0, fNumGlyphs, (jint*)fCharIndices);
		fEnv->SetObjectField(gvTarget, g_gvCharIndices, charIndicesArray);
	    }
        }
    }
}

void GlyphVector::writePositions(jobject gvTarget)
{
    if (initGVIDs(fEnv, gvTarget)) {
	jsize len = fNumGlyphs * 2 + 2;
        jfloatArray positions = fEnv->NewFloatArray(len);
        if (positions != NULL) {
	    if (fPositions) {
		fEnv->SetFloatArrayRegion(positions, 0, len, fPositions);
	    }
            fEnv->SetObjectField(gvTarget, g_gvPositions, positions);
        }
    }
}

/**
 * Return a Rectangle2D.Float containing the visual bounds of the GlyphVector.
 * The visual bounds is the union of the bounding box of the outlines of all
 * the glyphs.
 */
jobject 
GlyphVector::getVisualBounds() 
{
  hsScalar l = hsFloatToScalar(10000); // arbitrary value larger than smallest expected
  hsScalar t = l;
  hsScalar r = hsFloatToScalar(-10000); // arbitrary value smaller than largest expected
  hsScalar b = r;

  StrikeTable strikes(*this, NULL); // don't include devTX

  const jfloat* p = fPositions;
  for (UInt32 i = 0; i < fNumGlyphs; ++i) {
      hsRect bounds;
      strikes.getStrikeOutlineBounds(i, fGlyphs[i], bounds);

      if (!bounds.IsEmpty()) {
	  hsScalar x = hsFloatToScalar(*p++);
	  hsScalar y = hsFloatToScalar(*p++);
	  hsScalar gl = x + bounds.fLeft;
	  hsScalar gt = y + bounds.fTop;
	  hsScalar gr = x + bounds.fRight;
	  hsScalar gb = y + bounds.fBottom;

	  if (l > gl) l = gl;
	  if (t > gt) t = gt;
	  if (r < gr) r = gr;
	  if (b < gb) b = gb;
      } else {
	  p += 2;
      }
  }

  return makeFloatRect(fEnv, 
		       (jfloat)hsScalarToFloat(l),
		       (jfloat)hsScalarToFloat(t),
		       (jfloat)hsScalarToFloat(r - l),
		       (jfloat)hsScalarToFloat(b - t));
}

/**
 * Return a Shape containing the logical bounds of the glyph.
 * This shape is a rectangle oriented along the advance vector of
 * the glyph, its width is the advance, and its height is the
 * ascent + descent + leading of the font, scaled through the
 * glyph transform.
 */
jobject 
GlyphVector::getGlyphLogicalBounds(jint glyphIndex) 
{
    StrikeTable strikes(*this, NULL); // don't include devTX

    hsGGlyph glyphRef;
    hsFixedPoint2 advXY;

    jfloat x = fPositions[glyphIndex * 2];
    jfloat y = fPositions[glyphIndex * 2 + 1];

    Strike &strike = strikes.getStrike(glyphIndex);

    // fprintf(stdout, "\n\ngglb index: %d pos x: %g y: %g", glyphIndex, x, y);

    if (fTransforms) {
	int tx = fTXIndices[glyphIndex];
	if (tx) {
	    tx = (tx - 1) * 6 + 4;
	    jfloat dx = fTransforms[tx];
	    jfloat dy = fTransforms[tx + 1];

	    // fprintf(stdout, " dx: %g dy: %g", dx, dy);

	    x += dx;
	    y += dy;
	}
    }
    // fprintf(stdout, "\n");

    strike.getMetrics(fGlyphs[glyphIndex], glyphRef, advXY);

    hsFixedPoint2 ascent;
    hsFixedPoint2 descent;
    hsFixedPoint2 baseline;
    hsFixedPoint2 leading;
    hsFixedPoint2 maxAdvance;
    strike.GetLineHeight(ascent, descent, baseline, leading, maxAdvance);

    jfloat widX = hsFixedToFloat(advXY.fX);
    jfloat widY = hsFixedToFloat(advXY.fY);
    jfloat htX = hsFixedToFloat(-ascent.fX + descent.fX + leading.fX);
    jfloat htY = hsFixedToFloat(-ascent.fY + descent.fY + leading.fY);
    x += hsFixedToFloat(ascent.fX);
    y += hsFixedToFloat(ascent.fY);

    // fprintf(stdout, "   widx: %g widy: %g", widX, widY);
    // fprintf(stdout, " ascx: %g ascy: %g desx: %g desy: %g\n", hsFixedToFloat(ascent.fX), hsFixedToFloat(ascent.fY),
    //	  hsFixedToFloat(descent.fX), hsFixedToFloat(descent.fY));

    GeneralPath gp;
    gp.moveTo(x, y);
    gp.lineTo(x + widX, y + widY);
    gp.lineTo(x + widX + htX, y + widY + htY);
    gp.lineTo(x + htX, y + htY);
    gp.closePath();

    return gp.getShape(fEnv);
}

/**
 * Return a Rectangle2D.Float containing the visual bounds of the glyph.
 * The visual bounds is the bounding box of the outline of the glyph.
 */
jobject 
GlyphVector::getGlyphVisualBounds(jint glyphIndex) 
{
  hsScalar l = hsFloatToScalar(fPositions[glyphIndex * 2]);
  hsScalar t = hsFloatToScalar(fPositions[glyphIndex * 2 + 1]);
  hsScalar r = l;
  hsScalar b = t;

  StrikeTable strikes(*this, NULL); // don't include devTX

  hsRect bounds;
  strikes.getStrikeOutlineBounds(glyphIndex, fGlyphs[glyphIndex], bounds);

  if (!bounds.IsEmpty()) {
      l += bounds.fLeft;
      t += bounds.fTop;
      r += bounds.fRight;
      b += bounds.fBottom;
  }

  return makeFloatRect(fEnv, 
		       (jfloat)hsScalarToFloat(l),
		       (jfloat)hsScalarToFloat(t),
		       (jfloat)hsScalarToFloat(r - l),
		       (jfloat)hsScalarToFloat(b - t));
}

/**
 * Return a Rectangle2D.Float containing the pixel bounds of the glyphvector
 * when rendered at position x, y in user space.
 */
jobject 
GlyphVector::getPixelBounds(jfloat x, jfloat y) 
{
  fDevTX.transform(x, y);

  getImages(x, y);

  int l = 0x7fffffff;
  int t = l;
  int r = 0x80000000;
  int b = r;

  for (UInt32 i = 0; i < fNumGlyphs; ++i) {
      int gl = fImageRefs[i].x;
      int gt = fImageRefs[i].y;
      int gr = gl + fImageRefs[i].width;
      int gb = gt + fImageRefs[i].height;

      if (l > gl) l = gl;
      if (t > gt) t = gt;
      if (r < gr) r = gr;
      if (b < gb) b = gb;
  }

  return makeRect(fEnv, l, t, r - l, b - t);
}

/**
 * Return a Rectangle containing the pixel bounds of the glyph when
 * the glyphvector is rendered at x, y in user space.
 */
jobject 
GlyphVector::getGlyphPixelBounds(jint glyphIndex, jfloat x, jfloat y) 
{
  fDevTX.transform(x, y);

  getImages(x, y);

  return makeRect(fEnv, 
		  fImageRefs[glyphIndex].x,
		  fImageRefs[glyphIndex].y,
		  fImageRefs[glyphIndex].width,
		  fImageRefs[glyphIndex].height);
}

// a utility function that converts hs paths to GeneralPath format
void GlyphVector::scanGlyph(
    GeneralPath& gp,
    Strike& theStrike,
    hsPathSpline& path,
    CurveWalker& walker,
    float xPos, float yPos,
    jint glyph)
{
    if (IS_INVISIBLE_GLYPH(glyph)) {
      return;
    }

    path.Init(&path);
    theStrike.GetGlyphOutline(glyph, path);

    jboolean isQuad = theStrike.GetPathType(glyph) == hsPathSpline::kQuadType;

    for (unsigned int j = 0; j < path.fContourCount; j++) {
        Boolean firstTime = true;
        if (isQuad) {
            walker.Init(&path.fContours[j], true);
            while (walker.NextQuad()) {
                if (firstTime) {
		    gp.moveTo(hsScalarToFloat(walker.a.fX) + xPos,
			      hsScalarToFloat(walker.a.fY) + yPos);
                    firstTime = false;
                }
                if (walker.drawLine) {
                    gp.lineTo(hsScalarToFloat(walker.b.fX) + xPos,
			      hsScalarToFloat(walker.b.fY) + yPos);
                } else {
		    gp.quadTo(hsScalarToFloat(walker.b.fX) + xPos,
			      hsScalarToFloat(walker.b.fY) + yPos,
			      hsScalarToFloat(walker.c.fX) + xPos,
			      hsScalarToFloat(walker.c.fY) + yPos);
                }
            }
	    gp.closePath();
        } else {        // is kCubicType 
            walker.Init(&path.fContours[j], false);
            while (walker.NextCubic ()) {
                if (firstTime) {
		    gp.moveTo(hsScalarToFloat(walker.a.fX) + xPos,
			      hsScalarToFloat(walker.a.fY) + yPos);
		    firstTime = false;
                }
                if (walker.drawLine) {
		    gp.lineTo(hsScalarToFloat(walker.b.fX) + xPos,
			      hsScalarToFloat(walker.b.fY) + yPos);
                } else {
		    gp.curveTo(hsScalarToFloat(walker.b.fX) + xPos,
			       hsScalarToFloat(walker.b.fY) + yPos,
			       hsScalarToFloat(walker.c.fX) + xPos,
			       hsScalarToFloat(walker.c.fY) + yPos,
			       hsScalarToFloat(walker.d.fX) + xPos,
			       hsScalarToFloat(walker.d.fY) + yPos);
                }
            }
	    gp.closePath();
        }
    }
    path.Delete(&path);
}

/**
 * Return the outline of the glyphs from start to count.  
 * X and Y are the position of the glyphvector.
 *
 * X and Y are in logical (not device) coordinates.  The GlyphVector must have
 * positions in logical coordinates.
 */
jobject GlyphVector::getGlyphsOutline(jint start, jint count, jfloat x, jfloat y)
{
    if (fGlyphs && fPositions) {

	StrikeTable strikes(*this);
	
	GeneralPath gp;

	hsPathSpline path;
	CurveWalker walker;
	for (unsigned int i=start, e = start + count; i < e; i++) {
	    float xPos = fPositions[i*2];
	    float yPos = fPositions[i*2+1];

	    if (fTransforms) {
		int ix = fTXIndices[i];
		if (ix) {
		    ix = (ix - 1) * 6 + 4;
		    xPos += fTransforms[ix];
		    yPos += fTransforms[ix + 1];
		}
	    }

	    scanGlyph(gp, strikes.getStrike(i), path, walker, x + xPos, y + yPos, fGlyphs[i]);
	}

	return gp.getShape(fEnv);
    } else {
	JNU_ThrowInternalError(fEnv, "can't get outline of uninitialized glyphvector");
    }
    return NULL;
}

/**
 * Return the outline of the entire glyphvector.
 */
jobject GlyphVector::getOutline(jfloat x, jfloat y)
{
    return getGlyphsOutline(0, fNumGlyphs, x, y);
}

/**
 * Return the outline of the glyph.
 */
jobject GlyphVector::getGlyphOutline(jint glyphIndex, jfloat x, jfloat y)
{
    return getGlyphsOutline(glyphIndex, 1, x, y);
}

/**
 * Return the glyph info array x y ax ay vx vy vw vh 1 per glyph
 */
jfloatArray GlyphVector::getGlyphInfo()
{
    int resultlen = fNumGlyphs * 8;
    jfloat buffer[100 * 8];
    jfloat *bufp = buffer;
    if (fNumGlyphs > 100) {
	bufp = new jfloat[resultlen];
	if (!bufp) {
	    return NULL;
	}
    }

    StrikeTable strikes(*this, &fDevTX);
    StrikeTable* pfstrikes = &strikes; // assume identity devTX

    TX inv(fDevTX);
    Boolean needInv = !fDevTX.isIdentity();
    if (needInv) {
	inv.invert(); // ignore error
    }

    hsGGlyph glyphRef;
    hsFixedPoint2 advanceXY;
    hsRect bounds;

    jfloat *p = bufp;
    jfloat *pp = fPositions;
    jfloat x = 0;
    jfloat y = 0;
    UInt32 i;
    for (i=0; i < fNumGlyphs; i++) {
	int glyphID = fGlyphs[i];

	if (!IS_INVISIBLE_GLYPH(glyphID) &&
	    strikes.getStrikeMetrics(i, glyphID, glyphRef, advanceXY)) {
	    p[2] = hsFixedToFloat(advanceXY.fX);
	    p[3] = hsFixedToFloat(advanceXY.fY);
	} else {
	    p[2] = 0;
	    p[3] = 0;
	}

	if (pp) {
	    p[0] = *pp++;
	    p[1] = *pp++;
	} else {
	    p[0] = x;
	    p[1] = y;

	    x += p[2];
	    y += p[3];

	    if (needInv) {
		inv.transform(p[0], p[1]);
	    }
	}
	if (needInv) {
	    inv.vectorTransform(p[2], p[3]);
	}

	p += 8;
    }

    // !!! calls getGlyphStrike, which can lose a cached strike
    // so we do this in a separate pass after we're done with the first
    // strike
    // yecch! need to fix strike caching stuff

    if (needInv) {
	pfstrikes = new StrikeTable(*this, NULL);
    }
    p = bufp;
    for (i = 0; i < fNumGlyphs; ++i) {
	int glyphID = fGlyphs[i];

	pfstrikes->getStrikeOutlineBounds(i, glyphID, bounds);
	p[4] = p[0] + (jfloat)hsScalarToFloat(bounds.fLeft);
        p[5] = p[1] + (jfloat)hsScalarToFloat(bounds.fTop);
        p[6] = (jfloat)hsScalarToFloat(bounds.fRight - bounds.fLeft);
        p[7] = (jfloat)hsScalarToFloat(bounds.fBottom - bounds.fTop);

	p += 8;
    }

    if (pfstrikes != &strikes) {
	delete pfstrikes;
    }

    jfloatArray result = fEnv->NewFloatArray(resultlen);
    if (result != NULL) {
        fEnv->SetFloatArrayRegion(result, 0, resultlen, bufp);
    }

    if (bufp != buffer) {
	delete [] bufp;
    }

    return result;
}

jboolean GlyphVector::charsToGlyphs(Strike& theStrike, const jchar* theChars, UInt32* theGlyphs, jint count) 
{
    jboolean needShaping = JNI_FALSE;

    DefaultCharMapper mapper(true, false, true);
    for (jint i=0; i<count; i++) {
	Unicode32 theChar = theChars[i];

	if (theChar == 0xffff) {
	    theGlyphs[i] = INVISIBLE_GLYPH_ID;
	    continue;
	} else {
	    if (i < count - 1 && inRange(theChar, 0xD800, 0xDBFF)) {
		Unicode16 high = theChars[i];
		Unicode16 low  = theChars[i + 1];
		    
		if (inRange(low, 0xDC00, 0xDFFF)) {
		    theChar = (high - 0xD800) * 0x400 + low - 0xDC00 + 0x10000;
		}
	    }
	    jchar mappedChar = mapper.mapChar(theChar);
	    theGlyphs[i] = (mappedChar == (jchar)0xffff) ? INVISIBLE_GLYPH_ID : theStrike.CharToGlyph(theChar);
	    //theGlyphs[i] = theStrike.CharToGlyph(theChar);
	}

	if (theChar >= 0x10000) {
	    i += 1;
	    theGlyphs[i] = INVISIBLE_GLYPH_ID;
	}

	if (theChar < 0x0590 || needShaping) {
	    // if roman assume no shaping, BIDI, or if already shaping no need to recheck
	    continue;
	}
	if (inRange(theChar, 0x0590, 0x05ff)) {
	    // if Hebrew, assume shaping for presentation forms
	    needShaping = true;
	    continue;
	}
	if (inRange(theChar, 0x0600, 0x06ff)) {
	    // if Arabic, assume shaping for presentation forms
	    needShaping = true;
	    continue;
	}
	if (inRange(theChar, 0x0900, 0x0D7f)) {
	    // if Indic, assume shaping for conjuncts, reordering:
	    // 0900 - 097F Devanagari
	    // 0980 - 09FF Bengali
	    // 0A00 - 0A7F Gurmukhi
	    // 0A80 - 0AFF Gujarati
	    // 0B00 - 0B7F Oriya
	    // 0B80 - 0BFF Tamil
	    // 0C00 - 0C7F Telugu
	    // 0C80 - 0CFF Kannada
	    // 0D00 - 0D7F Malayalam
	    needShaping = true;
	    continue;
	}
	if (inRange(theChar, 0x0E00, 0x0E7f)) {
	    // if Thai, assume shaping for vowel, tone marks
	    needShaping = true;
	    continue;
	}
	if (inRange(theChar, 0x200c, 0x200d)) {
	    // if zwj or zwnj, assume shaping needed
	    needShaping = true;
	    continue;
	}
	if (inRange(theChar, 0x202a, 0x202e)) {
	    // if directional control, assume shaping needed
	    needShaping = true;
	    continue;
	}
	if (inRange(theChar, 0x206a, 0x206f)) {
	    // if shaping control, assume shaping needed
	    needShaping = true;
	    continue;
	}
    }

    return needShaping;
}

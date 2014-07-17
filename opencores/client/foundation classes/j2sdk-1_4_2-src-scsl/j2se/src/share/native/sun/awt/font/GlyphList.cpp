/*
 * @(#)GlyphList.cpp	1.13 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "sun_awt_font_GlyphList.h"
#include "GlyphVector.h"

#include "jni_util.h"
#include "gdefs.h"

extern "C" {

static const char *ShapingExceptionClassName = "sun/awt/font/ShapingException";

static jclass pShapingExceptionClass;
static jfieldID pDataID;

#define GetGlyphVector(env,obj) \
    ((GlyphVector *)(uintptr_t)(env)->GetLongField((obj),pDataID))
#define SetGlyphVector(env,obj,val) \
    (env)->SetLongField((obj),pDataID,(jlong)(uintptr_t)(val))

/*
 * Class:     sun_awt_font_GlyphList
 * Method:    initIDs
 * Signature: ()V
 */
JNIEXPORT void JNICALL
Java_sun_awt_font_GlyphList_initIDs(JNIEnv *env, jclass gl)
{
    pShapingExceptionClass = env->FindClass(ShapingExceptionClassName);
    if (pShapingExceptionClass == 0) {
	JNU_ThrowClassNotFoundException(env, ShapingExceptionClassName);
	return;
    }
    pShapingExceptionClass = (jclass)env->NewGlobalRef(pShapingExceptionClass);
    if (!pShapingExceptionClass) {
	JNU_ThrowInternalError(env, "could not create global ref");
	return;
    }
    pDataID = env->GetFieldID(gl, "pData", "J");
    if (pDataID == 0) {
	JNU_ThrowNoSuchFieldError(env, "GlyphList.pData");
    }
}

static void
throwShapingException(JNIEnv *env)
{
    env->ThrowNew(pShapingExceptionClass, "Shaping may be needed");
}

/*
 * Class:     sun_awt_font_GlyphList
 * Method:    setupStringData
 * Signature: (Ljava/lang/String;Ljava/awt/Font;FF[D[DZZ)V
 */
JNIEXPORT void JNICALL
Java_sun_awt_font_GlyphList_setupStringData
    (JNIEnv *env, jobject gList,
     jstring string,
     jobject font, jfloat x, jfloat y,
     jdoubleArray renderMat, jboolean isAA, jboolean useFract)
{
    if (GetGlyphVector(env, gList) != NULL) {
	JNU_ThrowInternalError(env, "GlyphList object in use");
	return;
    }
    fontObject *fo = ::getFontPtr(env, font);
    if (fo) {
	GlyphVector *gv = new GlyphVector(env, renderMat, NULL, isAA, useFract, fo);
	gv->setString(string);
	if (gv->needShaping()) {
	    delete gv;
	    throwShapingException(env);
	    return;
	}
	gv->positionAndGetImages(x, y);
	SetGlyphVector(env, gList, gv);
    }
}

/*
 * Class:     sun_awt_font_GlyphList
 * Method:    setupCharData
 * Signature: ([CIILjava/awt/Font;FF[D[DZZ)V
 */
JNIEXPORT void JNICALL
Java_sun_awt_font_GlyphList_setupCharData
    (JNIEnv *env, jobject gList,
     jcharArray chars, jint offset, jint count,
     jobject font, jfloat x, jfloat y,
     jdoubleArray renderMat, jboolean isAA, jboolean useFract)
{
    if (GetGlyphVector(env, gList) != NULL) {
	JNU_ThrowInternalError(env, "GlyphList object in use");
	return;
    }
    fontObject *fo = ::getFontPtr(env, font);
    if (fo) {
	GlyphVector *gv = new GlyphVector(env, renderMat, NULL, isAA, useFract, fo);
        gv->setText(chars, offset, count);
	if (gv->needShaping()) {
	    delete gv;
	    throwShapingException(env);
	    return;
	}
	gv->positionAndGetImages(x, y);
	SetGlyphVector(env, gList, gv);
    }
}

/*
 * Class:     sun_awt_font_GlyphList
 * Method:    setupByteData
 * Signature: ([BIILjava/awt/Font;FF[D[DZZ)V
 */
JNIEXPORT void JNICALL
Java_sun_awt_font_GlyphList_setupByteData
    (JNIEnv *env, jobject gList,
     jbyteArray bytes, jint offset, jint count,
     jobject font, jfloat x, jfloat y,
     jdoubleArray renderMat, jboolean isAA, jboolean useFract)
{
    if (GetGlyphVector(env, gList) != NULL) {
	JNU_ThrowInternalError(env, "GlyphList object in use");
	return;
    }
    fontObject *fo = ::getFontPtr(env, font);
    if (fo) {
	GlyphVector *gv = new GlyphVector(env, renderMat, NULL, isAA, useFract, fo);
	gv->setText(bytes, offset, count);
	gv->positionAndGetImages(x, y);
	SetGlyphVector(env, gList, gv);
    }
}

/*
 * Class:     sun_awt_font_GlyphList
 * Method:    setupGlyphData
 * Signature: ([I[FLjava/awt/Font;FF[D[DZZ)V
 */
JNIEXPORT void JNICALL
Java_sun_awt_font_GlyphList_setupGlyphData
    (JNIEnv *env, jobject gList,
     jobject glyphVector,
     jobject font, jfloat x, jfloat y, jdoubleArray fontTX,
     jdoubleArray devTX, jboolean isAA, jboolean useFract)
{
    if (GetGlyphVector(env, gList) != NULL) {
	JNU_ThrowInternalError(env, "GlyphList object in use");
	return;
    }
    fontObject *fo = ::getFontPtr(env, font);
    if (fo) {
	GlyphVector *gv = new GlyphVector(env, fontTX, devTX, isAA, useFract, fo);
	gv->getGlyphVector(glyphVector);
	gv->getImages(x, y);
	SetGlyphVector(env, gList, gv);
    }
}

/*
 * Class:     sun_awt_font_GlyphList
 * Method:    getNumGlyphs
 * Signature: ()I
 */
JNIEXPORT jint JNICALL
Java_sun_awt_font_GlyphList_getNumGlyphs
    (JNIEnv *env, jobject gList)
{
    GlyphVector *gv = GetGlyphVector(env, gList);
    if (gv == NULL) {
	JNU_ThrowNullPointerException(env, "GlyphList.pData");
	return 0;
    }
    return gv->getNumGlyphs();
}

/*
 * Class:     sun_awt_font_GlyphList
 * Method:    fillBounds
 * Signature: ([I)V
 */
JNIEXPORT void JNICALL
Java_sun_awt_font_GlyphList_fillBounds
    (JNIEnv *env, jobject gList, jintArray boundsArray)
{
    jint bounds[4];
    const void *pixelPtr;
    unsigned int rowBytes;
    int x, y, w, h;

    GlyphVector *gv = GetGlyphVector(env, gList);
    if (gv == NULL) {
	JNU_ThrowNullPointerException(env, "GlyphList.pData");
	return;
    }
    int num = gv->getNumGlyphs();
    if (num == 0) {
	bounds[0] = bounds[1] = bounds[2] = bounds[3] = 0;
    } else {
	bounds[0] = bounds[1] = 0x7fffffff;
	bounds[2] = bounds[3] = 0x80000000;

	for (int index = 0; index < num; index++) {
	    gv->getGlyphBlitInfo(index, pixelPtr, rowBytes, x, y, w, h);
	    if (pixelPtr) {
		jint dx0 = (jint) x;
		jint dy0 = (jint) y;
		jint dx1 = dx0 + w;
		jint dy1 = dy0 + h;
		if (bounds[0] > dx0) bounds[0] = dx0;
		if (bounds[1] > dy0) bounds[1] = dy0;
	    if (bounds[2] < dx1) bounds[2] = dx1;
	    if (bounds[3] < dy1) bounds[3] = dy1;
	    }
	}
    }
    env->SetIntArrayRegion(boundsArray, 0, 4, bounds);
}

/*
 * Class:     sun_awt_font_GlyphList
 * Method:    fillMetrics
 * Signature: (I[I)V
 */
JNIEXPORT void JNICALL
Java_sun_awt_font_GlyphList_fillMetrics
    (JNIEnv *env, jobject gList, jint index, jintArray metricsArray)
{
    jint metrics[4];
    const void *pixelPtr;
    unsigned int rowBytes;
    int x, y, w, h;

    GlyphVector *gv = GetGlyphVector(env, gList);
    if (gv == NULL) {
	JNU_ThrowNullPointerException(env, "GlyphList.pData");
	return;
    }
    if ((unsigned int)index > gv->getNumGlyphs()) {
	JNU_ThrowArrayIndexOutOfBoundsException(env, "glyph index");
	return;
    }

    gv->getGlyphBlitInfo(index, pixelPtr, rowBytes, x, y, w, h);
    if (pixelPtr) {
	metrics[0] = (jint) x;
	metrics[1] = (jint) y;
	metrics[2] = (jint) w;
	metrics[3] = (jint) h;
    } else {
	metrics[0] = metrics[1] = metrics[2] = metrics[3] = 0;
    }
    env->SetIntArrayRegion(metricsArray, 0, 4, metrics);
}

/*
 * Class:     sun_awt_font_GlyphList
 * Method:    fillGrayBits
 * Signature: (I[B)V
 */
JNIEXPORT void JNICALL
Java_sun_awt_font_GlyphList_fillGrayBits
    (JNIEnv *env, jobject gList, jint index, jbyteArray gbArray)
{
    const void *pixelPtr;
    unsigned int rowBytes;
    int x, y, w, h;

    GlyphVector *gv = GetGlyphVector(env, gList);
    if (gv == NULL) {
	JNU_ThrowNullPointerException(env, "GlyphList.pData");
	return;
    }
    if ((unsigned int)index > gv->getNumGlyphs()) {
	JNU_ThrowArrayIndexOutOfBoundsException(env, "glyph index");
	return;
    }

    // REMIND: Need to revalidate the glyph data in case it was flushed
    // from the cache...
    gv->getGlyphBlitInfo(index, pixelPtr, rowBytes, x, y, w, h);
    if (pixelPtr) {
	int len = w * h;
	int datalen = env->GetArrayLength(gbArray);
	if (datalen < len) {
	    JNU_ThrowArrayIndexOutOfBoundsException(env, "graybits byte array");
	    return;
	}
	jbyte *gbase = (jbyte *) env->GetPrimitiveArrayCritical(gbArray, NULL);
	if (rowBytes == (unsigned int)w) {
	    memcpy(gbase, pixelPtr, len);
	} else {
	    jbyte *gb = gbase;
	    for (int i = 0; i < h; i++) {
		memcpy(gb, pixelPtr, w);
		gb += w;
		pixelPtr = (const void *) (((intptr_t) pixelPtr) + rowBytes);
	    }
	}
	env->ReleasePrimitiveArrayCritical(gbArray, gbase, 0);
    }
}

/*
 * Class:     sun_awt_font_GlyphList
 * Method:    discardData
 * Signature: ()V
 */
JNIEXPORT void JNICALL
Java_sun_awt_font_GlyphList_discardData
    (JNIEnv *env, jobject gList)
{
    GlyphVector *gv = GetGlyphVector(env, gList);
    if (gv != NULL) {
	delete gv;
	SetGlyphVector(env, gList, 0);
    }
}

} // end of extern "C"

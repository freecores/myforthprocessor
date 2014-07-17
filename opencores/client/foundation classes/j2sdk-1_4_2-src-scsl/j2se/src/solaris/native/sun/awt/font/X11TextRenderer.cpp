/*
 * @(#)X11TextRenderer.cpp	1.7 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * Important note : All AWTxxx functions are defined in font.h.
 * These were added to remove the dependency of this file on X11.
 * These functions are used to perform X11 operations and should
 * be "stubbed out" in environments that do not support X11.
 * The implementation of these functions has been moved from this file
 * into X11TextRenderer_md.c, which is compiled into another library.
 */

#include "sun_awt_font_X11TextRenderer.h"

extern "C" {

#include "SurfaceData.h"
#include "GraphicsPrimitiveMgr.h"

}

#include "GlyphVector.h"

extern "C" {

extern jint RefineBounds(GlyphVector &gv, SurfaceDataBounds *bounds);
JNIEXPORT void JNICALL AWTDrawGlyphList(JNIEnv *env, jobject xtr,
     jobject sData, jobject clip, jint pixel,
     jobject glist, jlong glpeer, SurfaceDataBounds* bounds,
     struct ImageRef *glyphs, jint totalGlyphs);

}

/*
 * Class:     sun_awt_font_X11TextRenderer
 * Method:    doDrawGlyphList
 * Signature: (Lsun/java2d/SurfaceData;Ljava/awt/Rectangle;ILsun/awt/font/GlyphList;J)V
 */
JNIEXPORT void JNICALL Java_sun_awt_font_X11TextRenderer_doDrawGlyphList
    (JNIEnv *env, jobject xtr,
     jobject sData, jobject clip, jint pixel,
     jobject glist, jlong glpeer)
{
    SurfaceDataBounds bounds;
    SurfaceData_GetBoundsFromRegion(env, clip, &bounds);

    GlyphVector &gv = *(GlyphVector*)jlong_to_ptr(glpeer);
    if (!RefineBounds(gv, &bounds)) {
        return;
    }
    AWTDrawGlyphList(env, xtr, sData, clip, pixel, glist, glpeer,
		     &bounds, gv.getGlyphImages(), gv.getNumGlyphs());
}

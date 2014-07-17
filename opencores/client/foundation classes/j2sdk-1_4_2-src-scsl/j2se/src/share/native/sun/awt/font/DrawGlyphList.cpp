/*
 * @(#)DrawGlyphList.cpp	1.7 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */



#include "GlyphVector.h"

extern "C" {
#include "GraphicsPrimitiveMgr.h"
#include "sun_java2d_loops_DrawGlyphList.h"
#include "sun_java2d_loops_DrawGlyphListAA.h"


jint RefineBounds(GlyphVector &gv, SurfaceDataBounds *bounds) {
    int num = gv.getNumGlyphs();
    SurfaceDataBounds glyphs;
    const void *pixelPtr;
    unsigned int rowBytes;
    int x, y, w, h;

    glyphs.x1 = glyphs.y1 = 0x7fffffff;
    glyphs.x2 = glyphs.y2 = 0x80000000;
    for (int index = 0; index < num; index++) {
	gv.getGlyphBlitInfo(index, pixelPtr, rowBytes, x, y, w, h);
	jint dx1 = (jint) x;
	jint dy1 = (jint) y;
	jint dx2 = dx1 + w;
	jint dy2 = dy1 + h;
	if (glyphs.x1 > dx1) glyphs.x1 = dx1;
	if (glyphs.y1 > dy1) glyphs.y1 = dy1;
	if (glyphs.x2 < dx2) glyphs.x2 = dx2;
	if (glyphs.y2 < dy2) glyphs.y2 = dy2;
    }

    SurfaceData_IntersectBounds(bounds, &glyphs);
    return (bounds->x1 < bounds->x2 && bounds->y1 < bounds->y2);
}




    /* since the AA and non-AA loop functions share a common method
     * signature, can call both through this common function since
     * there's no difference except for the inner loop.
     * This could be a macro but there's enough of those already.
     */
static void drawGlyphList(JNIEnv *env, jobject self,
			  jobject sg2d, jobject sData,
			  jlong gvptr, jint pixel, jint color,
			  NativePrimitive *pPrim, DrawGlyphListFunc *func) {

    SurfaceDataOps *sdOps;
    SurfaceDataRasInfo rasInfo;
    CompositeInfo compInfo;

    sdOps = SurfaceData_GetOps(env, sData);
    if (sdOps == 0) {
	return;
    }

    if (pPrim->pCompType->getCompInfo != NULL) {
        GrPrim_Sg2dGetCompInfo(env, sg2d, pPrim, &compInfo);
    }

    GrPrim_Sg2dGetClip(env, sg2d, &rasInfo.bounds);
    if (rasInfo.bounds.y2 <= rasInfo.bounds.y1 ||
	rasInfo.bounds.x2 <= rasInfo.bounds.x1)
    {
	return;
    }

    GlyphVector &gv = *(GlyphVector*)jlong_to_ptr(gvptr);

    int ret = sdOps->Lock(env, sdOps, &rasInfo, pPrim->dstflags);
    if (ret != SD_SUCCESS) { 
	if (ret == SD_SLOWLOCK) {
	    if (!RefineBounds(gv, &rasInfo.bounds)) {
		SurfaceData_InvokeUnlock(env, sdOps, &rasInfo);
		return;
	    }
	} else {
	    return;
	}
    }

    sdOps->GetRasInfo(env, sdOps, &rasInfo);
    if (!rasInfo.rasBase) { 
	SurfaceData_InvokeUnlock(env, sdOps, &rasInfo);
	return;
    }


    int clipLeft    = rasInfo.bounds.x1;
    int clipRight   = rasInfo.bounds.x2;
    int clipTop     = rasInfo.bounds.y1;
    int clipBottom  = rasInfo.bounds.y2;
    if (clipRight > clipLeft &&	clipBottom > clipTop) {

	(*func)(&rasInfo,
		gv.getGlyphImages(), gv.getNumGlyphs(),
                pixel, color,
		clipLeft, clipTop,
		clipRight, clipBottom,
                pPrim, &compInfo);              
	SurfaceData_InvokeRelease(env, sdOps, &rasInfo);

    }
    SurfaceData_InvokeUnlock(env, sdOps, &rasInfo);
}

/*
 * Class:     sun_java2d_loops_DrawGlyphList
 * Method:    DrawGlyphList
 * Signature: (Lsun/java2d/SunGraphics2D;Lsun/java2d/SurfaceData;Lsun/java2d/font/GlyphList;J)V
 */
JNIEXPORT void JNICALL
Java_sun_java2d_loops_DrawGlyphList_DrawGlyphList
    (JNIEnv *env, jobject self,
     jobject sg2d, jobject sData,
     jobject glyphlist,
     jlong gvptr)
{
    NativePrimitive *pPrim = pPrim = GetNativePrim(env, self);
    if (pPrim == NULL) {
	return;
    }

    jint pixel = GrPrim_Sg2dGetPixel(env, sg2d);
    jint color = GrPrim_Sg2dGetRGB(env, sg2d);
    drawGlyphList(env, self, sg2d, sData, gvptr, pixel, color,
		  pPrim, pPrim->funcs.drawglyphlist);
}

/*
 * Class:     sun_java2d_loops_DrawGlyphListAA
 * Method:    DrawGlyphListAA
 * Signature: (Lsun/java2d/SunGraphics2D;Lsun/java2d/SurfaceData;Lsun/java2d/font/GlyphList;J)V
 */
JNIEXPORT void JNICALL
Java_sun_java2d_loops_DrawGlyphListAA_DrawGlyphListAA
    (JNIEnv *env, jobject self,
     jobject sg2d, jobject sData,
     jobject glyphlist,
     jlong gvptr)
{
    NativePrimitive *pPrim = pPrim = GetNativePrim(env, self);
    if (pPrim == NULL) {
	return;
    }
    jint pixel = GrPrim_Sg2dGetPixel(env, sg2d);
    jint color = GrPrim_Sg2dGetRGB(env, sg2d);
    drawGlyphList(env, self, sg2d, sData, gvptr, pixel, color,
		  pPrim, pPrim->funcs.drawglyphlistaa);
}

} /* end extern "C" */

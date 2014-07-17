/*
 * @(#)ScaledBlit.c	1.5 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "jni_util.h"
#include "GraphicsPrimitiveMgr.h"

#include "sun_java2d_loops_ScaledBlit.h"

static void
Scale_adjustUp(jint srcPos, jint srcLimit,
	       jint *pSrcLoc, jint srcInc, jint shift,
	       jint *pDstPos)
{
    jint srcLoc = *pSrcLoc;
    jint nsteps;

    if (srcPos >= srcLimit) return;
    /*
     * Overflow checking:
     * (srcLimit - srcPos) must be less than srcw
     * (srcw << shift) does not overflow so
     * ((srcLimit - srcPos) << shift) must not overflow either
     * From the calculation of srcLoc we have (0 <= srcLoc < srcInc)
     * and srcInc is less than "shift" bits wide
     * therefore (- srcLoc + srcInc - 1) must be less than "shift"
     * bits of precision.
     * Thus the sum below should not overflow an integer.
     * Also, since nsteps should be less than srcw then
     * (srcLoc + nsteps * srcInc) should not overflow either
     */
    nsteps = (((srcLimit - srcPos) << shift) - srcLoc + srcInc - 1) / srcInc;
    *pDstPos += nsteps;
    *pSrcLoc = (srcLoc + nsteps * srcInc) & ((1 << shift) - 1);
}

static void
Scale_adjustDn(jint srcPos, jint srcStart, jint srcLimit,
	       jint srcLoc, jint srcInc, jint shift,
	       jint dstPos, jint *pDstLimit)
{
    jint nsteps;

    if (srcLimit >= srcPos) return;
    /* The Overflow checking on this is nearly identical to adjustUp. */
    nsteps = (((srcLimit - srcStart) << shift) - srcLoc + srcInc - 1) / srcInc;
    *pDstLimit = dstPos + nsteps;
}

/*
 * Class:     sun_java2d_loops_ScaledBlit
 * Method:    Scale
 * Signature: (Lsun/java2d/SurfaceData;Lsun/java2d/SurfaceData;Ljava/awt/Composite;IIIIIIII)V
 */
JNIEXPORT void JNICALL
Java_sun_java2d_loops_ScaledBlit_Scale
    (JNIEnv *env, jobject self,
     jobject srcData, jobject dstData, jobject comp,
     jint srcx, jint srcy, jint dstx, jint dsty,
     jint srcw, jint srch, jint dstw, jint dsth,
     jint clipx1, jint clipy1, jint clipx2, jint clipy2)
{
    SurfaceDataOps *srcOps;
    SurfaceDataOps *dstOps;
    SurfaceDataRasInfo srcInfo;
    SurfaceDataRasInfo dstInfo;
    SurfaceDataBounds clipbounds;
    NativePrimitive *pPrim;
    CompositeInfo compInfo;
    jint sxloc, syloc, sxinc, syinc, shift;
    double scale;

    pPrim = GetNativePrim(env, self);
    if (pPrim == NULL) {
	return;
    }
    if (pPrim->pCompType->getCompInfo != NULL) {
	(*pPrim->pCompType->getCompInfo)(env, &compInfo, comp);
    }

    srcOps = SurfaceData_GetOps(env, srcData);
    dstOps = SurfaceData_GetOps(env, dstData);
    if (srcOps == 0 || dstOps == 0) {
	return;
    }

    /*
     * Determine the precision to use for the fixed point math
     * for the coordinate scaling.
     * - OR together srcw and srch to get the MSB between the two
     * - Next shift it up until it goes negative
     * - Count the shifts and that will be the most accurate
     *   precision available for the fixed point math
     * - 1.0 will be (1 << shift)
     * - srcw & srch will be (srcw << shift) and (srch << shift)
     *   and will not overflow
     */
    sxloc = srcw | srch;
    shift = 0;
    while ((sxloc <<= 1) > 0) {
	shift++;
    }
    sxloc = (1 << shift);
    scale = ((double) (srch)) / ((double) (dsth));
    syinc = (int) (scale * sxloc);
    scale = ((double) (srcw)) / ((double) (dstw));
    sxinc = (int) (scale * sxloc);

    /*
     * Round by setting the initial sxyloc to half a destination
     * pixel which equals half of the x/y increments.
     */
    sxloc = sxinc / 2;
    syloc = syinc / 2;

    srcw += srcx;
    srch += srcy;
    srcInfo.bounds.x1 = srcx;
    srcInfo.bounds.y1 = srcy;
    srcInfo.bounds.x2 = srcw;
    srcInfo.bounds.y2 = srch;
    if (srcOps->Lock(env, srcOps, &srcInfo, pPrim->srcflags) != SD_SUCCESS) {
	return;
    }
    if (srcInfo.bounds.x2 <= srcInfo.bounds.x1 ||
	srcInfo.bounds.y2 <= srcInfo.bounds.y1)
    {
	SurfaceData_InvokeUnlock(env, srcOps, &srcInfo);
	return;
    }
    dstw += dstx;
    dsth += dsty;
    Scale_adjustUp(srcx, srcInfo.bounds.x1,
		   &sxloc, sxinc, shift,
		   &dstx);
    Scale_adjustUp(srcy, srcInfo.bounds.y1,
		   &syloc, syinc, shift,
		   &dsty);
    Scale_adjustDn(srcw, srcInfo.bounds.x1, srcInfo.bounds.x2,
		   sxloc, sxinc, shift,
		   dstx, &dstw);
    Scale_adjustDn(srch, srcInfo.bounds.y1, srcInfo.bounds.y2,
		   syloc, syinc, shift,
		   dsty, &dsth);

    dstInfo.bounds.x1 = dstx;
    dstInfo.bounds.y1 = dsty;
    dstInfo.bounds.x2 = dstw;
    dstInfo.bounds.y2 = dsth;
    clipbounds.x1 = clipx1;
    clipbounds.y1 = clipy1;
    clipbounds.x2 = clipx2;
    clipbounds.y2 = clipy2;
    SurfaceData_IntersectBounds(&dstInfo.bounds, &clipbounds);
    if (dstOps->Lock(env, dstOps, &dstInfo, pPrim->dstflags) != SD_SUCCESS) {
	SurfaceData_InvokeUnlock(env, srcOps, &srcInfo);
	return;
    }

    if (dstInfo.bounds.x2 > dstInfo.bounds.x1 &&
	dstInfo.bounds.y2 > dstInfo.bounds.y1)
    {
	if (dstInfo.bounds.y1 > dsty) {
	    syloc += syinc * (dstInfo.bounds.y1 - dsty);
	}
	if (dstInfo.bounds.x1 > dstx) {
	    sxloc += sxinc * (dstInfo.bounds.x1 - dstx);
	}
	srcOps->GetRasInfo(env, srcOps, &srcInfo);
	dstOps->GetRasInfo(env, dstOps, &dstInfo);
	if (srcInfo.rasBase && dstInfo.rasBase) {
	    jint width = dstInfo.bounds.x2 - dstInfo.bounds.x1;
	    jint height = dstInfo.bounds.y2 - dstInfo.bounds.y1;
	    void *pSrc = PtrCoord(srcInfo.rasBase,
				  srcInfo.bounds.x1, srcInfo.pixelStride,
				  srcInfo.bounds.y1, srcInfo.scanStride);
	    void *pDst = PtrCoord(dstInfo.rasBase,
				  dstInfo.bounds.x1, dstInfo.pixelStride,
				  dstInfo.bounds.y1, dstInfo.scanStride);
	    (*pPrim->funcs.scaledblit)(pSrc, pDst, width, height,
				       sxloc, syloc, sxinc, syinc, shift,
				       &srcInfo, &dstInfo, pPrim, &compInfo);
	}
	SurfaceData_InvokeRelease(env, dstOps, &dstInfo);
	SurfaceData_InvokeRelease(env, srcOps, &srcInfo);
    }
    SurfaceData_InvokeUnlock(env, dstOps, &dstInfo);
    SurfaceData_InvokeUnlock(env, srcOps, &srcInfo);
}

/*
 * @(#)X11Renderer.c	1.14 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "sun_awt_X11Renderer.h"

#include "X11SurfaceData.h"
#include "SpanIterator.h"

#include <jlong.h>

#ifndef HEADLESS
#define POLYTEMPSIZE	(int)(256 / sizeof(XPoint))
#define ABS(n)		(((n) < 0) ? -(n) : (n))
#define CLAMP_TO_SHORT(x)   (((x) > 32767) ? 32767 : ((x) < -32768) ? -32768 : (x))
#define CLAMP_TO_USHORT(x)  (((x) > 65535) ? 65535 : ((x) < 0) ? 0 : (x))

static void
awt_drawArc(JNIEnv * env, X11SDOps *xsdo, GC xgc,
            int x, int y, int w, int h,
            int startAngle, int endAngle,
            int filled)
{
    int s, e;

    if (w < 0 || h < 0) {
        return;
    }
    if (endAngle >= 360 || endAngle <= -360) {
        s = 0;
        e = 360 * 64;
    } else {
        s = (startAngle % 360) * 64;
        e = endAngle * 64;
    }
    if (filled == 0) {
        XDrawArc(awt_display, xsdo->drawable, xgc, x, y, w, h, s, e);
    } else {
        XFillArc(awt_display, xsdo->drawable, xgc, x, y, w, h, s, e);
    }
}

static XPoint *
transformPoints(JNIEnv * env,
                jintArray xcoordsArray, jintArray ycoordsArray,
		jint transx, jint transy,
                XPoint * points, int *pNpoints, int close)
{
    int i;
    int npoints = *pNpoints;
    jint *xcoords, *ycoords;

    xcoords = (jint *)
	(*env)->GetPrimitiveArrayCritical(env, xcoordsArray, NULL);
    if (xcoords == NULL) {
        return 0;
    }
    
    ycoords = (jint *)
	(*env)->GetPrimitiveArrayCritical(env, ycoordsArray, NULL);
    if (ycoords == NULL) {
        (*env)->ReleasePrimitiveArrayCritical(env, xcoordsArray, xcoords,
                                              JNI_ABORT);
        return 0;
    }

    if (close) {
        close = (npoints > 2
                 && (xcoords[npoints - 1] != xcoords[0] ||
		     ycoords[npoints - 1] != ycoords[0]));
        if (close) {
            npoints++;
            *pNpoints = npoints;
        }
    }
    if (npoints > POLYTEMPSIZE) {
        points = (XPoint *) malloc(sizeof(XPoint) * npoints);
    }
    if (points != NULL) {
	if (close) {
	    npoints--;
	}
	for (i = 0; i < npoints; i++) {
	    points[i].x = CLAMP_TO_SHORT(xcoords[i] + transx);
	    points[i].y = CLAMP_TO_SHORT(ycoords[i] + transy);
	}
	if (close) {
	    points[npoints] = points[0];
	}
    }

    (*env)->ReleasePrimitiveArrayCritical(env, xcoordsArray, xcoords,
                                          JNI_ABORT);
    (*env)->ReleasePrimitiveArrayCritical(env, ycoordsArray, ycoords,
                                          JNI_ABORT);

    return points;
}
#endif /* !HEADLESS */

/*
 * Class:     sun_awt_X11Renderer
 * Method:    doDrawLine
 * Signature: (Lsun/java2d/SurfaceData;Lsun/java2d/pipe/Region;Ljava/awt/Composite;IIIII)V
 */
JNIEXPORT void JNICALL Java_sun_awt_X11Renderer_doDrawLine
    (JNIEnv *env, jobject xr,
     jobject sData, jobject clip, jobject comp, jint pixel,
     jint x1, jint y1, jint x2, jint y2)
{
#ifndef HEADLESS
    X11SDOps *xsdo;
    GC xgc;

    xsdo = X11SurfaceData_GetOps(env, sData);
    if (xsdo == NULL) {
	return;
    }
    xgc = xsdo->GetGC(env, xsdo, clip, comp, pixel);
    if (xgc == NULL) {
	return;
    }
    XDrawLine(awt_display, xsdo->drawable, xgc, 
	      CLAMP_TO_SHORT(x1), CLAMP_TO_SHORT(y1), 
	      CLAMP_TO_SHORT(x2), CLAMP_TO_SHORT(y2));
    xsdo->ReleaseGC(env, xsdo, xgc);
#endif /* !HEADLESS */
}

/*
 * Class:     sun_awt_X11Renderer
 * Method:    doDrawRect
 * Signature: (Lsun/java2d/SurfaceData;Lsun/java2d/pipe/Region;Ljava/awt/Composite;IIIII)V
 */
JNIEXPORT void JNICALL Java_sun_awt_X11Renderer_doDrawRect
    (JNIEnv *env, jobject xr,
     jobject sData, jobject clip, jobject comp, jint pixel,
     jint x, jint y, jint w, jint h)
{
#ifndef HEADLESS
    X11SDOps *xsdo;
    GC xgc;

    if (w < 0 || h < 0) {
        return;
    }

    xsdo = X11SurfaceData_GetOps(env, sData);
    if (xsdo == NULL) {
	return;
    }
    xgc = xsdo->GetGC(env, xsdo, clip, comp, pixel);
    if (xgc == NULL) {
	return;
    }
    XDrawRectangle(awt_display, xsdo->drawable, xgc, 
		   CLAMP_TO_SHORT(x),  CLAMP_TO_SHORT(y),
		   CLAMP_TO_USHORT(w), CLAMP_TO_USHORT(h));
    xsdo->ReleaseGC(env, xsdo, xgc);
#endif /* !HEADLESS */
}

/*
 * Class:     sun_awt_X11Renderer
 * Method:    doDrawRoundRect
 * Signature: (Lsun/java2d/SurfaceData;Lsun/java2d/pipe/Region;Ljava/awt/Composite;IIIIIII)V
 */
JNIEXPORT void JNICALL Java_sun_awt_X11Renderer_doDrawRoundRect
    (JNIEnv *env, jobject xr,
     jobject sData, jobject clip, jobject comp, jint pixel,
     jint x, jint y, jint w, jint h,
     jint arcW, jint arcH)
{
#ifndef HEADLESS
    X11SDOps *xsdo;
    GC xgc;
    long ty1, ty2, tx1, tx2;

    if (w < 0 || h < 0) {
        return;
    }

    xsdo = X11SurfaceData_GetOps(env, sData);
    if (xsdo == NULL) {
	return;
    }
    xgc = xsdo->GetGC(env, xsdo, clip, comp, pixel);
    if (xgc == NULL) {
	return;
    }

    arcW = ABS(arcW);
    arcH = ABS(arcH);
    if (arcW > w) {
        arcW = w;
    }
    if (arcH > h) {
        arcH = h;
    }

    if (arcW == 0 || arcH == 0) {
	XDrawRectangle(awt_display, xsdo->drawable, xgc, x, y, w, h);
	xsdo->ReleaseGC(env, xsdo, xgc);
	return;
    }

    awt_drawArc(env, xsdo, xgc,
                x, y, arcW, arcH,
                90, 90, JNI_FALSE);
    awt_drawArc(env, xsdo, xgc,
                x + w - arcW, y, arcW, arcH,
                0, 90, JNI_FALSE);
    awt_drawArc(env, xsdo, xgc,
                x, y + h - arcH, arcW, arcH,
                180, 90, JNI_FALSE);
    awt_drawArc(env, xsdo, xgc,
                x + w - arcW, y + h - arcH, arcW, arcH,
                270, 90, JNI_FALSE);

    tx1 = x + (arcW / 2) + 1;
    tx2 = x + w - (arcW / 2) - 1;
    ty1 = y + (arcH / 2) + 1;
    ty2 = y + h - (arcH / 2) - 1;

    if (tx1 <= tx2) {
	XDrawLine(awt_display, xsdo->drawable, xgc,
		  tx1, y, tx2, y);
	if (h > 0) {
	    XDrawLine(awt_display, xsdo->drawable, xgc,
		      tx1, y+h, tx2, y+h);
	}
    }
    if (ty1 <= ty2) {
	XDrawLine(awt_display, xsdo->drawable, xgc,
		  x, ty1, x, ty2);
	if (w > 0) {
	    XDrawLine(awt_display, xsdo->drawable, xgc,
		      x+w, ty1, x+w, ty2);
	}
    }

    xsdo->ReleaseGC(env, xsdo, xgc);
#endif /* !HEADLESS */
}

/*
 * Class:     sun_awt_X11Renderer
 * Method:    doDrawOval
 * Signature: (Lsun/java2d/SurfaceData;Lsun/java2d/pipe/Region;Ljava/awt/Composite;IIIII)V
 */
JNIEXPORT void JNICALL Java_sun_awt_X11Renderer_doDrawOval
    (JNIEnv *env, jobject xr,
     jobject sData, jobject clip, jobject comp, jint pixel,
     jint x, jint y, jint w, jint h)
{
#ifndef HEADLESS
    X11SDOps *xsdo;
    GC xgc;

    xsdo = X11SurfaceData_GetOps(env, sData);
    if (xsdo == NULL) {
	return;
    }
    xgc = xsdo->GetGC(env, xsdo, clip, comp, pixel);
    if (xgc == NULL) {
	return;
    }
    if (w < 2 || h < 2) {
	/*
	 * Fix for 4205762 - 1x1 ovals do not draw on Ultra1, Creator3d
	 * (related to 4411814 on Windows platform)
	 * Really small ovals degenerate to simple rectangles as they
	 * have no curvature or enclosed area.  Use XFillRectangle
	 * for speed and to deal better with degenerate sizes.
	 */
	if (w >= 0 && h >= 0) {
	    XFillRectangle(awt_display, xsdo->drawable, xgc, x, y, w+1, h+1);
	}
    } else {
	awt_drawArc(env, xsdo, xgc, x, y, w, h, 0, 360, JNI_FALSE);
    }
    xsdo->ReleaseGC(env, xsdo, xgc);
#endif /* !HEADLESS */
}

/*
 * Class:     sun_awt_X11Renderer
 * Method:    doDrawArc
 * Signature: (Lsun/java2d/SurfaceData;Lsun/java2d/pipe/Region;Ljava/awt/Composite;IIIIIII)V
 */
JNIEXPORT void JNICALL Java_sun_awt_X11Renderer_doDrawArc
    (JNIEnv *env, jobject xr,
     jobject sData, jobject clip, jobject comp, jint pixel,
     jint x, jint y, jint w, jint h,
     jint angleStart, jint angleExtent)
{
#ifndef HEADLESS
    X11SDOps *xsdo;
    GC xgc;

    xsdo = X11SurfaceData_GetOps(env, sData);
    if (xsdo == NULL) {
	return;
    }
    xgc = xsdo->GetGC(env, xsdo, clip, comp, pixel);
    if (xgc == NULL) {
	return;
    }
    awt_drawArc(env, xsdo, xgc, x, y, w, h, angleStart, angleExtent, JNI_FALSE);
    xsdo->ReleaseGC(env, xsdo, xgc);
#endif /* !HEADLESS */
}

/*
 * Class:     sun_awt_X11Renderer
 * Method:    doDrawPoly
 * Signature: (Lsun/java2d/SurfaceData;Lsun/java2d/pipe/Region;Ljava/awt/Composite;III[I[IIZ)V
 */
JNIEXPORT void JNICALL Java_sun_awt_X11Renderer_doDrawPoly
    (JNIEnv *env, jobject xr,
     jobject sData, jobject clip, jobject comp, jint pixel,
     jint transx, jint transy,
     jintArray xcoordsArray, jintArray ycoordsArray, jint npoints,
     jboolean isclosed)
{
#ifndef HEADLESS
    X11SDOps *xsdo;
    GC xgc;
    XPoint pTmp[POLYTEMPSIZE], *points;

    if (JNU_IsNull(env, xcoordsArray) || JNU_IsNull(env, ycoordsArray)) {
        JNU_ThrowNullPointerException(env, "coordinate array");
        return;
    }
    if ((*env)->GetArrayLength(env, ycoordsArray) < npoints ||
	(*env)->GetArrayLength(env, xcoordsArray) < npoints)
    {
        JNU_ThrowArrayIndexOutOfBoundsException(env, "coordinate array");
        return;
    }

    xsdo = X11SurfaceData_GetOps(env, sData);
    if (xsdo == NULL) {
	return;
    }
    xgc = xsdo->GetGC(env, xsdo, clip, comp, pixel);
    if (xgc == NULL) {
	return;
    }

    points = transformPoints(env, xcoordsArray, ycoordsArray, transx, transy,
                             pTmp, (int *)&npoints, isclosed);
    if (points == 0) {
        JNU_ThrowOutOfMemoryError(env, "translated coordinate array");
    } else {
	XDrawLines(awt_display, xsdo->drawable, xgc,
		   points, npoints, CoordModeOrigin);
	if (points != pTmp) {
	    free(points);
	}
    }

    xsdo->ReleaseGC(env, xsdo, xgc);
#endif /* !HEADLESS */
}

/*
 * Class:     sun_awt_X11Renderer
 * Method:    doFillRect
 * Signature: (Lsun/java2d/SurfaceData;Lsun/java2d/pipe/Region;Ljava/awt/Composite;IIIII)V
 */
JNIEXPORT void JNICALL Java_sun_awt_X11Renderer_doFillRect
    (JNIEnv *env, jobject xr,
     jobject sData, jobject clip, jobject comp, jint pixel,
     jint x, jint y, jint w, jint h)
{
#ifndef HEADLESS
    X11SDOps *xsdo;
    GC xgc;

    if (w <= 0 || h <= 0) {
        return;
    }

    xsdo = X11SurfaceData_GetOps(env, sData);
    if (xsdo == NULL) {
	return;
    }
    xgc = xsdo->GetGC(env, xsdo, clip, comp, pixel);
    if (xgc == NULL) {
	return;
    }
    XFillRectangle(awt_display, xsdo->drawable, xgc, 
		   CLAMP_TO_SHORT(x),  CLAMP_TO_SHORT(y),
		   CLAMP_TO_USHORT(w), CLAMP_TO_USHORT(h));
    xsdo->ReleaseGC(env, xsdo, xgc);
#endif /* !HEADLESS */
}

/*
 * Class:     sun_awt_X11Renderer
 * Method:    doFillRoundRect
 * Signature: (Lsun/java2d/SurfaceData;Lsun/java2d/pipe/Region;Ljava/awt/Composite;IIIIIII)V
 */
JNIEXPORT void JNICALL Java_sun_awt_X11Renderer_doFillRoundRect
    (JNIEnv *env, jobject xr,
     jobject sData, jobject clip, jobject comp, jint pixel,
     jint x, jint y, jint w, jint h,
     jint arcW, jint arcH)
{
#ifndef HEADLESS
    X11SDOps *xsdo;
    GC xgc;
    long ty1, ty2, tx1, tx2;

    if (w <= 0 || h <= 0) {
        return;
    }
    arcW = ABS(arcW);
    arcH = ABS(arcH);
    if (arcW > w) {
        arcW = w;
    }
    if (arcH > h) {
        arcH = h;
    }

    xsdo = X11SurfaceData_GetOps(env, sData);
    if (xsdo == NULL) {
	return;
    }
    xgc = xsdo->GetGC(env, xsdo, clip, comp, pixel);
    if (xgc == NULL) {
	return;
    }

    tx1 = x + (arcW / 2);
    tx2 = x + w - (arcW / 2);
    ty1 = y + (arcH / 2);
    ty2 = y + h - (arcH / 2);

    awt_drawArc(env, xsdo, xgc,
                x, y, arcW, arcH,
                90, 90, JNI_TRUE);
    awt_drawArc(env, xsdo, xgc,
                x + w - arcW, y, arcW, arcH,
                0, 90, JNI_TRUE);
    awt_drawArc(env, xsdo, xgc,
                x, y + h - arcH, arcW, arcH,
                180, 90, JNI_TRUE);
    awt_drawArc(env, xsdo, xgc,
                x + w - arcW, y + h - arcH, arcW, arcH,
                270, 90, JNI_TRUE);

    XFillRectangle(awt_display, xsdo->drawable, xgc,
                   tx1, y, tx2 - tx1, h);
    XFillRectangle(awt_display, xsdo->drawable, xgc,
                   x, ty1, tx1 - x, ty2 - ty1);
    XFillRectangle(awt_display, xsdo->drawable, xgc,
                   tx2, ty1, x + w - tx2, ty2 - ty1);

    xsdo->ReleaseGC(env, xsdo, xgc);
#endif /* !HEADLESS */
}

/*
 * Class:     sun_awt_X11Renderer
 * Method:    doFillOval
 * Signature: (Lsun/java2d/SurfaceData;Lsun/java2d/pipe/Region;Ljava/awt/Composite;IIIII)V
 */
JNIEXPORT void JNICALL Java_sun_awt_X11Renderer_doFillOval
    (JNIEnv *env, jobject xr,
     jobject sData, jobject clip, jobject comp, jint pixel,
     jint x, jint y, jint w, jint h)
{
#ifndef HEADLESS
    X11SDOps *xsdo;
    GC xgc;

    xsdo = X11SurfaceData_GetOps(env, sData);
    if (xsdo == NULL) {
	return;
    }
    xgc = xsdo->GetGC(env, xsdo, clip, comp, pixel);
    if (xgc == NULL) {
	return;
    }
    if (w < 3 || h < 3) {
	/*
	 * Fix for 4205762 - 1x1 ovals do not draw on Ultra1, Creator3d
	 * (related to 4411814 on Windows platform)
	 * Most X11 servers drivers have poor rendering
	 * for thin ellipses and the rendering is most strikingly
	 * different from our theoretical arcs.  Ideally we should
	 * trap all ovals less than some fairly large size and
	 * try to draw aesthetically pleasing ellipses, but that
	 * would require considerably more work to get the corresponding
	 * drawArc variants to match pixel for pixel.
	 * Thin ovals of girth 1 pixel are simple rectangles.
	 * Thin ovals of girth 2 pixels are simple rectangles with
	 * potentially smaller lengths.  Determine the correct length
	 * by calculating .5*.5 + scaledlen*scaledlen == 1.0 which
	 * means that scaledlen is the sqrt(0.75).  Scaledlen is
	 * relative to the true length (w or h) and needs to be
	 * adjusted by half a pixel in different ways for odd or
	 * even lengths.
	 */
#define SQRT_3_4 0.86602540378443864676
	if (w > 2 && h > 1) {
	    int adjw = (int) ((SQRT_3_4 * w - ((w&1)-1)) * 0.5);
	    adjw = adjw * 2 + (w&1);
	    x += (w-adjw)/2;
	    w = adjw;
	} else if (h > 2 && w > 1) {
	    int adjh = (int) ((SQRT_3_4 * h - ((h&1)-1)) * 0.5);
	    adjh = adjh * 2 + (h&1);
	    y += (h-adjh)/2;
	    h = adjh;
	}
#undef SQRT_3_4
	if (w > 0 && h > 0) {
	    XFillRectangle(awt_display, xsdo->drawable, xgc, x, y, w, h);
	}
    } else {
	awt_drawArc(env, xsdo, xgc, x, y, w, h, 0, 360, JNI_TRUE);
    }
    xsdo->ReleaseGC(env, xsdo, xgc);
#endif /* !HEADLESS */
}

/*
 * Class:     sun_awt_X11Renderer
 * Method:    doFillArc
 * Signature: (Lsun/java2d/SurfaceData;Lsun/java2d/pipe/Region;Ljava/awt/Composite;IIIIIII)V
 */
JNIEXPORT void JNICALL Java_sun_awt_X11Renderer_doFillArc
    (JNIEnv *env, jobject xr,
     jobject sData, jobject clip, jobject comp, jint pixel,
     jint x, jint y, jint w, jint h,
     jint angleStart, jint angleExtent)
{
#ifndef HEADLESS
    X11SDOps *xsdo;
    GC xgc;

    xsdo = X11SurfaceData_GetOps(env, sData);
    if (xsdo == NULL) {
	return;
    }
    xgc = xsdo->GetGC(env, xsdo, clip, comp, pixel);
    if (xgc == NULL) {
	return;
    }
    awt_drawArc(env, xsdo, xgc, x, y, w, h, angleStart, angleExtent, JNI_TRUE);
    xsdo->ReleaseGC(env, xsdo, xgc);
#endif /* !HEADLESS */
}

/*
 * Class:     sun_awt_X11Renderer
 * Method:    doFillPoly
 * Signature: (Lsun/java2d/SurfaceData;Lsun/java2d/pipe/Region;Ljava/awt/Composite;III[I[II)V
 */
JNIEXPORT void JNICALL Java_sun_awt_X11Renderer_doFillPoly
    (JNIEnv *env, jobject xr,
     jobject sData, jobject clip, jobject comp, jint pixel,
     jint transx, jint transy,
     jintArray xcoordsArray, jintArray ycoordsArray, jint npoints)
{
#ifndef HEADLESS
    X11SDOps *xsdo;
    GC xgc;
    XPoint pTmp[POLYTEMPSIZE], *points;

    if (JNU_IsNull(env, xcoordsArray) || JNU_IsNull(env, ycoordsArray)) {
        JNU_ThrowNullPointerException(env, "coordinate array");
        return;
    }
    if ((*env)->GetArrayLength(env, ycoordsArray) < npoints ||
	(*env)->GetArrayLength(env, xcoordsArray) < npoints)
    {
        JNU_ThrowArrayIndexOutOfBoundsException(env, "coordinate array");
        return;
    }

    xsdo = X11SurfaceData_GetOps(env, sData);
    if (xsdo == NULL) {
	return;
    }
    xgc = xsdo->GetGC(env, xsdo, clip, comp, pixel);
    if (xgc == NULL) {
	return;
    }

    points = transformPoints(env, xcoordsArray, ycoordsArray, transx, transy,
                             pTmp, (int *)&npoints, JNI_FALSE);
    if (points == 0) {
        JNU_ThrowOutOfMemoryError(env, "translated coordinate array");
    } else {
	XFillPolygon(awt_display, xsdo->drawable, xgc,
		     points, npoints, Complex, CoordModeOrigin);
	if (points != pTmp) {
	    free(points);
	}
    }

    xsdo->ReleaseGC(env, xsdo, xgc);
#endif /* !HEADLESS */
}

/*
 * Class:     sun_awt_X11Renderer
 * Method:    devFillSpans
 * Signature: (Lsun/java2d/pipe/SpanIterator;JLjava/awt/Composite;III)V
 */
JNIEXPORT void JNICALL Java_sun_awt_X11Renderer_devFillSpans
    (JNIEnv *env, jobject xr,
     jobject sData, jobject si, jlong pIterator, jobject comp,
     jint transx, jint transy, jint pixel)
{
#ifndef HEADLESS
    X11SDOps *xsdo;
    GC xgc;
    Drawable drawable;
    SpanIteratorFuncs *pFuncs = (SpanIteratorFuncs *) jlong_to_ptr(pIterator);
    void *srData;
    jint x, y, w, h;
    jint spanbox[4];

    if (JNU_IsNull(env, si)) {
	JNU_ThrowNullPointerException(env, "span iterator");
	return;
    }
    if (pFuncs == NULL) {
	JNU_ThrowNullPointerException(env, "native iterator not supplied");
	return;
    }

    xsdo = X11SurfaceData_GetOps(env, sData);
    if (xsdo == NULL) {
	return;
    }
    xgc = xsdo->GetGC(env, xsdo, NULL, comp, pixel);
    if (xgc == NULL) {
	return;
    }
    drawable = xsdo->drawable;

    srData = (*pFuncs->open)(env, si);
    while ((*pFuncs->nextSpan)(srData, spanbox)) {
	x = spanbox[0] + transx;
	y = spanbox[1] + transy;
	w = spanbox[2] - spanbox[0];
	h = spanbox[3] - spanbox[1];
	XFillRectangle(awt_display, drawable, xgc,
		       CLAMP_TO_SHORT(x),  CLAMP_TO_SHORT(y),
		       CLAMP_TO_USHORT(w), CLAMP_TO_USHORT(h));
    }
    (*pFuncs->close)(env, srData);

    xsdo->ReleaseGC(env, xsdo, xgc);
#endif /* !HEADLESS */
}

/*
 * Class:     sun_awt_X11Renderer
 * Method:    devCopyArea
 * Signature: (Lsun/java2d/SurfaceData;IIIIII)V
 */
JNIEXPORT void JNICALL
Java_sun_awt_X11Renderer_devCopyArea
    (JNIEnv *env, jobject xr,
     jobject xsd,
     jint srcx, jint srcy,
     jint dstx, jint dsty,
     jint width, jint height)
{
#ifndef HEADLESS
    X11SDOps *xsdo;
    GC xgc;

    xsdo = X11SurfaceData_GetOps(env, xsd);
    if (xsdo == NULL) {
	return;
    }
    xgc = xsdo->GetGC(env, xsdo, NULL, NULL, xsdo->lastpixel);
    if (xgc == NULL) {
	return;
    }

    XCopyArea(awt_display, xsdo->drawable, xsdo->drawable, xgc,
	      srcx, srcy, width, height, dstx, dsty);

    xsdo->ReleaseGC(env, xsdo, xgc);
#endif /* !HEADLESS */
}

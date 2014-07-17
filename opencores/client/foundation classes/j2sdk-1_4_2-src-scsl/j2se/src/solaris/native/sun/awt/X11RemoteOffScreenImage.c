/*
 * @(#)X11RemoteOffScreenImage.c	1.5 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


#include "sun_awt_motif_X11RemoteOffScreenImage.h"
#include "malloc.h"

#include "SurfaceData.h"

#include "colordata.h"
#include "img_util_md.h"

#include "jni_util.h"

static jfieldID		sDataID;
static jfieldID		rasterID;

/*
 * Class:     sun_awt_X11RemoteOffScreenImage
 * Method:    initIDs
 * Signature: ()V
 */
JNIEXPORT void JNICALL
Java_sun_awt_motif_X11RemoteOffScreenImage_initIDs(JNIEnv *env, jclass bisd)
{
    static char *bimgName = "java/awt/image/BufferedImage";
    jclass bimg = (*env)->FindClass(env, bimgName);
    if (JNU_IsNull(env, bimg)) {
	JNU_ThrowClassNotFoundException(env, bimgName);
	return;
    }
    sDataID = (*env)->GetFieldID(env, bimg, "sData", "Lsun/java2d/SurfaceData;");
    if (sDataID == 0) {
	JNU_ThrowInternalError(env, "Could not get field IDs");
    }

    rasterID = (*env)->GetFieldID(env, bimg, "raster", "Ljava/awt/image/WritableRaster;");
    if (rasterID == 0) {
	JNU_ThrowInternalError(env, "Could not get field IDs");
    }
}


/*
 * Class:     sun_awt_motif_X11RemoteOffScreenImage
 * Method:    setSurfaceData
 * Signature: (Lsun/java2d/SurfaceData;)V
 */
JNIEXPORT void JNICALL
Java_sun_awt_motif_X11RemoteOffScreenImage_setSurfaceDataNative
    (JNIEnv *env, jobject bufImg, jobject sData)
{
    (*env)->SetObjectField(env, bufImg, sDataID, sData);
}

/*
 * Class:     sun_awt_motif_X11RemoteOffScreenImage
 * Method:    setRasterNative
 * Signature: (Ljava/awt/image/WritableRaster;)V
 */
JNIEXPORT void JNICALL
Java_sun_awt_motif_X11RemoteOffScreenImage_setRasterNative
    (JNIEnv *env, jobject bufImg, jobject raster)
{
    (*env)->SetObjectField(env, bufImg, rasterID, raster);
}

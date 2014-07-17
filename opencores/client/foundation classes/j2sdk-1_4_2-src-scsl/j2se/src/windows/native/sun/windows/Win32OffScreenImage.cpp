/*
 * @(#)Win32OffScreenImage.cpp	1.7 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


#include "sun_awt_windows_Win32OffScreenImage.h"
#include "malloc.h"

#include "SurfaceData.h"

#include "colordata.h"

#include "jni_util.h"

static jfieldID		sDataID;

/*
 * Class:     sun_awt_windows_Win32OffScreenImage
 * Method:    initIDs
 * Signature: ()V
 */
JNIEXPORT void JNICALL
Java_sun_awt_windows_Win32OffScreenImage_initIDs(JNIEnv *env, jclass bisd)
{
    static char *bimgName = "java/awt/image/BufferedImage";
    jclass bimg = env->FindClass(bimgName);
    if (JNU_IsNull(env, bimg)) {
	JNU_ThrowClassNotFoundException(env, bimgName);
	return;
    }
    sDataID = env->GetFieldID(bimg, "sData", "Lsun/java2d/SurfaceData;");
    if (sDataID == 0) {
	JNU_ThrowInternalError(env, "Could not get field IDs");
    }
}


/*
 * Class:     sun_awt_windows_Win32OffScreenImage
 * Method:    setSurfaceData
 * Signature: (Ljava/awt/image/BufferedImage;Lsun/java2d/SurfaceData;)V
 */
JNIEXPORT void JNICALL
Java_sun_awt_windows_Win32OffScreenImage_setSurfaceData
    (JNIEnv *env, jclass bisd, jobject bufImg, jobject sData)
{
    env->SetObjectField(bufImg, sDataID, sData);
}


/*
 * Class:     sun_awt_windows_Win32OffScreenImage
 * Method:    getScanlineStride
 * Signature: (Lsun/java2d/SurfaceData;)I
 */
JNIEXPORT jint JNICALL
Java_sun_awt_windows_Win32OffScreenImage_getScanlineStride
    (JNIEnv *env, jclass bisd, jobject sData)
{
    SurfaceDataOps *sDataOps;
    SurfaceDataRasInfo sDataInfo;

    sDataOps = SurfaceData_GetOps(env, sData);
    if (sDataOps == 0) {
	return 0;
    }
    sDataInfo.bounds.x1 = 0;
    sDataInfo.bounds.y1 = 0;
    sDataInfo.bounds.x2 = 1;
    sDataInfo.bounds.y2 = 1;
    if (sDataOps->Lock(env, sDataOps, &sDataInfo, 
		       SD_LOCK_RD_WR) != SD_SUCCESS) 
    {
	return 0;
    }
    sDataOps->GetRasInfo(env, sDataOps, &sDataInfo);
    SurfaceData_InvokeRelease(env, sDataOps, &sDataInfo);
    SurfaceData_InvokeUnlock(env, sDataOps, &sDataInfo);

    // We divide-out the pixel size because we will later account
    // for that size when we access the pixels according to the
    // datatype of the DataBuffer we are using.
    return sDataInfo.scanStride / sDataInfo.pixelStride;
}


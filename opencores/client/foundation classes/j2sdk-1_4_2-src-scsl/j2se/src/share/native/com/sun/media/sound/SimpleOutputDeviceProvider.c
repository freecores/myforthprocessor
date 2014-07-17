/*
 * @(#)SimpleOutputDeviceProvider.c	1.10 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


// STANDARD includes


// JNI includes
#include <jni.h>

// ENGINE includes
#include "engine/X_API.h"
#include "engine/GenSnd.h"
#include "engine/GenPriv.h"		// $$kk: 10.02.98: need for references to MusicGlobals
#include "engine/HAE_API.h"		// $$kk: 10.02.98: need for HAE_GetMaxSamplePerSlice(); should get rid of this??


// UTILITY includes
#include "Utilities.h"

// SimpleOutputDeviceProvider includes
#include "com_sun_media_sound_SimpleOutputDeviceProvider.h"

// $$kk: 03.29.99: need to figure out how long the name might be.
// for win32, is is MAXPNAMELEN = 32, as defined in mmsystem.h.
// what about solaris??
#define MAX_STRING_LENGTH 128


JNIEXPORT jint JNICALL
    Java_com_sun_media_sound_SimpleOutputDeviceProvider_nGetNumDevices(JNIEnv* e, jobject thisObj)
{
    INT32 numDevices;
	
    TRACE0("Java_com_sun_media_sound_SimpleOutputDeviceProvider_nGetNumDevices.\n");

    numDevices = GM_MaxDevices();

    TRACE1("Java_com_sun_media_sound_SimpleOutputDeviceProvider_nGetNumDevices returning %d.\n", numDevices);

    return numDevices;
}


JNIEXPORT jstring JNICALL
    Java_com_sun_media_sound_SimpleOutputDeviceProvider_nGetName(JNIEnv* e, jobject thisObj, jint index)
{
    char devName[MAX_STRING_LENGTH];
    jstring jDevName;

    TRACE0("Java_com_sun_media_sound_SimpleOutputDeviceProvider_nGetName.\n");

    GM_GetDeviceName((INT32)index, devName, (UINT32)MAX_STRING_LENGTH);

    // $$jb: 11.15.99: GM_GetDeviceName() is not returning a Pascal-type string
    //XPtoCstr(devName);
    jDevName = (*e)->NewStringUTF(e, devName);

    TRACE0("Java_com_sun_media_sound_SimpleOutputDeviceProvider_nGetName completed.\n");

    return jDevName;
}


JNIEXPORT jstring JNICALL
    Java_com_sun_media_sound_SimpleOutputDeviceProvider_nGetVendor(JNIEnv* e, jobject thisObj, jint index)
{
    char vendorName[MAX_STRING_LENGTH] = "Unknown Vendor";
    jstring jVendorName;

    TRACE0("Java_com_sun_media_sound_SimpleOutputDeviceProvider_nGetVendor.\n");

    /* $$kk: 06.03.99: need to implement */
	
    // $$jb: 11.15.99: vendorName is not a Pascal-type string
    //XPtoCstr(vendorName);
    jVendorName = (*e)->NewStringUTF(e, vendorName);

    TRACE0("Java_com_sun_media_sound_SimpleOutputDeviceProvider_nGetVendor completed.\n");

    return jVendorName;
}


JNIEXPORT jstring JNICALL
    Java_com_sun_media_sound_SimpleOutputDeviceProvider_nGetDescription(JNIEnv* e, jobject thisObj, jint index)
{
    char description[MAX_STRING_LENGTH] = "No details available";
    jstring jDescription;

    TRACE0("Java_com_sun_media_sound_SimpleOutputDeviceProvider_nGetDescription.\n");

    /* $$kk: 06.03.99: need to implement */
	
    // $$jb: 11.15.99: description is not a Pascal-type string
    //XPtoCstr(description);
    jDescription = (*e)->NewStringUTF(e, description);

    TRACE0("Java_com_sun_media_sound_SimpleOutputDeviceProvider_nGetDescription completed.\n");

    return jDescription;
}


JNIEXPORT jstring JNICALL
    Java_com_sun_media_sound_SimpleOutputDeviceProvider_nGetVersion(JNIEnv* e, jobject thisObj, jint index)
{
    char version[MAX_STRING_LENGTH] = "Unknown Version";
    jstring jVersion;

    TRACE0("Java_com_sun_media_sound_SimpleOutputDeviceProvider_nGetVersion.\n");

    /* $$kk: 06.03.99: need to implement */
	
    // $$jb: 11.15.99: version is not a Pascal-type string
    //XPtoCstr(version);
    jVersion = (*e)->NewStringUTF(e, version);

    TRACE0("Java_com_sun_media_sound_SimpleOutputDeviceProvider_nGetVersion completed.\n");

    return jVersion;
}

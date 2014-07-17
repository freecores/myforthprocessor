/*
 * @(#)SimpleOutputDevice.c	1.8 03/01/23
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

// SimpleOutputDevice includes
#include "com_sun_media_sound_SimpleOutputDevice.h"


#define NO_DEVICE					-1

// $$kk: 06.03.99: this is the index of the currently open device.
// if -1, no device is acquired.
int currentOutputDeviceIndex = NO_DEVICE;



// NATIVE METHODS


JNIEXPORT void JNICALL
    Java_com_sun_media_sound_SimpleOutputDevice_nOpen(JNIEnv* e, jobject thisObj, jint index, jfloat sampleRate, jint sampleSizeInBits, jint channels, jint bufferSize)
{
    OPErr           opErr;
		
    TRACE0("Java_com_sun_media_sound_SimpleOutputDevice_nOpen.\n");

    // if this output device is already open, just return
    if (index == currentOutputDeviceIndex) 
	{
	    TRACE0("Java_com_sun_media_sound_SimpleOutputDevice_nOpen: this device already open\n");
	    return;
	}


    // if another output device is open, fail
    if (currentOutputDeviceIndex != NO_DEVICE)  
	{
	    char *msg = "Another output device is already open\0";
	    TRACE0("Java_com_sun_media_sound_SimpleOutputDevice_nOpen: another output device is already open\n");
	    ThrowJavaMessageException(e, JAVA_SAMPLED_PACKAGE_NAME"/LineUnavailableException", msg);
	}

    // ok, try to open this capture device.  
    // first set the requested device to be the current one.  (this is not elegant!)

    GM_SetDeviceID(index, NULL);

    // possible return values: NO_ERR, ALREADY_RESUMED, DEVICE_UNAVAILABLE
    opErr = GM_ResumeGeneralSound((void*)e);

    // we don't treat this as an error 
    if (opErr == ALREADY_RESUMED)
	{
	    TRACE0("Java_com_sun_media_sound_SimpleOutputDevice_nOpen: GM_ResumeGeneralSound returned ALREADY_RESUMED.\n");
	    opErr = NO_ERR;
	}

    // if we failed, throw a LineUnavailableException
    if (opErr != NO_ERR)
	{
	    TRACE0("Java_com_sun_media_sound_SimpleOutputDevice_nOpen failed; throwing LineUnavailableException\n");
	    ThrowJavaOpErrException(e, JAVA_SAMPLED_PACKAGE_NAME"/LineUnavailableException", opErr);
	}

    // it worked!  set the current device index to reflect the newly acquired device
    currentOutputDeviceIndex = index;

    TRACE0("Java_com_sun_media_sound_SimpleOutputDevice_nOpen succeeded\n");
}


JNIEXPORT void JNICALL
    Java_com_sun_media_sound_SimpleOutputDevice_nClose(JNIEnv* e, jobject thisObj)
{

    TRACE0("Java_com_sun_media_sound_SimpleOutputDevice_nClose.\n");

    GM_PauseGeneralSound((void*)e);	// disconnect from hardware
    //GM_StopHardwareSoundManager((void*)e);	// disconnect from hardware
    currentOutputDeviceIndex = NO_DEVICE;

    TRACE0("Java_com_sun_media_sound_SimpleOutputDevice_nClose succeeded\n");
}


JNIEXPORT void JNICALL
    Java_com_sun_media_sound_SimpleOutputDevice_nStart(JNIEnv* e, jobject thisObj)
{
    TRACE0("Java_com_sun_media_sound_SimpleOutputDevice_nStart.\n");
}


JNIEXPORT void JNICALL
    Java_com_sun_media_sound_SimpleOutputDevice_nStop(JNIEnv* e, jobject thisObj)
{
    TRACE0("Java_com_sun_media_sound_SimpleOutputDevice_nStop.\n");
}


JNIEXPORT jboolean JNICALL
    Java_com_sun_media_sound_SimpleOutputDevice_nSupportsSampleRate(JNIEnv* e, jobject thisObj, jfloat sampleRate) 
{
    TRACE1("Java_com_sun_media_sound_SimpleOutputDevice_nSupportsSampleRate: %d.\n", sampleRate);

    //$$fb 2001-07-19: added 16000 and 32000 as part of fix for bug 4479441. TODO: real detection !

    if ( (sampleRate == 8000) || (sampleRate == 11025) || (sampleRate == 16000) || (sampleRate == 22050) || (sampleRate == 32000) || (sampleRate == 44100) || (sampleRate == 48000) ) 
	{
	    TRACE1("Java_com_sun_media_sound_SimpleOutputDevice_nSupportsSampleRate: %d returning TRUE.\n", sampleRate);
	    return (jboolean)TRUE;
	}

    TRACE1("Java_com_sun_media_sound_SimpleOutputDevice_nSupportsSampleRate: %d returning FALSE.\n", sampleRate);
    return (jboolean)FALSE;
}


JNIEXPORT jboolean JNICALL
    Java_com_sun_media_sound_SimpleOutputDevice_nSupportsSampleSizeInBits(JNIEnv* e, jobject thisObj, jint sampleSizeInBits) 
{
    TRACE1("Java_com_sun_media_sound_SimpleOutputDevice_nSupportsSampleSizeInBits: %d.\n", sampleSizeInBits);

    if (sampleSizeInBits == 16)
	{
	    TRACE2("Java_com_sun_media_sound_SimpleOutputDevice_nSupportsSampleSizeInBits: %d returning %d.\n", sampleSizeInBits, (XIs16BitSupported()));
	    return (jboolean)XIs16BitSupported();
	}

    if (sampleSizeInBits == 8)
	{
	    TRACE2("Java_com_sun_media_sound_SimpleOutputDevice_nSupportsSampleSizeInBits: %d returning %d.\n", sampleSizeInBits, (XIs8BitSupported()));
	    return (jboolean)XIs8BitSupported();
	}

    TRACE1("Java_com_sun_media_sound_SimpleOutputDevice_nSupportsSampleSizeInBits: %d returning FALSE.\n", sampleSizeInBits);
    return (jboolean)FALSE;
}


JNIEXPORT jboolean JNICALL
    Java_com_sun_media_sound_SimpleOutputDevice_nSupportsChannels(JNIEnv* e, jobject thisObj, jint channels) 
{

    if (channels == 1)
	return (jboolean)TRUE;

    if (channels == 2)
	return (jboolean)XIsStereoSupported();

    return (jboolean)FALSE;
}


JNIEXPORT jint JNICALL
    Java_com_sun_media_sound_SimpleOutputDevice_nGetNumPorts(JNIEnv* e, jobject thisObj)
{

    TRACE0("Java_com_sun_media_sound_SimpleOutputDevice_nGetNumPorts.\n");

    ERROR0("Java_com_sun_media_sound_SimpleOutputDevice_nGetNumPorts not implemented\n");

    TRACE0("Java_com_sun_media_sound_SimpleOutputDevice_nGetNumPorts succeeded\n");

    return (jint)0;
}


JNIEXPORT jstring JNICALL
    Java_com_sun_media_sound_SimpleOutputDevice_nGetPortName(JNIEnv* e, jobject thisObj, jint index)
{

    TRACE0("Java_com_sun_media_sound_SimpleOutputDevice_nGetPortName.\n");

    ERROR0("Java_com_sun_media_sound_SimpleOutputDevice_nGetPortName not implemented\n");

    TRACE0("Java_com_sun_media_sound_SimpleOutputDevice_nGetPortName succeeded\n");

    return (jstring)NULL;
}

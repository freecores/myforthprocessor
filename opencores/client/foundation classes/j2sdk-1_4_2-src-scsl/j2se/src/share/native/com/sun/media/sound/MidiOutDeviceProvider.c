/*
 * @(#)MidiOutDeviceProvider.c	1.10 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


// STANDARD includes


// JNI includes
#include <jni.h>

#include "engine/X_API.h"

// Platform MIDI includes
#include "PlatformMidi.h"

// UTILITY includes
#include "Utilities.h"

// SimpleInputDeviceProvider includes
#include "com_sun_media_sound_MidiOutDeviceProvider.h"

// $$kk: 03.29.99: need to figure out how long the name might be.
// for win32, is is MAXPNAMELEN = 32, as defined in mmsystem.h.
// what about solaris??
#define MAX_STRING_LENGTH 128


JNIEXPORT jint JNICALL
    Java_com_sun_media_sound_MidiOutDeviceProvider_nGetNumDevices(JNIEnv* e, jobject thisObj)
{
    INT32 numDevices = 0;
	
    TRACE0("Java_com_sun_media_sound_MidiOutDeviceProvider_nGetNumDevices.\n");

#if USE_PLATFORM_MIDI_OUT == TRUE

    numDevices = MIDI_OUT_GetNumDevices();

#endif // USE_PLATFORM_MIDI_OUT 

    TRACE1("Java_com_sun_media_sound_MidiOutDeviceProvider_nGetNumDevices returning %d.\n", numDevices);

    return (jint)numDevices;
}


JNIEXPORT jstring JNICALL
    Java_com_sun_media_sound_MidiOutDeviceProvider_nGetName(JNIEnv* e, jobject thisObj, jint index)
{
    char name[MAX_STRING_LENGTH] = "Unknown name";
    jstring jString = NULL;

    TRACE0("Java_com_sun_media_sound_MidiOutDeviceProvider_nGetName.\n");

#if USE_PLATFORM_MIDI_OUT == TRUE

    MIDI_OUT_GetDeviceName((INT32)index, name, (UINT32)MAX_STRING_LENGTH);

#endif // USE_PLATFORM_MIDI_OUT 

    // $$jb: 11.15.99: As currently implemented, MIDI_OUT_GetDeviceName() does not
    // return a Pascal-type string.  Furthermore, name is not a Pascal-type string.
    // XPtoCstr(name);
    jString = (*e)->NewStringUTF(e, name);

    TRACE0("Java_com_sun_media_sound_MidiOutDeviceProvider_nGetName completed.\n");

    return (jString);
}


JNIEXPORT jstring JNICALL
    Java_com_sun_media_sound_MidiOutDeviceProvider_nGetVendor(JNIEnv* e, jobject thisObj, jint index)
{
    char name[MAX_STRING_LENGTH] = "Unknown Vendor";
    jstring jString = NULL;

    TRACE0("Java_com_sun_media_sound_MidiOutDeviceProvider_nGetVendor.\n");

#if USE_PLATFORM_MIDI_OUT == TRUE

    MIDI_OUT_GetDeviceVendor((INT32)index, name, (UINT32)MAX_STRING_LENGTH);

#endif // USE_PLATFORM_MIDI_OUT 

    // $$jb: 11.15.99: As currently implemented, MIDI_OUT_GetDeviceVendor() does not
    // return a Pascal-type string.  Furthermore, name is not a Pascal-type string.
    // XPtoCstr(name);
    jString = (*e)->NewStringUTF(e, name);

    TRACE0("Java_com_sun_media_sound_MidiOutDeviceProvider_nGetVendor completed.\n");

    return (jString);
}


JNIEXPORT jstring JNICALL
    Java_com_sun_media_sound_MidiOutDeviceProvider_nGetDescription(JNIEnv* e, jobject thisObj, jint index)
{
    char name[MAX_STRING_LENGTH] = "No details available";
    jstring jString = NULL;

    TRACE0("Java_com_sun_media_sound_MidiOutDeviceProvider_nGetDescription.\n");

#if USE_PLATFORM_MIDI_OUT == TRUE

    MIDI_OUT_GetDeviceDescription((INT32)index, name, (UINT32)MAX_STRING_LENGTH);

#endif // USE_PLATFORM_MIDI_OUT 

    // $$jb: 11.15.99: As currently implemented, MIDI_OUT_GetDeviceDescription() does not
    // return a Pascal-type string.  Furthermore, name is not a Pascal-type string.
    // XPtoCstr(name);
    jString = (*e)->NewStringUTF(e, name);

    TRACE0("Java_com_sun_media_sound_MidiOutDeviceProvider_nGetDescription completed.\n");

    return (jString);
}


JNIEXPORT jstring JNICALL
    Java_com_sun_media_sound_MidiOutDeviceProvider_nGetVersion(JNIEnv* e, jobject thisObj, jint index)
{
    char name[MAX_STRING_LENGTH] = "Unknown Version";
    jstring jString = NULL;

    TRACE0("Java_com_sun_media_sound_MidiOutDeviceProvider_nGetVersion.\n");

#if USE_PLATFORM_MIDI_OUT == TRUE

    MIDI_OUT_GetDeviceVersion((INT32)index, name, (UINT32)MAX_STRING_LENGTH);

#endif // USE_PLATFORM_MIDI_OUT 

    // $$jb: 11.15.99: As currently implemented, MIDI_OUT_GetDeviceVersion() does not
    // return a Pascal-type string.  Furthermore, name is not a Pascal-type string.
    // XPtoCstr(name);
    jString = (*e)->NewStringUTF(e, name);

    TRACE0("Java_com_sun_media_sound_MidiOutDeviceProvider_nGetVersion completed.\n");

    return (jString);
}

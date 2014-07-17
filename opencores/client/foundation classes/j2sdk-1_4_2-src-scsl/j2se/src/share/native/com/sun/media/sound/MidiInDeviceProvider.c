/*
 * @(#)MidiInDeviceProvider.c	1.10 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


// STANDARD includes


// JNI includes
#include <jni.h>

// ENGINE includes
// $$kk: 06.25.99: i don't want this, but i am using it for XPtoCstr
#include "engine/X_API.h"

// Platform MIDI includes
#include "PlatformMidi.h"

// UTILITY includes
#include "Utilities.h"

// SimpleInputDeviceProvider includes
#include "com_sun_media_sound_MidiInDeviceProvider.h"

// $$kk: 03.29.99: need to figure out how long the name might be.
// for win32, is is MAXPNAMELEN = 32, as defined in mmsystem.h.
// what about solaris??
#define MAX_STRING_LENGTH 128


JNIEXPORT jint JNICALL
    Java_com_sun_media_sound_MidiInDeviceProvider_nGetNumDevices(JNIEnv* e, jobject thisObj)
{
    INT32 numDevices = 0;
	
    TRACE0("Java_com_sun_media_sound_MidiInDeviceProvider_nGetNumDevices.\n");

#if USE_PLATFORM_MIDI_IN == TRUE

    numDevices = MIDI_IN_GetNumDevices();

#endif // USE_PLATFORM_MIDI_IN 

    TRACE1("Java_com_sun_media_sound_MidiInDeviceProvider_nGetNumDevices returning %d.\n", numDevices);

    return (jint)numDevices;
}


JNIEXPORT jstring JNICALL
    Java_com_sun_media_sound_MidiInDeviceProvider_nGetName(JNIEnv* e, jobject thisObj, jint index)
{
    char name[MAX_STRING_LENGTH] = "Unknown name";
    jstring jString = NULL;

    TRACE0("Java_com_sun_media_sound_MidiInDeviceProvider_nGetName.\n");

#if USE_PLATFORM_MIDI_IN == TRUE

    MIDI_IN_GetDeviceName((INT32)index, name, (UINT32)MAX_STRING_LENGTH);

#endif // USE_PLATFORM_MIDI_IN 

    // $$jb: 11.15.99: As currently implemented, MIDI_IN_GetDeviceName() does
    // not return a Pascal-type string.  Furthermore, name is not a Pascal-type
    // string.
    // XPtoCstr(name);
    jString = (*e)->NewStringUTF(e, name);

    TRACE0("Java_com_sun_media_sound_MidiInDeviceProvider_nGetName completed.\n");

    return (jString);
}


JNIEXPORT jstring JNICALL
    Java_com_sun_media_sound_MidiInDeviceProvider_nGetVendor(JNIEnv* e, jobject thisObj, jint index)
{
    char name[MAX_STRING_LENGTH] = "Unknown Vendor";
    jstring jString = NULL;

    TRACE0("Java_com_sun_media_sound_MidiInDeviceProvider_nGetVendor.\n");

#if USE_PLATFORM_MIDI_IN == TRUE

    MIDI_IN_GetDeviceVendor((INT32)index, name, (UINT32)MAX_STRING_LENGTH);

#endif // USE_PLATFORM_MIDI_IN 

    // $$jb: 11.15.99: As currently implemented, MIDI_IN_GetDeviceVendor() does
    // not return a Pascal-type string.  Furthermore, name is not a Pascal-type
    // string.
    // XPtoCstr(name);
    jString = (*e)->NewStringUTF(e, name);

    TRACE0("Java_com_sun_media_sound_MidiInDeviceProvider_nGetVendor completed.\n");

    return (jString);
}


JNIEXPORT jstring JNICALL
    Java_com_sun_media_sound_MidiInDeviceProvider_nGetDescription(JNIEnv* e, jobject thisObj, jint index)
{
    char name[MAX_STRING_LENGTH] = "No details available";
    jstring jString = NULL;

    TRACE0("Java_com_sun_media_sound_MidiInDeviceProvider_nGetDescription.\n");

#if USE_PLATFORM_MIDI_IN == TRUE

    MIDI_IN_GetDeviceDescription((INT32)index, name, (UINT32)MAX_STRING_LENGTH);

#endif // USE_PLATFORM_MIDI_IN 

    // $$jb: 11.15.99: As currently implemented, MIDI_IN_GetDeviceDescription() does
    // not return a Pascal-type string.  Furthermore, name is not a Pascal-type
    // string.
    // XPtoCstr(name);
    jString = (*e)->NewStringUTF(e, name);
	
    TRACE0("Java_com_sun_media_sound_MidiInDeviceProvider_nGetDescription completed.\n");

    return (jString);
}


JNIEXPORT jstring JNICALL
    Java_com_sun_media_sound_MidiInDeviceProvider_nGetVersion(JNIEnv* e, jobject thisObj, jint index)
{
    char name[MAX_STRING_LENGTH] = "Unknown Version";
    jstring jString = NULL;

    TRACE0("Java_com_sun_media_sound_MidiInDeviceProvider_nGetVersion.\n");

#if USE_PLATFORM_MIDI_IN == TRUE

    MIDI_IN_GetDeviceVersion((INT32)index, name, (UINT32)MAX_STRING_LENGTH);

#endif // USE_PLATFORM_MIDI_IN 

    // $$jb: 11.15.99: As currently implemented, MIDI_IN_GetDeviceVersion() does
    // not return a Pascal-type string.  Furthermore, name is not a Pascal-type
    // string.
    // XPtoCstr(name);
    jString = (*e)->NewStringUTF(e, name);

    TRACE0("Java_com_sun_media_sound_MidiInDeviceProvider_nGetVersion completed.\n");

    return (jString);
}

/*
 * @(#)MidiOutDevice.c	1.13 03/01/23
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
#include "com_sun_media_sound_MidiOutDevice.h"


// NATIVE METHODS


JNIEXPORT jlong JNICALL
    Java_com_sun_media_sound_MidiOutDevice_nOpen(JNIEnv* e, jobject thisObj, jint index) {
    void *deviceHandle = NULL;

    TRACE1("Java_com_sun_media_sound_MidiOutDevice_nOpen: index: %d\n", index);

#if USE_PLATFORM_MIDI_OUT == TRUE
    deviceHandle = MIDI_OUT_OpenDevice((INT32) index);
#endif

    // if we didn't get a valid handle, throw a MidiUnavailableException
    // $$kk: 06.24.99: should be getting more information here!
    if (!deviceHandle) {
	char *msg = "Failed to open the device.\0";
	ERROR0("Java_com_sun_media_sound_MidiOutDevice_nOpen: Failed to open the device\n");
	ThrowJavaMessageException(e, JAVA_MIDI_PACKAGE_NAME"/MidiUnavailableException", msg);
    }

    TRACE0("Java_com_sun_media_sound_MidiOutDevice_nOpen succeeded\n");
    return (jlong) (INT_PTR) deviceHandle;
}


JNIEXPORT void JNICALL
    Java_com_sun_media_sound_MidiOutDevice_nClose(JNIEnv* e, jobject thisObj, jlong deviceHandle) {
    TRACE0("Java_com_sun_media_sound_MidiOutDevice_nClose.\n");

#if USE_PLATFORM_MIDI_OUT == TRUE
    MIDI_OUT_CloseDevice((void *)deviceHandle);
#endif

    TRACE0("Java_com_sun_media_sound_MidiOutDevice_nClose succeeded\n");
}


JNIEXPORT void JNICALL
    Java_com_sun_media_sound_MidiOutDevice_nSendShortMessage(JNIEnv* e, jobject thisObj, jlong deviceHandle,
							     jint packedMsg, jlong timeStamp) {
    TRACE0("Java_com_sun_media_sound_MidiOutDevice_nSendShortMessage.\n");

#if USE_PLATFORM_MIDI_OUT == TRUE
    MIDI_OUT_SendShortMessage((MidiDeviceHandle*) deviceHandle,
			      (UINT32) packedMsg, (UINT32)timeStamp);
#endif

    TRACE0("Java_com_sun_media_sound_MidiOutDevice_nSendShortMessage succeeded\n");
}

JNIEXPORT void JNICALL
    Java_com_sun_media_sound_MidiOutDevice_nSendLongMessage(JNIEnv* e, jobject thisObj, jlong deviceHandle,
							    jbyteArray jData, jint size, jlong timeStamp) {
#if USE_PLATFORM_MIDI_OUT == TRUE
    UBYTE* data;
#endif

    TRACE0("Java_com_sun_media_sound_MidiOutDevice_nSendLongMessage.\n");

#if USE_PLATFORM_MIDI_OUT == TRUE
    data = (UBYTE*) ((*e)->GetByteArrayElements(e, jData, NULL));
    if (!data) {
	ERROR0("MidiOutDevice: Java_com_sun_media_sound_MidiOutDevice_nSendLongMessage: could not get array elements\n");
	return;
    }
    MIDI_OUT_SendLongMessage((MidiDeviceHandle*) deviceHandle, data,
			     (UINT32) size, (UINT32)timeStamp);
    // release the byte array
    (*e)->ReleaseByteArrayElements(e, jData, data, JNI_ABORT);
#endif

    TRACE0("Java_com_sun_media_sound_MidiOutDevice_nSendLongMessage succeeded\n");
}

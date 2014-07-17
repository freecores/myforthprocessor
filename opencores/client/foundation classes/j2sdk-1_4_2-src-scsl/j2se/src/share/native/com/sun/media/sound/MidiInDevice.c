/*
 * @(#)MidiInDevice.c	1.15 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*****************************************************************************/
/*
**	MidiInDevice.c
**
**	Overview:
**	Native functions for interfacing Java with the native implementation
**      of PlatformMidi.h's functions.
**	This implementation does not interface with the HAE engine at all.
**
**	History	-
**	2002-04-03	$$fb Completely remodeled architecture.
*/
/*****************************************************************************/

//#define USE_ERROR
//#define USE_TRACE


#include <jni.h>
#include "engine/X_API.h"
#include "PlatformMidi.h"
#include "com_sun_media_sound_MidiInDevice.h"


JNIEXPORT jlong JNICALL
    Java_com_sun_media_sound_MidiInDevice_nOpen(JNIEnv* e, jobject thisObj, jint index) {
    MidiDeviceHandle* deviceHandle = NULL;

    TRACE1("Java_com_sun_media_sound_MidiInDevice_nOpen: index: %d\n", index);

#if USE_PLATFORM_MIDI_IN == TRUE
    deviceHandle = MIDI_IN_OpenDevice((INT32) index);
#endif

    // if we didn't get a valid handle, throw a MidiUnavailableException
    // $$kk: 06.24.99: should be getting more information here!
    if ( !deviceHandle ) {
	char *msg = "Failed to open the device.\0";

	ERROR0("Java_com_sun_media_sound_MidiInDevice_nOpen: Failed to open the device\n");
	ThrowJavaMessageException(e, JAVA_MIDI_PACKAGE_NAME"/MidiUnavailableException", msg);
    }
    TRACE0("Java_com_sun_media_sound_MidiInDevice_nOpen succeeded\n");
    return (jlong) (UINT_PTR) deviceHandle;
}


JNIEXPORT void JNICALL
    Java_com_sun_media_sound_MidiInDevice_nClose(JNIEnv* e, jobject thisObj, jlong deviceHandle) {
    TRACE0("Java_com_sun_media_sound_MidiInDevice_nClose.\n");

#if USE_PLATFORM_MIDI_IN == TRUE
    MIDI_IN_CloseDevice((MidiDeviceHandle*) (UINT_PTR) deviceHandle);
#endif

    TRACE0("Java_com_sun_media_sound_MidiInDevice_nClose succeeded\n");
}


JNIEXPORT void JNICALL
    Java_com_sun_media_sound_MidiInDevice_nStart(JNIEnv* e, jobject thisObj, jlong deviceHandle) {
    jboolean ok = FALSE;

    TRACE0("Java_com_sun_media_sound_MidiInDevice_nStart.\n");

#if USE_PLATFORM_MIDI_IN == TRUE
    ok = MIDI_IN_StartDevice((MidiDeviceHandle*) (UINT_PTR) deviceHandle);
#endif

    // if we didn't get a valid handle, throw a MidiUnavailableException
    // $$kk: 06.24.99: should be getting more information here!
    if ( !ok ) {
	char *msg = "Failed to start the device.\0";
	ERROR0("Java_com_sun_media_sound_MidiInDevice_nStart: Failed to start the device\n");
	ThrowJavaMessageException(e, JAVA_MIDI_PACKAGE_NAME"/MidiUnavailableException", msg);
    } else {
	TRACE0("Java_com_sun_media_sound_MidiInDevice_nStart succeeded\n");
    }
}


JNIEXPORT void JNICALL
    Java_com_sun_media_sound_MidiInDevice_nStop(JNIEnv* e, jobject thisObj, jlong deviceHandle) {
    TRACE0("Java_com_sun_media_sound_MidiInDevice_nStop.\n");

#if USE_PLATFORM_MIDI_IN == TRUE
    // stop the device and remove all queued events for this device handle
    MIDI_IN_StopDevice((MidiDeviceHandle*) (UINT_PTR) deviceHandle);
#endif

    TRACE0("Java_com_sun_media_sound_MidiInDevice_nStop succeeded\n");
}


JNIEXPORT void JNICALL
    Java_com_sun_media_sound_MidiInDevice_nGetMessages(JNIEnv* e, jobject thisObj, jlong deviceHandle) {
#if USE_PLATFORM_MIDI_IN == TRUE
    MidiMessage* pMessage;
    jclass javaClass = NULL;
    jmethodID callbackShortMessageMethodID = NULL;
    jmethodID callbackLongMessageMethodID = NULL;
#endif

    TRACE0("> Java_com_sun_media_sound_MidiInDevice_nGetMessages\n");

#if USE_PLATFORM_MIDI_IN == TRUE
    while ((pMessage = MIDI_IN_GetMessage((MidiDeviceHandle*) (UINT_PTR) deviceHandle))) {
	if ((javaClass == NULL) || (callbackShortMessageMethodID == NULL)) {
	    if (!thisObj) {
		ERROR0("MidiInDevice: Java_com_sun_media_sound_MidiInDevice_nGetMessages: thisObj is NULL\n");
		return;
	    }

	    if (javaClass == NULL) {
		javaClass = (*e)->GetObjectClass(e, thisObj);
		if (javaClass == NULL) {
		    ERROR0("MidiInDevice: Java_com_sun_media_sound_MidiInDevice_nGetMessages: javaClass is NULL\n");
		    return;
		}
	    }

	    if (callbackShortMessageMethodID == NULL) {
		// save the callbackShortMessage callback method id.
		// this is valid as long as the class is not unloaded.
		callbackShortMessageMethodID = (*e)->GetMethodID(e, javaClass, "callbackShortMessage", "(IJ)V");
		if (callbackShortMessageMethodID == 0) {
		    ERROR0("MidiInDevice: Java_com_sun_media_sound_MidiInDevice_nGetMessages: callbackShortMessageMethodID is 0\n");
		    return;
		}
	    }
	    if (callbackLongMessageMethodID == NULL) {
		// save the callbackLongMessage callback method id.
		// this is valid as long as the class is not unloaded.
		callbackLongMessageMethodID = (*e)->GetMethodID(e, javaClass, "callbackLongMessage", "([BJ)V");
		if (callbackLongMessageMethodID == 0) {
		    ERROR0("MidiInDevice: Java_com_sun_media_sound_MidiInDevice_nGetMessages: callbackLongMessageMethodID is 0\n");
		    return;
		}
	    }
	}

	switch ((int)pMessage->type) {
	case SHORT_MESSAGE: {
	    jint msg = (jint)pMessage->data.s.packedMsg;
	    jlong ts = (jlong)pMessage->timestamp;
	    TRACE0("Java_com_sun_media_sound_MidiInDevice_nGetMessages: got SHORT_MESSAGE\n");
	    // now we can put this message object back in the queue
	    MIDI_IN_ReleaseMessage((MidiDeviceHandle*) (UINT_PTR) deviceHandle, pMessage);
	    // and notify Java space
	    (*e)->CallVoidMethod(e, thisObj, callbackShortMessageMethodID, msg, ts);
	    break;
	}

	case LONG_MESSAGE: {
	    jlong ts = (jlong)pMessage->timestamp;
	    jbyteArray jData;
	    UBYTE* data;
	    int isSXCont = 0;
	    TRACE0("Java_com_sun_media_sound_MidiInDevice_nGetMessages: got LONG_MESSAGE\n");
	    if ((*(pMessage->data.l.data) != 0xF0)
	    	&& (*(pMessage->data.l.data) != 0xF7)) {
	    	// this is a continued sys ex message
	    	// need to prepend 0xF7
	    	isSXCont = 1;
	    }
	    jData = (*e)->NewByteArray(e, pMessage->data.l.size + isSXCont);
	    if (!jData) {
		ERROR0("Java_com_sun_media_sound_MidiInDevice_nGetMessages: cannot create long byte array.\n");
		break;
	    }
	    data = (UBYTE*) ((*e)->GetByteArrayElements(e, jData, NULL));
	    if (!data) {
		ERROR0("MidiInDevice: Java_com_sun_media_sound_MidiInDevice_nGetMessages: array data is NULL\n");
		break;
	    }
	    // finally copy the long message
	    memcpy(data + isSXCont, pMessage->data.l.data, pMessage->data.l.size);

	    // now we can put this message object back in the queue
	    MIDI_IN_ReleaseMessage((MidiDeviceHandle*) (UINT_PTR) deviceHandle, pMessage);

	    // if this is a patched continued sys ex message, prepend 0xF7
	    if (isSXCont) {
	    	*data = 0xF7;
	    }

	    // commit the byte array
	    (*e)->ReleaseByteArrayElements(e, jData, data, 0);

	    (*e)->CallVoidMethod(e, thisObj, callbackLongMessageMethodID, jData, ts);
	    // release local reference to array: not needed anymore.
	    (*e)->DeleteLocalRef(e, jData);
	    break;
	}

	default:
	    // put this message object back in the queue
	    MIDI_IN_ReleaseMessage((MidiDeviceHandle*) (UINT_PTR) deviceHandle, pMessage);
	    ERROR1("Java_com_sun_media_sound_MidiInDevice_nGetMessages: got unsupported message, type %d\n", pMessage->type);
	    break;
	} // switch
    }

#endif // USE_PLATFORM_MIDI_IN

    TRACE0("< Java_com_sun_media_sound_MidiInDevice_nGetMessages returning\n");
}


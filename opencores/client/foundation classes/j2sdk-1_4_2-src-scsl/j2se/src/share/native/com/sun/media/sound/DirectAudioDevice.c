/*
 * @(#)DirectAudioDevice.c	1.2 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//#define USE_TRACE
#define USE_ERROR

// STANDARD includes

// JNI includes
#include <jni.h>

// for type definitions
#include "engine/X_API.h"

// DirectAudio includes
#include "DirectAudio.h"

// UTILITY includes
#include "Utilities.h"

// DirectAudioDevice includes
#include "com_sun_media_sound_DirectAudioDevice.h"

//////////////////////////////////////////// Utility function /////////////////////////////////////////////

/*
 * in-place conversion of this buffer:
 * conversion size=1 -> each byte is converted from signed to unsigned or vice versa
 * conversion size=2,3,4: the order of bytes in a sample is reversed (endianness)
 * for sign conversion of a 24-bit sample stored in 32bits, 4 should be passed
 * as conversionSize
 */
void handleSignEndianConversion(char* data, int byteSize, int conversionSize) {
    TRACE1("conversion with size %d\n", conversionSize);
    switch (conversionSize) {
    case 1: {
	while (byteSize > 0) {
	    *data += 128; // use wrap-around
	    byteSize--;
	    data++;
	}
	break;
    }
    case 2: {
	UINT16* sample = (UINT16*) data;
	byteSize = byteSize / 2;
	while (byteSize > 0) {
	    *sample = ((*sample & 0xFF) << 8) | ((*sample & 0xFF00) >> 8);
	    byteSize--;
	    sample++;
	}
	break;
    }
    case 3: {
	char h;
	byteSize = byteSize / 3;
	while (byteSize > 0) {
	    h = *data;
	    *data = data[2];
	    data++; data++;
	    *data = h;
	    byteSize--;
	    data++;
	}
	break;
    }
    case 4: {
	UINT32* sample = (UINT32*) data;
	byteSize = byteSize / 4;
	while (byteSize > 0) {
	    *sample = ((*sample & 0xFF) << 24) 
		| ((*sample & 0xFF00) << 8)
		| ((*sample & 0xFF0000) >> 8)
		| ((*sample & 0xFF000000) >> 24);
	    byteSize--;
	    data++;
	}
	break;
    }
    default: 
	ERROR1("DirectAudioDevice.c: wrong conversionSize %d!\n", conversionSize);
    }
}

//////////////////////////////////////////// DirectAudioDevice ////////////////////////////////////////////

/* ************************************** native control creation support ********************* */

// contains all the needed references so that the platform dependent code can call JNI wrapper functions
typedef struct tag_AddFormatCreator {
    // general JNI variables
    JNIEnv *env;
    // the vector to be filled with the formats
    jobject vector;
    // the class containing the addFormat method
    jclass directAudioDeviceClass;
    // the method to be called to add the format
    jmethodID addFormat; // signature (Ljava/util/Vector;IIFIBB)V
} AddFormatCreator;

void DAUDIO_AddAudioFormat(void* creatorV, int significantBits, int frameSizeInBytes,
			   int channels, float sampleRate,
			   int encoding, int isSigned,
			   int bigEndian) {
    AddFormatCreator* creator = (AddFormatCreator*) creatorV;
    if (frameSizeInBytes <= 0) {
	frameSizeInBytes = ((significantBits + 7) / 8) * channels;
    }
    TRACE4("AddAudioFormat with sigBits=%d bits, frameSize=%d bytes, channels=%d, sampleRate=%d ", 
	   significantBits, frameSizeInBytes, channels, (int) sampleRate);
    TRACE3("enc=%d, signed=%d, bigEndian=%d\n", encoding, isSigned, bigEndian);
    (*creator->env)->CallStaticVoidMethod(creator->env, creator->directAudioDeviceClass, 
					  creator->addFormat, creator->vector, significantBits, frameSizeInBytes,
					  channels, sampleRate, encoding, isSigned, bigEndian);
}

////////////////////////////////////// JNI /////////////////////////////////////////////////////////////////////

/*
 * Class:     com_sun_media_sound_DirectAudioDevice
 * Method:    nGetFormats
 * Signature: (IIZLjava/util/Vector;)V
 */
JNIEXPORT void JNICALL Java_com_sun_media_sound_DirectAudioDevice_nGetFormats
  (JNIEnv *env, jclass clazz, jint mixerIndex, jint deviceID, jboolean isSource, jobject formats) {
    
#if USE_DAUDIO == TRUE
    AddFormatCreator creator;
    creator.env = env;
    creator.vector = formats;
    creator.directAudioDeviceClass = clazz;
    creator.addFormat = (*env)->GetStaticMethodID(env, clazz, "addFormat",
						  "(Ljava/util/Vector;IIIFIZZ)V");
    if (creator.addFormat == NULL) {
	ERROR0("Could not get method ID for addFormat!\n");
    } else {
	DAUDIO_GetFormats((INT32) mixerIndex, (INT32) deviceID, (int) isSource, &creator);
    }
#endif
}


/*
 * Class:     com_sun_media_sound_DirectAudioDevice
 * Method:    nOpen
 * Signature: (IIZIFIIZZI)J
 */
JNIEXPORT jlong JNICALL Java_com_sun_media_sound_DirectAudioDevice_nOpen
  (JNIEnv* env, jclass clazz, jint mixerIndex, jint deviceID, jboolean isSource, 
   jint encoding, jfloat sampleRate, jint sampleSizeInBits, jint frameSize, jint channels, 
   jboolean isSigned, jboolean isBigendian, jint bufferSizeInBytes) {

    void* ret = NULL;
#if USE_DAUDIO == TRUE
    ret = DAUDIO_Open((int) mixerIndex, (INT32) deviceID, (int) isSource, 
		      (int) encoding, (float) sampleRate, (int) sampleSizeInBits, 
		      (int) frameSize, (int) channels, 
		      (int) isSigned, (int) isBigendian, (int) bufferSizeInBytes);
#endif
    return (jlong) (UINT_PTR) ret;
}

/*
 * Class:     com_sun_media_sound_DirectAudioDevice
 * Method:    nStart
 * Signature: (JZ)V
 */
JNIEXPORT void JNICALL Java_com_sun_media_sound_DirectAudioDevice_nStart
  (JNIEnv* env, jclass clazz, jlong id, jboolean isSource) {
#if USE_DAUDIO == TRUE
    if (id != 0) {
	DAUDIO_Start((void*) (UINT_PTR) id, (int) isSource);
    }
#endif
}


/*
 * Class:     com_sun_media_sound_DirectAudioDevice
 * Method:    nStop
 * Signature: (JZ)V
 */
JNIEXPORT void JNICALL Java_com_sun_media_sound_DirectAudioDevice_nStop
  (JNIEnv* env, jclass clazz, jlong id, jboolean isSource) {
#if USE_DAUDIO == TRUE
    if (id != 0) {
	DAUDIO_Stop((void*) (UINT_PTR) id, (int) isSource);
    }
#endif
}


/*
 * Class:     com_sun_media_sound_DirectAudioDevice
 * Method:    nClose
 * Signature: (JZ)V
 */
JNIEXPORT void JNICALL Java_com_sun_media_sound_DirectAudioDevice_nClose
  (JNIEnv* env, jclass clazz, jlong id, jboolean isSource) {
#if USE_DAUDIO == TRUE
    if (id != 0) {
	DAUDIO_Close((void*) (UINT_PTR) id, (int) isSource);
    }
#endif
}

/*
 * Class:     com_sun_media_sound_DirectAudioDevice
 * Method:    nWrite
 * Signature: (J[BII)I
 */
JNIEXPORT jint JNICALL Java_com_sun_media_sound_DirectAudioDevice_nWrite
  (JNIEnv *env, jclass clazz, jlong id, jbyteArray jData, jint offset, jint len, jint conversionSize) {
    int ret = -1;
#if USE_DAUDIO == TRUE
    char* data;
    char* dataOffset;

    /* a little sanity */
    if (offset < 0 || len < 0) {
	ERROR2("nWrite: wrong parameters: offset=%d, len=%d\n", offset, len);
	return ret;
    }
    if (id != 0) {
	data = (char*) ((*env)->GetByteArrayElements(env, jData, NULL));
	dataOffset = data;
	dataOffset += (int) offset;
	if (conversionSize > 0) {
	    handleSignEndianConversion(dataOffset, (int) len, (int) conversionSize);
	}
	
	ret = DAUDIO_Write((void*) (UINT_PTR) id, dataOffset, (int) len);

	// establish original state. Does JNI_ABORT guarantee non-commitment ?
	//if (conversionSize > 0) {
	//    handleSignEndianConversion(dataOffset, (int) len, (int) conversionSize);
	//}
	// do not commit the native array
	(*env)->ReleaseByteArrayElements(env, jData, data, JNI_ABORT);
    }
#endif
    return (jint) ret;
}

/*
 * Class:     com_sun_media_sound_DirectAudioDevice
 * Method:    nRead
 * Signature: (J[BII)I
 */
JNIEXPORT jint JNICALL Java_com_sun_media_sound_DirectAudioDevice_nRead
  (JNIEnv* env, jclass clazz, jlong id, jbyteArray jData, jint offset, jint len, jint conversionSize) {
    int ret = -1;
#if USE_DAUDIO == TRUE
    char* data;
    char* dataOffset;

    /* a little sanity */
    if (offset < 0 || len < 0) {
	ERROR2("nRead: wrong parameters: offset=%d, len=%d\n", offset, len);
	return ret;
    }
    if (id != 0) {
	data = (char*) ((*env)->GetByteArrayElements(env, jData, NULL)); 
	dataOffset = data;
	dataOffset += (int) offset;
	ret = DAUDIO_Read((void*) (UINT_PTR) id, dataOffset, (int) len);
	if (conversionSize > 0) {
	    handleSignEndianConversion(dataOffset, (int) len, (int) conversionSize);
	}
	// commit the native array
	(*env)->ReleaseByteArrayElements(env, jData, data, 0);
    }
#endif
    return (jint) ret;
}

/*
 * Class:     com_sun_media_sound_DirectAudioDevice
 * Method:    nGetBufferSize
 * Signature: (JZ)I
 */
JNIEXPORT jint JNICALL Java_com_sun_media_sound_DirectAudioDevice_nGetBufferSize
  (JNIEnv* env, jclass clazz, jlong id, jboolean isSource) {
    int ret = -1;
#if USE_DAUDIO == TRUE
    if (id != 0) {
	ret = DAUDIO_GetBufferSize((void*) (UINT_PTR) id, (int) isSource);
    }
#endif
    return (jint) ret;
}


/*
 * Class:     com_sun_media_sound_DirectAudioDevice
 * Method:    nIsStillDraining
 * Signature: (JZ)Z
 */
JNIEXPORT jboolean JNICALL Java_com_sun_media_sound_DirectAudioDevice_nIsStillDraining
  (JNIEnv* env, jclass clazz, jlong id, jboolean isSource) {
    int ret = FALSE;
#if USE_DAUDIO == TRUE
    if (id != 0) {
	ret = DAUDIO_StillDraining((void*) (UINT_PTR) id, (int) isSource)?TRUE:FALSE;
    }
#endif
    return (jboolean) ret;
}


/*
 * Class:     com_sun_media_sound_DirectAudioDevice
 * Method:    nFlush
 * Signature: (JZ)V
 */
JNIEXPORT void JNICALL Java_com_sun_media_sound_DirectAudioDevice_nFlush
  (JNIEnv* env, jclass clazz, jlong id, jboolean isSource) {
#if USE_DAUDIO == TRUE
    if (id != 0) {
	DAUDIO_Flush((void*) (UINT_PTR) id, (int) isSource);
    }
#endif
}


/*
 * Class:     com_sun_media_sound_DirectAudioDevice
 * Method:    nAvailable
 * Signature: (JZ)I
 */
JNIEXPORT jint JNICALL Java_com_sun_media_sound_DirectAudioDevice_nAvailable
  (JNIEnv* env, jclass clazz, jlong id, jboolean isSource) {
    int ret = -1;
#if USE_DAUDIO == TRUE
    if (id != 0) {
	ret = DAUDIO_GetAvailable((void*) (UINT_PTR) id, (int) isSource);
    }
#endif
    return (jint) ret;
}


/*
 * Class:     com_sun_media_sound_DirectAudioDevice
 * Method:    nGetBytePosition
 * Signature: (JZJ)J
 */
JNIEXPORT jlong JNICALL Java_com_sun_media_sound_DirectAudioDevice_nGetBytePosition
    (JNIEnv* env, jclass clazz, jlong id, jboolean isSource, jlong javaBytePos) {
    INT64 ret = (INT64) javaBytePos;
#if USE_DAUDIO == TRUE
    if (id != 0) {
	ret = DAUDIO_GetBytePosition((void*) (UINT_PTR) id, (int) isSource, (INT64) javaBytePos);
    }
#endif
    return (jlong) ret;
}

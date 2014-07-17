/*
 * @(#)MixerSynth.c	1.13 03/01/23
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
#include "engine/X_Formats.h" // for midi structures


// UTILITY includes
#include "Utilities.h"

// MixerSynth includes
#include "com_sun_media_sound_MixerSynth.h"


// MIDI SYNTHESIZER MANIPULATIONS


JNIEXPORT jlong JNICALL
    Java_com_sun_media_sound_MixerSynth_nCreateSynthesizer(JNIEnv* e, jobject thisObj) 
{

    GM_Song			*pSong = NULL;

    // $$kk: 03.23.98: GLOBAL REF!  this has definite memory leak potential; need to make sure
    // to release all global refs....
    jobject globalSynthesizerObj = (*e)->NewGlobalRef(e, thisObj);

    // possible error values: ????

    TRACE0("Java_com_sun_media_sound_MixerSynth_nCreateSynthesizer.\n");

    pSong = GM_CreateLiveSong((void *)e, (XShortResourceID)getMidiSongCount());

    if (!pSong)
	{
	    ERROR0("FAILED TO CREATE MIDI DIRECT: pSong is NULL\n");
	    return (jint)0;
	}
		
    // set the global java object reference as the user reference
    pSong->userReference = (void *)globalSynthesizerObj;
	
    TRACE0("Java_com_sun_media_sound_MixerSynth_nCreateSynthesizer.completed\n");

    return (jlong) (INT_PTR) pSong;
}


JNIEXPORT void JNICALL
    Java_com_sun_media_sound_MixerSynth_nDestroySynthesizer(JNIEnv* e, jobject thisObj, jlong id) 
{

    GM_Song			*pSong = (GM_Song *) (INT_PTR) id;

    TRACE0("Java_com_sun_media_sound_MixerSynth_nDestroySynthesizer.\n");

    if (pSong) {

	GM_KillSongNotes(pSong);
	pSong->disposeSongDataWhenDone = TRUE;   // free our midi pointer
	GM_FreeSong((void *)e, pSong);

    } else {

	ERROR0("pSong is NULL\n");
    }

    TRACE0("Java_com_sun_media_sound_MixerSynth_nDestroySynthesizer completed.\n");
}


JNIEXPORT jboolean JNICALL
    Java_com_sun_media_sound_MixerSynth_nStartSynthesizer(JNIEnv* e, jobject thisObj, jlong id) 
{

    OPErr           opErr = NO_ERR;
    GM_Song			*pSong = (GM_Song *) (INT_PTR) id;

    TRACE0("Java_com_sun_media_sound_MixerSynth_nStartSynthesizer.\n");

    // $$kk: 03.23.98: hard coding instrument loading here
    opErr = GM_StartLiveSong(pSong, 1);
 
    if (opErr)
	{
	    ERROR0("FAILED TO START MIDI DIRECT: error on GM_StartLiveSong\n");
	    // $$kk: 09.17.98: what to do here?

	    return (jboolean)FALSE;
	}

    TRACE0("Java_com_sun_media_sound_MixerSynth_nStartSynthesizer.completed\n");
    return (jboolean)TRUE;
}


JNIEXPORT jboolean JNICALL
    Java_com_sun_media_sound_MixerSynth_nLoadInstrument(JNIEnv* e, jobject thisObj, jlong id, jint instrumentId)  
{
    OPErr opErr;
	
    TRACE0("Java_com_sun_media_sound_MixerSynth_nLoadInstrument\n");

    opErr = GM_LoadInstrument((GM_Song *) (INT_PTR) id, (XLongResourceID)instrumentId);

    TRACE0("Java_com_sun_media_sound_MixerSynth_nLoadInstrument completed\n");

    return ( (opErr == NO_ERR) ? TRUE : FALSE );
}


JNIEXPORT jboolean JNICALL
    Java_com_sun_media_sound_MixerSynth_nUnloadInstrument(JNIEnv* e, jobject thisObj, jlong id, jint instrumentId)  
{
    OPErr opErr;
	
    TRACE0("Java_com_sun_media_sound_MixerSynth_nUnloadInstrument\n");

    opErr = GM_UnloadInstrument((GM_Song *) (INT_PTR) id, (XLongResourceID)instrumentId);

    TRACE0("Java_com_sun_media_sound_MixerSynth_nUnloadInstrument completed\n");

    return ( (opErr == NO_ERR) ? TRUE : FALSE );
}


JNIEXPORT jboolean JNICALL
    Java_com_sun_media_sound_MixerSynth_nRemapInstrument(JNIEnv* e, jobject thisObj, jlong id, jint from, jint to)  
{
    OPErr opErr;
	
    TRACE0("Java_com_sun_media_sound_MixerSynth_nRemapInstrument\n");

    opErr = GM_RemapInstrument((GM_Song *) (INT_PTR) id, (XLongResourceID)from, (XLongResourceID)to);

    TRACE0("Java_com_sun_media_sound_MixerSynth_nRemapInstrument completed\n");

    return ( (opErr == NO_ERR) ? TRUE : FALSE );
}



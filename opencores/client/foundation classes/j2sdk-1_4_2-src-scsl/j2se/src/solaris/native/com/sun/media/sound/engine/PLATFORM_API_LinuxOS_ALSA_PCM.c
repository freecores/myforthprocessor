/*
 * @(#)PLATFORM_API_LinuxOS_ALSA_PCM.c	1.4 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*****************************************************************************/
/*
**	PLATFORM_API_LinuxOS_ALSA_PCM.c
**
**	Overview:
**	This provides platform specific DirectAudio implementation for Linux using ALSA.
**	This implementation does not interface with the HAE engine at all.
**      More info on ALSA: http://www.alsa-project.org
**
**	History	-
**	2002-09-13	$$fb created
*/
/*****************************************************************************/

//#define USE_ERROR
//#define USE_TRACE

#include "PLATFORM_API_LinuxOS_ALSA_Utils.h"
#include "DirectAudio.h"

#if USE_DAUDIO == TRUE

// GetPosition method 1: based on how many bytes are passed to the kernel driver
//                       + does not need much processor resources
//                       - not very exact, "jumps"    
// GetPosition method 2: ask kernel about actual position of playback.
//                       - very exact
//                       - switch to kernel layer for each call
// GetPosition method 3: use snd_pcm_avail() call - not yet in official ALSA
// quick tests on a Pentium 200MMX showed max. 1.5% processor usage
// for playing back a CD-quality file and printing 20x per second a line
// on the console with the current time. So I guess performance is not such a 
// factor here.
//#define GET_POSITION_METHOD1
#define GET_POSITION_METHOD2


// The default time for a period in microseconds.
// For very small buffers, only 2 periods are used.
#define DEFAULT_PERIOD_TIME 20000 /* 20ms */

///// implemented functions of DirectAudio.h

INT32 DAUDIO_GetDirectAudioDeviceCount() {
    return (INT32) getAudioDeviceCount();
}


INT32 DAUDIO_GetDirectAudioDeviceDescription(INT32 mixerIndex, DirectAudioDeviceDescription* description) {
    ALSA_AudioDeviceDescription adesc;
    
    adesc.index = (int) mixerIndex;
    adesc.strLen = DAUDIO_STRING_LENGTH;

    adesc.maxSimultaneousLines = (int*) (&(description->maxSimulLines));
    adesc.deviceID = &(description->deviceID);
    adesc.name = description->name;
    adesc.vendor = description->vendor;
    adesc.description = description->description;
    adesc.version = description->version;

    return getAudioDeviceDescriptionByIndex(&adesc);
}

void DAUDIO_GetFormats(INT32 mixerIndex, INT32 deviceID, int isSource, void* creator) {
    snd_pcm_t* handle;
    snd_pcm_format_mask_t* formatMask;
    snd_pcm_format_t format;
    snd_pcm_hw_params_t* hwParams;

    int ret;
    int sampleSizeInBytes, significantBits, isSigned, isBigEndian, enc;
    int channels, minChannels, maxChannels;
    int rate;

    if (openPCMfromDeviceID(deviceID, &handle, isSource, TRUE /*query hardware*/) < 0) {
	return;
    }
    ret = snd_pcm_format_mask_malloc(&formatMask);
    if (ret != 0) {
	ERROR1("snd_pcm_format_mask_malloc returned error %d\n", ret);
    } else {
	ret = snd_pcm_hw_params_malloc(&hwParams);
	if (ret != 0) {
	    ERROR1("snd_pcm_hw_params_malloc returned error %d\n", ret);
	} else {
	    ret = snd_pcm_hw_params_any(handle, hwParams);
	    if (ret != 0) {
		ERROR1("snd_pcm_hw_params_any returned error %d\n", ret);
	    }
	}
	snd_pcm_hw_params_get_format_mask(hwParams, formatMask);
#ifdef ALSA_PCM_NEW_HW_PARAMS_API
	if (ret == 0) {
	    ret = snd_pcm_hw_params_get_channels_min(hwParams, &minChannels);
	    if (ret != 0) {
		ERROR1("snd_pcm_hw_params_get_channels_min returned error %d\n", ret);
	    }
	}
	if (ret == 0) {
	    ret = snd_pcm_hw_params_get_channels_max(hwParams, &maxChannels);
	    if (ret != 0) {
		ERROR1("snd_pcm_hw_params_get_channels_max returned error %d\n", ret);
	    }
	}
#else
	minChannels = snd_pcm_hw_params_get_channels_min(hwParams);
	maxChannels = snd_pcm_hw_params_get_channels_max(hwParams);
	if (minChannels > maxChannels) {
	    ERROR2("MinChannels=%d, maxChannels=%d\n", minChannels, maxChannels);
	}
#endif	

	// avoid too many channels and set to -1 (NOT_SPECIFIED) instead
	if (maxChannels - minChannels > MAXIMUM_LISTED_CHANNELS) {
	    minChannels = -1;
	    maxChannels = -1;
	}
	if (ret == 0) {
	    rate = -1;
	    for (format = 0; format <= SND_PCM_FORMAT_LAST; format++) {
		if (snd_pcm_format_mask_test(formatMask, format)) {
		    // format exists
		    if (getFormatFromAlsaFormat(format, &sampleSizeInBytes, 
						&significantBits, 
						&isSigned, &isBigEndian, &enc)) {
			for (channels = minChannels; channels <= maxChannels; channels++) {
			    DAUDIO_AddAudioFormat(creator, significantBits, 
						  sampleSizeInBytes * channels,
						  channels, rate,
						  enc, isSigned, isBigEndian);
			}
		    } else {
			TRACE1("could not get format from alsa for format %d\n", format);
		    }
		} else {
		    //TRACE1("Format %d not supported\n", format);
		}
	    } // for loop
	    snd_pcm_hw_params_free(hwParams);
	}
	snd_pcm_format_mask_free(formatMask);
    }
    snd_pcm_close(handle);
}

typedef struct tag_AlsaPcmInfo {
    snd_pcm_t* handle;
    snd_pcm_hw_params_t* hwParams;
    snd_pcm_sw_params_t* swParams;
    int bufferSizeInBytes;
    int frameSize; // storage size in Bytes
    int periods;
    snd_pcm_uframes_t periodSize;
#ifdef GET_POSITION_METHOD2
    // to be used exclusively by getBytePosition!
    snd_pcm_status_t* positionStatus;
#endif
} AlsaPcmInfo;


int setStartThresholdNoCommit(AlsaPcmInfo* info, int useThreshold) {
    int ret;
    int threshold;

    if (useThreshold) {
	// start device whenever anything is written to the buffer
	threshold = 1;
    } else {
	// never start the device automatically
	threshold = 2000000000; /* near UINT_MAX */
    }
    ret = snd_pcm_sw_params_set_start_threshold(info->handle, info->swParams, threshold);
    if (ret < 0) {
	ERROR1("Unable to set start threshold mode: %s\n", snd_strerror(ret));
	return FALSE;
    }
    return TRUE;
}

int setStartThreshold(AlsaPcmInfo* info, int useThreshold) {
    int ret = 0;

    if (!setStartThresholdNoCommit(info, useThreshold)) {
	ret = -1;
    }
    if (ret == 0) {
	// commit it
	ret = snd_pcm_sw_params(info->handle, info->swParams);
	if (ret < 0) {
	    ERROR1("Unable to set sw params: %s\n", snd_strerror(ret));
	}
    }
    return (ret == 0)?TRUE:FALSE;
}


// returns TRUE if successful
int setHWParams(AlsaPcmInfo* info,
		float sampleRate, 
		int channels, 
		int bufferSizeInFrames,
		snd_pcm_format_t format) {
    unsigned int rrate;
    int ret, dir, periods, periodTime;

    /* choose all parameters */
    ret = snd_pcm_hw_params_any(info->handle, info->hwParams);
    if (ret < 0) {
	ERROR1("Broken configuration: no configurations available: %s\n", snd_strerror(ret));
	return FALSE;
    }
    /* set the interleaved read/write format */
    ret = snd_pcm_hw_params_set_access(info->handle, info->hwParams, SND_PCM_ACCESS_RW_INTERLEAVED);
    if (ret < 0) {
	ERROR1("SND_PCM_ACCESS_RW_INTERLEAVED access type not available: %s\n", snd_strerror(ret));
	return FALSE;
    }
    /* set the sample format */
    ret = snd_pcm_hw_params_set_format(info->handle, info->hwParams, format);
    if (ret < 0) {
	ERROR1("Sample format not available: %s\n", snd_strerror(ret));
	return FALSE;
    }
    /* set the count of channels */
    ret = snd_pcm_hw_params_set_channels(info->handle, info->hwParams, channels);
    if (ret < 0) {
	ERROR2("Channels count (%d) not available: %s\n", channels, snd_strerror(ret));
	return FALSE;
    }
    /* set the stream rate */
    rrate = (int) (sampleRate + 0.5f);
#ifdef ALSA_PCM_NEW_HW_PARAMS_API
    dir = 0;
    ret = snd_pcm_hw_params_set_rate_near(info->handle, info->hwParams, &rrate, &dir);
#else
    ret = snd_pcm_hw_params_set_rate_near(info->handle, info->hwParams, rrate, 0);
#endif
    if (ret < 0) {
	ERROR2("Rate %dHz not available for playback: %s\n", (int) (sampleRate+0.5f), snd_strerror(ret));
	return FALSE;
    }
    if ((rrate-sampleRate > 2) || (rrate-sampleRate < - 2)) {
	ERROR2("Rate doesn't match (requested %2.2fHz, got %dHz)\n", sampleRate, rrate);
	return FALSE;
    }
    /* set the buffer time */
#ifdef ALSA_PCM_NEW_HW_PARAMS_API
    ret = snd_pcm_hw_params_set_buffer_size_near(info->handle, info->hwParams, (snd_pcm_uframes_t*) (&bufferSizeInFrames));
#else
    ret = snd_pcm_hw_params_set_buffer_size_near(info->handle, info->hwParams, bufferSizeInFrames);
#endif
    if (ret < 0) {
	ERROR2("Unable to set buffer size to %d frames: %s\n", 
	       bufferSizeInFrames, snd_strerror(ret));
	return FALSE;
    }
    /* set the period time */
    if (bufferSizeInFrames > 1024) {
	dir = 0;
	periodTime = DEFAULT_PERIOD_TIME;
#ifdef ALSA_PCM_NEW_HW_PARAMS_API
	ret = snd_pcm_hw_params_set_period_time_near(info->handle, info->hwParams, &periodTime, &dir);
#else
	periodTime = snd_pcm_hw_params_set_period_time_near(info->handle, info->hwParams, periodTime, &dir);
	ret = periodTime;
#endif
	if (ret < 0) {
	    ERROR2("Unable to set period time to %d: %s\n", DEFAULT_PERIOD_TIME, snd_strerror(ret));
	    return FALSE;
	}
    } else {
	/* set the period count for very small buffer sizes to 2 */
	dir = 0;
	periods = 2;
#ifdef ALSA_PCM_NEW_HW_PARAMS_API
	ret = snd_pcm_hw_params_set_periods_near(info->handle, info->hwParams, &periods, &dir);
#else
	periods = snd_pcm_hw_params_set_periods_near(info->handle, info->hwParams, periods, &dir);
	ret = periods;
#endif
	if (ret < 0) {
	    ERROR2("Unable to set period count to %d: %s\n", /*periods*/ 2, snd_strerror(ret));
	    return FALSE;
	}
    }
    /* write the parameters to device */
    ret = snd_pcm_hw_params(info->handle, info->hwParams);
    if (ret < 0) {
	ERROR1("Unable to set hw params: %s\n", snd_strerror(ret));
	return FALSE;
    }
    return TRUE;
}

// returns 1 if successful
int setSWParams(AlsaPcmInfo* info) {
    int ret;
    
    /* get the current swparams */
    ret = snd_pcm_sw_params_current(info->handle, info->swParams);
    if (ret < 0) {
	ERROR1("Unable to determine current swparams: %s\n", snd_strerror(ret));
	return FALSE;
    }
    /* never start the transfer automatically */
    if (!setStartThresholdNoCommit(info, FALSE /* don't use threshold */)) {
	return FALSE;
    }

    /* allow the transfer when at least period_size samples can be processed */
    ret = snd_pcm_sw_params_set_avail_min(info->handle, info->swParams, info->periodSize);
    if (ret < 0) {
	ERROR1("Unable to set avail min for playback: %s\n", snd_strerror(ret));
	return FALSE;
    }
    /* align all transfers to 1 sample */
    ret = snd_pcm_sw_params_set_xfer_align(info->handle, info->swParams, 1);
    if (ret < 0) {
	ERROR1("Unable to set transfer align: %s\n", snd_strerror(ret));
	return FALSE;
    }
    /* write the parameters to the playback device */
    ret = snd_pcm_sw_params(info->handle, info->swParams);
    if (ret < 0) {
	ERROR1("Unable to set sw params: %s\n", snd_strerror(ret));
	return FALSE;
    }
    return TRUE;
}

static snd_output_t* ALSA_OUTPUT = NULL;

void* DAUDIO_Open(INT32 mixerIndex, INT32 deviceID, int isSource, 
		  int encoding, float sampleRate, int sampleSizeInBits, 
		  int frameSize, int channels, 
		  int isSigned, int isBigEndian, int bufferSizeInBytes) {
    snd_pcm_format_mask_t* formatMask;
    snd_pcm_format_t format;
    int dir;
    int ret = 0;
    AlsaPcmInfo* info = NULL;

    TRACE0("> DAUDIO_Open\n");
#ifdef USE_TRACE
    // for using ALSA debug dump methods
    if (ALSA_OUTPUT == NULL) {
	snd_output_stdio_attach(&ALSA_OUTPUT, stdout, 0);
    }
#endif

    info = (AlsaPcmInfo*) malloc(sizeof(AlsaPcmInfo));
    if (!info) {
	ERROR0("Out of memory\n");
	return NULL;
    }
    memset(info, 0, sizeof(AlsaPcmInfo));

    ret = openPCMfromDeviceID(deviceID, &(info->handle), isSource, FALSE /* do open device*/);
    if (ret == 0) {
	// set to blocking mode
	snd_pcm_nonblock(info->handle, 0);
	ret = snd_pcm_hw_params_malloc(&(info->hwParams));
	if (ret != 0) {
	    ERROR1("  snd_pcm_hw_params_malloc returned error %d\n", ret);
	} else {
	    ret = -1;
	    if (getAlsaFormatFromFormat(&format, frameSize / channels, sampleSizeInBits,
					isSigned, isBigEndian, encoding)) {
		if (setHWParams(info,
				sampleRate, 
				channels, 
				bufferSizeInBytes / frameSize,
				format)) {
		    info->frameSize = frameSize;
#ifdef ALSA_PCM_NEW_HW_PARAMS_API
		    ret = snd_pcm_hw_params_get_period_size(info->hwParams, &(info->periodSize), &dir);
		    if (ret < 0) {
			ERROR1("ERROR: snd_pcm_hw_params_get_period: %s\n", snd_strerror(ret));
		    }
		    snd_pcm_hw_params_get_periods(info->hwParams, &(info->periods), &dir);
		    snd_pcm_hw_params_get_buffer_size(info->hwParams, (snd_pcm_uframes_t*) (&(info->bufferSizeInBytes)));
		    info->bufferSizeInBytes *= frameSize;
#else
		    info->periodSize = snd_pcm_hw_params_get_period_size(info->hwParams, &dir);
		    info->periods = snd_pcm_hw_params_get_periods(info->hwParams, &dir);
		    info->bufferSizeInBytes = snd_pcm_hw_params_get_buffer_size(info->hwParams) * frameSize;
		    ret = 0;
#endif
		    TRACE3("  DAUDIO_Open: period size = %d frames, periods = %d. Buffer size: %d bytes.\n", 
			   (int) info->periodSize, info->periods, info->bufferSizeInBytes);
		}
	    }
	}
	if (ret == 0) {
	    // set software parameters
	    ret = snd_pcm_sw_params_malloc(&(info->swParams));
	    if (ret != 0) {
		ERROR1("snd_pcm_hw_params_malloc returned error %d\n", ret);
	    } else {
		if (!setSWParams(info)) {
		    ret = -1;
		}
	    }
	}
	if (ret == 0) {
	    // prepare device
	    ret = snd_pcm_prepare(info->handle);
	    if (ret < 0) {
		ERROR1("ERROR: snd_pcm_prepare: %s\n", snd_strerror(ret));
	    }
	}
	
#ifdef GET_POSITION_METHOD2
	if (ret == 0) {
	    ret = snd_pcm_status_malloc(&(info->positionStatus));
	    if (ret != 0) {
		ERROR1("ERROR in snd_pcm_status_malloc: %s\n", snd_strerror(ret));
	    }
	}
#endif
    }
    if (ret != 0) {
	DAUDIO_Close((void*) info, isSource);
	info = NULL;
    } else {
	TRACE0("< DAUDIO_Open: Opened device successfully.\n");
    }
    return (void*) info;
}

#ifdef USE_TRACE
void printState(snd_pcm_state_t state) {
    if (state == SND_PCM_STATE_OPEN) {
	TRACE0("State: SND_PCM_STATE_OPEN\n");
    }
    else if (state == SND_PCM_STATE_SETUP) {
	TRACE0("State: SND_PCM_STATE_SETUP\n");
    }
    else if (state == SND_PCM_STATE_PREPARED) {
	TRACE0("State: SND_PCM_STATE_PREPARED\n");
    }
    else if (state == SND_PCM_STATE_RUNNING) {
	TRACE0("State: SND_PCM_STATE_RUNNING\n");
    }
    else if (state == SND_PCM_STATE_XRUN) {
	TRACE0("State: SND_PCM_STATE_XRUN\n");
    }
    else if (state == SND_PCM_STATE_DRAINING) {
	TRACE0("State: SND_PCM_STATE_DRAINING\n");
    }
    else if (state == SND_PCM_STATE_PAUSED) {
	TRACE0("State: SND_PCM_STATE_PAUSED\n");
    }
    else if (state == SND_PCM_STATE_SUSPENDED) {
	TRACE0("State: SND_PCM_STATE_SUSPENDED\n");
    }
}
#endif

int DAUDIO_Start(void* id, int isSource) {
    AlsaPcmInfo* info = (AlsaPcmInfo*) id;
    int ret;
    snd_pcm_state_t state;

    TRACE0("> DAUDIO_Start\n");
    // set to blocking mode
    snd_pcm_nonblock(info->handle, 0);
    // set start mode so that it always starts as soon as data is there
    setStartThreshold(info, TRUE /* use threshold */);
    state = snd_pcm_state(info->handle);
    if (state == SND_PCM_STATE_PAUSED) {
	// in case it was stopped previously
	TRACE0("  Un-pausing...\n");
	ret = snd_pcm_pause(info->handle, FALSE);
	if (ret != 0) {
	    ERROR2("  NOTE: error in snd_pcm_pause:%d: %s\n", ret, snd_strerror(ret));
	}
    }
    if (state == SND_PCM_STATE_SUSPENDED) {
	TRACE0("  Resuming...\n");
	ret = snd_pcm_resume(info->handle);
	if (ret < 0) {
	    if ((ret != -EAGAIN) && (ret != -ENOSYS)) {
		ERROR2("  ERROR: error in snd_pcm_resume:%d: %s\n", ret, snd_strerror(ret));
	    }
	}
    }
    if (state == SND_PCM_STATE_SETUP) {
    	TRACE0("need to call prepare again...\n");
	// prepare device
	ret = snd_pcm_prepare(info->handle);
	if (ret < 0) {
	    ERROR1("ERROR: snd_pcm_prepare: %s\n", snd_strerror(ret));
	}
    }
    // in case there is still data in the buffers
    ret = snd_pcm_start(info->handle);
    if (ret != 0) {
	if (ret != -EPIPE) {
	    ERROR2("  NOTE: error in snd_pcm_start: %d: %s\n", ret, snd_strerror(ret));
	}
    }
    // set to non-blocking mode
    ret = snd_pcm_nonblock(info->handle, 1);
    if (ret != 0) {
	ERROR1("  ERROR in snd_pcm_nonblock: %s\n", snd_strerror(ret));
    }
    state = snd_pcm_state(info->handle);
#ifdef USE_TRACE
    printState(state);
#endif
    ret = (state == SND_PCM_STATE_PREPARED) 
	|| (state == SND_PCM_STATE_RUNNING)
	|| (state == SND_PCM_STATE_XRUN)
	|| (state == SND_PCM_STATE_SUSPENDED);
    TRACE1("< DAUDIO_Start %s\n", ret?"success":"error");
    return ret?TRUE:FALSE;
}

int DAUDIO_Stop(void* id, int isSource) {
    AlsaPcmInfo* info = (AlsaPcmInfo*) id;
    int ret;

    TRACE0("> DAUDIO_Stop\n");
    // set to blocking mode
    snd_pcm_nonblock(info->handle, 0);
    setStartThreshold(info, FALSE /* don't use threshold */); // device will not start after buffer xrun
    ret = snd_pcm_pause(info->handle, 1);
    if (ret != 0) {
	ERROR1("ERROR in snd_pcm_pause: %s\n", snd_strerror(ret));
	return FALSE;
    }
    TRACE1("< DAUDIO_Stop %s\n", ret?"success":"error");
    return TRUE;
}

void DAUDIO_Close(void* id, int isSource) {
    AlsaPcmInfo* info = (AlsaPcmInfo*) id;

    TRACE0("DAUDIO_Close\n");
    if (info != NULL) {
	if (info->handle != NULL) {
	    snd_pcm_close(info->handle);
	}
	if (info->hwParams) {
	    snd_pcm_hw_params_free(info->hwParams);
	}
	if (info->swParams) {
	    snd_pcm_sw_params_free(info->swParams);
	}
#ifdef GET_POSITION_METHOD2
	if (info->positionStatus) {
	    snd_pcm_status_free(info->positionStatus);
	}
#endif
	free(info);
    }
}

/*
 * Underrun and suspend recovery
 * returns
 * 0:  exit native and return 0
 * 1:  try again to write/read
 * -1: error - exit native with return value -1
 */ 
int xrun_recovery(AlsaPcmInfo* info, int err) {
    int ret;

    if (err == -EPIPE) {    /* underrun / overflow */
	TRACE0("xrun_recovery: underrun/overflow.\n");
	ret = snd_pcm_prepare(info->handle);
	if (ret < 0) {
	    ERROR1("Can't recover from underrun/overflow, prepare failed: %s\n", snd_strerror(ret));
	    return -1;
	}
	return 1;
    } 
    else if (err == -ESTRPIPE) {
	TRACE0("xrun_recovery: suspended.\n");
	ret = snd_pcm_resume(info->handle);
	if (ret < 0) {
	    if (ret == -EAGAIN) {
		return 0; /* wait until the suspend flag is released */
	    }
	    return -1;
	}
	ret = snd_pcm_prepare(info->handle);
	if (ret < 0) {
	    ERROR1("Can't recover from underrun/overflow, prepare failed: %s\n", snd_strerror(ret));
	    return -1;
	}
	return 1;
    }
    else if (err == -EAGAIN) {
	TRACE0("xrun_recovery: EAGAIN try again flag.\n");
	return 0;
    }
    TRACE2("xrun_recovery: unexpected error %d: %s\n", err, snd_strerror(err));
    return -1;
}

// returns -1 on error
int DAUDIO_Write(void* id, char* data, int byteSize) {
    AlsaPcmInfo* info = (AlsaPcmInfo*) id;
    int ret, count;
    snd_pcm_sframes_t frameSize, writtenFrames;

    TRACE1("> DAUDIO_Write %d bytes\n", byteSize);
    count = 2; // maximum number of trials to recover from underrun
    //frameSize = snd_pcm_bytes_to_frames(info->handle, byteSize);
    frameSize = (snd_pcm_sframes_t) (byteSize / info->frameSize);
    do {
	writtenFrames = snd_pcm_writei(info->handle, (const void*) data, (snd_pcm_uframes_t) frameSize);
	if (writtenFrames < 0) {
	    ret = xrun_recovery(info, (int) writtenFrames);
	    if (ret <= 0) {
		TRACE1("DAUDIO_Write: xrun recovery returned %d -> return.\n", ret);
		return ret;
	    }
	    if (count-- <= 0) {
		ERROR0("DAUDIO_Write: too many attempts to recover from xrun/suspend\n");
		return -1;
	    }
	} else {
	    break;
	}
    } while (TRUE);
    //ret =  snd_pcm_frames_to_bytes(info->handle, writtenFrames);
    ret =  (int) (writtenFrames * info->frameSize);
    TRACE1("< DAUDIO_Write: returning %d bytes.\n", ret);
    return ret;
}

// returns -1 on error
int DAUDIO_Read(void* id, char* data, int byteSize) {
    AlsaPcmInfo* info = (AlsaPcmInfo*) id;
    int ret, count;
    snd_pcm_sframes_t frameSize, readFrames;

    TRACE1("> DAUDIO_Read %d bytes\n", byteSize);
    count = 2; // maximum number of trials to recover from error
    //frameSize = snd_pcm_bytes_to_frames(info->handle, byteSize);
    frameSize = (snd_pcm_sframes_t) (byteSize / info->frameSize);
    do {
	readFrames = snd_pcm_readi(info->handle, (void*) data, (snd_pcm_uframes_t) frameSize);
	if (readFrames < 0) {
	    ret = xrun_recovery(info, (int) readFrames);
	    if (ret <= 0) {
		TRACE1("DAUDIO_Read: xrun recovery returned %d -> return.\n", ret);
		return ret;
	    }
	    if (count-- <= 0) {
		ERROR0("DAUDIO_Read: too many attempts to recover from xrun/suspend\n");
		return -1;
	    }
	} else {
	    break;
	}
    } while (TRUE);
    //ret =  snd_pcm_frames_to_bytes(info->handle, readFrames);
    ret =  (int) (readFrames * info->frameSize);
    TRACE1("< DAUDIO_Read: returning %d bytes.\n", ret);
    return ret;
}


int DAUDIO_GetBufferSize(void* id, int isSource) {
    AlsaPcmInfo* info = (AlsaPcmInfo*) id;

    return info->bufferSizeInBytes;
}

int DAUDIO_StillDraining(void* id, int isSource) {
    AlsaPcmInfo* info = (AlsaPcmInfo*) id;
    snd_pcm_state_t state;
    
    state = snd_pcm_state(info->handle);
    //printState(state);
    //TRACE1("Still draining: %s\n", (state != SND_PCM_STATE_XRUN)?"TRUE":"FALSE");
    return (state == SND_PCM_STATE_RUNNING)?TRUE:FALSE;
}


int DAUDIO_Flush(void* id, int isSource) {
    AlsaPcmInfo* info = (AlsaPcmInfo*) id;
    int ret;

    TRACE0("DAUDIO_Flush\n");
    ret = snd_pcm_drop(info->handle);
    if (ret != 0) {
	ERROR1("ERROR in snd_pcm_drop: %s\n", snd_strerror(ret));
	return FALSE;
    }
    return TRUE;
}

int DAUDIO_GetAvailable(void* id, int isSource) {
    AlsaPcmInfo* info = (AlsaPcmInfo*) id;
    snd_pcm_sframes_t availableInFrames;
    snd_pcm_state_t state;
    int ret;

    state = snd_pcm_state(info->handle);
    if (state == SND_PCM_STATE_XRUN) {
	// if in xrun state then we have the entire buffer available,
	// not 0 as alsa reports
	ret = info->bufferSizeInBytes;
    } else {
	availableInFrames = snd_pcm_avail_update(info->handle);
	if (availableInFrames < 0) {
	    ret = 0;
	} else {
	    //ret = snd_pcm_frames_to_bytes(info->handle, availableInFrames);
	    ret = (int) (availableInFrames * info->frameSize);
	}
    }
    TRACE1("DAUDIO_GetAvailable returns %d bytes\n", ret);
    return ret;
}

INT64 estimatePositionFromAvail(AlsaPcmInfo* info, int isSource, INT64 javaBytePos, int availInBytes) {
    // estimate the current position with the buffer size and 
    // the available bytes to read or write in the buffer.
    // not an elegant solution - bytePos will stop on xruns,
    // and in race conditions it may jump backwards
    // Advantage is that it is indeed based on the samples that go through
    // the system (rather than time-based methods)
    if (isSource) {
	// javaBytePos is the position that is reached when the current 
	// buffer is played completely
	return (INT64) (javaBytePos - info->bufferSizeInBytes + availInBytes);
    } else {
	// javaBytePos is the position that was when the current buffer was empty
	return (INT64) (javaBytePos + availInBytes);
    }
}

INT64 DAUDIO_GetBytePosition(void* id, int isSource, INT64 javaBytePos) {
    AlsaPcmInfo* info = (AlsaPcmInfo*) id;
    int ret;
    INT64 result = javaBytePos;
    snd_pcm_state_t state;
    state = snd_pcm_state(info->handle);

    if (state != SND_PCM_STATE_XRUN) {
#ifdef GET_POSITION_METHOD2
	snd_timestamp_t* ts;
	snd_pcm_uframes_t framesAvail;
	
	// note: slight race condition if this is called simultaneously from 2 threads
	ret = snd_pcm_status(info->handle, info->positionStatus);
	if (ret != 0) {
	    ERROR1("ERROR in snd_pcm_status: %s\n", snd_strerror(ret));
	    result = javaBytePos;
	} else {
	    // calculate from time value, or from available bytes
	    framesAvail = snd_pcm_status_get_avail(info->positionStatus);
	    result = estimatePositionFromAvail(info, isSource, javaBytePos, framesAvail * info->frameSize);
	}
#endif
#ifdef GET_POSITION_METHOD3
	snd_pcm_uframes_t framesAvail;
	ret = snd_pcm_avail(info->handle, &framesAvail);
	if (ret != 0) {
	    ERROR1("ERROR in snd_pcm_avail: %s\n", snd_strerror(ret));
	    result = javaBytePos;
	} else {
	    result = estimatePositionFromAvail(info, isSource, javaBytePos, framesAvail * info->frameSize);
	}
#endif
#ifdef GET_POSITION_METHOD1
	result = estimatePositionFromAvail(info, isSource, javaBytePos, DAUDIO_GetAvailable(id, isSource));
#endif
    }
    //printf("getbyteposition: javaBytePos=%d , return=%d\n", (int) javaBytePos, (int) result);
    return result;
}

#endif // USE_DAUDIO

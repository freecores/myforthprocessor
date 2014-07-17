/*
 * @(#)PLATFORM_API_LinuxOS_ALSA_Utils.h	1.3 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*****************************************************************************/
/*
**	PLATFORM_API_LinuxOS_ALSA_Utils.h
**
**	Overview:
**	Platform specfic utility functions for ALSA on Linux.
**
**	History	-
**	2002-09-13	$$fb created
*/
/*****************************************************************************/

// define this with a later version of ALSA than 0.9.0rc3
#define ALSA_PCM_NEW_HW_PARAMS_API
#include <alsa/asoundlib.h>
#include "Utilities.h"

#ifndef PLATFORM_API_LINUXOS_ALSA_UTILS_H_INCLUDED
#define PLATFORM_API_LINUXOS_ALSA_UTILS_H_INCLUDED

// if this is defined, use plughw: devices
#define USE_PLUGHW
//#undef USE_PLUGHW

#define ALSA_VERSION_PROC_FILE "/proc/asound/version"
#define ALSA_HARDWARE "hw"
#define ALSA_HARDWARE_CARD ALSA_HARDWARE":%d"
#define ALSA_HARDWARE_DEVICE ALSA_HARDWARE_CARD",%d"
#define ALSA_HARDWARE_SUBDEVICE ALSA_HARDWARE_DEVICE",%d"

#define ALSA_PLUGHARDWARE "plughw"

// Environment variable for inclusion of subdevices in device listing.
// If this variable is unset or "no", then subdevices are ignored, and 
// it's ALSA's choice which one to use (enables hardware mixing)
#define ENV_ENUMERATE_SUBDEVICES "ALSA_ENUMERATE_SUBDEVICES"

// maximum number of channels that is listed in the formats. If more, than
// just -1 for channel count is used.
#define MAXIMUM_LISTED_CHANNELS 32

typedef struct tag_ALSA_AudioDeviceDescription {
    int index;          // in
    int strLen;         // in
    INT32* deviceID;    // out
    int* maxSimultaneousLines; // out
    char* name;         // out
    char* vendor;       // out
    char* description;  // out
    char* version;      // out
} ALSA_AudioDeviceDescription;

void initAlsaSupport(); // must be called before any ALSA calls
int enumerateSubdevices(); // if this returns true, sub devices are listed as separate mixers


int getAudioDeviceCount();
int getAudioDeviceDescriptionByIndex(ALSA_AudioDeviceDescription* desc);

// returns ALSA error code, or 0 if successful
int openPCMfromDeviceID(int deviceID, snd_pcm_t** handle, int isSource, int hardware);

// returns 1 if successful
// enc: 0 for PCM, 1 for ULAW, 2 for ALAW (see DirectAudio.h)
int getFormatFromAlsaFormat(snd_pcm_format_t alsaFormat, 
			    int* sampleSizeInBytes, int* significantBits, 
			    int* isSigned, int* isBigEndian, int* enc);

int getAlsaFormatFromFormat(snd_pcm_format_t* alsaFormat, 
			    int sampleSizeInBytes, int significantBits, 
			    int isSigned, int isBigEndian, int enc);
#endif // PLATFORM_API_LINUXOS_ALSA_UTILS_H_INCLUDED


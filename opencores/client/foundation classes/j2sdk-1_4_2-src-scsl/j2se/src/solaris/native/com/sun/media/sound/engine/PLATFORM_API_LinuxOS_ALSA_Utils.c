/*
 * @(#)PLATFORM_API_LinuxOS_ALSA_Utils.c	1.3 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*****************************************************************************/
/*
**	PLATFORM_API_LinuxOS_ALSA_Utils.c
**
**	Overview:
**	Utility functions for ALSA
**
**	History	-
**	2002-09-13	$$fb created
*/
/*****************************************************************************/

//#define USE_ERROR
//#define USE_TRACE

#include "PLATFORM_API_LinuxOS_ALSA_Utils.h"

void alsaDebugOutput(const char *file, int line, const char *function, int err, const char *fmt, ...) {
#ifdef USE_ERROR
    va_list args;
    va_start(args, fmt);
    printf("%s:%d function %s: error %d: %s\n", file, line, function, err, snd_strerror(err));
    if (strlen(fmt) > 0) {
	vprintf(fmt, args);
    }
    va_end(args);
#endif
}

static int alsa_inited = 0;
static int alsa_enumerate_subdevices = FALSE; // default: no

void initAlsaSupport() {
    char* enumerate;
    if (!alsa_inited) {
	alsa_inited = TRUE;
	snd_lib_error_set_handler(&alsaDebugOutput);
	enumerate = getenv(ENV_ENUMERATE_SUBDEVICES);
	if (enumerate != NULL && strlen(enumerate) > 0
	    && (enumerate[0] != 'f')   // false
	    && (enumerate[0] != 'F')   // False
	    && (enumerate[0] != 'n')   // no
	    && (enumerate[0] != 'N')) { // NO
		alsa_enumerate_subdevices = TRUE;
	}
    }
}

// if true, sub devices are listed as separate mixers
int enumerateSubdevices() {
    initAlsaSupport();
    return alsa_enumerate_subdevices;
}


/*
 * deviceID contains packed card, device and subdevice numbers
 * each number takes 10 bits
 */
int encodeDeviceID(int card, int device, int subdevice) {
    return ((card & 0x3FF) << 20) | ((device & 0x3FF) << 10) | (subdevice & 0x3FF);
}

void decodeDeviceID(int deviceID, int* card, int* device, int* subdevice) {
    *card = (deviceID >> 20) & 0x3FF;
    *device = (deviceID >> 10) & 0x3FF;
    if (enumerateSubdevices()) {
	*subdevice = deviceID  & 0x3FF;
    } else {
	*subdevice = -1; // ALSA will choose any subdevices
    }
}


// callback for iteration through devices
// returns TRUE if iteration should continue
typedef int (*DeviceIteratorPtr)(char* devName, int card, int device, int subdevice, 
				   snd_ctl_t *handle, snd_pcm_info_t* pcminfo, snd_ctl_card_info_t* info, void* userData);

// for each ALSA device, call iterator. userData is passed to the iterator
// returns total number of iterations
int iterateDevices(DeviceIteratorPtr iterator, void* userData) {
    int count = 0;
    int subdeviceCount;
    int card, dev, subDev;
    char devname[16];
    int err;
    snd_ctl_t *handle;
    snd_pcm_info_t* pcminfo;
    snd_ctl_card_info_t* info;
    int doContinue = TRUE;

    snd_pcm_info_malloc(&pcminfo);
    snd_ctl_card_info_malloc(&info);
    
    card = -1;
    if (snd_card_next(&card) >= 0) {
	while (doContinue && (card >= 0)) {
	    sprintf(devname, ALSA_HARDWARE_CARD, card);
	    TRACE1("Opening alsa device \"%s\"...\n", devname);
	    err = snd_ctl_open(&handle, devname, 0);
	    if (err < 0) {
		ERROR2("ERROR: snd_ctl_open, card=%d: %s\n", card, snd_strerror(err));
	    } else {
		err = snd_ctl_card_info(handle, info);
		if (err < 0) {
		    ERROR2("ERROR: snd_ctl_card_info, card=%d: %s\n", card, snd_strerror(err));
		} else {
		    dev = -1;
		    while (doContinue) {
			if (snd_ctl_pcm_next_device(handle, &dev) < 0) {
			    ERROR0("snd_ctl_pcm_next_device\n");
			}
			if (dev < 0) {
			    break;
			}
			snd_pcm_info_set_device(pcminfo, dev);
			snd_pcm_info_set_subdevice(pcminfo, 0);
			snd_pcm_info_set_stream(pcminfo, SND_PCM_STREAM_PLAYBACK);
			err = snd_ctl_pcm_info(handle, pcminfo);
			if (err == -ENOENT) {
			    // try with the other direction
			    snd_pcm_info_set_stream(pcminfo, SND_PCM_STREAM_CAPTURE);
			    err = snd_ctl_pcm_info(handle, pcminfo);
			}
			if (err < 0) {
			    if (err != -ENOENT) {
				ERROR2("ERROR: snd_ctl_pcm_info, card=%d: %s", card, snd_strerror(err));
			    }
			} else {
			    if (enumerateSubdevices()) {
				subdeviceCount = snd_pcm_info_get_subdevices_count(pcminfo);
			    } else {
				subdeviceCount = 1;
			    }
			    if (iterator!=NULL) {
				for (subDev = 0; subDev < subdeviceCount; subDev++) {
				    //TRACE3("  Iterating %d,%d,%d\n", card, dev, subDev);
				    doContinue = (*iterator)(devname, card, dev, subDev, handle, pcminfo, info, userData);
				    if (!doContinue) {
					break;
				    }
				}
			    }
			    count += subdeviceCount;
			}
		    } // of while(doContinue)
		}
		snd_ctl_close(handle);
	    }
	    if (snd_card_next(&card) < 0) {
		break;
	    }
	}
    }
    snd_ctl_card_info_free(info);
    snd_pcm_info_free(pcminfo);
    return count;
}

int getAudioDeviceCount() {
    initAlsaSupport();
    return iterateDevices(NULL, NULL);
}

void getDeviceString(char* buffer, int card, int device, int subdevice, int usePlugHw) {
    if (usePlugHw) {
	if (enumerateSubdevices()) {
	    sprintf(buffer, "%s:%d,%d,%d", ALSA_PLUGHARDWARE, card, device, subdevice);
	} else {
	    sprintf(buffer, "%s:%d,%d", ALSA_PLUGHARDWARE, card, device);
	}
    } else {
	if (enumerateSubdevices()) {
	    sprintf(buffer, "%s:%d,%d,%d", ALSA_HARDWARE, card, device, subdevice);
	} else {
	    sprintf(buffer, "%s:%d,%d", ALSA_HARDWARE, card, device);
	}
    }
}

void getDeviceStringFromDeviceID(char* buffer, int deviceID, int usePlugHw) {
    int card, device, subdevice;

    decodeDeviceID(deviceID, &card, &device, &subdevice);
    getDeviceString(buffer, card, device, subdevice, usePlugHw);
}

static int hasGottenALSAVersion = FALSE;
#define ALSAVersionString_LENGTH 200
static char ALSAVersionString[ALSAVersionString_LENGTH];

void getALSAVersion(char* buffer, int len) {
    if (!hasGottenALSAVersion) {
	// get alsa version from proc interface
	FILE* file;
	int curr, len, totalLen, inVersionString;
	file = fopen(ALSA_VERSION_PROC_FILE, "r");
	ALSAVersionString[0] = 0;
	if (file) {
	    fgets(ALSAVersionString, ALSAVersionString_LENGTH, file);
	    // parse for version number
	    totalLen = strlen(ALSAVersionString);
	    inVersionString = FALSE;
	    len = 0;
	    curr = 0;
	    while (curr < totalLen) {
		if (!inVersionString) {
		    // is this char the beginning of a version string ?
		    if (ALSAVersionString[curr] >= '0' && ALSAVersionString[curr] <= '9') {
			inVersionString = TRUE;
		    }
		}
		if (inVersionString) {
		    // the version string ends with white space
		    if (ALSAVersionString[curr] <= 32) {
			break;
		    }
		    if (curr != len) {
			// copy this char to the beginning of the string
			ALSAVersionString[len] = ALSAVersionString[curr];
		    }
		    len++;
		}
		curr++;
	    }
	    // remove trailing dots
	    while ((len > 0) && (ALSAVersionString[len - 1] == '.')) {
	    	len--;
	    }
	    // null terminate
	    ALSAVersionString[len] = 0;
	}
	hasGottenALSAVersion = TRUE;
    }
    strncpy(buffer, ALSAVersionString, len);
}

int deviceInfoIterator(char* devName, int card, int device, int subdevice, 
			snd_ctl_t *handle, snd_pcm_info_t* pcminfo, snd_ctl_card_info_t* info,
		       void* userData) {
    char buffer[300];
    ALSA_AudioDeviceDescription* desc = ( ALSA_AudioDeviceDescription*) userData;
#ifdef USE_PLUGHW
    int usePlugHw = 1;
#else
    int usePlugHw = 0;
#endif

    initAlsaSupport();
    if (desc->index == 0) {
	// we found the device with correct index
	if (enumerateSubdevices()) {
	    *(desc->maxSimultaneousLines) = 1;
	} else {
	    *(desc->maxSimultaneousLines) = snd_pcm_info_get_subdevices_count(pcminfo);
	}
	*desc->deviceID = encodeDeviceID(card, device, subdevice);
	buffer[0]=' '; buffer[1]='[';
	getDeviceString(&(buffer[2]), card, device, subdevice, usePlugHw);
	strcat(buffer, "]");
	strncpy(desc->name, snd_ctl_card_info_get_id(info), desc->strLen - strlen(buffer));
	strncat(desc->name, buffer, desc->strLen - strlen(desc->name));
	strncpy(desc->vendor, "ALSA (http://www.alsa-project.org)", desc->strLen);
	strncpy(desc->description, snd_ctl_card_info_get_name(info), desc->strLen);
	strncat(desc->description, ", ", desc->strLen - strlen(desc->description));
	strncat(desc->description, snd_pcm_info_get_id(pcminfo), desc->strLen - strlen(desc->description));
	strncat(desc->description, ", ", desc->strLen - strlen(desc->description));
	strncat(desc->description, snd_pcm_info_get_name(pcminfo), desc->strLen - strlen(desc->description));
	getALSAVersion(desc->version, desc->strLen);
	TRACE4("Returning %s, %s, %s, %s\n", desc->name, desc->vendor, desc->description, desc->version);
	return FALSE; // do not continue iteration
    }
    desc->index--;
    return TRUE;
}

// returns 0 if successful
int openPCMfromDeviceID(int deviceID, snd_pcm_t** handle, int isSource, int hardware) {
    char buffer[200];
    int ret;

    initAlsaSupport();
    getDeviceStringFromDeviceID(buffer, deviceID, !hardware);
    
    TRACE1("Opening ALSA device %s\n", buffer);
    ret = snd_pcm_open(handle, buffer, 
		       isSource?SND_PCM_STREAM_PLAYBACK:SND_PCM_STREAM_CAPTURE,
		       SND_PCM_NONBLOCK);
    if (ret != 0) {
	ERROR1("snd_pcm_open returned error code %d \n", ret);
	*handle = NULL;
    }
    return ret;
}


int getAudioDeviceDescriptionByIndex(ALSA_AudioDeviceDescription* desc) {
    initAlsaSupport();
    TRACE1(" getAudioDeviceDescriptionByIndex(mixerIndex = %d\n", desc->index);
    iterateDevices(&deviceInfoIterator, desc);
    return (desc->index == 0)?TRUE:FALSE;
}

// returns 1 if successful
// enc: 0 for PCM, 1 for ULAW, 2 for ALAW (see DirectAudio.h)
int getFormatFromAlsaFormat(snd_pcm_format_t alsaFormat, 
			    int* sampleSizeInBytes, int* significantBits, 
			    int* isSigned, int* isBigEndian, int* enc) {

    *sampleSizeInBytes = (snd_pcm_format_physical_width(alsaFormat) + 7) / 8;
    *significantBits = snd_pcm_format_width(alsaFormat);

    // defaults
    *enc = 0; // PCM
    *isSigned = (snd_pcm_format_signed(alsaFormat) > 0);
    *isBigEndian = (snd_pcm_format_big_endian(alsaFormat) > 0);
    
    // non-PCM formats
    if (alsaFormat == SND_PCM_FORMAT_MU_LAW) { // Mu-Law  
	*sampleSizeInBytes = 8; *enc = 1; *significantBits = *sampleSizeInBytes;
    }
    else if (alsaFormat == SND_PCM_FORMAT_A_LAW) {     // A-Law  
	*sampleSizeInBytes = 8; *enc = 2; *significantBits = *sampleSizeInBytes;
    }
    else if (snd_pcm_format_linear(alsaFormat) < 1) {
	return 0;
    }
    return (*sampleSizeInBytes > 0);
}

// returns 1 if successful
int getAlsaFormatFromFormat(snd_pcm_format_t* alsaFormat, 
			    int sampleSizeInBytes, int significantBits, 
			    int isSigned, int isBigEndian, int enc) {
    *alsaFormat = SND_PCM_FORMAT_UNKNOWN;

    if (enc == 0) {
	*alsaFormat = snd_pcm_build_linear_format(significantBits,
						  sampleSizeInBytes * 8,
						  isSigned?0:1,
						  isBigEndian?1:0);
    } 
    else if ((sampleSizeInBytes == 1) && (significantBits == 8)) {
	if (enc == 1) { // ULAW
	    *alsaFormat = SND_PCM_FORMAT_MU_LAW;
	}
	else if (enc == 2) { // ALAW
	    *alsaFormat = SND_PCM_FORMAT_A_LAW;
	}
    }
    return (*alsaFormat == SND_PCM_FORMAT_UNKNOWN)?0:1;
}


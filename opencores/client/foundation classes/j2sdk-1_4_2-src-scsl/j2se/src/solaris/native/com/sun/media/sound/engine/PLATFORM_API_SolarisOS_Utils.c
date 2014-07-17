/*
 * @(#)PLATFORM_API_SolarisOS_Utils.c	1.2 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*****************************************************************************/
/*
**	PLATFORM_API_SolarisOS_Utils.c
**
**	Overview:
**	Platform specfic utility functions for Solaris.
**
**	History	-
**	2002-08-06	$$fb created
*/
/*****************************************************************************/

#define USE_ERROR
//#define USE_TRACE

#include "PLATFORM_API_SolarisOS_Utils.h"

#define MAX_AUDIO_DEVICES 20

// not thread safe...
static AudioDevicePath globalADPaths[MAX_AUDIO_DEVICES];

int getAudioDeviceCount() {
    int count = MAX_AUDIO_DEVICES;
    
    getAudioDevices(globalADPaths, &count);
    return count;
}


int addAudioDevice(char* path, AudioDevicePath* adPath, int* count) {
    int i;
    int found = 0;
    int fileExists = 0;
    // not thread safe...
    static struct stat statBuf;

    // get stats on the file
    if (stat(path, &statBuf) == 0) {
	// file exists.
	fileExists = 1;
	// If it is not yet in the adPath array, add it to the array
	for (i = 0; i < *count; i++) {
	    if (adPath[i].st_ino == statBuf.st_ino 
		&& adPath[i].st_dev == statBuf.st_dev) {
		found = 1;
		break;
	    }
	}
	if (!found) {
	    adPath[*count].st_ino = statBuf.st_ino;
	    adPath[*count].st_dev = statBuf.st_dev;
	    strncpy(adPath[*count].path, path, MAX_NAME_LENGTH);
	    adPath[*count].path[MAX_NAME_LENGTH] = 0;
	    (*count)++;
	    TRACE1("Added audio device %s\n", path);
	}
    }
    return fileExists;
}


void getAudioDevices(AudioDevicePath* adPath, int* count) {
    int maxCount = *count;
    char* audiodev;
    char devsound[15];
    int i;

    *count = 0;
    // first device, if set, is AUDIODEV variable
    audiodev = getenv("AUDIODEV");
    if (audiodev != NULL && audiodev[0] != 0) {
	addAudioDevice(audiodev, adPath, count);
    }
    // then try /dev/audio
    addAudioDevice("/dev/audio", adPath, count);
    // then go through all of the /dev/sound/? devices
    for (i = 0; i < 100; i++) {
	sprintf(devsound, "/dev/sound/%d", i);
	if (!addAudioDevice(devsound, adPath, count)) {
	    break;
	}
    }
    // that's it
}

int getAudioDeviceDescriptionByIndex(int index, AudioDeviceDescription* adDesc, int getNames) {
    int count = MAX_AUDIO_DEVICES;
    int ret = 0;
    
    getAudioDevices(globalADPaths, &count);
    if (index>=0 && index < count) {
	ret = getAudioDeviceDescription(globalADPaths[index].path, adDesc, getNames);
    }
    return ret;
}

int getAudioDeviceDescription(char* path, AudioDeviceDescription* adDesc, int getNames) {
    int fd;
    audio_info_t info;
    audio_device_t deviceInfo;

    strncpy(adDesc->path, path, MAX_NAME_LENGTH);
    adDesc->path[MAX_NAME_LENGTH] = 0;
    strcpy(adDesc->pathctl, adDesc->path);
    strcat(adDesc->pathctl, "ctl");
    strcpy(adDesc->name, adDesc->path);
    adDesc->vendor[0] = 0;
    adDesc->version[0] = 0;
    adDesc->description[0] = 0;
    
    // try to open the pseudo device and get more information
    fd = open(adDesc->pathctl, O_WRONLY | O_NONBLOCK);
    if (fd >= 0) {
	close(fd);
	if (getNames) {
	    fd = open(adDesc->pathctl, O_RDONLY);
	    if (ioctl(fd, AUDIO_GETDEV, &deviceInfo) >= 0) {
		strncpy(adDesc->vendor, deviceInfo.name, MAX_AUDIO_DEV_LEN);
		adDesc->vendor[MAX_AUDIO_DEV_LEN] = 0;
		strncpy(adDesc->version, deviceInfo.version, MAX_AUDIO_DEV_LEN);
		adDesc->version[MAX_AUDIO_DEV_LEN] = 0;
		strncpy(adDesc->description, deviceInfo.config, MAX_AUDIO_DEV_LEN);
		adDesc->description[MAX_AUDIO_DEV_LEN] = 0;
	    }
	}
	return 1;
    }
    return 0;
}


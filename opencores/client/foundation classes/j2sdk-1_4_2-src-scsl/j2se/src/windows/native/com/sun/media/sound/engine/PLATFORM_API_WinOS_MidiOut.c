/*
 * @(#)PLATFORM_API_WinOS_MidiOut.c	1.15 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*****************************************************************************/
/*
**	PLATFORM_API_WinOS_MidiOut.c
**
**	This provides platform specfic MIDI output functions for Windows 95/NT.
**	This implementation does not interface with the HAE engine at all.
**  This interface is for Windows 95/NT and uses the midiOut API to send
**	MIDI messages through the multimedia system.
**
**	Overview:
**
**	History	-
**	06.24.99	Created
**	2002-04-03	$$fb Completely remodeled architecture.
*/
/*****************************************************************************/

//#define USE_ERROR
//#define USE_TRACE

#include "PLATFORM_API_WinOS_Util.h"

#if USE_PLATFORM_MIDI_OUT == TRUE


/* *************************** MidiOutDeviceProvider implementation *********************************** */


INT32	MIDI_OUT_GetNumDevices(void) {
    // add one for the MIDI_MAPPER
    // we want to return it first so it'll be the default, so we
    // decrement each deviceID for these methods....
    return (INT32) (midiOutGetNumDevs() + 1);
}

INT32 getMidiOutCaps(INT32 deviceID, MIDIOUTCAPS* caps) {
    INT32 result;

    if (deviceID == 0) {
	deviceID = MIDI_MAPPER;
    } else {
	deviceID--;
    }
    result = (midiOutGetDevCaps(deviceID, caps, sizeof(MIDIOUTCAPS)) == MMSYSERR_NOERROR);
    if (!result) {
	ERROR1("ERROR: MIDI_OUT_GetDeviceName: failed to get MIDIOUTCAPS for deviceID %d\n", deviceID);
    }
    return result;
}


INT32	MIDI_OUT_GetDeviceName(INT32 deviceID, char *name, UINT32 nameLength) {
    MIDIOUTCAPS midiOutCaps;
    INT32 result = getMidiOutCaps(deviceID, &midiOutCaps);

    if (result) {
	strncpy(name, midiOutCaps.szPname, nameLength-1);
	name[nameLength-1] = 0;
    }
    return result;
}


INT32	MIDI_OUT_GetDeviceVendor(INT32 deviceID, char *name, UINT32 nameLength) {
    /* $$kk: 06.24.99: need to implement */
    return FALSE;
}


INT32	MIDI_OUT_GetDeviceDescription(INT32 deviceID, char *name, UINT32 nameLength) {
    MIDIOUTCAPS midiOutCaps;
    char *desc;
    INT32 result = getMidiOutCaps(deviceID, &midiOutCaps);

    if (result) {
	int tech = (int)midiOutCaps.wTechnology;
	switch(tech) {
	case MOD_MIDIPORT:
	    desc = "External MIDI Port";
	    break;
	case MOD_SQSYNTH:
	    desc = "Internal square wave synthesizer";
	    break;
	case MOD_FMSYNTH:
	    desc = "Internal FM synthesizer";
	    break;
	case MOD_SYNTH:
	    desc = "Internal synthesizer (generic)";
	    break;
	case MOD_MAPPER:
	    desc = "Windows MIDI_MAPPER";
	    break;
	case 7 /* MOD_SWSYNTH*/:
	    desc = "Internal software synthesizer";
	    break;
	default:
	    return FALSE;
	}
	strncpy(name, desc, nameLength-1);
	name[nameLength-1] = 0;
    }
    return result;
}


INT32	MIDI_OUT_GetDeviceVersion(INT32 deviceID, char *name, UINT32 nameLength) {
    MIDIOUTCAPS midiOutCaps;
    INT32 result = getMidiOutCaps(deviceID, &midiOutCaps);

    if (result && nameLength>7) {
	sprintf(name, "%d.%d", (midiOutCaps.vDriverVersion & 0xFF00) >> 8, midiOutCaps.vDriverVersion & 0xFF);
    }
    return result;
}


/* *************************** MidiOutDevice implementation ***************************************** */


void unprepareLongBuffers(MidiDeviceHandle* handle) {
    SysExQueue* sysex;
    MMRESULT err = MMSYSERR_NOERROR;
    int i;

    if (!handle || !handle->deviceHandle || !handle->longBuffers) {
	ERROR0("MIDI_OUT_unprepareLongBuffers: handle, deviceHandle, or longBuffers == NULL\n")
	    return;
    }
    sysex = (SysExQueue*) handle->longBuffers;
    for (i = 0; i<sysex->count; i++) {
	MIDIHDR* hdr = &(sysex->header[i]);
	if (hdr->dwFlags) {
	    err = midiOutUnprepareHeader((HMIDIOUT) handle->deviceHandle, hdr, sizeof(MIDIHDR));
	}
    }
    if (err != MMSYSERR_NOERROR) {
	ERROR0("MIDI_OUT_unprepareLongBuffers: Error occured at midiOutUnprepareHeader\n")
	    }
}

void freeLongBuffer(MIDIHDR* hdr, HMIDIOUT deviceHandle, INT32 minToLeaveData) {
    if (!hdr) {
	ERROR0("MIDI_OUT_freeLongBuffer: hdr == NULL\n")
	    return;
    }
    if (hdr->dwFlags && deviceHandle) {
	midiOutUnprepareHeader(deviceHandle, hdr, sizeof(MIDIHDR));
    }
    if (hdr->lpData && (((INT32) hdr->dwBufferLength) < minToLeaveData || minToLeaveData < 0)) {
	free(hdr->lpData);
	hdr->lpData=NULL;
	hdr->dwBufferLength=0;
    }
    hdr->dwBytesRecorded=0;
    hdr->dwFlags=0;
}

void freeLongBuffers(MidiDeviceHandle* handle) {
    SysExQueue* sysex;
    int i;

    if (!handle || !handle->longBuffers) {
	ERROR0("MIDI_OUT_freeLongBuffers: handle or longBuffers == NULL\n")
	    return;
    }
    sysex = (SysExQueue*) handle->longBuffers;
    for (i = 0; i<sysex->count; i++) {
	freeLongBuffer(&(sysex->header[i]), (HMIDIOUT) handle->deviceHandle, -1);
    }
}

MidiDeviceHandle* MIDI_OUT_OpenDevice(INT32 deviceID) {
    MMRESULT err;
    MidiDeviceHandle* handle;

    TRACE1(">> MIDI_OUT_OpenDevice: deviceID: %d\n", deviceID);

    if (deviceID == 0) {
	deviceID = MIDI_MAPPER;
    } else {
	deviceID--;
    }
#ifdef USE_ERROR
    setvbuf(stdout, NULL, (int)_IONBF, 0);
    setvbuf(stderr, NULL, (int)_IONBF, 0);
#endif

    handle = (MidiDeviceHandle*) malloc(sizeof(MidiDeviceHandle));
    if (!handle) {
	ERROR0("ERROR: MIDI_OUT_OpenDevice: out of memory\n")
	    return NULL;
    }
    memset(handle, 0, sizeof(MidiDeviceHandle));

    // create long buffer queue
    if (!MIDI_WinCreateEmptyLongBufferQueue(handle, MIDI_OUT_LONG_QUEUE_SIZE)) {
	ERROR0("ERROR: MIDI_OUT_OpenDevice: could not create long Buffers\n")
	    free(handle);
	return NULL;
    }

    // create notification event
    handle->platformData = (void*) CreateEvent(NULL, FALSE /*manual reset*/, FALSE /*signaled*/, NULL);
    if (!handle->platformData) {
	ERROR0("ERROR: MIDI_OUT_StartDevice: could not create event\n")
	    MIDI_WinDestroyLongBufferQueue(handle);
	free(handle);
	return NULL; // failure
    }

    // finally open the device
    err = midiOutOpen((HMIDIOUT*) &(handle->deviceHandle), deviceID,
		      (UINT_PTR) handle->platformData, (UINT_PTR) handle, CALLBACK_EVENT);

    if ( (err != MMSYSERR_NOERROR) || (!handle->deviceHandle) ) {
	/* some devices return non zero, but no error! */
	if (midiOutShortMsg((HMIDIOUT) (handle->deviceHandle),0) == MMSYSERR_INVALHANDLE) {
	    ERROR1("ERROR: MIDI_OUT_OpenDevice: midiOutOpen returned an error: %d\n", err)
		CloseHandle((HANDLE) handle->platformData);
	    MIDI_WinDestroyLongBufferQueue(handle);
	    free(handle);
	    return NULL;
	}
    }
    //$$fb enable high resolution time
    timeBeginPeriod(1);
    TRACE0("<< MIDI_OUT_OpenDevice: succeeded\n")
	return handle;
}

INT32	MIDI_OUT_CloseDevice(MidiDeviceHandle* handle) {
    MMRESULT err = MMSYSERR_NOERROR;
    HANDLE event;

    TRACE0("> MIDI_OUT_CloseDevice\n")
	if (!handle) {
	    ERROR0("ERROR: MIDI_IN_StopDevice: handle is NULL\n")
		return FALSE; // failure
	}
    // encourage MIDI_OUT_SendLongMessage to return soon
    event = handle->platformData;
    handle->platformData = NULL;
    if (event) {
	SetEvent(event);
    } else {
	ERROR0("ERROR: MIDI_IN_StopDevice: event is NULL\n")
	    }

    if (handle->deviceHandle) {
	//$$fb disable high resolution time
	timeEndPeriod(1);
	err = midiOutReset((HMIDIOUT) handle->deviceHandle);
	if (err != MMSYSERR_NOERROR) {
	    ERROR0("ERROR: MIDI_OUT_ResetDevice returned error.\n")
		}
    } else {
	ERROR0("ERROR: MIDI_IN_StopDevice: deviceHandle is NULL\n")
	    }

    // issue a "SUSTAIN OFF" message to each MIDI channel, 0 to 15.
    // "CONTROL CHANGE" is 176, "SUSTAIN CONTROLLER" is 64, and the value is 0.
    // $$kk: 07.16.99: MidiOutReset is supposed to do this.  it does turn notes off for each channel, but
    // does not seem to turn off the sustain controller, which appears to override the all-notes-off message.
    // $$fb 2002-04-04: It is responsability of the application developer to
    // leave the device in a consistent state. So I put this in comments
    /*
      for (channel = 0; channel < 16; channel++)
      MIDI_OUT_SendShortMessage(deviceHandle, (unsigned char)(176 + channel), (unsigned char)64, (unsigned char)0, (UINT32)-1);
    */

    if (event) {
	// wait until MIDI_OUT_SendLongMessage has finished
	while (handle->isWaiting) Sleep(0);
    }

    unprepareLongBuffers(handle);

    if (handle->deviceHandle) {
	err = midiOutClose((HMIDIOUT) handle->deviceHandle);
	if (err != MMSYSERR_NOERROR) {
	    ERROR0("ERROR: MIDI_OUT_CloseDevice returned error.\n")
		}
	handle->deviceHandle = NULL;
    }
    freeLongBuffers(handle);

    if (event) {
	CloseHandle(event);
    }
    MIDI_WinDestroyLongBufferQueue(handle);
    free(handle);

    TRACE0("< MIDI_OUT_CloseDevice\n")
	return TRUE; // success
}


INT32	MIDI_OUT_SendShortMessage(MidiDeviceHandle* handle, UINT32 packedMsg, UINT32 timestamp) {
    MMRESULT err = MMSYSERR_NOERROR;

    TRACE2("> MIDI_OUT_SendShortMessage %x, time: %d\n", packedMsg, timestamp)
	if (!handle) {
	    ERROR0("ERROR: MIDI_IN_SendShortMessage: handle is NULL\n")
		return FALSE; // failure
	}
    err = midiOutShortMsg((HMIDIOUT) handle->deviceHandle, packedMsg);
    TRACE1("< MIDI_OUT_SendShortMessage %s\n", (err == MMSYSERR_NOERROR)?"success":"failed")
	return (err == MMSYSERR_NOERROR);
}

INT32	MIDI_OUT_SendLongMessage(MidiDeviceHandle* handle, UBYTE* data, UINT32 size, UINT32 timestamp) {
    MMRESULT err;
    SysExQueue* sysex;
    MIDIHDR* hdr = NULL;
    int i;

    TRACE2("> MIDI_OUT_SendLongMessage size %d, time: %d\n", size, timestamp)
	if (!handle || !data || !handle->longBuffers) {
	    ERROR0("ERROR: MIDI_IN_SendLongMessage: handle, data, or longBuffers is NULL\n")
		return FALSE; // failure
	}
    if (size == 0) {
	return TRUE;
    }

    sysex = (SysExQueue*) handle->longBuffers;
    while (!hdr && handle->platformData) {
	/* find a non-queued header */
	for (i = 0; i < sysex->count; i++) {
	    hdr = &(sysex->header[i]);
	    if ((hdr->dwFlags & MHDR_DONE) || (hdr->dwFlags == 0)) {
		break;
	    }
	    hdr = NULL;
	}
	/* wait for a buffer to free up */
	if (!hdr && handle->platformData) {
	    DWORD res;
	    handle->isWaiting = TRUE;
	    res = WaitForSingleObject((HANDLE) handle->platformData, 700);
	    handle->isWaiting = FALSE;
	    if (res == WAIT_TIMEOUT) {
				// break out back to Java if no buffer freed up after 700 milliseconds
		break;
	    }
	}
    }
    if (hdr) {
	freeLongBuffer(hdr, handle->deviceHandle, (INT32) size);
	if (hdr->lpData == NULL) {
	    hdr->lpData = malloc(size);
	    hdr->dwBufferLength = size;
	}
	hdr->dwBytesRecorded = size;
	memcpy(hdr->lpData, data, size);
	err = midiOutPrepareHeader((HMIDIOUT) handle->deviceHandle, hdr, sizeof(MIDIHDR));
	if (err != MMSYSERR_NOERROR) {
	    freeLongBuffer(hdr, handle->deviceHandle, -1);
	    ERROR0("ERROR: MIDI_OUT_SendLongMessage: midiOutPrepareHeader returned error.\n")
		return FALSE;
	}
	err = midiOutLongMsg((HMIDIOUT) handle->deviceHandle, hdr, sizeof(MIDIHDR));
	if (err != MMSYSERR_NOERROR) {
	    freeLongBuffer(hdr, handle->deviceHandle, -1);
	    ERROR0("ERROR: MIDI_OUT_SendLongMessage: midiOutLongMsg returned error.\n")
		return FALSE;
	}
	TRACE0("< MIDI_OUT_SendLongMessage success\n")
	    return TRUE;
    }
    return FALSE;
}

#endif // USE_PLATFORM_MIDI_OUT

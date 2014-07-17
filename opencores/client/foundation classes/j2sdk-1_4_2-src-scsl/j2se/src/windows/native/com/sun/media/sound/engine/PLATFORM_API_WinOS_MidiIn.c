/*
 * @(#)PLATFORM_API_WinOS_MidiIn.c	1.17 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*****************************************************************************/
/*
**	PLATFORM_API_WinOS_MidiIn.c
**
**	Overview:
**	This provides platform specfic MIDI input functions for Windows.
**	This implementation does not interface with the HAE engine at all.
**      This interface is for Windows and uses the midiIn API to receive
**	MIDI messages through the multimedia system.
**
**	History	-
**	06.24.99	Created
**	2002-04-03	$$fb Completely remodeled architecture.
*/
/*****************************************************************************/

//#define USE_ERROR
//#define USE_TRACE

#include "PLATFORM_API_WinOS_Util.h"

#if USE_PLATFORM_MIDI_IN == TRUE

/*
 * Callback from the MIDI device for all messages.
 */
//$$fb dwParam1 holds a pointer for long messages. How can that be a DWORD then ???
void CALLBACK MIDI_IN_PutMessage( HMIDIIN hMidiIn, UINT wMsg, UINT_PTR dwInstance, UINT_PTR dwParam1, UINT_PTR dwParam2 ) {

    MidiDeviceHandle* handle = (MidiDeviceHandle*) dwInstance;

    TRACE3("MIDI_IN_PutMessage, hMidiIn: %x, wMsg: %x, dwInstance: %x\n", hMidiIn, wMsg, dwInstance)
	TRACE2("                    dwParam1: %x, dwParam2: %x\n", dwParam1, dwParam2)

	switch(wMsg) {

	case MIM_OPEN:
	    TRACE0("MIDI_IN_PutMessage: MIM_OPEN\n")
		break;

	case MIM_CLOSE:
	    TRACE0("MIDI_IN_PutMessage: MIM_CLOSE\n")
	    break;

	case MIM_MOREDATA:
	case MIM_DATA:
	    TRACE3("MIDI_IN_PutMessage: MIM_MOREDATA or MIM_DATA. status=%x  data1=%x  data2=%x\n",
		   dwParam1 & 0xFF, (dwParam1 & 0xFF00)>>8, (dwParam1 & 0xFF0000)>>16)
		if (handle!=NULL && handle->queue!=NULL && handle->platformData) {
		    MIDI_QueueAddShort(handle->queue,
				       // queue stores packedMsg in big endian
				       //(dwParam1 << 24) | ((dwParam1 << 8) & 0xFF0000) | ((dwParam1 >> 8) & 0xFF00),
				       (UINT32) dwParam1,
				       // queue uses microseconds
				       ((INT64) dwParam2)*1000,
				       // overwrite if queue is full
				       TRUE);
		    SetEvent((HANDLE) handle->platformData);
		}
	    break;

	case MIM_LONGDATA:
	    TRACE0("MIDI_IN_PutMessage: MIM_LONGDATA\n")
		if (handle!=NULL && handle->queue!=NULL && handle->platformData) {
		    MIDIHDR* hdr = (MIDIHDR*) dwParam1;
		    MIDI_QueueAddLong(handle->queue,
				      (UBYTE*) hdr->lpData,
				      (UINT32) hdr->dwBytesRecorded,
				      // sysex buffer index
				      (INT32) hdr->dwUser,
				      // queue uses microseconds
				      ((INT64) dwParam2)*1000,
				      // overwrite if queue is full
				      TRUE);
		    SetEvent((HANDLE) handle->platformData);
		}
	    break;

	case MIM_ERROR:
	    ERROR0("ERROR: MIDI_IN_PutMessage: MIM_ERROR!\n")
		break;

	case MIM_LONGERROR:
	    ERROR0("ERROR: MIDI_IN_PutMessage: MIM_LONGERROR!\n")
		break;

	default:
	    ERROR1("ERROR: MIDI_IN_PutMessage: unknown message %d!\n", wMsg)
		break;

	} // switch (wMsg)
}


// PLATFORM_MIDI_IN method implementations


INT32	MIDI_IN_GetNumDevices(void) {
    return (INT32) midiInGetNumDevs();
}

INT32 getMidiInCaps(INT32 deviceID, MIDIINCAPS* caps) {
    return (midiInGetDevCaps(deviceID, caps, sizeof(MIDIINCAPS)) == MMSYSERR_NOERROR);
}

INT32	MIDI_IN_GetDeviceName(INT32 deviceID, char *name, UINT32 nameLength) {
    MIDIINCAPS midiInCaps;
    if (getMidiInCaps(deviceID, &midiInCaps)) {
	strncpy(name, midiInCaps.szPname, nameLength-1);
	name[nameLength-1] = 0;
	return TRUE;
    }
    return FALSE;
}


INT32	MIDI_IN_GetDeviceVendor(INT32 deviceID, char *name, UINT32 nameLength) {
    return FALSE;
}


INT32	MIDI_IN_GetDeviceDescription(INT32 deviceID, char *name, UINT32 nameLength) {
    return FALSE;
}



INT32	MIDI_IN_GetDeviceVersion(INT32 deviceID, char *name, UINT32 nameLength) {
    MIDIINCAPS midiInCaps;
    if (getMidiInCaps(deviceID, &midiInCaps) && (nameLength>7)) {
	sprintf(name, "%d.%d", (midiInCaps.vDriverVersion & 0xFF00) >> 8, midiInCaps.vDriverVersion & 0xFF);
	return TRUE;
    }
    return FALSE;
}


void prepareBuffers(MidiDeviceHandle* handle) {
    SysExQueue* sysex;
    MMRESULT res = MMSYSERR_NOERROR;
    int i;

    if (!handle || !handle->longBuffers || !handle->deviceHandle) {
	ERROR0("MIDI_IN_prepareBuffers: handle, or longBuffers, or deviceHandle==NULL\n")
	    return;
    }
    sysex = (SysExQueue*) handle->longBuffers;
    for (i = 0; i<sysex->count; i++) {
	MIDIHDR* hdr = &(sysex->header[i]);
	midiInPrepareHeader((HMIDIIN) handle->deviceHandle, hdr, sizeof(MIDIHDR));
	res = midiInAddBuffer((HMIDIIN) handle->deviceHandle, hdr, sizeof(MIDIHDR));
    }
    if (res != MMSYSERR_NOERROR) {
	ERROR0("MIDI_IN_prepareBuffers: Error occured at midiInPrepareHeader\n")
	    }
}

void unprepareBuffers(MidiDeviceHandle* handle) {
    SysExQueue* sysex;
    MMRESULT res = MMSYSERR_NOERROR;
    int i;

    if (!handle || !handle->longBuffers || !handle->deviceHandle) {
	ERROR0("MIDI_IN_unprepareBuffers: handle, or longBuffers, or deviceHandle==NULL\n")
	    return;
    }
    sysex = (SysExQueue*) handle->longBuffers;
    for (i = 0; i<sysex->count; i++) {
	res = midiInUnprepareHeader((HMIDIIN) handle->deviceHandle, &(sysex->header[i]), sizeof(MIDIHDR));
    }
    if (res != MMSYSERR_NOERROR) {
	ERROR0("MIDI_IN_unprepareBuffers: Error occured at midiInUnprepareHeader\n")
	    }
}

MidiDeviceHandle* MIDI_IN_OpenDevice(INT32 deviceID) {
    MMRESULT err;
    MidiDeviceHandle* handle;

    TRACE0("MIDI_IN_OpenDevice\n")
#ifdef USE_ERROR
    setvbuf(stdout, NULL, (int)_IONBF, 0);
    setvbuf(stderr, NULL, (int)_IONBF, 0);
#endif

    handle = (MidiDeviceHandle*) malloc(sizeof(MidiDeviceHandle));
    if (!handle) {
	ERROR0("ERROR: MIDI_IN_OpenDevice: out of memory\n")
	return NULL;
    }
    memset(handle, 0, sizeof(MidiDeviceHandle));

    // create queue
    handle->queue = MIDI_CreateQueue(MIDI_IN_MESSAGE_QUEUE_SIZE);
    if (!handle->queue) {
	ERROR0("ERROR: MIDI_IN_OpenDevice: could not create queue\n")
	free(handle);
	return NULL;
    }

    // create long buffer queue
    if (!MIDI_WinCreateLongBufferQueue(handle, MIDI_IN_LONG_QUEUE_SIZE, MIDI_IN_LONG_MESSAGE_SIZE, NULL)) {
	ERROR0("ERROR: MIDI_IN_OpenDevice: could not create long Buffers\n")
	MIDI_DestroyQueue(handle->queue);
	free(handle);
	return NULL;
    }

    // finally open the device
    err = midiInOpen( (HMIDIIN*) &(handle->deviceHandle), deviceID, (UINT_PTR)&(MIDI_IN_PutMessage), (UINT_PTR)handle, CALLBACK_FUNCTION|MIDI_IO_STATUS);

    if ( (err != MMSYSERR_NOERROR) || (!handle->deviceHandle) ) {
	ERROR1("ERROR: MIDI_IN_OpenDevice: midiInOpen returned an error: %d\n", err)
	MIDI_WinDestroyLongBufferQueue(handle);
	MIDI_DestroyQueue(handle->queue);
	free(handle);
	return NULL;
    }

    prepareBuffers(handle);

    TRACE0("MIDI_IN_OpenDevice: midiInOpen succeeded\n")
	return handle;
}


INT32	MIDI_IN_CloseDevice(MidiDeviceHandle* handle) {
    MMRESULT err;

    TRACE0("> MIDI_IN_CloseDevice: midiInClose\n")
	if (!handle) {
	    ERROR0("ERROR: MIDI_IN_CloseDevice: handle is NULL\n")
	    return 0; // failure
	}
    midiInReset((HMIDIIN) handle->deviceHandle);
    unprepareBuffers(handle);
    err = midiInClose((HMIDIIN) handle->deviceHandle);
    handle->deviceHandle=NULL;
    if (err != MMSYSERR_NOERROR) {
	ERROR1("ERROR: MIDI_IN_CloseDevice: midiInClose returned an error: %d\n", err)
	    }
    MIDI_WinDestroyLongBufferQueue(handle);

    if (handle->queue!=NULL) {
	MidiMessageQueue* queue = handle->queue;
	handle->queue = NULL;
	MIDI_DestroyQueue(queue);
    }
    free(handle);

    TRACE0("< MIDI_IN_CloseDevice: midiInClose succeeded\n")
	return TRUE; // success
}


INT32	MIDI_IN_StartDevice(MidiDeviceHandle* handle) {
    MMRESULT err;

    if (!handle || !handle->deviceHandle || !handle->queue) {
	ERROR0("ERROR: MIDI_IN_StartDevice: handle or queue is NULL\n")
	    return 0; // failure
    }

    // clear all the events from the queue
    MIDI_QueueClear(handle->queue);

    handle->platformData = (void*) CreateEvent(NULL, FALSE /*manual reset*/, FALSE /*signaled*/, NULL);
    if (!handle->platformData) {
	ERROR0("ERROR: MIDI_IN_StartDevice: could not create event\n")
	    return 0; // failure
    }

    err = midiInStart((HMIDIIN) handle->deviceHandle);

    if (err != MMSYSERR_NOERROR) {
	ERROR1("ERROR: MIDI_IN_StartDevice: midiInStart returned an error: %d\n", err)
	    return 0; // failure
    }
    TRACE0("MIDI_IN_StartDevice: midiInStart succeeded\n")
	return 1; // success
}


INT32	MIDI_IN_StopDevice(MidiDeviceHandle* handle) {
    MMRESULT err;
    HANDLE event;

    TRACE0("> MIDI_IN_StopDevice: midiInStop \n")
	if (!handle || !handle->platformData) {
	    ERROR0("ERROR: MIDI_IN_StopDevice: handle or event is NULL\n")
		return 0; // failure
	}
    // encourage MIDI_IN_GetMessage to return soon
    event = handle->platformData;
    handle->platformData = NULL;
    SetEvent(event);

    err = midiInStop((HMIDIIN) handle->deviceHandle);

    // wait until the Java thread has exited
    while (handle->isWaiting) Sleep(0);
    CloseHandle(event);

    if (err != MMSYSERR_NOERROR) {
	ERROR1("ERROR: MIDI_IN_StopDevice: midiInStop returned an error: %d\n", err)
	    return 0; // failure
    }
    TRACE0("< MIDI_IN_StopDevice: midiInStop succeeded\n")
	return 1; // success
}


// read the next message from the queue
MidiMessage* MIDI_IN_GetMessage(MidiDeviceHandle* handle) {
    if (handle == NULL) {
	return NULL;
    }
    while (handle->queue!=NULL && handle->platformData!=NULL) {
	MidiMessage* msg = MIDI_QueueRead(handle->queue);
	DWORD res;
	if (msg != NULL) {
	    //fprintf(stdout, "GetMessage returns index %d\n", msg->data.l.index); fflush(stdout);
	    return msg;
	}
	//TRACE0("MIDI_IN_GetMessage: before waiting\n")
	handle->isWaiting = TRUE;
	res = WaitForSingleObject((HANDLE) handle->platformData, 2000);
	handle->isWaiting = FALSE;
	if (res == WAIT_TIMEOUT) {
	    // break out back to Java from time to time - just to be sure
	    break;
	}
	//TRACE0("MIDI_IN_GetMessage: waiting finished\n")
    }
    return NULL;
}

void MIDI_IN_ReleaseMessage(MidiDeviceHandle* handle, MidiMessage* msg) {
    SysExQueue* sysex;
    if (handle == NULL || handle->queue == NULL) {
	return;
    }
    sysex = (SysExQueue*) handle->longBuffers;
    if (msg->type == LONG_MESSAGE && sysex) {
	MIDIHDR* hdr = &(sysex->header[msg->data.l.index]);
	//fprintf(stdout, "ReleaseMessage index %d\n", msg->data.l.index); fflush(stdout);
	hdr->dwBytesRecorded = 0;
	midiInAddBuffer((HMIDIIN) handle->deviceHandle, hdr, sizeof(MIDIHDR));
    }

    MIDI_QueueRemove(handle->queue, TRUE /*onlyLocked*/);
}

#endif // USE_PLATFORM_MIDI_IN

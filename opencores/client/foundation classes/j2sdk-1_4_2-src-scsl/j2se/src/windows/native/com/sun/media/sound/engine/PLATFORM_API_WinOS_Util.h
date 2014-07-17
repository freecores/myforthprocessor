/*
 * @(#)PLATFORM_API_WinOS_Util.h	1.2 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef WIN32_EXTRA_LEAN
#define WIN32_EXTRA_LEAN
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <mmsystem.h>
#include "PlatformMidi.h"

#ifndef PLATFORM_API_WINOS_UTIL_INCLUDED
#define PLATFORM_API_WINOS_UTIL_INCLUDED

#if (USE_PLATFORM_MIDI_IN == TRUE) || (USE_PLATFORM_MIDI_OUT == TRUE)

typedef struct tag_SysExQueue {
    int count;         // number of sys ex headers
    int size;          // data size per sys ex header
    int ownsLinearMem; // true when linearMem is to be disposed
    UBYTE* linearMem;  // where the actual sys ex data is, count*size bytes
    MIDIHDR header[1]; // Windows specific structure to hold meta info
} SysExQueue;

// the buffers do not contain memory
int MIDI_WinCreateEmptyLongBufferQueue(MidiDeviceHandle* handle, int count);
int MIDI_WinCreateLongBufferQueue(MidiDeviceHandle* handle, int count, int size, UBYTE* preAllocatedMem);
void MIDI_WinDestroyLongBufferQueue(MidiDeviceHandle* handle);

#endif // USE_PLATFORM_MIDI_IN || USE_PLATFORM_MIDI_OUT
#endif // PLATFORM_API_WINOS_UTIL_INCLUDED

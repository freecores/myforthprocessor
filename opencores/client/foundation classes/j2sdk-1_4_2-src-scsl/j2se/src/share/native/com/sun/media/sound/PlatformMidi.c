/*
 * @(#)PlatformMidi.c	1.3 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * $$fb: 2002-04-03
 * This file contains utility functions for platform midi support.
 * This does not interface with the Beatnik engine at all.
 */

#define USE_ERROR
//#define USE_TRACE

#include "PlatformMidi.h"

#if (USE_PLATFORM_MIDI_IN == TRUE) || (USE_PLATFORM_MIDI_OUT == TRUE)

// MessageQueue implementation

MidiMessageQueue* MIDI_CreateQueue(int capacity) {
    MidiMessageQueue* queue = (MidiMessageQueue*) malloc(sizeof(MidiMessageQueue) + ((capacity-1) * sizeof(MidiMessage)));
    if (queue) {
	TRACE0("MIDI_CreateQueue\n");
	queue->lock = MIDI_CreateLock();
	queue->capacity = capacity;
	queue->size = 0;
	queue->readIndex = 0;
	queue->writeIndex = 0;
    }
    return queue;
}

void MIDI_DestroyQueue(MidiMessageQueue* queue) {
    if (queue) {
	void* lock = queue->lock;
	MIDI_Lock(lock);
	free(queue);
	MIDI_Unlock(lock);
	MIDI_DestroyLock(lock);
	TRACE0("MIDI_DestroyQueue\n");
    }
}

// if overwrite is true, oldest messages will be overwritten when the queue is full
// returns true, if message has been added
int MIDI_QueueAddShort(MidiMessageQueue* queue, UINT32 packedMsg, INT64 timestamp, int overwrite) {
    if (queue) {
	MIDI_Lock(queue->lock);
	if (queue->size == queue->capacity) {
	    TRACE0("MIDI_QueueAddShort: overflow\n");
	    if (!overwrite || queue->queue[queue->writeIndex].locked) {
		return FALSE; // failed
	    }
	    // adjust overwritten readIndex
	    queue->readIndex = (queue->readIndex+1) % queue->capacity;
	} else {
	    queue->size++;
	}
	TRACE2("MIDI_QueueAddShort. index=%d, size=%d\n", queue->writeIndex, queue->size);
	queue->queue[queue->writeIndex].type = SHORT_MESSAGE;
	queue->queue[queue->writeIndex].data.s.packedMsg = packedMsg;
	queue->queue[queue->writeIndex].timestamp = timestamp;
	queue->writeIndex = (queue->writeIndex+1) % queue->capacity;
	MIDI_Unlock(queue->lock);
	return TRUE;
    }
    return FALSE;
}

int MIDI_QueueAddLong(MidiMessageQueue* queue, UBYTE* data, UINT32 size,
		      INT32 sysexIndex, INT64 timestamp, int overwrite) {
    if (queue) {
	MIDI_Lock(queue->lock);
	if (queue->size == queue->capacity) {
	    TRACE0("MIDI_QueueAddLong: overflow\n");
	    if (!overwrite || queue->queue[queue->writeIndex].locked) {
		return FALSE; // failed
	    }
	    // adjust overwritten readIndex
	    queue->readIndex = (queue->readIndex+1) % queue->capacity;
	} else {
	    queue->size++;
	}
	TRACE2("MIDI_QueueAddLong. index=%d, size=%d\n", queue->writeIndex, queue->size);
	//fprintf(stdout, "MIDI_QueueAddLong sysex-index %d\n", sysexIndex); fflush(stdout);
	queue->queue[queue->writeIndex].type = LONG_MESSAGE;
	queue->queue[queue->writeIndex].data.l.size = size;
	queue->queue[queue->writeIndex].data.l.data = data;
	queue->queue[queue->writeIndex].data.l.index = sysexIndex;
	queue->queue[queue->writeIndex].timestamp = timestamp;
	queue->writeIndex = (queue->writeIndex+1) % queue->capacity;
	MIDI_Unlock(queue->lock);
	return TRUE;
    }
    return FALSE;
}

// returns NULL if no messages in queue.
MidiMessage* MIDI_QueueRead(MidiMessageQueue* queue) {
    MidiMessage* msg = NULL;
    if (queue) {
	MIDI_Lock(queue->lock);
	if (queue->size > 0) {
	    msg = &(queue->queue[queue->readIndex]);
	    TRACE2("MIDI_QueueRead. index=%d, size=%d\n", queue->readIndex, queue->size);
	    msg->locked = TRUE;
	}
	MIDI_Unlock(queue->lock);
    }
    return msg;
}

void MIDI_QueueRemove(MidiMessageQueue* queue, INT32 onlyLocked) {
    if (queue) {
	MIDI_Lock(queue->lock);
	if (queue->size > 0) {
	    MidiMessage* msg = &(queue->queue[queue->readIndex]);
	    if (!onlyLocked || msg->locked) {
		TRACE2("MIDI_QueueRemove. index=%d, size=%d\n", queue->readIndex, queue->size);
		queue->readIndex = (queue->readIndex+1) % queue->capacity;
		queue->size--;
	    }
	    msg->locked = FALSE;
	}
	MIDI_Unlock(queue->lock);
    }
}

void MIDI_QueueClear(MidiMessageQueue* queue) {
    if (queue) {
	MIDI_Lock(queue->lock);
	queue->size = 0;
	queue->readIndex = 0;
	queue->writeIndex = 0;
	MIDI_Unlock(queue->lock);
    }
}

#endif



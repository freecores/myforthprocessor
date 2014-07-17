/*
 * @(#)popFrames.c	1.12 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
#include <string.h>
#include <stdlib.h>
#include <jni.h>
#include <jvmdi.h>

#include "JDWP.h"
#include "popFrames.h"
#include "util.h"
#include "threadControl.h"
#include "eventHandler.h"

/*
 * popFrameEventLock is used to notify that the event has been received 
 */
static JVMDI_RawMonitor popFrameEventLock = NULL;

/*
 * popFrameProceedLock is used to assure that the event thread is
 * re-suspended immediately after the event is acknowledged.
 */
static JVMDI_RawMonitor popFrameProceedLock = NULL;

static void
initLocks()
{
    if (popFrameEventLock == NULL) {
        popFrameEventLock = debugMonitorCreate("JDWP PopFrame Event Lock");
        popFrameProceedLock = debugMonitorCreate("JDWP PopFrame Proceed Lock");
    }
}

/**
 * special event handler for events on the popped thread
 * that occur during the pop operation.
 */
static jboolean
eventDuringPop(JVMDI_Event *event, jthread thread)
{
    switch (event->kind) {
        
    case JVMDI_EVENT_SINGLE_STEP:
      /* this is an event we requested to mark the */
      /* completion of the pop frame */

      debugMonitorEnter(popFrameProceedLock);
      {
          /* notify that we got the event */
          debugMonitorEnter(popFrameEventLock);
          {
              threadControl_setPopFrameEvent(thread, JNI_TRUE);
              debugMonitorNotify(popFrameEventLock);
          }
          debugMonitorExit(popFrameEventLock);

          /* make sure we get suspended again */
          threadControl_setPopFrameProceed(thread, JNI_FALSE);
          while (threadControl_getPopFrameProceed(thread) == JNI_FALSE) {
              debugMonitorWait(popFrameProceedLock);
          }
      }
      debugMonitorExit(popFrameProceedLock);

      /* ignored by eventHandler */
      return JNI_TRUE;  


    case JVMDI_EVENT_BREAKPOINT:
    case JVMDI_EVENT_EXCEPTION:
    case JVMDI_EVENT_FIELD_ACCESS:
    case JVMDI_EVENT_FIELD_MODIFICATION:
    case JVMDI_EVENT_METHOD_ENTRY:
    case JVMDI_EVENT_METHOD_EXIT:
    case JVMDI_EVENT_THREAD_START:
    case JVMDI_EVENT_THREAD_END:
      /* ignored by us and eventHandler */
      return JNI_TRUE;  

    case JVMDI_EVENT_USER_DEFINED: 
    case JVMDI_EVENT_VM_DEATH: 
        ERROR_MESSAGE_EXIT(
             "Thread-less events should not come here\n");

    case JVMDI_EVENT_CLASS_UNLOAD: 
    case JVMDI_EVENT_CLASS_LOAD: 
    case JVMDI_EVENT_CLASS_PREPARE: 
    default:
        /* we don't handle these, don't consume them */
        return JNI_FALSE; 
    }
}

/**
 * Pop one frame off the stack of thread.
 * popFrameEventLock is already held
 */
static jvmdiError
popOneFrame(jthread thread)
{
    jvmdiError error;

    error = jvmdi->PopFrame(thread);
    if (error != JVMDI_ERROR_NONE) {
        return error;
    }
    
    /* resume the popped thread so that the pop occurs and so we */
    /* will get the event (step or method entry) after the pop */
    error = jvmdi->ResumeThread(thread);
    if (error != JVMDI_ERROR_NONE) {
        return error;
    }

    /* wait for the event to occur */
    threadControl_setPopFrameEvent(thread, JNI_FALSE);
    while (threadControl_getPopFrameEvent(thread) == JNI_FALSE) {
        debugMonitorWait(popFrameEventLock);
    }

    /* make sure not to suspend until the popped thread is on the wait */
    debugMonitorEnter(popFrameProceedLock);
    {
        /* return popped thread to suspended state */
        error = jvmdi->SuspendThread(thread);

        /* notify popped thread so it can proceed when resumed */
        threadControl_setPopFrameProceed(thread, JNI_TRUE);
        debugMonitorNotify(popFrameProceedLock);
    }
    debugMonitorExit(popFrameProceedLock);

    return error;
}

/**
 * Return the number of frames that would need to be popped
 * to pop the specified frame.  Negative for error.
 */
#define RETURN_ON_ERROR(call)                     \
    do {                                          \
        if ((jvmdi->call) != JVMDI_ERROR_NONE) {  \
            return -1;                            \
        }                                         \
    } while (0)

static jint
computeFramesToPop(jthread thread, jframeID frame)
{
    jframeID candidate;
    jint popCount = 1;

    RETURN_ON_ERROR(GetCurrentFrame(thread, &candidate));
    while (frame != candidate) {
        jclass clazz;
        jmethodID method;
        jlocation location;
        jboolean isNative=JNI_FALSE;

        RETURN_ON_ERROR(GetCallerFrame(candidate, &candidate));
        RETURN_ON_ERROR(GetFrameLocation(candidate, 
                                         &clazz, &method, &location));
        
        /* When class is redefined only non EMCP java methods are marked */
        /* obsolete. So count this obsolete method for frames to Pop. */
        if (method != OBSOLETE_METHOD_ID) {
            RETURN_ON_ERROR(IsMethodNative(clazz, method, &isNative));
        }

        /* native methods will be popped automatically so don't count them */
        if (!isNative) {
            ++popCount;
        }
    }
    return popCount;
}

/**
 * pop frames of the stack of 'thread' until 'frame' is popped.
 */
jvmdiError
popFrames_pop(jthread thread, jframeID frame)
{
    jvmdiError error = JVMDI_ERROR_NONE;
    jint prevStepMode;
    jint framesPopped = 0;
    jint popCount;

    initLocks();

    /* enable instruction level single step, but first note prev value */
    prevStepMode = threadControl_getInstructionStepMode(thread);
    error = threadControl_setEventMode(JVMDI_ENABLE, 
                                       JVMDI_EVENT_SINGLE_STEP, thread);
    if (error != JVMDI_ERROR_NONE) {
        return error;
    }

    /* compute the number of frames to pop */
    popCount = computeFramesToPop(thread, frame);
    if (popCount < 1) {
        return JVMDI_ERROR_INVALID_FRAMEID;
    }

    /* redirect all events on this thread to eventDuringPop() */
    redirectedEventThread = thread;
    redirectedEventFunction = eventDuringPop;

    debugMonitorEnter(popFrameEventLock);

    /* pop frames using single step */
    while (framesPopped++ < popCount) {
        error = popOneFrame(thread);
        if (error != JVMDI_ERROR_NONE) {
            break;
        }
    }

    debugMonitorExit(popFrameEventLock);

    /*  Reset StepRequest info (fromLine and stackDepth) after popframes
     *  only if stepping is enabled.
     */
    if (prevStepMode == JVMDI_ENABLE) {
        stepControl_resetRequest(thread);
    }
    
    /* restore state */
    threadControl_setEventMode(prevStepMode, 
                               JVMDI_EVENT_SINGLE_STEP, thread);
    redirectedEventThread = NULL;
    redirectedEventFunction = NULL;

    return error;
}

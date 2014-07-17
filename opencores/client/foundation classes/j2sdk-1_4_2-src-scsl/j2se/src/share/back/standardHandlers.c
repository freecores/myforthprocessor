/*
 * @(#)standardHandlers.c	1.3 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*
 * handlers
 *
 * The default event request handler functions
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <jvmdi.h>
#include "eventHandler.h"
#include "threadControl.h"
#include "eventHelper.h"
#include "classTrack.h"
#include "JDWP.h"
#include "util.h"


static void 
handleClassPrepare(JNIEnv *env, JVMDI_Event *event,
                   HandlerNode *node,
                   struct bag *eventBag)
{
    jthread thread = event->u.class_event.thread;
    jclass clazz = event->u.class_event.clazz;

    /* We try hard to avoid class loads/prepares in debugger
     * threads, but it is still possible for them to happen (most
     * likely for exceptions that are thrown within JNI
     * methods). If such an event occurs, we must report it, but
     * we cannot suspend the debugger thread.
     * 
     * 1) We report the thread as NULL because we don't want the
     *    application to get hold of a debugger thread object. 
     * 2) We try to do the right thing wrt to suspending
     *    threads without suspending debugger threads. If the
     *    requested suspend policy is NONE, there's no problem. If
     *    the requested policy is ALL, we can just suspend all
     *    application threads without producing any surprising
     *    results by leaving the debugger thread running. However,
     *    if the requested policy is EVENT_THREAD, we are forced
     *    to do something different than requested. The most
     *    useful behavior is to suspend all application threads
     *    (just as if the policy was ALL). This allows the
     *    application to operate on the class before it gets into
     *    circulation and so it is preferable to the other
     *    alternative of suspending no threads.  
     */
    if (threadControl_isDebugThread(thread)) { 
        event->u.class_event.thread = NULL;
        if (node->suspendPolicy == JDWP_SuspendPolicy_EVENT_THREAD) {
            node->suspendPolicy = JDWP_SuspendPolicy_ALL;
        }
    }
    eventHelper_recordEvent(event, node->handlerID, 
                            node->suspendPolicy, eventBag);
}

static void 
handleClassPrepareTracking(JNIEnv *env, JVMDI_Event *event,
                           HandlerNode *node, 
                           struct bag *eventBag)
{
    jclass clazz = event->u.class_event.clazz;

    /* Record that the prepare occurred for class tracking
     * purposes.  
     */
    classTrack_addPreparedClass(env, clazz);
}


static void 
handleClassUnload(JNIEnv *env, JVMDI_Event *event,
                  HandlerNode *node,
                  struct bag *eventBag)
{
    JDI_ASSERT_MSG(JNI_FALSE, "Should never call handleClassUnload");
}

static void 
handleFrameEvent(JNIEnv *env, JVMDI_Event *event,
                 HandlerNode *node,
                 struct bag *eventBag)
{
    /*
     * The frame id that comes with this event is very transient.
     * We can't send the frameID to the helper thread because it
     * might be useless by the time the helper thread can use it 
     * (if suspend policy is NONE). So, get the needed info from 
     * the frame and then use a special command to the helper
     * thread.
     */
    
    jclass clazz;
    jmethodID method;
    jlocation location;
    JVMDI_frame_event_data *eventData = &event->u.frame;
    jint error;
    
    error = threadControl_getFrameLocation(eventData->thread, 
                                           eventData->frame,
                                           &clazz, &method, 
                                           &location);
    if (error == JVMDI_ERROR_NONE) {
        (*env)->DeleteGlobalRef(env, clazz);
    } else {
        location = -1;
    }

    eventHelper_recordFrameEvent(node->handlerID, 
                                 node->suspendPolicy,
                                 (jbyte)event->kind, 
                                 eventData->thread, 
                                 eventData->clazz, 
                                 eventData->method, 
                                 location, eventBag);
}

static void 
handleUserDefined(JNIEnv *env, JVMDI_Event *event,
                  HandlerNode *node, 
                  struct bag *eventBag)
{
}

static void  
genericHandler(JNIEnv *env, JVMDI_Event *event,
               HandlerNode *node,  
               struct bag *eventBag)
{ 
    eventHelper_recordEvent(event, node->handlerID, node->suspendPolicy, 
                            eventBag); 
}

HandlerFunction
standardHandlers_defaultHandler(jint kind) 
{
    switch (kind) {
        case JVMDI_EVENT_BREAKPOINT:
        case JVMDI_EVENT_EXCEPTION:
        case JVMDI_EVENT_FIELD_ACCESS:
        case JVMDI_EVENT_FIELD_MODIFICATION:
        case JVMDI_EVENT_SINGLE_STEP:
        case JVMDI_EVENT_THREAD_START:
        case JVMDI_EVENT_THREAD_END:
        case JVMDI_EVENT_VM_DEATH: 
            return genericHandler;

        case JVMDI_EVENT_USER_DEFINED: 
            return handleUserDefined;

        case JVMDI_EVENT_CLASS_PREPARE: 
            return handleClassPrepare;

        case JVMDI_EVENT_CLASS_UNLOAD: 
            return handleClassUnload;

        case JVMDI_EVENT_METHOD_ENTRY:
        case JVMDI_EVENT_METHOD_EXIT:
            return handleFrameEvent;

        default:
            ERROR_MESSAGE_EXIT("Attempt to install handler for invalid event");
            return NULL;
    }
}

void 
standardHandlers_onConnect()
{
    jboolean installed;

    /* always report VMDeath to a connected debugger */
    installed = (eventHandler_createInternal(
        JVMDI_EVENT_VM_DEATH, genericHandler) != NULL);
    if (!installed) {
        ERROR_MESSAGE_EXIT(
            "Unable to install VM Death event handler");
    }
    /* for class tracking need internal class prepare handler */
    installed = (eventHandler_createInternal(
        JVMDI_EVENT_CLASS_PREPARE, 
        handleClassPrepareTracking) != NULL);
    if (!installed) {
        ERROR_MESSAGE_EXIT(
     "Unable to install Class Prepare tracking event handler");
    }
}

void 
standardHandlers_onDisconnect()
{
}

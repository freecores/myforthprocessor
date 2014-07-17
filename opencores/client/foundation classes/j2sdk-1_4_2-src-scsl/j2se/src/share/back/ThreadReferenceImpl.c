/*
 * @(#)ThreadReferenceImpl.c	1.58 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
#include <string.h>
#include <stdlib.h>

#include "ThreadReferenceImpl.h"
#include "util.h"
#include "eventHandler.h"
#include "threadControl.h"
#include "inStream.h"
#include "outStream.h"
#include "JDWP.h"

static jboolean 
name(PacketInputStream *in, PacketOutputStream *out) 
{
    JNIEnv *env = getEnv();
    JVMDI_thread_info info;
    jthread thread = inStream_readThreadRef(in);
    if (inStream_error(in)) {
        return JNI_TRUE;
    }

    if (threadControl_isDebugThread(thread)) {
        outStream_setError(out, JVMDI_ERROR_INVALID_THREAD);
        return JNI_TRUE;
    }

    threadInfo(thread, &info);
    outStream_writeString(out, info.name);

    (*env)->DeleteGlobalRef(env, info.thread_group);
    if (info.context_class_loader != NULL) {
        (*env)->DeleteGlobalRef(env, info.context_class_loader);
    }
    jdwpFree(info.name);
    return JNI_TRUE;
}

static jboolean 
suspend(PacketInputStream *in, PacketOutputStream *out) 
{
    jint error;
    jthread thread = inStream_readThreadRef(in);
    if (inStream_error(in)) {
        return JNI_TRUE;
    }

    if (threadControl_isDebugThread(thread)) {
        outStream_setError(out, JVMDI_ERROR_INVALID_THREAD);
        return JNI_TRUE;
    }
    error = threadControl_suspendThread(thread, JNI_FALSE);
    if (error != JVMDI_ERROR_NONE) {
        outStream_setError(out, error);
    }
    return JNI_TRUE;
}

static jboolean 
resume(PacketInputStream *in, PacketOutputStream *out) 
{
    jint error;
    jthread thread = inStream_readThreadRef(in);
    if (inStream_error(in)) {
        return JNI_TRUE;
    }

    if (threadControl_isDebugThread(thread)) {
        outStream_setError(out, JVMDI_ERROR_INVALID_THREAD);
        return JNI_TRUE;
    }

    /* true means it is okay to unblock the commandLoop thread */
    error = threadControl_resumeThread(thread, JNI_TRUE);
    if (error != JVMDI_ERROR_NONE) {
        outStream_setError(out, error);
    }
    return JNI_TRUE;
}

static jboolean 
status(PacketInputStream *in, PacketOutputStream *out) 
{
    jint threadStatus;
    jint suspendStatus;
    jint error;
    jthread thread = inStream_readThreadRef(in);
    if (inStream_error(in)) {
        return JNI_TRUE;
    }

    if (threadControl_isDebugThread(thread)) {
        outStream_setError(out, JVMDI_ERROR_INVALID_THREAD);
        return JNI_TRUE;
    }

    error = threadControl_applicationThreadStatus(thread, &threadStatus, 
                                                          &suspendStatus);
    if (error != JVMDI_ERROR_NONE) {
        outStream_setError(out, error);
        return JNI_TRUE;
    }
    outStream_writeInt(out, threadStatus);

    /* 
     * Mask off at-breakpoint status which is not passed back to front
     * end since it is not necessarily accurate at that level.
     */
    outStream_writeInt(out, suspendStatus & ~JVMDI_SUSPEND_STATUS_BREAK);
    return JNI_TRUE;
}

static jboolean 
threadGroup(PacketInputStream *in, PacketOutputStream *out) 
{
    JNIEnv *env = getEnv();
    JVMDI_thread_info info;
    jthread thread = inStream_readThreadRef(in);
    if (inStream_error(in)) {
        return JNI_TRUE;
    }

    if (threadControl_isDebugThread(thread)) {
        outStream_setError(out, JVMDI_ERROR_INVALID_THREAD);
        return JNI_TRUE;
    }

    threadInfo(thread, &info);
    WRITE_GLOBAL_REF(env, out, info.thread_group);
    
    if (info.context_class_loader != NULL) {
        (*env)->DeleteGlobalRef(env, info.context_class_loader);
    }
    jdwpFree(info.name);
    return JNI_TRUE;
}

static jboolean 
validateSuspendedThread(PacketOutputStream *out, jthread thread)
{
    jint error;
    jint count;
    error = threadControl_suspendCount(thread, &count);
    if (error != JVMDI_ERROR_NONE) {
        outStream_setError(out, error);
        return JNI_FALSE;
    }

    if (count == 0) {
        outStream_setError(out, JVMDI_ERROR_THREAD_NOT_SUSPENDED);
        return JNI_FALSE;
    }

    return JNI_TRUE;
}

static jboolean 
frames(PacketInputStream *in, PacketOutputStream *out) 
{
    JNIEnv *env = getEnv();
    jint error;
    jint i;
    jint count;
    jframeID frame;

    jthread thread = inStream_readThreadRef(in);
    jint startIndex = inStream_readInt(in);
    jint length = inStream_readInt(in);
    if (inStream_error(in)) {
        return JNI_TRUE;
    }

    if (threadControl_isDebugThread(thread)) {
        outStream_setError(out, JVMDI_ERROR_INVALID_THREAD);
        return JNI_TRUE;
    }

    if (!validateSuspendedThread(out, thread)) {
        return JNI_TRUE;
    }

    error = frameCount(thread, &count);
    if (error != JVMDI_ERROR_NONE) {
        outStream_setError(out, error);
        return JNI_TRUE;
    }

    if (length == -1) {
        length = count - startIndex;
    }

    if (length == 0) {
        outStream_writeInt(out, 0);
        return JNI_TRUE;
    }

    if ((startIndex < 0) || (startIndex > count - 1)) {
        outStream_setError(out, JDWP_Error_INVALID_INDEX);
        return JNI_TRUE;
    }

    if ((length < 0) || (length + startIndex > count)) {
        outStream_setError(out, JDWP_Error_INVALID_LENGTH);
        return JNI_TRUE;
    }

    outStream_writeInt(out, length);
    error = jvmdi->GetCurrentFrame(thread, &frame);
    i = 0;
    while ((i < startIndex + length) && (error == JVMDI_ERROR_NONE)) { 
        if (i >= startIndex) {
            jclass clazz;
            jmethodID method;
            jlocation location;
    
            /* Get location info for the frame */
            error = threadControl_getFrameLocation(thread, frame, 
                                          &clazz, &method, &location);
            if (error == JVMDI_ERROR_OPAQUE_FRAME) {
                location = -1L;
                error = JVMDI_ERROR_NONE;
            } else if (error != JVMDI_ERROR_NONE) {
                break;
            }

            outStream_writeFrameID(out, frame);
            writeCodeLocation(out, clazz, method, location);
        } 
        error = jvmdi->GetCallerFrame(frame, &frame);
        i++;
    }

    /* ignore the final out-of-frames error */
    if ((i == startIndex + length) && (error == JVMDI_ERROR_NO_MORE_FRAMES)) {
        error = JVMDI_ERROR_NONE;
    }

    if (error != JVMDI_ERROR_NONE) {
        outStream_setError(out, error);
    }
    return JNI_TRUE;
}

static jboolean 
getFrameCount(PacketInputStream *in, PacketOutputStream *out) 
{
    JNIEnv *env = getEnv();
    jint error;
    jint count;

    jthread thread = inStream_readThreadRef(in);
    if (inStream_error(in)) {
        return JNI_TRUE;
    }

    if (threadControl_isDebugThread(thread)) {
        outStream_setError(out, JVMDI_ERROR_INVALID_THREAD);
        return JNI_TRUE;
    }

    if (!validateSuspendedThread(out, thread)) {
        return JNI_TRUE;
    }

    error = frameCount(thread, &count);
    if (error != JVMDI_ERROR_NONE) {
        outStream_setError(out, error);
        return JNI_TRUE;
    }
    outStream_writeInt(out, count);

    return JNI_TRUE;
}

static jboolean 
ownedMonitors(PacketInputStream *in, PacketOutputStream *out)
{
    JNIEnv *env = getEnv();
    jint error;
    JVMDI_owned_monitor_info info;
    jint i;

    jthread thread = inStream_readThreadRef(in);
    if (inStream_error(in)) {
        return JNI_TRUE;
    }

    if (threadControl_isDebugThread(thread)) {
        outStream_setError(out, JVMDI_ERROR_INVALID_THREAD);
        return JNI_TRUE;
    }

    if (!validateSuspendedThread(out, thread)) {
        return JNI_TRUE;
    }

    error = jvmdi->GetOwnedMonitorInfo(thread, &info);
    if (error != JVMDI_ERROR_NONE) {
        outStream_setError(out, error);
        return JNI_TRUE;
    }

    outStream_writeInt(out, info.owned_monitor_count);
    for (i = 0; i < info.owned_monitor_count; i++) {
        jobject monitor = info.owned_monitors[i];
        outStream_writeByte(out, specificTypeKey(monitor));
        WRITE_GLOBAL_REF(env, out, monitor);
    }

    if (info.owned_monitor_count != 0) {
        jdwpFree(info.owned_monitors);
    }
    return JNI_TRUE;
}

static jboolean 
currentContendedMonitor(PacketInputStream *in, PacketOutputStream *out)
{
    JNIEnv *env = getEnv();
    jobject monitor;
    jint error;

    jthread thread = inStream_readThreadRef(in);
    if (inStream_error(in)) {
        return JNI_TRUE;
    }

    if (threadControl_isDebugThread(thread)) {
        outStream_setError(out, JVMDI_ERROR_INVALID_THREAD);
        return JNI_TRUE;
    }

    if (!validateSuspendedThread(out, thread)) {
        return JNI_TRUE;
    }

    error = jvmdi->GetCurrentContendedMonitor(thread, &monitor);
    if (error != JVMDI_ERROR_NONE) {
        outStream_setError(out, error);
        return JNI_TRUE;
    }

    outStream_writeByte(out, specificTypeKey(monitor));
    WRITE_GLOBAL_REF(env, out, monitor);
    return JNI_TRUE;
}

static jboolean 
stop(PacketInputStream *in, PacketOutputStream *out) 
{
    jint error;
    jthread thread = inStream_readThreadRef(in);
    jobject throwable = inStream_readObjectRef(in);
    if (inStream_error(in)) {
        return JNI_TRUE;
    }

    if (threadControl_isDebugThread(thread)) {
        outStream_setError(out, JVMDI_ERROR_INVALID_THREAD);
        return JNI_TRUE;
    }

    error = threadControl_stop(thread, throwable);
    if (error != JVMDI_ERROR_NONE) {
        outStream_setError(out, error);
    }
    return JNI_TRUE;
}

static jboolean 
interrupt(PacketInputStream *in, PacketOutputStream *out) 
{
    jint error;
    jthread thread = inStream_readThreadRef(in);
    if (inStream_error(in)) {
        return JNI_TRUE;
    }

    if (threadControl_isDebugThread(thread)) {
        outStream_setError(out, JVMDI_ERROR_INVALID_THREAD);
        return JNI_TRUE;
    }

    error = threadControl_interrupt(thread);
    if (error != JVMDI_ERROR_NONE) {
        outStream_setError(out, error);
    }
    return JNI_TRUE;
}

static jboolean 
suspendCount(PacketInputStream *in, PacketOutputStream *out) 
{
    jint error;
    jint count;
    jthread thread = inStream_readThreadRef(in);
    if (inStream_error(in)) {
        return JNI_TRUE;
    }

    if (threadControl_isDebugThread(thread)) {
        outStream_setError(out, JVMDI_ERROR_INVALID_THREAD);
        return JNI_TRUE;
    }

    error = threadControl_suspendCount(thread, &count);
    if (error != JVMDI_ERROR_NONE) {
        outStream_setError(out, error);
        return JNI_TRUE;
    }

    outStream_writeInt(out, count);
    return JNI_TRUE;
}

void *ThreadReference_Cmds[] = { (void *)12,
    (void *)name,
    (void *)suspend,
    (void *)resume,
    (void *)status,
    (void *)threadGroup,
    (void *)frames,
    (void *)getFrameCount,
    (void *)ownedMonitors,
    (void *)currentContendedMonitor,
    (void *)stop,
    (void *)interrupt,
    (void *)suspendCount
    };



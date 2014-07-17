/*
 * @(#)EventRequestImpl.c	1.32 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
#include "EventRequestImpl.h"
#include "eventHandler.h"
#include "util.h"
#include "inStream.h"
#include "outStream.h"
#include "JDWP.h"
#include "stepControl.h"

#define JDWPEventType_to_JVMDIEventKind(eventType) (eventType)
    
#define ERROR_RETURN(func) \
{jvmdiError error = (func); if (error != JVMDI_ERROR_NONE) return error; }

/**
 * Take JDWP "modifiers" (which are JDI explicit filters, like
 * addCountFilter(), and implicit filters, like the LocationOnly
 * filter that goes with breakpoints) and add them as filters
 * (eventFilter) to the HandlerNode (eventHandler).
 */
static jvmdiError
readAndSetFilters(PacketInputStream *in, HandlerNode *node, 
                  jint filterCount)
{
    jthread threadOnly = NULL;
    JNIEnv *env = getEnv();
    jint i;

    for (i = 0; i < filterCount; ++i) {
        jbyte modifier = inStream_readByte(in);
        switch (modifier) {
            case JDWP_REQUEST_MODIFIER(Conditional): {
                jint exprID = inStream_readInt(in);
                ERROR_RETURN(inStream_error(in));
                ERROR_RETURN(eventFilter_setConditionalFilter(node, i, 
                                                               exprID));
                break;
            }

            case JDWP_REQUEST_MODIFIER(Count): {
                jint count = inStream_readInt(in);
                ERROR_RETURN(inStream_error(in));
                ERROR_RETURN(eventFilter_setCountFilter(node, i, 
                                                            count));
                break;
            }

            case JDWP_REQUEST_MODIFIER(ThreadOnly): {
                jthread thread = inStream_readThreadRef(in);
                ERROR_RETURN(inStream_error(in));
                ERROR_RETURN(eventFilter_setThreadOnlyFilter(node, i, 
                                                                thread));
                break;
            }

            case JDWP_REQUEST_MODIFIER(LocationOnly): {
                jbyte tag = inStream_readByte(in); /* not currently used */
                jclass clazz = inStream_readClassRef(in);
                jmethodID method = inStream_readMethodID(in);
                jlocation location = inStream_readLocation(in);
                ERROR_RETURN(inStream_error(in));
                ERROR_RETURN(eventFilter_setLocationOnlyFilter(node, i,
                                             clazz, method, location));
                break;
            }

            case JDWP_REQUEST_MODIFIER(FieldOnly): {
                jclass clazz = inStream_readClassRef(in);
                jfieldID field = inStream_readFieldID(in);
                ERROR_RETURN(inStream_error(in));
                ERROR_RETURN(eventFilter_setFieldOnlyFilter(node, i,
                                                      clazz, field));
                break;
            }

            case JDWP_REQUEST_MODIFIER(ClassOnly): {
                jclass clazz = inStream_readClassRef(in);
                ERROR_RETURN(inStream_error(in));
                ERROR_RETURN(eventFilter_setClassOnlyFilter(node, i, clazz));
                break;
            }

            case JDWP_REQUEST_MODIFIER(ExceptionOnly): {
                jclass exception = inStream_readClassRef(in);
                jboolean caught = inStream_readBoolean(in);
                jboolean uncaught = inStream_readBoolean(in);
                ERROR_RETURN(inStream_error(in));
                ERROR_RETURN(eventFilter_setExceptionOnlyFilter(node, i,
                                             exception, caught, uncaught));
                break;
            }

            case JDWP_REQUEST_MODIFIER(InstanceOnly): {
                jobject instance = inStream_readObjectRef(in);
                ERROR_RETURN(inStream_error(in));
                ERROR_RETURN(eventFilter_setInstanceOnlyFilter(node, i,
								instance));
                break;
            }

            case JDWP_REQUEST_MODIFIER(ClassMatch): {
                char *pattern = inStream_readString(in);
                ERROR_RETURN(inStream_error(in));
                ERROR_RETURN(eventFilter_setClassMatchFilter(node, i,
                                                                pattern));
                break;
            }

            case JDWP_REQUEST_MODIFIER(ClassExclude): {
                char *pattern = inStream_readString(in);
                ERROR_RETURN(inStream_error(in));
                ERROR_RETURN(eventFilter_setClassExcludeFilter(node, i,
                                                                 pattern));
                break;
            }
            case JDWP_REQUEST_MODIFIER(Step): {
                jthread thread = inStream_readThreadRef(in);
                jint size = inStream_readInt(in);
                jint depth = inStream_readInt(in);
                ERROR_RETURN(inStream_error(in));
                ERROR_RETURN(eventFilter_setStepFilter(node, i,
                                                 thread, size, depth));
                break;
            }

            default:
                return JVMDI_ERROR_ILLEGAL_ARGUMENT;
        }
    }
    return JVMDI_ERROR_NONE;
}

/**
 * This is the back-end implementation for enabling
 * (what are at the JDI level) EventRequests.
 *
 * Allocate the event request handler (eventHandler).
 * Add any filters (explicit or implicit).
 * Install the handler.
 * Return the handlerID which is used to map subsequent 
 * events to the EventRequest that created it.
 */
static jboolean 
setCommand(PacketInputStream *in, PacketOutputStream *out) 
{
    jint error;
    jbyte kind;
    HandlerNode *node;
    jbyte eventType = inStream_readByte(in);
    jbyte suspendPolicy = inStream_readByte(in);
    jint filterCount = inStream_readInt(in);

    if (inStream_error(in)) {
        return JNI_TRUE;
    }

    kind = JDWPEventType_to_JVMDIEventKind(eventType);
    node = eventHandler_alloc(filterCount, kind, suspendPolicy);
    if (node == NULL) {
        outStream_setError(out, JVMDI_ERROR_OUT_OF_MEMORY);
        return JNI_TRUE;
    }

    error = readAndSetFilters(in, node, filterCount);
    if (error == JVMDI_ERROR_NONE) {
        error = eventHandler_installExternal(node);
    }

    if (error == JVMDI_ERROR_NONE) {
        outStream_writeInt(out, node->handlerID);
    } else {
        eventHandler_free(node);
        outStream_setError(out, error);
    }

    return JNI_TRUE;
}

/**
 * This is the back-end implementation for disabling
 * (what are at the JDI level) EventRequests.
 */
static jboolean
clearCommand(PacketInputStream *in, PacketOutputStream *out) 
{
    jint error = JVMDI_ERROR_NONE;
    jbyte kind;
    jbyte eventType = inStream_readByte(in);
    HandlerID handlerID = inStream_readInt(in);

    if (inStream_error(in)) {
        return JNI_TRUE;
    }

    kind = JDWPEventType_to_JVMDIEventKind(eventType);

    error = eventHandler_freeByID(kind, handlerID);

    if (error != JVMDI_ERROR_NONE) {
        outStream_setError(out, error);
    }

    return JNI_TRUE;
}

static jboolean 
clearAllBreakpoints(PacketInputStream *in, PacketOutputStream *out) 
{
    jint error = eventHandler_freeAll(JVMDI_EVENT_BREAKPOINT);
    if (error != JVMDI_EVENT_BREAKPOINT) {
        outStream_setError(out, error);
    }
    return JNI_TRUE;
}

void *EventRequest_Cmds[] = { (void *)0x3
    ,(void *)setCommand
    ,(void *)clearCommand
    ,(void *)clearAllBreakpoints};


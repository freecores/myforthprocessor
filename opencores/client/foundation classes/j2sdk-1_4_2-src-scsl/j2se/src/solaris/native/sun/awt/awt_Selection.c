/*
 * @(#)awt_Selection.c	1.54 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifdef HEADLESS
    #error This file should not be included in headless library
#endif

#include "awt_p.h"
#include "awt_DataTransferer.h"
#include "java_awt_datatransfer_Transferable.h"
#include "java_awt_datatransfer_DataFlavor.h"
#include "sun_awt_motif_X11Selection.h"
#include "sun_awt_motif_X11Clipboard.h"
#include "sun_awt_datatransfer_ClipboardTransferable.h"
#include <X11/Intrinsic.h>
#include <X11/Xatom.h>
#include <inttypes.h>

#include <jni.h>
#include <jni_util.h>

typedef enum { 
    SelectionPending, 
    SelectionSuccess, 
    SelectionFailure, 
    SelectionOwnerTimedOut 
} SelectionStatus;

static Atom TARGETS;

/* Should only be accessed by the current owner of AWT_LOCK. */
static SelectionStatus globalSelectionStatus = SelectionPending;

static SelectionStatus get_selection_status() {
    return globalSelectionStatus;
}

static void set_selection_status(SelectionStatus status) {
    globalSelectionStatus = status;
}

/* fieldIDs for X11Selection fields that may be accessed from C */
static struct X11SelectionIDs {
    jfieldID holder;
    jfieldID atom;
    jfieldID contents;
    jfieldID selections;
} x11SelectionIDs;

DECLARE_JAVA_CLASS(selectionClazz, "sun/awt/motif/X11Selection")

static jobject
call_getSelectionsArray(JNIEnv* env) {
    DECLARE_STATIC_OBJECT_JAVA_METHOD(getSelectionsArray, selectionClazz, 
                                    "getSelectionsArray", "()[Ljava/lang/Object;")
    DASSERT(!JNU_IsNull(env, getSelectionsArray));
    return (*env)->CallStaticObjectMethod(env, clazz, getSelectionsArray);
}

/*
 * Class:     sun_awt_motif_MScrollPanePeer
 * Method:    initIDs
 * Signature: ()V
 */

/* This function gets called from the static initializer for
   X11Selection.java to initialize the fieldIDs for fields that may
   be accessed from C */

JNIEXPORT void JNICALL Java_sun_awt_motif_X11Selection_initIDs
    (JNIEnv *env, jclass cls)
{
    x11SelectionIDs.holder = (*env)->
        GetFieldID(env, cls, "holder","Lsun/awt/motif/X11SelectionHolder;");
    x11SelectionIDs.atom = (*env)->GetFieldID(env, cls, "atom", "J");
    x11SelectionIDs.contents = (*env)->
        GetFieldID(env, cls, "contents",
                   "Ljava/awt/datatransfer/Transferable;");
    x11SelectionIDs.selections = (*env)->
        GetStaticFieldID(env, cls, "selections", "Ljava/util/Vector;");
}

/*
 * Class:     sun_awt_motif_X11Selection
 * Method:    init
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_sun_awt_motif_X11Selection_init
    (JNIEnv *env, jclass this) 
{
    AWT_LOCK();

    TARGETS = XInternAtom(awt_display, "TARGETS", False);

    AWT_UNLOCK();
}

static jobject 
getX11Selection(JNIEnv * env, Atom atom)
{
    jobjectArray selections;
    jsize selectionCount, i;
    jobject selection;

    selections = (jobjectArray)call_getSelectionsArray(env);

    if (JNU_IsNull(env, selections)) {
        return NULL;
    }

    selectionCount = (*env)->GetArrayLength(env, selections);

    for (i = 0; i < selectionCount; i++) {
        selection = (*env)->GetObjectArrayElement(env, selections, i);
        if (JNU_IsNull(env, selection) || 
            !JNU_IsNull(env, (*env)->ExceptionOccurred(env))) {
            /* Something went wrong just exit gracefully */
            break;
        }
        if ((*env)->GetLongField(env, selection, x11SelectionIDs.atom)
            == atom) {
            return selection;
        }
    }

    return NULL;
}

Boolean 
awtJNI_isSelectionOwner(JNIEnv * env, char *sel_str)
{
    Atom selection;
    jobject x11sel;

    selection = XInternAtom(awt_display, sel_str, False);

    x11sel = getX11Selection(env, selection);
    if (!JNU_IsNull(env, x11sel)) {
        jobject holder;

        holder = (*env)->GetObjectField(env, x11sel, x11SelectionIDs.holder);
        if (!JNU_IsNull(env, holder)) {
            return TRUE;
        }
    }
    return FALSE;
}

static void losingSelectionOwnership(Widget w, Atom * selection);

void 
awtJNI_notifySelectionLost(JNIEnv * env, char *sel_str)
{
    Atom selection;

    selection = XInternAtom(awt_display, sel_str, False);
    losingSelectionOwnership(NULL, &selection);
}

static void 
losingSelectionOwnership(Widget w, Atom * selection)
{
    JNIEnv *env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);
    jobject this = getX11Selection(env, *selection);

    /* 
     * SECURITY: OK to call this on privileged thread - peer does
     *         not call into client code
     */
    JNU_CallMethodByName(env, NULL, this, "lostSelectionOwnership", "()V");
    if ((*env)->ExceptionOccurred(env)) {
        (*env)->ExceptionDescribe(env);
        (*env)->ExceptionClear(env);
    }
    /*
     * Fix for 4692059. 
     * The native context is cleaned up on the event dispatch thread after the
     * references to the current contents and owner are cleared.
     */
}

static void 
getSelectionTargets(Widget w, XtPointer client_data, Atom * selection,
                    Atom * type, XtPointer value, unsigned long *length,
                    int32_t *format)
{
    JNIEnv *env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);
    jobject* pReturnArray = (jobject*)client_data;
    size_t count = 0, i = 0, j = 0;
    SelectionStatus status = SelectionFailure;

    /*
     * It is highly unlikely that TARGETS will ever be passed even though that
     * was what was requested. However, XA_ATOM ("ATOM") is likely. 
     * Actually they are the same so treat them as such. See XToolKit 
     * Intrinsic Manual on XtSelectionCallbackProc for more details on type.
     */
    if (*type == TARGETS || *type == XA_ATOM) {
        Atom *targets = (Atom *) value;

        /* Get rid of zero atoms if there are any. */
        for (; i < *length; i++) {
            if (targets[i] != 0) {
                count++;
            }
        }

        if (count > 0) {
            jlong* checkedTargets = calloc(count, sizeof(jlong));

            if (checkedTargets == NULL) {
                JNU_ThrowOutOfMemoryError(env, "");
                (*env)->ExceptionDescribe(env);
                (*env)->ExceptionClear(env);
            } else {
                for (i = 0; i < *length; i++) {
                    if (targets[i] != 0) {
                        checkedTargets[j++] = targets[i];
                    }
                }

                DASSERT(j == count);

                if ((*env)->EnsureLocalCapacity(env, 1) >= 0) {

                    jlongArray targetArray = (*env)->NewLongArray(env, count);
                    
                    if (!JNU_IsNull(env, targetArray)) {
                        (*env)->SetLongArrayRegion(env, targetArray, 0, count,
                                                   checkedTargets);
                        
                        if (!JNU_IsNull(env, (*env)->ExceptionOccurred(env))) {
                            (*env)->ExceptionDescribe(env);
                            (*env)->ExceptionClear(env);
                        } else {
                            *pReturnArray =
                                (*env)->NewGlobalRef(env, targetArray);
                            status = SelectionSuccess;
                        }
                        
                        (*env)->DeleteLocalRef(env, targetArray);
                    }
                }
                free(checkedTargets);
            }
        } else {
            /* 
             * Fix for 4655996.
             * Report success if the selection owner doesn't export any target
             * types. 
             */
            status = SelectionSuccess;
        }
    } else if (*type == XT_CONVERT_FAIL) {
        status = SelectionOwnerTimedOut;
    } else {
        /* 
         * Fix for 4655996.
         * Report success if there is no owner for this selection or the owner
         * fails to provide target types.
         */
        status = SelectionSuccess;
    }

    if (value != NULL) {
        XtFree(value);
        value = NULL;
    }

    set_selection_status(status);
}

static void 
getSelectionData(Widget w, XtPointer client_data, Atom * selection,
                 Atom * type, XtPointer value, unsigned long *length,
                 int32_t *format)
{
    JNIEnv *env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);
    jobject* pData = (jobject*)client_data;
    SelectionStatus status = SelectionFailure;

    if (*type == XT_CONVERT_FAIL) {
        status = SelectionOwnerTimedOut;
    } else if (*type != None) {
        if ((*env)->EnsureLocalCapacity(env, 1) >= 0) {
            jsize size = (*length <= INT_MAX) ? *length : INT_MAX;
            jbyteArray array = (*env)->NewByteArray(env, size);

            if (!JNU_IsNull(env, array)) {
                (*env)->SetByteArrayRegion(env, array, 0, size, (jbyte*)value);
                if (!JNU_IsNull(env, (*env)->ExceptionOccurred(env))) {
                    (*env)->ExceptionDescribe(env);
                    (*env)->ExceptionClear(env);
                } else {
                    *pData = (*env)->NewGlobalRef(env, array);
                    status = SelectionSuccess;
                }

                (*env)->DeleteLocalRef(env, array);
            }
        }
    }

    if (value != NULL) {
        XtFree(value);
        value = NULL;
    }

    set_selection_status(status);
}

static int32_t 
WaitForSelectionEvent(void *data)
{
    process_convert_data_requests();
    return get_selection_status() != SelectionPending;
}

/*
 * Class:     sun_awt_motif_X11Selection
 * Method:    pGetSelectionOwnership
 * Signature: (Ljava/lang/Object;Ljava/awt/datatransfer/Transferable;[JLjava/util/Map;Lsun/awt/motif/X11SelectionHolder;)Z
 */
JNIEXPORT jboolean JNICALL 
Java_sun_awt_motif_X11Selection_pGetSelectionOwnership(JNIEnv *env, 
                                                       jobject this,
                                                       jobject source,
                                                       jobject transferable,
                                                       jlongArray formats,
                                                       jobject formatMap,
                                                       jobject holder) 
{
    Boolean gotit = False;
    Atom selection = (Atom)(*env)->GetLongField(env, this, 
                                                x11SelectionIDs.atom);
    awt_convertDataCallbackStruct* structPtr = NULL;
    Time time = CurrentTime;

    AWT_LOCK();

    time = awt_util_getCurrentServerTime();

    (*env)->SetObjectField(env, this, x11SelectionIDs.holder, NULL);
    (*env)->SetObjectField(env, this, x11SelectionIDs.contents, NULL);

    gotit = XtOwnSelection(awt_root_shell, selection, time, awt_convertData,
                           losingSelectionOwnership, NULL);

    if (gotit) {
        if (XFindContext(awt_display, selection, awt_convertDataContext,
                         (XPointer*)&structPtr) == 0 && structPtr != NULL) {
            (*env)->DeleteGlobalRef(env, structPtr->source);
            (*env)->DeleteGlobalRef(env, structPtr->transferable);
            (*env)->DeleteGlobalRef(env, structPtr->formatMap);
            (*env)->DeleteGlobalRef(env, structPtr->formats);
            memset(structPtr, 0, sizeof(awt_convertDataCallbackStruct));
        } else {
            XDeleteContext(awt_display, selection, awt_convertDataContext);

            structPtr = calloc(1, sizeof(awt_convertDataCallbackStruct));

            if (structPtr == NULL) {
                XtDisownSelection(awt_root_shell, selection, time);
                AWT_UNLOCK();
                JNU_ThrowOutOfMemoryError(env, "");
                return JNI_FALSE;
            }

            if (XSaveContext(awt_display, selection, awt_convertDataContext, 
                             (XPointer)structPtr) == XCNOMEM) {
                XtDisownSelection(awt_root_shell, selection, time);
                free(structPtr);
                AWT_UNLOCK();
                JNU_ThrowInternalError(env, "Failed to save context data for selection.");
                return JNI_FALSE;
            }
        }

        structPtr->source = (*env)->NewGlobalRef(env, source);
        structPtr->transferable = (*env)->NewGlobalRef(env, transferable);
        structPtr->formatMap = (*env)->NewGlobalRef(env, formatMap); 
        structPtr->formats = (*env)->NewGlobalRef(env, formats);

        if (JNU_IsNull(env, structPtr->source) ||
            JNU_IsNull(env, structPtr->transferable) ||
            JNU_IsNull(env, structPtr->formatMap) ||
            JNU_IsNull(env, structPtr->formats)) {

            if (!JNU_IsNull(env, structPtr->source)) {
                (*env)->DeleteGlobalRef(env, structPtr->source);
            }
            if (!JNU_IsNull(env, structPtr->transferable)) {
                (*env)->DeleteGlobalRef(env, structPtr->transferable);
            }
            if (!JNU_IsNull(env, structPtr->formatMap)) {
                (*env)->DeleteGlobalRef(env, structPtr->formatMap);
            }
            if (!JNU_IsNull(env, structPtr->formats)) {
                (*env)->DeleteGlobalRef(env, structPtr->formats);
            }
            XtDisownSelection(awt_root_shell, selection, time);
            XDeleteContext(awt_display, selection, awt_convertDataContext);
            free(structPtr);
            AWT_UNLOCK();
            JNU_ThrowOutOfMemoryError(env, "");
            return JNI_FALSE;
        }

        (*env)->SetObjectField(env, this, x11SelectionIDs.holder, holder);
        (*env)->SetObjectField(env, this, x11SelectionIDs.contents, transferable);
    }
    AWT_UNLOCK();

    return (gotit ? JNI_TRUE : JNI_FALSE);
}

/*
 * Class:     sun_awt_motif_X11Selection
 * Method:    clearNativeContext
 * Signature: ()V
 */
JNIEXPORT void JNICALL 
Java_sun_awt_motif_X11Selection_clearNativeContext(JNIEnv *env, jobject this) {
    Atom selection = (Atom)(*env)->GetLongField(env, this, 
                                                x11SelectionIDs.atom);

    AWT_LOCK();

    XtDisownSelection(awt_root_shell, selection, CurrentTime);
    awt_cleanupConvertDataContext(env, selection);

    AWT_UNLOCK();
}

/*
 * Class:     sun_awt_datatransfer_ClipboardTransferable
 * Method:    getClipboardFormats
 * Signature: (J)[J
 */
JNIEXPORT jlongArray JNICALL 
Java_sun_awt_datatransfer_ClipboardTransferable_getClipboardFormats
    (JNIEnv *env, jobject this, jlong selectionAtom)
{
    jlongArray ret     = NULL;
    Atom selection     = (Atom)selectionAtom;
    jlongArray formats = NULL;
    SelectionStatus status = SelectionPending;

    AWT_LOCK();

    set_selection_status(SelectionPending);
    XtGetSelectionValue(awt_root_shell, selection, TARGETS, 
                        getSelectionTargets, (XtPointer)&formats, 
                        awt_util_getCurrentServerTime());
    awt_MToolkit_modalWait(WaitForSelectionEvent, NULL);
    status = get_selection_status();

    AWT_FLUSH_UNLOCK();

    if (!JNU_IsNull(env, formats)) {
        ret = (*env)->NewLocalRef(env, formats);
        (*env)->DeleteGlobalRef(env, formats);
    }

    switch (status) {
    case SelectionSuccess:
        break;
    case SelectionFailure:
        JNU_ThrowIOException(env, "Failed to get selection targets");
        break;
    case SelectionOwnerTimedOut:
        JNU_ThrowIOException(env, "Selection owner timed out");
        break;
    default:
        JNU_ThrowIOException(env, "Unexpected selection status");
        break;
    }

    return ret;
}

/*
 * Class:     sun_awt_datatransfer_ClipboardTransferable
 * Method:    getClipboardData
 * Signature: (JJ)[B
 */
JNIEXPORT jbyteArray JNICALL 
Java_sun_awt_datatransfer_ClipboardTransferable_getClipboardData
    (JNIEnv *env, jobject this, jlong selectionAtom, jlong format)
{
    jbyteArray ret    = NULL;
    Atom selection    = (Atom)selectionAtom;
    Atom target       = (Atom)format;
    jbyteArray data   = NULL;
    SelectionStatus status = SelectionPending;

    AWT_LOCK();

    set_selection_status(SelectionPending);
    XtGetSelectionValue(awt_root_shell, selection, target, 
                        getSelectionData, (XtPointer)&data, 
                        awt_util_getCurrentServerTime());
    
    awt_MToolkit_modalWait(WaitForSelectionEvent, NULL);
    status = get_selection_status();

    AWT_FLUSH_UNLOCK();

    if (!JNU_IsNull(env, data)) {
        ret = (*env)->NewLocalRef(env, data);
        (*env)->DeleteGlobalRef(env, data);
    }

    switch (status) {
    case SelectionSuccess:
        break;
    case SelectionFailure:
        JNU_ThrowIOException(env, "Failed to get selection targets");
        break;
    case SelectionOwnerTimedOut:
        JNU_ThrowIOException(env, "Selection owner timed out");
        break;
    default:
        JNU_ThrowIOException(env, "Unexpected selection status");
        break;
    }

    return ret;
}

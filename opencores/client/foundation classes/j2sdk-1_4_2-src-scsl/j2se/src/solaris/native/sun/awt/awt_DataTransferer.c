/*
 * @(#)awt_DataTransferer.c	1.17 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifdef HEADLESS
    #error This file should not be included in headless library
#endif

#include "awt_p.h"
#include <X11/Intrinsic.h>
#include <X11/Xutil.h>

#include <sys/utsname.h>

#include <jni.h>
#include <jni_util.h>

#include "sun_awt_datatransfer_DataTransferer.h"
#include "sun_awt_motif_MDataTransferer.h"

#include "awt_XmDnD.h"
#include "awt_DataTransferer.h"

static jclass string;   

XContext awt_convertDataContext = 0;

extern struct X11GraphicsConfigIDs x11GraphicsConfigIDs;

/**
 * global function to initialize this client as a Dynamic-only app.
 *
 * gets called once during toolkit initialization.
 */

void awt_initialize_DataTransferer() {
    JNIEnv* env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);
    jclass stringClassLocal = NULL;

    DASSERT(string == NULL);

    stringClassLocal = (*env)->FindClass(env, "java/lang/String");

    if (!JNU_IsNull(env, (*env)->ExceptionOccurred(env))) {
        (*env)->ExceptionDescribe(env);
        (*env)->ExceptionClear(env);
        DASSERT(False);
    }

    if (JNU_IsNull(env, stringClassLocal)) return;

    string = (*env)->NewGlobalRef(env, stringClassLocal); /* never freed! */
    (*env)->DeleteLocalRef(env, stringClassLocal);

    if (JNU_IsNull(env, string)) {
        JNU_ThrowOutOfMemoryError(env, "");
        return;
    }

    DASSERT(awt_convertDataContext == 0);
    awt_convertDataContext = XUniqueContext();
    DASSERT(awt_convertDataContext != 0);

    /*
     * Fix for 4513976.
     * Set a limitation on the selection owner request time
     * to the maximum possible value.
     */
    XtAppSetSelectionTimeout(awt_appContext, ULONG_MAX);
}

/*
 * Single routine to convert to target FILE_NAME or _DT_FILENAME
 */
Boolean 
convertFileType(jbyteArray data, Atom * type, XtPointer * value, 
                unsigned long *length, int32_t *format)
{
    /*
     * Convert the internal representation to an File Name.
     * The data passed is an array of
     * null separated bytes. Each series of bytes is a string
     * that is then converted to an XString which are then put
     * into an XStringList and put into an XTextProperty for
     * usage in other programs.
     *
     * It would be desireable to have dataConvert to this conversion
     * but it isn't possible to return a byte array that represents
     * the XTextProperty.
     */

    JNIEnv*       env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);
    jboolean      isCopy=JNI_FALSE;
    XTextProperty tp;
    jsize         len;
    jsize         strings = 0;
    jsize         i;
    char**        stringList;
    Status        s;
    jbyte*        bytes;
    char*         start;
    size_t        slen;
    char*         utf;

    if ((*env)->PushLocalFrame(env, 16) < 0) {
        return False;
    }

    /* convert the data to an Array of Byte Elements */
    bytes = (*env)->GetByteArrayElements(env, data, &isCopy);
    if (!JNU_IsNull(env, (*env)->ExceptionOccurred(env))) {
        (*env)->ExceptionDescribe(env);
        (*env)->ExceptionClear(env);
        (*env)->PopLocalFrame(env, NULL);
        return False;
    }

    if (JNU_IsNull(env, bytes)) {
        (*env)->PopLocalFrame(env, NULL);
        return False;
    }

    /* Get the length of the area */
    len = (*env)->GetArrayLength(env, data);
    if (len == 0) {
        (*env)->ReleaseByteArrayElements(env, data, bytes, JNI_ABORT);
        (*env)->PopLocalFrame(env, NULL);
        return False;
    }

    /* 
     * determine the number of lists. The byteArray is null separated list of
     * strings.
     */
    for (i = 0; i < len; i++) {
        if (bytes[i] == '\0') {
            strings++;
        }
    }

    /* Allocate an X11 string list */
    stringList = (char **)XtCalloc(strings, sizeof(char *));
    if (stringList == (char**)NULL) {
        (*env)->ReleaseByteArrayElements(env, data, bytes, JNI_ABORT);
        (*env)->PopLocalFrame(env, NULL);
        return False;
    }

    for (i = 0; i < strings; i++) {
        if (i == 0) {
            start = (char*)bytes;
        } else {
            start = (char*)&bytes[slen];
        }

        /* 
         * if start is a NULL we're at the end of the list
         * We'll just a have null entry on the end of the list
         */
        if (start[0] == '\0') {
            stringList[i] = NULL;
            continue;
        }
        slen = strlen(start) + 1;

        stringList[i] = (char*)XtCalloc(slen, sizeof(char));

        if (stringList[i] == (char *)NULL) {
            jsize j;

            (*env)->ReleaseByteArrayElements(env, data, bytes, JNI_ABORT);

            for (j = 0; j < i; j++) {
                XtFree((void *)stringList[j]);
            }

            (*env)->PopLocalFrame(env, NULL);

            return False;
        }

        memcpy((void *)stringList[i], (const void*)start, slen);
    }

    (*env)->ReleaseByteArrayElements(env, data, bytes, JNI_ABORT);
    s = XStringListToTextProperty(stringList, strings, &tp);

    /* free the strings that were created */
    for (i = 0; i < strings; i++) {
        if (stringList[i] != NULL) {
            XtFree((void*)stringList[i]);
        }
    }

    XtFree((void*)stringList);

    if (s == 0) {
        (*env)->PopLocalFrame(env, NULL);
        return False;
    }

    *value = (XtPointer)XtCalloc(tp.nitems, sizeof(char));

    if (*value == (XtPointer)NULL) {
        XFree((void*)tp.value);

        (*env)->PopLocalFrame(env, NULL);

        return False;
    }

    memcpy((void *)(*value), (const void *)tp.value, tp.nitems);

    XFree((void*)tp.value);

    *length = tp.nitems;
    *type   = tp.encoding;
    *format = tp.format;
    (*env)->PopLocalFrame(env, NULL);
    return True;
}

/*
 * Class:     sun_awt_motif_MDataTransferer
 * Method:    getAtomForTarget
 * Signature: (Ljava/lang/String;)J
 */

JNIEXPORT jlong JNICALL
Java_sun_awt_motif_MDataTransferer_getAtomForTarget(JNIEnv *env, 
                                                    jclass cls,
                                                    jstring targetString) 
{
    Atom target;
    char *target_str;

    if (JNU_IsNull(env, targetString)) {
        JNU_ThrowNullPointerException(env, "NullPointerException");
        return -1;
    }
    target_str = (char *) JNU_GetStringPlatformChars(env, targetString, NULL);

    AWT_LOCK();

    target = XInternAtom(awt_display, target_str, False);

    AWT_UNLOCK();

    JNU_ReleaseStringPlatformChars(env, targetString,
                                   (const char *) target_str);
    return target;
}

/*
 * Class:     sun_awt_motif_MDataTransferer
 * Method:    getTargetNameForAtom
 * Signature: (J)Ljava/lang/String;
 */

JNIEXPORT jstring JNICALL 
Java_sun_awt_motif_MDataTransferer_getTargetNameForAtom(JNIEnv *env, 
                                                        jclass cls,
                                                        jlong atom)
{
    jstring targetString;
    char *name;

    AWT_LOCK();

    name = XGetAtomName(awt_display, (Atom) atom);
    
    if (name == NULL) {
        JNU_ThrowNullPointerException(env, "Failed to retrieve atom name.");
        AWT_UNLOCK();
        return NULL;
    }

    targetString = (*env)->NewStringUTF(env, (const char *)name);

    if (!JNU_IsNull(env, (*env)->ExceptionOccurred(env))) {
        (*env)->ExceptionDescribe(env);
        (*env)->ExceptionClear(env);
        XFree (name);
        AWT_UNLOCK();
        return NULL;
    }

    if (JNU_IsNull(env, targetString)) {
        JNU_ThrowNullPointerException(env, "Failed to create a string.");
        XFree (name);
        AWT_UNLOCK();
        return NULL;
    }

    XFree (name);

    AWT_UNLOCK();
    return targetString;
}

/*
 * Class:     sun_awt_datatransfer_DataTransferer
 * Method:    dragQueryFile
 * Signature: ([B)[Ljava/lang/String;
 *
 * This method converts a byte array that came from File most likely from a 
 * drag operation into a String array.
 */

JNIEXPORT jobjectArray JNICALL 
Java_sun_awt_datatransfer_DataTransferer_dragQueryFile
    (JNIEnv *env, jobject this, jbyteArray bytes)
{
    XTextProperty tp;
    jbyte         *value;
    
    char**        strings  = (char **)NULL;
    int32_t       nstrings = 0;
    jobject       filenames;
    jobject       ret = NULL;
    int32_t       i;
    jsize         len;
    jboolean      isCopy=JNI_FALSE;

    /*
     * If the length of the byte array is 0 just return a null
     */
    len = (*env)->GetArrayLength(env, bytes);
    if (len == 0) {
        return NULL;
    }

    value = (*env)->GetByteArrayElements(env, bytes, &isCopy);
    if (JNU_IsNull(env, value)) {
        return NULL;
    }

    AWT_LOCK();

    tp.encoding = XInternAtom(awt_display, "STRING", False);
    tp.value    = (unsigned char *)value;
    tp.nitems   = len;
    tp.format   = 8;

    /*
     * Convert the byte stream into a list of X11 strings
     */
    if (XTextPropertyToStringList(&tp, &strings, &nstrings) == 0 ||
        nstrings == 0)
        {
            (*env)->ReleaseByteArrayElements(env, bytes, value, JNI_ABORT);
            AWT_UNLOCK();
            return NULL;
        }
    
    (*env)->ReleaseByteArrayElements(env, bytes, value, JNI_ABORT);

    filenames = (*env)->NewObjectArray(env, nstrings, string, NULL);

    if (!JNU_IsNull(env, (*env)->ExceptionOccurred(env))) {
        (*env)->ExceptionDescribe(env);
        (*env)->ExceptionClear(env);
        goto wayout;
    }

    if (JNU_IsNull(env, filenames)) {
        goto wayout;
    }
    
    /*
     * Actuall conversion code per X11 String
     */
    for (i = 0; i < nstrings; i++) {
        jstring string = (*env)->NewStringUTF(env, 
                                              (const char *)strings[i]);
        if (!JNU_IsNull(env, (*env)->ExceptionOccurred(env))) {
            (*env)->ExceptionDescribe(env);
            (*env)->ExceptionClear(env);
            goto wayout;
        }

        if (JNU_IsNull(env, string)) {
            goto wayout;
        }
        
        (*env)->SetObjectArrayElement(env, filenames, i, string);
        
        if (!JNU_IsNull(env, (*env)->ExceptionOccurred(env))) {
            (*env)->ExceptionDescribe(env);
            (*env)->ExceptionClear(env);
            goto wayout;
        }

        (*env)->DeleteLocalRef(env, string);
    }

    ret = filenames;
 wayout:
    /*
     * Clean up and return
     */
    XFreeStringList(strings);
    AWT_UNLOCK();
    return ret;
}

DECLARE_JAVA_CLASS(dataTransfererClazz, "sun/awt/datatransfer/DataTransferer")

/**
 * Returns a local reference to the singleton DataTransferer instance.
 * The caller should delete the reference when done.
 */
static jobject 
get_data_transferer(JNIEnv* env) {
    jobject transferer = NULL;

    DECLARE_STATIC_OBJECT_JAVA_METHOD(getInstanceMethodID, dataTransfererClazz, 
                                     "getInstance", 
                                     "()Lsun/awt/datatransfer/DataTransferer;");

    transferer = (*env)->CallStaticObjectMethod(env, clazz, getInstanceMethodID);

    if ((*env)->ExceptionCheck(env) == JNI_TRUE) {
        (*env)->ExceptionDescribe(env);
        (*env)->ExceptionClear(env);
    }

    DASSERT(!JNU_IsNull(env, transferer));

    return transferer;
}    

static jobject
call_convertData(JNIEnv* env, jobject source, jobject contents, jlong format,
                 jobject formatMap) {
    jobject transferer = get_data_transferer(env);
    jobject ret = NULL;
    DECLARE_OBJECT_JAVA_METHOD(convertDataMethodID, dataTransfererClazz, 
                               "convertData", 
                               "(Ljava/lang/Object;Ljava/awt/datatransfer/Transferable;JLjava/util/Map;Z)[B");

    ret = (*env)->CallObjectMethod(env, transferer, convertDataMethodID,
                                   source, contents, format, formatMap,
                                   awt_currentThreadIsPrivileged(env));

    if ((*env)->ExceptionCheck(env) == JNI_TRUE) {
        (*env)->ExceptionDescribe(env);
        (*env)->ExceptionClear(env);
    }

    (*env)->DeleteLocalRef(env, transferer);

    return ret;
}

void
process_convert_data_requests() {
    JNIEnv* env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_4);
    jobject transferer = get_data_transferer(env);

    DECLARE_VOID_JAVA_METHOD(processDataConversionRequestsMethodID, 
                             dataTransfererClazz, 
                             "processDataConversionRequests", 
                             "()V");

    (*env)->CallVoidMethod(env, transferer,
			   processDataConversionRequestsMethodID);

    if ((*env)->ExceptionCheck(env) == JNI_TRUE) {
        (*env)->ExceptionDescribe(env);
        (*env)->ExceptionClear(env);
    }

    (*env)->DeleteLocalRef(env, transferer);
}

Boolean 
awt_convertData(Widget w, Atom * selection, Atom * target, Atom * type,
                XtPointer * value, unsigned long *length, int32_t *format) {
    JNIEnv*  env  = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);
    Display* dpy = XtDisplay(w);
    awt_convertDataCallbackStruct* structPtr = NULL;

    if (XFindContext(dpy, *selection, awt_convertDataContext, 
                     (XPointer*)&structPtr) == XCNOMEM || structPtr == NULL) {
        return False;
    }

    if ((*env)->PushLocalFrame(env, 2) < 0) {
        (*env)->ExceptionDescribe(env);
        (*env)->ExceptionClear(env);
        return False;
    }

    if (*target == XInternAtom(dpy, "TARGETS", False)) {
        jlongArray formats = structPtr->formats;
        jsize      count;
        jlong*     targets;
        jboolean   isCopy;

#ifndef _LP64 /* Atom and jlong are different sizes in the 32-bit build */
        Atom*      aValue;
        jlong*     saveTargets;
        jsize      i;
#endif

        if (JNU_IsNull(env, formats)) {
            (*env)->PopLocalFrame(env, NULL);
            return False;
        }

        count = (*env)->GetArrayLength(env, formats);
        if (count == 0) {
            (*env)->PopLocalFrame(env, NULL);
            return False;
        }

        targets = (*env)->GetLongArrayElements(env, formats, &isCopy);

        *type = XA_ATOM;
        *format = 32;

#ifdef _LP64
        *value = XtMalloc(count * sizeof(Atom));
        memcpy((void *)*value, (void *)targets, count * sizeof(Atom));
#else
        *value = aValue = (Atom *)XtMalloc(count * sizeof(Atom));
        saveTargets = targets;
        for (i = 0; i < count; i++, aValue++, targets++) {
            *aValue = (Atom)*targets;
        }
        targets = saveTargets;
#endif
        (*env)->ReleaseLongArrayElements(env, formats, targets, JNI_ABORT);

        *length = count;

    } else if (*target == XInternAtom(dpy, _XA_DELETE, False)) {

        /*
         * acknowledge the DELETE target here ... the "delete" semantic
         * of move will take place after the drop is complete.
         */

        *type   = XInternAtom(dpy, _XA_NULL, False);
        *length = 0;
        *value  = (XtPointer)NULL;
    } else if (*target == XInternAtom(dpy, _XA_HOSTNAME, False)) {
        struct utsname name;
        XTextProperty  tp;

        uname(&name);

        if (!XStringListToTextProperty((char **)&name.nodename, 1, &tp)) {
            (*env)->PopLocalFrame(env, NULL);
            return False;
        }

        *value = (XtPointer)XtCalloc(tp.nitems, sizeof(char));

        memcpy((void *)*value, (const void *)tp.value, tp.nitems);

        XFree((void *)tp.value);

        *type   = tp.encoding;
        *length = tp.nitems + 1;
        *format = tp.format;
    } else if (*target == XInternAtom(dpy, _XA_FILENAME, False) ||
               *target == XInternAtom(dpy, _DT_FILENAME, False)) {

        /*
         * Convert the internal representation to an File Name.
         * The data returned from dataConvert is a an array of
         * null separated bytes. Each series of bytes is a string
         * that is then converted to an XString which are then put
         * into an XStringList and put into an XTextProperty for
         * usage in other programs.
         *
         * It would be desireable to have dataConvert to this conversion
         * but it isn't possible to return a byte array that represents
         * the XTextProperty.
         */
        jbyteArray    data;
        
        /*
         * Fix for 4513976.
         * Type None should be used instead of XT_CONVERT_FAIL 
         * to report conversion failure. 
         */
        /*  assume forthcoming error */
        *type   = None;
        *value  = (XtPointer)NULL;
        *length = 0;
        *format = 8;

        data = call_convertData(env, structPtr->source, structPtr->transferable,
                                (jlong)*target, structPtr->formatMap);
                                                
        /* error test */
        if (!JNU_IsNull(env, (*env)->ExceptionOccurred(env))) {
            (*env)->ExceptionDescribe(env);
            (*env)->ExceptionClear(env);
            (*env)->PopLocalFrame(env, NULL);
            return False;
        }
        if (JNU_IsNull(env, data)) {
            (*env)->PopLocalFrame(env, NULL);
            return False;
        }

        if (convertFileType(data, type, value, length, format) == False) {
            (*env)->PopLocalFrame(env, NULL);
            return False;
        }
    } else {
        jbyteArray bytes = NULL;
        jbyte*     copy = NULL;

        /*
         * Fix for 4513976.
         * Type None should be used instead of XT_CONVERT_FAIL 
         * to report conversion failure. 
         */
        *type   = None; /* assume forthcoming error */
        *value  = (XtPointer)NULL;
        *length = 0;
        *format = 8;

        bytes = call_convertData(env, structPtr->source, structPtr->transferable,
                                 (jlong)*target, structPtr->formatMap);

        if (!JNU_IsNull(env, (*env)->ExceptionOccurred(env))) {
            (*env)->ExceptionDescribe(env);
            (*env)->ExceptionClear(env);
            (*env)->PopLocalFrame(env, NULL);
            return False;
        }

        if (bytes == NULL) {
            (*env)->PopLocalFrame(env, NULL);
            return False;
        } else {
            jsize len = (*env)->GetArrayLength(env, bytes);

            if (len == 0) {
                *type   = *target;
                *format = 8;
                (*env)->PopLocalFrame(env, NULL);
                return True;
            }

            copy = (jbyte*)XtCalloc(1, len * sizeof(jbyte));
            if (copy == (jbyte*)NULL) {
                JNU_ThrowOutOfMemoryError(env, "OutOfMemoryError");
                (*env)->PopLocalFrame(env, NULL);
                return False;
            }

            (*env)->GetByteArrayRegion(env, (jbyteArray)bytes, 0, len, copy);

            if (!JNU_IsNull(env, (*env)->ExceptionOccurred(env))) {
                (*env)->ExceptionDescribe(env);
                (*env)->ExceptionClear(env);
                XtFree((void *)copy);
                (*env)->PopLocalFrame(env, NULL);
                return False;
            }

            *value  = (XtPointer)copy;
            *type   = *target;
            *length = len;
            *format = 8;
        }
    }

    (*env)->PopLocalFrame(env, NULL);
    return True;
}

void
awt_cleanupConvertDataContext(JNIEnv *env, Atom selectionAtom) {
    awt_convertDataCallbackStruct* structPtr = NULL;

    if (XFindContext(awt_display, selectionAtom, awt_convertDataContext, 
                     (XPointer*)&structPtr) == 0 && structPtr != NULL) {

        (*env)->DeleteGlobalRef(env, structPtr->source);
        (*env)->DeleteGlobalRef(env, structPtr->transferable);
        (*env)->DeleteGlobalRef(env, structPtr->formatMap);
        (*env)->DeleteGlobalRef(env, structPtr->formats);
        free(structPtr);
    } else {
        DASSERT(False);
    }
    /* 
     * Xlib Programming Manual says that it is better to erase
     * the current entry with XDeleteContext() before XSaveContext().
     */
    XDeleteContext(awt_display, selectionAtom, awt_convertDataContext);
    if (XSaveContext(awt_display, selectionAtom, awt_convertDataContext, 
                     (XPointer)NULL) == XCNOMEM) {
        JNU_ThrowInternalError(env, "XError");
        (*env)->ExceptionDescribe(env);
        (*env)->ExceptionClear(env);
    }
}

static Bool exitSecondaryLoop = True;

/*
 * This predicate procedure allows the Toolkit thread to process specific events
 * while it is blocked waiting for the event dispatch thread to process 
 * a SunDropTargetEvent. We need this to prevent deadlock when the client code
 * processing SunDropTargetEvent sets or gets the contents of the system
 * clipboard/selection. In this case the event dispatch thread waits for the
 * Toolkit thread to process PropertyNotify or SelectionNotify events.
 */
static Bool
secondary_loop_event(Display* dpy, XEvent* event, char* arg) {
    return (event->type == SelectionNotify || 
            event->type == SelectionClear  ||
            event->type == PropertyNotify) ? True : False;
}


JNIEXPORT void JNICALL 
Java_sun_awt_motif_MToolkitThreadBlockedHandler_enter(JNIEnv *env, jobject this) {
    DASSERT(exitSecondaryLoop && awt_currentThreadIsPrivileged(env));
    exitSecondaryLoop = False;
    while (!exitSecondaryLoop) {
        XEvent event;
        while (XCheckIfEvent(awt_display, &event, secondary_loop_event, NULL)) {
            XtDispatchEvent(&event);
        }
        AWT_WAIT(AWT_DND_POLL_INTERVAL);
    }
}

JNIEXPORT void JNICALL 
Java_sun_awt_motif_MToolkitThreadBlockedHandler_exit(JNIEnv *env, jobject this) {
    DASSERT(!exitSecondaryLoop && !awt_currentThreadIsPrivileged(env));
    exitSecondaryLoop = True;
    AWT_NOTIFY_ALL();
}

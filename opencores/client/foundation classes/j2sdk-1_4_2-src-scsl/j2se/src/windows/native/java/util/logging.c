/*
 * @(#)logging.c	1.4 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * Win32 specific code to support logging.
 */

#include "jni_util.h"


JNIEXPORT jboolean JNICALL
Java_java_util_logging_FileHandler_isSetUID(JNIEnv *env, jclass thisclass) {

    /* There is no set UID on Windows. */
    return JNI_FALSE;
}

/*
 * @(#)logging.c	1.4 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * Solaris/Linux specifoc code to support logging.
 */

#include <unistd.h>
#include "jni_util.h"


JNIEXPORT jboolean JNICALL
Java_java_util_logging_FileHandler_isSetUID(JNIEnv *env, jclass thisclass) {

    /* Return true if we are in a set UID or set GID process. */
    if (getuid() != geteuid() || getgid() != getegid()) {
	return JNI_TRUE;
    }
    return JNI_FALSE;
}

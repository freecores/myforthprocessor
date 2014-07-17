/*
 * @(#)awt_AWTKeyStroke.c	1.4 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "jni.h"
#include "jni_util.h"

#include "java_awt_AWTKeyStroke.h"

/*
 * Class:     java_awt_AWTKeyStroke
 * Method:    allocateNewInstance
 * Signature: (Ljava/lang/Class;)Ljava/awt/AWTKeyStroke;
 */
JNIEXPORT jobject JNICALL Java_java_awt_AWTKeyStroke_allocateNewInstance
    (JNIEnv *env, jclass awtKeyStroke, jclass subclass)
{
    jmethodID constructor = (*env)->GetMethodID(env, subclass, "<init>",
						"()V");
    if (constructor == NULL) {
        return NULL;
    }

    return (*env)->NewObject(env, subclass, constructor);
}


/*
 * @(#)awt_AWTKeyStroke.cpp	1.3 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "awt.h"
#include <java_awt_AWTKeyStroke.h>

extern "C" {

/*
 * Class:     java_awt_AWTKeyStroke
 * Method:    allocateNewInstance
 * Signature: (Ljava/lang/Class;)Ljava/awt/AWTKeyStroke;
 */
JNIEXPORT jobject JNICALL Java_java_awt_AWTKeyStroke_allocateNewInstance
    (JNIEnv *env, jclass awtKeyStroke, jclass subclass)
{
    jmethodID constructor = env->GetMethodID(subclass, "<init>", "()V");
    if (constructor == 0) {
        return NULL;
    }

    return env->NewObject(subclass, constructor);
}

}

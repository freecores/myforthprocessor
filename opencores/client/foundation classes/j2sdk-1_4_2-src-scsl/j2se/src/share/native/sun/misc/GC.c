/*
 * @(#)GC.c	1.6 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <jni.h>
#include <jvm.h>
#include "sun_misc_GC.h"


JNIEXPORT jlong JNICALL 
Java_sun_misc_GC_maxObjectInspectionAge(JNIEnv *env, jclass cls)
{
    return JVM_MaxObjectInspectionAge();
}

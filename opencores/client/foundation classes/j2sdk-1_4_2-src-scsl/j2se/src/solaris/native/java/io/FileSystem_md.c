/*
 * @(#)FileSystem_md.c	1.7 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "jni.h"
#include "jni_util.h"
#include "java_io_FileSystem.h"


JNIEXPORT jobject JNICALL  
Java_java_io_FileSystem_getFileSystem(JNIEnv *env, jclass ignored)
{
    return JNU_NewObjectByName(env, "java/io/UnixFileSystem", "()V");
}

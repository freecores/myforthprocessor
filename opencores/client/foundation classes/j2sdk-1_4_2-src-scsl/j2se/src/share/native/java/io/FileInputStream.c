/*
 * @(#)FileInputStream.c	1.17 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "jni.h"
#include "jni_util.h"
#include "jlong.h"
#include "io_util.h"
#include "jvm.h"

#include "java_io_FileInputStream.h"

#include <fcntl.h>
#include <limits.h>

/*******************************************************************/
/*  BEGIN JNI ********* BEGIN JNI *********** BEGIN JNI ************/
/*******************************************************************/

jfieldID fis_fd; /* id for jobject 'fd' in java.io.FileInputStream */

/**************************************************************
 * static methods to store field ID's in initializers
 */

JNIEXPORT void JNICALL
Java_java_io_FileInputStream_initIDs(JNIEnv *env, jclass fdClass) {
    fis_fd = (*env)->GetFieldID(env, fdClass, "fd", "Ljava/io/FileDescriptor;");
}

/**************************************************************
 * Input stream
 */

JNIEXPORT void JNICALL
Java_java_io_FileInputStream_open(JNIEnv *env, jobject this, jstring path) {
    fileOpen(env, this, path, fis_fd, O_RDONLY);
}

JNIEXPORT void JNICALL
Java_java_io_FileInputStream_close0(JNIEnv *env, jobject this) {
    int fd = GET_FD(fis_fd);
    SET_FD(-1, fis_fd);
    JVM_Close(fd);
}

JNIEXPORT jint JNICALL
Java_java_io_FileInputStream_read(JNIEnv *env, jobject this) {
    return readSingle(env, this, fis_fd);
}

JNIEXPORT jint JNICALL
Java_java_io_FileInputStream_readBytes(JNIEnv *env, jobject this,
	jbyteArray bytes, jint off, jint len) {
    return readBytes(env, this, bytes, off, len, fis_fd);
}

JNIEXPORT jlong JNICALL
Java_java_io_FileInputStream_skip(JNIEnv *env, jobject this, jlong toSkip) {
    jlong cur = jlong_zero;
    jlong end = jlong_zero;
    int fd = GET_FD(fis_fd);

    if ((cur = JVM_Lseek(fd, 0, SEEK_CUR)) == -1) {
        JNU_ThrowIOExceptionWithLastError(env, "Seek error");
    } else if ((end = JVM_Lseek(fd, toSkip, SEEK_CUR)) == -1) {
	JNU_ThrowIOExceptionWithLastError(env, "Seek error");
    }
    return (end - cur);
}

JNIEXPORT jint JNICALL
Java_java_io_FileInputStream_available(JNIEnv *env, jobject this) {
    int fd;
    jlong ret;

    fd = GET_FD(fis_fd);

    if (JVM_Available(fd, &ret)) {
        if (ret > INT_MAX) {
	    ret = (jlong) INT_MAX;
	}
	return jlong_to_jint(ret);
    }

    JNU_ThrowIOExceptionWithLastError(env, NULL);
    return 0;
}

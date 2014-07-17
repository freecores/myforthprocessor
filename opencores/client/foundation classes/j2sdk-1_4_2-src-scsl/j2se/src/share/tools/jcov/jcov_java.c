/*
 * @(#)jcov_java.c	1.3 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "jni.h"
#include "jcov_error.h"
#include "jcov_types.h"

int jcov_java_init_done = 0;
jmethodID mid_get_stream;
jmethodID mid_available;
jmethodID mid_read;

#define READ_CHUNK_SIZE 1024
#define NAME_BUF_SIZE 255
#define RET_IF_(cond) if (cond) return FALSE

Bool get_class_binary_data(JNIEnv *env, const char *class_name, UINT8 **buf, jint *buf_len) {
    jstring name;
    jobject istream;
    jbyteArray arr;
    jint len;
    jint res = 0;
    jint n = 0;
    char name_buf[NAME_BUF_SIZE];
    jclass cls_class_loader;
    jclass cls_input_stream;
    char *suff = ".class";

    cls_class_loader = (*env)->FindClass(env, "java/lang/ClassLoader");
    RET_IF_(cls_class_loader == NULL);
    cls_input_stream  = (*env)->FindClass(env, "java/io/InputStream");
    RET_IF_(cls_input_stream == NULL);

    if (strlen(class_name) + strlen(suff) >= NAME_BUF_SIZE) {
        printf("*** Jcov errror: class name too long: %s (skipped)\n", class_name);
        return FALSE;
    }
    sprintf(name_buf, "%s%s", class_name, suff);
    name = (*env)->NewStringUTF(env, name_buf);
    RET_IF_(name == NULL);
    istream = (*env)->CallStaticObjectMethod(env, cls_class_loader, mid_get_stream, name);
    RET_IF_(istream == NULL);

    len = (*env)->CallIntMethod(env, istream, mid_available);
    arr = (*env)->NewByteArray(env, (jsize)len);

    do {
        jint i;
        res += n;
        i = len - res; 
        if (i > READ_CHUNK_SIZE)
            i = READ_CHUNK_SIZE;
        n = (*env)->CallIntMethod(env, istream, mid_read, arr, res, i);
    } while (n > 0);
    RET_IF_(res != len);
    *buf = (UINT8*)malloc(len);
    RET_IF_(*buf == NULL);
    (*env)->GetByteArrayRegion(env, arr, 0, len, (jbyte*)(*buf));
    *buf_len = len;

    return TRUE;
}

Bool jcov_java_init(JNIEnv *env) {
    jclass cls_class_loader;
    jclass cls_input_stream;
    char *sig = "(Ljava/lang/String;)Ljava/io/InputStream;";

    cls_class_loader = (*env)->FindClass(env, "java/lang/ClassLoader");
    RET_IF_(cls_class_loader == NULL);
    mid_get_stream = (*env)->GetStaticMethodID(env, cls_class_loader, "getSystemResourceAsStream", sig);
    RET_IF_(mid_get_stream == NULL);
    cls_input_stream  = (*env)->FindClass(env, "java/io/InputStream");
    RET_IF_(cls_input_stream == NULL);
    mid_available = (*env)->GetMethodID(env, cls_input_stream, "available", "()I");
    RET_IF_(mid_available == NULL);
    mid_read = (*env)->GetMethodID(env, cls_input_stream, "read", "([BII)I");
    RET_IF_(mid_read == NULL);

    jcov_java_init_done = 1;
    return TRUE;
}

#undef RET_IF_

/*
 * @(#)FileDispatcher.c	1.21 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <windows.h>
#include "jni.h"
#include "jni_util.h"
#include "jvm.h"
#include "jlong.h"
#include "sun_nio_ch_FileDispatcher.h"
#include <io.h>
#include "nio_util.h"


/**************************************************************
 * FileDispatcher.c
 */

JNIEXPORT jint JNICALL
Java_sun_nio_ch_FileDispatcher_read0(JNIEnv *env, jclass clazz, jobject fdo,
                                      jlong address, jint len)
{
    DWORD read = 0;
    BOOL result = 0;
    jint fd = fdval(env, fdo);
    HANDLE h = (HANDLE)(jlong)_get_osfhandle(fd);

    if (h == INVALID_HANDLE_VALUE) {
        JNU_ThrowIOExceptionWithLastError(env, "Invalid handle");
        return -2;
    }
    result = ReadFile(h,          /* File handle to read */
                      (LPVOID)address,    /* address to put data */
                      len,        /* number of bytes to read */
                      &read,      /* number of bytes read */
                      NULL);      /* no overlapped struct */
    if (result == 0) {
        int error = GetLastError();
        if (error == ERROR_BROKEN_PIPE) {
            return -1;
        }
        if (error == ERROR_NO_DATA) {
            return 0;
        }
        JNU_ThrowIOExceptionWithLastError(env, "Read failed");
        return -2;
    }
    return convertReturnVal(env, (jint)read, JNI_TRUE);
}

JNIEXPORT jlong JNICALL
Java_sun_nio_ch_FileDispatcher_readv0(JNIEnv *env, jclass clazz, jobject fdo,
                                       jlong address, jint len)
{
    DWORD read = 0;
    BOOL result = 0;
    jint fd = fdval(env, fdo);
    jlong totalRead = 0;
    LPVOID loc;
    int i = 0;
    DWORD num = 0;
    struct iovec *iovecp = (struct iovec *)jlong_to_ptr(address);
    HANDLE h = (HANDLE)(jlong)_get_osfhandle(fd);

    if (h == INVALID_HANDLE_VALUE) {
        JNU_ThrowIOExceptionWithLastError(env, "Invalid handle");
        return -2;
    }

    for(i=0; i<len; i++) {
        loc = (LPVOID)jlong_to_ptr(iovecp[i].iov_base);
        num = iovecp[i].iov_len;
        result = ReadFile(h,                /* File handle to read */
                          loc,              /* address to put data */
                          num,              /* number of bytes to read */
                          &read,            /* number of bytes read */
                          NULL);            /* no overlapped struct */
        if (read > 0) {
            totalRead += read;
        }
        if (read < num) {
            break;
        }
    }

    if (result == 0) {
        int error = GetLastError();
        if (error == ERROR_BROKEN_PIPE) {
            return -1;
        }
        if (error == ERROR_NO_DATA) {
            return 0;
        }
        JNU_ThrowIOExceptionWithLastError(env, "Read failed");
        return -2;
    }

    return convertLongReturnVal(env, totalRead, JNI_TRUE);
}

JNIEXPORT jint JNICALL
Java_sun_nio_ch_FileDispatcher_pread0(JNIEnv *env, jclass clazz, jobject fdo,
                            jlong address, jint len, jlong offset)
{
    DWORD read = 0;
    BOOL result = 0;
    jint fd = fdval(env, fdo);
    HANDLE h = (HANDLE)(jlong)_get_osfhandle(fd);
    DWORD lowPos = 0;
    long highPos = 0;
    DWORD lowOffset = 0;
    long highOffset = 0;

    if (h == INVALID_HANDLE_VALUE) {
        JNU_ThrowIOExceptionWithLastError(env, "Invalid handle");
        return -2;
    }

    lowPos = SetFilePointer(h, 0, &highPos, FILE_CURRENT);
    if (lowPos == ((DWORD)-1)) {
        if (GetLastError() != ERROR_SUCCESS) {
            JNU_ThrowIOExceptionWithLastError(env, "Seek failed");
            return -2;
        }
    }

    lowOffset = (DWORD)offset;
    highOffset = (DWORD)(offset >> 32);
    lowOffset = SetFilePointer(h, lowOffset, &highOffset, FILE_BEGIN);
    if (lowOffset == ((DWORD)-1)) {
        if (GetLastError() != ERROR_SUCCESS) {
            JNU_ThrowIOExceptionWithLastError(env, "Seek failed");
            return -2;
        }
    }

    result = ReadFile(h,                /* File handle to read */
                      (LPVOID)address,  /* address to put data */
                      len,              /* number of bytes to read */
                      &read,            /* number of bytes read */
                      NULL);              /* struct with offset */

    if (result == 0) {
        int error = GetLastError();
        if (error == ERROR_BROKEN_PIPE) {
            return -1;
        }
        if (error == ERROR_NO_DATA) {
            return 0;
        }
        JNU_ThrowIOExceptionWithLastError(env, "Read failed");
        return -2;
    }

    lowPos = SetFilePointer(h, lowPos, &highPos, FILE_BEGIN);
    if (lowPos == ((DWORD)-1)) {
        if (GetLastError() != ERROR_SUCCESS) {
            JNU_ThrowIOExceptionWithLastError(env, "Seek failed");
            return -2;
        }
    }
    return convertReturnVal(env, (jint)read, JNI_TRUE);
}

JNIEXPORT jint JNICALL
Java_sun_nio_ch_FileDispatcher_write0(JNIEnv *env, jclass clazz, jobject fdo,
                                       jlong address, jint len)
{
    BOOL result = 0;
    DWORD written = 0;
    jint fd = fdval(env, fdo);
    HANDLE h = (HANDLE)(jlong)_get_osfhandle(fd);
    
    if (h != INVALID_HANDLE_VALUE) {
        result = WriteFile(h,           /* File handle to write */
                      (LPCVOID)address, /* pointers to the buffers */
                      len,              /* number of bytes to write */
                      &written,         /* receives number of bytes written */
                      NULL);            /* no overlapped struct */
    }

    if ((h == INVALID_HANDLE_VALUE) || (result == 0)) {
        JNU_ThrowIOExceptionWithLastError(env, "Write failed");
    }

    return convertReturnVal(env, (jint)written, JNI_FALSE);
}

JNIEXPORT jlong JNICALL
Java_sun_nio_ch_FileDispatcher_writev0(JNIEnv *env, jclass clazz, jobject fdo,
                                       jlong address, jint len)
{
    BOOL result = 0;
    DWORD written = 0;
    jint fd = fdval(env, fdo);
    HANDLE h = (HANDLE)(jlong)_get_osfhandle(fd);
    jlong totalWritten = 0;

    if (h != INVALID_HANDLE_VALUE) {
        LPVOID loc;
        int i = 0;
        DWORD num = 0;
        struct iovec *iovecp = (struct iovec *)jlong_to_ptr(address);

        for(i=0; i<len; i++) {
            loc = (LPVOID)jlong_to_ptr(iovecp[i].iov_base);
            num = iovecp[i].iov_len;
            result = WriteFile(h,       /* File handle to write */
                               loc,     /* pointers to the buffers */
                               num,     /* number of bytes to write */
                               &written,/* receives number of bytes written */
                               NULL);   /* no overlapped struct */
            if (written > 0) {
                totalWritten += written;
            }
            if (written < num) {
                break;
            }
        }
    }

    if ((h == INVALID_HANDLE_VALUE) || (result == 0)) {
        JNU_ThrowIOExceptionWithLastError(env, "Write failed");
    }

    return convertLongReturnVal(env, totalWritten, JNI_FALSE);
}

JNIEXPORT jint JNICALL
Java_sun_nio_ch_FileDispatcher_pwrite0(JNIEnv *env, jclass clazz, jobject fdo,
                            jlong address, jint len, jlong offset)
{
    BOOL result = 0;
    DWORD written = 0;
    jint fd = fdval(env, fdo);
    HANDLE h = (HANDLE)(jlong)_get_osfhandle(fd);
    DWORD lowPos = 0;
    long highPos = 0;
    DWORD lowOffset = 0;
    long highOffset = 0;

    lowPos = SetFilePointer(h, 0, &highPos, FILE_CURRENT);
    if (lowPos == ((DWORD)-1)) {
        if (GetLastError() != ERROR_SUCCESS) {
            JNU_ThrowIOExceptionWithLastError(env, "Seek failed");
            return -2;
        }
    }

    lowOffset = (DWORD)offset;
    highOffset = (DWORD)(offset >> 32);
    lowOffset = SetFilePointer(h, lowOffset, &highOffset, FILE_BEGIN);
    if (lowOffset == ((DWORD)-1)) {
        if (GetLastError() != ERROR_SUCCESS) {
            JNU_ThrowIOExceptionWithLastError(env, "Seek failed");
            return -2;
        }
    }
    
    result = WriteFile(h,               /* File handle to write */
                      (LPCVOID)address, /* pointers to the buffers */
                      len,              /* number of bytes to write */
                      &written,         /* receives number of bytes written */
                      NULL);            /* no overlapped struct */

    if ((h == INVALID_HANDLE_VALUE) || (result == 0)) {
        JNU_ThrowIOExceptionWithLastError(env, "Write failed");
        return -2;
    }

    lowPos = SetFilePointer(h, lowPos, &highPos, FILE_BEGIN);
    if (lowPos == ((DWORD)-1)) {
        if (GetLastError() != ERROR_SUCCESS) {
            JNU_ThrowIOExceptionWithLastError(env, "Seek failed");
            return -2;
        }
    }

    return convertReturnVal(env, (jint)written, JNI_FALSE);
}

static void closeFileDescriptor(JNIEnv *env, int fd) {
    if (fd != -1) {
        int result = close(fd);
        if (result < 0)
            JNU_ThrowIOExceptionWithLastError(env, "Close failed");
    }
}

JNIEXPORT void JNICALL
Java_sun_nio_ch_FileDispatcher_close0(JNIEnv *env, jclass clazz, jobject fdo)
{
    jint fd = fdval(env, fdo);
    closeFileDescriptor(env, fd);
}

JNIEXPORT void JNICALL
Java_sun_nio_ch_FileDispatcher_closeIntFD(JNIEnv *env, jclass clazz, jint fd)
{
    closeFileDescriptor(env, fd);
}

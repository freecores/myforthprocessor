/*
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)DatagramDispatcher.c	1.6 03/01/23
 */

#include <windows.h>
#include <winsock2.h>
#include <ctype.h>
#include "jni.h"
#include "jni_util.h"
#include "jvm.h"
#include "jlong.h"
#include "sun_nio_ch_DatagramDispatcher.h"

#include "nio_util.h"


/**************************************************************
 * DatagramDispatcher.c
 */

JNIEXPORT jint JNICALL
Java_sun_nio_ch_DatagramDispatcher_read0(JNIEnv *env, jclass clazz, jobject fdo,
                                      jlong address, jint len)
{
    /* set up */
    int i = 0;
    DWORD read = 0;
    DWORD flags = 0;
    jint fd = fdval(env, fdo);
    WSABUF buf;

    /* destination buffer and size */
    buf.buf = (char *)address;
    buf.len = (u_long)len;
    
    /* read into the buffers */
    i = WSARecv((SOCKET)fd, /* Socket */
            &buf,           /* pointers to the buffers */
            (DWORD)1,       /* number of buffers to process */
            &read,          /* receives number of bytes read */
            &flags,         /* no flags */
            0,              /* no overlapped sockets */
            0);             /* no completion routine */

    if (i == SOCKET_ERROR) {
        int theErr = (jint)WSAGetLastError();
        if (theErr == WSAEWOULDBLOCK) {
            return 0;
        }
        if (theErr == WSAECONNRESET) {
            purgeOutstandingICMP(env, clazz, fd);
            JNU_ThrowByName(env, JNU_JAVANETPKG "PortUnreachableException", 0);
            return -2;
        }
        JNU_ThrowIOExceptionWithLastError(env, "Write failed");
        return -2;
    }

    return convertReturnVal(env, (jint)read, JNI_TRUE);
}

JNIEXPORT jlong JNICALL
Java_sun_nio_ch_DatagramDispatcher_readv0(JNIEnv *env, jclass clazz,
                                          jobject fdo, jlong address, jint len)
{
    /* set up */
    int i = 0;
    DWORD read = 0;
    DWORD flags = 0;
    jint fd = fdval(env, fdo);
    struct iovec *iovp = (struct iovec *)address;
    WSABUF *bufs = malloc(len * sizeof(WSABUF));

    /* copy iovec into WSABUF */
    for(i=0; i<len; i++) {
        bufs[i].buf = (char *)iovp[i].iov_base;
        bufs[i].len = (u_long)iovp[i].iov_len;
    }
    
    /* read into the buffers */
    i = WSARecv((SOCKET)fd, /* Socket */
            bufs,           /* pointers to the buffers */
            (DWORD)len,     /* number of buffers to process */
            &read,          /* receives number of bytes read */
            &flags,         /* no flags */
            0,              /* no overlapped sockets */
            0);             /* no completion routine */

    /* clean up */
    free(bufs);

    if (i != 0) {
        int theErr = (jint)WSAGetLastError();
        if (theErr == WSAEWOULDBLOCK) {
            return 0;
        }
        if (theErr == WSAECONNRESET) {
            purgeOutstandingICMP(env, clazz, fd);
            JNU_ThrowByName(env, JNU_JAVANETPKG "PortUnreachableException", 0);
            return -2;
        }
        JNU_ThrowIOExceptionWithLastError(env, "Write failed");
        return -2;
    }

    return convertLongReturnVal(env, (jlong)read, JNI_TRUE);
}


JNIEXPORT jint JNICALL
Java_sun_nio_ch_DatagramDispatcher_write0(JNIEnv *env, jclass clazz,
                                          jobject fdo, jlong address, jint len)
{
    /* set up */
    int i = 0;
    DWORD written = 0;
    jint fd = fdval(env, fdo);
    WSABUF buf;

    /* copy iovec into WSABUF */
    buf.buf = (char *)address;
    buf.len = (u_long)len;
    
    /* read into the buffers */
    i = WSASend((SOCKET)fd, /* Socket */
            &buf,           /* pointers to the buffers */
            (DWORD)1,       /* number of buffers to process */
            &written,       /* receives number of bytes written */
            0,              /* no flags */
            0,              /* no overlapped sockets */
            0);             /* no completion routine */

    if (i == SOCKET_ERROR) {
        int theErr = (jint)WSAGetLastError();
        if (theErr == WSAEWOULDBLOCK) {
            return 0;
        }
        if (theErr == WSAECONNRESET) {
            purgeOutstandingICMP(env, clazz, fd);
            JNU_ThrowByName(env, JNU_JAVANETPKG "PortUnreachableException", 0);
            return -2;
        }
        JNU_ThrowIOExceptionWithLastError(env, "Write failed");
    }

    return convertReturnVal(env, (jint)written, JNI_FALSE);
}

JNIEXPORT jlong JNICALL
Java_sun_nio_ch_DatagramDispatcher_writev0(JNIEnv *env, jclass clazz,
                                         jobject fdo, jlong address, jint len)
{
    /* set up */
    int i = 0;
    DWORD written = 0;
    jint fd = fdval(env, fdo);
    struct iovec *iovp = (struct iovec *)address;
    WSABUF *bufs = malloc(len * sizeof(WSABUF));

    /* copy iovec into WSABUF */
    for(i=0; i<len; i++) {
        bufs[i].buf = (char *)iovp[i].iov_base;
        bufs[i].len = (u_long)iovp[i].iov_len;
    }
    
    /* read into the buffers */
    i = WSASend((SOCKET)fd, /* Socket */
            bufs,           /* pointers to the buffers */
            (DWORD)len,     /* number of buffers to process */
            &written,       /* receives number of bytes written */
            0,              /* no flags */
            0,              /* no overlapped sockets */
            0);             /* no completion routine */

    /* clean up */
    free(bufs);

    if (i != 0) {
        int theErr = (jint)WSAGetLastError();
        if (theErr == WSAEWOULDBLOCK) {
            return 0;
        }
        if (theErr == WSAECONNRESET) {
            purgeOutstandingICMP(env, clazz, fd);
            JNU_ThrowByName(env, JNU_JAVANETPKG "PortUnreachableException", 0);
            return -2;
        }
        JNU_ThrowIOExceptionWithLastError(env, "Write failed");
    }

    return convertLongReturnVal(env, (jlong)written, JNI_FALSE);
}

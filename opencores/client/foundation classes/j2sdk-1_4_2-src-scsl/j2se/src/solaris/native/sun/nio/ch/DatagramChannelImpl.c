/*
 * @(#)DatagramChannelImpl.c	1.19 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "jni.h"
#include "jni_util.h"
#include "jvm.h"
#include "jlong.h"

#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#if __linux__
#include <netinet/in.h>
#endif

#include "net_util.h"
#include "net_util_md.h"
#include "nio.h"
#include "nio_util.h"

#include "sun_nio_ch_DatagramChannelImpl.h"

static jfieldID isa_addrID;	/* address in java.net.InetSocketAddress */
static jfieldID isa_portID;	/* port in java.net.InetSocketAddress */
static jfieldID dci_senderID;   /* sender in sun.nio.ch.DatagramChannelImpl */
static jfieldID dci_senderAddrID; /* sender InetAddress in sun.nio.ch.DatagramChannelImpl */
static jfieldID dci_senderPortID; /* sender port in sun.nio.ch.DatagramChannelImpl */
static jclass isa_class;	/* java.net.InetSocketAddress */
static jmethodID isa_ctorID;	/*   .InetSocketAddress(InetAddress, int) */

JNIEXPORT void JNICALL
Java_sun_nio_ch_DatagramChannelImpl_initIDs(JNIEnv *env, jclass clazz)
{
    clazz = (*env)->FindClass(env, "java/net/InetSocketAddress");
    isa_class = (*env)->NewGlobalRef(env, clazz);
    isa_ctorID = (*env)->GetMethodID(env, clazz, "<init>",
				     "(Ljava/net/InetAddress;I)V");
    isa_addrID = (*env)->GetFieldID(env, clazz, "addr",
                                    "Ljava/net/InetAddress;");
    isa_portID = (*env)->GetFieldID(env, clazz, "port", "I");

    clazz = (*env)->FindClass(env, "sun/nio/ch/DatagramChannelImpl");
    dci_senderID = (*env)->GetFieldID(env, clazz, "sender",
                                      "Ljava/net/SocketAddress;");
    dci_senderAddrID = (*env)->GetFieldID(env, clazz,
                                          "cachedSenderInetAddress",
                                          "Ljava/net/InetAddress;");
    dci_senderPortID = (*env)->GetFieldID(env, clazz,
					  "cachedSenderPort", "I");
}

JNIEXPORT void JNICALL
Java_sun_nio_ch_DatagramChannelImpl_disconnect0(JNIEnv *env, jobject this,
                                                jobject fdo)
{
    jint fd = fdval(env, fdo);
    int rv;

#ifdef __solaris__
    rv = connect(fd, 0, 0);
#endif

#ifdef __linux__
    {
	int len;
	SOCKADDR sa;

	memset(&sa, 0, sizeof(sa));

#ifdef AF_INET6
        if (ipv6_available()) {
	    struct sockaddr_in6 *him6 = (struct sockaddr_in6 *)&sa;
	    him6->sin6_family = AF_UNSPEC;
	    len = sizeof(struct sockaddr_in6);
 	} else  
#endif
        {
	    struct sockaddr_in *him4 = (struct sockaddr_in*)&sa;
	    him4->sin_family = AF_UNSPEC;
	    len = sizeof(struct sockaddr_in);
        }

        rv = connect(fd, (struct sockaddr *)&sa, len);
    }
#endif

    if (rv < 0)
        handleSocketError(env, errno);

}

JNIEXPORT jint JNICALL
Java_sun_nio_ch_DatagramChannelImpl_receive0(JNIEnv *env, jobject this,
					     jobject fdo, jlong address,
					     jint len, jboolean connected)
{
    jint fd = fdval(env, fdo);
    void *buf = (void *)jlong_to_ptr(address);
    SOCKADDR sa;
    int sa_len = SOCKADDR_LEN;
    jboolean retry = JNI_FALSE;
    jint n = 0;
    jobject senderAddr;

    if (len > MAX_PACKET_LEN) {
        len = MAX_PACKET_LEN;
    }

    do {
        retry = JNI_FALSE;
        n = recvfrom(fd, buf, len, 0, (struct sockaddr *)&sa, &sa_len);
        if (n < 0) {
            if (errno == EWOULDBLOCK) {
                return IOS_UNAVAILABLE;
            }
	    if (errno == EINTR) {
		return IOS_INTERRUPTED;
	    }
            if (errno == ECONNREFUSED) {
                if (connected == JNI_FALSE) {
                    retry = JNI_TRUE;
                } else {
                    JNU_ThrowByName(env, JNU_JAVANETPKG 
                                    "PortUnreachableException", 0);
                    return IOS_THROWN;
                }
            } else {
                return handleSocketError(env, errno);
            }
        }
    } while (retry == JNI_TRUE);

    /*
     * If the source address and port match the cached address
     * and port in DatagramChannelImpl then we don't need to
     * create InetAddress and InetSocketAddress objects.
     */
    senderAddr = (*env)->GetObjectField(env, this, dci_senderAddrID);
    if (senderAddr != NULL) {
        if (!NET_SockaddrEqualsInetAddress(env, (struct sockaddr *)&sa,
                                           senderAddr)) {
            senderAddr = NULL;
        } else {
            jint port = (*env)->GetIntField(env, this, dci_senderPortID);
            if (port != NET_GetPortFromSockaddr((struct sockaddr *)&sa)) {
                senderAddr = NULL;
            }
        }
    }
    if (senderAddr == NULL) {
        jobject isa = NULL;
        int port;
        jobject ia = NET_SockaddrToInetAddress(env, (struct sockaddr *)&sa,
                                               &port);

        if (ia != NULL) {
            isa = (*env)->NewObject(env, isa_class, isa_ctorID, ia, port);
        }

        if (isa == NULL) {
            JNU_ThrowOutOfMemoryError(env, "heap allocation failed");
            return IOS_THROWN;
        }

        (*env)->SetObjectField(env, this, dci_senderAddrID, ia);
        (*env)->SetIntField(env, this, dci_senderPortID, 
                            NET_GetPortFromSockaddr((struct sockaddr *)&sa));
        (*env)->SetObjectField(env, this, dci_senderID, isa);
    }
    return n;
}

JNIEXPORT jint JNICALL
Java_sun_nio_ch_DatagramChannelImpl_send0(JNIEnv *env, jobject this,
		                            jobject fdo, jlong address,
                                            jint len, jobject dest)
{
    jint fd = fdval(env, fdo);
    void *buf = (void *)jlong_to_ptr(address);
    SOCKADDR sa;
    int sa_len = SOCKADDR_LEN;
    jint n = 0;
    jobject destAddress = (*env)->GetObjectField(env, dest, isa_addrID);
    jint destPort = (*env)->GetIntField(env, dest, isa_portID);

    if (len > MAX_PACKET_LEN) {
        len = MAX_PACKET_LEN;
    }
    
    NET_InetAddressToSockaddr(env, destAddress, destPort,
                              (struct sockaddr *)&sa, &sa_len);
    n = sendto(fd, buf, len, 0, (struct sockaddr *)&sa, sa_len);
    if (n < 0) {
	if (errno == EAGAIN) {
	    return IOS_UNAVAILABLE;
        }
	if (errno == EINTR) {
	    return IOS_INTERRUPTED;
	}
        if (errno == ECONNREFUSED) {
            JNU_ThrowByName(env, JNU_JAVANETPKG "PortUnreachableException", 0);
            return IOS_THROWN;
        }
        return handleSocketError(env, errno);
    }
    return n;
}

/*
 * @(#)net_util_md.h	1.32 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <winsock2.h>

#include "java_io_FileDescriptor.h"
#include "java_net_SocketOptions.h"

#define MAX_BUFFER_LEN		2048
#define MAX_HEAP_BUFFER_LEN	65536

/* true if SO_RCVTIMEO is supported by underlying provider */
extern jboolean isRcvTimeoutSupported;

void NET_ThrowCurrent(JNIEnv *env, char *msg);

/*
 * Return default Type Of Service
 */
int NET_GetDefaultTOS(void);

JNIEXPORT int JNICALL NET_SocketClose(int fd);

JNIEXPORT int JNICALL NET_Timeout(int fd, long timeout);

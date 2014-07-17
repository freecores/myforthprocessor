/*
 * @(#)Inet6Address.c	1.6 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <string.h>

#include "java_net_Inet6Address.h"
#include "net_util.h"

/************************************************************************
 * Inet6Address
 */
jclass ia6_class;
jfieldID ia6_ipaddressID;
jmethodID ia6_ctrID;

/*
 * Class:     java_net_Inet6Address
 * Method:    init
 * Signature: ()V 
 */
JNIEXPORT void JNICALL
Java_java_net_Inet6Address_init(JNIEnv *env, jclass cls) {
    jclass c = (*env)->FindClass(env, "java/net/Inet6Address");
    CHECK_NULL(c);
    ia6_class = (*env)->NewGlobalRef(env, c);
    CHECK_NULL(ia6_class);
    ia6_ipaddressID = (*env)->GetFieldID(env, ia6_class, "ipaddress", "[B");
    CHECK_NULL(ia6_ipaddressID);
    ia6_ctrID = (*env)->GetMethodID(env, ia6_class, "<init>", "()V");
    CHECK_NULL(ia6_ctrID);
}

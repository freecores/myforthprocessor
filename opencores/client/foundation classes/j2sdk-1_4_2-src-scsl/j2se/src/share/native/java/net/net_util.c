/*
 * @(#)net_util.c	1.8 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "jni.h"
#include "jvm.h"
#include "jni_util.h"
#include "net_util.h"

int IPv6_supported() ;

static int IPv6_available;

JNIEXPORT jint JNICALL ipv6_available() 
{
    return IPv6_available ;
}

JNIEXPORT jint JNICALL
JNI_OnLoad(JavaVM *vm, void *reserved)
{
    JNIEnv *env;
    jclass iCls;
    jmethodID mid;
    jstring s;
    jint preferIPv4Stack;
    
    if ((*vm)->GetEnv(vm, (void **)&env, JNI_VERSION_1_2) == JNI_OK) {
        if (JVM_InitializeSocketLibrary() < 0) {
	    JNU_ThrowByName(env, "java/lang/UnsatisfiedLinkError",
			    "failed to initialize net library.");
	    return JNI_VERSION_1_2;
	}
    }
    iCls = (*env)->FindClass(env, "java/lang/Boolean"); 
    CHECK_NULL_RETURN(iCls, JNI_VERSION_1_2);
    mid = (*env)->GetStaticMethodID(env, iCls, "getBoolean", "(Ljava/lang/String;)Z");
    CHECK_NULL_RETURN(mid, JNI_VERSION_1_2);
    s = (*env)->NewStringUTF(env, "java.net.preferIPv4Stack");
    CHECK_NULL_RETURN(s, JNI_VERSION_1_2);
    preferIPv4Stack = (*env)->CallStaticBooleanMethod(env, iCls, mid, s);
  
    /* 
       Since we have initialized and loaded the Socket library we will 
       check now to whether we have IPv6 on this platform and if the 
       supporting socket APIs are available 
    */
    IPv6_available = IPv6_supported() & (!preferIPv4Stack);
    return JNI_VERSION_1_2;
}

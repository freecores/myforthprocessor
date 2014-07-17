/*
 * @(#)verify_stub.c	1.6 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


/*
 * The real verifier now lives in libverifier.so/verifier.dll.
 *
 * This dummy exists so that HotSpot will run with the new
 * libjava.so/java.dll which is where is it accustomed to finding the
 * verifier.
 */

#include "jni.h"

struct struct_class_size_info;
typedef struct struct_class_size_info class_size_info;


JNIIMPORT jint 
VerifyFormat(char *class_name, unsigned char *data,
	       unsigned int data_size, class_size_info *size,
	       char *message_buffer, jint buffer_length,
	       jboolean measure_only, jboolean check_relaxed);


JNIEXPORT jint
VerifyClassFormat(char *class_name, unsigned char *data,
		  unsigned int data_size, class_size_info *size,
		  char *message_buffer, jint buffer_length,
		  jboolean measure_only, jboolean check_relaxed)
{
    return VerifyFormat(class_name, data, data_size, size, message_buffer,
			buffer_length, measure_only, check_relaxed);
}


JNIIMPORT jboolean
VerifyClass(JNIEnv *env, jclass cb, char *buffer, jint len);

JNIEXPORT jboolean
VerifyClassCodes(JNIEnv *env, jclass cb, char *buffer, jint len)
{
    return VerifyClass(env, cb, buffer, len);
}

/*
 * @(#)Runtime_md.c	1.7 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "jni.h"
#include "jni_util.h"

#include "java_lang_Runtime.h"

JNIEXPORT jobject JNICALL  
Java_java_lang_Runtime_execInternal(JNIEnv *env, jobject this,
				    jobjectArray cmdarray, jobjectArray envp,
                                    jstring path)
{
    if (cmdarray == NULL) {
        JNU_ThrowNullPointerException(env, 0);
	return 0;
    }

    if ((*env)->GetArrayLength(env, cmdarray) == 0) {
	JNU_ThrowArrayIndexOutOfBoundsException(env, 0);
	return 0;
    }

    return JNU_NewObjectByName(env,
			       "java/lang/Win32Process",
			       "([Ljava/lang/String;[Ljava/lang/String;Ljava/lang/String;)V", 
			       cmdarray, envp, path);
} 

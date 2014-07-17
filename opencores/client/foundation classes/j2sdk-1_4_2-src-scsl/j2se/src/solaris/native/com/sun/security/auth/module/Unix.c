/*
 * @(#)Unix.c	1.6 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <jni.h>
#include "com_sun_security_auth_module_UnixSystem.h"
#include <stdio.h>
#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

JNIEXPORT void JNICALL 
Java_com_sun_security_auth_module_UnixSystem_getUnixInfo
						(JNIEnv *env, jobject obj) {

    int i;
    struct passwd *pwd;
    jsize numSuppGroups = getgroups(0, NULL);
    gid_t *groups = (gid_t *)calloc(numSuppGroups, sizeof(gid_t));

    jfieldID fid;
    jstring jstr;
    jlongArray jgroups;
    jlong *jgroupsAsArray;
    jclass cls = (*env)->GetObjectClass(env, obj);

    if ((pwd = getpwuid(getuid())) != NULL &&
	getgroups(numSuppGroups, groups) != -1) {

	/*
	 * set username
	 */
	fid = (*env)->GetFieldID(env, cls, "username", "Ljava/lang/String;");
	if (fid == 0) {
	    jclass newExcCls =
		(*env)->FindClass(env, "java/lang/IllegalArgumentException");
	    if (newExcCls == 0) {
		/* Unable to find the new exception class, give up. */
		return;
	    }
	    (*env)->ThrowNew(env, newExcCls, "invalid field: username");
	}
	jstr = (*env)->NewStringUTF(env, pwd->pw_name);
	(*env)->SetObjectField(env, obj, fid, jstr);

	/*
	 * set uid
	 */
	fid = (*env)->GetFieldID(env, cls, "uid", "J");
	if (fid == 0) {
	    jclass newExcCls =
		(*env)->FindClass(env, "java/lang/IllegalArgumentException");
	    if (newExcCls == 0) {
		/* Unable to find the new exception class, give up. */
		return;
	    }
	    (*env)->ThrowNew(env, newExcCls, "invalid field: username");
	}
	(*env)->SetLongField(env, obj, fid, pwd->pw_uid);

	/*
	 * set gid
	 */
	fid = (*env)->GetFieldID(env, cls, "gid", "J");
	if (fid == 0) {
	    jclass newExcCls =
		(*env)->FindClass(env, "java/lang/IllegalArgumentException");
	    if (newExcCls == 0) {
		/* Unable to find the new exception class, give up. */
		return;
	    }
	    (*env)->ThrowNew(env, newExcCls, "invalid field: username");
	}
	(*env)->SetLongField(env, obj, fid, pwd->pw_gid);

	/*
	 * set supplementary groups
	 */
	fid = (*env)->GetFieldID(env, cls, "groups", "[J");
	if (fid == 0) {
	    jclass newExcCls =
		(*env)->FindClass(env, "java/lang/IllegalArgumentException");
	    if (newExcCls == 0) {
		/* Unable to find the new exception class, give up. */
		return;
	    }
	    (*env)->ThrowNew(env, newExcCls, "invalid field: username");
	}

	jgroups = (*env)->NewLongArray(env, numSuppGroups);
	jgroupsAsArray = (*env)->GetLongArrayElements(env, jgroups, 0);
	for (i = 0; i < numSuppGroups; i++)
	    jgroupsAsArray[i] = groups[i];
	(*env)->ReleaseLongArrayElements(env, jgroups, jgroupsAsArray, 0);
	(*env)->SetObjectField(env, obj, fid, jgroups);
    }
    return;
}

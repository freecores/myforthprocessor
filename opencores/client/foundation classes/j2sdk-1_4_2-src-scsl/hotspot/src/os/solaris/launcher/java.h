#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)java.h	1.5 03/01/23 11:04:46 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

/*
/// Temporary copy of JDK1.2beta4 launcher for HotSPARC.
/// "$JDK/src/share/bin/java.h":15
*/

#ifndef _JAVA_H_
#define _JAVA_H_

/*
 * Get system specific defines.
 */
#include "jni.h"
#include "java_md.h"

/*
 * Pointers to the needed JNI invocation API, initialized by LoadJavaVM.
 */
typedef struct {
    jint (JNICALL *CreateJavaVM)(JavaVM **pvm, void **env, void *args);
    jint (JNICALL *GetDefaultJavaVMInitArgs)(void *args);
} InvocationFunctions;

/*
 * Protoypes for launcher functions in the system specific java_md.c.
 */
jboolean LoadJavaVM(char *jvmtype, InvocationFunctions *ifn);
void GetXUsagePath(char *buf, jint bufsize);
jboolean GetApplicationHome(char *buf, jint bufsize);

/*
 * Make launcher spit debug output.
 */
extern jboolean debug;

#endif /* _JAVA_H_ */

#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)sa.hpp	1.4 03/01/23 11:13:46 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "jni.h"

extern "C" {
JNIEXPORT jint JNICALL 
JVM_OnLoad(JavaVM *vm, char *options, void *reserved);
}

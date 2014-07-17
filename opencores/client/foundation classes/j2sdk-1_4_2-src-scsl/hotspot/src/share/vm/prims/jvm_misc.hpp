#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)jvm_misc.hpp	1.13 03/01/23 12:20:00 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Useful entry points shared by JNI and JVM interface. 
// We do not allow real JNI or JVM entry point to call each other.

jclass find_class_from_class_loader(JNIEnv* env, symbolHandle name, jboolean init, Handle loader, Handle protection_domain, jboolean throwError, TRAPS);


/*
 * Support for Serialization and RMI. Currently used by HotSpot only.
 */

JNIEXPORT void JNICALL 
JVM_SetPrimitiveFieldValues(JNIEnv *env, jclass cb, jobject obj, 
                            jlongArray fieldIDs, jcharArray typecodes, jbyteArray data);

JNIEXPORT void JNICALL 
JVM_GetPrimitiveFieldValues(JNIEnv *env, jclass cb, jobject obj,
                            jlongArray fieldIDs, jcharArray typecodes, jbyteArray data);


/*
 * Support for -Xcheck:jni
 */

extern const struct JNINativeInterface_* jni_functions_nocheck();
extern const struct JNINativeInterface_* jni_functions_check();

/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <jni.h>

/*
 * Class:     sun_security_krb5_internal_ccache_FileCredentialsCache
 * Method:    chmod
 * Signature: (Ljava/lang/String;I)I
 */
JNIEXPORT jint JNICALL Java_sun_security_krb5_internal_ccache_FileCredentialsCache_chmod
  (JNIEnv *env, jclass cls, jstring java_fname, jint permission)
{

  const char *fname = (*env)->GetStringUTFChars(env, java_fname, NULL);
  int result =  chmod(fname, permission);
  if (result != 0)
     result = errno;
  (*env)->ReleaseStringUTFChars(env, java_fname, fname);
  return (jint) result;

}

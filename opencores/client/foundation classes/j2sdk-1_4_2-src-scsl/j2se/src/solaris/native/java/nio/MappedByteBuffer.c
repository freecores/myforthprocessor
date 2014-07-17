/*
 * @(#)MappedByteBuffer.c	1.10 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "jni.h"
#include "jni_util.h"
#include "jvm.h"
#include "jlong.h"
#include "java_nio_MappedByteBuffer.h"
#include <sys/mman.h>
#include <stdlib.h>


/* Linux 2.2 kernels do not have these, though 2.4 kernels do */

#ifdef __solaris__
#define HAVE_MINCORE
#define HAVE_MADVISE
#endif


JNIEXPORT jboolean JNICALL
Java_java_nio_MappedByteBuffer_isLoaded0(JNIEnv *env, jobject obj,
                                        jlong address, jlong len)
#ifdef HAVE_MINCORE
{
    jboolean loaded = JNI_TRUE;
    jint pageSize = sysconf(_SC_PAGESIZE);
    jint numPages = (len + pageSize - 1) / pageSize;
    int result = 0;
    int i = 0;
    void *a = (void *) jlong_to_ptr(address);
    char * vec = (char *)malloc(numPages * sizeof(char));

    result = mincore(a, (size_t)len, vec);
    if (result != 0) {
        free(vec);
        JNU_ThrowIOExceptionWithLastError(env, "mincore failed");
        return JNI_FALSE;
    }
    
    for (i=0; i<numPages; i++) {
        if (vec[i] == 0) {
            loaded = JNI_FALSE;
            break;
        }
    }
    free(vec);
    return loaded;
}
#else /* HAVE_MINCORE */
{
    return JNI_FALSE;
}
#endif /* HAVE_MINCORE */


JNIEXPORT jint JNICALL
Java_java_nio_MappedByteBuffer_load0(JNIEnv *env, jobject obj, jlong address,
                                     jlong len, jint pageSize)
{
    int pageIncrement = pageSize / sizeof(int);
    int numPages = (len + pageSize - 1) / pageSize;
    int *ptr = (int *)jlong_to_ptr(address);
    int i = 0;
    int j = 0;

#ifdef HAVE_MADVISE
    int result = madvise((caddr_t)ptr, len, MADV_WILLNEED);
#endif /* HAVE_MADVISE */

    /* touch every page */
    for (i=0; i<numPages; i++) {
        j += *((volatile int *)ptr);
        ptr += pageIncrement;
    }
    return j;
}


JNIEXPORT void JNICALL
Java_java_nio_MappedByteBuffer_force0(JNIEnv *env, jobject obj, jlong address,
                                      jlong len)
{
    jlong pageSize = sysconf(_SC_PAGESIZE);
    jlong offset = address % pageSize;
    void *a = (void *) jlong_to_ptr(address - offset);
    int result = msync(a, (size_t)(len + offset), MS_SYNC);
    if (result != 0) {
        JNU_ThrowIOExceptionWithLastError(env, "msync failed");
    }
}

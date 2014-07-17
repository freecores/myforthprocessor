/*
 * @(#)ThreadGroupReferenceImpl.c	1.19 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
#include <string.h>

#include "ThreadGroupReferenceImpl.h"
#include "util.h"
#include "inStream.h"
#include "outStream.h"

static jboolean 
name(PacketInputStream *in, PacketOutputStream *out) 
{
    JNIEnv *env = getEnv();
    JVMDI_thread_group_info info;

    jthreadGroup group = inStream_readThreadGroupRef(in);
    if (inStream_error(in)) {
        return JNI_TRUE;
    }

    threadGroupInfo(group, &info);
    outStream_writeString(out, info.name);

    (*env)->DeleteGlobalRef(env, info.parent);
    jdwpFree(info.name);
    return JNI_TRUE;
}

static jboolean 
parent(PacketInputStream *in, PacketOutputStream *out) 
{
    JNIEnv *env = getEnv();
    JVMDI_thread_group_info info;

    jthreadGroup group = inStream_readThreadGroupRef(in);
    if (inStream_error(in)) {
        return JNI_TRUE;
    }

    threadGroupInfo(group, &info);
    WRITE_GLOBAL_REF(env, out, info.parent);

    jdwpFree(info.name);
    return JNI_TRUE;
}

static jboolean 
children(PacketInputStream *in, PacketOutputStream *out) 
{
     jint error;
     jint i;
     jint threadCount;
     jint groupCount;
     jthread *theThreads;
     jthread *theGroups;
     JNIEnv *env = getEnv();
 
     jthreadGroup group = inStream_readThreadGroupRef(in);
     if (inStream_error(in)) {
         return JNI_TRUE;
     }
 
     error = jvmdi->GetThreadGroupChildren(group,
                                          &threadCount,&theThreads,
                                          &groupCount, &theGroups);
     if (error != JVMDI_ERROR_NONE) {
         outStream_setError(out, error);
         return JNI_TRUE;
     }


     /* Squish out all of the debugger-spawned threads */
     threadCount = filterDebugThreads(theThreads, threadCount);
  
     outStream_writeInt(out, threadCount);
     for (i = 0; i < threadCount; i++) {
         WRITE_GLOBAL_REF(env, out, theThreads[i]);
     }
     outStream_writeInt(out, groupCount);
     for (i = 0; i < groupCount; i++) {
         WRITE_GLOBAL_REF(env, out, theGroups[i]);
     }

     jdwpFree(theGroups);
     jdwpFree(theThreads);
     return JNI_TRUE;
}

void *ThreadGroupReference_Cmds[] = { (void *)3,
                                      (void *)name,
                                      (void *)parent,
                                      (void *)children };


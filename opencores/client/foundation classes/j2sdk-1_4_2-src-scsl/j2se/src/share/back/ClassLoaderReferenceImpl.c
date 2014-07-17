/*
 * @(#)ClassLoaderReferenceImpl.c	1.10 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
#include "ClassLoaderReferenceImpl.h"
#include "util.h"
#include "inStream.h"
#include "outStream.h"

static jboolean 
visibleClasses(PacketInputStream *in, PacketOutputStream *out)
{
    JNIEnv *env = getEnv();
    jint error;
    jint count;
    jclass *classes;
    int i;

    jobject loader = inStream_readClassLoaderRef(in);
    if (inStream_error(in)) {
        return JNI_TRUE;
    }

    error = jvmdi->GetClassLoaderClasses(loader, &count, &classes);
    if (error != JVMDI_ERROR_NONE) {
        outStream_setError(out, error);
        return JNI_TRUE;
    }

    outStream_writeInt(out, count);
    for (i = 0; i < count; i++) {
        jbyte tag;
        jclass clazz;

        clazz = classes[i];
        tag = referenceTypeTag(clazz);

        outStream_writeByte(out, tag);
        WRITE_GLOBAL_REF(env, out, clazz);
    }

    jdwpFree(classes);

    return JNI_TRUE;
}

void *ClassLoaderReference_Cmds[] = { (void *)0x1
    ,(void *)visibleClasses
};

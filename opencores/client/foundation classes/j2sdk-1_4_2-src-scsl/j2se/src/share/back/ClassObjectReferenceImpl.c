/*
 * @(#)ClassObjectReferenceImpl.c	1.9 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
#include <stdlib.h>
#include <string.h>

#include "ClassObjectReferenceImpl.h"
#include "util.h"
#include "inStream.h"
#include "outStream.h"

static jboolean 
reflectedType(PacketInputStream *in, PacketOutputStream *out)
{
    JNIEnv *env = getEnv();
    jclass clazz;
    jbyte tag;
    jobject object = inStream_readObjectRef(in);
    if (inStream_error(in)) {
        return JNI_TRUE;
    }

    /*
     * In our implementation, the reference type id is the same as the
     * class object id, so we bounce it right back.
     *
     * (Both inStream and WRITE_GLOBAL_REF) delete their global references
     * so we need to create a new one to write.)
     */
    clazz = (*env)->NewGlobalRef(env, object);
    if (clazz == NULL) {
        outStream_setError(out, JVMDI_ERROR_OUT_OF_MEMORY);
    } else {
        tag = referenceTypeTag(clazz);
        outStream_writeByte(out, tag);
        WRITE_GLOBAL_REF(env, out, clazz);
    }
    return JNI_TRUE;
}

void *ClassObjectReference_Cmds[] = { (void *)1
    ,(void *)reflectedType
};


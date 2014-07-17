/*
 * @(#)StringReferenceImpl.c	1.15 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
#include <string.h>

#include "StringReferenceImpl.h"
#include "util.h"
#include "inStream.h"
#include "outStream.h"

static jboolean 
value(PacketInputStream *in, PacketOutputStream *out)
{
    JNIEnv *env = getEnv();
    char *utf;
    jstring string = inStream_readStringRef(in);
    if (inStream_error(in)) {
        return JNI_TRUE;
    }

    WITH_LOCAL_REFS(env, 1);

    utf = (char *)(*env)->GetStringUTFChars(env, string, NULL);
    outStream_writeString(out, utf);
    (*env)->ReleaseStringUTFChars(env, string, utf);

    END_WITH_LOCAL_REFS(env);
    return JNI_TRUE;
}

void *StringReference_Cmds[] = { (void *)0x1
    ,(void *)value};

/*
 * @(#)ClassTypeImpl.c	1.30 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
#include <stdlib.h>

#include "ClassTypeImpl.h"
#include "util.h"
#include "inStream.h"
#include "outStream.h"
#include "JDWP.h"

static jboolean 
superclass(PacketInputStream *in, PacketOutputStream *out)
{
    JNIEnv *env = getEnv();
    jclass superclass;
    jclass clazz = inStream_readClassRef(in);
    if (inStream_error(in)) {
        return JNI_TRUE;
    }

    WITH_LOCAL_REFS(env, 1);

    superclass = (*env)->GetSuperclass(env,clazz);
    WRITE_LOCAL_REF(env, out, superclass);

    END_WITH_LOCAL_REFS(env);
    return JNI_TRUE;
}

static jint
readStaticFieldValue(JNIEnv *env, PacketInputStream *in, jclass clazz,
                     jfieldID field, char *signature)
{
    jvalue value;
    jint error = JVMDI_ERROR_NONE;

    switch (signature[0]) {
        case JDWP_Tag_ARRAY:
        case JDWP_Tag_OBJECT:
            value.l = inStream_readObjectRef(in);
            (*env)->SetStaticObjectField(env, clazz, field, value.l);
            break;
        
        case JDWP_Tag_BYTE:
            value.b = inStream_readByte(in);
            (*env)->SetStaticByteField(env, clazz, field, value.b);
            break;

        case JDWP_Tag_CHAR:
            value.c = inStream_readChar(in);
            (*env)->SetStaticCharField(env, clazz, field, value.c);
            break;

        case JDWP_Tag_FLOAT:
            value.f = inStream_readFloat(in);
            (*env)->SetStaticFloatField(env, clazz, field, value.f);
            break;

        case JDWP_Tag_DOUBLE:
            value.d = inStream_readDouble(in);
            (*env)->SetStaticDoubleField(env, clazz, field, value.d);
            break;

        case JDWP_Tag_INT:
            value.i = inStream_readInt(in);
            (*env)->SetStaticIntField(env, clazz, field, value.i);
            break;

        case JDWP_Tag_LONG:
            value.j = inStream_readLong(in);
            (*env)->SetStaticLongField(env, clazz, field, value.j);
            break;

        case JDWP_Tag_SHORT:
            value.s = inStream_readShort(in);
            (*env)->SetStaticShortField(env, clazz, field, value.s);
            break;

        case JDWP_Tag_BOOLEAN:
            value.z = inStream_readBoolean(in);
            (*env)->SetStaticBooleanField(env, clazz, field, value.z);
            break;
    }

    if ((*env)->ExceptionOccurred(env)) {
        error = JVMDI_ERROR_INTERNAL;
    }
    
    return error;
}

static jboolean 
setValues(PacketInputStream *in, PacketOutputStream *out)
{
    JNIEnv *env = getEnv();
    jint i;
    jfieldID field;
    char *signature;
    jint count;
    jint error = JVMDI_ERROR_NONE;

    jclass clazz = inStream_readClassRef(in);
    count = inStream_readInt(in);
    if (inStream_error(in)) {
        return JNI_TRUE;
    }

    WITH_LOCAL_REFS(env, count);

    for (i = 0; (i < count) && (error == JVMDI_ERROR_NONE); i++) {
        field = inStream_readFieldID(in);

        error = fieldSignature(clazz, field, &signature);
        if (error == JVMDI_ERROR_NONE) {
            error = readStaticFieldValue(env, in, clazz, 
                                         field, signature);
            jdwpFree(signature);
        }
    }

    END_WITH_LOCAL_REFS(env);
    return JNI_TRUE;
}

static jboolean 
invokeStatic(PacketInputStream *in, PacketOutputStream *out)
{
    return sharedInvoke(in, out);
}

/* Now done on front end 
static jboolean 
validateObjectAssignments(PacketInputStream *in, PacketOutputStream *out)
{
    JNIEnv *env = getEnv();
    jint i;

    jclass clazz = inStream_readClassRef(in);
    jint count = inStream_readInt(in);
    if (inStream_error(in)) {
        return JNI_TRUE;
    }

    outStream_writeInt(out, count);
    for (i = 0; i < count; i++) {
        jint result;
        jobject object;
        char *signature = inStream_readString(in);
        if (signature == NULL) {
            outStream_setError(out, JVMDI_ERROR_OUT_OF_MEMORY);
            return JNI_TRUE;
        }
        object = inStream_readObjectRef(in);
        if (inStream_error(in)) {
            return JNI_TRUE;
        }
        result = validateAssignment(clazz, object, signature);
        outStream_writeInt(out, result);
        jdwpFree(signature);
    }

    return JNI_TRUE;
} */

void *ClassType_Cmds[] = { (void *)0x4
    ,(void *)superclass
    ,(void *)setValues
    ,(void *)invokeStatic
    ,(void *)invokeStatic
};


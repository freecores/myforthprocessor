/*
 * @(#)inStream.c	1.18 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
#include <string.h>

#include "util.h"
#include "stream.h"
#include "inStream.h"
#include "transport.h"
#include "bag.h"
#include "commonRef.h"
#include "JDWP.h"

#define INITIAL_REF_ALLOC 50

/*
 * TO DO: Support processing of replies through command input streams.
 */
void
inStream_init(PacketInputStream *stream, struct Packet packet)
{
    /*
     * Copying packet root to the stream
     */
    stream->packet = packet;

    stream->error = JVMDI_ERROR_NONE;
    stream->segment = &stream->packet.type.cmd.data;
    stream->left = packet.type.cmd.data.length;
    stream->current = stream->segment->data; 
    stream->refs = bagCreateBag(sizeof(jobject), INITIAL_REF_ALLOC);
    if (stream->refs == NULL) {
        stream->error = JVMDI_ERROR_OUT_OF_MEMORY;
    }
}

jint 
inStream_id(PacketInputStream *stream)
{
    return stream->packet.type.cmd.id;
}

jbyte
inStream_command(PacketInputStream *stream)
{
    return stream->packet.type.cmd.cmd;
}

static jint 
readBytes(PacketInputStream *stream, void *dest, int size) 
{
    /*
     * Iteration handles items that span multiple packet segments
     */
    if (stream->error) {
        return stream->error;
    }
    while (size > 0) {
        jint count = MIN(size, stream->left);
        if (count == 0) {
            /* end of input */
            stream->error = JVMDI_ERROR_INTERNAL;
            return stream->error;
        }
        if (dest) {
            memcpy(dest, stream->current, count);
        }
        stream->current += count;
        stream->left -= count;
        if (stream->left == 0) {
            /*
             * Move to the next segment
             */
            stream->segment = stream->segment->next;
            if (stream->segment) {
                stream->current = stream->segment->data;
                stream->left = stream->segment->length;
            }
        }
        size -= count;
        if (dest) {
            dest = (char *)dest + count;
        }
    }
    return stream->error;
}

jint
inStream_skipBytes(PacketInputStream *stream, jint size) {
    return readBytes(stream, NULL, size);
}

jboolean 
inStream_readBoolean(PacketInputStream *stream)
{
    jbyte flag;
    readBytes(stream, &flag, sizeof(flag));
    if (stream->error) {
        return 0;
    } else {
        return flag ? JNI_TRUE : JNI_FALSE;
    }
}

jbyte 
inStream_readByte(PacketInputStream *stream)
{
    jbyte val = 0;
    readBytes(stream, &val, sizeof(val));
    return val;
}

jbyte *
inStream_readBytes(PacketInputStream *stream, int length, jbyte *buf)
{
    readBytes(stream, buf, length);
    return buf;
}

jchar 
inStream_readChar(PacketInputStream *stream)
{
    jchar val = 0;
    readBytes(stream, &val, sizeof(val));
    return JAVA_TO_HOST_CHAR(val);
}

jshort 
inStream_readShort(PacketInputStream *stream)
{
    jshort val = 0;
    readBytes(stream, &val, sizeof(val));
    return JAVA_TO_HOST_SHORT(val);
}

jint 
inStream_readInt(PacketInputStream *stream)
{
    jint val = 0;
    readBytes(stream, &val, sizeof(val));
    return JAVA_TO_HOST_INT(val);
}

jlong 
inStream_readLong(PacketInputStream *stream)
{
    jlong val = 0;
    readBytes(stream, &val, sizeof(val));
    return JAVA_TO_HOST_LONG(val);
}

jfloat 
inStream_readFloat(PacketInputStream *stream)
{
    jfloat val = 0;
    readBytes(stream, &val, sizeof(val));
    return JAVA_TO_HOST_FLOAT(val);
}

jdouble 
inStream_readDouble(PacketInputStream *stream)
{
    jdouble val = 0;
    readBytes(stream, &val, sizeof(val));
    return JAVA_TO_HOST_DOUBLE(val);
}

/*
 * Read an object from the stream. The ID used in the wire protocol
 * is converted to a reference which is returned. The reference is 
 * global and strong, but it should *not* be deleted by the caller
 * since it is freed when this stream is destroyed. 
 */
jobject 
inStream_readObjectRef(PacketInputStream *stream)
{
    jobject ref;
    jobject *refPtr;
    JNIEnv *env = getEnv();
    jlong id = inStream_readLong(stream);
    if (stream->error) {
        return NULL;
    } 
    if (id == NULL_OBJECT_ID) {
        return NULL;
    }

    ref = commonRef_idToRef(id);
    if (ref == NULL) {
        stream->error = JVMDI_ERROR_INVALID_OBJECT;
        return NULL;
    }

    refPtr = bagAdd(stream->refs);
    if (refPtr == NULL) {
        (*env)->DeleteGlobalRef(env, ref);
        return NULL;
    }

    *refPtr = ref;
    return ref;
}

/*
 * Read a raw object id from the stream. This should be used rarely.
 * Normally, inStream_readObjectRef is preferred since it takes care
 * of reference conversion and tracking. Only code that needs to 
 * perform maintence of the commonRef hash table uses this function.
 */
jlong 
inStream_readObjectID(PacketInputStream *stream)
{
    return inStream_readLong(stream);
}

jclass 
inStream_readClassRef(PacketInputStream *stream)
{
    jobject object = inStream_readObjectRef(stream);
    if (object == NULL) {
        /* 
         * Could be error or just the null reference. In either case,
         * stop now.
         */
        return NULL;
    }
    if (!isClass(object)) {
        stream->error = JVMDI_ERROR_INVALID_CLASS;
        return NULL;
    }
    return object;
}

jthread 
inStream_readThreadRef(PacketInputStream *stream)
{
    jobject object = inStream_readObjectRef(stream);
    if (object == NULL) {
        /* 
         * Could be error or just the null reference. In either case,
         * stop now.
         */
        return NULL;
    }
    if (!isThread(object)) {
        stream->error = JVMDI_ERROR_INVALID_THREAD;
        return NULL;
    }
    return object;
}

jthreadGroup 
inStream_readThreadGroupRef(PacketInputStream *stream)
{
    jobject object = inStream_readObjectRef(stream);
    if (object == NULL) {
        /* 
         * Could be error or just the null reference. In either case,
         * stop now.
         */
        return NULL;
    }
    if (!isThreadGroup(object)) {
        stream->error = JVMDI_ERROR_INVALID_THREAD_GROUP;
        return NULL;
    }
    return object;
}

jstring 
inStream_readStringRef(PacketInputStream *stream)
{
    jobject object = inStream_readObjectRef(stream);
    if (object == NULL) {
        /* 
         * Could be error or just the null reference. In either case,
         * stop now.
         */
        return NULL;
    }
    if (!isString(object)) {
        stream->error = JDWP_Error_INVALID_STRING;
        return NULL;
    }
    return object;
}

jclass 
inStream_readClassLoaderRef(PacketInputStream *stream)
{
    jobject object = inStream_readObjectRef(stream);
    if (object == NULL) {
        /* 
         * Could be error or just the null reference. In either case,
         * stop now.
         */
        return NULL;
    }
    if (!isClassLoader(object)) {
        stream->error = JDWP_Error_INVALID_CLASS_LOADER;
        return NULL;
    }
    return object;
}

jarray 
inStream_readArrayRef(PacketInputStream *stream)
{
    jobject object = inStream_readObjectRef(stream);
    if (object == NULL) {
        /* 
         * Could be error or just the null reference. In either case,
         * stop now.
         */
        return NULL;
    }
    if (!isArray(object)) {
        stream->error = JDWP_Error_INVALID_ARRAY;
        return NULL;
    }
    return object;
}

/*
 * Next 3 functions read an Int and convert to a Pointer!?
 * If sizeof(jxxxID) == 8 we must read these values as Longs.
 */
jframeID 
inStream_readFrameID(PacketInputStream *stream)
{
    if (sizeof(jframeID) == 8) {
        return (jframeID)inStream_readLong(stream);
    } else {
	return (jframeID)inStream_readInt(stream);
    }
}

jmethodID 
inStream_readMethodID(PacketInputStream *stream)
{
    if (sizeof(jmethodID) == 8) {
        return (jmethodID)inStream_readLong(stream);
    } else {
        return (jmethodID)inStream_readInt(stream);
    }
}

jfieldID 
inStream_readFieldID(PacketInputStream *stream)
{
    if (sizeof(jfieldID) == 8) {
        return (jfieldID)inStream_readLong(stream);
    } else {
        return (jfieldID)inStream_readInt(stream);
    }
}

jlocation 
inStream_readLocation(PacketInputStream *stream)
{
    return (jlocation)inStream_readLong(stream);
}

char * 
inStream_readString(PacketInputStream *stream)
{
    int length;
    char *string;
    length = inStream_readInt(stream);
    string = jdwpAlloc(length + 1);
    if (string != NULL) {
        readBytes(stream, string, length);
        string[length] = '\0';
    }
    return string;
}

jboolean 
inStream_endOfInput(PacketInputStream *stream)
{
    return (stream->left > 0);
}

jint 
inStream_error(PacketInputStream *stream)
{
    return stream->error;
}

jvalue 
inStream_readValue(PacketInputStream *stream, jbyte *typeKeyPtr)
{
    jvalue value;
    jbyte typeKey = inStream_readByte(stream);
    if (stream->error) {
        value.j = 0L;
        return value;
    }

    if (isObjectTag(typeKey)) {
        value.l = inStream_readObjectRef(stream);
    } else {
        switch (typeKey) {
            case JDWP_Tag_BYTE:
                value.b = inStream_readByte(stream);
                break;
    
            case JDWP_Tag_CHAR:
                value.c = inStream_readChar(stream);
                break;
    
            case JDWP_Tag_FLOAT:
                value.f = inStream_readFloat(stream);
                break;
    
            case JDWP_Tag_DOUBLE:
                value.d = inStream_readDouble(stream);
                break;
    
            case JDWP_Tag_INT:
                value.i = inStream_readInt(stream);
                break;
    
            case JDWP_Tag_LONG:
                value.j = inStream_readLong(stream);
                break;
    
            case JDWP_Tag_SHORT:
                value.s = inStream_readShort(stream);
                break;
    
            case JDWP_Tag_BOOLEAN:
                value.z = inStream_readBoolean(stream);
                break;
            default:
                stream->error = JDWP_Error_INVALID_TAG;
                break;
        }
    }
    if (typeKeyPtr) {
        *typeKeyPtr = typeKey;
    }
    return value;
}

static jboolean
deleteRef(void *elementPtr, void *arg)
{
    JNIEnv *env = arg;
    jobject *refPtr = elementPtr;
    (*env)->DeleteGlobalRef(env, *refPtr);
    return JNI_TRUE;
}

void 
inStream_destroy(PacketInputStream *stream)
{
    struct PacketData *next = stream->packet.type.cmd.data.next;
    jdwpFree(stream->packet.type.cmd.data.data);
    /*
     * TO DO: Decide who is responsible for packet memory. 
     * Perhaps there should ba a freePacket function added to 
     * the transport interface.
     */
    while (next != NULL) {
        struct PacketData *p = next;
        next = p->next;
        jdwpFree(p->data);
        jdwpFree(p);
    }

    bagEnumerateOver(stream->refs, deleteRef, (void *)getEnv());
    bagDestroyBag(stream->refs);
}



/*
 * @(#)SharedMemoryConnection.c	1.8 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
#include <stdlib.h>
#include <jni.h>
#include "SharedMemory.h"
#include "com_sun_tools_jdi_SharedMemoryConnection.h"
#include "transportSPI.h"
#include "shmemBase.h"
#include "sys.h" 

/*
 * JNI interface to the shared memory transport. These JNI methods 
 * call the base shared memory support to do the real work.
 *
 * That is, this is the front-ends interface to our shared memory
 * communication code.
 */
 
jfieldID idField;
jfieldID flagsField;
jfieldID cmdSetField;
jfieldID cmdField;
jfieldID errorCodeField;
jfieldID dataField;

static void
initFields(JNIEnv *env, jclass clazz)
{
    idField = (*env)->GetFieldID(env, clazz, "id", "I");
    if ((*env)->ExceptionOccurred(env)) {
        return;
    }
    flagsField = (*env)->GetFieldID(env, clazz, "flags", "S");
    if ((*env)->ExceptionOccurred(env)) {
        return;
    }
    cmdSetField = (*env)->GetFieldID(env, clazz, "cmdSet", "S");
    if ((*env)->ExceptionOccurred(env)) {
        return;
    }
    cmdField = (*env)->GetFieldID(env, clazz, "cmd", "S");
    if ((*env)->ExceptionOccurred(env)) {
        return;
    }
    errorCodeField = (*env)->GetFieldID(env, clazz, "errorCode", "S");
    if ((*env)->ExceptionOccurred(env)) {
        return;
    }
    dataField = (*env)->GetFieldID(env, clazz, "data", "[B");
    if ((*env)->ExceptionOccurred(env)) {
        return;
    }
}

/*
 * Fill a packet object from a packet struct.
 */
static void
setPacketObject(JNIEnv *env, Packet *str, jobject obj)
{
    jbyteArray array;
    jsize length;
    jsize start;
    PacketData *chunk;

    if (idField == NULL) {
        initFields(env, (*env)->GetObjectClass(env, obj));
        if ((*env)->ExceptionOccurred(env)) {
            return;
        }
    }
    (*env)->SetIntField(env, obj, idField, str->type.cmd.id);
    (*env)->SetShortField(env, obj, flagsField, str->type.cmd.flags);


    if (str->type.cmd.flags & FLAGS_Reply) {
        (*env)->SetShortField(env, obj, errorCodeField, str->type.reply.errorCode);
    } else {
        (*env)->SetShortField(env, obj, cmdSetField, str->type.cmd.cmdSet);
        (*env)->SetShortField(env, obj, cmdField, str->type.cmd.cmd);
    }

    length = 0;
    chunk = &str->type.cmd.data;
    while (chunk != NULL) {
        length = chunk->length;
        chunk = chunk->next;
    }

    array = (*env)->NewByteArray(env, length);
    if ((*env)->ExceptionOccurred(env)) {
        return;
    }

    chunk = &str->type.cmd.data;
    start = 0;
    while (chunk != NULL) {
        (*env)->SetByteArrayRegion(env, array, start, 
                                   chunk->length, chunk->data);
        if ((*env)->ExceptionOccurred(env)) {
            break;
        }
        start += chunk->length;
        chunk = chunk->next;
    }
    if ((*env)->ExceptionOccurred(env)) {
        return;
    }

    (*env)->SetObjectField(env, obj, dataField, array);
}

/*
 * Fill a packet struct from a packet object.
 */
static void
setPacketStruct(JNIEnv *env, jobject obj, Packet *str)
{
    jbyteArray array;
    jsize length;
    jbyte *data;

    if (idField == NULL) {
        initFields(env, (*env)->GetObjectClass(env, obj));
        if ((*env)->ExceptionOccurred(env)) {
            return;
        }
    }

    str->type.cmd.id = (*env)->GetIntField(env, obj, idField);
    str->type.cmd.flags = (jbyte)(*env)->GetShortField(env, obj, flagsField);
    str->type.cmd.cmdSet = (jbyte)(*env)->GetShortField(env, obj, cmdSetField);
    str->type.cmd.cmd = (jbyte)(*env)->GetShortField(env, obj, cmdField);
    array = (*env)->GetObjectField(env, obj, dataField);

    length = (*env)->GetArrayLength(env, array);

    if (length == 0) {
        data = NULL;
    } else {
        data = malloc(length);
        if (data == NULL) {
            throwException(env, "java/lang/OutOfMemoryError", 
                           "Unable to allocate command data buffer");
            return;
        }
    
        (*env)->GetByteArrayRegion(env, array, 0, length, data);
        if ((*env)->ExceptionOccurred(env)) {
            free(data);
            return;
        }
    }

    str->type.cmd.data.length = length;
    str->type.cmd.data.data = data;
    str->type.cmd.data.next = NULL;
}

static void
freePacketData(Packet *packet) 
{
    PacketData *next;

    if (packet->type.cmd.data.length > 0) {
        free(packet->type.cmd.data.data);
    }
    next = packet->type.cmd.data.next;
    while (next != NULL) {
        struct PacketData *p = next;
        next = p->next;
        free(p->data);
        free(p);
    }
}

/*
 * Class:     com_sun_tools_jdi_SharedMemoryConnection
 * Method:    close0
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_com_sun_tools_jdi_SharedMemoryConnection_close0
  (JNIEnv *env, jobject thisObject, jlong id)
{
    SharedMemoryConnection *connection = ID_TO_CONNECTION(id);
    shmemBase_closeConnection(connection);
}

/*
 * Class:     com_sun_tools_jdi_SharedMemoryConnection
 * Method:    receiveByte0
 * Signature: (J)B
 */
JNIEXPORT jbyte JNICALL Java_com_sun_tools_jdi_SharedMemoryConnection_receiveByte0
  (JNIEnv *env, jobject thisObject, jlong id)
{
    SharedMemoryConnection *connection = ID_TO_CONNECTION(id);
    jbyte b = 0;
    jint rc;

    rc = shmemBase_receiveByte(connection, &b);
    if (rc != SYS_OK) {
        throwShmemException(env, "shmemBase_receiveByte failed", rc);
    }

    return b;
}

/*
 * Class:     com_sun_tools_jdi_SharedMemoryConnection
 * Method:    receivePacket0
 * Signature: (JLcom/sun/tools/jdi/Packet;)V
 */
JNIEXPORT void JNICALL Java_com_sun_tools_jdi_SharedMemoryConnection_receivePacket0
  (JNIEnv *env, jobject thisObject, jlong id, jobject packetObject)
{
    SharedMemoryConnection *connection = ID_TO_CONNECTION(id);
    Packet packet;
    jint rc;

    rc = shmemBase_receivePacket(connection, &packet);
    if (rc != SYS_OK) {
        throwShmemException(env, "shmemBase_receivePacket failed", rc);
    } else {
        setPacketObject(env, &packet, packetObject);

        /* Free the packet even if there was an exception above */
        freePacketData(&packet);
    }
}

/*
 * Class:     com_sun_tools_jdi_SharedMemoryConnection
 * Method:    sendByte0
 * Signature: (JB)V
 */
JNIEXPORT void JNICALL Java_com_sun_tools_jdi_SharedMemoryConnection_sendByte0
  (JNIEnv *env, jobject thisObject, jlong id, jbyte b)
{
    SharedMemoryConnection *connection = ID_TO_CONNECTION(id);
    jint rc;

    rc = shmemBase_sendByte(connection, b);
    if (rc != SYS_OK) {
        throwShmemException(env, "shmemBase_sendByte failed", rc);
    }
}

/*
 * Class:     com_sun_tools_jdi_SharedMemoryConnection
 * Method:    sendPacket0
 * Signature: (JLcom/sun/tools/jdi/Packet;)V
 */
JNIEXPORT void JNICALL Java_com_sun_tools_jdi_SharedMemoryConnection_sendPacket0
  (JNIEnv *env, jobject thisObject, jlong id, jobject packetObject)
{
    SharedMemoryConnection *connection = ID_TO_CONNECTION(id);
    Packet packet;
    jint rc;

    setPacketStruct(env, packetObject, &packet);
    if ((*env)->ExceptionOccurred(env)) {
        return;
    }

    rc = shmemBase_sendPacket(connection, &packet);
    if (rc != SYS_OK) {
        throwShmemException(env, "shmemBase_sendPacket failed", rc);
    }  
    freePacketData(&packet);
}
 
 


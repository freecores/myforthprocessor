/*
 * @(#)shmemBack.c	1.10 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
#include <string.h>

#include "transportSPI.h"
#include "shmemBase.h"
#include "sys.h"

/*
 * Back-end access to the shared memory transport implementation.
 * Access to which is thru the dispatch table "functions", below.
 * Installed by JDWP_OnLoad(), also below.
 * Interface defined by transportSPI.h
 */
 
SharedMemoryTransport *transport = NULL;  /* maximum of 1 transport */ 
SharedMemoryConnection *connection = NULL;  /* maximum of 1 connection */ 
TransportCallback *callbacks;

static jint
shmemListen(char **address) 
{
    jint rc = shmemBase_listen(*address, &transport);
    /*
     * If a name was selected by the function above, find it and return
     * it in place of the original arg. 
     */
    if ((rc == SYS_OK) && ((*address == NULL) || (*address[0] == '\0'))) {
        char *name;
        char *name2;
        rc = shmemBase_name(transport, &name);
        if (rc == SYS_OK) {
            name2 = (callbacks->alloc)(strlen(name) + 1);
            if (name2 == NULL) {
                rc = SYS_ERR;
            } else {
                strcpy(name2, name);
                *address = name2;
            }
        }
    }
    return rc;
}

static jint
shmemAccept(void) 
{
    return shmemBase_accept(transport, &connection);
}

static jint
shmemStopListening(void) 
{
    shmemBase_closeTransport(transport);
    return SYS_OK;
}

static jint
shmemAttach(char *address) 
{
    return shmemBase_attach(address, &connection);
}

static jint
shmemSendByte(jbyte b) 
{
    return shmemBase_sendByte(connection, b);
}

static jint
shmemReceiveByte(jbyte *b) 
{
    return shmemBase_receiveByte(connection, b);
}

static jint
shmemSendPacket(struct Packet *packet) 
{
    return shmemBase_sendPacket(connection, packet);
}

static jint
shmemReceivePacket(struct Packet *packet) 
{
    return shmemBase_receivePacket(connection, packet);
}

static void
shmemClose(void) 
{
    shmemBase_closeConnection(connection);
}

struct Transport functions = {
    shmemListen,
    shmemAccept,
    shmemStopListening,
    shmemAttach,
    shmemSendByte,
    shmemReceiveByte,
    shmemSendPacket,
    shmemReceivePacket,
    shmemClose
};

JNIEXPORT jint JNICALL 
JDWP_OnLoad(JavaVM *vm, Transport **tablePtr, 
            TransportCallback *cbTablePtr, void *reserved) 
{
    jint rc = shmemBase_initialize(vm, cbTablePtr);
    if (rc != SYS_OK) {
        return rc;
    }
    *tablePtr = &functions;
    callbacks = cbTablePtr;
    return 0;
}



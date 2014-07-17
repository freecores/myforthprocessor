/*
 * @(#)shmemBase.c	1.14 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include "sysShmem.h"
#include "shmemBase.h"
#include "transportSPI.h"  /* for Packet, TransportCallback */
#include "util.h" /* for MIN */

/*
 * This is the base shared memory transport implementation that is used
 * by both front-end transports (through com.sun.tools.jdi) and 
 * back-end transports (through JDWP_OnLoad and the function tables 
 * it requires). It supports multiple connections for the benefit of the 
 * front-end client; the back end interface assumes only a single connection.
 */  

#define MAX_IPC_PREFIX 50   /* user-specified or generated name for */
                            /* shared memory seg and prefix for other IPC */
#define MAX_IPC_SUFFIX 25   /* suffix to shmem name for other IPC names */
#define MAX_IPC_NAME   (MAX_IPC_PREFIX + MAX_IPC_SUFFIX)

#define MAX_GENERATION_RETRIES 20
#define SHARED_BUFFER_SIZE 5000

#define CHECK_ERROR(expr) do { \
                              jint error = (expr); \
                              if (error != SYS_OK) { \
                                  return error; \
                              } \
                          } while (0)

/*
 * The following assertions should hold anytime the stream's mutex is not held
 */
#define STREAM_INVARIANT(stream) \
        do { \
            SHMEM_ASSERT((stream->shared->readOffset < SHARED_BUFFER_SIZE) \
                         && (stream->shared->readOffset >= 0)); \
            SHMEM_ASSERT((stream->shared->writeOffset < SHARED_BUFFER_SIZE) \
                         && (stream->shared->writeOffset >= 0)); \
        } while (0)

/*   
 * Transports are duplex, so carve the shared memory into "streams",
 * one used to send from client to server, the other vice versa.
 */
typedef struct SharedMemoryListener {
    char mutexName[MAX_IPC_NAME];
    char acceptEventName[MAX_IPC_NAME];
    char attachEventName[MAX_IPC_NAME];
    jboolean isListening;
    jboolean isAccepted;
    jlong acceptingPID;
    jlong attachingPID;
} SharedListener;

typedef struct SharedMemoryTransport {
    char name[MAX_IPC_PREFIX];
    sys_ipmutex_t mutex;
    sys_event_t acceptEvent;
    sys_event_t attachEvent;
    sys_shmem_t sharedMemory;
    SharedListener *shared;
} SharedMemoryTransport;

/*
 * Access must be syncronized.  Holds one shared
 * memory buffer and its state.
 */
typedef struct SharedStream {
    char mutexName[MAX_IPC_NAME];
    char hasDataEventName[MAX_IPC_NAME];
    char hasSpaceEventName[MAX_IPC_NAME];
    int readOffset;
    int writeOffset;
    jboolean isFull;
    jbyte buffer[SHARED_BUFFER_SIZE];
} SharedStream;

/*
 * The two shared streams: client to server and
 * server to client.
 */
typedef struct SharedMemory {
    SharedStream toClient;
    SharedStream toServer;
} SharedMemory;

/*
 * Local (to process) access to the shared memory 
 * stream.  access to hasData and hasSpace synchronized
 * by OS.
 */
typedef struct Stream {
    sys_ipmutex_t mutex;
    sys_event_t hasData;
    sys_event_t hasSpace;
    SharedStream *shared;
    jint state;
} Stream;

/*
 * Values for Stream.state field above.
 */
#define STATE_CLOSED 0xDEAD
#define STATE_OPEN   (STATE_CLOSED -1)
/*
 * State checking macro. We compare against the STATE_OPEN value so
 * that STATE_CLOSED and any other value will be considered closed.
 * This catches a freed Stream as long as the memory page is still
 * valid. If the memory page is gone, then there is little that we
 * can do.
 */
#define IS_STATE_CLOSED(state) (state != STATE_OPEN)


typedef struct SharedMemoryConnection {
    char name[MAX_IPC_NAME];
    SharedMemory *shared;
    sys_shmem_t sharedMemory;
    Stream incoming;   
    Stream outgoing;
    sys_process_t otherProcess;
    sys_event_t shutdown;	    /* signalled to indicate shutdown */
} SharedMemoryConnection;

static TransportCallback *callback;
static JavaVM *jvm;

typedef jint (*CreateFunc)(char *name, void *arg);

static void 
exitWithError(char *fileName, char *date, int lineNumber, 
              char *message, jint errorCode)
{
    char buf[500];

    /* Get past path info */
    char *p1 = strrchr(fileName, '\\');
    char *p2 = strrchr(fileName, '/');
    p1 = ((p1 > p2) ? p1 : p2);
    if (p1 != NULL) {
        fileName = p1 + 1;
    }

    if (errorCode != 0) {		                              
        sprintf(buf, "JDI \"%s\" (%s), line %d: %s, error code = %d (%s)\n",
                fileName, date, lineNumber, message, errorCode, 
                "");             \
    } else {                                                       
        sprintf(buf, "JDI \"%s\" (%s), line %d: %s\n",                    
                fileName, date, lineNumber, message);             \
    }                                                              
    printf("%s", buf);
    exit(0);
}

jint 
shmemBase_initialize(JavaVM *vm, TransportCallback *cbPtr)
{
    jvm = vm;
    callback = cbPtr;
    return SYS_OK;
}

static jint
createWithGeneratedName(char *prefix, char *nameBuffer, CreateFunc func, void *arg)
{
    jint error;
    jint i = 0;

    do {
        strcpy(nameBuffer, prefix);
        if (i > 0) {
            char buf[10];
            sprintf(buf, ".%d", i+1);
            strcat(nameBuffer, buf);
        }
        error = func(nameBuffer, arg);
        i++;
    } while ((error == SYS_INUSE) && (i < MAX_GENERATION_RETRIES));

    return error;
}

typedef struct SharedMemoryArg {
    jint size;
    sys_shmem_t memory;
    void *start;
} SharedMemoryArg;

static jint 
createSharedMem(char *name, void *ptr)
{
    SharedMemoryArg *arg = ptr;
    return sysSharedMemCreate(name, arg->size, &arg->memory, &arg->start);
}

static jint 
createMutex(char *name, void *arg)
{
    sys_ipmutex_t *retArg = arg;
    return sysIPMutexCreate(name, retArg);
}

/*
 * Creates named or unnamed event that is automatically reset 
 * (in other words, no need to reset event after it has signalled
 * a thread).
 */
static jint 
createEvent(char *name, void *arg)
{
    sys_event_t *retArg = arg;
    return sysEventCreate(name, retArg, JNI_FALSE);
}

#define ADD_OFFSET(o1, o2) ((o1 + o2) % SHARED_BUFFER_SIZE)
#define FULL(stream) (stream->shared->isFull)
#define EMPTY(stream) ((stream->shared->writeOffset == stream->shared->readOffset) \
                       && !stream->shared->isFull)

static jint
leaveMutex(Stream *stream)
{
    return sysIPMutexExit(stream->mutex);
}

/* enter the stream's mutex and (optionally) check for a closed stream */
static jint
enterMutex(Stream *stream, sys_event_t event)
{
    jint ret = sysIPMutexEnter(stream->mutex, event);
    if (ret != SYS_OK) {
        return ret;
    }
    if (IS_STATE_CLOSED(stream->state)) {
        (void)leaveMutex(stream);
        return SYS_ERR;
    }
    return SYS_OK;
}

/*
 * Enter/exit with stream mutex held.
 * On error, does not hold the stream mutex.
 */
static jint 
waitForSpace(SharedMemoryConnection *connection, Stream *stream)
{
    jint error = SYS_OK; 

    /* Assumes mutex is held on call */
    while ((error == SYS_OK) && FULL(stream)) {
        CHECK_ERROR(leaveMutex(stream));
        error = sysEventWait(connection->otherProcess, stream->hasSpace);
        if (error == SYS_OK) {
            CHECK_ERROR(enterMutex(stream, connection->shutdown));
        }
    }
    return error;
}

static jint
signalSpace(Stream *stream)
{
    return sysEventSignal(stream->hasSpace);
}

/*
 * Enter/exit with stream mutex held.
 * On error, does not hold the stream mutex.
 */
static jint
waitForData(SharedMemoryConnection *connection, Stream *stream)
{
    jint error = SYS_OK;

    /* Assumes mutex is held on call */
    while ((error == SYS_OK) && EMPTY(stream)) {
        CHECK_ERROR(leaveMutex(stream));
        error = sysEventWait(connection->otherProcess, stream->hasData);
        if (error == SYS_OK) {
            CHECK_ERROR(enterMutex(stream, connection->shutdown));
        }
    }
    return error;
}

static jint
signalData(Stream *stream)
{
    return sysEventSignal(stream->hasData);
}


static jint
closeStream(Stream *stream) 
{
    /* 
     * Lock stream during close - ignore shutdown event as we are
     * closing down and shutdown should be signalled.
     */
    CHECK_ERROR(enterMutex(stream, NULL));

    /* mark the stream as closed */
    stream->state = STATE_CLOSED;
    /* wake up waitForData() if it is in sysEventWait() */
    sysEventSignal(stream->hasData);
    sysEventClose(stream->hasData);
    /* wake up waitForSpace() if it is in sysEventWait() */
    sysEventSignal(stream->hasSpace);
    sysEventClose(stream->hasSpace);
    CHECK_ERROR(leaveMutex(stream));
    sysIPMutexClose(stream->mutex);
    return SYS_OK;
}

/*
 * Server creates stream.
 */
static int
createStream(char *name, Stream *stream)
{
    jint error;
    char prefix[MAX_IPC_PREFIX];

    sprintf(prefix, "%s.mutex", name);
    error = createWithGeneratedName(prefix, stream->shared->mutexName,
                                    createMutex, &stream->mutex);
    if (error != SYS_OK) {
        fprintf(stderr,"Error creating mutex, rc = %d\n", error);
        return error;
    }

    sprintf(prefix, "%s.hasData", name);
    error = createWithGeneratedName(prefix, stream->shared->hasDataEventName,
                                    createEvent, &stream->hasData);
    if (error != SYS_OK) {
        fprintf(stderr,"Error creating event, rc = %d\n", error);
        (void)closeStream(stream);
        return error;
    }

    sprintf(prefix, "%s.hasSpace", name);
    error = createWithGeneratedName(prefix, stream->shared->hasSpaceEventName,
                                    createEvent, &stream->hasSpace);
    if (error != SYS_OK) {
        fprintf(stderr,"Error creating event, rc = %d\n", error);
        (void)closeStream(stream);
        return error;
    }

    stream->shared->readOffset = 0;
    stream->shared->writeOffset = 0;
    stream->shared->isFull = JNI_FALSE;
    stream->state = STATE_OPEN;
    return SYS_OK;
}


/*
 * Initialization for the stream opened by the other process
 */
static int
openStream(Stream *stream)
{
    jint error;

    error = sysIPMutexOpen(stream->shared->mutexName, &stream->mutex);
    if (error != SYS_OK) {
        fprintf(stderr,"Error accessing mutex, rc = %d\n", error);
        return error;
    }

    error = sysEventOpen(stream->shared->hasDataEventName,
                             &stream->hasData);
    if (error != SYS_OK) {
        fprintf(stderr,"Error accessing mutex, rc = %d\n", error);
        (void)closeStream(stream);
        return error;
    }

    error = sysEventOpen(stream->shared->hasSpaceEventName,
                             &stream->hasSpace);
    if (error != SYS_OK) {
        fprintf(stderr,"Error accessing mutex, rc = %d\n", error);
        (void)closeStream(stream);
        return error;
    }

    stream->state = STATE_OPEN;

    return SYS_OK;
}

/********************************************************************/

static SharedMemoryConnection *
allocConnection(void)
{
    /*
     * TO DO: Track all allocated connections for clean shutdown?
     */
    SharedMemoryConnection *conn = (*callback->alloc)(sizeof(SharedMemoryConnection));
    if (conn != NULL) {
	memset(conn, 0, sizeof(SharedMemoryConnection));
    }
    return conn;
}

static void 
freeConnection(SharedMemoryConnection *connection)
{
    (*callback->free)(connection);
}

static void
closeConnection(SharedMemoryConnection *connection)
{
    /*
     * Signal all threads accessing this connection that we are
     * shutting down.
     */
    if (connection->shutdown) {
	sysEventSignal(connection->shutdown);
    }


    (void)closeStream(&connection->incoming);
    (void)closeStream(&connection->outgoing);

    if (connection->sharedMemory) {
	sysSharedMemClose(connection->sharedMemory, connection->shared);
    }
    if (connection->otherProcess) {
	sysProcessClose(connection->otherProcess);
    }

    /*
     * Ideally we should close the connection->shutdown event and 
     * free the connection structure. However as closing the 
     * connection is asynchronous it means that other threads may
     * still be accessing the connection structure. On Win32 this
     * means we leak 132 bytes and one event per connection. This
     * memory will be reclaim at process exit.
     *
     * if (connection->shutdown) 
     *     sysEventClose(connection->shutdown);
     * freeConnection(connection);
     */
}


/*
 * For client: connect to the shared memory.  Open incoming and
 * outgoing streams.
 */
static jint
openConnection(SharedMemoryTransport *transport, jlong otherPID, 
               SharedMemoryConnection **connectionPtr)
{
    jint error;

    SharedMemoryConnection *connection = allocConnection();
    if (connection == NULL) {
        return SYS_NOMEM;
    }

    sprintf(connection->name, "%s.%ld", transport->name, sysProcessGetID());
    error = sysSharedMemOpen(connection->name, &connection->sharedMemory,
                             &connection->shared);
    if (error != SYS_OK) {
        closeConnection(connection);
        return error;
    }

    /* This process is the client */
    connection->incoming.shared = &connection->shared->toClient;
    connection->outgoing.shared = &connection->shared->toServer;

    error = openStream(&connection->incoming);
    if (error != SYS_OK) {
        closeConnection(connection);
        return error;
    }

    error = openStream(&connection->outgoing);
    if (error != SYS_OK) {
        closeConnection(connection);
        return error;
    }

    error = sysProcessOpen(otherPID, &connection->otherProcess);
    if (error != SYS_OK) {
        fprintf(stderr,"Error accessing process, rc = %d\n", error);
        closeConnection(connection);
        return error;
    }

    /*
     * Create an event that signals that the connection is shutting 
     * down. The event is unnamed as it's process local, and is 
     * manually reset (so that signalling the event will signal
     * all threads waiting on it).
     */
    error = sysEventCreate(NULL, &connection->shutdown, JNI_TRUE); 
    if (error != SYS_OK) {
        fprintf(stderr,"Error creating unnamed event, rc = %d\n", error);
	closeConnection(connection);
	return error;
    }

    *connectionPtr = connection;
    return SYS_OK;
}

/*
 * For server: create the shared memory.  Create incoming and
 * outgoing streams.
 */
static jint
createConnection(SharedMemoryTransport *transport, jlong otherPID, 
                 SharedMemoryConnection **connectionPtr)
{
    jint error;
    char streamPrefix[MAX_IPC_NAME];

    SharedMemoryConnection *connection = allocConnection();
    if (connection == NULL) {
        return SYS_NOMEM;
    }

    sprintf(connection->name, "%s.%ld", transport->name, otherPID);
    error = sysSharedMemCreate(connection->name, sizeof(SharedMemory),
                               &connection->sharedMemory, &connection->shared);
    if (error != SYS_OK) {
        closeConnection(connection);
        return error;
    }

    memset(connection->shared, 0, sizeof(SharedMemory));

    /* This process is the server */
    connection->incoming.shared = &connection->shared->toServer;
    connection->outgoing.shared = &connection->shared->toClient;

    strcpy(streamPrefix, connection->name);
    strcat(streamPrefix, ".ctos");
    error = createStream(streamPrefix, &connection->incoming);
    if (error != SYS_OK) {
        closeConnection(connection);
        return error;
    }

    strcpy(streamPrefix, connection->name);
    strcat(streamPrefix, ".stoc");
    error = createStream(streamPrefix, &connection->outgoing);
    if (error != SYS_OK) {
        closeConnection(connection);
        return error;
    }

    error = sysProcessOpen(otherPID, &connection->otherProcess);
    if (error != SYS_OK) {
        fprintf(stderr,"Error accessing process, rc = %d\n", error);
        closeConnection(connection);
        return error;
    }

    /*
     * Create an event that signals that the connection is shutting 
     * down. The event is unnamed as it's process local, and is 
     * manually reset (so that a signalling the event will signal
     * all threads waiting on it).
     */
    error = sysEventCreate(NULL, &connection->shutdown, JNI_TRUE);
    if (error != SYS_OK) {
        fprintf(stderr,"Error creating unnamed event, rc = %d\n", error);
	closeConnection(connection);
	return error;
    }

    *connectionPtr = connection;
    return SYS_OK;
}

/********************************************************************/

static SharedMemoryTransport *
allocTransport(void)
{
    /*
     * TO DO: Track all allocated transports for clean shutdown?
     */
    return (*callback->alloc)(sizeof(SharedMemoryTransport));
}

static void 
freeTransport(SharedMemoryTransport *transport)
{
    (*callback->free)(transport);
}

static void
closeTransport(SharedMemoryTransport *transport) 
{
    sysIPMutexClose(transport->mutex);
    sysEventClose(transport->acceptEvent);
    sysEventClose(transport->attachEvent);
    sysSharedMemClose(transport->sharedMemory, transport->shared);
    freeTransport(transport);
}

static int
openTransport(const char *address, SharedMemoryTransport **transportPtr)
{
    jint error;
    SharedMemoryTransport *transport;

    transport = allocTransport();
    if (transport == NULL) {
        return SYS_NOMEM;
    }
    memset(transport, 0, sizeof(*transport));

    if (strlen(address) >= MAX_IPC_PREFIX) {
        fprintf(stderr,"Error: address strings longer than %d characters are invalid\n", MAX_IPC_PREFIX);
        closeTransport(transport);
        return SYS_ERR;
    }

    error = sysSharedMemOpen(address, &transport->sharedMemory, &transport->shared);
    if (error != SYS_OK) {
        fprintf(stderr,"Error accessing shared memory, rc = %d\n", error);
        closeTransport(transport);
        return error;
    }
    strcpy(transport->name, address);

    error = sysIPMutexOpen(transport->shared->mutexName, &transport->mutex);
    if (error != SYS_OK) {
        fprintf(stderr,"Error accessing mutex, rc = %d\n", error);
        closeTransport(transport);
        return error;
    }

    error = sysEventOpen(transport->shared->acceptEventName,
                             &transport->acceptEvent);
    if (error != SYS_OK) {
        fprintf(stderr,"Error accessing mutex, rc = %d\n", error);
        closeTransport(transport);
        return error;
    }

    error = sysEventOpen(transport->shared->attachEventName,
                             &transport->attachEvent);
    if (error != SYS_OK) {
        fprintf(stderr,"Error accessing mutex, rc = %d\n", error);
        closeTransport(transport);
        return error;
    }

    *transportPtr = transport;
    return SYS_OK;
}

static jint 
createTransport(const char *address, SharedMemoryTransport **transportPtr)
{
    SharedMemoryTransport *transport;
    jint error;
    char prefix[MAX_IPC_PREFIX];



    transport = allocTransport();
    if (transport == NULL) {
        return SYS_NOMEM;
    }
    memset(transport, 0, sizeof(*transport));

    if ((address == NULL) || (address[0] == '\0')) {
        SharedMemoryArg arg;
        arg.size = sizeof(SharedListener);
        error = createWithGeneratedName("javadebug", transport->name,
                                        createSharedMem, &arg);
        transport->shared = arg.start;
        transport->sharedMemory = arg.memory;
    } else {
        if (strlen(address) >= MAX_IPC_PREFIX) {
            fprintf(stderr,"Error: address strings longer than %d characters are invalid\n", MAX_IPC_PREFIX);
            closeTransport(transport);
            return SYS_ERR;
        }
        strcpy(transport->name, address);
        error = sysSharedMemCreate(address, sizeof(SharedListener),
                                   &transport->sharedMemory, &transport->shared);
    }
    if (error != SYS_OK) {
        fprintf(stderr,"Error creating shared memory, rc = %d\n", error);
        closeTransport(transport);
        return error;
    }

    memset(transport->shared, 0, sizeof(SharedListener));
    transport->shared->acceptingPID = sysProcessGetID();

    sprintf(prefix, "%s.mutex", transport->name);
    error = createWithGeneratedName(prefix, transport->shared->mutexName,
                                    createMutex, &transport->mutex);
    if (error != SYS_OK) {
        fprintf(stderr,"Error creating mutex, rc = %d\n", error);
        closeTransport(transport);
        return error;
    }

    sprintf(prefix, "%s.accept", transport->name);
    error = createWithGeneratedName(prefix, transport->shared->acceptEventName,
                                    createEvent, &transport->acceptEvent);
    if (error != SYS_OK) {
        fprintf(stderr,"Error creating event, rc = %d\n", error);
        closeTransport(transport);
        return error;
    }

    sprintf(prefix, "%s.attach", transport->name);
    error = createWithGeneratedName(prefix, transport->shared->attachEventName,
                                    createEvent, &transport->attachEvent);
    if (error != SYS_OK) {
        fprintf(stderr,"Error creating event, rc = %d\n", error);
        closeTransport(transport);
        return error;
    }

    *transportPtr = transport;
    return SYS_OK;
}


jint 
shmemBase_listen(const char *address, SharedMemoryTransport **transportPtr)
{
    int error;

    error = createTransport(address, transportPtr);
    if (error == SYS_OK) {
        (*transportPtr)->shared->isListening = JNI_TRUE;
    }
    return error;
}


jint
shmemBase_accept(SharedMemoryTransport *transport, 
                 SharedMemoryConnection **connectionPtr) 
{
    jint error;
    SharedMemoryConnection *connection;

    CHECK_ERROR(sysEventWait(NULL, transport->attachEvent));


    error = createConnection(transport, transport->shared->attachingPID, 
                             &connection);
    if (error != SYS_OK) {
        /*
         * Reject the attacher
         */
        transport->shared->isAccepted = JNI_FALSE;
        sysEventSignal(transport->acceptEvent);

        freeConnection(connection);
        return error;
    }

    transport->shared->isAccepted = JNI_TRUE;
    error = sysEventSignal(transport->acceptEvent);
    if (error != SYS_OK) {
        /*
         * No real point trying to reject it.
         */
        closeConnection(connection);
        return error;
    }

    *connectionPtr = connection;
    return SYS_OK;
}

static jint
doAttach(SharedMemoryTransport *transport) 
{
    transport->shared->attachingPID = sysProcessGetID();
    CHECK_ERROR(sysEventSignal(transport->attachEvent));
    CHECK_ERROR(sysEventWait(NULL, transport->acceptEvent));
    return SYS_OK;
}

jint
shmemBase_attach(const char *addressString, SharedMemoryConnection **connectionPtr) 
{
    int error;
    SharedMemoryTransport *transport;
    jlong acceptingPID;

    error = openTransport(addressString, &transport);
    if (error != SYS_OK) {
        return error;
    }
    
    /* lock transport - no additional event to wait on as no connection yet */
    error = sysIPMutexEnter(transport->mutex, NULL);
    if (error != SYS_OK) {
        closeTransport(transport);
        return error;
    }

    if (transport->shared->isListening) {
        error = doAttach(transport);
        if (error == SYS_OK) {
            acceptingPID = transport->shared->acceptingPID;
        }
    } else {
        /* Not listening: error */
        error = SYS_ERR;
    }

    sysIPMutexExit(transport->mutex);
    if (error != SYS_OK) {
        closeTransport(transport);
        return error;
    }
    
    error = openConnection(transport, acceptingPID, connectionPtr);

    closeTransport(transport);

    return error;
}




void 
shmemBase_closeConnection(SharedMemoryConnection *connection)
{
    closeConnection(connection);
}

void 
shmemBase_closeTransport(SharedMemoryTransport *transport)
{
    closeTransport(transport);
}

jint 
shmemBase_sendByte(SharedMemoryConnection *connection, jbyte data)
{
    Stream *stream = &connection->outgoing;
    SharedStream *shared = stream->shared;
    int offset;

    CHECK_ERROR(enterMutex(stream, connection->shutdown));
    CHECK_ERROR(waitForSpace(connection, stream));
    SHMEM_ASSERT(!FULL(stream));
    offset = shared->writeOffset;
    shared->buffer[offset] = data;
    shared->writeOffset = ADD_OFFSET(offset, 1);
    shared->isFull = (shared->readOffset == shared->writeOffset);

    STREAM_INVARIANT(stream);
    CHECK_ERROR(leaveMutex(stream));

    CHECK_ERROR(signalData(stream));

    return SYS_OK;
}

jint 
shmemBase_receiveByte(SharedMemoryConnection *connection, jbyte *data)
{
    Stream *stream = &connection->incoming;
    SharedStream *shared = stream->shared;
    int offset;

    CHECK_ERROR(enterMutex(stream, connection->shutdown));
    CHECK_ERROR(waitForData(connection, stream));
    SHMEM_ASSERT(!EMPTY(stream));
    offset = shared->readOffset;
    *data = shared->buffer[offset];
    shared->readOffset = ADD_OFFSET(offset, 1);
    shared->isFull = JNI_FALSE;

    STREAM_INVARIANT(stream);
    CHECK_ERROR(leaveMutex(stream));

    CHECK_ERROR(signalSpace(stream));

    return SYS_OK;
}

static jint
sendBytes(SharedMemoryConnection *connection, void *bytes, jint length)
{
    Stream *stream = &connection->outgoing;
    SharedStream *shared = stream->shared;
    jint fragmentStart;
    jint fragmentLength;
    jint index = 0;
    jint maxLength;

    CHECK_ERROR(enterMutex(stream, connection->shutdown));
    while (index < length) {
        CHECK_ERROR(waitForSpace(connection, stream));
        SHMEM_ASSERT(!FULL(stream));

        fragmentStart = shared->writeOffset;

        if (fragmentStart < shared->readOffset) {
            maxLength = shared->readOffset - fragmentStart;
        } else {
            maxLength = SHARED_BUFFER_SIZE - fragmentStart;
        }
        fragmentLength = MIN(maxLength, length - index);
        memcpy(shared->buffer + fragmentStart, (jbyte *)bytes + index, fragmentLength);
        shared->writeOffset = ADD_OFFSET(fragmentStart, fragmentLength);
        index += fragmentLength;

        shared->isFull = (shared->readOffset == shared->writeOffset);

        STREAM_INVARIANT(stream);
        CHECK_ERROR(signalData(stream));

    }
    CHECK_ERROR(leaveMutex(stream));

    return SYS_OK;
}


/*
 * Send packet header followed by data.
 */
jint 
shmemBase_sendPacket(SharedMemoryConnection *connection, struct Packet *packet)
{
    jint length = 0;
    struct PacketData *data;

    CHECK_ERROR(sendBytes(connection, &packet->type.cmd.id, sizeof(jint)));
    CHECK_ERROR(sendBytes(connection, &packet->type.cmd.flags, sizeof(jbyte)));

    if (packet->type.cmd.flags & FLAGS_Reply) {
        CHECK_ERROR(sendBytes(connection, &packet->type.reply.errorCode, sizeof(jshort)));
    } else {
        CHECK_ERROR(sendBytes(connection, &packet->type.cmd.cmdSet, sizeof(jbyte)));
        CHECK_ERROR(sendBytes(connection, &packet->type.cmd.cmd, sizeof(jbyte)));
    }

    data = &(packet->type.cmd.data);
    do {
        length += data->length;
        data = data->next;
    } while (data != NULL);


    CHECK_ERROR(sendBytes(connection, &length, sizeof(jint)));

    data = &(packet->type.cmd.data);
    do {
        CHECK_ERROR(sendBytes(connection, data->data, data->length));
        data = data->next;
    } while (data != NULL);

    return SYS_OK;
}

static jint
receiveBytes(SharedMemoryConnection *connection, void *bytes, jint length)
{
    Stream *stream = &connection->incoming;
    SharedStream *shared = stream->shared;
    jint fragmentStart;
    jint fragmentLength;
    jint index = 0;
    jint maxLength;

    CHECK_ERROR(enterMutex(stream, connection->shutdown));
    while (index < length) {
        CHECK_ERROR(waitForData(connection, stream));
        SHMEM_ASSERT(!EMPTY(stream));

        fragmentStart = shared->readOffset;
        if (fragmentStart < shared->writeOffset) {
            maxLength = shared->writeOffset - fragmentStart;
        } else {
            maxLength = SHARED_BUFFER_SIZE - fragmentStart;
        }
        fragmentLength = MIN(maxLength, length - index);
        memcpy((jbyte *)bytes + index, shared->buffer + fragmentStart, fragmentLength);
        shared->readOffset = ADD_OFFSET(fragmentStart, fragmentLength);
        index += fragmentLength;

        shared->isFull = JNI_FALSE;

        STREAM_INVARIANT(stream);
        CHECK_ERROR(signalSpace(stream));
    }
    CHECK_ERROR(leaveMutex(stream));

    return SYS_OK;
}

/*
 * Read packet header and insert into packet structure.
 * Allocate space for the data and fill it in.
 */
jint 
shmemBase_receivePacket(SharedMemoryConnection *connection, struct Packet *packet)
{
    jint length;
    jint error;

    CHECK_ERROR(receiveBytes(connection, &packet->type.cmd.id, sizeof(jint)));
    CHECK_ERROR(receiveBytes(connection, &packet->type.cmd.flags, sizeof(jbyte)));

    if (packet->type.cmd.flags & FLAGS_Reply) {
        CHECK_ERROR(receiveBytes(connection, &packet->type.reply.errorCode, sizeof(jshort)));
    } else {
        CHECK_ERROR(receiveBytes(connection, &packet->type.cmd.cmdSet, sizeof(jbyte)));
        CHECK_ERROR(receiveBytes(connection, &packet->type.cmd.cmd, sizeof(jbyte)));
    }

    CHECK_ERROR(receiveBytes(connection, &length, sizeof(jint)));

    if (length < 0) {
        return SYS_ERR;
    } else if (length == 0) {
        packet->type.cmd.data.length = 0;
        packet->type.cmd.data.data = NULL;
        packet->type.cmd.data.next = NULL;
    } else {
        packet->type.cmd.data.length = length;
        packet->type.cmd.data.next = NULL;
        packet->type.cmd.data.data= (*callback->alloc)(length);
        if (packet->type.cmd.data.data == NULL) {
            return SYS_ERR;
        }

        error = receiveBytes(connection, packet->type.cmd.data.data, length);
        if (error != SYS_OK) {
            (*callback->free)(packet->type.cmd.data.data);
            return error;
        }
    }

    return SYS_OK;
}

jint 
shmemBase_name(struct SharedMemoryTransport *transport, char **name)
{
    *name = transport->name;
    return SYS_OK;
}


void 
exitTransportWithError(char *message, char *fileName, 
                       char *date, int lineNumber)
{
    JNIEnv *env;
    jint error;
    char buffer[500];

    sprintf(buffer, "Shared Memory Transport \"%s\" (%s), line %d: %s\n",
            fileName, date, lineNumber, message); 
    error = (*jvm)->GetEnv(jvm, (void **)&env, JNI_VERSION_1_2);
    if (error != JNI_OK) {
        /*
         * We're forced into a direct call to exit()
         */
        fprintf(stderr, "%s", buffer);
        exit(-1);
    } else {
        (*env)->FatalError(env, buffer);
    }
}




/*
 * @(#)transport.c	1.28 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
#include <string.h>

#include "transport.h"
#include "util.h"
#include "debugLoop.h"
#include "debugInit.h"
#include "sys.h"
#include "JDWP.h"

static struct Transport *transport;
static const char *handshakeString = "JDWP-Handshake";
static JVMDI_RawMonitor listenerLock;
static JVMDI_RawMonitor sendLock;

/*                  
 * data structure used for passing transport info from thread to thread
 */
typedef struct TransportInfo {
    char *name;
    struct Transport *transport;
    char *address;
} TransportInfo;

#ifdef JDWP_ALLOC_TRACE
static struct TransportCallback callback = {jdwpAllocReal, jdwpFreeReal};
#else
static struct TransportCallback callback = {jdwpAlloc, jdwpFree};
#endif

/*
 * loadTransport() is adapted from loadJVMHelperLib() in 
 * JDK 1.2 javai.c v1.61
 */
static jint
loadTransport(char *name, struct Transport **transportPtr)
{
    JDWP_OnLoad_t JDWP_OnLoad;
    const char *onLoadSymbols[] = JDWP_ONLOAD_SYMBOLS;
    char fn2[MAXPATHLEN];
    char errmsg[MAXPATHLEN + 100];
    void *handle;
    int i;

    /* First look along the path used by the native dlopen/LoadLibrary
     *  functions.
     */
    dbgsysBuildLibName(fn2, sizeof(fn2), "", name);
    handle = dbgsysLoadLibrary(fn2, errmsg, sizeof(errmsg));
    if (handle == NULL) {
        /* dlopen (unix) / LoadLibrary (windows) didn't find the lib using their
         * search algorithms.  Now look for it in our own shared lib dir in case
         * dbgsysLoadLibrary didn't look there (this happens on windows when
         * .../jdkxxx/bin/java.exe as opposed to .../jdkxxx/jre/bin/java.exe
         * is executed).  In all other cases, our own shared lib dir will have been 
         * searched in the first dbgsysLoadLibrary call so this one will be a NOP.
         * This mimics the action of System.loadLibrary which first searches
         * java.library.path and then searches sun.boot.library.path.
         */
        char *shlibDir = getPropertyCString("sun.boot.library.path");
        if (shlibDir == NULL) {
            return JDWP_ERROR(TRANSPORT_LOAD);
        }
          
        dbgsysBuildLibName(fn2, sizeof(fn2), shlibDir, name);
        jdwpFree(shlibDir);
        handle = dbgsysLoadLibrary(fn2, errmsg, sizeof(errmsg));
        if (handle == NULL) {
            return JDWP_ERROR(TRANSPORT_LOAD);
        } 
    }

    for (i = 0; i < sizeof(onLoadSymbols) / sizeof(char *); i++) {
        JDWP_OnLoad = (JDWP_OnLoad_t)dbgsysFindLibraryEntry(handle, onLoadSymbols[i]);
        if (JDWP_OnLoad) {
            break;
        }
    }
    if (JDWP_OnLoad) {
        JNIEnv *env = getEnv();
        jint res;
        JavaVM *jvm;
        (*env)->GetJavaVM(env, &jvm);
        res = (*JDWP_OnLoad)(jvm, transportPtr, &callback, NULL);
        if (res < 0) {
            return JDWP_ERROR(TRANSPORT_INIT);
        }
        return JDWP_ERROR(NONE);
    } else {
        return JDWP_ERROR(TRANSPORT_LOAD);
    }
}

static jint 
handshake(Transport *transport)
{
    jbyte b;
    int i,len;

    len = strlen(handshakeString);

    for (i=0; i<len; i++) {
        jint rc = (*transport->receiveByte)(&b);
        if (rc != JDWP_ERROR(NONE))
            return rc;

        if (b != (jbyte)handshakeString[i])
            return JDWP_ERROR(TRANSPORT_INIT);
    }

    for (i=0; i<len; i++) {
        jint rc = (*transport->sendByte)((jbyte)handshakeString[i]);
        if (rc != JDWP_ERROR(NONE)) {
            return JDWP_ERROR(TRANSPORT_INIT);
        }
    }

    return JDWP_ERROR(NONE);
}

static void 
connectionInitiated(struct Transport *t)
{
    jint isValid = JNI_FALSE;
    jint error = JDWP_ERROR(NONE);

    /*
     * Don't allow a connection until initialization is complete
     */
    debugInit_waitInitComplete();

    debugMonitorEnter(listenerLock);

    /* Are we the first transport to get a connection? */

    if (transport == NULL) {
        error = handshake(t);
        if (error == JDWP_ERROR(NONE)) {
            transport = t;
            isValid = JNI_TRUE;
            debugMonitorNotifyAll(listenerLock);
            /*
             * TO DO: It would be nice to shut down the other listener threads.
             */
        } 
    } else {
        /*
         * TO DO: Fix when multple transports supported
         */
        JDI_ASSERT(JNI_FALSE);
    }


    debugMonitorExit(listenerLock);

    if (!isValid) {
        t->close();
        return;
    }

    debugLoop_run();
}

static void
acceptThread(void *ptr) 
{
    TransportInfo *info = ptr;
    struct Transport *transport = info->transport;
    jint error = (*transport->accept)();
    if (error != JDWP_ERROR(NONE)) {
        /*
         * TO DO: There's no caller to communicate an error to, but 
         * we shouldn't crash the VM for this either? For now, 
         * just print an error message to the console.
         */
         fprintf(stderr, "%s transport error; accept failed, rc = %d\n",
                 info->name, error);
    }

    (*transport->stopListening)();

    connectionInitiated(transport);
}

static void
attachThread(void *ptr) 
{
    connectionInitiated(ptr);
}



void 
transport_initialize()
{
    transport = NULL;
    listenerLock = debugMonitorCreate("JDWP Transport Listener Monitor");
    sendLock = debugMonitorCreate("JDWP Transport Send Monitor");
}

void 
transport_reset()
{
    transport = NULL;
}

static jint
launch(char *command, char *name, char *address) 
{
    jint error;
    char *commandLine = jdwpAlloc(strlen(command) +
                                 strlen(name) +
                                 strlen(address) + 3);
    if (commandLine == NULL) {
        return JVMDI_ERROR_OUT_OF_MEMORY;
    }
    strcpy(commandLine, command);
    strcat(commandLine, " ");
    strcat(commandLine, name);
    strcat(commandLine, " ");
    strcat(commandLine, address);

    error = dbgsysExec(commandLine);
    jdwpFree(commandLine);
    if (error != SYS_OK) {
        return JDWP_ERROR(TRANSPORT_INIT);
    } else {
        return JDWP_ERROR(NONE);
    }
}

jint 
transport_startTransport(jboolean isServer, char *name, char *address)
{
    struct Transport *transport;
    char threadName[MAXPATHLEN + 100];
    jint err = loadTransport(name, &transport);
    if (err != JDWP_ERROR(NONE)) {
        return err;
    }

    if (isServer) {
        char *retAddress;
        char *launchCommand;
        TransportInfo *info = jdwpAlloc(sizeof(*info));
        if (info == NULL) {
            return JVMDI_ERROR_OUT_OF_MEMORY;
        }
        info->name = jdwpStrdup(name);
        info->address = NULL; 
        if (info->name == NULL) {
            err = JVMDI_ERROR_OUT_OF_MEMORY;
            goto handleError;
        }
        if (address != NULL) {
            info->address = jdwpStrdup(address);
            if (info->address == NULL) {
                err = JVMDI_ERROR_OUT_OF_MEMORY;
                goto handleError;
            }
        }

        info->transport = transport;

        retAddress = address;
        err = (*transport->listen)(&retAddress);
        if (err != JDWP_ERROR(NONE)) {
           goto handleError;
        }

        strcpy(threadName, "JDWP Transport Listener: ");
        strcat(threadName, name);
        err = spawnNewThread(acceptThread, info, threadName);
        if (err != JDWP_ERROR(NONE)) {
            goto handleError;
        }

        launchCommand = debugInit_launchOnInit();
        if (launchCommand != NULL) {
            err = launch(launchCommand, name, retAddress);
            if (err != JDWP_ERROR(NONE)) {
                goto handleError;
            }
        }

        if ((address == NULL) || (strcmp(address, retAddress) != 0)) {
            if (launchCommand == NULL) {
                fprintf(stdout, "Listening for transport %s at address: %s\n",
                        name, retAddress);
                fflush(stdout);
            }
            jdwpFree(retAddress);
        }
        return JDWP_ERROR(NONE);
        
handleError:
        jdwpFree(info->name);
        jdwpFree(info->address);
        jdwpFree(info);
        return err;
    } else {
        /*
         * Note that we don't attempt to do a launch here. Launching
         * is currently supported only in server mode.
         */

        /*
         * If we're connecting to another process, there shouldn't be
         * any concurrent listens, so its ok if we block here in this
         * thread, waiting for the attach to finish.
         */
         err = (*transport->attach)(address);
         if (err != JDWP_ERROR(NONE)) {
            return err;
         }

         /*
          * Start the transport loop in a separate thread
          */
         strcpy(threadName, "JDWP Transport Listener: ");
         strcat(threadName, name);
         return spawnNewThread(attachThread, transport, threadName);
    }
}

void 
transport_close() 
{
    (*transport->close)();
}

jint 
transport_sendPacket(Packet *packet)
{
    jint retVal = JDWP_ERROR(NONE);

    /*
     * If the VM is suspended on debugger initialization, we wait 
     * for a connection before continuing. This ensures that all
     * events are delivered to the debugger. (We might as well do this
     * this since the VM won't continue until a remote debugger attaches
     * and resumes it.) If not suspending on initialization, we must
     * just drop any packets (i.e. events) so that the VM can continue
     * to run. The debugger may not attach until much later.
     */
    if ((transport == NULL) && debugInit_suspendOnInit()) {
        debugMonitorEnter(listenerLock);
        while (transport == NULL) {
            debugMonitorWait(listenerLock);
        }
        debugMonitorExit(listenerLock);
    }

    if (transport != NULL) {
        debugMonitorEnter(sendLock);
        retVal = (*transport->sendPacket)(packet);
        debugMonitorExit(sendLock);
    } /* else, bit bucket */

    return retVal;
}

jint 
transport_receivePacket(Packet *packet) {
    return (*transport->receivePacket)(packet);
}

void 
transport_lock() 
{
    debugMonitorEnter(listenerLock);
    debugMonitorEnter(sendLock);
}

void 
transport_unlock() 
{
    debugMonitorExit(sendLock);
    debugMonitorExit(listenerLock);
}

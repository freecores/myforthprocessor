/*
 * @(#)socket_md.c	1.8 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/* Adapted from JDK1.2 socket_md.c        1.9 98/06/30 */

/*
 * sysBind, sysSetSockOpt added with implementations based on 
 * PlainSocketImpl v1.18 (see below)
 */

#include <windef.h>
#include <winsock2.h>

#include "sysSocket.h"
#include "socketTransport.h"

#ifdef DEBUG
#define sysAssert(expression) {		\
    if (!(expression)) {		\
	    exitTransportWithError \
            ("\"%s\", line %d: assertion failure\n", \
             __FILE__, __DATE__, __LINE__); \
    }					\
}
#else
#define sysAssert(expression) ((void) 0)
#endif

typedef jboolean bool_t;

#include "mutex_md.h"

struct sockaddr;
struct hostent;

#define FN_RECV          0
#define FN_SEND          1
#define FN_LISTEN        2
#define FN_ACCEPT        3
#define FN_RECVFROM      4
#define FN_SENDTO        5
#define FN_SELECT        6
#define FN_CONNECT       7
#define FN_CLOSESOCKET   8
#define FN_SHUTDOWN      9
#define FN_DOSHUTDOWN    10
#define FN_GETHOSTBYNAME 11
#define FN_HTONS         12
#define FN_SOCKET        13
#define FN_WSASENDDISCONNECT 14
#define FN_SOCKETAVAILABLE 15

#define FN_BIND           16         
#define FN_SETSOCKETOPTION 17         
#define FN_GETPROTOBYNAME 18      
#define FN_NTOHS          19
#define FN_HTONL          20
#define FN_GETSOCKNAME    21
#define FN_NTOHL          22

static int (PASCAL FAR *sockfnptrs[])() = 
    {NULL, NULL, NULL, NULL, NULL,
     NULL, NULL, NULL, NULL, NULL,
     NULL, NULL, NULL, NULL, NULL,
     NULL, NULL, NULL, NULL, NULL,
     NULL, NULL, NULL };                   

static bool_t sockfnptrs_initialized = FALSE;
static mutex_t sockFnTableMutex;

/* is Winsock2 loaded? better to be explicit than to rely on sockfnptrs */
static bool_t winsock2Available = FALSE;


/* IMPORTANT: whenever possible, we want to use Winsock2 (ws2_32.dll)
 * instead of Winsock (wsock32.dll). Other than the fact that it is
 * newer, less buggy and faster than Winsock, Winsock2 lets us to work
 * around the following problem:
 *
 * Generally speaking, it is important to shutdown a socket before
 * closing it, since failing to do so can sometimes result in a TCP
 * RST (abortive close) which is disturbing to the peer of the
 * connection.
 * 
 * The Winsock way to shutdown a socket is the Berkeley call
 * shutdown(). We do not want to call it on Win95, since it
 * sporadically leads to an OS crash in IFS_MGR.VXD.  Complete hull
 * breach.  Blue screen.  Ugly.
 *
 * So, in initSockTable we look for Winsock 2, and if we find it we
 * assign wsassendisconnectfn function pointer. When we close, we
 * first check to see if it's bound, and if it is, we call it. Winsock
 * 2 will always be there on NT, and we recommend that win95 user
 * install it.
 *
 * - br 10/11/97
 */

static void
initSockFnTable() {
    int (PASCAL FAR* WSAStartupPtr)(WORD, LPWSADATA); 
    WSADATA wsadata;

    mutexInit(&sockFnTableMutex);
    mutexLock(&sockFnTableMutex);
    if (sockfnptrs_initialized == FALSE) {
        HANDLE hWinsock;

        /* try to load Winsock2, and if that fails, load Winsock */
        hWinsock = LoadLibrary("ws2_32.dll");
        if (hWinsock == NULL) {
            hWinsock = LoadLibrary("wsock32.dll");
            winsock2Available = FALSE;
        } else {
            winsock2Available = TRUE;
        }

        if (hWinsock == NULL) {
            fprintf(stderr, "Could not load Winsock 1 or 2 (error: %d)\n", 
                        GetLastError());
        }

        /* If we loaded a DLL, then we might as well initialize it.  */
        WSAStartupPtr = (int (PASCAL FAR *)(WORD, LPWSADATA))
                            GetProcAddress(hWinsock, "WSAStartup");
        if (WSAStartupPtr(MAKEWORD(1,1), &wsadata) != 0) {
            fprintf(stderr, "Could not initialize Winsock\n"); 
        }

        sockfnptrs[FN_RECV]
            = (int (PASCAL FAR *)())GetProcAddress(hWinsock, "recv");
        sockfnptrs[FN_SEND]
            = (int (PASCAL FAR *)())GetProcAddress(hWinsock, "send");
        sockfnptrs[FN_LISTEN]
            = (int (PASCAL FAR *)())GetProcAddress(hWinsock, "listen");
        sockfnptrs[FN_ACCEPT]
            = (int (PASCAL FAR *)())GetProcAddress(hWinsock, "accept");
        sockfnptrs[FN_RECVFROM]
            = (int (PASCAL FAR *)())GetProcAddress(hWinsock, "recvfrom");
        sockfnptrs[FN_SENDTO]
            = (int (PASCAL FAR *)())GetProcAddress(hWinsock, "sendto");
        sockfnptrs[FN_SELECT]
            = (int (PASCAL FAR *)())GetProcAddress(hWinsock, "select");
        sockfnptrs[FN_CONNECT]
            = (int (PASCAL FAR *)())GetProcAddress(hWinsock, "connect");
        sockfnptrs[FN_CLOSESOCKET]
            = (int (PASCAL FAR *)())GetProcAddress(hWinsock, "closesocket");
        /* we don't use this */
        sockfnptrs[FN_SHUTDOWN]
            = (int (PASCAL FAR *)())GetProcAddress(hWinsock, "shutdown");
        sockfnptrs[FN_GETHOSTBYNAME]
            = (int (PASCAL FAR *)())GetProcAddress(hWinsock, "gethostbyname");
        sockfnptrs[FN_HTONS]
            = (int (PASCAL FAR *)())GetProcAddress(hWinsock, "htons");
        sockfnptrs[FN_SOCKET]
            = (int (PASCAL FAR *)())GetProcAddress(hWinsock, "socket");
        /* in winsock 1, this will simply be 0 */
        sockfnptrs[FN_WSASENDDISCONNECT]
            = (int (PASCAL FAR *)())GetProcAddress(hWinsock,
                                                   "WSASendDisconnect");
        sockfnptrs[FN_SOCKETAVAILABLE]
            = (int (PASCAL FAR *)())GetProcAddress(hWinsock,
                                                   "ioctlsocket");
        sockfnptrs[FN_BIND]
            = (int (PASCAL FAR *)())GetProcAddress(hWinsock, "bind");
        sockfnptrs[FN_SETSOCKETOPTION]
            = (int (PASCAL FAR *)())GetProcAddress(hWinsock, "setsockopt");
        sockfnptrs[FN_GETPROTOBYNAME]
            = (int (PASCAL FAR *)())GetProcAddress(hWinsock, 
                                                   "getprotobyname");
        sockfnptrs[FN_NTOHS]
            = (int (PASCAL FAR *)())GetProcAddress(hWinsock, "ntohs");
        sockfnptrs[FN_HTONL]
            = (int (PASCAL FAR *)())GetProcAddress(hWinsock, "htonl");
        sockfnptrs[FN_GETSOCKNAME]
            = (int (PASCAL FAR *)())GetProcAddress(hWinsock, "getsockname");
        sockfnptrs[FN_NTOHL]
            = (int (PASCAL FAR *)())GetProcAddress(hWinsock, "ntohl");
    }            

    sysAssert(sockfnptrs[FN_RECV] != NULL);
    sysAssert(sockfnptrs[FN_SEND] != NULL);
    sysAssert(sockfnptrs[FN_LISTEN] != NULL);
    sysAssert(sockfnptrs[FN_ACCEPT] != NULL);
    sysAssert(sockfnptrs[FN_RECVFROM] != NULL);
    sysAssert(sockfnptrs[FN_SENDTO] != NULL);
    sysAssert(sockfnptrs[FN_SELECT] != NULL);
    sysAssert(sockfnptrs[FN_CONNECT] != NULL);
    sysAssert(sockfnptrs[FN_CLOSESOCKET] != NULL);
    sysAssert(sockfnptrs[FN_SHUTDOWN] != NULL);
    sysAssert(sockfnptrs[FN_GETHOSTBYNAME] != NULL);
    sysAssert(sockfnptrs[FN_HTONS] != NULL);
    sysAssert(sockfnptrs[FN_SOCKET] != NULL);
    
    if (winsock2Available) {
        sysAssert(sockfnptrs[FN_WSASENDDISCONNECT] != NULL);
    }

    sysAssert(sockfnptrs[FN_SOCKETAVAILABLE] != NULL);
    sysAssert(sockfnptrs[FN_BIND] != NULL);
    sysAssert(sockfnptrs[FN_SETSOCKETOPTION] != NULL);
    sysAssert(sockfnptrs[FN_GETPROTOBYNAME] != NULL);
    sysAssert(sockfnptrs[FN_NTOHS] != NULL);
    sysAssert(sockfnptrs[FN_HTONL] != NULL);
    sysAssert(sockfnptrs[FN_GETSOCKNAME] != NULL);
    sysAssert(sockfnptrs[FN_NTOHL] != NULL);

    sockfnptrs_initialized = TRUE;
    mutexUnlock(&sockFnTableMutex);
}

/*
 * If we get a nonnull function pointer it might still be the case
 * that some other thread is in the process of initializing the socket
 * function pointer table, but our pointer should still be good.
 */
int
dbgsysListen(int fd, long count) {
    int (PASCAL FAR *listenfn)();
    if ((listenfn = sockfnptrs[FN_LISTEN]) == NULL) {
        initSockFnTable();
        listenfn = sockfnptrs[FN_LISTEN];
    }
    sysAssert(sockfnptrs_initialized == TRUE && listenfn != NULL);
    return (*listenfn)(fd, count);
}

int
dbgsysConnect(int fd, struct sockaddr *name, int namelen) {
    int (PASCAL FAR *connectfn)();
    if ((connectfn = sockfnptrs[FN_CONNECT]) == NULL) {
        initSockFnTable();
        connectfn = sockfnptrs[FN_CONNECT];
    }
    sysAssert(sockfnptrs_initialized == TRUE);
    sysAssert(connectfn != NULL);
    return (*connectfn)(fd, name, namelen);
}

int
dbgsysAccept(int fd, struct sockaddr *name, int *namelen) {
    int (PASCAL FAR *acceptfn)();
    if ((acceptfn = sockfnptrs[FN_ACCEPT]) == NULL) {
        initSockFnTable();
        acceptfn = sockfnptrs[FN_ACCEPT];
    }
    sysAssert(sockfnptrs_initialized == TRUE && acceptfn != NULL);
    return (*acceptfn)(fd, name, namelen);
}

int
dbgsysRecvFrom(int fd, char *buf, int nBytes,
                  int flags, struct sockaddr *from, int *fromlen) {
    int (PASCAL FAR *recvfromfn)();
    if ((recvfromfn = sockfnptrs[FN_RECVFROM]) == NULL) {
        initSockFnTable();
        recvfromfn = sockfnptrs[FN_RECVFROM];
    }
    sysAssert(sockfnptrs_initialized == TRUE && recvfromfn != NULL);
    return (*recvfromfn)(fd, buf, nBytes, flags, from, fromlen);
}

int
dbgsysSendTo(int fd, char *buf, int len,
                int flags, struct sockaddr *to, int tolen) {
    int (PASCAL FAR *sendtofn)();
    if ((sendtofn = sockfnptrs[FN_SENDTO]) == NULL) {
        initSockFnTable();
        sendtofn = sockfnptrs[FN_SENDTO];
    }
    sysAssert(sockfnptrs_initialized == TRUE && sendtofn != NULL);
    return (*sendtofn)(fd, buf, len, flags, to, tolen);
}

int
dbgsysRecv(int fd, char *buf, int nBytes, int flags) {
    int (PASCAL FAR *recvfn)();
    if ((recvfn = sockfnptrs[FN_RECV]) == NULL) {
        initSockFnTable();
        recvfn = sockfnptrs[FN_RECV];
    }
    sysAssert(sockfnptrs_initialized == TRUE && recvfn != NULL);
    return (*recvfn)(fd, buf, nBytes, flags);
}

int
dbgsysSend(int fd, char *buf, int nBytes, int flags) {
    int (PASCAL FAR *sendfn)();
    if ((sendfn = sockfnptrs[FN_SEND]) == NULL) {
        initSockFnTable();
        sendfn = sockfnptrs[FN_SEND];
    }
    sysAssert(sockfnptrs_initialized == TRUE && sendfn != NULL);
    return (*sendfn)(fd, buf, nBytes, flags);
}

struct hostent *
dbgsysGetHostByName(char *hostname) {
    int (PASCAL FAR *sendfn)();
    if ((sendfn = sockfnptrs[FN_GETHOSTBYNAME]) == NULL) {
        initSockFnTable();
        sendfn = sockfnptrs[FN_GETHOSTBYNAME];
    }
    sysAssert(sockfnptrs_initialized == TRUE && sendfn != NULL);
    return (struct hostent *)
        (*(struct hostent *(PASCAL FAR *)(char *))sendfn)(hostname);
}

unsigned short
dbgsysHostToNetworkShort(unsigned short hostshort) {
    int (PASCAL FAR *sendfn)();
    if ((sendfn = sockfnptrs[FN_HTONS]) == NULL) {
        initSockFnTable();
        sendfn = sockfnptrs[FN_HTONS];
    }
    sysAssert(sockfnptrs_initialized == TRUE && sendfn != NULL);
    return (*sendfn)((int)hostshort);
}

int
dbgsysSocket(int domain, int type, int protocol) {
    int (PASCAL FAR *socketfn)();
    if ((socketfn = sockfnptrs[FN_SOCKET]) == NULL) {
        initSockFnTable();
        socketfn = sockfnptrs[FN_SOCKET];
    }
    sysAssert(sockfnptrs_initialized == TRUE && socketfn != NULL);
    return (*socketfn)(domain, type, protocol);
}

/*
 * This function is carefully designed to work around a bug in Windows
 * 95's networking winsock. Please see the beginning of this file for
 * a complete description of the problem.
 */
int dbgsysSocketClose(int fd) {

    if (fd > 0) {
        int (PASCAL FAR *closesocketfn)();
        int (PASCAL FAR *wsasenddisconnectfn)();
        int dynamic_ref = -1;

        if ((closesocketfn = sockfnptrs[FN_CLOSESOCKET]) == NULL) {
            initSockFnTable();
        }
        /* At this point we are guaranteed the sockfnptrs are initialized */
            sysAssert(sockfnptrs_initialized == TRUE);

        closesocketfn = sockfnptrs[FN_CLOSESOCKET];
        sysAssert(closesocketfn != NULL);

        if (winsock2Available) {
            wsasenddisconnectfn = sockfnptrs[FN_WSASENDDISCONNECT];
            (void) (*wsasenddisconnectfn)(fd, NULL);
        }
        (void) (*closesocketfn)(fd);
    }
    return TRUE;
}

/*
 * Poll the fd for reading for timeout ms.  Returns 1 if something's
 * ready, 0 if it timed out, -1 on error, -2 if interrupted (although
 * interruption isn't implemented yet).  Timeout in milliseconds.  */
int
dbgsysTimeout(int fd, long timeout) {
    int res;
    fd_set tbl;
    struct timeval t;
    int (PASCAL FAR *selectfn)();

    t.tv_sec = timeout / 1000;
    t.tv_usec = (timeout % 1000) * 1000;
    FD_ZERO(&tbl);
    FD_SET((unsigned int)fd, &tbl);

    if ((selectfn = sockfnptrs[FN_SELECT]) == NULL) {
        initSockFnTable();
        selectfn = sockfnptrs[FN_SELECT];
    }
    sysAssert(sockfnptrs_initialized == TRUE && selectfn != NULL); 
    res = (*selectfn)(fd + 1, &tbl, 0, 0, &t);
    return res;
}

long
dbgsysSocketAvailable(int fd, long *pbytes)
{
    int (PASCAL FAR *socketfn)();
    if ((socketfn = sockfnptrs[FN_SOCKETAVAILABLE]) == NULL) {
        initSockFnTable();
        socketfn = sockfnptrs[FN_SOCKETAVAILABLE];
    }
    sysAssert(sockfnptrs_initialized == TRUE && socketfn != NULL);
    return (*socketfn)(fd, FIONREAD, pbytes);
}

/* Additions to original follow */

int
dbgsysBind(int fd, struct sockaddr *name, int namelen) {
    int (PASCAL FAR *socketfn)();
    if ((socketfn = sockfnptrs[FN_BIND]) == NULL) {
        initSockFnTable();
        socketfn = sockfnptrs[FN_BIND];
    }

    sysAssert(sockfnptrs_initialized == TRUE && socketfn != NULL);
    return (*socketfn)(fd, name, namelen);
}

unsigned long
dbgsysHostToNetworkLong(unsigned long hostlong) {
    int (PASCAL FAR *socketfn)();
    if ((socketfn = sockfnptrs[FN_HTONL]) == NULL) {
        initSockFnTable();
        socketfn = sockfnptrs[FN_HTONL];
    }

    sysAssert(sockfnptrs_initialized == TRUE && socketfn != NULL);
    return (*socketfn)(hostlong);
}

unsigned short
dbgsysNetworkToHostShort(unsigned short netshort) {
    int (PASCAL FAR *socketfn)();
    if ((socketfn = sockfnptrs[FN_NTOHS]) == NULL) {
        initSockFnTable();
        socketfn = sockfnptrs[FN_NTOHS];
    }

    sysAssert(sockfnptrs_initialized == TRUE && socketfn != NULL);
    return (*socketfn)(netshort);
}

int
dbgsysGetSocketName(int fd, struct sockaddr *name, int *namelen) {
    int (PASCAL FAR *socketfn)();
    if ((socketfn = sockfnptrs[FN_GETSOCKNAME]) == NULL) {
        initSockFnTable();
        socketfn = sockfnptrs[FN_GETSOCKNAME];
    }
    sysAssert(sockfnptrs_initialized == TRUE && socketfn != NULL);
    return (*socketfn)(fd, name, namelen);
}

unsigned long
dbgsysNetworkToHostLong(unsigned long netlong) {
    int (PASCAL FAR *socketfn)();
    if ((socketfn = sockfnptrs[FN_NTOHL]) == NULL) {
        initSockFnTable();
        socketfn = sockfnptrs[FN_NTOHL];
    }

    sysAssert(sockfnptrs_initialized == TRUE && socketfn != NULL);
    return (*socketfn)(netlong);
}

/*
 * Below Adapted from PlainSocketImpl.c, win32 version 1.18. Changed exception
 * throws to returns of SYS_ERR; we should improve the error codes
 * eventually. Changed java objects to values the debugger back end can
 * more easily deal with. 
 */

int
dbgsysSetSocketOption(int fd, jint cmd, jboolean on, jvalue value) 
{
    int (PASCAL FAR *setsockoptfn)();
    int (PASCAL FAR *getprotofn)();
    if ((setsockoptfn = sockfnptrs[FN_SETSOCKETOPTION]) == NULL) {
        initSockFnTable();
        setsockoptfn = sockfnptrs[FN_SETSOCKETOPTION];
    }
    getprotofn = sockfnptrs[FN_GETPROTOBYNAME];
    sysAssert(sockfnptrs_initialized == TRUE && 
              setsockoptfn != NULL && getprotofn != NULL);

    if (cmd == TCP_NODELAY) {
        struct protoent *proto = (struct protoent *)
            (*(struct protoent *(PASCAL FAR *)(char *))getprotofn)("TCP");
        int tcp_level = (proto == 0 ? IPPROTO_TCP: proto->p_proto);
        long onl = (long)on;

        if ((*setsockoptfn)(fd, tcp_level, TCP_NODELAY,
                       (char *)&onl, sizeof(long)) < 0) {
                return SYS_ERR;
        }
    } else if (cmd == SO_LINGER) {
        struct linger arg;
        arg.l_onoff = on;

        if(on) {
            arg.l_linger = (unsigned short)value.i;
            if((*setsockoptfn)(fd, SOL_SOCKET, SO_LINGER,
                          (char*)&arg, sizeof(arg)) < 0) {
                return SYS_ERR;
            }
        } else {
            if ((*setsockoptfn)(fd, SOL_SOCKET, SO_LINGER,
                           (char*)&arg, sizeof(arg)) < 0) {
                return SYS_ERR;
            }
        }
    } else if (cmd == SO_SNDBUF) {
        jint buflen = value.i;
        if ((*setsockoptfn)(fd, SOL_SOCKET, SO_SNDBUF,
                       (char *)&buflen, sizeof(buflen)) < 0) {
            return SYS_ERR;
        }
    } else if (cmd == SO_REUSEADDR) {
        int oni = (int)on;
        if ((*setsockoptfn)(fd, SOL_SOCKET, SO_REUSEADDR,
                       (char *)&oni, sizeof(oni)) < 0) {
            return SYS_ERR;

        }
    } else {
        return SYS_ERR;
    }
    return SYS_OK;
}



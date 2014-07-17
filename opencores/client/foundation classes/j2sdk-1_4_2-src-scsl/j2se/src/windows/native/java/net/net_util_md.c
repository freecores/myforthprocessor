/*
 * @(#)net_util_md.c	1.46 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <winsock2.h>
#include <ws2tcpip.h>

#include "net_util.h"
#include "jni.h"

#ifndef IPTOS_TOS_MASK
#define IPTOS_TOS_MASK 0x1e
#endif
#ifndef IPTOS_PREC_MASK
#define IPTOS_PREC_MASK 0xe0
#endif 

/* true if SO_RCVTIMEO is supported */
jboolean isRcvTimeoutSupported = JNI_TRUE;

/*
 * Table of Windows Sockets errors, the specific exception we
 * throw for the error, and the error text.
 *
 * Note that this table excludes OS dependent errors.
 *
 * Latest list of Windows Sockets errors can be found at :-
 * http://msdn.microsoft.com/library/psdk/winsock/errors_3wc2.htm
 */
static struct {
    int errCode;
    const char *exc;
    const char *errString;
} const winsock_errors[] = {
    { WSAEACCES, 		0,	"Permission denied" },
    { WSAEADDRINUSE, 		"BindException",	"Address already in use" },
    { WSAEADDRNOTAVAIL, 	"BindException",	"Cannot assign requested address" },
    { WSAEAFNOSUPPORT,		0,	"Address family not supported by protocol family" },
    { WSAEALREADY,		0, 	"Operation already in progress" },
    { WSAECONNABORTED,		0,	"Software caused connection abort" },
    { WSAECONNREFUSED,		"ConnectException",	"Connection refused" },
    { WSAECONNRESET,		0,	"Connection reset by peer" },
    { WSAEDESTADDRREQ,		0,	"Destination address required" },
    { WSAEFAULT,		0,	"Bad address" },
    { WSAEHOSTDOWN,		0,	"Host is down" },
    { WSAEHOSTUNREACH,		"NoRouteToHostException",	"No route to host" },
    { WSAEINPROGRESS,		0,	"Operation now in progress" },
    { WSAEINTR,			0,	"Interrupted function call" },
    { WSAEINVAL,		0,	"Invalid argument" },
    { WSAEISCONN,		0,	"Socket is already connected" },
    { WSAEMFILE,		0,	"Too many open files" },
    { WSAEMSGSIZE,		0,	"The message is larger than the maximum supported by the underlying transport" },
    { WSAENETDOWN,		0,	"Network is down" },
    { WSAENETRESET,		0,	"Network dropped connection on reset" },
    { WSAENETUNREACH,		0,	"Network is unreachable" },
    { WSAENOBUFS,		0,	"No buffer space available (maximum connections reached?)" },
    { WSAENOPROTOOPT,		0,	"Bad protocol option" },
    { WSAENOTCONN,		0,	"Socket is not connected" },
    { WSAENOTSOCK,		0,	"Socket operation on nonsocket" },
    { WSAEOPNOTSUPP,		0,	"Operation not supported" },
    { WSAEPFNOSUPPORT,		0,	"Protocol family not supported" },
    { WSAEPROCLIM,		0,	"Too many processes" },
    { WSAEPROTONOSUPPORT,	0,	"Protocol not supported" },
    { WSAEPROTOTYPE,		0,	"Protocol wrong type for socket" },
    { WSAESHUTDOWN,		0,	"Cannot send after socket shutdown" },
    { WSAESOCKTNOSUPPORT,	0,	"Socket type not supported" },
    { WSAETIMEDOUT,		"ConnectException",	"Connection timed out" },
    { WSATYPE_NOT_FOUND,	0,	"Class type not found" },
    { WSAEWOULDBLOCK,		0,	"Resource temporarily unavailable" },
    { WSAHOST_NOT_FOUND,	0,	"Host not found" },
    { WSA_NOT_ENOUGH_MEMORY,	0,	"Insufficient memory available" },
    { WSANOTINITIALISED,	0,	"Successful WSAStartup not yet performed" },
    { WSANO_DATA,		0,	"Valid name, no data record of requested type" },
    { WSANO_RECOVERY,		0,	"This is a nonrecoverable error" },
    { WSASYSNOTREADY,		0,	"Network subsystem is unavailable" },
    { WSATRY_AGAIN,		0,	"Nonauthoritative host not found" },
    { WSAVERNOTSUPPORTED,	0,	"Winsock.dll version out of range" },
    { WSAEDISCON,		0,	"Graceful shutdown in progress" },
    { WSA_OPERATION_ABORTED,	0,	"Overlapped operation aborted" },
};

/*
 * Initialize Windows Sockets API support
 */
BOOL WINAPI
DllMain(HINSTANCE hinst, DWORD reason, LPVOID reserved)
{
    WSADATA wsadata;

    switch (reason) {
	case DLL_PROCESS_ATTACH:
	    if (WSAStartup(MAKEWORD(2,2), &wsadata) != 0) {
		return FALSE;
	    }
	    break;

	case DLL_PROCESS_DETACH:
	    WSACleanup();
	    break;

	default:
	    break;
    }
    return TRUE;
}

/*
 * Since winsock doesn't have the equivalent of strerror(errno)
 * use table to lookup error text for the error.
 */
JNIEXPORT void JNICALL
NET_ThrowNew(JNIEnv *env, int errorNum, char *msg) 
{
    int i;
    int table_size = sizeof(winsock_errors) /
		     sizeof(winsock_errors[0]);
    char exc[256];
    char fullMsg[256];
    char *excP = NULL;

    /*
     * If exception already throw then don't overwrite it.
     */
    if ((*env)->ExceptionOccurred(env)) {
        return;
    }

    /*
     * Default message text if not provided
     */
    if (!msg) {
	msg = "no further information";
    }

    /*
     * Check table for known winsock errors
     */
    i=0;
    while (i < table_size) {
	if (errorNum == winsock_errors[i].errCode) {
	    break;
        }
	i++;
    }

    /*
     * If found get pick the specific exception and error
     * message corresponding to this error.
     */
    if (i < table_size) {
    	excP = (char *)winsock_errors[i].exc;
	jio_snprintf(fullMsg, sizeof(fullMsg), "%s: %s",
		     (char *)winsock_errors[i].errString, msg);
    } else {
	jio_snprintf(fullMsg, sizeof(fullMsg), 
		     "Unrecognized Windows Sockets error: %d: %s", 
		     errorNum, msg);

    }

    /*
     * Throw SocketException if no specific exception for this
     * error.
     */
    if (excP == NULL) {
	excP = "SocketException";
    }
    sprintf(exc, "%s%s", JNU_JAVANETPKG, excP);
    JNU_ThrowByName(env, exc, fullMsg);
}

void 
NET_ThrowCurrent(JNIEnv *env, char *msg) 
{
    NET_ThrowNew(env, WSAGetLastError(), msg);
}

void
NET_ThrowSocketException(JNIEnv *env, char* msg) 
{
    static jclass cls = NULL;
    if (cls == NULL) {
	cls = (*env)->FindClass(env, "java/net/SocketException");
	CHECK_NULL(cls);
	cls = (*env)->NewGlobalRef(env, cls);
	CHECK_NULL(cls);
    }
    (*env)->ThrowNew(env, cls, msg);
}
	

jfieldID
NET_GetFileDescriptorID(JNIEnv *env)
{
    jclass cls = (*env)->FindClass(env, "java/io/FileDescriptor");
    CHECK_NULL_RETURN(cls, NULL);
    return (*env)->GetFieldID(env, cls, "fd", "I");
}

jint  IPv6_supported()
{
    return JNI_FALSE;
}


/*
 * Return the default TOS value
 */
int NET_GetDefaultTOS() {
    static int default_tos = -1;
    OSVERSIONINFO ver;
    HKEY hKey;
    LONG ret;

    /*
     * If default ToS already determined then return it
     */
    if (default_tos >= 0) {
	return default_tos;
    }

    /*
     * Assume default is "normal service"
     */	
    default_tos = 0;

    /* 
     * Which OS is this?
     */
    ver.dwOSVersionInfoSize = sizeof(ver);
    GetVersionEx(&ver);

    /*
     * If 2000 or greater then no default ToS in registry
     */
    if (ver.dwPlatformId == VER_PLATFORM_WIN32_NT) {
	if (ver.dwMajorVersion >= 5) {
	    return default_tos;
	}
    }

    /*
     * Query the registry to see if a Default ToS has been set.
     * Different registry entry for NT vs 95/98/ME.
     */
    if (ver.dwPlatformId == VER_PLATFORM_WIN32_NT) {
	ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
		           "SYSTEM\\CurrentControlSet\\Services\\Tcp\\Parameters",
		           0, KEY_READ, (PHKEY)&hKey);
    } else {
	ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
		           "SYSTEM\\CurrentControlSet\\Services\\VxD\\MSTCP\\Parameters", 
		           0, KEY_READ, (PHKEY)&hKey);
    }
    if (ret == ERROR_SUCCESS) {	
	DWORD dwLen;
	DWORD dwDefaultTOS;
	ULONG ulType;
	dwLen = sizeof(dwDefaultTOS);

	ret = RegQueryValueEx(hKey, "DefaultTOS",  NULL, &ulType,
			     (LPBYTE)&dwDefaultTOS, &dwLen);
	RegCloseKey(hKey);
	if (ret == ERROR_SUCCESS) {
	    default_tos = (int)dwDefaultTOS;
        }
    }
    return default_tos;
}



/*
 * Map the Java level socket option to the platform specific
 * level and option name. 
 */
JNIEXPORT int JNICALL
NET_MapSocketOption(jint cmd, int *level, int *optname) {
    
    typedef struct {
	jint cmd;
	int level;
	int optname;
    } sockopts;

    static sockopts opts[] = {
	{ java_net_SocketOptions_TCP_NODELAY,	IPPROTO_TCP, 	TCP_NODELAY },
	{ java_net_SocketOptions_SO_OOBINLINE,	SOL_SOCKET,	SO_OOBINLINE },
	{ java_net_SocketOptions_SO_LINGER,	SOL_SOCKET,	SO_LINGER },
	{ java_net_SocketOptions_SO_SNDBUF,	SOL_SOCKET,	SO_SNDBUF },
	{ java_net_SocketOptions_SO_RCVBUF,	SOL_SOCKET,	SO_RCVBUF },
	{ java_net_SocketOptions_SO_KEEPALIVE,	SOL_SOCKET,	SO_KEEPALIVE },
	{ java_net_SocketOptions_SO_REUSEADDR,	SOL_SOCKET,	SO_REUSEADDR },
	{ java_net_SocketOptions_SO_BROADCAST,	SOL_SOCKET,	SO_BROADCAST },
	{ java_net_SocketOptions_IP_MULTICAST_IF,   IPPROTO_IP,	IP_MULTICAST_IF },
	{ java_net_SocketOptions_IP_MULTICAST_LOOP, IPPROTO_IP, IP_MULTICAST_LOOP },
	{ java_net_SocketOptions_IP_TOS,	    IPPROTO_IP,	IP_TOS },

    };


    int i;

    /*
     * Map the Java level option to the native level 
     */
    for (i=0; i<(int)(sizeof(opts) / sizeof(opts[0])); i++) {
	if (cmd == opts[i].cmd) {
	    *level = opts[i].level;
	    *optname = opts[i].optname;
	    return 0;
	}
    }

    /* not found */
    return -1;
}


/*
 * Wrapper for setsockopt dealing with Windows specific issues :-
 *
 * IP_TOS and IP_MULTICAST_LOOP can't be set on some Windows
 * editions. 
 * 
 * The value for the type-of-service (TOS) needs to be masked
 * to get consistent behaviour with other operating systems.
 */
JNIEXPORT int JNICALL
NET_SetSockOpt(int s, int level, int optname, const void *optval,
	       int optlen)
{   
    int rv;

    if (level == IPPROTO_IP && optname == IP_TOS) {
	int *tos = (int *)optval;
	*tos &= (IPTOS_TOS_MASK | IPTOS_PREC_MASK);
    }

    rv = setsockopt(s, level, optname, optval, optlen);

    if (rv == SOCKET_ERROR) {
	/*
	 * IP_TOS & IP_MULTICAST_LOOP can't be set on some versions
	 * of Windows.
	 */
	if ((WSAGetLastError() == WSAENOPROTOOPT) &&
	    (level == IPPROTO_IP) &&
	    (optname == IP_TOS || optname == IP_MULTICAST_LOOP)) {
	    rv = 0;
	}

	/*
	 * IP_TOS can't be set on unbound UDP sockets.
	 */
	if ((WSAGetLastError() == WSAEINVAL) && 
	    (level == IPPROTO_IP) &&
	    (optname == IP_TOS)) {
	    rv = 0;
	}
    }

    return rv;
}

/*
 * Wrapper for setsockopt dealing with Windows specific issues :-
 *
 * IP_TOS is not supported on some versions of Windows so 
 * instead return the default value for the OS.
 */
JNIEXPORT int JNICALL
NET_GetSockOpt(int s, int level, int optname, void *optval,
	       int *optlen)
{
    int rv;

    rv = getsockopt(s, level, optname, optval, optlen);


    /*
     * IPPROTO_IP/IP_TOS is not supported on some Windows
     * editions so return the default type-of-service
     * value.
     */
    if (rv == SOCKET_ERROR) {

	if (WSAGetLastError() == WSAENOPROTOOPT &&
	    level == IPPROTO_IP && optname == IP_TOS) {

	    int *tos;
	    tos = (int *)optval;
	    *tos = NET_GetDefaultTOS();

	    rv = 0;
	}
    }

    return rv;
}

/*
 * Wrapper for bind winsock call - transparent converts an 
 * error related to binding to a port that has exclusive access
 * into an error indicating the port is in use (facilitates
 * better error reporting).
 */
JNIEXPORT int JNICALL
NET_Bind(int s, struct sockaddr *him, int len)
{
    int rv = bind(s, him, len);

    if (rv == SOCKET_ERROR) {
	/*
	 * If bind fails with WSAEACCES it means that a privileged
	 * process has done an exclusive bind (NT SP4/2000/XP only).
	 */
	if (WSAGetLastError() == WSAEACCES) {
	    WSASetLastError(WSAEADDRINUSE);
	}
    }

    return rv;
}

JNIEXPORT int JNICALL
NET_SocketClose(int fd) {
    struct linger l;
    int ret;
    int len = sizeof (l);
    if (getsockopt(fd, SOL_SOCKET, SO_LINGER, (char *)&l, &len) == 0) {
	if (l.l_onoff == 0) {
	    WSASendDisconnect(fd, NULL);
	}
    }
    ret = closesocket (fd);
    return ret;
}

JNIEXPORT int JNICALL
NET_Timeout(int fd, long timeout) {
    int ret; 
    fd_set tbl; 
    struct timeval t; 
    t.tv_sec = timeout / 1000; 
    t.tv_usec = (timeout % 1000) * 1000; 
    FD_ZERO(&tbl); 
    FD_SET(fd, &tbl); 
    ret = select (fd + 1, &tbl, 0, 0, &t); 
    return ret;
}


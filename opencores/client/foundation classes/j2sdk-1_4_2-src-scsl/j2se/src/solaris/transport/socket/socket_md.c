/*
 * @(#)socket_md.c	1.11 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


#include <unistd.h>
#include "socket_md.h"
#include "sysSocket.h"

int
dbgsysListen(int fd, INT32 count) {
    return listen(fd, count);
}

int
dbgsysConnect(int fd, struct sockaddr *name, int namelen) {
    return connect(fd, name, namelen);
}

int
dbgsysAccept(int fd, struct sockaddr *name, int *namelen) {
    return accept(fd, name, namelen);
}

int
dbgsysRecvFrom(int fd, char *buf, int nBytes,
                  int flags, struct sockaddr *from, int *fromlen) {
    return recvfrom(fd, buf, nBytes, flags, from, fromlen);
}

int
dbgsysSendTo(int fd, char *buf, int len,
                int flags, struct sockaddr *to, int tolen) {
    return sendto(fd, buf, len, flags, to, tolen);
}

int
dbgsysRecv(int fd, char *buf, int nBytes, int flags) {
    return recv(fd, buf, nBytes, flags);
}

int
dbgsysSend(int fd, char *buf, int nBytes, int flags) {
    return send(fd, buf, nBytes, flags);
}

struct hostent *
dbgsysGetHostByName(char *hostname) {
    return gethostbyname(hostname);
}

unsigned short
dbgsysHostToNetworkShort(unsigned short hostshort) {
    return htons(hostshort);
}

int
dbgsysSocket(int domain, int type, int protocol) {
    return socket(domain, type, protocol);
}

int dbgsysSocketClose(int fd) {
    return close(fd);
}

int
dbgsysBind(int fd, struct sockaddr *name, int namelen) {
    return bind(fd, name, namelen);
}

UINT32
dbgsysHostToNetworkLong(UINT32 hostlong) {
    return htonl(hostlong);
}

unsigned short
dbgsysNetworkToHostShort(unsigned short netshort) {
    return ntohs(netshort);
}

int
dbgsysGetSocketName(int fd, struct sockaddr *name, int *namelen) {
    return getsockname(fd, name, namelen);
}

UINT32
dbgsysNetworkToHostLong(UINT32 netlong) {
    return ntohl(netlong);
}


int
dbgsysSetSocketOption(int fd, jint cmd, jboolean on, jvalue value) 
{
    if (cmd == TCP_NODELAY) {
        struct protoent *proto = getprotobyname("TCP");
        int tcp_level = (proto == 0 ? IPPROTO_TCP: proto->p_proto);
        INT32 onl = (INT32)on;

        if (setsockopt(fd, tcp_level, TCP_NODELAY,
                       (char *)&onl, sizeof(INT32)) < 0) {
                return SYS_ERR;
        }
    } else if (cmd == SO_LINGER) {
        struct linger arg;
        arg.l_onoff = on;

        if(on) {
            arg.l_linger = (unsigned short)value.i;
            if(setsockopt(fd, SOL_SOCKET, SO_LINGER,
                          (char*)&arg, sizeof(arg)) < 0) {
                return SYS_ERR;
            }
        } else {
            if (setsockopt(fd, SOL_SOCKET, SO_LINGER,
                           (char*)&arg, sizeof(arg)) < 0) {
                return SYS_ERR;
            }
        }
    } else if (cmd == SO_SNDBUF) {
        jint buflen = value.i;
        if (setsockopt(fd, SOL_SOCKET, SO_SNDBUF,
                       (char *)&buflen, sizeof(buflen)) < 0) {
            return SYS_ERR;
        }
    } else if (cmd == SO_REUSEADDR) {
        int oni = (int)on;
        if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
                       (char *)&oni, sizeof(oni)) < 0) {
            return SYS_ERR;

        }
    } else {
        return SYS_ERR;
    }
    return SYS_OK;
}



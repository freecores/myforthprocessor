/*
 * @(#)NetworkInterface.h	1.2 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef NETWORK_INTERFACE_H
#define NETWORK_INTERFACE_H

/*
 * Structures used when enumerating interfaces and addresses
 */
typedef struct _netaddr  {
    unsigned long addr;		    /* IPv4 address */
    struct _netaddr *next;
} netaddr;

typedef struct _netif {
    char *name;
    char *displayName;
    DWORD dwIndex;		/* Internal index */
    int index;			/* Friendly index */
    struct _netif *next;
} netif;


extern void free_netif(netif *netifP);
extern void free_netaddr(netaddr *netaddrP);

#endif

/*
 * @(#)ProxySupport5.h	1.5 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef PROXYSUPPORT_H
#define PROXYSUPPORT_H

#include "JavaVM5.h"

class ProxySupport5 {
public:
    ProxySupport5(JavaVM5 *javaVM);
    void ProxmapReply(const char *stream_url, int len, void *buffer);
    void ProxmapFindProxy(nsIPluginInstance *inst, char * url,
				    char * host);
protected:
    struct LongTermState* state;
    JavaVM5*              javaVM;
};

#endif


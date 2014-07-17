/*
 * @(#)hprof_md.h	1.7 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef _SOLARIS_HPROF_MD_H_
#define _SOLARIS_HPROF_MD_H_

#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/param.h>

#ifdef LP64
#define HASH_OBJ_SHIFT 4    /* objects aligned on 32-byte boundary */
#else /* !LP64 */
#define HASH_OBJ_SHIFT 3    /* objects aligned on 16-byte boundary */
#endif /* LP64 */

#endif /*_SOLARIS_HPROF_MD_H_*/

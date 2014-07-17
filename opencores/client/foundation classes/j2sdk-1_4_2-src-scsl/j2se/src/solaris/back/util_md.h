/*
 * @(#)util_md.h	1.11 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
#include <limits.h>
#include <sys/types.h>

#ifdef _LP64
typedef unsigned long UNSIGNED_JLONG;
typedef unsigned int UNSIGNED_JINT;
#else /* _LP64 */
typedef unsigned long long UNSIGNED_JLONG;
typedef unsigned long UNSIGNED_JINT;
#endif /* _LP64 */

#define MAXPATHLEN		PATH_MAX

#define JDWP_ONLOAD_SYMBOLS   {"JDWP_OnLoad"}


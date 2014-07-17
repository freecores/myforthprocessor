/*
 * @(#)gdefs_md.h	1.5 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * Win32 dependent type definitions
 */

#include <stddef.h>
#ifndef _WIN64
typedef int intptr_t;
typedef unsigned int uintptr_t;
#endif

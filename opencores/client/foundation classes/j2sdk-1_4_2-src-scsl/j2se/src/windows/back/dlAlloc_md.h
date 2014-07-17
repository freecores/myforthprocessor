/*
 * @(#)dlAlloc_md.h	1.10 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*
 * Platform-specific declarations for Doug Lea's malloc. See dlAlloc.c for details.
 */
#include <windows.h>
#include <crtdbg.h>
#include <stddef.h>

#define LACKS_UNISTD_H

#ifndef INTERNAL_SIZE_T
#define INTERNAL_SIZE_T size_t
#endif

#ifndef INTERNAL_SSIZE_T
#ifndef WIN32
#define INTERNAL_SSIZE_T ssize_t
#else
#define INTERNAL_SSIZE_T ptrdiff_t
#endif
#endif


#define __STD_C 1
#ifdef _M_IA64
#define malloc_getpagesize (8*1024)
#elif x86
#define malloc_getpagesize (4096)
#endif

#define ASSERT _ASSERT

/* 
  WIN32 causes an emulation of sbrk to be compiled in
  mmap-based options are not currently supported in WIN32.
*/

/* #define WIN32 */
#ifdef WIN32
#define MORECORE wsbrk
#define HAVE_MMAP 0
#endif

void* wsbrk (INTERNAL_SSIZE_T size);




/*
 * @(#)mlib_sys.c	1.14 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifdef __SUNPRO_C
#pragma ident	"@(#)mlib_sys.c	1.7	98/06/26 SMI"
#endif /* __SUNPRO_C */

#include <stdlib.h>
#include <string.h>

#ifdef __linux__
#include <malloc.h>
#endif

void *mlib_malloc(unsigned int size)
{
#ifdef _MSC_VER
  /*
   * Currently, all MS C compilers for Win32 platforms default to 8 byte
   * alignment. -- from stdlib.h of MS VC++5.0.
   */
  return malloc(size);
#else
  return memalign(8, size);
#endif
}

void mlib_free(void *ptr)
{
  free(ptr);
}

void *mlib_memset(void *s, int c, unsigned int n)
{
  return memset(s, c, n);
}

void *mlib_memcpy(void *s1, void *s2, unsigned int n)
{
  return memcpy(s1, s2, n);
}


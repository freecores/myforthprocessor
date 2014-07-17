/* @(#)ZUTIL.C	1.1 01/02/19 */
/* zutil.c -- target dependent utility functions for the compression library
 * Copyright (C) 1995-1998 Jean-loup Gailly.
 * For conditions of distribution and use, see copyright notice in zlib.h 
 */

/* @(#) $Id$ */

#include <windows.h>
#include "zutil.h"


voidpf zcalloc (opaque, items, size)
    voidpf opaque;
    unsigned items;
    unsigned size;
{
    return (voidpf)GlobalAlloc(GPTR,items*size);
}

void  zcfree (opaque, ptr)
    voidpf opaque;
    voidpf ptr;
{
    if (ptr) GlobalFree(ptr);
}

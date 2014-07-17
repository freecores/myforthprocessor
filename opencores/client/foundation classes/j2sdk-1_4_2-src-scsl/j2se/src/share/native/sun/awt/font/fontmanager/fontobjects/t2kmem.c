/*
 * @(#)t2kmem.c	1.7 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <malloc.h>
#include <string.h>
#include "HeadSpin.h"

#ifdef __cplusplus
extern "C"
{
#endif

struct sizeBlock {
	/* t2k_malloc must maintain mallocs alignment 
           of double the size of a pointer */
        union {
		void *size_pad[2];
		size_t	size;
	} u;
};

typedef struct sizeBlock sizeBlock;

void* t2k_malloc(size_t size)
{
	sizeBlock* block = (sizeBlock*)malloc(size + sizeof(sizeBlock));

	if (block)
	{	block->u.size = block->u.size;
		return block + 1;
	}
	return 0;
}

void t2k_free(void* p)
{
	free((sizeBlock*)p - 1);
}

void* t2k_realloc(void* oldp, size_t newSize)
{
	size_t	oldSize = ((sizeBlock*)oldp - 1)->u.size;
	void*	newp = malloc(newSize);

	if (newp)
	{	((sizeBlock*)newp - 1)->u.size = newSize;

		memmove(newp, oldp, hsMinimum(oldSize, newSize));
	
		t2k_free(oldp);
	}
	return newp;
}

#ifdef __cplusplus
}
#endif

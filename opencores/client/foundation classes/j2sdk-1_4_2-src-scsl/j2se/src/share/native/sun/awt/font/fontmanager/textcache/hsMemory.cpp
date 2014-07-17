/*
 * @(#)hsMemory.cpp	1.10 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * Copyright (C) 1996-1997 all rights reserved by HeadSpin Technology Inc. Chapel Hill, NC USA
 *
 * This software is the property of HeadSpin Technology Inc. and it is furnished
 * under a license and may be used and copied only in accordance with the
 * terms of such license and with the inclusion of the above copyright notice.
 * This software or any other copies thereof may not be provided or otherwise
 * made available to any other person or entity except as allowed under license.
 * No title to and ownership of the software or intellectual property
 * therewithin is hereby transferred.
 *
 * HEADSPIN MAKES NO REPRESENTATIONS OR WARRANTIES ABOUT THE SUITABILITY
 * OF THE SOFTWARE, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
 * TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE, OR NON-INFRINGEMENT. SUN SHALL NOT BE LIABLE FOR
 * ANY DAMAGES SUFFERED BY LICENSEE AS A RESULT OF USING, MODIFYING OR
 * DISTRIBUTING THIS SOFTWARE OR ITS DERIVATIVES.
 *
 * This information in this software is subject to change without notice
*/

#include "hsTemplates.h"


#if HS_BUILD_FOR_MAC
	#include <Memory.h>
#endif

#if HS_BUILD_FOR_WIN32
	#include <string.h>
#endif

Boolean HSMemory::EqualBlocks(const void* block1, const void* block2, UInt32 length)
{
    const Byte*	byte1 = (Byte*)block1;
    const Byte*	byte2 = (Byte*)block2;
    
    while (length--)
        if (*byte1++ != *byte2++)
            return false;
    return true;
}

void* HSMemory::New(UInt32 size)
{
    UInt32 longSize = size + 3 >> 2;
    void *ptr = new UInt32[longSize];
    memset(ptr, 0, longSize << 2);
    return ptr;
}

void HSMemory::Delete(void* block)
{
	delete[] (UInt32*)block;
}

void* HSMemory::Copy(UInt32 length, const void* source)
{
	void* destination = HSMemory::New(length);

	HSMemory::BlockMove(source, destination, length);
	return destination;
}

void HSMemory::Clear(void* m, UInt32 byteLen)
{
	if(!(byteLen & 0x3)) {
		int longLen = byteLen >> 2;
		UInt32 *l = (UInt32 *)m;
		while(longLen--)
			*l++ = 0;
	} else {
		Byte *b = (Byte*)m;
		while(byteLen--)
			*b++ = 0;
	}
}

//////////////////////////////////////////////////////////////////////////////////////

template <class T> T* hsSoftNew(T*& obj)
{
#ifndef CC_NOEX
	try {
#endif // !CC_NOEX
		obj = new T;
#ifndef CC_NOEX
	}
	catch (...) {
		obj = nil;
	}
#endif // !CC_NOEX
	return obj;
}

template <class T> T* hsSoftNew(T*& obj, unsigned count)
{
#ifndef CC_NOEX
	try {
#endif // !CC_NOEX
		obj = new T[count];
#ifndef CC_NOEX
	}
	catch (...) {
		obj = nil;
	}
#endif // !CC_NOEX
	return obj;
}

void* HSMemory::SoftNew(UInt32 size)
{
	UInt32*	p;
	return hsSoftNew(p, (unsigned int) (size + 3 >> 2));
}

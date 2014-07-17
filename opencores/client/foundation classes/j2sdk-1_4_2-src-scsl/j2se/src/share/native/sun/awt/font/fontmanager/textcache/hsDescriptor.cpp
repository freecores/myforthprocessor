/*
 * @(#)hsDescriptor.cpp	1.6 03/01/23
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

#include "hsDescriptor.h"
#include "hsMemory.h"
#include "hsDescriptor_Internal.h"

#ifdef HS_DEBUGGING
	#define kReasonableDescCount		64

	static void VALID_DESC(hsConstDescriptor desc)
	{
		hsAssert(desc->fCount < kReasonableDescCount, "bad count");
		
		UInt32		length = sizeof(hsDescriptorHeader);
		const DescRec*	rec = desc->GetConstRec();

		for (unsigned i = 0; i < desc->fCount; i++)
		{	length += sizeof(DescRec) + hsLongAlign(rec->fLength);
			rec = rec->Next();
		}
		hsAssert(desc->fLength == length, "bad length");
	}
#else
	#define VALID_DESC(desc)
#endif

//////////////////////////////////////////////////////////////////////////////

UInt32 hsDescriptor_ComputeSize(UInt32 count, const UInt32 sizes[])
{
	UInt32	size = sizeof(hsDescriptorHeader) + count * sizeof(DescRec);

	for (UInt32 i = 0; i < count; i++)
		size += hsLongAlign(sizes[i]);

	return size;
}

hsDescriptor hsDescriptor_New(UInt32 size)
{
	hsAssert(size >= sizeof(hsDescriptorHeader) && (size & 3) == 0, "bad size");

	hsDescriptor	desc = (hsDescriptor)HSMemory::New(size);
	hsDescriptor_Reset(desc);
	return desc;
}

hsDescriptor hsDescriptor_New(UInt32 count, const UInt32 sizes[])
{
	return hsDescriptor_New(hsDescriptor_ComputeSize(count, sizes));
}

hsDescriptor hsDescriptor_Copy(hsConstDescriptor src, hsDescriptor dst)
{
	VALID_DESC(src);

	UInt32 length = src->fLength;

	if (dst == nil)
		dst = (hsDescriptor)HSMemory::New(length);
	HSMemory::BlockMove(src, dst, length);

	return dst;
}

void hsDescriptor_Delete(hsDescriptor desc)
{
	VALID_DESC(desc);

	HSMemory::Delete(desc);
}

//////////////////////////////////////////////////////////////////////////////

#ifdef HS_DEBUGGING
	void hsDescriptor_Reset(hsDescriptor desc)
	{
		desc->fLength	= sizeof(hsDescriptorHeader);
		desc->fCount	= 0;
	}

	UInt32 hsDescriptor_Size(hsConstDescriptor desc)
	{
		VALID_DESC(desc);
		return desc->fLength;
	}

	Boolean hsDescriptor_Equal(hsConstDescriptor a, hsConstDescriptor b)
	{
		VALID_DESC(a);
		VALID_DESC(b);

		UInt32		longCount = a->fLength >> 2;
		const UInt32*	ptr_a = (UInt32*)a;
		const UInt32*	ptr_b = (UInt32*)b;

		do {
			if (*ptr_a++ != *ptr_b++)
				return false;
		} while (--longCount);

		return true;
	}

	void* hsDescriptor_Add(hsDescriptor desc, UInt32 tag, UInt32 length)
	{
		VALID_DESC(desc);

		DescRec*	rec	= (DescRec*)((char*)desc + desc->fLength);
		rec->fTag		= tag;
		rec->fLength	= length;

		void*	recData = rec->GetData();
		UInt32	longLength = hsLongAlign(length);

		// clear the last long in case length is not long aligned
		// we want it cleared so that the CheckSum will be reproducible
		if (longLength > length)
			*(UInt32*)((char*)recData + longLength - sizeof(UInt32)) = 0;

		desc->fCount	+= 1;
		desc->fLength	+= sizeof(DescRec) + longLength;
		return recData;
	}
#endif

const void* hsDescriptor_Find(hsConstDescriptor desc, UInt32 tag, UInt32* length, void* data)
{
	VALID_DESC(desc);

	const DescRec*	rec = desc->GetConstRec();
	UInt32		count = desc->fCount;

	for (UInt32 i = 0; i < count; i++)
	{	if (rec->fTag == tag)	
		{	const void* srcData = rec->GetConstData();
			if (length)
				*length = rec->fLength;
			if (data)
				HSMemory::BlockMove(srcData, data, rec->fLength);
			return srcData;
		}
		rec = rec->Next();
	}
	return nil;
}

void* hsDescriptor_Find(hsDescriptor desc, UInt32 tag, UInt32* length, void* data)
{
	VALID_DESC(desc);

	DescRec*	rec = desc->GetRec();
	UInt32	count = desc->fCount;

	for (UInt32 i = 0; i < count; i++)
	{	if (rec->fTag == tag)	
		{	void* srcData = rec->GetData();
			if (length)
				*length = rec->fLength;
			if (data)
				HSMemory::BlockMove(srcData, data, rec->fLength);
			return srcData;
		}
		rec = rec->Next();
	}
	return nil;
}

void hsDescriptor_Remove(hsDescriptor desc, UInt32 tag)
{
	VALID_DESC(desc);

	DescRec*	rec = desc->GetRec();
	UInt32	count = desc->fCount;

	for (UInt32 i = 0; i < count; i++)
	{	if (rec->fTag == tag)	
		{	DescRec*	next = rec->Next();

			HSMemory::BlockMove(next, rec, (char*)next - (char*)desc - desc->fLength);
			desc->fLength	-= (char*)next - (char*)rec;
			desc->fCount	-= 1;
			return;
		}
	}
}

void hsDescriptor_UpdateCheckSum(hsDescriptor desc)
{
	VALID_DESC(desc);

	desc->fCheckSum = 0;	// set to known value before computing

	UInt32	longCount = desc->fLength >> 2;
	UInt32*	ptr = (UInt32*)desc;
	UInt32	checkSum = 0;

	do {
		checkSum += *ptr++;
	} while (--longCount);
	desc->fCheckSum = checkSum;
}

//////////////////////////////////////////////////////////////////////////////

#ifdef HS_DEBUGGING
	UInt32 hsDescriptor_Find32(hsConstDescriptor desc, UInt32 tag)
	{
		hsDebugCode(UInt32	length;)
		hsAssert(hsDescriptor_Find(desc, tag, &length, nil) && length == sizeof(UInt32), "bad Find32");

		return *(UInt32*)hsDescriptor_Find(desc, tag, nil, nil);
	}

	hsScalar hsDescriptor_FindScalar(hsConstDescriptor desc, UInt32 tag)
	{
		hsDebugCode(UInt32	length;)
		hsAssert(hsDescriptor_Find(desc, tag, &length, nil) && length == sizeof(hsScalar), "bad FindScalar");

		return *(hsScalar*)hsDescriptor_Find(desc, tag, nil, nil);
	}
#endif


/*
 * @(#)hsStream.cpp	1.12 03/01/23
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
#include <stdio.h>

#include "HeadSpin.h"
#include "hsMemory.h"
#include <ctype.h>

#if HS_BUILD_FOR_MAC
	#include <Files.h>
#endif
#if HS_BUILD_FOR_WIN32
	#define WIN32_EXTRA_LEAN
	#define WIN32_LEAN_AND_MEAN

	#include <windows.h>

#endif

static int CStrLen(const char s[])
{
	const char* base = s;

	while (*s)
		++s;
	return s - base;
}

//////////////////////////////////////////////////////////////////////////////////

#if HS_BUILD_FOR_WIN32
	static void swapIt(Int32 *swap)
	{
		Byte*	c = (Byte*)swap;
		Byte		t = c[0];

		c[0] = c[3];
		c[3] = t;
		t = c[1];
		c[1] = c[2];
		c[2] = t;
	}

	static void swapIt(int *swap)
	{
		swapIt((Int32*)swap);
	}

	static void swapIt(float *swap)
	{
		swapIt((Int32*)swap);
	}

	static void swapIt(Int16 *swap)
	{
		Byte *c = (Byte*)swap;
		Byte t;
		t = c[0];
		c[0] = c[1];
		c[1] = t;
	}
#else
	#define swapIt(value)
#endif

//////////////////////////////////////////////////////////////////////////////////

void hsStream::WriteString(const char cstring[])
{
	Write(CStrLen(cstring), cstring);
}

UInt8 hsStream::ReadByte()
{
	UInt8	value;

	this->Read(sizeof(UInt8), &value);
	return value;
}

Boolean  hsStream::AtEnd()
{
	hsAssert(0,"No hsStream::AtEnd() implemented for this stream class");
	return false;
}

Boolean hsStream::GetToken(char *s, int maxLen)
{	
	Int32 k = 0;
	Byte b;
	while(!AtEnd())
	{
		b=this->ReadByte();
		if(!isspace(b))
		{
			if(  (k <= maxLen ) || (maxLen == 0) )
				s[k++] = b;
		}
		else
		{
			if(k>0)
			{
				s[k] = 0;
				return true;
			}
		}
	}
	return false;
}
   
UInt16 hsStream::ReadSwap16()
{
	UInt16	value;
	this->Read(sizeof(UInt16), &value);
	swapIt((Int16*)&value);
	return value;
}

void hsStream::ReadSwap16(int count, UInt16 values[])
{
	this->Read(count * sizeof(UInt16), values);
#if HS_BUILD_FOR_WIN32
	for (int i = 0; i < count; i++)
		swapIt((Int16*)&values[i]);
#endif
}

UInt32 hsStream::ReadSwap32()
{
	UInt32	value;
	this->Read(sizeof(UInt32), &value);
	swapIt((Int32*)&value);
	return value;
}

void hsStream::ReadSwap32(int count, UInt32 values[])
{
	this->Read(count * sizeof(UInt32), values);
#if HS_BUILD_FOR_WIN32
	for (int i = 0; i < count; i++)
		swapIt((Int32*)&values[i]);
#endif
}

float hsStream::ReadSwapFloat()
{
	float	value;
	this->Read(sizeof(float), &value);
	swapIt(&value);
	return value;
}

hsScalar hsStream::ReadSwapScalar()
{
	hsScalar	value;
	this->Read(sizeof(hsScalar), &value);
	swapIt(&value);
	return value;
}

void hsStream::ReadSwapFloat(int count, float values[])
{
	this->Read(count * sizeof(float), values);
#if HS_BUILD_FOR_WIN32
	for (int i = 0; i < count; i++)
		swapIt(&values[i]);
#endif
}

void hsStream::WriteByte(UInt8 value)
{
	this->Write(sizeof(UInt8), &value);
}

void  hsStream::WriteSwap16(UInt16 value)
{
	swapIt((Int16*)&value);
	this->Write(sizeof(Int16), &value);
}

void  hsStream::WriteSwap16(int count, const UInt16 values[])
{
	for (int i = 0; i < count; i++)
		this->WriteSwap16(values[i]);
}

void  hsStream::WriteSwap32(UInt32 value)
{
	swapIt((Int32*)&value);
	this->Write(sizeof(Int32), &value);
}

void  hsStream::WriteSwap32(int count, const UInt32 values[])
{
	for (int i = 0; i < count; i++)
		this->WriteSwap32(values[i]);
}

void hsStream::WriteSwapFloat(float value)
{
	swapIt(&value);
	this->Write(sizeof(float), &value);
}

void hsStream::WriteSwapScalar(hsScalar value)
{
	swapIt(&value);
	this->Write(sizeof(hsScalar), &value);
}

void hsStream::WriteSwapFloat(int count, const float values[])
{
	for (int i = 0; i < count; i++)
		this->WriteSwapFloat(values[i]);
}

//////////////////////////////////////////////////////////////////////////////////

UInt32 hsFileStream::GetFileRef()
{
	return fRef;
}

void hsFileStream::SetFileRef(UInt32 ref)
{
	hsAssert(ref != (UInt32)-1, "SetFileRef == -1");
	fRef = ref;
}

hsFileStream::hsFileStream()
{
	fRef = (UInt32)-1;
}

hsFileStream::~hsFileStream()
{

}

UInt32 hsFileStream::Read(UInt32 bytes,  void* buffer)
{
	fBytesRead += bytes;
#if HS_BUILD_FOR_MAC
	Int16	err;

	hsAssert(fRef != -1, "fRef == -1");

	err = FSRead(fRef, (long*)&bytes, buffer);
	if (err == noErr)
		return bytes;
	else
		return -1;
#elif HS_BUILD_FOR_WIN32
	UInt32 rBytes;
	ReadFile((HANDLE)fRef, buffer, bytes, &rBytes, nil);
	if(bytes == rBytes)
		return bytes;
	else
		return -1;
#else
	return (UInt32)-1;
#endif
}

UInt32 hsFileStream::Write(UInt32 bytes, const void* buffer)
{
#if HS_BUILD_FOR_MAC
	Int16	err;

	hsAssert(fRef != -1, "fRef == -1");

	err = FSWrite(fRef, (long*)&bytes, buffer);
	if (err == noErr)
		return bytes;
	else
	{	
		hsDebugMessage("hsFileStream::Write failed", err);
		return -1;
	}
#elif HS_BUILD_FOR_WIN32
	UInt32 wBytes;
	WriteFile((HANDLE)fRef, buffer, bytes, &wBytes, nil);
	if(bytes == wBytes)
		return bytes;
	else
	{
		char str[128];
		sprintf(str, "hsFileStream::Write failed.  err %d", GetLastError());
		hsAssert(false, str);
		return -1;
	}
#else
	return (UInt32)-1;
#endif
}


Boolean  hsFileStream::AtEnd()
{
#if HS_BUILD_FOR_MAC
	Int32 eof;
	Int32 pos;
	GetEOF(fRef, &eof);
	GetFPos(fRef, &pos);
	return pos >= eof;
#elif HS_BUILD_FOR_WIN32
	UInt32 bytes;
	PeekNamedPipe((void*)fRef, nil, 0, nil, &bytes, nil);
	
	return bytes>0;
#else
	hsAssert(0,"No hsStream::AtEnd() implemented for this stream class");
	return false;
#endif
}

void hsFileStream::Skip(UInt32 delta)
{
#if HS_BUILD_FOR_MAC
	short err = SetFPos(fRef, fsFromMark, delta);
	
	hsAssert(err == noErr, "SetFPos failed");
#elif HS_BUILD_FOR_WIN32
	hsDebugMessage("hsFileStream::Skip unimplemented", 0);
#endif
}

//////////////////////////////////////////////////////////////////////////////////////

#if !(HS_BUILD_FOR_REFERENCE)
UInt32 hsUNIXStream::Read(UInt32 bytes,  void* buffer)
{
	fBytesRead += bytes;
	int numItems = fread(buffer, 1 /*size*/, bytes /*count*/, fRef);
	if ((unsigned)numItems < bytes) {
		if (feof(fRef)) {
			// EOF ocurred
			char str[128];
			sprintf(str, "Hit EOF on UNIX Read, only read %d out of requested %d bytes\n", numItems, bytes);
			hsDebugMessage(str, 0);
		}
		else {
			hsDebugMessage("Error on UNIX Read", 0/*ferror(fRef)*/);
		}
	}
	return numItems;
}

Boolean  hsUNIXStream::AtEnd()
{
	Boolean rVal;
	int x = getc(fRef);
	rVal = feof(fRef) != 0;
	ungetc(x, fRef);
	return rVal;
}

UInt32 hsUNIXStream::Write(UInt32 bytes, const void* buffer)
{
	return fwrite(buffer, bytes, 1, fRef);
}

void hsUNIXStream::Skip(UInt32 delta)
{
	(void)fseek(fRef, delta, SEEK_CUR);
}
#endif

//////////////////////////////////////////////////////////////////////////////////////

#define kRAMStreamChunkSize		1024

struct hsRAMChunk {
	hsRAMChunk*	fNext;
	char			fData[kRAMStreamChunkSize];
};

static hsRAMChunk* AssureNextChunk(hsRAMChunk* chunk)
{
	if (chunk->fNext == nil)
	{	chunk->fNext = new hsRAMChunk;
		chunk->fNext->fNext = nil;
	}
	return chunk->fNext;
}

hsRAMStream::hsRAMStream()
{
	fTotalOffset	= 0;
	fTotalWritten	= 0;
	fChunkBase	= nil;
	
	fLocalOffset	= 0;
	fCurrChunk	= nil;
}

hsRAMStream::~hsRAMStream()
{
	hsRAMChunk*	chunk = fChunkBase;

	while (chunk != nil)
	{	hsRAMChunk* next = chunk->fNext;
		delete chunk;
		chunk = next;
	}
}

Boolean  hsRAMStream::AtEnd()
{
	return (fBytesRead >= fTotalWritten);
}

UInt32 hsRAMStream::Read(UInt32 byteCount, void * buffer)
{
	fBytesRead += byteCount;

	if (byteCount + fTotalOffset > fTotalWritten)
	{	hsDebugMessage("hsRAMStream::Read past EOF", byteCount);
		byteCount = fTotalWritten - fTotalOffset;
	}
	fTotalOffset += byteCount;

	UInt32	firstSize = kRAMStreamChunkSize - fLocalOffset;
	if (firstSize > byteCount)
		firstSize = byteCount;
	Int32	middleCount = (byteCount - firstSize) / kRAMStreamChunkSize;
	Int32	lastSize = byteCount - middleCount * kRAMStreamChunkSize - firstSize;
	
	hsAssert(firstSize + middleCount * kRAMStreamChunkSize + lastSize == byteCount, "bad sizes in RAM::Read");
	
	HSMemory::BlockMove(fCurrChunk->fData + fLocalOffset, buffer, firstSize);
	buffer = (char*)buffer + firstSize;
	fLocalOffset += firstSize;

	if (fLocalOffset == kRAMStreamChunkSize)
	{	fCurrChunk = fCurrChunk->fNext;

		for (; middleCount; --middleCount)
		{	HSMemory::BlockMove(fCurrChunk->fData, buffer, kRAMStreamChunkSize);
			fCurrChunk = fCurrChunk->fNext;
			buffer = (char*)buffer + kRAMStreamChunkSize;
		}
		HSMemory::BlockMove(fCurrChunk->fData, buffer, lastSize);
		fLocalOffset = lastSize;
	}
	return byteCount;
}

UInt32 hsRAMStream::Write(UInt32 byteCount, const void* buffer)
{
	fTotalOffset += byteCount;
	if (fTotalOffset > fTotalWritten)
		fTotalWritten = fTotalOffset;

	if (fChunkBase == nil)
	{	fChunkBase = new hsRAMChunk;
		fCurrChunk = fChunkBase;
		fCurrChunk->fNext = nil;
	}

	UInt32	firstSize = kRAMStreamChunkSize - fLocalOffset;
	if (firstSize > byteCount)
		firstSize = byteCount;
	Int32	middleCount = (byteCount - firstSize) / kRAMStreamChunkSize;
	Int32	lastSize = byteCount - middleCount * kRAMStreamChunkSize - firstSize;
	
	hsAssert(firstSize + middleCount * kRAMStreamChunkSize + lastSize == byteCount, "bad sizes in RAM::Write");
	
	HSMemory::BlockMove(buffer, fCurrChunk->fData + fLocalOffset, firstSize);
	buffer = (const char*)buffer + firstSize;
	fLocalOffset += firstSize;

	if (fLocalOffset == kRAMStreamChunkSize)
	{	fCurrChunk = AssureNextChunk(fCurrChunk);

		for (; middleCount; --middleCount)
		{	HSMemory::BlockMove(buffer, fCurrChunk->fData, kRAMStreamChunkSize);
			fCurrChunk = AssureNextChunk(fCurrChunk);
			buffer = (const char*)buffer + kRAMStreamChunkSize;
		}
		HSMemory::BlockMove(buffer, fCurrChunk->fData, lastSize);
		fLocalOffset = lastSize;
	}
	return byteCount;
}

void hsRAMStream::Skip(UInt32 deltaByteCount)
{
	hsDebugMessage("RAM::Skip not implemented", deltaByteCount);
}

void hsRAMStream::Rewind()
{
	fTotalOffset = 0;
	fLocalOffset = 0;
	fCurrChunk = fChunkBase;
}


/*
 * Nullsoft "SuperPimp" Installation System 
 * version 1.0j - November 12th 2000
 *
 * Copyright (C) 1999-2000 Nullsoft, Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 * Justin Frankel
 * justin@nullsoft.com
 *
 * This source distribution includes portions of zlib. see zlib/zlib.h for
 * its license and so forth. Note that this license is also borrowed from zlib.
 *
 *
 * Portions Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)fileform.c	1.3 03/01/23
 */


#include <windows.h>
#include <tchar.h>
#include "fileform.h"
#include "util.h"
#include "state.h"
#include "zlib/zlib.h"

// returns -1 on read error
// returns -2 on header corrupt
// returns -3 on error seeking (should never occur)
// returns offset of data block in file on success
// on success, *r_head will be set to a pointer that should eventually be GlobalFree()'d.

int isheader(firstheader *h)
{
  unsigned int x;
  if (h->_xDEADBEEF != 0xDEADBEEF || 
      (h->_x0BADF00D_or_DEADF00D != 0x0BADF00D 
#ifdef NSIS_CONFIG_UNINSTALL_SUPPORT
      && h->_x0BADF00D_or_DEADF00D != 0x0DEADF00D
#endif
      )) return 0;
  for (x = 0; x < _tcslen(_T("nsisinstall")) + 1; x ++) if (h->_nsisinstall[x] != _T("nsisinstall")[x]) return 0;
  return 1;
}

int loadHeaders(HANDLE hFile, header **r_head, section **r_sec, entry **r_ent, uninstall_header **r_uhead, int *datablocklen)
{
  DWORD r;
  void *data;
  firstheader h;
  
  if (!ReadFile(hFile,(LPVOID)&h,sizeof(h),&r,NULL) || r != sizeof(h)||!isheader(&h)) return -1;
  *datablocklen=h.length_of_all_following_data;
  *datablocklen-=h.length_of_header;

  if (*datablocklen < 0)
  {
    return -1;
  }
  data=(void*)GlobalAlloc(GMEM_FIXED,h.length_of_header);
  if (!ReadFile(hFile,data,h.length_of_header,&r,NULL) || 
    (int)r != h.length_of_header)
  {
    GlobalFree((HGLOBAL)data);
    return -1;
  }

#ifdef NSIS_CONFIG_UNINSTALL_SUPPORT
  if (h._x0BADF00D_or_DEADF00D==0x0DEADF00D)
  {
    *r_head=NULL;
    *r_sec=NULL;
    *r_uhead=(uninstall_header*)data;
    *r_ent=(entry *) (((char *)(*r_uhead)) + sizeof(uninstall_header));
  }
  else
#endif
  {
    *r_uhead=NULL;
    *r_sec=(section *) (((char *)data)+sizeof(header));
    *r_head=(header *)data;
    *r_ent=(entry *) (((char *)(*r_sec)) + sizeof(section)* (*r_head)->num_sections);
  }
  r=SetFilePointer(hFile,0,NULL,FILE_CURRENT);
  return (int)r;
}

int GetStringFromDataBlock(HANDLE hFile, int offset, LPTSTR str, int maxlen)
{
  SetFilePointer(hFile,offset,NULL,FILE_BEGIN);
  while (1)
  {
    DWORD r;
    if (!ReadFile(hFile,(LPVOID)str,1,&r,NULL)||r!=1) return -1;
    if (!*str) return 0;
    str=CharNext(str);
    if (--maxlen<1) { *str=0; return -2; }
  }
}

int GetIntFromDataBlock(HANDLE hFile, int offset)
{
  DWORD r;
  int ret;
  SetFilePointer(hFile,offset,NULL,FILE_BEGIN);
  if (!ReadFile(hFile,(LPVOID)&ret,sizeof(int),&r,NULL) || r != sizeof(int)) return 0;
  return ret;
}


// returns -3 if compression error/eof/etc

int GetCompressedDataFromDataBlock(HANDLE hFile, int offset, HANDLE hFileOut)
{
  int retval=0;
  char *inbuffer=(char *)GlobalAlloc(GMEM_FIXED,COMPRESS_MAX_BLOCKSIZE+COMPRESS_MAX_BLOCKSIZE+12+COMPRESS_MAX_BLOCKSIZE/1000);
  char *outbuffer = inbuffer+COMPRESS_MAX_BLOCKSIZE+12+COMPRESS_MAX_BLOCKSIZE/1000;
  char c;
  DWORD r;
  z_stream stream={0,};
  SetFilePointer(hFile,offset,NULL,FILE_BEGIN);

  if (!ReadFile(hFile,(LPVOID)&c,1,&r,NULL)) { GlobalFree((HGLOBAL)inbuffer); return -3; }


  if (c==1)
  {
    inflateInit(&stream);

    while (1)
    {
      DWORD r;
      int err;
      unsigned int inlen;
      int outlen=COMPRESS_MAX_BLOCKSIZE;

      if (!ReadFile(hFile,(LPVOID)&inlen,sizeof(int),&r,NULL) || r != sizeof(unsigned int)) { retval=-3; break;}
      if (!inlen) break;
      if (!ReadFile(hFile,(LPVOID)inbuffer,inlen,&r,NULL) || r != (unsigned int)inlen) { retval=-3; break;}


      stream.next_in = (Bytef*)inbuffer;
      stream.avail_in = (uInt)inlen;
      stream.next_out = outbuffer;
      stream.avail_out = (uInt)outlen;

      err=inflate(&stream, Z_SYNC_FLUSH);
      if (err == Z_STREAM_ERROR || err==Z_BUF_ERROR||err==Z_DATA_ERROR||err==Z_NEED_DICT) { retval=-3; break; }
      outlen-=stream.avail_out;

      if (!WriteFile(hFileOut,outbuffer,outlen,&r,NULL) || (int)r != outlen) { retval=-2; break; }
      retval+=r;
    }
    inflateEnd(&stream);
  }
  else if (c == 0)
  {
    int len;
    if (!ReadFile(hFile,(LPVOID)&len,sizeof(int),&r,NULL)) { GlobalFree((HGLOBAL)inbuffer); return -3; }
    while (len > 0)
    {
      DWORD t;
      if (!ReadFile(hFile,(LPVOID)inbuffer,min(len,65536),&r,NULL)) { retval=-3; break;}
      if (!WriteFile(hFileOut,inbuffer,r,&t,NULL)) { retval=-2; break; }
      retval+=r;
      len-=r;
    }
  }
  else 
  {
    log_printf2("unknown compression: %d\n",c);
    retval=-6;
  }
  GlobalFree((HGLOBAL)inbuffer);
  return retval;
}

int GetCompressedDataFromDataBlockToMemory(HANDLE hFile, int offset, char *out, int out_len)
{
  int retval=0;
  char *inbuffer=(char *)GlobalAlloc(GMEM_FIXED,COMPRESS_MAX_BLOCKSIZE+12+COMPRESS_MAX_BLOCKSIZE/1000);
  char c;
  DWORD r;
  z_stream stream={0,};
  SetFilePointer(hFile,offset,NULL,FILE_BEGIN);

  if (!ReadFile(hFile,(LPVOID)&c,1,&r,NULL)) { GlobalFree((HGLOBAL)inbuffer); return -3; }

  if (c==1)
  {
    inflateInit(&stream);

    while (1)
    {
      DWORD r;
      int err;
      unsigned int inlen;
      int outlen=32768;

      if (!ReadFile(hFile,(LPVOID)&inlen,sizeof(int),&r,NULL) || r != sizeof(unsigned int)) { retval=-3; break;}
      if (!inlen) break;
      if (!ReadFile(hFile,(LPVOID)inbuffer,inlen,&r,NULL) || r != (unsigned int)inlen) { retval=-3; break;}

      stream.next_in = (Bytef*)inbuffer;
      stream.avail_in = (uInt)inlen;
      stream.next_out = out;
      stream.avail_out = (uInt)out_len;

      err=inflate(&stream, Z_SYNC_FLUSH);
      if (err<0) { retval=-3; break; }
      r=stream.next_out-out;
      out+=r;
      out_len-=r;
      retval+=r;
    }
    inflateEnd(&stream);
  }
  else if (c == 0)
  {
    int len;
    if (!ReadFile(hFile,(LPVOID)&len,sizeof(int),&r,NULL)) { GlobalFree((HGLOBAL)inbuffer); return -3; }
    if (!ReadFile(hFile,(LPVOID)out,min(len,out_len),&r,NULL)) { GlobalFree((HGLOBAL)inbuffer); return -3; }
    retval+=r;
  }
  else retval=-6;
  GlobalFree((HGLOBAL)inbuffer);
  return retval;
}


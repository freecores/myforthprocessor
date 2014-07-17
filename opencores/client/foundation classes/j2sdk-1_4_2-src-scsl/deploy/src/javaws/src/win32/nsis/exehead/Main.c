/*
 * Nullsoft "SuperPimp" Installation System - main.cpp - executable header main code
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
 * Portions Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)Main.c	1.11 03/01/23
 */

#include <windows.h>
#include <tchar.h>
#include <malloc.h>
#include <stdio.h>
#include "util.h"
#include "fileform.h"
#include "state.h"
#include "ui.h"
#include "libc.h"
#include "zlib/zlib.h"
#include "resource.h"

static TCHAR _nscopyright[MAX_PATH];

TCHAR g_tempdir[MAX_PATH];
TCHAR state_install_directory[MAX_PATH];
TCHAR state_output_directory[MAX_PATH];
TCHAR state_alt_directory[MAX_PATH];
TCHAR state_alt_name[MAX_PATH];
TCHAR state_alt_version[64];
DWORD state_return_value = STATE_RETURN_NONE;
DWORD state_reboot_needed = 0;
HWND g_hwnd;
TCHAR g_caption[160];
int g_filehdrsize;
LPTSTR args[64];
int argsc = 0;
int noisy=1;
/* autoinstall: 0 = show all UI; 1 = show only errors; */
/* 2 = show license + errors                           */
int autoinstall=0;
int admincheckoverride=0;
int nodesktopicon=0;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInst,LPSTR lpszCmdParam, int nCmdShow)
{
  HANDLE hFile;
  int ret=0;
  int datablock_offset=-1;
  TCHAR temp[1024];
  static char *crcstr="DONOCHK=";
  TCHAR m_Err[MAX_PATH];
  header *head=NULL;
  uninstall_header *uhead=NULL;
  section *sec=NULL;
  entry *ent=NULL;
  LPTSTR token;
  LPTSTR commandLine=NULL, origCL=NULL;
  HANDLE tokenHandle;
  LUID luid;
  TOKEN_PRIVILEGES tpriv;
  TCHAR tmpbuf[MAX_PATH];

  /* setup the args */
  origCL = GetCommandLine();
  commandLine = (LPTSTR)GlobalAlloc(GMEM_FIXED, sizeof(TCHAR) * MAX_PATH);
  _tcscpy(commandLine, origCL);
  token = _tcstok(commandLine, _T(" "));
  while (token != NULL) {
	  if (!_tcsicmp(token,_T("/silent")) || !_tcsicmp(token,_T("-silent")) ||
		  !_tcsicmp(token,_T("/s")) || !_tcsicmp(token,_T("-s"))) noisy=0;
	  if (!_tcsicmp(token,_T("/auto")) || !_tcsicmp(token,_T("-auto")) ||
		  !_tcsicmp(token,_T("/a")) || !_tcsicmp(token,_T("-a"))) autoinstall=1;
	  if (!_tcsicmp(token,_T("/license")) || !_tcsicmp(token,_T("-license")) ||
		  !_tcsicmp(token,_T("/l")) || !_tcsicmp(token,_T("-l"))) autoinstall=2;
	  if (!_tcsicmp(token,_T("/force")) || !_tcsicmp(token,_T("-force")) ||
		  !_tcsicmp(token,_T("/f")) || !_tcsicmp(token,_T("-f"))) admincheckoverride=1;
	  if (!_tcsicmp(token,_T("/nodesktopicon")) || !_tcsicmp(token,_T("-nodesktopicon")) ||
		  !_tcsicmp(token,_T("/n")) || !_tcsicmp(token,_T("-n"))) nodesktopicon=1;
	  args[argsc] = malloc((_tcsclen(token) + 1) * sizeof(TCHAR));
	  _tcscpy(args[argsc++], token);
	  token=_tcstok(NULL, _T(" "));
  }
  g_hInstance=GetModuleHandle(NULL);
  GetModuleFileName(g_hInstance,temp,sizeof(temp));

  GetTempPath(MAX_PATH, g_tempdir);

  GETRESOURCE(_nscopyright, JAVAWS_NS_COPYRIGHT);
  GETRESOURCE(m_Err, JAVAWS_ERROR_OPENSELF);
  GETRESOURCE(g_caption, JAVAWS_ERROR_GCAPTION);

  hFile=CreateFile(temp,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL);
  if (hFile==INVALID_HANDLE_VALUE)
  {
err:
    if (hFile!=INVALID_HANDLE_VALUE) CloseHandle(hFile);
    if (noisy) MessageBox(NULL,m_Err,g_caption,MB_OK|MB_ICONSTOP);
    goto end;
  }
  {
    int file_length=SetFilePointer(hFile,crcstr[8]?0:-4,NULL,FILE_END);
    DWORD l;
    int acrc=0;
    int crc=adler32(0L, Z_NULL, 0);
    int pos=0;
    int length=file_length;
    if (!crcstr[8] && !ReadFile(hFile,&acrc,4,&l,NULL))
    {
	  GETRESOURCE(m_Err, JAVAWS_ERROR_READCRC);
      goto err;
    }
    SetFilePointer(hFile,0,NULL,FILE_BEGIN);
    while (length > 0)
    {
      static char buffer[512];
      if (!ReadFile(hFile,buffer,min(sizeof(buffer),length),&l,NULL))
      {
 	    GETRESOURCE(m_Err, JAVAWS_ERROR_READDATA);
        goto err;
      }
      crc=adler32(crc, buffer, l);
      length -= l;
      if (!g_filehdrsize && isheader((firstheader*)buffer))
        g_filehdrsize=pos;
      pos+=l;
    }
    if (!crcstr[8] && crc != acrc)
    {
	  GETRESOURCE(m_Err, JAVAWS_ERROR_CRC);
      goto err;
    }
    if (g_filehdrsize)
    {
      int lenofdatablock;
      SetFilePointer(hFile,g_filehdrsize,NULL,FILE_BEGIN);
      datablock_offset=loadHeaders(hFile,&head,&sec,&ent,&uhead,&lenofdatablock);
      if (datablock_offset>=0 && datablock_offset+lenofdatablock > file_length)
      {
		TCHAR error[MAX_PATH];
		GETRESOURCE(error, JAVAWS_ERROR_TOOSHORT);
        CloseHandle(hFile);
        if (noisy) MessageBox(NULL,error,g_caption,MB_OK);
        goto end;
      }
    }
  }
  if (datablock_offset < 0)
  {
	TCHAR error[MAX_PATH];
	GETRESOURCE(error, JAVAWS_ERROR_FINDDATA);
    CloseHandle(hFile);
    if (noisy) MessageBox(NULL,error,g_caption,MB_OK);
    goto end;
  }

  /* make sure we are running as administrator or power user */
  if (admincheckoverride == 0 &&
      (IsRunningOnNT4() || IsRunningOn2000XP()) &&
      !IsCurrentUserAdministratorOrPowerUser()) {
    int doclose = 1;
    TCHAR error[MAX_PATH];
    GETRESOURCE(error, JAVAWS_ERROR_PRIVILEGE);
    if (noisy && MessageBox(NULL, error, g_caption,
			    MB_OKCANCEL | MB_ICONWARNING) == IDCANCEL) {
	doclose = 0;
    }
    if (doclose == 1) {
      CloseHandle(hFile);
      goto end;
    }
  } 

#ifdef NSIS_CONFIG_UNINSTALL_SUPPORT
  if (uhead)
  {
    LPTSTR buf;
    LPTSTR cmdline;
    LPTSTR heapCL;

    heapCL=(LPTSTR)GlobalAlloc(GMEM_FIXED, sizeof(TCHAR) * MAX_PATH);
    buf=(LPTSTR)GlobalAlloc(GMEM_FIXED, sizeof(TCHAR) * MAX_PATH);
    _tcscpy(heapCL, origCL);
    cmdline=heapCL;
    state_install_directory[0]=0;
    if (*cmdline == _T('\"'))
    {
      cmdline = CharNext(cmdline);
      while (*cmdline && *cmdline != _T('\"')) { cmdline=CharNext(cmdline); }
      if (*cmdline) { cmdline=CharNext(cmdline); }
    }
    else {
      while (*cmdline && *cmdline != _T(' ')) { cmdline=CharNext(cmdline); }
    }
    while (*cmdline == _T(' ')) { cmdline=CharNext(cmdline); }
    _tcscpy(buf,cmdline);
    cmdline=buf;
    while (*cmdline && *cmdline != '=') { 
      cmdline=CharNext(cmdline);
    }
    if (*cmdline)
    {
      *cmdline=0;
      cmdline++;
      if (!_tcsicmp(buf, _T("uninstallfrom")))
      {
        _tcscpy(state_install_directory,cmdline);
      }
    }
    _tcscpy(state_output_directory,state_install_directory);
    if (!state_output_directory[0])
    {
      TCHAR buf2[MAX_PATH],ibuf[MAX_PATH];
      GetTempPath(MAX_PATH / 2,buf);
      GetTempFileName(buf,_T("nsu"),0,buf2);
      _tcscpy(buf2+_tcslen(buf2)-_tcslen(_T(".nsu")),_T(".exe"));
      GetModuleFileName(g_hInstance,ibuf,MAX_PATH / 2);
      if (CopyFile(ibuf,buf2,FALSE))
      {
        PROCESS_INFORMATION ProcInfo={0,};
        STARTUPINFO StartUp={sizeof(STARTUPINFO),};
        TCHAR *p=ibuf;  TCHAR *start=ibuf;
        while (*p) p=CharNext(p);
        while (p > ibuf && *p != _T('\\')) p=CharPrev(start, p);
        *p=_T('\0');
        _stprintf(buf, _T("\"%s\" uninstallfrom=%s"),buf2,ibuf);
        p=buf2;  start=buf2;
        while (*p) p=CharNext(p);
        while (p > buf2 && *p != _T('\\')) p=CharPrev(start,p);
        *p=_T('\0');
        if (CreateProcess( NULL, buf, NULL, NULL, FALSE, DETACHED_PROCESS, NULL, buf2, &StartUp, &ProcInfo) )
        {
          if (NULL != ProcInfo.hThread) CloseHandle( ProcInfo.hThread );
          if (NULL != ProcInfo.hProcess) CloseHandle( ProcInfo.hProcess );
        }
      }
      goto end;      
    }
  }
#endif
// setup security here

  if (!OpenProcessToken(GetCurrentProcess(),
						TOKEN_ADJUST_PRIVILEGES,
						&tokenHandle)) {
	  if (GetLastError()!=ERROR_CALL_NOT_IMPLEMENTED) {
		  GETRESOURCE(tmpbuf,JAVAWS_ERROR_OPENTOKEN);
		  if (noisy) MessageBox(g_hwnd,tmpbuf,g_caption,MB_OK);
	  }
  } else {
	  if (!LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &luid)) {
		  // non NT should return CALL NOT IMPLEMENTED -- fail silently in this case
		  if (GetLastError()!=ERROR_CALL_NOT_IMPLEMENTED) {
			  GETRESOURCE(tmpbuf,JAVAWS_ERROR_PRIVALUE);
			  if (noisy) MessageBox(g_hwnd,tmpbuf,g_caption,MB_OK);
		  }
	  } else {
		  tpriv.PrivilegeCount=1;
		  tpriv.Privileges[0].Luid=luid;
		  tpriv.Privileges[0].Attributes=SE_PRIVILEGE_ENABLED;
		  if (!AdjustTokenPrivileges(tokenHandle, FALSE, &tpriv, 0,NULL,NULL)) {
			  if (GetLastError()!=ERROR_CALL_NOT_IMPLEMENTED) {
				  GETRESOURCE(tmpbuf,JAVAWS_ERROR_ADJUSTTOKEN);
				  if (noisy) MessageBox(g_hwnd,tmpbuf,g_caption,MB_OK);
			  }
		  }
	  }
  }
  /*
  }
  */
						
  ret=ui_doinstall(hFile,datablock_offset,head,sec,ent,uhead);
  if (head) GlobalFree((HGLOBAL)head);
#ifdef NSIS_CONFIG_UNINSTALL_SUPPORT
  else if (uhead) GlobalFree((HGLOBAL)uhead);
#endif

  CloseHandle(hFile);
#ifdef NSIS_CONFIG_LOG
  log_write(1);
#endif
end:
  ExitProcess(ret);
}

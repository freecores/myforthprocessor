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
 * @(#)util.c	1.14 03/01/23
 */


#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <shlobj.h>
#include "resource.h"
#include "util.h"
#include "libc.h"
#include "state.h"
#include "i18n.h"
#ifdef NSIS_CONFIG_LOG
TCHAR g_log_file[MAX_PATH];
#endif

#define MAX_VARNAME 64
#define MAX_NAME 256
#define FILE_SEPARATOR '\\'

HANDLE g_hInstance;

int IsCurrentUserAdministratorOrPowerUser(void) {
  int result = 0;
  DWORD i, dwSize = 0, dwResult = 0;
  HANDLE hToken;
  PTOKEN_GROUPS pGroupInfo;
  BYTE sidBuffer[100];
  BYTE sidBuffer2[100];
  PSID pSID = (PSID)&sidBuffer;
  PSID pSID2 = (PSID)&sidBuffer2;
  SID_IDENTIFIER_AUTHORITY SIDAuth = SECURITY_NT_AUTHORITY;
  
  /* Open a handle to the access token for the calling process. */
  
  if (!OpenProcessToken( GetCurrentProcess(), TOKEN_QUERY, &hToken )) {
    return 0;
  }
  
  /* Call GetTokenInformation to get the buffer size. */
  
  if(!GetTokenInformation(hToken, TokenGroups, NULL, dwSize, &dwSize)) {
    dwResult = GetLastError();
    if( dwResult != ERROR_INSUFFICIENT_BUFFER ) {
      return 0;
    }
  }
  
  /* Allocate the buffer. */
  
  pGroupInfo = (PTOKEN_GROUPS) GlobalAlloc( GPTR, dwSize );
  
  /* Call GetTokenInformation again to get the group information. */
  
  if(! GetTokenInformation(hToken, TokenGroups, pGroupInfo, 
			   dwSize, &dwSize ) ) {
    result = 0;
  } else {
    
    /* Create a SID for the BUILTIN\Administrators group. */
    
    if(! AllocateAndInitializeSid( &SIDAuth, 2,
				   SECURITY_BUILTIN_DOMAIN_RID,
				   DOMAIN_ALIAS_RID_ADMINS,
				   0, 0, 0, 0, 0, 0,
				   &pSID) ||
       ! AllocateAndInitializeSid( &SIDAuth, 2,
				   SECURITY_BUILTIN_DOMAIN_RID,
				   DOMAIN_ALIAS_RID_POWER_USERS,
				   0, 0, 0, 0, 0, 0,
				   &pSID2)) {
      result = 0;
    } else {
      
      /* Loop through the group SIDs looking for the administrator SID. */
      
      for(i=0; i<pGroupInfo->GroupCount; i++) {
	if ( EqualSid(pSID, pGroupInfo->Groups[i].Sid) ||
	     EqualSid(pSID2, pGroupInfo->Groups[i].Sid) ) {
	  result = 1;
	  break;
	}
      }
    }
  }
  
  if (pSID)
    FreeSid(pSID);
  if ( pGroupInfo )
    GlobalFree( pGroupInfo );
  return result;
}

int evalStr(LPTSTR s) {
	TCHAR buf[MAX_PATH];
	LPTSTR p,word1,word2;
	int not=0;
	
	_tcscpy(buf,s);
	p=buf;
	while(*p == ' ') p++;
	word1=p;
	while(*p && *p!='!' && *p!='=') {
		if (*p==' ') *p=0;
		p++;
	}
	if (*p=='!') not=1;
	*p++=0;
	if (*p++ != '=') return 0;
	while(*p && *p==' ') p++;
	word2=p;
	while(*p && *p!=' ') p++;
	if (*p) *p=0;

	if (!_tcsicmp(word1,word2))
		return !not;
	else
		return not;
}

void ReportResourceError(UINT uID) {
	TCHAR error[MAX_PATH];
	TCHAR caption[MAX_PATH];
	if (!LoadString(g_hInstance, JAVAWS_ERROR_RESOURCE, error, MAX_PATH)) {
		_stprintf(error, _T("Error loading resource: %ld"), uID);
	}		
    if (!LoadString(g_hInstance, JAVAWS_ERROR_RESOURCE_CAPTION, caption,
		MAX_PATH)) {
		_stprintf(caption, _T("Installer Error"));
	}
	if (noisy) MessageBox(g_hwnd, error, caption, MB_OK | MB_ICONERROR);
} 

void recursive_create_directory(LPTSTR directory)
{
	LPTSTR p, p2;
	int x;
	TCHAR buf[MAX_PATH];
	_tcscpy(buf,directory);
	p=buf+_tcslen(buf);
	while (p > buf && *p != '\\') p=CharPrev(buf, p);
	if (p == buf && *p != '\\') p = CharPrev(buf,p);
	p2 = buf;
	if (*(CharNext(p2)) == ':') {
		for(x=0;x<4;x++) {
			p2=CharNext(p2);
		}
	}
	else if (*p2 == '\\' && *(CharNext(p2)) == '\\')
	{
		p2=CharNext(p2); p2=CharNext(p2);
		while (*p2 && *p2 != '\\') p2=CharNext(p2);
		if (*p2) p2=CharNext(p2);
		while (*p2 && *p2 != '\\') p2=CharNext(p2);
		if (*p2) p2=CharNext(p2);
	}
	if (p >= p2)
	{
		*p=0;
		recursive_create_directory(buf);
	}
	CreateDirectory(directory,NULL);
}

static int strcmp_nstr2(LPTSTR *s1, LPTSTR s2)
{
  LPTSTR ps1=*s1;
  while (*ps1 && *ps1 == *s2) { ps1=CharNext(ps1); s2=CharNext(s2); }
  if (!*s2) *s1=ps1;
  return *s2;
}

static void queryShellFolders(LPTSTR name, LPTSTR out, int queryCommon)
{
	HKEY hKey;

	if (queryCommon) {
		if ( RegOpenKeyEx(HKEY_LOCAL_MACHINE,"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders",0,KEY_READ,&hKey) == ERROR_SUCCESS) {
			TCHAR buf[MAX_PATH];
			int l = MAX_PATH;
			int t = REG_SZ;
			wsprintf(buf, "Common %s", name);
			if (RegQueryValueEx(hKey,buf,NULL,&t,out,&l)==ERROR_SUCCESS) {
				RegCloseKey(hKey);
				return;
			}
		}
	}
	if ( RegOpenKeyEx(HKEY_CURRENT_USER,"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders",0,KEY_READ,&hKey) == ERROR_SUCCESS) {
		int l = MAX_PATH;
		int t=REG_SZ;
		RegQueryValueEx(hKey,name,NULL,&t,out,&l );
		RegCloseKey(hKey);
	}
}

LPTSTR scriptFindString(LPTSTR name) {
	int match = 0;
	int x;

	for (x=0; x<g_nstrings; x++) {
		if (!_tcscmp(name, g_strings[x]->name)) {
			match=1;
			break;
		}
	}

	return (match == 1 ? _tcsdup(g_strings[x]->value) : NULL);
}

void enable_control_chars(LPTSTR out,LPTSTR in) {
        int isfirstchar = 1;
	while(*in) {
	        // this says while we have not reached a "real" backslash (not
		// a DBCS trailing byte backslash, which is actually
		// part of another character), move through the string
		while(*in && (*in!='\\' || (!isfirstchar && IsDBCSLeadByte(*(in-1))))) {
			*out++=*in++;
			isfirstchar = 0;
		}
		if (*in) {
			in++;
			isfirstchar = 0;
			if (*in && *in=='n') { *out++='\n'; }
			in++;
		}
	}
	*out=0;
}

// returns 0 if out==in
// returns 2 if invalid symbol
// returns 3 if error looking up symbol
int process_string(LPTSTR out, LPTSTR in, LPTSTR install_dir)
{
  LPTSTR s=out;

  while (*in)
  {
	  if (*in != '$') {
		  *out=*in;
		  out=CharNext(out);
		  in=CharNext(in);
	  }
    else if (!strcmp_nstr2(&in,_T("$$")))
    {
      *out++=_T('$');
    }
    else
    {
      if (!strcmp_nstr2(&in,"$PROGRAMFILES"))
      {
        TCHAR tbuf[MAX_PATH]=_T("C:\\Program Files");
		    HKEY hKey;
		    if ( RegOpenKeyEx(HKEY_LOCAL_MACHINE,"SOFTWARE\\Microsoft\\Windows\\CurrentVersion",0,KEY_READ,&hKey) == ERROR_SUCCESS)
        {
			    int l = sizeof(tbuf);
			    int t=REG_SZ;
			    RegQueryValueEx(hKey,"ProgramFilesDir",NULL,&t,tbuf,&l);
			    RegCloseKey(hKey);
        }
        _tcscpy(out,tbuf);
      }
	  else if (!strcmp_nstr2(&in,"$STRING_"))
	  {
		TCHAR buf[MAX_PATH];
		TCHAR buf2[MAX_PATH];
		TCHAR buf3[MAX_PATH];
		LPTSTR str, p;
		p=buf;
		while(*in) *p++=*in++;
		*p=0;
		if ((str = scriptFindString(buf)) != NULL) {
			_tcscpy(buf3,str);
			enable_control_chars(buf2,buf3);
			p=buf2;
			while(*p) *out++=*p++;
			*out=0;
		} else {
			wsprintf(buf2, "$STRING_%s", buf);
			p=buf2;
			while(*p) *out++=*p++;
			*out=0;
		}	
	  }
      else if (!strcmp_nstr2(&in,"$UDESKTOP"))
      {
        TCHAR buf[MAX_PATH]="";
        queryShellFolders("Desktop",buf,0);
        if (!buf[0]) return 3;
        _tcscpy(out,buf);
      }
      else if (!strcmp_nstr2(&in,"$CDESKTOP"))
      {
        TCHAR buf[MAX_PATH]="";
        queryShellFolders("Desktop",buf,1);
        if (!buf[0]) return 3;
        _tcscpy(out,buf);
      }
      else if (!strcmp_nstr2(&in,"$INSTDIR"))
      {
        if (!install_dir) return 3;
        _tcscpy(out,install_dir);
      }
      else if (!strcmp_nstr2(&in,"$ALTDIR"))
      {
        if (!state_alt_directory) return 3;
        _tcscpy(out,state_alt_directory);
      }
      else if (!strcmp_nstr2(&in,"$WAVISDIR"))
      {
        TCHAR buf[MAX_PATH];
        if (!install_dir) return 3;
        wsprintf(buf,_T("%s\\winamp.ini"),install_dir);
        GetPrivateProfileString("Winamp","VISDir","",out,MAX_PATH,buf);
        if (!*out) wsprintf(out,"%s\\Plugins",install_dir);
      }
      else if (!strcmp_nstr2(&in,"$WADSPDIR"))
      {
        TCHAR buf[MAX_PATH];
        if (!install_dir) return 3;
        wsprintf(buf,"%s\\winamp.ini",install_dir);
        GetPrivateProfileString("Winamp","DSPDir","",out,MAX_PATH,buf);
        if (!*out) wsprintf(out,"%s\\Plugins",install_dir);
      }
      else if (!strcmp_nstr2(&in,"$WINDIR"))
      {
        GetWindowsDirectory(out,MAX_PATH);
      }
      else if (!strcmp_nstr2(&in,"$SYSDIR"))
      {
        GetSystemDirectory(out,MAX_PATH);
      }
      else if (!strcmp_nstr2(&in,"$TEMP"))
      {
        GetTempPath(MAX_PATH,out);
      }
      else if (!strcmp_nstr2(&in,"$USTARTMENU"))
      {
        TCHAR buf[MAX_PATH]="";
        queryShellFolders("Start Menu",buf,0);
        if (!buf[0]) return 3;
        _tcscpy(out,buf);
      }
      else if (!strcmp_nstr2(&in,"$USMPROGRAMS"))
      {
        TCHAR buf[MAX_PATH]="";
        queryShellFolders("Programs",buf,0);
        if (!buf[0]) return 3;
        _tcscpy(out,buf);
      }
      else if (!strcmp_nstr2(&in,"$USMSTARTUP"))
      {
        TCHAR buf[MAX_PATH]="";
        queryShellFolders("Startup",buf,0);
        if (!buf[0]) return 3;
        _tcscpy(out,buf);
      }
      else if (!strcmp_nstr2(&in,"$CSTARTMENU"))
      {
        TCHAR buf[MAX_PATH]="";
        queryShellFolders("Start Menu",buf,1);
        if (!buf[0]) return 3;
        _tcscpy(out,buf);
      }
      else if (!strcmp_nstr2(&in,"$CSMPROGRAMS"))
      {
        TCHAR buf[MAX_PATH]="";
        queryShellFolders("Programs",buf,1);
        if (!buf[0]) return 3;
        _tcscpy(out,buf);
      }
      else if (!strcmp_nstr2(&in,"$CSMSTARTUP"))
      {
        TCHAR buf[MAX_PATH]="";
        queryShellFolders("Startup",buf,1);
        if (!buf[0]) return 3;
        _tcscpy(out,buf);
      }
      else if (!strcmp_nstr2(&in,"$DOCOMMONINSTALL"))
      {
		if (state_docommoninstall) {
			_tcscpy(out, _T("1"));
		} else {
			_tcscpy(out, _T("0"));
		}
      }
      else if (!strcmp_nstr2(&in,"$NEEDSREBOOT"))
      {
		if (state_reboot_needed == 1) {
			_tcscpy(out, _T("1"));
		} else {
			_tcscpy(out, _T("0"));
		}
      }
      else if (!strcmp_nstr2(&in,"$AUTOINSTALL"))
      {
		if (autoinstall > 0) {
			_tcscpy(out, _T("1"));
		} else {
			_tcscpy(out, _T("0"));
		}
      }
      else if (!strcmp_nstr2(&in,"$NODESKTOPICON"))
      {
		if (nodesktopicon > 0) {
			_tcscpy(out, _T("1"));
		} else {
			_tcscpy(out, _T("0"));
		}
      }
      else if (!strcmp_nstr2(&in,"$USERPROFILE"))
      {
	/* The code below is taken from: src/win32/native/classes/java/lang/java_props_md.c
	 * It is the same code that computes the user.home property in Java
	 */
	
	/* Given user name XXX:
	 * On multi-user NT, user.home gets set to c:\winnt\profiles\XXX.
	 * On multi-user Win95, user.home gets set to c:\windows\profiles\XXX.
	 * On single-user Win95, user.home gets set to c:\windows.
	 */
        static TCHAR buf[MAX_PATH]="";
	static int initialized = 0;
	LPITEMIDLIST itemList;
	if (!initialized) {
	  initialized = 1;
	  if (SUCCEEDED(SHGetSpecialFolderLocation(NULL,
						   CSIDL_DESKTOPDIRECTORY,
						   &itemList))) {
	    TCHAR szUserPath[MAX_PATH];
	    IMalloc *pMalloc;
	    LPTSTR p;
	    SHGetPathFromIDList(itemList, szUserPath);
	    if (SUCCEEDED(SHGetMalloc(&pMalloc))) {
	      pMalloc->lpVtbl->Free(pMalloc, itemList);
	      pMalloc->lpVtbl->Release(pMalloc);
	    }
	    /* Get the parent of Desktop directory */
	    p = _tcsrchr(szUserPath, FILE_SEPARATOR);
	    if (p) {
	      *p = '\0';
	    }
	    _tcscpy(buf, szUserPath);
	  } else {
	    _tcscpy(buf, "C:");
	  }
	}
        if (!buf[0]) return 3;
        _tcscpy(out,buf);
      }
	  else if (!strcmp_nstr2(&in,"$?"))
	  {
		wsprintf(out, "%ld", state_return_value);
	  }
	  else if (!strcmp_nstr2(&in,"$1"))
	  {
		if (!args || !argsc || argsc < 2 || !args[1] || !args[1][0]) return 0;
		_tcscpy(out, args[1]);
	  }
	  else if (!strcmp_nstr2(&in,"$2"))
	  {
		if (!args || !argsc || argsc < 3 || !args[2] || !args[2][0]) return 0;
		_tcscpy(out, args[2]);
	  }
	  else if (!strcmp_nstr2(&in,"$3"))
	  {
		if (!args || !argsc || argsc < 4 || !args[3] || !args[3][0]) return 0;
		_tcscpy(out, args[3]);
	  }
      else if (!strcmp_nstr2(&in,"$QUICKLAUNCH"))
      {
        TCHAR buf[MAX_PATH]="";
        queryShellFolders("AppData",buf,0);
        if (buf[0])
        {
		      HANDLE h;
		      WIN32_FIND_DATA fd;
          _tcscat(buf,"\\Microsoft\\Internet Explorer\\Quick Launch");
    		  h=FindFirstFile(buf,&fd);
		      if (h != INVALID_HANDLE_VALUE)
		      {
			      if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) buf[0]=0;
			      FindClose(h);
    		  } 
          else buf[0]=0;
        }
        if (!buf[0])
        {
          GetTempPath(MAX_PATH,buf);
        }
        _tcscpy(out,buf);
      }
      else return 2;
      while (*out && *(CharNext(out))) out=CharNext(out);
      if (*out=='\\') *out=0;
      else out=CharNext(out);
    }
  }
  *out=0;
  return 0;
}

#ifdef NSIS_CONFIG_LOG

TCHAR log_text[2048];
int log_dolog;
void log_write(int close)
{ 
  extern TCHAR g_log_file[MAX_PATH];
  static HANDLE fp=INVALID_HANDLE_VALUE;
  if (close)
  {
    if (fp!=INVALID_HANDLE_VALUE) 
    {
      CloseHandle(fp);
      fp=INVALID_HANDLE_VALUE;
    }
    return;
  }
  if (log_dolog)
  {
    if (g_log_file[0] && fp==INVALID_HANDLE_VALUE)
    {
      fp = CreateFile(g_log_file,GENERIC_WRITE,FILE_SHARE_READ,NULL,OPEN_ALWAYS,0,NULL);
      if (fp!=INVALID_HANDLE_VALUE) 
        SetFilePointer(fp,0,NULL,FILE_END);
    }
    if (fp!=INVALID_HANDLE_VALUE)
    {
      DWORD d;
      WriteFile(fp,log_text,_tcslen(log_text),&d,NULL);
    }
  }
}


#endif

//=--------------------------------------------------------------------------=
// IsRunningOnNT4
//=--------------------------------------------------------------------------=
// Determine if the system is Windows NT 4.0
//
// Parameters:
//
// Output:
//      BOOL
//
// Notes:
//
BOOL IsRunningOnNT4(void)
{
   OSVERSIONINFOEX osvi;
   BOOL bOsVersionInfoEx;

   // Try calling GetVersionEx using the OSVERSIONINFOEX structure.
   // If that fails, try using the OSVERSIONINFO structure.

   ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
   osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

   if( !(bOsVersionInfoEx = GetVersionEx ((OSVERSIONINFO *) &osvi)) )
   {
      // If OSVERSIONINFOEX doesn't work, try OSVERSIONINFO.
      osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
      if (! GetVersionEx ( (OSVERSIONINFO *) &osvi) ) 
         return FALSE;
   }

   return ((osvi.dwPlatformId == VER_PLATFORM_WIN32_NT) &&
           (osvi.dwMajorVersion == 4));
}



//=--------------------------------------------------------------------------=
// IsRunningOn2000XP
//=--------------------------------------------------------------------------=
// Determine if the system is Windows 2000 or XP
//
// Parameters:
//
// Output:
//      BOOL
//
// Notes:
//
BOOL IsRunningOn2000XP(void)
{
   OSVERSIONINFOEX osvi;
   BOOL bOsVersionInfoEx;

   // Try calling GetVersionEx using the OSVERSIONINFOEX structure.
   // If that fails, try using the OSVERSIONINFO structure.

   ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
   osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

   if( !(bOsVersionInfoEx = GetVersionEx ((OSVERSIONINFO *) &osvi)) )
   {
      // If OSVERSIONINFOEX doesn't work, try OSVERSIONINFO.
      osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
      if (! GetVersionEx ( (OSVERSIONINFO *) &osvi) ) 
         return FALSE;
   }

   return ((osvi.dwPlatformId == VER_PLATFORM_WIN32_NT) &&
           (osvi.dwMajorVersion > 4));
}

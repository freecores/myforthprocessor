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
 * @(#)Ui.c	1.11 03/01/23
 */


#include <windows.h>
#include <shlobj.h>
#include <tchar.h>
#include <stdio.h>
#include "resource.h"
#include "shell.h"

#include "fileform.h"
#include "state.h"
#include "util.h"
#include "ui.h"
#include "libc.h"
#include "i18n.h"

static BOOL CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam); 
static BOOL CALLBACK DialogProcUninstall(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam); 
static int CALLBACK WINAPI BrowseCallbackProc( HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData);
static BOOL CALLBACK LicenseProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam); 
static BOOL CALLBACK DirProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam); 
// static BOOL CALLBACK SelProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam); 
static BOOL CALLBACK InstProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam); 
static BOOL CALLBACK UninstProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam); 
static DWORD WINAPI install_thread(LPVOID p);

extern int skip;
static HWND insthwnd;
HANDLE m_hFile;
int m_offset;
header *m_header;
section *m_section;
static entry *m_entry;
#ifdef NSIS_CONFIG_UNINSTALL_SUPPORT
uninstall_header *m_uninstheader;
#endif
static int m_page,m_abort;
static HWND m_curwnd;
static int m_whichcfg;
StringTable **g_strings = NULL;
int g_nstrings=0;
int state_docommoninstall=0;

int ui_doinstall(HANDLE hFile,int offset,header *head, section *sec, entry *ent, uninstall_header *uhead)
{
  TCHAR tmpbuf[MAX_PATH];
  int ret = 3;
  m_hFile=hFile;
  m_offset=offset;
  m_section=sec;
  m_entry=ent;
  m_header=head;

  if (!uhead) {
	  if (noisy) {
		  noisy=!(head->silent_install);
	  } else {
		  head->silent_install = !noisy;
	  }
	  /* Setup localized strings */
	  if ((g_strings = getStrings(NULL)) == NULL) {
		  GETRESOURCE(tmpbuf, JAVAWS_ERROR_STRINGTABLE);
		  if (noisy) MessageBox(NULL,tmpbuf,g_caption,MB_OK|MB_ICONWARNING);
	  }
	  _tcscpy(tmpbuf, m_header->name);
	  process_string(m_header->name, tmpbuf, NULL);
	  _tcscpy(tmpbuf, m_header->licensetext);
	  process_string(m_header->licensetext, tmpbuf, NULL);
	  _tcscpy(tmpbuf, m_header->componenttext);
	  process_string(m_header->componenttext, tmpbuf, NULL);
	  _tcscpy(tmpbuf, m_header->text);
	  process_string(m_header->text, tmpbuf, NULL);
  }

#ifdef NSIS_CONFIG_UNINSTALL_SUPPORT
  m_uninstheader=uhead;
  if (uhead)
  {
	  INT32 args[] = { (INT32)m_uninstheader->name };
	  g_strings = getStrings(NULL);
	  _tcscpy(tmpbuf, m_uninstheader->uninstalltext);
	  process_string(m_uninstheader->uninstalltext, tmpbuf, NULL);
	  GETRESOURCE2(tmpbuf, JAVAWS_MESSAGE_UNINSTALL, args);
      wsprintf(g_caption,tmpbuf);
      return DialogBox(g_hInstance,MAKEINTRESOURCE(IDD_INST),GetDesktopWindow(),DialogProcUninstall);
  }
  else
#endif
  {
    TCHAR buf[MAX_PATH];
    int st=0;

    state_install_directory[0]=0;
    if (head->install_reg_key[0])
    {
      HKEY hKey;
		  if ( RegOpenKeyEx((HKEY)head->install_reg_rootkey,head->install_reg_key,0,KEY_READ,&hKey) == ERROR_SUCCESS)
      {
			  int l = sizeof(buf);
			  int t=REG_SZ;
        if (RegQueryValueEx(hKey,head->install_reg_value,NULL,&t,buf,&l ) == ERROR_SUCCESS && t == REG_SZ && buf[0])
			  {
          LPTSTR e;
          LPTSTR p=buf;
          while (*p && *p != _T('\"')) p=CharNext(p);
          if (*p)
          {
            LPTSTR p2;
			p=CharNext(p);
			p2 = p;
            while (*p2 && *p2 != _T('\"')) p2=CharNext(p2);
            if (*p2)
            {
              *p2=0;
            }
            else p=buf;
          }
          else p=buf; 
          // p is the path now, check for .exe extension
          e=p;
          while (*e) e=CharNext(e);
          while (e>p && *e != _T('.') && *e != _T('\\')) e=CharPrev(p, e);
          if (*e ==_T('.'))
          {
            if ((e[1] == _T('e') || e[1]==_T('E')) &&
                (e[2] == _T('x') || e[2]==_T('X')) &&
                (e[3] == _T('e') || e[3]==_T('E')))        // check extension
            {
              DWORD d;
              e[4]=0;
              d=GetFileAttributes(p);               // get the file attributes
              if (d == 0xFFFFFFFFF || !(d&FILE_ATTRIBUTE_DIRECTORY)) // if not exists, or not directory, then remove suffix
              {
				while (e>p && *e != _T('\\')) e=CharPrev(p, e);
                if (*e == _T('\\')) *e=0;
              }
            }
          }


          _tcscpy(state_install_directory,buf);
        }
			  RegCloseKey(hKey);
      }
    }

    if (!state_install_directory[0]) 
    {
      _tcscpy(state_install_directory,head->install_directory);
#ifdef NSIS_CONFIG_WINAMPHACKCHECK
#warning NSIS_CONFIG_WINAMPHACKCHECK defined, this section NOT internationalized, proceed with caution!
      if (!mini_stricmp(state_install_directory,"$PROGRAMFILES\\Winamp"))
      {
        HKEY hKey;
		    if ( RegOpenKeyEx(HKEY_LOCAL_MACHINE,"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Winamp",0,KEY_READ,&hKey) == ERROR_SUCCESS)
		    {
			    int l = sizeof(buf);
			    int t=REG_SZ;
			    if (RegQueryValueEx(hKey,"UninstallString",NULL,&t,buf,&l ) == ERROR_SUCCESS && t == REG_SZ)
			    {
				    char *p=buf+mini_strlen(buf);
				    while (p >= buf && *p != '\\') p--;
				    if ( p >= buf)
				    {
					    char *i=buf;
					    *p=0;
					    while (*i == ' ' || *i == '\"') i++;
					    mini_strcpy(state_install_directory,i);
              st=1;
				    }
			    }
			    RegCloseKey(hKey);
		    }
      }
#endif

      if (!st && !process_string(buf,state_install_directory,NULL)) {
        _tcscpy(state_install_directory,buf);
	  }
    }

	{
		INT32 args[] = { (INT32)head->name };
		GETRESOURCE2(tmpbuf, JAVAWS_MESSAGE_SETUP, args);
		wsprintf(g_caption,"%s Setup",head->name);
	}
#ifdef NSIS_CONFIG_LOG
    if (head->silent_install==2) 
    {
      wsprintf(g_log_file,"%s%sinstall.log",state_install_directory,state_install_directory[mini_strlen(state_install_directory)-1]=='\\'?"":"\\");    
      log_dolog=1;
    }
#endif

    if (!head->silent_install && autoinstall != 1) {
      ret = DialogBox(g_hInstance,MAKEINTRESOURCE(IDD_INST),
		       GetDesktopWindow(),DialogProc);
      /* if ret was 3 we only showed the license agreement, */
      /* so continue as if we are doing an autoinstall and  */
      /* run the install thread without a UI.               */
      if (ret != 3) return ret;
    }
    m_section[0].default_state=0x80000000;
    return install_thread(NULL);
  }
}

static int CALLBACK WINAPI BrowseCallbackProc( HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	if (uMsg==BFFM_INITIALIZED)
	{
		TCHAR buf[MAX_PATH], tmpbuf[MAX_PATH];
		GetDlgItemText((HWND)lpData,IDC_DIR,buf,sizeof(buf));
		GETRESOURCE(tmpbuf, JAVAWS_MESSAGE_SELECTDIR);	    
        SetWindowText(hwnd,tmpbuf);
		SendMessage(hwnd,BFFM_SETSELECTION,(WPARAM)1,(LPARAM)buf);
	}
	return 0;
}


static BOOL CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static HICON hIcon;
	TCHAR tmpbuf[MAX_PATH];
  if (uMsg == WM_DESTROY && hIcon) { DeleteObject(hIcon); hIcon=0; }
  if (uMsg == WM_INITDIALOG || uMsg == WM_USER+1)
	{
    int backenabled=0,iscp=1,islp=1, x;
    struct 
    {
      LPTSTR id;
      WNDPROC proc;
      LPTSTR s;
    }
    windows[4]=
    {
      {MAKEINTRESOURCE(IDD_LICENSE),LicenseProc,""},
      {MAKEINTRESOURCE(IDD_SELCOM),LicenseProc,""},
      {MAKEINTRESOURCE(IDD_DIR),DirProc,""},
      {MAKEINTRESOURCE(IDD_INSTFILES),InstProc,""},
    };
	int messages[4] = { JAVAWS_MESSAGE_LICENSE, JAVAWS_MESSAGE_INSTOPT,
		JAVAWS_MESSAGE_INSTDIR, JAVAWS_MESSAGE_INSTFILES };
	for( x=0; x<4; x++) {
		GETRESOURCE(tmpbuf, messages[x]);
		windows[x].s = (LPTSTR)malloc((_tcslen(tmpbuf) + 1) * sizeof(TCHAR));
		_tcscpy(windows[x].s, tmpbuf);
	}
    if (uMsg == WM_INITDIALOG)
    {
      g_hwnd=hwndDlg;
		  hIcon=LoadIcon(g_hInstance,MAKEINTRESOURCE(IDI_ICON2));
		  SetClassLong(hwndDlg,GCL_HICON,(long)hIcon);
    }
    if (m_curwnd) DestroyWindow(m_curwnd);
    if (!m_header->licensetext[0] || m_header->licensedata_offset==-1) islp=0;
    if (m_page==0 && !islp) m_page++;
    if (m_page==1 && autoinstall == 2) {
      /* skip the rest of the dialog because it's an autoinstall */
      g_hwnd = NULL;
      EndDialog(hwndDlg,3);
    }

    if (m_header->num_sections < 2) iscp=0;
	{
		int id=LOWORD(wParam);
		if (uMsg==(WM_USER+1) && id==IDC_BACK) {
			if (m_page==1 && !iscp) m_page--;
		} else {
			if (m_page==1 && !iscp) m_page++;
		}
	}

    if (m_page==1&&islp) backenabled=1;
    if (m_page==2&&(islp||iscp)) backenabled=1;

    if (m_page < 0 || m_page > 3)
    {
      EndDialog(hwndDlg,0);
    }
    else
    {
		HWND hwnd;
		GETRESOURCE(tmpbuf,JAVAWS_BACK);
		SetDlgItemText(hwndDlg,IDC_BACK,tmpbuf);
		if (m_page==0) {
			GETRESOURCE(tmpbuf,JAVAWS_DECLINE);
			SetDlgItemText(hwndDlg,IDCANCEL,tmpbuf);
			GETRESOURCE(tmpbuf,JAVAWS_ACCEPT);
			SetDlgItemText(hwndDlg,IDOK,tmpbuf);
			hwnd=GetDlgItem(hwndDlg, IDC_BACK);
			ShowWindow(hwnd,SW_HIDE);
		} else {
			GETRESOURCE(tmpbuf,JAVAWS_CANCEL);
			SetDlgItemText(hwndDlg,IDCANCEL,tmpbuf);
			GETRESOURCE(tmpbuf,JAVAWS_NEXT);
			SetDlgItemText(hwndDlg,IDOK,tmpbuf);
			hwnd=GetDlgItem(hwndDlg, IDC_BACK);
			ShowWindow(hwnd,SW_SHOWNA);
		}
		{
	  INT32 args[] = { (INT32)m_header->name, (INT32)windows[m_page].s };
      m_curwnd=CreateDialog(g_hInstance,windows[m_page].id,hwndDlg,windows[m_page].proc);
	  GETRESOURCE2(tmpbuf, JAVAWS_MESSAGE_SETUP2, args);
      SetWindowText(hwndDlg,tmpbuf);
		}
    }
		if (m_curwnd) 
    {
			RECT r;
      GetWindowRect(GetDlgItem(hwndDlg,IDC_CHILDRECT),&r);
			ScreenToClient(hwndDlg,(LPPOINT)&r);
			SetWindowPos(m_curwnd,0,r.left,r.top,0,0,SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOZORDER);
			ShowWindow(m_curwnd,SW_SHOWNA);
      EnableWindow(GetDlgItem(hwndDlg,IDC_BACK),backenabled);
		} 
  }
  if (uMsg == WM_COMMAND)
  {
    int id=LOWORD(wParam);

    if (id == IDOK && m_curwnd)
    {
      m_page++;
      SendMessage(hwndDlg,WM_USER+1,wParam,0);
    }
		if (id == IDC_BACK && m_curwnd && m_page>0)
    {
      m_page--;
      SendMessage(hwndDlg,WM_USER+1,wParam,0);
    }
	  if (id == IDCANCEL )
    {
      if (m_abort)
      {
   			EndDialog(hwndDlg,2);
      }
      else
      {
   			EndDialog(hwndDlg,1);
      }
		}
	}
	return 0;
}

#ifdef NSIS_CONFIG_UNINSTALL_SUPPORT
static BOOL CALLBACK DialogProcUninstall(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static HICON hIcon;
	int x;
	TCHAR tmpbuf[MAX_PATH];
  if (uMsg == WM_DESTROY && hIcon) { DeleteObject(hIcon); hIcon=0; }
  if (uMsg == WM_INITDIALOG || uMsg == WM_USER+1)
	{
    struct 
    {
      LPTSTR id;
      WNDPROC proc;
      LPTSTR s;
    }
    windows[2]=
    {
      {MAKEINTRESOURCE(IDD_UNINST),UninstProc,""},
      {MAKEINTRESOURCE(IDD_INSTFILES),InstProc,""},
    };
	int messages[2] = { JAVAWS_MESSAGE_CONFIRM, JAVAWS_MESSAGE_UNINSTFILES };
	for (x=0; x<2; x++) {
		GETRESOURCE(tmpbuf, messages[x]);
		windows[x].s = (LPTSTR)GlobalAlloc(GMEM_FIXED, (_tcslen(tmpbuf) + 1) * sizeof(TCHAR));
		_tcscpy(windows[x].s, tmpbuf);
	}
    if (uMsg == WM_INITDIALOG)
    {
      g_hwnd=hwndDlg;
	  hIcon=LoadIcon(g_hInstance,MAKEINTRESOURCE(IDI_ICON2));
		  SetClassLong(hwndDlg,GCL_HICON,(long)hIcon);
      EnableWindow(GetDlgItem(hwndDlg,IDC_BACK),0);
    }
    if (m_curwnd) DestroyWindow(m_curwnd);
    if (m_page < 0 || m_page > 1)
    {
      EndDialog(hwndDlg,0);
    }
    else
    {
		INT32 args[] = { (INT32)m_uninstheader->name, (INT32)windows[m_page].s };
        m_curwnd=CreateDialog(g_hInstance,windows[m_page].id,hwndDlg,windows[m_page].proc);
		GETRESOURCE2(tmpbuf, JAVAWS_MESSAGE_UNINSTALL2, args);
        SetWindowText(hwndDlg,tmpbuf);
    }
		if (m_curwnd)
    {
			RECT r;
      GetWindowRect(GetDlgItem(hwndDlg,IDC_CHILDRECT),&r);
			ScreenToClient(hwndDlg,(LPPOINT)&r);
			SetWindowPos(m_curwnd,0,r.left,r.top,0,0,SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOZORDER);
			ShowWindow(m_curwnd,SW_SHOWNA);
		} 
  }
  if (uMsg == WM_COMMAND)
  {
    int id=LOWORD(wParam);

    if (id == IDOK && m_curwnd)
    {
      m_page++;
      SendMessage(hwndDlg,WM_USER+1,0,0);
    }
	  if (id == IDCANCEL)
    {
   		EndDialog(hwndDlg,2);
		}
	}
	return 0;
}
#endif




static BOOL CALLBACK LicenseProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if (uMsg == WM_INITDIALOG)
  {
    LPTSTR buf=(LPTSTR)GlobalAlloc(GMEM_FIXED,32769);
    int l=0;
    buf[0]=0;
    l=GetCompressedDataFromDataBlockToMemory(m_hFile, m_offset+m_header->licensedata_offset, buf, 32768);
    if (l >= 0)
    {
      buf[l]=0;
      SetDlgItemText(hwndDlg,IDC_EDIT1,buf);
    }
    GlobalFree((HGLOBAL)buf);
    SetDlgItemText(hwndDlg,IDC_LICTEXT,m_header->licensetext);
  }
  return 0;
}

#ifdef NSIS_CONFIG_UNINSTALL_SUPPORT
static BOOL CALLBACK UninstProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if (uMsg == WM_INITDIALOG)
  {
    SetDlgItemText(hwndDlg,IDC_UNINSTTEXT,m_uninstheader->uninstalltext);
    SetDlgItemText(hwndDlg,IDC_EDIT1,state_install_directory);
  }
  return 0;
}
#endif

BOOL CALLBACK DirProc2(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (uMsg== WM_INITDIALOG)
	{
		TCHAR str[MAX_PATH];
		TCHAR msg[128];
		TCHAR msg2[128];
		INT32 args[] = {(INT32)(m_header->name), (INT32)msg};
		INT32 args2[] ={(INT32)msg2};
		wsprintf(msg2,"%s %s", state_alt_name, state_alt_version);
		GETRESOURCE(msg, JAVAWS_MESSAGE_INSTDIR);
		GETRESOURCE2(str, JAVAWS_MESSAGE_SETUP2, args);
		SetWindowText(hwndDlg, str);
		GETRESOURCE(str, JAVAWS_MESSAGE_JRE);
		SetDlgItemText(hwndDlg,IDC_DIR,state_alt_directory);
		SetDlgItemText(hwndDlg,IDC_INTROTEXT, str);
		GETRESOURCE2(str, JAVAWS_MESSAGE_SELECTDIR2, args2);
		SetDlgItemText(hwndDlg,IDC_SELDIRTEXT,str);
		SendMessage(hwndDlg,WM_USER+3,0,0);
	}
	if (uMsg == WM_COMMAND) {
		int id=LOWORD(wParam);
		if (id == IDC_OK) {
			GetDlgItemText(hwndDlg, IDC_DIR, state_alt_directory, MAX_PATH);
			EndDialog(hwndDlg, 1);
		}
		if (id == IDC_CANCEL) {
			state_alt_directory[0]=0;
			EndDialog(hwndDlg, 0);
		}
		if (id == IDC_DIR && HIWORD(wParam) == EN_CHANGE)
		{
			SendMessage(hwndDlg,WM_USER+3,0,0);
		}
		if (id == IDC_BROWSE)
		{
			TCHAR name[MAX_PATH];
			BROWSEINFO bi={0,};
			ITEMIDLIST *idlist;
			GetDlgItemText(hwndDlg,IDC_DIR,name,MAX_PATH);
			bi.hwndOwner = hwndDlg;
			bi.pszDisplayName = name;
			bi.lpfn=BrowseCallbackProc;
			bi.lParam=(LPARAM)hwndDlg;
			bi.lpszTitle = (LPTSTR)malloc(128 * sizeof(TCHAR));
			GETRESOURCE(bi.lpszTitle, JAVAWS_MESSAGE_SELECTDIR3);
			bi.ulFlags = BIF_RETURNONLYFSDIRS;
			idlist = SHBrowseForFolder( &bi );
			if (idlist) 
			{
				LPTSTR p;
				SHGetPathFromIDList( idlist, name );		
				Shell_Free(idlist);
				
				p=name+_tcslen(name)-_tcslen(state_alt_version);
				if (p <= name || *(CharPrev(name, p))!=_T('\\') || _tcsicmp(p,state_alt_version))
				{
					if ( *(CharPrev(name, &(name[_tcslen(name)]))) != _T('\\') ) _tcscat(name,_T("\\"));
					_tcscat(name,state_alt_version);
				}
				
				SetDlgItemText(hwndDlg,IDC_DIR,name);
				uMsg = WM_USER+3;
			}
		}
	}
	if (uMsg == WM_USER+3)
	{
		EnableWindow(GetDlgItem(GetParent(hwndDlg),IDOK),1);
	}
	return 0;
}

static BOOL CALLBACK DirProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_DESTROY)
	{
		GetDlgItemText(hwndDlg,IDC_DIR,state_install_directory,sizeof(state_install_directory));
		if (IsDlgButtonChecked(hwndDlg,IDC_CHECK1)) state_docommoninstall=1;
		else state_docommoninstall=0;
#ifdef NSIS_CONFIG_LOG
		wsprintf(g_log_file,"%s%sinstall.log",state_install_directory,state_install_directory[mini_strlen(state_install_directory)-1]=='\\'?"":"\\");
		if (IsDlgButtonChecked(hwndDlg,IDC_CHECK1)) log_dolog=1;
		else log_dolog=0;
#endif
	}
	if (uMsg == WM_INITDIALOG)
	{
		TCHAR str[MAX_PATH];
		INT32 args[] = { (INT32)(m_header->name) };
#ifdef NSIS_CONFIG_LOG
		if (GetAsyncKeyState(VK_SHIFT)&0x8000) 
			ShowWindow(GetDlgItem(hwndDlg,IDC_CHECK1),SW_SHOWNA);
#endif
		if (IsCurrentUserAdministratorOrPowerUser()) {
			ShowWindow(GetDlgItem(hwndDlg, IDC_CHECK1), SW_SHOWNA);
			SendDlgItemMessage(hwndDlg, IDC_CHECK1, BM_SETCHECK, BST_CHECKED, 0);
		} else {
			ShowWindow(GetDlgItem(hwndDlg, IDC_CHECK1), SW_HIDE);
		}
		SetDlgItemText(hwndDlg,IDC_DIR,state_install_directory);
		SetDlgItemText(hwndDlg,IDC_INTROTEXT,m_header->text);
		GETRESOURCE2(str, JAVAWS_MESSAGE_SELECTDIR2, args);
		SetDlgItemText(hwndDlg,IDC_SELDIRTEXT,str);
		SendMessage(hwndDlg,WM_USER+3,0,0);
	}
	if (uMsg == WM_COMMAND)
	{
		int id=LOWORD(wParam);
		if (id == IDC_DIR && HIWORD(wParam) == EN_CHANGE)
		{
			SendMessage(hwndDlg,WM_USER+3,0,0);
		}
		if (id == IDC_BROWSE)
		{
			TCHAR name[MAX_PATH];
			BROWSEINFO bi={0,};
			ITEMIDLIST *idlist;
			GetDlgItemText(hwndDlg,IDC_DIR,name,MAX_PATH);
			bi.hwndOwner = hwndDlg;
			bi.pszDisplayName = name;
			bi.lpfn=BrowseCallbackProc;
			bi.lParam=(LPARAM)hwndDlg;
			bi.lpszTitle = (LPTSTR)malloc(128 * sizeof(TCHAR));
			GETRESOURCE(bi.lpszTitle, JAVAWS_MESSAGE_SELECTDIR3);
			bi.ulFlags = BIF_RETURNONLYFSDIRS;
			idlist = SHBrowseForFolder( &bi );
			if (idlist) 
			{
				LPTSTR p;
				SHGetPathFromIDList( idlist, name );		
				Shell_Free(idlist);
				
				p=name+_tcslen(name)-_tcslen(m_header->name);
				if (p <= name || *(CharPrev(name, p))!=_T('\\') || _tcsicmp(p,m_header->name))
				{
					if ( *(CharPrev(name, &(name[_tcslen(name)]))) != _T('\\') ) _tcscat(name,_T("\\"));
					_tcscat(name,m_header->name);
				}
				
				SetDlgItemText(hwndDlg,IDC_DIR,name);
				uMsg = WM_USER+3;
			}
		}
	}
	if (uMsg == WM_USER+3)
	{
		int x;
		int total=m_section[0].size_kb, available=-1;
		TCHAR s[MAX_PATH];
		DWORD spc,bps,fc,tc;
		LPTSTR p;
		GetDlgItemText(hwndDlg,IDC_DIR,s,MAX_PATH);
		p=s;
		if (*p == _T('\\') &&  *(CharNext(p)) == _T('\\'))
		{
			if (*(CharPrev(s, &s[_tcslen(s)]))!=_T('\\')) _tcscat(s,_T("\\"));
		}
		if (*(p=CharNext(p)) == _T(':')) { p=CharNext(p); *(CharNext(p))=0; }
		
		if (GetDiskFreeSpace(s,&spc,&bps,&fc,&tc))
		{
			DWORD r;
			r=bps*spc*(fc>>10);
			if (!r) r=(bps*spc*fc)>>10;
			if (r > 0x7fffffff) r=0x7fffffff;
			available=(int)r;
		}
		for (x = 1; x < m_header->num_sections; x ++)
		{
			if (m_section[x].name[0]==_T('-')||m_section[x].default_state&0x80000000)
				total+=m_section[x].size_kb;
		}
		{
			INT32 args[] = {(INT32)total};
			GETRESOURCE2(s, JAVAWS_MESSAGE_SPACEREQ, args);
		}
		SetDlgItemText(hwndDlg,IDC_SPACEREQUIRED,s);
		if (available != -1)
		{
			INT32 args[] = {(INT32)available};
			GETRESOURCE2(s, JAVAWS_MESSAGE_SPACEAVAIL, args);
			SetDlgItemText(hwndDlg,IDC_SPACEAVAILABLE,s);
		}
		else
			SetDlgItemText(hwndDlg,IDC_SPACEAVAILABLE,_T(""));
		
		EnableWindow(GetDlgItem(GetParent(hwndDlg),IDOK),(available < total && available != -1) ? 0 : 1);
	}
	return 0;
}

static HBITMAP hBMcheck[2];

/*
static BOOL CALLBACK SelProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  TCHAR tmpbuf[MAX_PATH];
  if (uMsg == WM_INITDIALOG)
  {
    hBMcheck[0]=LoadBitmap(g_hInstance, MAKEINTRESOURCE(IDB_BITMAP1)); 
    hBMcheck[1]=LoadBitmap(g_hInstance, MAKEINTRESOURCE(IDB_BITMAP2)); 
    SetDlgItemText(hwndDlg,IDC_INTROTEXT,m_header->componenttext);
    if (!m_header->install_types[0][0])
    {
      ShowWindow(GetDlgItem(hwndDlg,IDC_COMBO1),SW_HIDE);
      ShowWindow(GetDlgItem(hwndDlg,IDC_TEXT1),SW_HIDE);
	  GETRESOURCE(tmpbuf, JAVAWS_MESSAGE_SELECTCOMPS);
      SetDlgItemText(hwndDlg,IDC_TEXT2,tmpbuf);
    }
    else
    {
      int x;
      for (x = 0; x < 4 && m_header->install_types[x][0]; x ++)
      {
        SendDlgItemMessage(hwndDlg,IDC_COMBO1,CB_ADDSTRING,0,(LPARAM)m_header->install_types[x]);
      }
	  GETRESOURCE(tmpbuf, JAVAWS_MESSAGE_CUSTOMTYPE);
      SendDlgItemMessage(hwndDlg,IDC_COMBO1,CB_ADDSTRING,0,(LPARAM)tmpbuf);
      SendDlgItemMessage(hwndDlg,IDC_COMBO1,CB_SETCURSEL,m_whichcfg,0);
    }
    {
      int x;
      int a=1;
      if (m_section[0].name[0]) 
        SendDlgItemMessage(hwndDlg,IDC_LIST1,LB_ADDSTRING,0,(LPARAM)m_section[0].name);
      else {
		GETRESOURCE(tmpbuf, JAVAWS_MESSAGE_REQDCOMPS);
        SendDlgItemMessage(hwndDlg,IDC_LIST1,LB_ADDSTRING,0,(LPARAM)tmpbuf);
	  }
      SendDlgItemMessage(hwndDlg,IDC_LIST1,LB_SETSEL,TRUE,0);
      for (x = 1; x < m_header->num_sections; x ++)
      {
        if (m_section[x].name[0]!=_T('-'))
        {
          SendDlgItemMessage(hwndDlg,IDC_LIST1,LB_ADDSTRING,0,(LPARAM)m_section[x].name);
          if (m_section[x].default_state & (0x80000000|(1<<m_whichcfg)) 
              || !m_header->install_types[0][0])
            SendDlgItemMessage(hwndDlg,IDC_LIST1,LB_SETSEL,TRUE,a);
          a++;
        }
      }
      SendDlgItemMessage(hwndDlg,IDC_LIST1,WM_VSCROLL,SB_TOP,0);
    }
    SendMessage(hwndDlg,WM_USER+3,0,0);
  }
  if (uMsg == WM_COMMAND)
  {
    int id=LOWORD(wParam),code=HIWORD(wParam);
    if (id == IDC_LIST1 && code==LBN_SELCHANGE)
    {
      int r,l=SendDlgItemMessage(hwndDlg,IDC_COMBO1,CB_GETCOUNT,0,0)-1;
      for (r = 0; r < l; r ++)
      {
        int a=1;
        int x;
        for (x = 1; x < m_header->num_sections; x ++)
        {
          if (m_section[x].name[0]!=_T('-'))
          {
            int q=(m_section[x].default_state&(1<<r))?1:0;
            if (q != SendDlgItemMessage(hwndDlg,IDC_LIST1,LB_GETSEL,a++,0)?1:0) break;
          }
        }
        if (x == m_header->num_sections) break;
      }

      SendDlgItemMessage(hwndDlg,IDC_COMBO1,CB_SETCURSEL,r,0);
      m_whichcfg=r;
    
      uMsg=WM_USER+3;
    }
    if (id == IDC_COMBO1 && code==CBN_SELCHANGE)
    {
      int t=SendDlgItemMessage(hwndDlg,IDC_COMBO1,CB_GETCURSEL,0,0);
      if (t != CB_ERR)
      {
        m_whichcfg=t;
        if (t != SendDlgItemMessage(hwndDlg,IDC_COMBO1,CB_GETCOUNT,0,0)-1)
        {
          int x;
          int a=1;
          SendDlgItemMessage(hwndDlg,IDC_LIST1,LB_SETSEL,TRUE,0);
          for (x = 1; x < m_header->num_sections; x ++)
          {
            if (m_section[x].name[0]!=_T('-'))
            {
              SendDlgItemMessage(hwndDlg,IDC_LIST1,LB_SETSEL,(m_section[x].default_state & (1<<t))?TRUE:FALSE,a);
              a++;
            }
          }
          SendDlgItemMessage(hwndDlg,IDC_LIST1,WM_VSCROLL,SB_TOP,0);
        }
        uMsg=WM_USER+3;
      }
    }
  }
  if (uMsg == WM_DESTROY)
  {
    int x;
    int a=1;
    m_section[0].default_state|=0x80000000;
    for (x = 1; x < m_header->num_sections; x ++)
    {
      if (m_section[x].name[0]!=_T('-'))
      {
        if (SendDlgItemMessage(hwndDlg,IDC_LIST1,LB_GETSEL,a,0))
          m_section[x].default_state|=0x80000000;
        else
          m_section[x].default_state&=~0x80000000;
        a++;
      }
      else 
        m_section[x].default_state|=0x80000000;
    }  
    DeleteObject(hBMcheck[0]);
    DeleteObject(hBMcheck[1]);
  }
  if (uMsg == WM_USER+3)
  {
    int x;
    int a=1;
    int total=m_section[0].size_kb;
    TCHAR s[256];
    for (x = 1; x < m_header->num_sections; x ++)
    {
      if (m_section[x].name[0]==_T('-')||SendDlgItemMessage(hwndDlg,IDC_LIST1,LB_GETSEL,a++,0))
        total+=m_section[x].size_kb;
    }
	{
		INT32 args[] = {(INT32)total};
		GETRESOURCE2(s, JAVAWS_MESSAGE_SPACEREQ, args);
	}
    SetDlgItemText(hwndDlg,IDC_SPACEREQUIRED,s);
  }
  if (uMsg == WM_MEASUREITEM)
  {
    LPMEASUREITEMSTRUCT lpmis = (LPMEASUREITEMSTRUCT) lParam; 
    lpmis->itemHeight = 20; 
    return TRUE; 
  }
  if (uMsg == WM_CTLCOLORLISTBOX)
  {
    SetTextColor((HDC)wParam,GetNearestColor((HDC)wParam,RGB(0,0,0)));
    SetBkColor((HDC)wParam,RGB(255,255,255));
    return (int)GetStockObject(WHITE_BRUSH);
  }
  if (uMsg == WM_DRAWITEM)
  {
    LPDRAWITEMSTRUCT lpdis = (LPDRAWITEMSTRUCT) lParam; 

    if (lpdis->itemID != -1) 
    {
      if (lpdis->itemAction == ODA_SELECT || lpdis->itemAction == ODA_DRAWENTIRE)
      {
        TEXTMETRIC tm;
        int y;

        HBITMAP oldbm;
        HDC memdc;
        TCHAR tchBuffer[MAX_PATH]="";
        SendMessage(lpdis->hwndItem, LB_GETTEXT, lpdis->itemID, (LPARAM) tchBuffer); 

        GetTextMetrics(lpdis->hDC, &tm); 

        y = (lpdis->rcItem.bottom + lpdis->rcItem.top - tm.tmHeight) / 2; 

        TextOut(lpdis->hDC, 20 + 2, y, tchBuffer, _tcslen(tchBuffer)); 

        memdc=CreateCompatibleDC(lpdis->hDC);
        oldbm=SelectObject(memdc,hBMcheck[((lpdis->itemState & ODS_SELECTED) || !lpdis->itemID)?0:1]);
        BitBlt(lpdis->hDC,lpdis->rcItem.left,lpdis->rcItem.top,20,20,memdc,0,0,SRCCOPY);
        SelectObject(memdc,oldbm);
        DeleteObject(memdc);

      }
    } 
    return TRUE; 
  }
  return 0;
}
*/

void update_status_text(LPTSTR text1)
{
  if (insthwnd)
  {
    TCHAR buf[MAX_PATH];
    wsprintf(buf, _T("%s"),text1);
    SendMessage(insthwnd,LB_ADDSTRING,(WPARAM)0,(LPARAM)buf);
    SendMessage(insthwnd,WM_VSCROLL,SB_LINEDOWN,0);
  }
}

static DWORD WINAPI install_thread(LPVOID p)
{
  HWND hwndDlg=(HWND)p;
  TCHAR tmpbuf[MAX_PATH];
  int m_inst_sec=0, m_inst_whennextsec=-1, m_inst_entry=0;
  recursive_create_directory(state_install_directory);
#ifdef NSIS_CONFIG_UNINSTALL_SUPPORT
  if (m_header) 
  {
    if (m_header->uninstdata_offset != -1)
    {
      TCHAR buf[MAX_PATH*2];
      HANDLE hFile;
	  LPTSTR extra;
//      log_printf2("uninstoffs: %d\n",m_header->uninstdata_offset);
	  GETRESOURCE(tmpbuf, JAVAWS_STATUS_CREATEUNINST);
      update_status_text(tmpbuf);
	  extra = (*(CharPrev(state_install_directory,
		  &(state_install_directory[_tcslen(state_install_directory)]))) == _T('\\')) ? _T("") : _T("\\");
      wsprintf(buf, _T("%s%s%s"),state_install_directory, extra,
            m_header->uninstall_exe_name);
      hFile=CreateFile(buf,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,0,NULL);
      if (hFile != INVALID_HANDLE_VALUE)
      {
        char *filebuf;
        DWORD l;
        int ret=-666;
        filebuf=(char *)GlobalAlloc(GMEM_FIXED,g_filehdrsize);
        if (filebuf)
        {
          int tofs=m_offset+m_header->uninstdata_offset;
          int fixoffs;
          SetFilePointer(m_hFile,0,NULL,FILE_BEGIN);
          ReadFile(m_hFile,filebuf,g_filehdrsize,&l,NULL);
          fixoffs=GetIntFromDataBlock(m_hFile,tofs);
          tofs+=4;
//          log_printf2("fixoffs: %d\n",fixoffs);
          if (fixoffs > 0)
          {
            DWORD dw;
            SetFilePointer(m_hFile,tofs,NULL,FILE_BEGIN);
            ReadFile(m_hFile,filebuf+fixoffs,766-32,&dw,NULL);
            tofs+=766-32;
          }
          WriteFile(hFile,filebuf,g_filehdrsize,&l,NULL);
          GlobalFree(filebuf);
          ret=GetCompressedDataFromDataBlock(m_hFile,tofs,hFile);         
        }
        CloseHandle(hFile);
        if (ret < 0)
        {
          DeleteFile(buf);
          log_printf3("error creating uninstaller: %d, \"%s\"\n",ret,buf);
        }
        else log_printf2("created uninstaller: \"%s\"\n",buf);
      }
      else log_printf2("error creating uninstaller: \"%s\"\n",buf);
    }
  }
#endif
  while (1)
  {
    if (m_inst_whennextsec<0)
    {
      log_printf("Default Section\n");
      if (m_header) 
        m_inst_whennextsec=m_section[0].num_entries;
#ifdef NSIS_CONFIG_UNINSTALL_SUPPORT
      else m_inst_whennextsec=m_uninstheader->num_entries;
#endif
    }
      
    if (m_inst_entry==m_inst_whennextsec)
    {
      do
      {
        m_inst_sec++;
        if (!m_header || m_inst_sec>=m_header->num_sections)
        {
          if (hwndDlg)
          {
            Sleep(1000);
            SendMessage(hwndDlg,WM_USER+2,0,0);
          }
          return FALSE;
        }
        m_inst_whennextsec+=m_section[m_inst_sec].num_entries;
        if (m_section[m_inst_sec].default_state&0x80000000) 
        {
          log_printf2("Section: \"%s\"\n",m_section[m_inst_sec].name);
          if (m_section[m_inst_sec].num_entries) break; // session good.
        }
        log_printf2("Skipping Section: \"%s\"\n",m_section[m_inst_sec].name);
        m_inst_entry=m_inst_whennextsec;
      } while (m_inst_sec < m_header->num_sections);
    }

    if (ExecuteEntry(m_hFile, m_offset, m_entry+m_inst_entry) < 0)
    {
      m_abort=1;
      if (hwndDlg) SendMessage(hwndDlg,WM_USER+2,1,0);
      return TRUE;
    }
    if (hwndDlg) 
    {
      SendDlgItemMessage(hwndDlg,IDC_PROGRESS1,PBM_DELTAPOS,1,0);
      Sleep(20);
    }
    else Sleep(0);
    m_inst_entry++;
  }
  return FALSE;
}

static BOOL CALLBACK InstProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  TCHAR tmpbuf[MAX_PATH];
  if (uMsg == WM_INITDIALOG)
  {
    DWORD id;
    HWND hwnd;
#ifdef NSIS_CONFIG_UNINSTALL_SUPPORT
    int num=m_header?m_section[0].num_entries:m_uninstheader->num_entries;
#else
    int num=m_section[0].num_entries;
#endif
    int x;
    insthwnd=GetDlgItem(hwndDlg,IDC_LIST1);
    if (m_header)
    {
	  GETRESOURCE(tmpbuf, JAVAWS_MESSAGE_INSTPROG);
	  SetDlgItemText(hwndDlg,IDC_PROGRESSTEXT,tmpbuf);
      log_printf3("New install of \"%s\" to \"%s\"!\n",m_header->name,state_install_directory);
      for (x = 1; x < m_header->num_sections; x ++)
      {
        if (m_section[x].default_state&0x80000000)
          num+=m_section[x].num_entries;
      }
    }
    else {
		GETRESOURCE(tmpbuf, JAVAWS_MESSAGE_UNINSTPROGRESS);
		SetDlgItemText(hwndDlg,IDC_PROGRESSTEXT,tmpbuf);
	}
  	SendDlgItemMessage(hwndDlg,IDC_PROGRESS1,PBM_SETRANGE,0,MAKELPARAM(0,num));
	  SendDlgItemMessage(hwndDlg,IDC_PROGRESS1,PBM_SETPOS,0,0);       
    hwnd=GetParent(hwndDlg);
    EnableWindow(GetDlgItem(hwnd,IDC_BACK),0);
    EnableWindow(GetDlgItem(hwnd,IDOK),0);
    EnableWindow(GetDlgItem(hwnd,IDCANCEL),0);
    ShowWindow(GetDlgItem(hwnd,IDC_INSTVER),SW_SHOWNA);
    CloseHandle(CreateThread(NULL,0,install_thread,(LPVOID)hwndDlg,0,&id));
  }
  if (uMsg == WM_CTLCOLORLISTBOX)
  {
    SetTextColor((HDC)wParam,GetNearestColor((HDC)wParam,RGB(0,255,0)));
    SetBkColor((HDC)wParam,RGB(0,0,0));
    return (int)GetStockObject(BLACK_BRUSH);
  }
  if (uMsg == WM_USER+2)
  {
    if (!wParam)
    {
      EnableWindow(GetDlgItem(GetParent(hwndDlg),IDOK),1);
      m_page++;
      SendMessage(GetParent(hwndDlg),WM_USER+1,0,0);
    }
    else
    {
      EnableWindow(GetDlgItem(GetParent(hwndDlg),IDCANCEL),1);
    }
  }
  return 0;
}

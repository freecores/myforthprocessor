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
 * @(#)exec.c	1.18 03/01/23
 */


#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <shellapi.h>
#include <string.h>
#include "fileform.h"
#include "shell.h"
#include "state.h"
#include "ui.h"
#include "libc.h"
#include "resource.h"
#include "util.h"

#define JAVAWS_REBOOT_FILENAME "jawsboot.ini"
#define JAVAWS_APPLICATION_NAME "Java Web Start"
#define JAVAWS_REBOOT_KEY "reboot"

int skip=0;
// return 0 on success, return -1 on installer error (i.e. installer corrupted)
// return -2 on other error (cancel install process)

BOOL CALLBACK DirProc2(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	
int ExecuteEntry(HANDLE hFile, int offset, entry *thisentry)
{
  static TCHAR buf[MAX_PATH], buf2[MAX_PATH], tmpbuf[MAX_PATH];
  if (skip && thisentry->which != 15) return 0;
  switch (thisentry->which)
  {
    case 0: // set output directory
      if (!GetStringFromDataBlock(hFile,offset+thisentry->offsets[0],buf,sizeof(buf)))
      {
        if (!buf[0]) 
        {
          _tcscpy(state_output_directory,state_install_directory);
        }
        else 
        {
          if (process_string(state_output_directory,buf,state_install_directory))
            _tcscpy(state_output_directory,state_install_directory);
        }
        log_printf3("SetOutPath: \"%s\"->\"%s\"\n",buf,state_output_directory);
		{
			INT32 args[] = { (INT32)state_output_directory };
			GETRESOURCE2(tmpbuf, JAVAWS_STATUS_OUTDIR, args);
			update_status_text(tmpbuf);
		}
        recursive_create_directory(state_output_directory);
        return 0;
      }
      log_printf("SetOutPath: INSTALLER CORRUPTED\n");
    break;
    case 1: // extract file
      {
	TCHAR tempFile[MAX_PATH];
	int queueForRebootFlag = 0;
        int overwriteflag=thisentry->offsets[0];
        LPTSTR p;
        _tcscpy(buf,state_output_directory);
        p=buf; while (*p) p=CharNext(p); if (p == buf || (CharPrev(buf,p))[0]!=_T('\\')) _tcscat(buf,_T("\\"));

        if (!GetStringFromDataBlock(hFile,offset+thisentry->offsets[1],buf2,MAX_PATH))
        {
          HANDLE hOut;
          int ret;
          log_printf3("File: overwriteflag=%d, name=\"%s\"\n",overwriteflag,buf2);
          _tcscat(buf,buf2);
_tryagain:
          if ((overwriteflag&3)==0)
          {
            int attr=GetFileAttributes(buf);
            if (attr & FILE_ATTRIBUTE_READONLY)
              SetFileAttributes(buf,attr^FILE_ATTRIBUTE_READONLY);
          }
          hOut=CreateFile(buf,GENERIC_WRITE,0,NULL,(overwriteflag&1)?CREATE_NEW:CREATE_ALWAYS,0,NULL);
          if (hOut == INVALID_HANDLE_VALUE)
          {
            if (overwriteflag&3) 
            {
	      INT32 args[] = { (INT32)buf };
	      GETRESOURCE2(tmpbuf, JAVAWS_STATUS_SKIPPED, args);
	      update_status_text(tmpbuf);
              log_printf3("File: skipping: \"%s\" (overwriteflag=%d)\n",buf,overwriteflag); 
              return 0;
            }
            log_printf2("File: error creating \"%s\"\n",buf);
	    {
	      INT32 args[] = { (INT32)buf };
	      GETRESOURCE2(tmpbuf, JAVAWS_ERROR_OPENWRITE, args);
	      wsprintf(buf2, _T("%s"), tmpbuf);
            }

	    /* If we're not in silent mode then tell the user up front that */
	    /* the file is locked.  Otherwise (silent mode), queue the file */
	    /* for copy on next reboot.                                     */
	    if (noisy) {
	      switch (MessageBox(g_hwnd,buf2,g_caption,MB_ABORTRETRYIGNORE|MB_ICONSTOP))
		{
		case IDABORT:
		  log_printf("File: on error, user selected abort.\n");
		  {
		    INT32 args[]={ (INT32)buf };
		    GETRESOURCE2(tmpbuf, JAVAWS_ERROR_ABORTWRITE, args);
		    update_status_text(tmpbuf);
		  }
		  return -2;
		case IDRETRY:
		  log_printf("File: on error, user selected retry.\n"); 
		  goto _tryagain;
		case IDIGNORE:
		  log_printf("File: on error, user selected cancel.\n"); 
		  return 0;
		}
	    } else {
	      /* try to queue the file for copy on reboot */	      
	      GetTempFileName(g_tempdir, "jws", 0, tempFile);
	      hOut=CreateFile(tempFile,GENERIC_WRITE,0,NULL,(overwriteflag&1)?CREATE_NEW:CREATE_ALWAYS,0,NULL);
	      if (hOut == INVALID_HANDLE_VALUE) return 0;
	      queueForRebootFlag = 1;
	    }
          }
	  {
	    INT32 args[] = { (INT32)buf };
	    GETRESOURCE2(tmpbuf, JAVAWS_STATUS_EXTRACT, args);
            update_status_text(tmpbuf);
	  }
          ret=GetCompressedDataFromDataBlock(hFile,offset+thisentry->offsets[2],hOut);
	  
          log_printf3("File: wrote %d to \"%s\"\n",ret,buf);

          CloseHandle(hOut);

          if (ret >= 0) {
	    if (queueForRebootFlag == 1) {
	      TCHAR winini[MAX_PATH];
	      TCHAR shortname[MAX_PATH];
	      TCHAR rebootFlagFile[MAX_PATH];
	      int len;
	      _tcscpy(rebootFlagFile, g_tempdir);
	      _tcscat(rebootFlagFile, JAVAWS_REBOOT_FILENAME);

	      if (IsRunningOnNT4() || IsRunningOn2000XP()) {
		MoveFileEx(tempFile, buf, MOVEFILE_DELAY_UNTIL_REBOOT);
		MoveFileEx(rebootFlagFile, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
	      } else {
		/* queue for copy on boot; tempFile is the file to be copied */
		/* buf holds the target filename that was locked previously. */
		GetWindowsDirectory(winini, MAX_PATH);
		/* append a trailing backslash if necessary */
		len = _tcslen(winini);
		if (winini[len - 1] != '\\') {
		  winini[len] = '\\';
		  winini[len+1] = 0;
		}
		_tcscat(winini, "WININIT.INI");
		/* write to the wininit.ini file to queue the file for copy on */
		/* reboot.                                                     */
		GetShortPathName(buf, shortname, MAX_PATH);
		WritePrivateProfileString("rename", shortname, tempFile, winini);
		
		/* write the wininit.ini file to tell Windows to delete the  */
		/* reboot flag file after it reboots                         */
		WritePrivateProfileString("rename", "NUL", rebootFlagFile,
					  winini);
	      }
	      /* write the reboot flag file                                */
	      WritePrivateProfileString(JAVAWS_APPLICATION_NAME,
					JAVAWS_REBOOT_KEY,
					"1",
					rebootFlagFile);
	      state_reboot_needed = 1;
	    }
	    return ret;
	  }
          if (ret == -3 || ret == -6)
          {
            log_printf2("File: INSTALLER CORRUPTED (gcdfdb:%d)\n",ret);          
            goto installer_corrupted;
          }
          else
          {
            DeleteFile(buf);
            log_printf("File: error writing. deleted.\n");
            return -2;
          }

        }
        log_printf("File: INSTALLER CORRUPTED\n");
      }
    break;
    case 2: // execute program
      {
        PROCESS_INFORMATION ProcInfo={0,};
        STARTUPINFO StartUp={sizeof(STARTUPINFO),};
	int disabled = 0;
        if (!GetStringFromDataBlock(hFile,offset+thisentry->offsets[0],buf2,sizeof(buf2)))
        {
          log_printf2("Exec: command=\"%s\"\n",buf2);
          if (process_string(buf,buf2,state_install_directory)) _tcscpy(buf,buf2);
		  {
			  INT32 args[] = { (INT32)buf };
			  GETRESOURCE2(tmpbuf, JAVAWS_STATUS_EXECUTE, args);
			  update_status_text(tmpbuf);
		  }
		  if (IsWindowEnabled(GetDlgItem(g_hwnd,IDOK))!=0) {
		    /* if Next button is already disabled, don't disable */
                    /* and re-enable it. (bug 4624948) */
		    EnableWindow(GetDlgItem(g_hwnd,IDOK), 0);
                    disabled = 1;
                  }
          if (CreateProcess( NULL, buf, NULL, NULL, FALSE, DETACHED_PROCESS, NULL, state_output_directory, &StartUp, &ProcInfo) )
          {
            log_printf2("Exec: success (\"%s\")\n",buf);
            if (NULL != ProcInfo.hThread) CloseHandle( ProcInfo.hThread );
            if (NULL != ProcInfo.hProcess)
            {
              if (thisentry->offsets[1]==1) 
              {
                WaitForSingleObject(ProcInfo.hProcess,INFINITE);
                ShowWindow(g_hwnd,SW_HIDE);
                ShowWindow(g_hwnd,SW_SHOW);
                InvalidateRect(g_hwnd,NULL,TRUE);
                UpdateWindow(g_hwnd);
                while (1)
                {
                  MSG msg;
                  if (!PeekMessage(&msg,g_hwnd,WM_PAINT,WM_PAINT,PM_REMOVE)) break;
                  DispatchMessage(&msg);
                }      
              }
			  {
				  DWORD exitcode;
				  if (!GetExitCodeProcess(ProcInfo.hProcess, (LPDWORD)&exitcode)) {
					  log_printf("Exec: failed getting exit code\n");
					  state_return_value = STATE_RETURN_INVALID;
				  } else {
					  state_return_value = exitcode;
				  }
			  }
              CloseHandle( ProcInfo.hProcess );
            }
          }
          else { log_printf2("Exec: failed createprocess (\"%s\")\n",buf); }
	  if (disabled != 0) {
	    EnableWindow(GetDlgItem(g_hwnd,IDOK), 1);
	  }
          return 0;
        }
        log_printf("Exec: INSTALLER CORRUPTED\n"); 
      }
    break;
    case 3: // register DLL
#ifdef NSIS_SUPPORT_ACTIVEXREG
      if (!GetStringFromDataBlock(hFile,offset+thisentry->offsets[0],buf2,sizeof(buf2)))
      {
        HRESULT hr;
        hr=OleInitialize(NULL);
        if (hr == S_OK || hr == S_FALSE)
        {
          if (!process_string(buf,buf2,state_install_directory)) 
          {
            HANDLE h;
            int is_uninstall=0;
            if (buf[0] == _T('-') && buf[1] == _T(' '))
              is_uninstall=2;
            
            h=LoadLibrary(buf+is_uninstall);
            if (h)
            {
              FARPROC funke = GetProcAddress(h,is_uninstall?"DllUnregisterServer":"DllRegisterServer");
              if (funke) 
              {
                if (!is_uninstall) {
					INT32 args[] = { (INT32)buf };
					GETRESOURCE2(tmpbuf, JAVAWS_STATUS_REGDLL, args);
					update_status_text(tmpbuf);
				}
                else {
					INT32 args[] = { (INT32)buf+is_uninstall };
					GETRESOURCE2(tmpbuf, JAVAWS_STATUS_UNREGDLL, args);
					update_status_text(tmpbuf);
				}
                funke();
              }
              else if (!is_uninstall)
              {
				INT32 args[] = { (INT32)buf };
				GETRESOURCE2(buf2,JAVAWS_ERROR_REGDLL,args);
                if (noisy) MessageBox(g_hwnd,buf2,g_caption,MB_OK);
              }
              FreeModule(h);
            }
            else if (!is_uninstall)
            {
			  INT32 args[] = { (INT32)buf };
			  GETRESOURCE2(buf2,JAVAWS_ERROR_REGDLLU, args);
              if (noisy) MessageBox(g_hwnd,buf2,g_caption,MB_OK);
            }
          }
          if (hr == S_OK) OleUninitialize();
        }
        else
        {
		  INT32 args[] = { (INT32)buf };
		  GETRESOURCE2(buf2,JAVAWS_ERROR_OLEINIT,args);
          if (noisy) MessageBox(g_hwnd,buf2,g_caption,MB_OK);
        }
        return 0;
      }
#endif
    break;
    case 4: // install netscape plug-in
    case 11:
#ifdef NSIS_SUPPORT_NETSCAPEPLUGINS
#warning NSIS_SUPPORT_NETSCAPEPLUGINS defined, this section NOT internationalized, proceed with caution!
if (!GetStringFromDataBlock(hFile,offset+thisentry->offsets[0],buf2,sizeof(buf2)))
      {
        HKEY hKey;
		    if ( RegOpenKeyEx(HKEY_LOCAL_MACHINE,"SOFTWARE\\Netscape\\Netscape Navigator",0,KEY_READ,&hKey) == ERROR_SUCCESS)
		    {
          int x=0;
          while (1)
          {
            FILETIME pft;
            char name[256];
            HKEY subKey;
            DWORD lname=sizeof(name);
            if (RegEnumKeyEx(hKey,x++,name,&lname,NULL,NULL,NULL,&pft) != ERROR_SUCCESS) break;
            mini_strcat(name,"\\Main");
            if (RegOpenKeyEx(hKey,name,0,KEY_READ,&subKey) == ERROR_SUCCESS)
            {
			        int l = sizeof(buf);
			        int t=REG_SZ;
			        if (RegQueryValueEx(subKey,"Plugins Directory",NULL,&t,buf,&l ) == ERROR_SUCCESS && t == REG_SZ)
			        {
				        mini_strcat(buf,"\\");
                mini_strcat(buf,buf2);
                {
                  HANDLE hOut=INVALID_HANDLE_VALUE;
                  retryagainns:
                  {
                    int attr=GetFileAttributes(buf);
                    if (attr & FILE_ATTRIBUTE_READONLY)
                      SetFileAttributes(buf,attr^FILE_ATTRIBUTE_READONLY);
                  }
                  
                  hOut=CreateFile(buf,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,0,NULL);                  
                  if (thisentry->which != 4)
                  {
                    if (hOut != INVALID_HANDLE_VALUE)
                    {
                      CloseHandle(hOut);
                      hOut=(HANDLE)1;
                      if (!DeleteFile(buf))
                      {
                        log_printf2("InstNSPlug: error removing: %s\n",buf);
                        hOut=INVALID_HANDLE_VALUE;
                      }
                      else
                        log_printf2("InstNSPlug: removed: %s\n",buf);
                    }
                    else
                      hOut=(HANDLE)1;

                  }
                  
                  if (hOut == INVALID_HANDLE_VALUE)
                  {
                    if (MessageBox(g_hwnd,"Error accessing Netscape plug-in.\r\nMake sure all windows of Netscape are closed.\r\nHit Retry to try again, Cancel to skip",g_caption,MB_RETRYCANCEL|MB_APPLMODAL|MB_TOPMOST)==IDOK) goto retryagainns;
                    log_printf2("InstNSPlug: install to %s aborted by user.\n",buf);
                  }

                  if (hOut != INVALID_HANDLE_VALUE && thisentry->which == 4)
                  {
                    int ret=GetCompressedDataFromDataBlock(hFile,offset+thisentry->offsets[1],hOut);

                    CloseHandle(hOut);

                    if (ret<0)
                    {
                      DeleteFile(buf);
                      if (ret==-3)
                      {
                        log_printf("InstNSPlug: INSTALLER CORRUPTED (2)\n");
                        goto installer_corrupted;
                      }
                      log_printf2("InstNSPlug: couldn't write to: %s\n",buf);
                    }
                    else 
                    {
                      update_status_text("installed netscape plug-in: ",buf);
                      log_printf2("InstNSPlug: wrote: %s\n",buf);
                    }
                  }                  
                  else if (thisentry->which == 11)
                  {
                    update_status_text("removed netscape plug-in: ",buf);
                  }
                }
			        }
              RegCloseKey(subKey);
            }
          }
			    RegCloseKey(hKey);
        }
        else
        {
          log_printf("InstNSPlug: Netscape registry settings not found\n");
        }
        return 0;
      }
#endif
      log_printf("InstNSPlug: INSTALLER CORRUPTED\n");
    break;
    case 12:
      if (!GetStringFromDataBlock(hFile,offset+thisentry->offsets[1],buf,sizeof(buf)))
      {
        int rootkey=thisentry->offsets[0];
        if (thisentry->offsets[2] != -1)
        {
          HKEY hKey;
          if (RegOpenKey((HKEY)rootkey,buf,&hKey) == ERROR_SUCCESS) 
          {
            if (!GetStringFromDataBlock(hFile,offset+thisentry->offsets[2],buf2,sizeof(buf2)))
            {
              if (process_string(buf,buf2,state_install_directory)) _tcscpy(buf,buf2);
              RegDeleteValue(hKey,buf);
            }
            RegCloseKey(hKey);
          }
        }
        else
        {
          log_printf3("DeleteRegKey: %d\\%s\n",rootkey,buf);
          RegDeleteKey((HKEY)rootkey,buf);
        }
        return 0;
      }
    break;
    case 5: // write registry value
      if (!GetStringFromDataBlock(hFile,offset+thisentry->offsets[1],buf,sizeof(buf)) &&
          !GetStringFromDataBlock(hFile,offset+thisentry->offsets[2],buf2,sizeof(buf2)))
      {
        int rootkey=thisentry->offsets[0];
        int type=thisentry->offsets[4];
        HKEY hKey;
        if (RegCreateKey((HKEY)rootkey,buf,&hKey) == ERROR_SUCCESS) 
        {
          TCHAR buf4[MAX_PATH];
          if (type == 1)
          {
            if (!GetStringFromDataBlock(hFile,offset+thisentry->offsets[3],buf4,sizeof(buf4)))
            {
              TCHAR buf3[MAX_PATH];
              if (process_string(buf3,buf4,state_install_directory)) _tcscpy(buf3,buf4);
              if (process_string(buf4,buf2,state_install_directory)) _tcscpy(buf4,buf2);
              log_printf5("WriteRegStr: set %d\\%s\\%s to %s\n",rootkey,buf,buf4,buf3);
              RegSetValueEx(hKey,buf4,0,REG_SZ,buf3,_tcslen(buf3)+1);
            }
          }
          else if (type == 2)
          {
            if (process_string(buf4,buf2,state_install_directory)) _tcscpy(buf4,buf2);
            log_printf5("WriteRegDword: set %d\\%s\\%s to %d\n",rootkey,buf,buf4,thisentry->offsets[3]);
            RegSetValueEx(hKey,buf4,0,REG_DWORD,(BYTE*)&thisentry->offsets[3],4);
          }
          else if (type == 3)
          {
            BYTE data[512];
            int len=GetCompressedDataFromDataBlockToMemory(hFile, 
              offset+thisentry->offsets[3], data,sizeof(data));

            if (len >= 0)
            {
              if (process_string(buf4,buf2,state_install_directory)) _tcscpy(buf4,buf2);
              RegSetValueEx(hKey,buf4,0,REG_BINARY,data,len);
            }
            log_printf5("WriteRegBin: set %d\\%s\\%s with %d bytes\n",rootkey,buf,buf4,len);

          }
          RegCloseKey(hKey);
        }
        else { log_printf3("WriteReg: error creating key %d\\%s\n",rootkey,buf); }
        if (type > 0 && type < 4) return 0;
      }
      log_printf("WriteReg: INSTALLER CORRUPTED\n");
    break;
    case 6:
      {
        TCHAR section[MAX_PATH];
        TCHAR name[MAX_PATH];
        TCHAR t[MAX_PATH];
        TCHAR t2[MAX_PATH];
        if (!GetStringFromDataBlock(hFile,offset+thisentry->offsets[0],section,sizeof(section)) &&
            !GetStringFromDataBlock(hFile,offset+thisentry->offsets[1],name,sizeof(name)) &&
            !GetStringFromDataBlock(hFile,offset+thisentry->offsets[2],buf,sizeof(buf)) &&
            !GetStringFromDataBlock(hFile,offset+thisentry->offsets[3],buf2,sizeof(buf2)))
        {
          if (process_string(t,buf2,state_install_directory)) _tcscpy(t,buf2);
          if (process_string(buf2,buf,state_install_directory)) _tcscpy(buf2,buf);
          if (process_string(t2,name,state_install_directory)) _tcscpy(t2,name);
          if (process_string(buf,section,state_install_directory)) _tcscpy(buf,section);
          log_printf5("WriteINIStr: wrote [%s] %s=%s in %s\n",buf,t2,buf2,t);
          WritePrivateProfileString(buf,t2,buf2,t);
          return 0;
        }
        log_printf("WriteINIStr: INSTALLER CORRUPTED\n");
      }
    break;
    case 7:
#ifdef NSIS_SUPPORT_CREATESHORTCUT
      {
        TCHAR scf[MAX_PATH],iconfile[MAX_PATH], parms[MAX_PATH];
        int iindex=thisentry->offsets[4];
        if (GetStringFromDataBlock(hFile,offset+thisentry->offsets[3],buf,sizeof(buf))) break;
        if (process_string(iconfile,buf,state_install_directory)) break;
        if (GetStringFromDataBlock(hFile,offset+thisentry->offsets[2],buf,sizeof(buf))) break;
        if (process_string(parms,buf,state_install_directory)) break;
        if (GetStringFromDataBlock(hFile,offset+thisentry->offsets[1],buf,sizeof(buf))) break;
        if (process_string(buf2,buf,state_install_directory)) break;
        if (GetStringFromDataBlock(hFile,offset+thisentry->offsets[0],buf,sizeof(buf))) break;
        if (process_string(scf,buf,state_install_directory)) break;

        log_printf6("CreateShortCut: out: \"%s\", in: \"%s %s\", icon: %s,%d\n",scf,buf2,parms,iconfile,iindex); 
        CreateShortCut(g_hwnd, scf, iconfile[0]?iconfile:NULL, iindex, buf2, parms[0]?parms:NULL,state_output_directory);
		{
			INT32 args[] = { (INT32)scf };
			GETRESOURCE2(tmpbuf, JAVAWS_STATUS_CREATESHORTCUT, args);
			update_status_text(tmpbuf);
		}
        return 0;
      }
#endif
      log_printf("CreateShortCut: INSTALLER CORRUPTED\n"); 
    break;
    case 8:
      if (!GetStringFromDataBlock(hFile,offset+thisentry->offsets[0],buf,sizeof(buf)))
      {
        log_printf2("Delete: \"%s\"\n",buf); 
        if (!process_string(buf2,buf,state_install_directory))
        {
		      HANDLE h;
		      WIN32_FIND_DATA fd;
          LPTSTR p=buf;
          _tcscpy(buf,buf2);
          while (*p) p=CharNext(p);
          while (p > buf && *p != _T('\\')) p=CharPrev(buf, p);
          *p=_T('\0');
    		  h=FindFirstFile(buf2,&fd);
		      if (h != INVALID_HANDLE_VALUE)
		      {
            do
            {
			        if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) 
              {
                wsprintf(buf2,_T("%s\\%s"),buf,fd.cFileName);
                log_printf2("Delete: DeleteFile(\"%s\")\n",buf2);
				{
					INT32 args[] = { (INT32)buf2 };
					GETRESOURCE2(tmpbuf, JAVAWS_STATUS_DELETEFILE, args);
					update_status_text(tmpbuf);
				}
                DeleteFile(buf2);
              }
            } while (FindNextFile(h,&fd));
			      FindClose(h);
    		  } 
        }
        else { log_printf("Delete: error parsing parameter\n"); }
        return 0;
      }
      log_printf("Delete: INSTALLER CORRUPTED\n"); 
    break;
    case 9:
      if (!GetStringFromDataBlock(hFile,offset+thisentry->offsets[1],buf,sizeof(buf)))
      {
        int whattodo=thisentry->offsets[0];
        log_printf3("FindWindow: checking for window class: %s . whattodo=%d\n",buf,whattodo); 
        if (whattodo==2)
        {
          if (!GetStringFromDataBlock(hFile,offset+thisentry->offsets[2],buf2,sizeof(buf2)))
          {
            while (FindWindow(buf,NULL))
            {
				int r=noisy ? MessageBox(g_hwnd,buf2,g_caption,MB_ABORTRETRYIGNORE) : IDIGNORE;
              if (r == IDABORT) 
              {
                log_printf("FindWindow: user abort\n");
				{
					INT32 args[] = { (INT32)buf };
					GETRESOURCE2(tmpbuf, JAVAWS_STATUS_ABORTED, args);
					update_status_text(tmpbuf);
				}
                return -2;
              }
              if (r == IDIGNORE) 
              {
                log_printf("FindWindow: user ignore\n"); 
                break;
              }
            }
          }
          else
          {
            log_printf("FindWindow: INSTALLER CORRUPTED\n"); 
            break;
          }
        }
        else if (whattodo==1)
        {
          HWND hwnd=FindWindow(buf,NULL);
          if (hwnd) 
          {
            log_printf2("FindWindow: closing window (%s) (one-shot)\n",buf); 
            SendMessage(hwnd,WM_CLOSE,0,0);
          }
        }
        else if (whattodo==0)
        {
          HWND hwnd;
          int r=8;
          while ((hwnd=FindWindow(buf,NULL)))
          {
            SendMessage(hwnd,WM_CLOSE,0,0);
            Sleep(250);
            if (r--<0) 
            {
              break;
            }
          }
          if (!hwnd)
          {
            log_printf3("FindWindow: closed window (%s) (multi-try). %d\n",buf,r); 
          }
          else
          {
            log_printf2("FindWindow: gave up closing window (%s)\n",buf); 
          }
        }
        return 0;
      }
      log_printf("FindWindow: INSTALLER CORRUPTED\n"); 
    break;
    case 10: // MessageBox      
      if (!GetStringFromDataBlock(hFile,offset+thisentry->offsets[1],buf,sizeof(buf)))
      {
	if (!process_string(buf2, buf, state_install_directory)) {
	  log_printf3("MessageBox: %d,\"%s\"\n",thisentry->offsets[0],buf2);
	  if (noisy) {
	    DWORD ret=MessageBox(g_hwnd,buf2,g_caption,thisentry->offsets[0]);
	    if ( ret == IDNO || ret == IDCANCEL) {
	      state_return_value=0;
	    } else {
	      state_return_value=1;
	    }
	  }
	} else { log_printf("MessageBox: error parsing parameter\n"); }
	return 0;
      }
    log_printf("MessageBox: INSTALLER CORRUPTED\n"); 
    break;
   case 13: //RMDir
      if (!GetStringFromDataBlock(hFile,offset+thisentry->offsets[0],buf,sizeof(buf)))
      {
        log_printf2("RMDir: \"%s\"\n",buf); 
        if (!process_string(buf2,buf,state_install_directory))
        {
          log_printf2("RMDir: RemoveDirectory(\"%s\")\n",buf2);
		  {
			  INT32 args[] = { (INT32)buf };
			  GETRESOURCE2(tmpbuf, JAVAWS_STATUS_REMOVEDIR, args);
			  update_status_text(tmpbuf);
		  }
	  RemoveDirectory(buf2);
        }
        else { log_printf("RMDir: error parsing parameter\n"); }
        return 0;
      }
      log_printf("RMDir: INSTALLER CORRUPTED\n"); 
    break;
   case 19:
      if (!GetStringFromDataBlock(hFile,offset+thisentry->offsets[0],buf,sizeof(buf)))
      {
        log_printf2("RMDirRecursive: \"%s\"\n",buf); 
        if (!process_string(buf2,buf,state_install_directory))
        {
		  SHFILEOPSTRUCT fos;
          log_printf2("RMDirRecursive: RemoveDirectory(\"%s\")\n",buf2);
		  {
			  INT32 args[] = { (INT32)buf };
			  GETRESOURCE2(tmpbuf, JAVAWS_STATUS_REMOVEDIR, args);
			  update_status_text(tmpbuf);
		  }
		  buf2[_tcslen(buf2)]=0;
		  buf2[_tcslen(buf2)+1]=0;
		  if (GetFileAttributes(buf2)!=0xffffffff) {
		    // Only do SHFileOperation if file exists
		    fos.hwnd = g_hwnd;
		    fos.wFunc = FO_DELETE;
		    fos.pFrom = buf2;
		    fos.pTo = NULL;
		    fos.fFlags = FOF_SILENT | FOF_NOERRORUI | FOF_NOCONFIRMATION;
		    fos.hNameMappings = NULL;
		    fos.lpszProgressTitle = NULL;
		    SHFileOperation(&fos);
		  }
        }
        else { log_printf("RMDirRecursive: error parsing parameter\n"); }
        return 0;
      }
      log_printf("RMDirRecursive: INSTALLER CORRUPTED\n"); 
    break;
   case 14: // If
      if (!GetStringFromDataBlock(hFile,offset+thisentry->offsets[0],buf,sizeof(buf)))
      {
        log_printf2("If: \"%s\"\n",buf); 
        if (!process_string(buf2,buf,state_install_directory))
        {
		  int  ret=0;
          log_printf2("If(variables evaluated): (\"%s\")\n",buf2);
		  if (!(ret=evalStr(buf2))) {
			  skip=1;
			  return 0;
		  }
		}
        else { log_printf("If: error parsing parameter\n"); }
        return 0;
      }
      log_printf("If: INSTALLER CORRUPTED\n"); 
    break;
   case 15: //EndIf
	   skip=0;
	   return 0;
	break;
   case 16: //RebootBox
	   if (noisy) {
			GETRESOURCE(buf,JAVAWS_MESSAGE_REBOOT);
			if (MessageBox(g_hwnd,buf,g_caption,MB_YESNO|MB_ICONWARNING)==IDYES) {
				if (!ExitWindowsEx(EWX_REBOOT, (DWORD)0)) {
					GETRESOURCE(buf,JAVAWS_ERROR_EXITWINDOWS);
					if (noisy) MessageBox(g_hwnd,buf,g_caption,MB_OK);
				}

			}
	   }
	   return 0;
	break;
   case 17: //Browser
	  if (!noisy) return 0;
      if (!GetStringFromDataBlock(hFile,offset+thisentry->offsets[0],buf,sizeof(buf)))
      {
        log_printf2("Browser: \"%s\"\n",buf); 
        if (!process_string(buf2,buf,state_install_directory))
        {	
	  ShellExecute(g_hwnd, "open", buf2, "-new", state_install_directory, 0);
	}
        else { log_printf("Browser: error parsing parameter\n"); }
        return 0;
      }
      log_printf("Browser: INSTALLER CORRUPTED\n"); 
    break;
   case 18:  // AltDirBox
	   {
		   TCHAR name[MAX_PATH];
		   TCHAR buf3[MAX_PATH];
		   TCHAR buf4[MAX_PATH];
		   if (!GetStringFromDataBlock(hFile,offset+thisentry->offsets[0],buf,sizeof(buf)) &&
			   !GetStringFromDataBlock(hFile,offset+thisentry->offsets[1],name,sizeof(name)) &&
			   !GetStringFromDataBlock(hFile,offset+thisentry->offsets[2],buf3,sizeof(buf3)))
		   {
			   if (!process_string(tmpbuf,buf,state_install_directory) &&
				   !process_string(buf4,buf3,state_install_directory) &&
				   !process_string(buf2,name,state_install_directory))
			   {
				   _tcscpy(state_alt_directory,tmpbuf);
				   _tcscpy(state_alt_name, buf2);
				   _tcscpy(state_alt_version,buf4);
				   state_return_value = noisy ? DialogBox(g_hInstance,MAKEINTRESOURCE(IDD_DIR1),g_hwnd,DirProc2): 1;
			   }
			   return 0;
		   }
	   }
	break;
   case 41:
	   if (!GetStringFromDataBlock(hFile,offset+thisentry->offsets[0],buf,sizeof(buf)))
      {
        log_printf2("MKDir: \"%s\"\n",buf); 
        if (!process_string(buf2,buf,state_install_directory))
        {
          log_printf2("MKDir: recursive_create_directory(\"%s\")\n",buf2);
		  {
			  INT32 args[] = { (INT32)buf };
			  GETRESOURCE2(tmpbuf, JAVAWS_STATUS_MAKEDIR, args);
			  update_status_text(tmpbuf);
		  }
          recursive_create_directory(buf2);
        }
        else { log_printf("RMDir: error parsing parameter\n"); }
        return 0;
      }
      log_printf("MKDir: INSTALLER CORRUPTED\n"); 
    break;
  }
installer_corrupted:
  {
     GETRESOURCE(tmpbuf, JAVAWS_ERROR_INSTCORRUPT);
     if (noisy) MessageBox(g_hwnd,tmpbuf,g_caption,MB_OK|MB_ICONSTOP);
  }
  return -1;
}

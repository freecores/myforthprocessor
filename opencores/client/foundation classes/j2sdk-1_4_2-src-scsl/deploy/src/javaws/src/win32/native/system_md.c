/*
 * @(#)system_md.c	1.25 03/05/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/* Implementation of win32 specific OS layer */

#include "util.h"
#include "system.h"
#include "locale_str.h"
#include "configurationFile.h"

#define PIPE_SIZE 512

/* 
 * A pointer to the WSASendDisconnect winsock2 function.  If winsock2
 * isn't available, this function pointer will remain NULL.  See
 * sysCloseSocket().
 */
static int (PASCAL FAR *sendDisconnect)() = NULL;

static char *sysSplashExtension = "jpg";

/* 
 * Print an (obscure) error message to stderr and exit.  If errno has 
 * been set or there was a winsock error, then print the corresponding
 * error messages too. For an explanation of the WinSock error codes, see:
 *   metalab.unc.edu/pub/micro/pc-stuff/ms-windows/winsock/winsock-1.1/winsock.html#ErrorCodes
 */
void sysErrorExit(char *msg) 
{
#ifndef WIN_CONSOLE

  MessageBox(NULL, msg, JAVAWS_NAME,
          MB_OK | MB_ICONERROR | MB_SYSTEMMODAL | MB_DEFAULT_DESKTOP_ONLY);
#endif
  
  fprintf(stderr, getMsgString(MSG_SPLASH_EXIT));
  if (errno != 0) {
      perror(msg);
  }
  else {
      fprintf(stderr, "\t%s\n", msg);
  }
  fprintf(stderr, "%s %d\n", getMsgString(MSG_WSA_ERROR), WSAGetLastError());
  exit(-1);
}

/*
 * Prints out a message. This is handy to debug windows code, since they
 * do not have a console
 *
 */
void sysMessage(char *msg) {
#ifdef WIN_CONSOLE
    printf(JAVAWS_NAME " %s\n", msg); 
#else
    MessageBox(NULL, msg, JAVAWS_NAME,
               MB_OK | MB_ICONERROR | MB_SYSTEMMODAL | MB_DEFAULT_DESKTOP_ONLY);
#endif
}


/* 
 * Initialize the socket library.   This function is called by all of the 
 * other functions in this file.
 * 
 * Note that initialization is only required on Win32.
 * 
 * Win32: try to load the winsock2 dll, and if that fails, load winsock.
 * This is really only neccesary on Windows95 as winsock2 is always
 * available on NT.  Once the dll is available, it's initalized with 
 * WSAStartup.
 */
void sysInitSocketLibrary()
{
    static int initialized = 0;

    if (!initialized) {
	HANDLE hWinsock;
	WORD wVersionRequested = MAKEWORD(2, 0);
	WSADATA wsaData;
	int winsock2Available = 1;

	hWinsock = LoadLibrary("ws2_32.dll");
	if (hWinsock == NULL) {
	    hWinsock = LoadLibrary("wsock32.dll");
	    winsock2Available = 0;
	}

	if (hWinsock == NULL) {
	    sysErrorExit(getMsgString(MSG_WSA_LOAD));
	}

	if (WSAStartup(wVersionRequested, &wsaData) != 0) {
	    sysErrorExit(getMsgString(MSG_WSA_START));
	}

	if (winsock2Available) {
	    sendDisconnect = (int (PASCAL FAR *)())GetProcAddress(hWinsock, "WSASendDisconnect");
	}

	initialized = 1;
    }
}



/* 
 * Safely close a socket.  The following note comes from socket_md.c 
 * which is in src/win32/hpi/src/ in the JDK sources.
 * 
 *   Generally speaking, it is important to shutdown a socket before
 *   closing it, since failing to do so can sometimes result in a TCP
 *   RST (abortive close) which is disturbing to the peer of the
 *   connection.
 *   
 *   The Winsock way to shutdown a socket is the Berkeley call
 *   shutdown(). We do not want to call it on Win95, since it
 *   sporadically leads to an OS crash in IFS_MGR.VXD.  Complete hull
 *   breach.  Blue screen.  Ugly.
 *
 * So we use the WSASendDisconnect function if it was found at initialization
 * time, before calling closesocket.
 */  
void sysCloseSocket(SOCKET s) 
{
    sysInitSocketLibrary();

    if (s <= 0) {
	return;
    }

    if (sendDisconnect) {
        sendDisconnect(s, NULL); /* WSASendDisconnect */
    }
    closesocket(s);
}


/* 
 * Fork a subprocess and exec the specified application.  Remember
 * that argv[0] must be the 'name' of the application, conventionally
 * it's the same as the executable filename, and the last argv element
 * must be NULL.  Here's a Unix example:
 * 
 *   char *argv[] = {"ls", "/etc", "/tmp", NULL};
 *   int pid = sysExec("/usr/bin/ls", argv);
 */

int sysExec(int type, char *path, char *argv[]) 
{
    int pid;
    char **argvp;

    /* Win32 inexplicably breaks up arguments that contain white space,
     * so we quote them here.
     */
    for(argvp = argv; *argvp; argvp++) {
        *argvp = sysQuoteString(*argvp);
    }
    if (type == SYS_EXEC_REPLACE) {
        pid = spawnv(_P_NOWAIT, path, argv);
        /* The window API does not support replacing the existing process.
           However, we can simulate this by quitly existing this application */
        exit(0);
    }
    else if (type == SYS_EXEC_FORK) {
        pid = spawnv(_P_NOWAIT, path, argv);
    }
    else {
        spawnv(_P_WAIT, path, argv);
        /* WAIT return value is exit status of process, which we don't
           care about. */
        pid = 0;
    }
    return pid;
}

char* sysGetOsName(void) {
  return "Windows";
}

char* sysGetOsArch(void) {
  return "x86";
}

char* sysGetSplashExtension(void) {
  return sysSplashExtension;
}


/*
 * If app is "c:\j2se1.4.2\javaws\javaws.exe",
 *    then put "c:\j2se1.4.2\javaws" into buf.
 */
char* sysGetApplicationHome(void) {
  static char home[MAXPATHLEN];
  static int  initialized = FALSE;

  if (!initialized) {  
    char *env = getenv("JAVAWS_HOME");
    if (env != NULL && strlen(env) != 0) {
      strcpy(home, env);
      if (home[strlen(home)-1] == FILE_SEPARATOR) {
        home[strlen(home)-1] = '\0';
      }
    } else {
      GetModuleFileName(0, home, sizeof(home));
      *strrchr(home, FILE_SEPARATOR) = '\0'; /* remove .exe file name */
    }
    initialized = TRUE;
  }
  return home;
}

char *sysGetInstallJREHome(void) {
   static char jreHome[MAXPATHLEN];
   static int  initialized = FALSE; 
   char *ptr;
   
   if (!initialized) {  
     strcpy(jreHome, sysGetApplicationHome()); 
     ptr = strrchr(jreHome, FILE_SEPARATOR); 
     strcpy(ptr, "\0");
   }
   
   return jreHome;
}

/* 
 * If app is "c:\j2se1.4.2\javaws\javaws.exe",
 *    then put "c:\j2se1.4.2\bin\javaw.exe" into buf. (if it exists) 
 */
char* sysGetInstallJRE(void) { 
    struct stat statBuf;
    static char jre[MAXPATHLEN];
    char nativeStr[100];
    static int  initialized = FALSE; 
    char *ptr;
      
    if (!initialized) {  
        strcpy(jre, sysGetApplicationHome()); 
        ptr = strrchr(jre, FILE_SEPARATOR); 
	sprintf(nativeStr, "%s%c%s", "bin", FILE_SEPARATOR, "javaw.exe");
        strcpy(ptr+1, nativeStr);
    }   
    if (stat(jre, &statBuf) == 0) {  
        return jre;
    }
    return NULL; 
} 

char* sysGetUserHome(void) {
  static char userhome[MAXPATHLEN];
  static int  initialized = FALSE;
  LPITEMIDLIST itemList;

  if (!initialized) {  
    initialized = TRUE;   

    /* The code below is taken from: src/win32/native/classes/java/lang/java_props_md.c
     * It is the same code that computes the user.home property in Java
     */
  
    /* Given user name XXX:
     * On multi-user NT, user.home gets set to c:\winnt\profiles\XXX.
     * On multi-user Win95, user.home gets set to c:\windows\profiles\XXX.
     * On single-user Win95, user.home gets set to c:\windows.
     */
  
    if (SUCCEEDED(SHGetSpecialFolderLocation(NULL,
                                           CSIDL_DESKTOPDIRECTORY,
                                           &itemList))) {
      char szUserPath[MAX_PATH];
      IMalloc *pMalloc;
      char *p;
      SHGetPathFromIDList(itemList, szUserPath);
       if (SUCCEEDED(SHGetMalloc(&pMalloc))) {
         pMalloc->lpVtbl->Free(pMalloc, itemList);
         pMalloc->lpVtbl->Release(pMalloc);
       }
       /* Get the parent of Desktop directory */
       p = strrchr(szUserPath, FILE_SEPARATOR);
       if (p) {
         *p = '\0';
       }
       strcpy(userhome, szUserPath);
    } else {
       strcpy(userhome, "C:");
    }
  }
  return userhome;
}

/*  Returns a tempoary filename. The string returned must be
 *  freed by the caller
 */
char* sysTempnam() {
  char* argFileName = _tempnam(NULL, "javaws");
  int fd = -1;
  while (argFileName != NULL) {
      fd = _open(argFileName, _O_CREAT|_O_EXCL, _S_IREAD|_S_IWRITE);
      if (fd != -1 || errno != EEXIST) { 
          break;
      } 
      argFileName = _tempnam(NULL, "javaws");
   }
  if (fd != -1 ) _close(fd);
  argFileName = strdup(argFileName);
  return argFileName;
}


/*
 *  case insensitive compare of two string
 */
int sysStrCaseCmp(char* s1, char* s2) {
    return stricmp(s1, s2);
}

char *sysGetJavawsbin() {
#ifdef _DEBUG
    return "javaws_g";
#else
    return "javaws";
#endif
}


static char _localeStr[64];
static int locale_initialized = FALSE;
char *sysGetLocaleStr() {
    LCID lcid;
    int i;
    int len = sizeof(localeIDMap) / sizeof(LCIDtoLocale);

    if (!locale_initialized) {
        lcid = GetThreadLocale();
        /* first look for whole thing */
        for (i=0; i< len; i++) {
	    if (lcid == localeIDMap[i].winID) {
	        break;
	    }   
        }
        if (i == len) {
            lcid &= 0xff;      /* look for just language */
            for (i=0; i<len; i++) {
                if (lcid == localeIDMap[i].winID) {
                    break;
                }
            }    
        }
        if (i < len) {
            strncpy(_localeStr, localeIDMap[i].javaID, 64);
        } else {
            strcpy(_localeStr, "en_US");
        }
        if (sysStrCaseCmp(_localeStr, "C") == 0) {
            strcpy(_localeStr, "en_US");
        } 
    }
    return _localeStr;
}

char *sysWideCharToMBCS(twchar_t *message, size_t len) {
  char *p = NULL;
  p=(char *)GlobalAlloc(GMEM_FIXED, MAX_PATH);
  WideCharToMultiByte(CP_ACP,
		      0,
		      message,
		      -1,
		      p,
		      MAX_PATH,
		      NULL,
		      NULL);
  return p;
}

char* sysMBCSToSeqUnicode(char *mbcs) {
  char * output;
  unsigned short *bufU = (unsigned short*)malloc(sizeof(unsigned short)*MAX_PATH);
  MultiByteToWideChar(CP_ACP, 0, mbcs, -1, bufU, MAX_PATH);
  /* now convert it to sequenced uni-code for java properties file */
  output = sysSaveConvert(bufU);
  free(bufU);
  return output;
}

void sysExec2Buf(char *cmd, int argc, char *argv[], char *buf, int *buflen) {
  HANDLE hread[2], hwrite[2];
  SECURITY_ATTRIBUTES sa;
  PROCESS_INFORMATION pi;
  STARTUPINFO si;
  BOOL ret = FALSE;
  int len, ofs = 0, i;
  char cmdline[MAXPATHLEN];
  
  len = *buflen;
  *buflen = 0;
  sa.nLength = sizeof(sa);
  sa.lpSecurityDescriptor = 0;
  sa.bInheritHandle = TRUE;
  
  memset(hread, 0, sizeof(hread));
  memset(hwrite, 0, sizeof(hwrite));
  if (!(CreatePipe(&hread[0], &hwrite[0], &sa, PIPE_SIZE) &&
	CreatePipe(&hread[1], &hwrite[1], &sa, PIPE_SIZE))) {
    CloseHandle(hread[0]);
    CloseHandle(hread[1]);
    CloseHandle(hwrite[0]);
    CloseHandle(hwrite[1]);
    return;
  }

  memset(&si, 0, sizeof(si));
  si.cb = sizeof(si);
  si.dwFlags = STARTF_USESTDHANDLES;
  si.hStdInput  = hread[0];
  si.hStdOutput = hwrite[1];
  si.hStdError  = hwrite[1];
  
  SetHandleInformation(hwrite[0], HANDLE_FLAG_INHERIT, FALSE);
  SetHandleInformation(hread[1],  HANDLE_FLAG_INHERIT, FALSE);
  
  _tcscpy(cmdline, "\"");
  _tcscat(cmdline, argv[0]);
  _tcscat(cmdline, "\"");
  for (i = 1; i < argc; i++) {
    _tcscat(cmdline, " ");
    _tcscat(cmdline, argv[i]);
  } 

  ret = CreateProcess(NULL,             /* executable name */
		      cmdline,          /* command line */
		      NULL,             /* process security attribute */
		      NULL,             /* thread security attribute */
		      TRUE,             /* inherits system handles */
		      0,                /* creation flags */
		      NULL,             /* environment block */
		      NULL,             /* current directory */
		      &si,              /* (in)  startup information */
		      &pi);             /* (out) process information */
  
  if (ret != FALSE && pi.hProcess != NULL) {
    WaitForSingleObject(pi.hProcess, INFINITE);
  }

  CloseHandle(hread[0]);
  CloseHandle(hwrite[1]);
  
  if (!ret) {
    CloseHandle(hwrite[0]);
    CloseHandle(hread[1]);
    return;
  }
  
  CloseHandle(pi.hThread);
  CloseHandle(pi.hProcess);
  
  ReadFile(hread[1], buf, len, buflen, NULL);
  buf[*buflen] = 0;

  CloseHandle(hwrite[0]);
  CloseHandle(hread[1]);
}

void sysGetRegistryJREs (JREDescription jre_list[], int *nJREs) {
  HKEY hKey;
  DWORD cbData=MAX_PATH;

  *nJREs = 0;
  /* get JRE Paths only.  common native code will figure out the versions */
  if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, JRE_VERSION_REG_KEY,
		   0, KEY_READ, &hKey) == ERROR_SUCCESS) {
    DWORD dwIndex = 0, cbSubKey = MAX_PATH;
    char szSubKey[MAX_PATH];
    HRESULT result;

    while ((result = RegEnumKeyEx(hKey, dwIndex, szSubKey, &cbSubKey,
				  NULL, NULL, NULL, NULL)) == ERROR_SUCCESS) {
      /* only look at product version keys */
      char szTemp[MAX_PATH];
      DWORD cbDataJRE = MAX_PATH;
      HKEY hKeyJRE;
      int dotcount = 0;
      unsigned int i;
      for (i = 0; i < cbSubKey; i++) {
	if (szSubKey[i] == '.') {
	  if (++dotcount > 1) break;
	}
      }
      if (dotcount > 1 || strncmp(szSubKey, "1.2", 3)==0) {
      
	wsprintf(szTemp, "%s\\%s", JRE_VERSION_REG_KEY, szSubKey);
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, szTemp,
			 0, KEY_READ, &hKeyJRE) == ERROR_SUCCESS) {
	  DWORD dwType;
	  jre_list[*nJREs].path = (char *)malloc(MAX_PATH);
	  if (RegQueryValueEx(hKeyJRE, JAVAHOMEVALUE, NULL, &dwType,
			      (unsigned char *)jre_list[*nJREs].path,
			      &cbDataJRE) == ERROR_SUCCESS) {
	    _tcscat(jre_list[*nJREs].path, "\\bin\\javaw.exe");
	    (*nJREs)++;
	  } else {
	    free(jre_list[*nJREs].path);
	  }
	  RegCloseKey(hKeyJRE);
	}
      }
      cbSubKey = MAX_PATH;
      dwIndex++;
    }
    RegCloseKey(hKey);
  }
}

void sysCreateDirectory(char *dir) {
  CreateDirectory(dir, NULL);
}

typedef HRESULT (WINAPI *LPFNSHGetFolderPath)(HWND hwndOwner, 
	int nFolder, HANDLE hToken, DWORD dwFlags, LPTSTR pszPath);

typedef BOOL (WINAPI *LPFNSHGetSpecialFolderPath)(HWND hwndOwner, 
	LPTSTR lpszPath, int nFolder, BOOL fCreate);


static char* getSystemHome(void) {
    static char systemHome[MAXPATHLEN];
    LPTSTR lpszSystemHome;

    lpszSystemHome = systemHome;
    GetWindowsDirectory(lpszSystemHome, MAXPATHLEN);

    return systemHome;
}

static char* getUserHome(void)
{
    HMODULE hModule = NULL;
    static char userHome[MAXPATHLEN];
    LPTSTR lpszUserInfo;
    LPFNSHGetFolderPath lpfnSHGetFolderPath;
    LPFNSHGetSpecialFolderPath lpfnSHGetSpecialFolderPath;
  
    lpszUserInfo = userHome;
    __try {
        hModule = LoadLibrary("shfolder.dll");
        // use SHGetFolderPath if avaliable
        if (hModule != NULL) {
            // SHGetFolderPath can work everywhere SHFOLDER is installed.
            lpfnSHGetFolderPath = (LPFNSHGetFolderPath)
				GetProcAddress(hModule, "SHGetFolderPathA");

            if (lpfnSHGetFolderPath != NULL) {
                lpfnSHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, lpszUserInfo);
            }
        } else {        
            hModule = LoadLibrary("shell32.dll");
            // SHGetSpecialFolderPath may also work
            lpfnSHGetSpecialFolderPath = (LPFNSHGetSpecialFolderPath) 
		GetProcAddress(hModule, "SHGetSpecialFolderPathA");
        
            if (lpfnSHGetSpecialFolderPath != NULL) {
                lpfnSHGetSpecialFolderPath(NULL, lpszUserInfo, 
					   CSIDL_APPDATA, TRUE);
            }
        }
    } __finally {
      if (hModule != NULL)
      FreeLibrary(hModule);
    }

    return userHome;
}

char *sysGetDeploymentUserHome() {
    static char deploymentUserHome[MAXPATHLEN];
    static int  initialized = FALSE;
    if (!initialized) {
        sprintf(deploymentUserHome, "%s\\Sun\\Java\\Deployment",
                                                getUserHome());
        initialized = TRUE;
    }
    return deploymentUserHome;
}
 
char *sysGetDeploymentSystemHome() {
    static char deploymentSystemHome[MAXPATHLEN];
    static int  initialized = FALSE;
    if (!initialized) {
        sprintf(deploymentSystemHome, "%s\\Sun\\Java\\Deployment",
                                                getSystemHome());
        initialized = TRUE;
    }
    return deploymentSystemHome;
}


/* returns the registry value; -1 if the registry value does
   not exist */
int sysGetRegistryValue(HKEY hKey, char* subKey, char* value) {
  HKEY hkResult;
  DWORD dwType;
  DWORD cbData=sizeof(DWORD);
  DWORD data = -1;   /* default registry key does not exist */
  
  if (RegOpenKeyEx(hKey, subKey,
		   0, KEY_READ, &hkResult) == ERROR_SUCCESS) {
    
    RegQueryValueEx(hkResult, value, NULL, &dwType,
		    (unsigned char *)&data, &cbData);
  }
  
  return data;
}

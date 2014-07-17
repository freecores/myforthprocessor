/*
 * @(#)main.cpp	1.9 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <process.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "shared.h"

#define ERROR_ARGS -1
#define ERROR_FORK -2

int debug = 0;

extern "C" LONG CALLBACK ViseEntry(LONG, LPCSTR , DWORD);

void debugf(char *str) {
  if (debug && str) {
    fprintf(stderr, "%s", str);
  }
}

char *installJREIfNecessary(char *, char *, char *);
void removeNetscapeRegistryEntries();
char *isJREInstalled(char *, BOOL);
void execAutoRunURL(char *);

int main(int argc, char *argv[]) {
  int fn;

  if (getenv("JAVAWS_INSTALLER_DEBUG") != NULL) {
    debug = 1;
  }

  if (argc < 2) {
    debugf("argc too small, exiting\n");
    exit(ERROR_ARGS);
  }

  fn = atoi(argv[1]);
  if (fn <= 0) {
    debugf("fn too small, exiting\n");
    exit(ERROR_ARGS);
  }
  
  if (fn <= 5) {
    return (int)ViseEntry(fn - 1, argv[2], 0);
  } else if (fn == 6) {
    return (int)installJREIfNecessary(argv[2], argv[3], argv[4]);
  } else if (fn == 7) {
    removeNetscapeRegistryEntries();
    return 1;
  } else if (fn == 8) {
    return (isJREInstalled(argv[2], FALSE) == NULL ? 0 : 1);
  } else {
    exit(ERROR_ARGS);
  }

}

char *installJRE(char *, char *);
int changeISSFile(const char *issFilePath, const char *jrePath);

char *installJREIfNecessary(char *jrePath, char *jreVersion, char *installPath) {
  char *jreInstalledPath;
  char envPath[MAX_PATH];
  char *fooPath;
  FILE *fp;
  printf("Installing JRE v%s...", jreVersion);
  if ((jreInstalledPath = isJREInstalled(jreVersion, TRUE)) == NULL) {
    jreInstalledPath = installJRE(jrePath, installPath);
    printf("done.\n");
  } else {
    printf("JRE already installed, done.\n");
  }
  _sleep(1000);
  
  fooPath = strcat(strdup(installPath), "\\foo.ini");
  
  if ((fp = fopen(fooPath, "w")) != NULL) {
    fprintf(fp, "[foo]\n");
    fprintf(fp, "jrePath=%s", jreInstalledPath);
    fclose(fp);
  }
  return jreInstalledPath;
}

char *installJRE(char *jrePath, char *installPath) {
  int  tempPathLength;
  char tempPath[MAX_PATH];
  int pid;
  char exePath[MAX_PATH];
  char issPath[MAX_PATH];
  char newJREPath[MAX_PATH];
  char *args[5];

  strcpy(tempPath, installPath);
  tempPathLength = strlen(installPath);
  if (tempPathLength > 0 && tempPath[tempPathLength - 1] == '\\') {
    tempPathLength--;
    strcat(tempPath, "jre.iss");
  }
  else {
    strcat(tempPath, "\\jre.iss");
  }
  strcpy(issPath, tempPath);
  debugf("adding jrePath ");  debugf(jrePath); debugf(" to "); debugf(issPath);
  debugf(".... ");
  strcpy(newJREPath, "\"");
  strcat(newJREPath, jrePath);
  strcat(newJREPath, "\"");
  if (changeISSFile(issPath, newJREPath) != 0) {
    debugf("failed\n");
    return NULL;
  }
  debugf("successful\n");

  strcpy(exePath, installPath);
  tempPathLength = strlen(exePath);
  if (exePath[tempPathLength - 1] != '\\') {
    exePath[tempPathLength] = '\\';
    exePath[tempPathLength + 1] = '\0';
  }
  strcat(exePath, "jre.exe");
  strcpy(tempPath, "-f1");
  strcat(tempPath, "\"");
  strcat(tempPath, issPath);
  strcat(tempPath, "2");
  strcat(tempPath, "\"");
  fflush(stderr);
  args[0] = strdup("-s");
  args[1] = strdup("-a");
  args[2] = strdup("-s");
  args[3] = strdup(tempPath);
  args[4] = NULL;
  {
    char printer[MAX_PATH];
    sprintf(printer, "Executing: %s %s %s %s %s\n", exePath, args[0], args[1], args[2], args[3]);
    debugf(printer);
    fflush(stderr);
  }
  if (_spawnv(_P_WAIT, strdup(exePath), args) < 0) {
    return NULL;
  }
  return strdup(jrePath);
}

void removeNetscapeRegistryEntries() {
    HKEY    hKey;

    if (RegOpenKeyEx(HKEY_CURRENT_USER, NETSCAPE_MIME_REG_KEY, 0,
                     KEY_READ, &hKey) == ERROR_SUCCESS) {
        HKEY    subKey;
        DWORD   dwType;
        TCHAR  value[MAX_PATH];
        char    typeString[24];
        DWORD   cbData=MAX_PATH;
        int     nType = 0;
        BOOL    done = FALSE;

        while (nType < 100 && !done) {
            int retValue;
            sprintf(typeString, "TYPE%d", nType);
            cbData = MAX_PATH;
            if (RegQueryValueEx(hKey, (const char *)typeString, NULL,
                                &dwType, (UCHAR *)value, &cbData) ==
                               ERROR_SUCCESS) {
                if (!strcmp((const char *)value, JNLP_MIME_TYPE)) {
                    done = TRUE;
                    if (RegOpenKeyEx(HKEY_CURRENT_USER, NETSCAPE_MIME_REG_KEY,
                                     0, KEY_ALL_ACCESS, &subKey) ==
                        ERROR_SUCCESS) {

                        RegDeleteValue(subKey, typeString);
                        RegDeleteValue(subKey, JNLP_MIME_TYPE);
                        RegCloseKey(subKey);
                    }
                }
            }
            nType++;
        }
        RegCloseKey(hKey);
    }
}

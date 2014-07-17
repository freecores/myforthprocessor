/*
 * @(#)uninstall.cpp	1.6 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <process.h>
#include <stdio.h>
#include <windows.h>

#include "shared.h"

// This files contains functions that are used as part of the MindVision
// uninstall process.
// Uninstall calls ViseEntry, which will invoke Java Web Start with -uninstall,
// as well as invoking removeNetscapeRegistryEntries to remove the
// netscape registry entries that were created as part of the install.

void removeNetscapeRegistryEntries() {
    HKEY    hKey;

    if (RegOpenKeyEx(HKEY_CURRENT_USER, NETSCAPE_MIME_REG_KEY, 0,
                     KEY_READ, &hKey) == ERROR_SUCCESS) {
        HKEY    subKey;
        DWORD   dwType;
        LPCSTR  value[MAX_PATH];
        char    typeString[24];
        DWORD   cbData;
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

extern "C" LONG CALLBACK ViseEntry(LONG bogus, LPCSTR path, DWORD bogus2) {
    int pid;
    char **argvp;

    removeNetscapeRegistryEntries();

    /* Win32 inexplicably breaks up arguments that contain white space,
     * so we quote them here.
     */
    char *name = strrchr(path, '\\');
    if (name == NULL) {
        name = "javaws.exe";
    }
    spawnl(_P_WAIT, path, name, "-uninstall", NULL);
    
    /* The spawn returns almost immediatly. We will do a pause here
     * to let the Javaws finish it work - otherwise files it depends on
     * might get removed by the installer before it is done. A more complicate
     * scheme should probably be use - but uninstall is an infrequent operation
     */
    Sleep(10 * 1000); /* 10 secs should be enough for most systems */
    
    return 0;
}

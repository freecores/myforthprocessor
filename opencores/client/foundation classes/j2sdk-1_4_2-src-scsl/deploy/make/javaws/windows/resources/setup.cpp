/*
 * @(#)setup.cpp	1.23 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <io.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <tchar.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "shared.h"

//
// This files contains functions that are used as part of the MindVision
// install process.
// The last stage of the install calls ViseEntry, which will update
// the registry (for netscape) and create the config file.
//


extern "C" int isValidMSVCRTVersion(DWORD, DWORD);

extern "C" int isNetscapeRunning();

// Writes out a string to a FILE escaping every occurence of "\" with "\\"
void writeEscapedString(FILE *fp, const char *string) {
    int last = 0;
    int counter;
    for (counter = 0; string[counter] != '\0'; counter++) {
	if (string[counter] == '\\') {
	    if (last != counter) {
		fwrite(string + last, sizeof(char), counter - last, fp);
	    }
	    fputs("\\\\", fp);
	    last = counter + 1;
	}
    }
    if (last < counter) {
	fwrite(string + last, sizeof(char), counter - last, fp);
    }
}

// Creates registry entries so that Java Web Start is associated to mimeType for
// Netscape.
// If Netscape hasn't been installed, this will do nothing!
void createRegistryEntries(const char *path) {
    HKEY    hKeyParent,hKey;
	TCHAR newpath[MAX_PATH];
	
	wsprintf(newpath, "%s%s%s", "\"", path, "\"");
    if (RegOpenKeyEx(HKEY_CURRENT_USER, NETSCAPE_MIME_KEY, 0,
		KEY_WRITE, &hKeyParent) == ERROR_SUCCESS) {
		DWORD disp;
		if (RegCreateKeyEx(HKEY_CURRENT_USER, NETSCAPE_MIME_REG_KEY, 0, "",
			REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, &disp) == ERROR_SUCCESS) {
			
			DWORD   dwType;
			TCHAR  value[MAX_PATH];
			char    typeString[24];
			DWORD   cbData;
			int     nType = 0;
			BOOL    done = FALSE;
			
			while (nType < 100 && !done) {
				int retValue;
				sprintf(typeString, "TYPE%d", nType);
				cbData = MAX_PATH;
				if (RegQueryValueEx(hKey, (const char *)typeString, NULL, &dwType,
					(UCHAR *)value, &cbData) == ERROR_SUCCESS) {
					if (!strcmp((const char *)value, JNLP_MIME_TYPE)) {
						done = TRUE;
						RegCloseKey(hKey);
						if (RegOpenKeyEx(HKEY_CURRENT_USER, NETSCAPE_MIME_REG_KEY,
							0, KEY_WRITE, &hKey) == ERROR_SUCCESS) {
							// Reset mime type
							RegSetValueEx(hKey, JNLP_MIME_TYPE, 0, REG_SZ,
								(const unsigned char *)newpath,
								strlen(newpath) + 1);
							RegCloseKey(hKey);
						}
					}
				}
				else {
					done = TRUE;
					RegCloseKey(hKey);
					if (RegOpenKeyEx(HKEY_CURRENT_USER, NETSCAPE_MIME_REG_KEY, 0,
						KEY_WRITE, &hKey) == ERROR_SUCCESS) {
						// Didn't find it, and there is an empty slot, use it.
						RegSetValueEx(hKey, (const char *)typeString, 0, REG_SZ,
							(const unsigned char *)JNLP_MIME_TYPE,
							strlen(JNLP_MIME_TYPE) + 1);
						RegSetValueEx(hKey, JNLP_MIME_TYPE, 0, REG_SZ,
							(const unsigned char *)newpath,
							strlen(newpath) + 1);
						RegCloseKey(hKey);
					}
				}
				nType++;
			}
		}
		RegCloseKey(hKeyParent);
	}
}


// Creates the config file
// path            - location to create the config file at
// productVersion  - version of the JRE (also default)
// platformVersion - version of the JRE (also default)
// jrePath          - path to the JRE.
void createConfigFile(FILE* configFile,
		     int n,
		     const char *platformVersion,
		     const char *productVersion,
		     const char *jrePath) {
    
     if (configFile != NULL) {
	int counter;
	int length = strlen(jrePath);

	// It writes out three entries
	//   javaws.cfg.jre.<n>.platform
	//   javaws.cfg.jre.<n>.path
	//   javaws.cfg.jre.<n>.product
	//   javaws.cfg.jre.<n>.location (set by Java code)
	fprintf(configFile, "javaws.cfg.jre.%d.platform=%s\n", n, platformVersion);
	fprintf(configFile, "javaws.cfg.jre.%d.product=%s\n", n,  productVersion);
	fprintf(configFile, "javaws.cfg.jre.%d.path=", n);
	writeEscapedString(configFile, jrePath);
	if (length > 0 && jrePath[length - 1] != '\\') {
	    fputs("\\\\", configFile);
	}
	fputs("bin\\\\javaw.exe\n", configFile);
	fflush(configFile);
   }
}

void createDefaultEntry(FILE* configFile, int n) {
  if (configFile != NULL) {
     fprintf(configFile, "javaws.cfg.jre.default=%d\n", n);
     fflush(configFile);
   }
}
			
// Returns 1 if version is >= 1.2 (eg 1.2.2, 1.2, 2, but not 1.1.x)
int isValidJREVersion(const char *version) {
    int major = 0;
    int minor = 0;
	int micro = 0;
    // Major
    int index = 0;
    while (version[index] != '\0' && isdigit(version[index])) {
		major = major * 10 + (version[index] - '0');
		index++;
    }
    if (major > 1) {
		// Always accept a version > 1
		return 1;
    }
	
    // minor
    if (major == 1 && version[index] != '\0') {
		index++;
		while (version[index] != '\0' && isdigit(version[index])) {
			minor = minor * 10 + (version[index] - '0');
			index++;
		}
		//  always accept version >= 1.3
		if (minor >= 3) return 1;
		if (minor == 2 && version[index] != '\0') {
			index++;
			while (version[index] != '\0' && isdigit(version[index])) {
				micro = micro * 10 + (version[index++] - '0');
			}
			// >= 1.2
			if (micro >= 2) {
				return 1;
			}
		}
    }
	
    // Either bogus version string, or < 1.2.2
    return 0;
}

// Obtains the path and real version for the passed in version string.
// For example, if version is 1.2 this will look up the value for the key
// JavaHome in SOFTWARE\\JavaSoft\\Java Runtime Environment\\1.2 and
// return the value in *javaHome. This will also return the realVersion
// (version plus microversion) in
// SOFTWARE\\JavaSoft\\Java Runtime Environment\\1.2\\MicroVersion and
// return the value in realVersion.
// If *realVersion (or *javaHome) points to a non-NULL it will be freed and
// the new value malloced. It is the callers responsibility to free them.
DWORD getJavaHomeAndVersion(const char *version, char **realVersion,
			    char **javaHome) {
    char            versionPath[MAX_PATH];
    HKEY            subHKey;
    LONG            result;

    sprintf(versionPath,"%s\\%s", JRE_VERSION_REG_KEY, version);
    if ((result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, versionPath, 0, KEY_READ,
			       &subHKey)) == ERROR_SUCCESS) {
	DWORD           childType;
	unsigned char   microVersion[16];
	DWORD           microVersionSize = 16;
	DWORD           javaHomeSize = MAX_PATH;
	char            javaHomeBuf[MAX_PATH];
        int             dotCount = 0;
        char            *dotSearch = (char *)version;

	// Get MicroVersion
        // With 1.3, the MicroVersion is now part of the registry key,
        // have to make sure we don't include it twice.
        while ((dotSearch = strchr(dotSearch, '.')) != NULL) {
          dotSearch++;
          dotCount++;
        }
	if (dotCount >= 2 || RegQueryValueEx(subHKey,
                             JRE_MICROVERSION_REG_VALUE, NULL,
			     &childType, microVersion,
			     &microVersionSize) != ERROR_SUCCESS ||
	    childType != REG_SZ) {

	    microVersionSize = 0;
	}

	// Then the path to the JRE
	if ((result = RegQueryValueEx(subHKey, JRE_PATH_REG_VALUE, NULL,
				      &childType, (unsigned char *)javaHomeBuf,
				      &javaHomeSize)) == ERROR_SUCCESS &&
	    childType == REG_SZ && javaHomeSize > 1) {

	    // Versions match, verify that bin/javaw.exe exists.
	    struct _stat statBuf;
	    if (javaHomeBuf[javaHomeSize - 2] != '\\') {
		javaHomeBuf[javaHomeSize - 1] = '\\';
		javaHomeBuf[javaHomeSize] = '\0';
	    }
	    strcat(javaHomeBuf, "bin\\javaw.exe");
	    if (_stat(javaHomeBuf, &statBuf) == 0 &&
		(statBuf.st_mode & S_IFREG) != 0) {

		// Reset realVersion
		if (*realVersion != NULL) {
		    free(*realVersion);
		}
		if (microVersionSize > 1) {
		    // Only add 1 since microVersionSize includes \0 in its
		    // size
		    *realVersion = (char *)malloc(sizeof(char) *
						  (strlen(version) + 1) +
						  microVersionSize);
		    sprintf(*realVersion, "%s.%s", version, microVersion);
		}
		else {
		    *realVersion = (char *)malloc(sizeof(char) *
						  (strlen(version) + 1));
		    strcpy(*realVersion, version);
		}

		// And then javaHome
		*javaHome = (char *)malloc(javaHomeSize);
                javaHomeBuf[javaHomeSize - 1] = '\0';
		strcpy(*javaHome, (char *)javaHomeBuf);
                result = ERROR_SUCCESS;
	    }
            else {
                result = ERROR_BAD_ENVIRONMENT;
            }
	}
        else {
            result = ERROR_BAD_ENVIRONMENT;
        }
	RegCloseKey(subHKey);
    }
    return result;
}

// Creates the config file. This will obtain the versions to use from
// the registry. This will return 0 if succesful, that is at least one
// value was written to the config file, otherwise 1 will be returned.
// path - locate to create the config file at.
int createConfigFileFromRegistry(const char *path, const char *jreVersion,
				 const char *jrePath) {
    HKEY      hKey;

    int jreIndex = 0;
    int regIndex = 0;
    int defaultIndex = -1;
    FILE  *configFile = fopen(path, "w");
    if (configFile == NULL) return 0;
             
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, JRE_VERSION_REG_KEY, 0,
		     KEY_READ, &hKey) == ERROR_SUCCESS) {
	
	char            subKey[MAX_PATH];
	DWORD           subKeyLength = MAX_PATH;
	FILETIME        lastWrite;
		
	// This will point to the last written out values for the JRE.
	// This is set in getJavaHomeAndVersion.
	char            *lastVersion = NULL;

	while (RegEnumKeyEx(hKey, regIndex, subKey, &subKeyLength, NULL,
			    NULL, NULL, &lastWrite) == ERROR_SUCCESS) {
	    char            *javaHome = NULL;

	    //fprintf(configFile, "debug.subKey=%s\n",  subKey);
	    
	    if (getJavaHomeAndVersion(subKey, &lastVersion, &javaHome) ==
		ERROR_SUCCESS) {
				       		
		//fprintf(configFile, "debug.lastVersion=%s\n",  lastVersion);
		//fprintf(configFile, "debug.javaHome=%s\n",  javaHome);
		//fflush(configFile);
		
		if (isValidJREVersion(lastVersion)) {
                    // Create the config file is necessary.
                     int    javaHomeLength = strlen(javaHome);
		
		     createConfigFile(configFile,
		     		      jreIndex,
		     		      lastVersion,
		     		      lastVersion,
		     		      javaHome);
                      if (defaultIndex == -1 && !strcmp(subKey, jreVersion)) {
                          // This is the default
                          defaultIndex = jreIndex;
                      }
                      jreIndex++;
                }
            }
            regIndex++;
	    free(javaHome);
	    javaHome = NULL;
	    subKeyLength = MAX_PATH;
	}
	if (defaultIndex == -1) defaultIndex = 0;
	if (lastVersion != NULL) free(lastVersion);
	RegCloseKey(hKey);
    }
    
    if (jreIndex == 0) {
       // Either couldn't open the registry, or the config file, try using the
       // passed in values.
       createConfigFile(configFile, 0, jreVersion, jreVersion, jrePath);
       defaultIndex = 0;
    }
    
    // Set default index. 0 is always a safe bet.
    createDefaultEntry(configFile, 0);
    
    fclose(configFile);
    return 0;
}

// Searches the file at issFilePath for the first occurence of
// <JREPATH>, replacing it with jrePath. The resulting file is written
// to issFilePath2.
int changeISSFile(const char *issFilePath, const char *jrePath) {
    FILE *inFP = fopen(issFilePath, "r");
    char resultPath[MAX_PATH];
    FILE *outFP;
    int length = strlen(issFilePath);

    if (inFP != NULL) {
	if ((length + 2) < MAX_PATH) {
	    strcpy(resultPath, issFilePath);
	    resultPath[length] = '2';
	    resultPath[length + 1] = '\0';
	    outFP = fopen(resultPath, "w");
	    if (outFP != NULL) {
		// This has the obvious restriction of only allowing
		// MAX_PATH chars on a line, which seems perfectly reasonable.
		char lineBuffer[MAX_PATH];
		while (fgets(lineBuffer, MAX_PATH, inFP) != NULL) {
		    char *match = strstr(lineBuffer, "<JREPATH>");
		    if (match != NULL) {
			if (match - lineBuffer > 0) {
			    fwrite(lineBuffer, sizeof(char),
				   match - lineBuffer, outFP);
			}
			fwrite(jrePath, sizeof(char), strlen(jrePath),
			       outFP);
			if ((match - lineBuffer + strlen("<JREPATH>")) <
			    strlen(lineBuffer)) {
			    fputs(match + strlen("<JREPATH>"), outFP);
			}
			else {
			    fputs("", outFP);
			}
		    }
		    else {
			fputs(lineBuffer, outFP);
		    }
		}
		fclose(outFP);
		fclose(inFP);
		return 0;
	    }
	}
	fclose(inFP);
    }
    // failed!
    return 1;
}

//
// Checks for one of two things:
//  . The registry entry for the passed in string exists
//  . The registry entry for the passed in string minus everything past the
//    last '.' exists (eg if you pass in 1.2.2 this will verify 1.2 exists
//    with a microversion of 2)
// If an entry is found, and the file path ${JavaHome}/bin/javaw.exe
// exists, then ${JavaHome} is returned. JavaHome comes from the registry.
// This two step process is needed as in certain version of the JRE
// (pre 1.3) the version could only be identified by the registry
// key.MicroVersion registry key, where as with 1.3 the complete version
// string (1.3.0) is put in the registry.
//
char *isJREInstalled(char *version, BOOL exactVersion) {
  char *retValue = NULL;
  if (strlen(version) > 0) {
    // Try the complete version first.
    char *fullVersion, *home;

    fullVersion = home = NULL;
    if (getJavaHomeAndVersion(version, &fullVersion, &home) == ERROR_SUCCESS &&
        fullVersion != NULL && home != NULL &&
	(!exactVersion || !strcmp(version, fullVersion))) {
      retValue = home;
    }
    else {
      char *tmp = strrchr(version, '.');
      int microIndex = (tmp == NULL) ? -1 : tmp - version;
      if (microIndex != -1 && (strchr(version, '.') - version) != microIndex) {
        char *jreVersion = strdup(version);

        fullVersion = home = NULL;
        jreVersion[microIndex] = '\0';
        if (getJavaHomeAndVersion(jreVersion, &fullVersion, &home) ==
            ERROR_SUCCESS && fullVersion != NULL && home != NULL) {
          jreVersion[microIndex] = '.';
          if (!strcmp(jreVersion, fullVersion)) {
            retValue = home;
          }
          else {
            free(home);
          }
          free(fullVersion);
        }
        free(jreVersion);
      }
    }
  }
  return retValue;
}

int isHexString(char *string) {
    int counter = 0;
    while (string[counter] != '\0') {
        if (!((string[counter] >= '0' && string[counter] <= '9') ||
              (string[counter] >= 'a' && string[counter] <= 'f') ||
              (string[counter] >= 'A' && string[counter] <= 'F'))) {
            return 0;
        }
        counter++;
    }
    return (counter > 0);
}

DWORD convertHexStringToInt(char *string) {
    DWORD retValue = 0;
    int counter = 0;

    while (string[counter] != '\0') {
        retValue *= 16;
        if (string[counter] >= '0' && string[counter] <= '9') {
            retValue += (string[counter] - '0');
        }
        else if (string[counter] >= 'a' && string[counter] <= 'f') {
            retValue += (string[counter] - 'a') + 10;
        }
        else {
            retValue += (string[counter] - 'A') + 10;
        }
        counter++;
    }
    return retValue;
}

//
// Returns 1 if a reboot is needed.
//
int needsReboot(const char *path) {
	// Another way to detect if we need reboot.
	// Comparing MSVCRT versions might not do the trick, because
	// there might be other files waiting for a reboot as well.

	// There are 2 ways to tell windows to update files on reboot
	// one is wininit.ini and the other is the registry key
	// HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Session Manager\PendingFileRenameOperations .
	// So, if either the file or the key exists, we recommend a reboot.
	HANDLE hfile;
	TCHAR szPathnameWinInit[MAX_PATH];
	TCHAR szPFRKey[]=_T("SYSTEM\\CurrentControlSet\\Control\\Session Manager\\PendingFileRenameOperations");
	HKEY hKey;
	
	// Method 1
	
	// Construct the full pathname of the WININIT.INI file.
	GetWindowsDirectory(szPathnameWinInit, MAX_PATH);
	_tcscat(szPathnameWinInit, _T("\\WinInit.Ini"));
	
	// Try to open the WININIT.INI file.
	if ((hfile = CreateFile(szPathnameWinInit,      
		GENERIC_READ, 0, NULL, OPEN_EXISTING, 
		FILE_ATTRIBUTE_NORMAL, NULL))!=INVALID_HANDLE_VALUE) {
		CloseHandle(hfile);
		return 1;
	}
	
	// Method 2

	// Try to open the Reg Key, if this fails the key does not exist
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, // handle to open key
		szPFRKey, // address of name of subkey to open
		0,        // reserved
		KEY_READ, // security access mask
		&hKey)==ERROR_SUCCESS) {  // address of handle to open key
		RegCloseKey(hKey);
		return 1;
	}

return 0;
	
	/*
    char profileMS[24];
    char profileLS[24];
	
	  if (GetPrivateProfileString("foo", "versionMS", "", profileMS, 24, path) >
	  0 && GetPrivateProfileString("foo", "versionLS", "", profileLS, 24,
	  path) > 0 && isHexString(profileMS) &&
	  isHexString(profileLS)) {
	  DWORD versionMS = convertHexStringToInt(profileMS);
	  DWORD versionLS = convertHexStringToInt(profileLS);
	  
        if (!isValidMSVCRTVersion(versionMS, versionLS)) {
		return 1;
        }
		}
		return 0;
	*/
}

//
// The callback from MindVIsion install process, called after all the
// files have been installed.
//
extern "C" LONG CALLBACK ViseEntry(LONG bogus, LPCSTR path, DWORD bogus2) {
    char installPath[MAX_PATH];
    int  installPathLength;
    char jreVersion[MAX_PATH];
    char jrePath[MAX_PATH];
    char installJRE[MAX_PATH];
    char tempPath[MAX_PATH];
    int  tempPathLength;

    if (bogus == 0) {
      // Check if the requested JRE version is available.
      char *versionPath;
      GetPrivateProfileString("foo", "jreVersion", "", jreVersion, MAX_PATH,
			      (const char *)path);
      if ((versionPath = isJREInstalled(jreVersion, TRUE)) != NULL) {
	WritePrivateProfileString("foo", "jrePath", versionPath,
				  (const char *)path);
	return 0;
      }
      return 1;
    }
    GetPrivateProfileString("foo", "jrePath", "", jrePath, MAX_PATH,
			    (const char *)path);
    if (bogus == 1) {
	// If we're doing an install of the JRE, we need to tweak the jre.iss
	// file to point to where we're installing it.
	GetPrivateProfileString("foo", "installJRE", "", installJRE, MAX_PATH,
				(const char *)path);
	if (strlen(installJRE) > 0 && strcmp("0", installJRE) &&
	    strlen(jrePath) > 0) {
	    GetPrivateProfileString("foo", "tempDir", "", tempPath, MAX_PATH,
				    (const char *)path);
	    tempPathLength = strlen(installPath);
	    if (tempPathLength > 0 && tempPath[tempPathLength - 1] == '\\') {
		tempPathLength--;
		strcat(tempPath, "jre.iss");
	    }
	    else {
		strcat(tempPath, "\\jre.iss");
	    }
	    return changeISSFile(tempPath, jrePath);
	}
	return 0;
    }

    if (bogus == 2) {
        if (needsReboot(path)) {
            return 1;
        }
        return 0;
    }

    if (bogus == 3) {

        // Update custom registry entries
        GetPrivateProfileString("foo", "targetDir", "", installPath, MAX_PATH,
                                (const char *)path);
        installPathLength = strlen(installPath);
        if (installPathLength > 0 && installPath[installPathLength - 1] ==
            '\\') {

            installPathLength--;
            strcat(installPath, "javaws.exe");
        }
        else {
            strcat(installPath, "\\javaws.exe");
        }
        createRegistryEntries(installPath);

        // Create config file.
        GetPrivateProfileString("foo", "jreVersion", "", jreVersion, MAX_PATH,
                                (const char *)path);
        strcpy(installPath + installPathLength, "\\javaws.cfg");

        if (createConfigFileFromRegistry(installPath, jreVersion, jrePath)) {
            // failed!
            return 1;
        }
        return 0;
    }

    if (bogus == 4) {
        return isNetscapeRunning();
    }

    // success
    return 0;
}

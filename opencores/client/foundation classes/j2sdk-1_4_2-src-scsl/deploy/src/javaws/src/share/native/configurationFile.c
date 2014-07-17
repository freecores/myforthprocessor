/*
 * @(#)configurationFile.c	1.38 03/05/15
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "system.h"
#include "propertyParser.h"
#include "configurationFile.h"
#include "util.h"
#include "msgString.h"
#include "launcher.h"
#include "versionId.h"

static PropertyFileEntry* UserCfgFileHead = NULL;
static PropertyFileEntry* SystemCfgFileHead = NULL;

/* ------------------------------------------------------------------------ */
/* Configuration file management */

/*
 * Store all user regular properties, unregistered JREs, and (registered &&
 * confirmed) JREs in the user properties file
 */
void StoreConfigurationFile(void) {
  char propFullFileName[MAXPATHLEN];
  
  /* Build string to user-specific propFileName (e.g., "deployment.properties"
     or "deployment.properties.jdpa") */
  sprintf(propFullFileName, "%s%c%s",
          sysGetDeploymentUserHome(),
          FILE_SEPARATOR,
          CFG_FILENAME);

  storePropertyFile(propFullFileName, UserCfgFileHead, SystemCfgFileHead);

}  

static int nIndices;
static JREIndexMap indexArray[255];

int LoadConfigurationFile(void) {

  nIndices = 0;
  memset(indexArray, 0, sizeof(indexArray));
  LoadCfgFile(CFG_FILENAME, &UserCfgFileHead, &SystemCfgFileHead);
  return (SystemCfgFileHead != UserCfgFileHead);
}


/* CL: Code factored out of original version of LoadConfigurationFile(),
 * so this code can be used by other types of configuration loading
 * functions if desired, e.g., LoadJpdaConfigurationFile() in "jpda.c".
 */
void LoadCfgFile(char* propFileName, PropertyFileEntry** UserPropertyFileEntry,
		 PropertyFileEntry** SystemPropertyFileEntry) {
  char propFullFileName[MAXPATHLEN];
 
  /* Build string to system wide property file name
     (e.g., "deployment.properties" or "deployment.properties.jdpa") */
  sprintf(propFullFileName, "%s%c%s",
          sysGetDeploymentSystemHome(),
          FILE_SEPARATOR,
          propFileName);

  /* Load and parse the system wide configuration property file */
  *SystemPropertyFileEntry = parsePropertyFile(propFullFileName, NULL);
  
  /* Build string to user-specific property file name
     (e.g., "deployment.properties" or "deployment.properties.jdpa") */
  sprintf(propFullFileName, "%s%c%s",
          sysGetDeploymentUserHome(),
          FILE_SEPARATOR,
          propFileName);

  /* Load and parse the users configuration property file, adding to system */
  *UserPropertyFileEntry = parsePropertyFile(propFullFileName, 
						*SystemPropertyFileEntry);
}

void UpdateJREInfo (JREDescription *jre) {
  char *argv[10];
  int buflen = 1024;
  char *buf = NULL, *start, *end;
  static char path[MAXPATHLEN];

  buf = (char *)malloc(buflen);
  argv[0] = jre->path;
  argv[1] = "-classpath";
  argv[2] = sysQuoteString(GetClassPath());
  argv[3] = JRELOCATORCLASSNAME;
  argv[4] = NULL;

  sysExec2Buf(jre->path, 4, argv, buf, &buflen);

  /* find version info */
  if (buflen > 0) { 

    start = strstr(buf, PRODUCT_ID);
    if (start != NULL) {
      start += strlen(PRODUCT_ID);
      end = strchr(start, '\n');
      if (end > start) {
	char *p = jre->product_version = (char *)malloc(end - start + 1);
	while(!isWhitespace(*start)) {
	    *p++ = *start++;
	}
	*p = 0;
      }
    }

    start = strstr(buf, PLATFORM_ID);
    if (start != NULL) {
      start += strlen(PLATFORM_ID);
      end = strchr(start, '\n');
      if (end > start) {
	char *p = jre->platform_version = (char *)malloc(end - start + 1);
	while(!isWhitespace(*start)) {
	    *p++ = *start++;
	}
	*p = 0;
      }
    }
    
    if (jre->platform_version != NULL && 
	strcmp(jre->platform_version, "1.2") == 0) {
      /* 1.2 platform doesn't contain the full version, extract from
	 -fullversion */
      buflen = 1024;
      argv[0] = jre->path;
      argv[1] = "-fullversion";
      argv[2] = NULL;
      sysExec2Buf(jre->path, 2, argv, buf, &buflen);

      if (buflen > 0) {
	start = strstr(buf, "1.2");
	if (start != NULL) {
	  end = strchr(buf, '\n');
	  if (end > start) {
	    char *p = NULL;
	    if (jre->product_version != NULL) free(jre->product_version);
	    p = jre->product_version = (char *) malloc(end - start + 1);
	    while(!isWhitespace(*start) && *start != '\"') {
	      *p++ = *start++;
	    }
	    *p = 0;
	  }
	}
      }
    }
  }

  jre->href = strdup(DEFHREF);
  jre->osname = PLATFORM;
  jre->osarch = ARCH;
}

/*
 * Get a list of all the JREs from the registry.  For each JRE in this
 * list, 1. add it to the config list if it's not already there, and
 * 2. mark it as "confirmed" in the config list.  Then, remove all
 * unconfirmed registered JREs from the config list.
 */
int RescanJREs() {
  JREDescription reg_list[255];
  int nJREs = 0;
  int i, j;
  char *path;
  int return_value = FALSE;

  memset(reg_list, 0, sizeof(JREDescription) * 255);
  sysGetRegistryJREs(reg_list, &nJREs);
  
  for (i = 0; i < nJREs; i++) {
    /* check if the JRE is already in the list */
    int match = 0;

    for (j = 0; j < nIndices; j++) {
      int index = indexArray[j].index;
      path = GetJREJavaCmd(index);

      if (path != NULL && sysStrCaseCmp(path, reg_list[i].path) == 0) {
	/* it matches.  mark it confirmed and registered */
	match = 1;
	indexArray[j].confirmed = 1;
	if (!isJRERegistered(index)) {
	    SetJRERegistered(index, "true");
	    return_value = TRUE;
	}
        /* keep going here - may be more with same path */
      }
    }

    /* if there was no match we need to add the JRE to the list */
    if (match == 0) {
      int new = getUniqueIndexArrayIndex();
      UpdateJREInfo(&reg_list[i]);

      /* fix for 4813021 */
      /* check to make sure all the info exist before adding */
      if (!(reg_list[i].platform_version == NULL ||
	  reg_list[i].product_version == NULL ||
	  reg_list[i].href == NULL ||
	  reg_list[i].osname == NULL ||
	    reg_list[i].osarch == NULL)) {

	indexArray[nIndices].index = new;
	indexArray[nIndices++].confirmed = 1;
	SetJREPlatformVersion(new, reg_list[i].platform_version);
	SetJREProductVersion(new, reg_list[i].product_version);
	SetJRELocation(new, reg_list[i].href);
	SetJREJavaCmd(new, reg_list[i].path);
	SetJREOsName(new, reg_list[i].osname);
	SetJREOsArch(new, reg_list[i].osarch);
	SetJREEnabled(new, "true");
	SetJRERegistered(new, "true");
	return_value = TRUE;
      }
    }
  }
  return return_value;
}

int getUniqueIndexArrayIndex() {
    int i, j;
    int max = -1;
    for (i=0; i<nIndices; i++) {
       if (indexArray[i].index > max) max = indexArray[i].index;
    }
    /* check for a hole */
    for (j=0; j<max; j++) {
        for (i=0; i<nIndices; i++) { 
	    if (indexArray[i].index == j) break;
        }
        if (i == nIndices) return j; /* nobody using j */
    }
    /* no holes */
    return max + 1;
}

void addToIndexArray(int newIndex) {
    int j;
    for (j = 0; j < nIndices; j++) {
        if (indexArray[j].index == newIndex) return; /* not new */
    }
    indexArray[nIndices++].index = newIndex;
}

void SetJREKey(int i, char *part, char *value) {
  PropertyFileEntry* entry = NULL;
  char *key;
  char *oldval;
  
  /* check if entry exists */
  key = (char *)malloc(MAX_KEY_SIZE);
  sprintf(key, "%s%d%s", CFG_JRE_KEY, i, part);
  oldval = GetPropertyValue(UserCfgFileHead, key);
  if (oldval != NULL) {
    strcpy(oldval, value);
    free(key);
    return;
  }

  /* create entry since it didn't exist */
  entry = (PropertyFileEntry*)malloc(sizeof(PropertyFileEntry));
  entry->key = key;
  entry->value = strdup(value);
  entry->next = UserCfgFileHead;
  UserCfgFileHead = entry;
}

void SetJREPlatformVersion(int i, char *value) {
    SetJREKey(i, PLATFORM_VERSION, value);
}

void SetJREProductVersion(int i, char *value) {
    SetJREKey(i, PRODUCT_VERSION, value);
}

void SetJRELocation(int i, char *value) {
    SetJREKey(i, PRODUCT_LOCATION, value);
}

void SetJREJavaCmd(int i, char *value) {
    SetJREKey(i, INSTALL_PATH, value);
}

void SetJREOsName(int i, char *value) {
    SetJREKey(i, OS_NAME, value);
}

void SetJREOsArch(int i, char *value) {
    SetJREKey(i, OS_ARCH, value);
}

void SetJREEnabled(int i, char *value) {
    SetJREKey(i, ISENABLED, value);
}

void SetJRERegistered(int i, char *value) {
    SetJREKey(i, ISREGISTERED, value);
}

char* GetJREKey(int i, char* part) {
    char key[MAX_KEY_SIZE];
    char* value;
    sprintf(key, "%s%d%s", CFG_JRE_KEY, i, part);
    value = GetPropertyValue(UserCfgFileHead, key);
    return value;
}

char* GetJREPlatformVersion(int i) {
    return GetJREKey(i, PLATFORM_VERSION);
}

char* GetJREProductVersion(int i) {
    return GetJREKey(i, PRODUCT_VERSION);
}

char* GetJRELocation(int i) {
    return GetJREKey(i, PRODUCT_LOCATION);
}

char* GetJREJavaCmd(int i) {
    return GetJREKey(i, INSTALL_PATH);
}

char* GetJREOsName(int i) {
    return GetJREKey(i, OS_NAME);
}

char* GetJREOsArch(int i) {
    return GetJREKey(i, OS_ARCH);
}

int isJREEnabled(int i) {
    char *value = GetJREKey(i, ISENABLED);
    if (value != NULL && sysStrCaseCmp(value,"false")==0) {
      return 0;
    }
    return 1;
}

int isJRERegistered(int i) {
    char *value = GetJREKey(i, ISREGISTERED);
    if (value != NULL && sysStrCaseCmp(value,"true")==0) {
      return 1;
    }
    return 0;
}

int isJREConfirmed(int index) {
  int i;
  for (i = 0; i < nIndices; i++) {
    if (indexArray[i].index == index) {
      return indexArray[i].confirmed;
    }
  }  
  return FALSE;
}



/*
 * Returns the index of the best matching JRE entry in the
 * configuration file. This might be the default JRE if no
 * match is found
 *
 * We will first try to match on the first entry in the version
 * string, e.g., a version string of the form "1.3 1.2", we
 * will try to match on "1.3". If that fails, we will match on
 * the full string.
 *
 */
int DetermineVersion(char* version, char* location) {
   char* firstMatch, *p;
   int index;
   
   firstMatch = strdup(version);
   p = strchr(firstMatch, ' ');
   while (p != NULL) {
      *p = '\0';
      index = matchVersionString(firstMatch, location);
      if (index != -1) return index;
      /* OK - look at the next one if there is one */
      firstMatch = p+1;
      if (*firstMatch != 0) {
         p = strchr(firstMatch, ' ');
      } else {
         p = NULL;
      }
   }
   
   index = matchVersionString(firstMatch, location);
   if (index == -1) {
      /* nothing matches, return latest one, including disabled */
      int best = -1;
      int pos;
      for (pos = 0; pos < nIndices; pos++) {
         index = indexArray[pos].index;
         if ((GetJREOsName(index) == NULL && GetJREOsArch(index) == NULL) ||
             (GetJREOsName(index) != NULL && GetJREOsArch(index) != NULL &&
	      strcmp(sysGetOsName(), GetJREOsName(index)) == 0 &&
	      strcmp(sysGetOsArch(), GetJREOsArch(index)) == 0)) {
	   /* fix for 4813021 */
	   /* only make the comparison if the JRE platform and product info
	      exist */
	   if (GetJREPlatformVersion(index) != NULL && GetJREProductVersion(index) != NULL) {
	     best = laterVersion(best, index);
	   }
         }
      }   
      return best;
   }
   return index;
}

int matchVersionString(char* version, char* location) {
    int index = 0;
    int pos = 0;
    int bestMatch = -1;

    /* Then iterate */
    for (pos = 0; pos < nIndices; pos++) {
        index = indexArray[pos].index;
	/* fix for 4813021 */
	/* only make the comparison if the JRE platform and product info
	   exist */
	if (GetJREPlatformVersion(index) != NULL && 
	    GetJREProductVersion(index) != NULL &&
	    JreMatch(version, location, index)) {
	        bestMatch = laterVersion(bestMatch, index);
        }
    }
    return bestMatch;
}

int laterVersion(index1, index2) {
    int comp;
    if (index1 < 0) {
	return index2;
    }
    comp = strcmp(GetJREPlatformVersion(index1), GetJREPlatformVersion(index2));
    if (comp > 0) return index1;
    if (comp < 0) return index2;
    /* same platform version - try product version */
    comp = strcmp(GetJREProductVersion(index1), GetJREProductVersion(index2)); 
    if (comp >= 0) return index1; /* tie goes to first */
    return index2; 
}

int isCurrentVersion(char *javaCmd) {
    char *current = sysGetInstallJRE();
    /* we'd really like to see if two strings refer to the same file */
    if ((current != NULL) && (strcmp(current, javaCmd) == 0)) {
	return TRUE;
    }
    return FALSE;
}

/*
 * Check if a particular entry matches
 *
 */
int JreMatch(char* version, char* location, int index) {
    char* jrePlatformVersion;
    char* jreProductVersion;
    char* jreLocation;
    char* jreJavaCmd;
    struct stat statBuf;
    
    if (!isJREEnabled(index)) return FALSE;

    /* Get entries from configuration file */
    jrePlatformVersion = GetJREPlatformVersion(index);

    /* Empty entry? */
    if (jrePlatformVersion == NULL) return FALSE;
    jreProductVersion = GetJREProductVersion(index);

    jreLocation = GetJRELocation(index);
    jreJavaCmd = GetJREJavaCmd(index);
    
    /* Entry match? */
      
    /* osname and osarch must match */
    /* if osname and osarch doesn't exist, we assume it is for the */
    /* current platform */
    if ((GetJREOsName(index) == NULL && GetJREOsArch(index) == NULL) || 
        (GetJREOsName(index) != NULL && GetJREOsArch(index) != NULL &&
	strcmp(sysGetOsName(), GetJREOsName(index)) == 0 &&
	strcmp(sysGetOsArch(), GetJREOsArch(index)) == 0)) {

      /* make sure the executable actually exists */
      if (stat(GetJREJavaCmd(index), &statBuf) == 0) {
	
	if (location == NULL) {
	  /* Platform match 
	   * Make sure that it is not a non-fcs version. A non-FCS
	   * will have a - in the platform version for 1.3 and later.
	   */        
	  if (MatchVersionString(version, jrePlatformVersion) &&
	      ( jreProductVersion == NULL ||
		strncmp("1.2", jreProductVersion, 3) == 0 ||
		strchr(jreProductVersion, '-') == NULL) ||
		/* now -beta ok if it's the one this javaws comes with */
		isCurrentVersion(jreJavaCmd)) {
            return TRUE;
	  }
	} else {
	  if (jreLocation != NULL &&
	      strcmp(location, jreLocation) == 0 &&
	      MatchVersionString(version, jreProductVersion)) {
	    return TRUE;
	  }
	}
      }
    }
    return FALSE;
}

/*
 * Check if we should show the splash screen
 */
int isSplashScreenEnabled(void) {
  char* value = GetPropertyValue(UserCfgFileHead, CFG_SPLASH_MODE);
  return (value == NULL || strcmp(value, "false") != 0);
}

/*
 *  get preferred JRE versions for Application Manager and apps that don't say
 */
char *getDefaultJREs(void) {
  /* default: 1.) mantis,  2.) any 1.4 version, 3.) anything from 1.3 and above */
  return "1.4.2* 1.4* 1.3+";
}

char *getSplashFile(char *url) {
    char *splashCacheFile;
    PropertyFileEntry *splashCacheFileHead;

    splashCacheFile = GetPropertyValue(UserCfgFileHead, CFG_SPLASH_CACHE);

    if (splashCacheFile != NULL) {
        splashCacheFileHead = parsePropertyFile(splashCacheFile, NULL);
        if (splashCacheFileHead != NULL) {
	    char *ret;
	    char *escaped_url = NULL;
	    if (url != NULL) {
		int len = strlen(url);
		escaped_url = calloc(2,len);
		if (escaped_url != NULL) {
		    int i, j;
		    for (i=0, j=0; i<len; i++) {
			if (url[i] == ':' || url[i] == '=') {
			    escaped_url[j++] = '\\';
			}
			escaped_url[j++] = url[i];
		    }
		}
	    }
            ret = GetPropertyValue(splashCacheFileHead, escaped_url);
	    if (escaped_url != NULL) {
		free (escaped_url);
	    }
            return ret;
	}
    }
    return NULL;
}

char *getConfigSecureProperties() {
    return GetPropertyValue(UserCfgFileHead, CFG_SECURE_PROPS);
} 
 


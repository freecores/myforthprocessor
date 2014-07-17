
/*
 * @(#)splashFile.c	1.6 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "system.h"
#include "launchFile.h"
#include "configurationFile.h"

/*
 * Locates the jpeg files to be used in the splash screen.
 *
 */

static char path1[MAXPATHLEN];
static char path2[MAXPATHLEN];


void getDefaultSplashFiles(int playerMode, char **splash1, char **splash2) {

    char *name = (playerMode) ? "splash." : "miniSplash." ;
    
    sprintf(path1, "%s%c%s%c%s%s", sysGetApplicationHome(),
        FILE_SEPARATOR, "resources", FILE_SEPARATOR, name, sysGetSplashExtension());
    *splash1 = path1;

    name = (playerMode) ? "copyright." : "miniCopy." ;

    sprintf(path2, "%s%c%s%c%s%s", sysGetApplicationHome(),
        FILE_SEPARATOR, "resources", FILE_SEPARATOR, name, sysGetSplashExtension());
    *splash2 = path2;

}

getAppSplashFiles(JNLFile *jnlFile, char **splash1, char **splash2) {
    /* Try to get application defined splash screen */
    *splash1 = getSplashFile(jnlFile->canonicalHome);
    *splash2 = NULL;

    /* if none - use our default */
    if ((*splash1 == NULL) || (!endsWith(*splash1, sysGetSplashExtension()))) {

        getDefaultSplashFiles(FALSE, splash1, splash2);
    }
}

int endsWith(char *str1, char *str2) {
    if ((str1 != NULL) && (str2 != NULL)) {
        size_t len1,len2;
        len1 = strlen(str1);
        len2 = strlen(str2);
        if (len1 >= len2) {
            if (strcmp(str1+(len1-len2), str2) == 0) {
                return TRUE;
            }
        }
    }
    return FALSE;
}
    




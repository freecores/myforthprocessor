/*
 * @(#)fontpath.c	1.38 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *  Author: Alex D. Gelfenbain
 */

#ifdef __linux__
#include <string.h>
#endif /* __linux__ */
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#include <jni.h>
#include <jni_util.h>
#include <sun_awt_font_NativeFontWrapper.h>
#ifndef HEADLESS
#include <X11/Xlib.h>
#endif /* !HEADLESS */

#if defined(__linux__) && !defined(MAP_FAILED)
#define MAP_FAILED ((caddr_t)-1)
#endif

#ifndef HEADLESS
extern Display *awt_display;
#endif /* !HEADLESS */

/* The goal of this function is to find all Solaris "system" fonts.
 * We consider that a system font is one that has been added to the system
 * with a pkgadd(1M) command. All files added to the system with pkgadd
 * have an entry in the /var/sadm/system/contents file.
 *
 * This algorithm means that if a user just copies some fonts to his
 * home directory, these fonts will not be considered "system" fonts
 * and picked up by the JDK by default. That's what JAVA_FONTS variable
 * is for.
 *
 * XXX This algorithm doesn't handle the fonts installed by the Solaris Font Administrator
 * FontAdmin doesn't register the fonts in the contents file, so we need to scan
 * its specific font directories like /etc/openwin to find fonts installed by it.
 */



#define MAXFDIRS 512                    /* Max number of directories that contain fonts */

/* 
 * This can be set in the makefile to "/usr/X11" if so desired.
 */
#ifndef OPENWINHOME
#define OPENWINHOME "/usr/openwin"
#endif


#define SCAN_FONT_DIRS 1

static char *defaultPath = OPENWINHOME "/lib/X11/fonts/Type1:" \
			   OPENWINHOME "/lib/X11/fonts/TrueType";

typedef struct {
    const char *name[MAXFDIRS];
    int  num;
} fDirRecord, *fDirRecordPtr;

#ifndef HEADLESS

/*
 * Returns True if display is local, False of it's remote.
 */
jboolean isDisplayLocal(JNIEnv *env) {
    static jboolean isLocal = False;
    static jboolean isLocalSet = False;
    jboolean ret;

    if (isLocalSet) {
	return isLocal;
    }

    isLocal = JNU_CallStaticMethodByName(env, NULL,
					 "sun/awt/X11GraphicsEnvironment",
					 "isDisplayLocal",
					 "()Z").z;
    isLocalSet = True;
    return isLocal;
}

static void AddFontsToX11FontPath ( fDirRecord *fDirP )
{
    char *onePath;
    int index, nPaths;
    int origNumPaths, length;
    int origIndex;
    int totalDirCount;
    char  **origFontPath;
    char  **tempFontPath;
    int doNotAppend;
    int *appendDirList;
    char **newFontPath;
    int err, compareLength;
    char fontDirPath[512];
    int dirFile;

    doNotAppend = 0;

    if ( fDirP->num == 0 ) return;

    appendDirList = malloc ( fDirP->num * sizeof ( int ));
    if ( appendDirList == NULL ) {
      return;  /* if it fails we cannot do much */
    }

    origFontPath = XGetFontPath ( awt_display, &nPaths );

    totalDirCount = nPaths;
    origNumPaths = nPaths;
    tempFontPath = origFontPath;


    for (index = 0; index < fDirP->num; index++ ) {

        doNotAppend = 0;
   
        tempFontPath = origFontPath;
        for ( origIndex = 0; origIndex < nPaths; origIndex++ ) {

            onePath = *tempFontPath;

	    compareLength = strlen ( onePath );
	    if ( onePath[compareLength -1] == '/' )
	      compareLength--;

	    /* there is a slash at the end of every solaris X11 font path name */
	    if ( strncmp ( onePath, fDirP->name[index], compareLength ) == 0 ) {
	      doNotAppend = 1;
	      break;
	    }
	    tempFontPath++;
	}

	appendDirList[index] = 0;
	if ( doNotAppend == 0 ) {
	    strcpy ( fontDirPath, fDirP->name[index] );
	    strcat ( fontDirPath, "/fonts.dir" );
	    dirFile = open ( fontDirPath, O_RDONLY, 0 );
	    if ( dirFile == -1 ) {
		doNotAppend = 1;
	    } else {
	       close ( dirFile );
	       totalDirCount++;
	       appendDirList[index] = 1;
	    }
	}

    }

    /* if no changes are required do not bother to do a setfontpath */
    if ( totalDirCount == nPaths ) {
      free ( ( void *) appendDirList );
      XFreeFontPath ( origFontPath );
      return;
    }


    newFontPath = malloc ( totalDirCount * sizeof ( char **) );
    /* if it fails free things and get out */
    if ( newFontPath == NULL ) {
      free ( ( void *) appendDirList );
      XFreeFontPath ( origFontPath );
      return;
    }
    
    for ( origIndex = 0; origIndex < nPaths; origIndex++ ) {
      onePath = origFontPath[origIndex];  
      newFontPath[origIndex] = onePath;
    }
  
    /* now add the other font paths */
  
    for (index = 0; index < fDirP->num; index++ ) {

      if ( appendDirList[index] == 1 ) {

	/* printf ( "Appending %s\n", fDirP->name[index] ); */

	onePath = malloc ( ( strlen (fDirP->name[index]) + 2 )* sizeof( char ) );
	strcpy ( onePath, fDirP->name[index] );
	strcat ( onePath, "/" );
	newFontPath[nPaths++] = onePath;
	/* printf ( "The path to be appended is %s\n", onePath ); */
      }
    }

    /*   printf ( "The dir count = %d\n", totalDirCount ); */
    free ( ( void *) appendDirList );

    XSetFontPath ( awt_display, newFontPath, totalDirCount );

	for ( index = origNumPaths; index < totalDirCount; index++ ) {
   		free( newFontPath[index] );
    }

	free ( (void *) newFontPath );
    XFreeFontPath ( origFontPath );
    return;
}
#endif /* !HEADLESS */


static void AppendFont(fDirRecordPtr fdir, char *filename, int len, int noType1)
{
    char *end = filename + len;
    static char *previous = NULL;                           /* cache for the last inserted directory name */
    int i, res, l = 0, r = fdir->num-1;
    char *ptr;

    if ( noType1 ) {
      /* This is essentially to get around an XServer bug 4215548 which crashes if certain
	 Type1 fonts are in the fontpath. The way to get around is to not include
	 Type1 fonts in the fontpath - those that already were in the path will
	 not be affected */
        char tempFileStr[1024];
        int temp;
	/* very unlikely that paths will be longer than 1024 */
        if ( len < 1024 ) {
	    for ( temp = 0; temp < len; temp++) {
	        tempFileStr[temp] = filename[temp];
	    }
            tempFileStr[len] = 0x00;
	    /* Postscript fonts have either .pfa, .pfb upper and lower case */
            if ( strstr ( tempFileStr, ".PF" ) || strstr ( tempFileStr, ".pf" )) {
	         return;
	    }
	}
    }

    if (fdir->num == MAXFDIRS) {
	/* no more room in the array */
        return;
    }

#if 0
    printf("AppendFont: '%.*s'\n", len, filename);
#endif

    /* Extract the pathname component */
    while (*end != '/' && end  > filename) {
        end--;
    }

    if (*end != '/') {
	/* corrupted contents file */
        return;
    }
    len = end - filename;

    if (previous &&
	!strncmp(filename, previous, len) &&
	previous[len] == '\0')
    {
	/* this name is the same as previous, don't bother */
        return;
    }

    /* Binary search to find the insertion point */
    while (l <= r) {
        i = (l+r) >> 1;
        res = strncmp(fdir->name[i], filename, len);
	if (res == 0 && fdir->name[i][len] != '\0') {
	    res = 1;
	}
        if (res >= 0) {
            l = i + 1;
        }
        if (res <= 0) {
            r = i - 1;
        }
    }

    if (l - 1 > r) {
	/* directory is found, don't insert */
        return;
    }

    if ((ptr = malloc(len + 1)) == NULL) {        /* not enough memory */
        return;
    }
    memcpy(ptr, filename, len);
    ptr[len] = '\0';

    /* if the insertion point is on the border, don't move the pointers */

    if (l < fdir->num) {
        memmove(&(fdir->name[l+1]), &(fdir->name[l]), (fdir->num - l) * sizeof (char *));
    }

    fdir->name[l] = ptr;
#if 0
    printf("AppendDir: '%s'\n", ptr);
#endif
    fdir->num++;
    previous = ptr;
}

/* Norbert suggested that we load all Solaris fonts. */
static char *getSolarisFontLocations(JNIEnv *env, int noType1)
{
    char *addr, *line, *end;
    char *suffix, *result = NULL;
    fDirRecord fDir;
    int i, len=0;
    int fd;
    struct stat statbuf;

    addr = NULL;
    fd = open("/var/sadm/install/contents", O_RDONLY, 0);
    if (fd >= 0) {
	if (fstat(fd, &statbuf) >= 0) {
	    addr = mmap(0, statbuf.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	    if (addr == MAP_FAILED) {
		addr = NULL;
	    }
	}
	close(fd);
    }

    /* printf("JAVA_FONTS was not set, looking for installed fonts...\n"); */

    fDir.num = 0;

    if (addr == NULL) {
        return strdup(defaultPath);
    }

    line = addr;
    end = line + statbuf.st_size;

    while (line < end) {

        /* XXX Intentionally don't check for the ".PS" suffix, which is not unique for fonts
         * and  there are a *lot* of Postscript files with this suffix.
         * This might be a problem for CJK fonts that use this suffix for CID fonts. */

	for (suffix = line; suffix < end; suffix++) {
	    if (*suffix == ' ') {
		break;
	    }
	    if (*suffix == '=') {
		break;
	    }
	    if (*suffix == '\n') {
		line = suffix+1;
	    }
	}
	if (suffix+1 >= end) {
	    // Must have room for a trailing 'f'
	    break;
	}
	if (suffix < line+4) {
	    // Must have room for a preceding ".XXX" file suffix
	    goto nextline;
	}
	if ((suffix[0] != '=' && suffix[1] != 'f') || suffix[-4] != '.') {
	    goto nextline;
	}
	if (suffix[-3] == 'p') {
	    if (suffix[-2] != 'f') {
		goto nextline;
	    }
	    if (suffix[-1] != 'a' && suffix[-1] != 'b') {
		goto nextline;
	    }
	} else if (suffix[-3] == 'P') {
	    if (suffix[-2] != 'F') {
		goto nextline;
	    }
	    if (suffix[-1] != 'A' && suffix[-1] != 'B') {
		goto nextline;
	    }
	} else if (suffix[-3] == 't') {
	    if (suffix[-2] != 't') {
		goto nextline;
	    }
	    if (suffix[-1] != 'f' && suffix[-1] != 'c') {
		goto nextline;
	    }
	} else if (suffix[-3] == 'T') {
	    if (suffix[-2] != 'T') {
		goto nextline;
	    }
	    if (suffix[-1] != 'F' && suffix[-1] != 'C') {
		goto nextline;
	    }
	} else {
	    goto nextline;
	}

        AppendFont(&fDir, line, suffix - line, noType1);

    nextline:
	while (++suffix < end) {
	    if (*suffix == '\n') {
		break;
	    }
	}
	line = suffix+1;
    }

    munmap(addr, statbuf.st_size);

    /* Now convert everything to one string */
    for (i=0; i<fDir.num; i++) {
        len += (strlen(fDir.name[i]) + 1);
    }

    if (len > 0 && (result = malloc(len))) {
        *result = 0;
        for (i = 0; i<fDir.num; i++) {
            if (i != 0) {
                strcat(result, ":");
            }
            strcat(result, fDir.name[i]);
        }
    } else {
        result = strdup(defaultPath);
    }
#if 0
// do not add the fontpath here - add as needed this is really excessive
#ifndef HEADLESS
    if (awt_display != NULL && isDisplayLocal(env)) {
        AddFontsToX11FontPath ( &fDir );
    }
#endif /* !HEADLESS */
#endif
    for (i=0; i<fDir.num; i++) {
        free((char*)fDir.name[i]);
    }
    
    

    return result;
}

#ifndef HEADLESS
static char *getSolarisFontPath ()
{
    int nPaths;
    char **solarisPath, **tempPath;
    char *thePath, *aPath;
    int i, position;
    int length = 0;

    thePath = NULL;
    solarisPath = XGetFontPath ( awt_display, &nPaths );
    tempPath = solarisPath;

    for ( i=0; i < nPaths; i++) {
        aPath = *tempPath;
	length += strlen ( aPath );
	length++; // for the :
	tempPath++;
    }
//    printf ( "The nPaths = %d  length = %d \n", nPaths, length );
    thePath = (char *) malloc ( length );

    tempPath = solarisPath;
    position = 0;
    thePath[0] = '\0';
    for ( i=0; i < nPaths; i++) {
        aPath = *tempPath;
	// printf ( "The first path = %s\n", aPath );
	strcat ( thePath, aPath );
	if ( i < (nPaths -1))
	    strcat ( thePath, ":" );
	tempPath++;
    }
//    printf ("The path = %s\n", thePath );
    return thePath;
}


#endif /* !HEADLESS */

static char *getPlatformFontPathChars(JNIEnv *env, jboolean noType1) {
    char *ptr;

#ifdef __linux__
    ptr = "/usr/X11R6/lib/X11/fonts/Type1:"
          "/usr/X11R6/lib/X11/fonts/TrueType:"
          "/usr/X11R6/lib/X11/fonts/tt:"
          "/usr/share/fonts/ja/TrueType";
#elif defined( HEADLESS )
    ptr = getSolarisFontLocations(env, (int) (noType1==JNI_TRUE));
#elif SCAN_FONT_DIRS
    ptr = getSolarisFontLocations(env, (int) (noType1==JNI_TRUE));

#else /* !SCAN_FONT_DIRS */
    ptr = getSolarisFontPath();
#endif /* !SCAN_FONT_DIRS */
    return ptr;
}

static void freePlatformFontPathChars(JNIEnv *env, char *ptr) {
#ifndef __linux__
    free(ptr);
#endif /* !__linux__ */
}

JNIEXPORT jstring JNICALL Java_sun_awt_font_NativeFontWrapper_getFontPath(JNIEnv *env, jclass obj, jboolean noType1)
{
    jstring ret;
    char *ptr = getPlatformFontPathChars(env, noType1);
    ret = (*env)->NewStringUTF(env, ptr);
    freePlatformFontPathChars(env, ptr);
    return ret;
}


JNIEXPORT void JNICALL Java_sun_awt_font_NativeFontWrapper_setNativeFontPath (JNIEnv *env, jclass obj, jstring theString)
{
#ifdef HEADLESS
    return;
#else
    fDirRecord fDir;
    const char *theChars;

    if (awt_display == NULL) {
        return;
    }
    
    if (awt_display != NULL && isDisplayLocal(env)) {
        theChars = (*env)->GetStringUTFChars (env, theString, 0);
	fDir.num = 1;
	fDir.name[0] = theChars;
	/* printf ("Registering the font path here %s \n", theChars ); */
        AddFontsToX11FontPath ( &fDir );
	if (theChars) {
	    (*env)->ReleaseStringUTFChars (env, theString, (const char*)theChars);
	}
    }

#endif
}


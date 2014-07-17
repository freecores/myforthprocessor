/*
 * @(#)system_md.c	1.28 03/05/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/* Implementation of solaris/linux specific OS layer */
#include <locale.h>
#include <wchar.h>
#include <malloc.h>
#include <langinfo.h>
#include <iconv.h>
#include <wait.h>
#include <sys/utsname.h>	/* For os_name */
#include "system.h"
#include "util.h"
#include "configurationFile.h"

#ifdef SOLARIS                                                          
#include <sys/systeminfo.h>       /* For os_arch */
#endif

#ifdef __linux__
#define CODESET _NL_CTYPE_CODESET_NAME
#endif

static char *splashExtension = "jpg"; /* use jpeg files on unix */

/* 
 * Print an (obscure) error message to stderr and exit.  If errno has 
 * been set then print the corresponding error messages too.
 *
 */
void sysErrorExit(char *msg) 
{
    fprintf(stderr, getMsgString(MSG_SPLASH_EXIT));
    if (errno != 0) {
	perror(msg);
    }
    else {
	fprintf(stderr, "\t%s\n", msg);
    }
    exit(-1);
}

/*
 * Print a message to console. This is abstracted, since windows do not
 * have a console
 */
void sysMessage(char *msg) {
    fprintf(stdout, JAVAWS_NAME " %s", msg);
}

/* 
 * Initialize the socket library.   This function is called by all of the 
 * other functions in this file.
 * 
 * Note that initialization is only required on Win32.
 * 
 */
void sysInitSocketLibrary()
{
    /* Nothing to do */
}



/* 
 *  Safely close a socket. (This is more complicated on Win32)
 */
void sysCloseSocket(SOCKET s) 
{
    sysInitSocketLibrary();

    if (s <= 0) {
	return;
    }
    close(s);
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

    if (type == SYS_EXEC_REPLACE) {
        return execv(path, argv);
    } else {
        
        if ((pid = fork()) == 0) {
            int err = execv(path, argv);
            /* It's neccessary to call "_exit" here, rather than exit, see
             * the fork(2) manual page.
             */
            perror(getMsgString(MSG_BADINST_EXECV));
            _exit(-1);
        } else {
	  if (type == SYS_EXEC_WAIT) {
	    wait(0);
	  }
	}
        return pid;
    }
}


/* code copied from JRE 1.4 java_props_md.c */
char* sysGetOsName(void) {

   struct utsname name;
   uname(&name);
   return strdup(name.sysname);
}

/* code copied from JRE 1.4 java_props_md.c */
char* sysGetOsArch(void) {

  char arch[12];

#ifdef WIN32
  return "x86";
#endif

#ifdef LINUX
  return "i386";
#else
  sysinfo(SI_ARCHITECTURE, arch, sizeof(arch));
  if (strcmp(arch,"sparc") == 0 ) {
#ifdef _LP64
    return "sparcv9";
#else
    return "sparc";
#endif
  } else if (strcmp(arch,"i386") == 0 ) {
    /* use x86 to match the value received on win32 */
    return "x86";
  } else if (strcmp(arch,"ppc") == 0 ) {
    return "ppc";
#ifdef __linux__
  } else if (strcmp(arch,"m68k") == 0 ) {
    return "m68k";
#endif
  } else {
    return "Unknown";
  }
#endif
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
            /* Default location */
            Abort(getMsgString(MSG_BADINST_NOHOME));
        }
        initialized = TRUE;
    }
    return home;
}

/*
 * If app is "c:\j2se1.4.2\javaws\javaws.exe", 
 *    then put "c:\j2se1.4.2\bin\java" into buf. (if it exists)
 */
char* sysGetInstallJRE(void) {
    struct stat statBuf;
    static char jre[MAXPATHLEN];
    char nativeStr[100];
    static int  init1 = FALSE;
    char *ptr;
     
    if (!init1) {  
	strcpy(jre, sysGetApplicationHome());
	ptr = strrchr(jre, FILE_SEPARATOR);
	sprintf(nativeStr, "%s%c%s", "bin", FILE_SEPARATOR, "java"); 
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
    
    if (!initialized) {  
        initialized = TRUE;   
        
        { struct passwd *pwent = getpwuid(getuid());
          strcpy(userhome, pwent ? pwent->pw_dir : "");
        
          if (userhome[strlen(userhome)-1] == FILE_SEPARATOR) {
             userhome[strlen(userhome)-1] = '\0';
          }
        }
    }
    return userhome;
}

/*  Returns a tempoary filename. The string returned must be
 *  freed by the caller
 */
char* sysTempnam(void) {
  char* argFileName = tempnam(NULL, "javaws");
  return argFileName;
}

/*
 *  get extension for splash files
 */
char* sysGetSplashExtension() {
  return splashExtension;
}


/*
 *  case insensitive compare of two string
 */
int sysStrCaseCmp(char* s1, char* s2) {
    return strcasecmp(s1, s2);
}

char *sysGetJavawsbin() {
#ifdef _DEBUG
    return "javawsbin_g";
#else
    return "javawsbin";
#endif
}


static char _localeStr[64];
static int locale_initialized = FALSE;
char *sysGetLocaleStr() {

    /*
    ** The following code to get the locale string is taken mostly from java's:
    ** src/solaris/native/java/lang/java_props_md.c.
    */
    if (!locale_initialized) {
        char *p; 
        char *lc = setlocale(LC_CTYPE, "");
	char *javawsloc = getenv("_JAVAWS_LOCALE");
	if (javawsloc != NULL) {
	    strcpy(_localeStr, javawsloc);
	} else if (lc == NULL) {
            /*   
             * 'lc == null' means system doesn't support user's environment
             * variable's locale.
             */  
            strcpy(_localeStr, "C");
        } else { 
            /*
             * <language name>_<region name>.<encoding>@<varient>
             * we only want the first two if they are there:
             */  
            strcpy(_localeStr, lc);
            p = strchr(_localeStr, '.');
            if (p == NULL) {
                p = strchr(_localeStr, '@');
            }
            if (p != NULL) {
                *p = '\0';    /* chop off .<encoding>@<varient> or @<varient> */            }
        }
        if (sysStrCaseCmp(_localeStr, "C") == 0) {
            strcpy(_localeStr, "en_US");
        }
	locale_initialized = TRUE;
    }
    return _localeStr;
}

unsigned short *sysMBCSToWideChar(char *mbcs) {
  size_t len = strlen(mbcs);
  char *p = NULL;
  char *utf8 = NULL, *ucs2 = NULL;
  iconv_t cd;
  size_t oleft = MAXPATHLEN*2;

  /* get codeset (encoding) */
  setlocale(LC_ALL, "");
  p = nl_langinfo(CODESET);
  
  /* use Latin-1 encoding if we couldn't find a default one */
  if (p==NULL || p[0]=='\0') {
    p = "ISO8859-1";
  }
  
  if (strcmp(p, "UTF-8") != 0) {
    /* Convert from MB(locale specific) to UTF-8 , we use UTF-8 as a universal
       format from which other conversions can be done easily */
     
    cd = iconv_open("UTF-8", p);
    if (cd == (iconv_t)-1) {
      return NULL;
    }

    utf8 = (char *)malloc(MAXPATHLEN*2);
    if (utf8==NULL) 
      return NULL;

    memset(utf8, 0, MAXPATHLEN*2);

    p = utf8;
    
    if (iconv(cd, (const char**)&mbcs, &len, &p, &oleft) == (size_t)-1) {
      free(utf8);
      return NULL;
    } else {
      *p=0;
    }
    iconv_close(cd);
    
    len = (MAXPATHLEN*2) - oleft; 
  } else {
    /* we are in UTF-8 already! */
    utf8 = mbcs;
  }
      
  /* Convert from UTF-8 to the UCS-2 encoding */
  cd = iconv_open("UCS-2", "UTF-8");
  if (cd == (iconv_t)-1) {
    free(utf8);
    return NULL;
  }
  
  ucs2 = (char *)malloc(MAXPATHLEN*2);
  if (ucs2==NULL) {
    free(utf8);
    return NULL;
  }

  memset(ucs2, 0, MAXPATHLEN*2);
  
  p = ucs2;
  oleft = MAXPATHLEN*2;
  len = strlen(utf8);
  
  if (iconv(cd, (const char **)&utf8, &len, (char**)&p, &oleft) == (size_t)-1) {
    free(utf8);
    free(ucs2);
    return NULL;
  } else {
    *p = 0;
  }
  iconv_close(cd);

  return (unsigned short*)ucs2;
  
}


char *sysMBCSToSeqUnicode(char *mbcs) {
  unsigned short * stringU;
  char * output;
  
  stringU = sysMBCSToWideChar(mbcs);

  output = sysSaveConvert(stringU);
 
  free(stringU);
  return output;
}

char *sysWideCharToMBCS(twchar_t *message, size_t len) {
  char *p = NULL, *p2 = NULL;
  char *out = NULL, *out2 = NULL;
  iconv_t cd;
  size_t oleft = MAXPATHLEN*2;
  int i;
  size_t count = 0;
  size_t len2 = len*2;

  /* get codeset (encoding) */
  p = nl_langinfo(CODESET);

  /* use Latin-1 encoding if we couldn't find a default one */
  if (p==NULL || p[0]=='\0') {
    p = "ISO8859-1";
  }

  /* Convert from UCS-2 to UTF-8 , we use UTF-8 as a universal */
  /* format from which other conversions can be done easily */
  cd = iconv_open("UTF-8", "UCS-2");
  if (cd == (iconv_t)-1) {
    return NULL;
  }
  
  out = (char *)malloc(MAXPATHLEN*2);
  if (out==NULL) return NULL;
  p2 = out;
  if (iconv(cd, (const char **)&message, &len2, &p2, &oleft) == (size_t)-1) {
    free(out);
    out = NULL;
  } else {
    *p2=0;
  }
  iconv_close(cd);
  p2 = out;

  /* Convert from UTF-8 to the locale-specific encoding */
  if ((strcmp(p, "UTF-8") != 0) && (p2!=NULL)) {
    cd = iconv_open(p, "UTF-8");
    if (cd == (iconv_t)-1) {
      free(p2);
      return NULL;
    }
    
    out2 = (char *)malloc(MAXPATHLEN*2);
    if (out2==NULL) {
      free(p2);
      return NULL;
    }
    p2 = out2;
    len2 = (MAXPATHLEN*2) - oleft ;
    oleft = MAXPATHLEN*2;
    if (iconv(cd, (const char **)&out, &len2, &p2, &oleft) == (size_t)-1) {
      free(out);
      free(out2);
      out2 = NULL;
    } else {
      *p2 = 0;
    }
    iconv_close(cd);
    p2 = out2;
  }
  
  return p2;
}

/* argv MUST include cmd. */
void sysExec2Buf(char *cmd, int argc, char *argv[], char *buf, int *buflen) {
  int fdin[2], fdout[2], fderr[2], k, resultPid, n, ofs = 0, len, options;
  siginfo_t info;
  len = *buflen;
  *buflen = 0;
  if ((k=0, pipe(fdin)<0) || (k=1, pipe(fdout)<0) || (k=2, pipe(fderr)<0)) {
    switch (k) {
    case 2:	
      close(fdout[0]);
      close(fdout[1]);
    case 1:
      close(fdin[0]);
      close(fdin[1]);
    case 0: ;
    }
    return;
  }

#ifdef SOLARIS
  resultPid = fork1();
#else
  resultPid = fork();
#endif
  
  if (resultPid < 0) {
    /* fork error - make sure we clean up the pipes */
    close(fdin[1]);
    close(fdout[0]);
    close(fderr[0]);
    close(fdin[0]);
    close(fdout[1]);
    close(fderr[1]);
    return;
  }

  if (resultPid == 0) {
    /* 0 open for reading, 1 open for writing */
    /* (Note: it is possible for fdin[0] == 0 - 4180429) */

    dup2(fdin[0], 0);
    dup2(fdout[1], 1);
    dup2(fdout[1], 2);
    
    execv(cmd, argv);
  }
  
  /* parent process */

  /* Wait for the child process to exit.  This returns immediately if
     the child has already exited. */
#ifdef SOLARIS
  options = WEXITED;
#else
  options = 0;
#endif

  while (waitid(P_PID, resultPid, &info, options) != 0) {
    /* do nothing */
  }

  /* clean up the child's side of the pipes */
  close(fdin[0]);
  close(fdout[1]);
  close(fderr[1]);

  while ((n = read(fdout[0], &(buf[ofs]), len - ofs)) > 0) {
    ofs += n;
  }
  buf[ofs] = 0;
  *buflen = ofs-1;
  
  close(fdin[1]);
  close(fdout[0]);
  close(fdout[0]);
}

void sysGetRegistryJREs (JREDescription jre_list[], int *nJREs) {
  char *path = sysGetInstallJRE();
  if (path != NULL) {
      jre_list[0].path = strdup(path);
      *nJREs = 1;
  } else {
      *nJREs = 0;
  }
}

void sysCreateDirectory(char *dir) {
  mkdir(dir, S_IRWXG | S_IRWXU | S_IRWXO );
}

char *sysGetDeploymentUserHome() {
    static char deploymentUserHome[MAXPATHLEN];
    static int  initialized = FALSE;
    if (!initialized) {
	sprintf(deploymentUserHome, "%s/.java/.deployment",sysGetUserHome());
	initialized = TRUE;
    }
    return deploymentUserHome;
}

char *sysGetDeploymentSystemHome() {
    return "/etc/.java/.deployment";
}

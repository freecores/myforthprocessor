/*
 * @(#)launcher.c	1.95 03/02/19
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


/*
 * Java Web Start launcher
 * -----------------------
 *
 *    javaws [<options>] [<target> [<args>]]
 *    or
 *    javaws -splash port splash1 splash2
 *
 * All <options> start with '-'
 *
 * All <args>, <target>, and <options> are passed unmodified to com.sun.javaws.Main
 * (javaws.jar), *except* (if JPDA #define'd) -jpda... (see RFE 4462959: JPDA
 * compliant debugging extension to Java Web Start); the latter option may instead
 * be expanded into a number of other options, which are then passed to the JRE (java
 * executable) that runs com.sun.javaws.Main.
 *
 * If no JRE option is found, then one is selected from deployment.properties
 * using the default specification (see configurationFile.c).
 *
 */


#include "launcher.h"
#include "configurationFile.h"


/*
 * Entry point.
 */
int main(int argc, char** argv) {
  char* jreversion = NULL;
  char* jrelocation = NULL;
  int   jreindex = 0;
  char* vmargs     = NULL;
  char* javaexe    = NULL;
  int   copiedfile = FALSE;
  int   wait       = FALSE;
  char* initialHeap = NULL;
  char* maxHeap = NULL;
  char* splash1 = NULL;
  char* splash2 = NULL;
  /* added for single instance service */
  char* canonicalHome = NULL;
  char* jnlpfile = NULL;
  int   ret;
  int   bExists, bUpdated, bIsCurrent;
  int   isPlayer = FALSE;
  int   copiedFileIndex = -1;
  

#ifdef JPDA
  JPDA_OPTIONS_T  JPDA_OPTIONS  =  InitJpdaOptions(GetJpdaOptions());

  /* Load JPDA configuration file ("deployment.properties.jpda") into memory if
     it exists (returns NULL if no options are found) */
  LoadJpdaConfigurationFile();
  JPDA_OPTIONS->in_copy = JPDA_OPTIONS->in = GetJpdaCfgOptions();
#endif

  if ((argc >= 4) && (strcmp("-splash", argv[1]) == 0)) {
	int port = atoi(argv[2]);
	ret = sysSplash(port, argv[3], argv[4]);
	exit(0);
  }
	

  /* Platform-specific setup. On windows, this sets up the PATH environment to include
   * the application home. (On unix, this is done in a shell script) */
  LauncherSetup_md();
  
  /* Load configuration file into memory */
  bExists = LoadConfigurationFile();

  /* rescan system registry for new or removed JREs */
  bUpdated = RescanJREs();

  /* store user-specific configuration file changes resulting from
     rescan of registry */
  if (bUpdated || !bExists) {
    StoreConfigurationFile();
  }

  /* Scan arguments and jnlp file (if present) for options */
  ScanFileArgumentForOptions(argc, 
                             argv, 
			     &jnlpfile,
			     &canonicalHome,
                             &jreversion, 
                             &jrelocation, 
                             &vmargs, 
                             &copiedfile,
                             &splash1,
                             &splash2,
                             &initialHeap,
                             &maxHeap    _JPDA_OPTIONS,
			     &wait,
			     &isPlayer,
			     &copiedFileIndex);
 
  /* if browser downloaded jnlp file */
  if (jnlpfile != NULL && canonicalHome != NULL) {
    /* try to setup single instance service */   
    ret = SetupSingleInstance(jnlpfile, canonicalHome);   
    free(jnlpfile);
    free(canonicalHome);
    if (ret == 1) sysExit(copiedfile, copiedFileIndex, argv);
  } 

 
  /* A NULL jreversions implies we should use the default, otherwise map the
     the version string to a valid version and look up the location of the
     VM. */
  if (jreversion == NULL) {
      jreversion = getDefaultJREs(); /* pref is to use 1.4.2 */
      /* use default JRE location if nothing is specified */
      jrelocation = strdup(DEFHREF);
  }

#ifdef JPDA
  JPDA_OPTIONS->jreIndex =
#endif
  jreindex = DetermineVersion(jreversion, jrelocation);
  
   /* check EULA for win32 platforms */
  if (EULA_md(argc, argv, isPlayer) != 1) {
    sysExit(copiedfile, copiedFileIndex, argv);
  }
  
  LaunchJava(jreindex, 
             vmargs, 
             argc, 
             argv, 
             FALSE,
             wait,
             copiedfile,
	     splash1,
             splash2,
	     initialHeap,
	     maxHeap    _JPDA_OPTIONS);

  return 0;
}

int SetupSingleInstance(char *jnlpfile, char* canonicalHome) {
   
  FILE* sisIN = NULL;
  char* magicword = "javaws.singleinstance.init";
  char* eof = "EOF";
  char* ack = "javaws.singleinstance.ack";
  char sispath[MAXPATHLEN];
  char url[80];  
  int port;
  int ret;
  char* ret_s;
  SOCKET s;
  int i;
  char* sisProperties;
  int size;
     
  /* sis.properties is the single instance config file */
  sprintf(sispath, "%s%csis.properties", sysGetDeploymentUserHome(), FILE_SEPARATOR);
      
  /* read the sis.properties file */
  size = ReadFileToBuffer(sispath, &sisProperties);

  if (sisProperties != NULL) {
 
    /* parse the sis.properties file line by line*/
    for (ret_s = strtok(sisProperties, "\n") ; ret_s != NULL; ret_s = strtok(NULL, "\n")) {
      sscanf(ret_s, "%s %d", &url, &port);
    
      /* check if the single instance server for this jnlp file is running */
      if (strcmp(url, canonicalHome) == 0) {

	/* talk to single instance server */
	s = sysCreateClientSocket(port); 	
	if (s == INVALID_SOCKET) break;
	
	/* send MAGICWORD */
	ret = sysWriteSocket(s, magicword); 		  
	if (ret == 0) break;
	  
	/* send over the jnlp file */
	ret = sysWriteSocket(s, jnlpfile);
	if (ret == 0) break;
		
	/* incicate EOF */
	ret = sysWriteSocket(s, eof); 
	if (ret == 0) break;
	
	/* wait for ack from server */
	/* try read socket for 5 times - do we need to retry at all? */
	for (i = 0; i< 5; i++) {
	  ret_s = sysReadSocket(s);
	  
	  if (ret_s != NULL && strcmp(ret_s, ack) == 0) {	   
	    /* if acked, exit program */
	    sysCloseSocket(s);
	    return 1;
	  }		
	}
	sysCloseSocket(s);
      }
    }          
  }
  return 0;
}

/* ------------------------------------------------------------------------------ */
/* Launching of Java  */

void LaunchJava(int jreIndex, char* vmoptions, int argc, char** argv, int replace, int wait, int copiedFile, char* splash1, char *splash2, char *initialHeap, char *maxHeap    _JPDA_OPTIONS_T JPDA_OPTIONS) {
  char* vmargs[50]; /* A 50 seems to be more than enough */
  char** javaargv  = NULL;
  int nofvmargs = 0;
  int totalargs;
  int i, no;
/* int foundFirst = FALSE;	//  CL: clean up (not used) */
  char* javaexe;
/* char **oldjavaargv;		//  CL: clean up (not used) */
  char *savedoptions = NULL;
#ifdef JPDA
  int no_jpdaEnv;	/* index of JPDA environment arg */
  static char buf[MAXPATHLEN];
  JavaMain* javaMain = InitJavaMain(GetJavaMain(), argc);
#endif

  javaexe = GetJREJavaCmd(jreIndex);
  if (javaexe == NULL) {
      /* This should never happen, since we have already checked if it exists */
      Abort(getMsgString(MSG_BADINST_NOJRE));
  }
  
  if (splash1 != NULL) {
      ShowSplashScreen(splash1, splash2);
  }

  /* Convert vmoptions into an array of strings (note: if JPDA is #define'd,
   * ConvertVmArgs() also calls "jpda.c":DecodeJpdaOptions() to decode
   * JPDA_OPTIONS, which -- if JPDA == JWS or JPDA == JWSJNL -- will be-
   * expanded into a number of additional VM options): */
  if (vmoptions) savedoptions = strdup(vmoptions);
  nofvmargs = ConvertVmArgs(vmoptions, vmargs, initialHeap, maxHeap, 50    _JPDA_OPTIONS);

  /* Build argument array for java launcher */
  totalargs = argc + nofvmargs + (vmoptions ? 1 : 0) +
#ifdef JPDA
				  15;	/* we need extra arg for JPDA env */
#else
				  14;
#endif

  javaargv = (char**)malloc(totalargs * sizeof(char*));
  no = 0;
  javaargv[no++] = javaexe;  /* Arg 0 is always the program to execute */
  for(i = 0; i < nofvmargs; i++) {
    javaargv[no++] = sysQuoteString(vmargs[i]);
  }

  javaargv[no++] = GetBootClassPathArg();             /* Extra argument 1 */
  javaargv[no++] = "-classpath";		      /* Extra argument 2 */
  javaargv[no++] = GetNonBootClassPath();             /* Extra argument 3 */
  javaargv[no++] = GetJnlpxHomeOption();              /* Extra argument 4 */
  javaargv[no++] = GetJnlpxSplashPortOption();        /* Extra argument 5 */
  javaargv[no++] = GetJnlpxJVMOption(javaexe);        /* Extra argument 6 */
  javaargv[no++] = GetJnlpxRemoveOption(copiedFile);  /* Extra argument 7 */
  javaargv[no++] = GetSecurityPolicyOption();         /* Extra argument 8 */
  javaargv[no++] = GetTrustProxyOption();             /* Extra argument 9 */
  javaargv[no++] = GetHeapSizeOption(initialHeap, maxHeap);/* argument 10 */
  javaargv[no++] = GetJnlpxDeploymentSystemHome();   /* Extra argument 11 */
  javaargv[no++] = GetJnlpxDeploymentUserHome();     /* Extra argument 12 */
  if (savedoptions) javaargv[no++] = GetVMArgsOption(savedoptions); /* 13 */
#ifdef JPDA
  no_jpdaEnv = no++;		    /* do later: Extra argument 13  or 14 */
#endif
  javaargv[no++] = "com.sun.javaws.Main";  /* Extra argument 13, 14 or 15 */

  for(i = 1; i < argc; i++) {
#ifdef JPDA
	  /* If argv[i] represents a valid JPDA option (see function
	     ScanFileArgumentForOptions()), it isn't valid as an argument
	     to "com.sun.javaws.Main"; instead (see ConvertVmArgs()), the
	     JPDA option is decoded and the result passed as argument(s)
	     to the JRE ("java" executable) that is being invoked to launch
	     "com.sun.javaws.Main": */
	if (i == JPDA_OPTIONS->cmdLineArgIndex) {
		totalargs--;	/* to satisfy forthcoming assert() */
	}
	else {
		javaargv[no] = sysQuoteString(argv[i]);
		sprintf(buf, "%s,", javaargv[no++]);
		strcat(javaMain->args, buf);
	}
  }
  /* Next, get rid of the trailing ',': */
  if ((i = strlen(javaMain->args)) > 0) {
	javaMain->args[i-1] = '\0';
  }
#else
	javaargv[no++] = sysQuoteString(argv[i]);
  }
#endif

  javaargv[no++] = NULL;                        /* Extra argument 16 */
  assert(totalargs == no, "wrong memory allocation");

#ifdef JPDA
  /* Now do  Extra argument 11:
     Pass the JPDA debugging mode environment on to the invoked "java"
     executable, containing: (1) the necessary information needed to,
     if necessary, recursively launch a new "java" process in JPDA
     debugging mode (using the next available port); (2) some diagnostic
     information for a JPDA debugging mode notification to the user: */
	javaargv[no_jpdaEnv] = GetJpdaEnvOption(JPDA_OPTIONS, javaMain);
#if (JPDA==JWS || JPDA==JWSJNL)
  /* If we are in JPDA mode, notify user before spawning the Java launcher: */
 	if (JPDA_OPTIONS->jpdaMode)
		ShowJpdaNotificationWindow(
				javaargv[no_jpdaEnv], jreIndex);
#endif
#endif

  if (getenv("JAVAWS_TRACE_NATIVE") != NULL) {
    int i;
    char message[MAXPATHLEN];
    strcpy(message, "Launching: ");
    for(i=0;i < no; i++) {
      strcat(message, (javaargv[i] == NULL ? " " : javaargv[i]));
      strcat(message, " ");
    }
    strcat(message, "\n");
    sysMessage(message);
  }

  /* Spawn the java launcher */
  /* NOTE: We don't really want to ever use SYS_EXEC_REPLACE on Unix.
     Doing a replace causes searching for JREs to fail because of
     bug 427214.
  */
  if (replace) {
      if (sysExec(SYS_EXEC_REPLACE, javaexe, javaargv) == -1) {
          char *msg = malloc(2*MAXPATHLEN); 
          sprintf(msg, "%s \n%s",getMsgString(MSG_BADINST_EXECV), javaexe); 
	  Abort(msg);
      }
  }
  else if (wait) {
      if (sysExec(SYS_EXEC_WAIT, javaexe, javaargv) == -1) {
          char *msg = malloc(2*MAXPATHLEN); 
          sprintf(msg, "%s \n%s",getMsgString(MSG_BADINST_EXECV), javaexe); 
	  Abort(msg);

      }
  }
  else {
      if (sysExec(SYS_EXEC_FORK, javaexe, javaargv) == -1) {
          char *msg = malloc(2*MAXPATHLEN); 
          sprintf(msg, "%s \n%s",getMsgString(MSG_BADINST_SYSEXE), javaexe); 
	  Abort(msg);
      }
  }
}
 
char *GetVMArgsOption(char *vmoptions) {
  static char option[MAXPATHLEN];
  sprintf(option, "-Djnlpx.vmargs=\"%s\"", vmoptions);
  return strdup(option);
}

char *GetHeapSizeOption(char *initialHeap, char *maxHeap) {
  static char option[MAXPATHLEN];
  sprintf(option, "-Djnlpx.heapsize=%s,%s",
		  (initialHeap == NULL ? "NULL" : initialHeap), 
		  (maxHeap == NULL ? "NULL" : maxHeap));
  return option;
}

/* Get the arg for -XBootclasspath */
char* GetBootClassPathArg(void) {
  static char bootclasspatharg[MAXPATHLEN];
  sprintf(bootclasspatharg, "-Xbootclasspath/a:%s",GetBootClassPath());
  return bootclasspatharg;
}

char* GetBootClassPath(void) {
  static char bootclasspath[MAXPATHLEN];
  sprintf(bootclasspath, "%s%c%s", 
		sysGetApplicationHome(), FILE_SEPARATOR, "javaws.jar");
  return bootclasspath;
}

char * GetNonBootClassPath(void) {
  static char nonbootclasspath[MAXPATHLEN];
  sprintf(nonbootclasspath, "%s%c%s", 
		sysGetApplicationHome(), FILE_SEPARATOR, "javaws-l10n.jar");
  return nonbootclasspath;
}

/* get the entire classpath (boot, + regular) */
char * GetClassPath(void) {
  static char classpath[MAXPATHLEN];
  sprintf(classpath, "%s%c%s", 
	  GetBootClassPath(), PATH_SEPARATOR, GetNonBootClassPath());
  return classpath;
}

char* GetJnlpxDeploymentSystemHome(void) {
  static char option[MAXPATHLEN];
  sprintf(option, "-Djnlpx.deployment.system.home=%s",
		  sysGetDeploymentSystemHome());
  return option;
}

char* GetJnlpxDeploymentUserHome(void) {
  static char option[MAXPATHLEN];
  sprintf(option, "-Djnlpx.deployment.user.home=%s",sysGetDeploymentUserHome());
  return option;
}

char* GetJnlpxHomeOption(void) {
  static char option[MAXPATHLEN];
  sprintf(option, "-Djnlpx.home=%s", sysGetApplicationHome());
  return option;
}

char* GetJnlpxCmdOption(char* javawscmd) {
  static char option[MAXPATHLEN];
  sprintf(option, "-Djnlpx.cmd=%s", javawscmd);
  return option;
}

char* GetJnlpxSplashPortOption() {
  static char option[MAXPATHLEN];
  sprintf(option, "-Djnlpx.splashport=%d", GetSplashPort());
  return option;
}

char* GetJnlpxJVMOption(char* path) {
  static char option[MAXPATHLEN];
  sprintf(option, "-Djnlpx.jvm=%s",   sysQuoteString(path));
  return option;
}

char* GetJnlpxRemoveOption(int copiledFile) {
  static char option[MAXPATHLEN];
  sprintf(option, "-Djnlpx.remove=%s", (copiledFile) ? "true" : "false");
  return option;
}

char* GetSecurityPolicyOption(void) {
  static char option[MAXPATHLEN];
  sprintf(option, "-Djava.security.policy=file:%s/javaws.policy", sysGetApplicationHome());
  return option;
}

char* GetTrustProxyOption(void) {
  static char option[MAXPATHLEN];
  sprintf(option, "-DtrustProxy=true");
  return option;
}

char* GetSystemClassLoaderOption(void) {
  static char option[MAXPATHLEN];
  sprintf(option, "-Djava.system.class.loader=com.sun.jnlp.JNLPClassLoader");
  return option;
}

/* Convert a string of arguments into a list of arguments */
int ConvertVmArgs(char *vmoption, char **args, char *initialHeap,
		  char *maxHeap, int maxargs    _JPDA_OPTIONS_T JPDA_OPTIONS) {
  int no = 0;

#ifdef JPDA
  /* Decode JPDA_OPTIONS->in.  NOTE: In case this decoding leads to the
     "-classic" switch to the "java" executable (spawned by LaunchJava()),
     we're passing this switch as the 1st argument, otherwise the "java"
     command may issue the error message  "Unrecognized option: -classic.
     Could not create the Java virtual machine"  and exit. */

  DecodeJpdaOptions(JPDA_OPTIONS);

#if (JPDA==JWS || JPDA==JWSJNL)
  if (JPDA_OPTIONS->jpdaMode) {
	if (JPDA_OPTIONS->classic) {
		args[no++] = "-classic";
		args[no++] = "-Xnoagent";
		args[no++] = "-Djava.compiler=NONE";
	}
	args[no++] = "-Xdebug";
	args[no++] = JPDA_OPTIONS->out;
  }
#endif
#endif
  
  if (initialHeap != NULL) {
	char *str = malloc(8+strlen(initialHeap));
	sprintf(str, "-Xms%s",initialHeap);
	args[no++] = str;
  }

  if (maxHeap != NULL) {
	char *str = malloc(8+strlen(maxHeap));
	sprintf(str, "-Xmx%s",maxHeap);
	args[no++] = str;
  }

  if (vmoption == NULL) return no;

  /* Scan through options, and convert it to multiple '\0\ terminated
     strings */
  while(*vmoption) {
    /* Skip leading withspace */
    while(iswspace(*vmoption)) vmoption++;
    if (*vmoption) {
      args[no++] = vmoption;
      /* skip argument */
      while(*vmoption && !iswspace(*vmoption)) vmoption++;
      if (*vmoption) *vmoption++ = '\0';
      /* Reach limit - just skip the rest */
      if (no >= maxargs) return no;
    }
  }

  return no;
}

/* ------------------------------------------------------------------------------ */
/* Parsing of commandline and JNL file */

void ScanFileArgumentForOptions(int argc, char **argv, char** jnlpfile, char** canonicalHome, char** jreversion, char** jrelocation, char** argsoption, int *copiedfile, char** splash1, char **splash2, char **initialHeap, char **maxHeap    _JPDA_OPTIONS_T JPDA_OPTIONS, int *wait, int *isPlayer, int *copiedFileIndex) {
  int isUninstall;
  int isUpdateConfig;
  int skipSplash;
/*char* jdkid = NULL;		//  CL: clean up (not used) */
  char* filename = NULL;
  char *copyfilename;
  int i, len, size;
  int filenameIndex = -1;
  char* jnlbuffer;
  JNLFile* jnlfile;
  char *argsfromenv;

  isUninstall = FALSE;
  isUpdateConfig = FALSE;
  *copiedfile = FALSE;

  /* Splash mode can be overridden in configuration file */
  skipSplash = (!isSplashScreenEnabled());

  /* Do not overwrite commandline options */
  if (*jreversion != NULL && *argsoption != NULL) return;

  if ((argsfromenv = getenv("JAVAWS_VM_ARGS")) != NULL) {
    *argsoption = malloc(strlen(argsfromenv) + 1);
    strcpy(*argsoption, argsfromenv);
  }

  /* Find first non-option argument */
  for(i = 1; i < argc && filename == NULL; i++) {
    len = strlen(argv[i]);
    if (len > 0) {
      if (*argv[i] != '-') {
        filenameIndex = i;
        filename = argv[i];
	if (filename[0] == '@') {
	  filename = ReadFileNameFromFile(filename + 1);
	  if (filename == NULL) {
	    /* Couldn't obtain file name, treat as normal file. */
	    filename = argv[i];
	  }
	}
      }
#ifdef JPDA
      else if (!strncmp("-jpda", argv[i], 5)) {
	  HandleJpdaCommandLineArg(argv[i], i, JPDA_OPTIONS);
      }
#endif
      else if (!strcmp("-uninstall", argv[i])) {
	  isUninstall = TRUE;
	  skipSplash = TRUE;
          *wait = TRUE;
      }
      else if (!strcmp("-updateVersions", argv[i])) {
	  isUpdateConfig = TRUE;
	  skipSplash = TRUE;
          *wait = TRUE;
      } 
      else if (!strcmp("-Xnosplash", argv[i])) {
	  skipSplash = TRUE;
      }
    }
  }

  if (skipSplash) {
     *splash1 = NULL;
     *splash2 = NULL;
  } else {
      getDefaultSplashFiles((filename == NULL), splash1, splash2);
  }

  /* No file specified, thus no jdk specified */
  if (filename == NULL) return;

  /* If uninstalling, no need to copy for uninstall, and no need to parse
   * file for JRE to use, can use default.
   */
  if (isUninstall && filename != NULL) {
      argv[filenameIndex] = filename;
      return;
  }

  /* Load, copy, & parse the specified JNL file */
  size = ReadFileToBuffer(filename, &jnlbuffer);
  if (jnlbuffer != NULL) {

      /* Create a copy of the file too */
      copyfilename = sysTempnam();
      if (!SaveBufferToFile(copyfilename, jnlbuffer, size)) {
          /* Ignore error - just did not make a copy */
          Message("Failed to copy argument");
          Message(copyfilename);
      } else {
          /* Update arguments to have a reference to the copy */
          argv[filenameIndex] = copyfilename;
          *copiedfile = TRUE;
          *copiedFileIndex = filenameIndex;
      }

      /* Extract jre version out of option file */
      if (isUTF8(jnlbuffer, size)) {
          jnlfile = ParseJNLFile(jnlbuffer);
      } else {
         /* arg is non utf-8 - cannot parse */
         jnlfile = ParseJNLFile(NULL);
         jnlfile->isPlayer = FALSE;
      }

      *jnlpfile = strdup(jnlbuffer);

      if (jnlfile->canonicalHome != NULL) {
	*canonicalHome = strdup(jnlfile->canonicalHome);
      }
  } else {
      /* arg is http url, not file - cannot read or parse */
      jnlfile = ParseJNLFile(NULL);
      jnlfile->jnlp_url = filename;    
      jnlfile->isPlayer = FALSE;
  }

  /*
   * we may wish to add AuxArgs from the jnlfile to the *argsoption string
   */
  if (jnlfile->auxArgCount > 0) {
	int i;
	char *new;
	int totalSize;
	for (i=0; i<jnlfile->auxArgCount; i++) {
	    if (jnlfile->auxArg[i] != NULL) {
		if (*argsoption == NULL ) {
			*argsoption = "";
		}
	        totalSize = strlen(*argsoption) + strlen(jnlfile->auxArg[i]) + 4;
		new = malloc(totalSize);
		sprintf(new, "%s -D%s", *argsoption, jnlfile->auxArg[i]);
		*argsoption = new;
	    }
	}
  }
  
  if (jnlfile->jreVersion != NULL) {
      if (*jreversion != NULL) free(*jreversion);
      if (*jrelocation != NULL) free(*jrelocation);
      
      *jreversion  = strdup(jnlfile->jreVersion);
      if (jnlfile->jreLocation != NULL) {
          *jrelocation = strdup(jnlfile->jreLocation);
      }
  }

  if (jnlfile->initialHeap != NULL) {
      *initialHeap = strdup(jnlfile->initialHeap);
  }
  if (jnlfile->maxHeap != NULL) {
      *maxHeap = strdup(jnlfile->maxHeap);
  }
  if (!skipSplash) {
      if (jnlfile->canonicalHome != NULL) {
          getAppSplashFiles(jnlfile, splash1, splash2);
      } else {
          getDefaultSplashFiles(jnlfile->isPlayer, splash1, splash2);
      }
  }
  
  if (jnlfile->isPlayer) {
      *isPlayer = TRUE;
  }

  /* Free allocated memory */
  FreeJNLFile(jnlfile);
  free(jnlbuffer);
}


/* Reads the filename to load from from the passed in file. It is the callers
   responsibility to free the returned string. This will return null if
   there is a problem determining the filename.
*/
char *ReadFileNameFromFile(char *file) {
  char *contents ;
  int size = ReadFileToBuffer(file, &contents);

  if (contents != NULL) {
    char *ptr = contents;

    while (*ptr != '\r' && *ptr != '\n' && *ptr != '\0') {
      ptr++;
    }
    *ptr = '\0';
  }
  return contents;
}
 
/* ------------------------------------------------------------------------- */
/* Splash Screen */


static int SplashPort = -1;
static int SplashPID = -1;


int GetSplashPort() {
    return SplashPort;
}

void ShowSplashScreen(char* splash1, char *splash2) {

    int port;      /* the port that splash process will callback on */
    SOCKET server; /* the socket that splash process will callback on */

    /* Create a temporary socket on an ephemeral port.  We'll pass this
     * port to the splash application (as a command line argument)
     * so that it can send us the ephemeral port it's going to use.
     */
    if ((server = sysCreateListenerSocket(&port)) == INVALID_SOCKET) {
	Abort(getMsgString(MSG_LISTENER_FAILED));
    }

    /* Launch the splash screen application with the callback port and
     * splash screen jpeg file to use as it's arguments
     */
    {
	char str[32], *argv[8];
	char exe[MAXPATHLEN];
        
	sprintf(exe, "%s%c%s", sysGetApplicationHome(), FILE_SEPARATOR, sysGetJavawsbin());

	sprintf(str, "%d", port);

	argv[0] = "JavaWSSplashScreen";
	argv[1] = "-splash";
        argv[2] = str;
	argv[3] = splash1;
	argv[4] = splash2;
	argv[5] = NULL;

	sysExec(SYS_EXEC_FORK, exe, argv);
    }

    /* Wait for the splash screen to connect and tell us what ephemeral
     * port it's listening on.
     */
    {
	SOCKADDR_IN iname = {0};
	int client, length = sizeof(iname);
	char data[6];

	if ((client = accept(server, (SOCKADDR *)&iname, &length)) == INVALID_SOCKET) {
	    Abort(getMsgString(MSG_ACCEPT_FAILED));
	}
	if (recv(client, data, 6, 0) != 6) {
	    Abort(getMsgString(MSG_RECV_FAILED));
	}
	if ((SplashPort = atoi(data)) <= 0) {
	    Abort(getMsgString(MSG_INVALID_PORT));
	}

	sysCloseSocket(client);
	sysCloseSocket(server);
    }
}


/*
 * @(#)CJavaVM.cpp	1.68 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*  
 * The CJavaVM class represents our connect to the actual VM.  It is
 * created, intialized and then started, which spawns off the VM
 * process.  The basic execution structure is the
 * same as that in the original plugin 1.1.       BG Sept 98
 *                                      
 */

/*
 * Support for Cookies, Proxies, Caching, Javascript (pre 5.0),
 * Status, show document and https are provided by calls into the
 * browser by the Worker.
 *    Status, show document and post https are simple calls that require
 *    no return value.
 * 
 *    Cookies, Proxies, Caching, Javascript and get https require a
 *    return value.
 *
 */
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/param.h>		// For MAXPATHLEN
#include <fcntl.h>
#include <stropts.h>
#include <string.h>
#include <stdarg.h>
#include <poll.h>
#include <dlfcn.h>
#include <limits.h>

#ifdef LINUX
#include <linux/limits.h>
#include <linux/types.h>
#include <linux/dirent.h>
#define _DIRENTRY_H
#else
#include <ulimit.h>
#endif
#include <dirent.h>

#include <dlfcn.h>
#include <locale.h>
#include <libintl.h>

#include "jni.h"
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Vendor.h>
#include <X11/Shell.h>

#include "Navig4.h"
#include "pluginversion.h"

extern "C" {
#include <assert.h>
}


/*
 * Hold values of environment variables. These are setup by 
 * SetupChildEnvironment before the fork, but are not actually
 * putenv'ed. After the fork they are putenv'ed into the child.
 * In this manner, the startup of the child does not involve
 * the mallocs and other code required to set up these environment
 * variables and the exec is close to the fork, to avoid
 * potential deadlocks due to locks held by other threads after
 * the fork1 and before the exec. See the fork1 man page.
 */
static char* env_LD_LIBRARY_PATH;
static char* env_JAVA_HOME;
static char* env_PLUGIN_HOME;

/* Type of the init_invoke function */
typedef void (*init_fn_type)(void* );

/*
NS_DEFINE_IID(kNetworkManagerIID, NS_INETWORKMANAGER_IID);
*/

/* This is the callback that gets invoked when there is data to
   be read on the worker pipe
*/
static void
child_event_handler(XtPointer data, int *fid, XtInputId *id) 
{
    CJavaVM *vm = (CJavaVM*) data;
    vm->ProcessWorkQueue();
    UNUSED(fid);
    UNUSED(id);
}

/*  
 * Create a new JavaVM, associated with the plugin factory 'plugin'
 * (an instance of NPIPlugin).  Initialize and then StartVM must be called.
 */
CJavaVM::CJavaVM(CJavaPluginFactory *plugin) {
    state = new LongTermState();
    sprintf(stateENVName,"%s%s","JAVA_PLUGIN_STATE",PLUGIN_NODOTVERSION);
    mPlugin = plugin;

    // Initialize the state
    memset(state, 0, sizeof(struct LongTermState));

    mStatus = nsJVMStatus_Enabled;
}

/*  
 * Destroy the VM.
 */
CJavaVM::~CJavaVM() {
    trace("Destroying CJavaVM\n");
    //delete(state);
}


/*
 * Connect the pipe to its handler
 * its attached handler (child_handle_event)
 */
void CJavaVM::ConnectPipeEvent(int fd) {

    Display *display = (Display *) NULL;

    trace("CJavaVM::ConnectPipeEvent\n");

    mPlugin->GetPluginManager()->GetValue(nsPluginManagerVariable_XDisplay,
					  &display);

    if (display == NULL)
      plugin_error("Could not open display!");

    XtAppContext apctx = XtDisplayToApplicationContext(display);
    if (apctx == NULL)
      plugin_error("Could not obtain application context!");

    state->inputid = XtAppAddInput(apctx,fd,
                                   (XtPointer) XtInputReadMask,
                                   child_event_handler,this); 
}

/*
 * The following fns handle tracing messages after the fork1 
 * and before the exec. Avoid anything fancy, mallocs etc.
 */
static FILE *plugin_java_fork_trace;

static void fork_trace_init(void) {
    if (getenv("JAVA_PLUGIN_TRACE")) {
	plugin_java_fork_trace = fopentrace("/tmp/plugin_java_fork"PLUGIN_NODOTVERSION"_");
    }
}

static void fork_trace(const char *msg, int i) {
    if (plugin_java_fork_trace != NULL) {
	fprintf(stderr, " Fork trace: %s %s %d\n",PLUGIN_VERSION, msg, i);
	fprintf(plugin_java_fork_trace, "%s %d\n", msg, i);
	fflush(plugin_java_fork_trace);
    }
}

/*
 * Start the actual VM, using the class path 'addClasspath'
 * in addition to anything else determined to be in the class path.
 * Determine the location of the JRE, determine the child's
 * environment (set up the env_* strings) and then do the
 * fork1. Then putenv the env_* strings and do an exec of
 * java_vm or java_vm_native
 */
nsresult CJavaVM::StartJavaVM(const char *addClasspath)
{

    UNUSED(addClasspath);

    trace("CJavaVM::StartJavaVM\n");

    int rc;
    int command_fds[2];
    int work_fds[2];
    int print_fds[2];
    char *buff;
    const char *cbuff;
    char *env_JAVA_PLUGIN_AGENT;
    const char *progname;
    const char *agent = "No agent";
    nsresult res;
#ifndef __linux__
    char domain_path[TEXTDOMAINMAX];
#else
    char domain_path[256];
#endif

    trace("CJavaVM::StartJavaVM\n");
    
    if (NS_OK != (res = mPlugin->GetPluginManager()->UserAgent(&agent))) {
	return res;
    }


    trace("Getting JAVA_PLUGIN_STATE\n");
    
    // Check if we already have an Java child process.
    // We use the Unix environment to locate our saved state
    cbuff = getenv(stateENVName);
    if (cbuff == NULL) {
	cbuff = "";
    }
    rc = sscanf(cbuff, "%X", &state);
    if (rc == 1 && state != 0) {
	// We already have an active connection. 
	trace("reusing child process\n");
	mStatus = nsJVMStatus_Running;
	trace("Status is running... reusing child\n");
        ConnectPipeEvent(state->work_pipe);
        ProcessWorkQueue(); 
	return NS_OK;
    }

    // Allocate and record a state truct.
    buff = (char *)malloc(140);
    sprintf(buff, "%s=%X", stateENVName,state);
    putenv(buff);

    env_JAVA_PLUGIN_AGENT = (char *)malloc(40 + slen(agent));
    sprintf(env_JAVA_PLUGIN_AGENT, "JAVA_PLUGIN_AGENT=%s", agent);


    // Try to figure our what version of Netscape we're in 
    trace("AGENT = %s\n", agent);

    // Find directory where plugin JRE is installed 
    FindJavaDir();
    if (state->java_dir == NULL) {
	fprintf(stderr, "%s\n",
		DGETTEXT(JAVA_PLUGIN_DOMAIN, 
			 "Plugin: can't find plugins/../java directory"));
	trace("Could not find jre directory\n");
	return NS_ERROR_NOT_AVAILABLE;
    }

    trace("Java directory = %s\n", state->java_dir);

    // Let dgettext mechanism know where to find the files.
    if (state->java_dir != NULL) {
	setlocale(LC_ALL, "");
	sprintf(domain_path, "%s/lib/locale", state->java_dir);
	bindtextdomain(JAVA_PLUGIN_DOMAIN, domain_path);
    }

    /* Set up the child environment variables. Wait till after the
       fork to actually put them into the environment.*/
    SetupChildEnvironment();

#if defined(DEBUG_VERSION)
    progname = "java_vm_g";
#else
    progname = "java_vm";
#endif

    // Allocate a pipe to send commands to our child 
    rc = s_pipe(command_fds);
    if (rc != 0) {
	fprintf(stderr, "command pipe failed: %d\n", errno);
	return NS_ERROR_OUT_OF_MEMORY;
    }
    trace("Allocated first pipe\n");
    // Allocate a pipe to receive work from our child 
    // Old: rc = socketpair(AF_UNIX, SOCK_STREAM, 0, work_fds);
    rc = s_pipe(work_fds);
    if (rc != 0) {
	fprintf(stderr, "work pipe failed: %d\n", errno);
	return NS_ERROR_OUT_OF_MEMORY;
    }

    trace("Connecting worker-pipe to event\n");
    ConnectPipeEvent(work_fds[0]);

    rc = s_pipe(print_fds);
    if (rc != 0) {
	fprintf(stderr, "print pipe failed: %d\n", errno);
	return NS_ERROR_OUT_OF_MEMORY;
    }

    char *fullprogname = (char *)malloc(100 + slen(state->java_dir));
    sprintf(fullprogname, "%s/bin/%s", state->java_dir, progname);

    trace("Ready to fork/exec %s\n", fullprogname);
   
    // Create our child java process 
#ifdef __linux__
    rc = fork();
#else
    rc = fork1();
#endif
    if (rc == -1) {
        fprintf(stderr, "Fork failed: %d\n", errno);
	return NS_ERROR_OUT_OF_MEMORY;
    }

    if (rc == 0) {
	// Child process.  Prepare to exec. 
	int i, max;
	
	fork_trace_init();
	fork_trace("work_fds", work_fds[1]);
	fork_trace("command_fds", command_fds[1]);
	fork_trace("Print_pipe",print_fds[1]);
	putenv(env_JAVA_PLUGIN_AGENT);
	putenv(env_LD_LIBRARY_PATH);
	putenv(env_JAVA_HOME);
	putenv(env_PLUGIN_HOME);
	char* ld_flags = getenv("LD_DEBUG");
	if (ld_flags != NULL) {
	    putenv((char *) "LD_DEBUG=libs,detail");
	}

	// Make sure that we don't dup into existing file descriptors 
	// This is done rather crudely and can lead to failures in 
	// unlikely cases. For a more robust approach see the Navig5
	// version which dups all the descriptors out of the way first
        if (work_fds[1] == COMMAND_FD) {
            fork_trace("Using FD workaround for work fd 1", work_fds[1]);
            if ((work_fds[1] = dup2(work_fds[1], COMMAND_FD+5)) < 0){
                fork_trace("work workaround failed ", errno);
            }
        }

        if ((print_fds[1] == COMMAND_FD) || (print_fds[1] == WORK_FD)) {
            fork_trace("Using FD move (2) for print fd 1", print_fds[1]);
            if ((print_fds[1] = dup2(print_fds[1], COMMAND_FD + 6)) < 0){
                fork_trace("print workaround failed ", errno);
            }
        }

        if ((dup2(command_fds[1], COMMAND_FD)) < 0){
            fork_trace("command dup2 failed ", errno);
        }

        if (dup2(work_fds[1], WORK_FD) < 0) {
            fork_trace("worker dup2 failed ", errno);
        }

	// The others have been reassigned, so PRINT_FD is safe 
        if ((dup2(print_fds[1], PRINT_FD)) < 0){
            fork_trace("print dup2 failed ", errno);
        }

        // Clear the close-on-exec flags for stdio and our pipe. 
        for (i = 0; i < 3; i++) {
            fcntl(i, F_SETFD, 0);
        }
        fcntl(COMMAND_FD, F_SETFD, 0);
        fcntl(COMMAND_FD+1, F_SETFD, 0);
        fcntl(PRINT_FD, F_SETFD, 0);
	
	// Close all other file descriptors. 
#ifndef __linux__
	max = ulimit(UL_GDESLIM);
#else
        max = sysconf(_SC_OPEN_MAX);
#endif
	for (i = 3; i <= max; i++) {
	    if (i != COMMAND_FD && i != WORK_FD && i!= PRINT_FD) {
	        close(i);
	    }
	}
	fork_trace("Closed fds...", max);
	if (tracing) {
	    execlp(fullprogname, progname, "-t", 0);
	} else {
	    execlp(fullprogname, progname, 0);
	}
	fprintf(stderr, "Exec of %s failed: %d\n", fullprogname, errno);
	fork_trace("Exec failed!", errno);
	exit(6);
    }
    if (tracing) trace("Continuing in parent process....\n");
    state->child_pid = rc;
    state->command_pipe = command_fds[0];
    close(command_fds[1]);
    state->work_pipe = work_fds[0];
    close(work_fds[1]);
    state->print_pipe = print_fds[0];
    close(print_fds[1]);

    // As a safety measure, read from the command pipe.
    // View this as an assertion that both pipes are ok
    {
	int cmdres = -1;
	while(cmdres == -1){
	    sleep(1);
	    char buf[4];
	    trace("Parent before command read\n");
	    cmdres = read(command_fds[0],buf,1);
	    trace("Parent command read result = %d %d\n", cmdres, 
		  (int)buf[0]);
	}
    }
    mStatus = nsJVMStatus_Running;
    return NS_OK;
}

/*
 * Receive the printed data from the VM and insert it into the
 * printstream
 */
void CJavaVM::ReceivePrinting(FILE *fp) {
     int n;
     int finished = 0;
     int timeout = -1;
     char buf[1024];
     struct pollfd pfd[2];
     // In a loop, read the next data from the print pipe.
     // Continue in the loop until we receive an input of 
     // 'ok' on the command pipe. 
     pfd[0].fd = state->command_pipe;
     pfd[0].events = POLLIN;
     pfd[1].fd = state->print_pipe;
     pfd[1].events = POLLIN;
     for (;;) {
       if (poll(pfd, 2, timeout) < 0) {
	 continue;
       }
       if (pfd[1].revents & POLLIN) {
           n = read(pfd[1].fd, buf, sizeof(buf));
           if (n > 0){
	     fwrite(buf, n, 1, fp);
	   }
	   else if (n < 0) {
	       fprintf(stderr, "read failed: %d\n", errno);
	       return;
           }
       } else if (finished){
	   return;
       }
       // Get an 'ok' response on the Print pipe 
       if (!finished)
	 if (pfd[0].revents & POLLIN) {
	   read(pfd[0].fd, buf, 4);
	   int code=get_int(buf,0);
	   if (code == JAVA_PLUGIN_OK)
	     {
	       finished = 1;
	       timeout = 0;
	     }
	 }
     }
}

/*
 * This is called when Netscape is about to unload the library.
 * All our plugin instances should be closed down at this point.
 * We keep our Java child process alive for reuse later.
 */
void
CJavaVM::ShutdownJavaVM(int RealShutdown)
{
    trace("CJavaVM::ShutdownJavaVM\n");

    char buff[4];

    // This is just advice, the java_vm process will actually stick around 
    put_int(buff, 0, JAVA_PLUGIN_SHUTDOWN);
    SendRequest(buff, 4, FALSE);

    XtRemoveInput(state->inputid);

    // Clean off our work queue 
    ProcessWorkQueue();

    if (RealShutdown) {
        char * lbuff = (char *) malloc(strlen(stateENVName) + strlen("=0x0") +1);
        sprintf(lbuff,"%s=0x0",stateENVName);
	putenv((char *) lbuff);
	close(state->command_pipe);
	close(state->work_pipe);
	free(state);
	state = NULL;
    }
}

/*
 * Process work from the worker queue, coming from the out-of-proc
 * VM. Basically wait in an infinite loop on a timed select.
 * If we time out, return, since this may be called in the main
 * thread, and it is important that other work also get done
 * in the browser!
 */
void CJavaVM::ProcessWorkQueue(void) {    
    trace("CJavaVM::ProcessWorkQueue\n");
    if (state->work_pipe < 0) {
	trace("CJavaVM::work pipe is dead\n");
	return;
    }
    for (;;) {
	int rc;
	fd_set fs;
	struct timeval tv;
	tv.tv_sec =0;
	tv.tv_usec = 0;
	trace("CJavaVM::Doing work...\n");
	FD_ZERO(&fs);
	FD_SET(state->work_pipe, &fs);
	rc = select(state->work_pipe+1, &fs, NULL, NULL, &tv);
	if (rc < 1) {
	    trace("CJavaVM::No work on either pipe\n");
	    break;
	}
	/* We have work in the pipe */
	trace("CJavaVM::Doing work\n");
	DoWork();
    }
    trace("CJavaVM::Done with processing work queue\n");
}


/* 
 * Inform plugin to terminate current request without a reply
 */
void CJavaVM::TerminateRequestAbruptly(const char* infoMsg) 
{
    trace("CJavaVM::TerminateRequestAbruptly: %s\n", infoMsg);
   
    char msg[4];
    put_int(msg, 0, JAVA_PLUGIN_REQUEST_ABRUPTLY_TERMINATED);
    SendRequest(msg, 4, FALSE);

}

/*
 * This method is called when we detect that our child process has 
 * sent us a work request that we ought to process.
 */
void CJavaVM::DoWork(void)
{
    trace("CJavaVM::DoWork\n");

    char buff[4];
    int code = 0, rc = 0;
    short ix = 0;
    CJavaPluginInstance *pluginInstance = NULL;

    trace("CJavaVM::DoWork\n");
    // First read the request code. The special value of 0 is used
    // to indicate that the child is ready and is otherwise ignored.
    rc = read(state->work_pipe, buff, 4);
    if (rc != 4) {
	plugin_error("Error in reading data. Read %d\n", rc);
	WorkError(1);
	return;
    }
    code = get_int(buff, 0);

    if (code == 0) {
	trace("Child is ready\n");
	// the java process is ready for more exotic requests
	return;
    }


    // Then read the index of the plugin instance making the request.
    // In the case of some requests (such as JAVA_PLUGIN_FIND_PROXY),
    // the instance index may be -1, indicating a random instance.
    // Should not be null, since these requests come from a worker
    // thread, which indicates that an instance must exist
    trace("Trying to read code..\n");
    rc = read(state->work_pipe, buff, 2);
    if (rc != 2) {
        WorkError(2);
        return;
    }

    trace("Reading plugin index\n");
    ix = get_short(buff, 0);

    trace("CJavaVM received a work request: 0x%X on %d \n", code, (int) ix);
    pluginInstance = mPlugin->GetInstance(ix);

    trace("CJavaVM received pluginInstance 0x%x\n", pluginInstance);

    if (code == JAVA_PLUGIN_JAVASCRIPT_REQUEST) 
      {
        trace("CJavaVM::DoWork - JAVA_PLUGIN_JAVASCRIPT_REQUEST\n");

	rc = read(state->work_pipe, buff, 2);
	if (rc != 2) {
	  WorkError(2);
	  return;
	}
	short type = get_short(buff, 0);
	char* url = NULL;

	// This is to workaround the pipe buffer limitation
	// If the data is more than 4K/8K, the sender will
	// block. Thus, we send the request through local 
	// temp file in that case.
	if (type == 1)
	  {
	    // The string is the location of the JS request
	    char* tempFile = ReadWorkString();
	    FILE* fp = fopen(tempFile, "r");
	    free(tempFile);

	    if (fp != NULL)
	      {
		int length = 0;
		int totalLength = 0;
		char buffer[4096];
		
		// This is very important to have items=4096
		// because the return value is always the 
		// number of items read
		while (length = fread(buffer, 1, 4096, fp))
		  {
		    if (length >= 0)
		      buffer[length] = 0;

		    if (url == NULL)
		      {
			url = (char*) malloc(sizeof(char) * (length + 1));
			memcpy(url, buffer, length);
			totalLength = length;
		      }
		    else
		      {
			// url is already allocated
			url = (char*) realloc(url, sizeof(char) * (totalLength + length + 1));
			memcpy(url + totalLength, buffer, length);
			totalLength += length;
		      }
		  }

		if (url != NULL)
		  {
		  url[totalLength] = '\0';
		  }

		fclose(fp);
	      }
	  }
	else
	  {
	    // The string is the JavaScript URL
	     url = ReadWorkString();
	  }

        if (url == NULL) {
            WorkError(8);
            return;
        }
        if (pluginInstance != NULL) {
            // If the plugin instance is null don't make the request
            trace("JAVA_PLUGIN_JAVASCRIPT_REQUEST %s\n", url);
            nsIPluginManager* mgr = mPlugin->GetPluginManager();
            mgr->GetURL((nsIPluginInstance*)pluginInstance, url, 0, 
                        (void *) JAVA_PLUGIN_JAVASCRIPT_REQUEST);
        } else {
            TerminateRequestAbruptly("JavaScriptRequest");
        }
        free(url);
    } else {
        plugin_error("Unexpected work request from child");
    }
}


/*
 * Read a string from the worker pipe 
 */
char *CJavaVM::ReadWorkString(void) 
{
  trace("CJavaVM::ReadWorkString\n");

  char buff[4];
  short len;
  char *result;
  int rc = read(state->work_pipe, buff, 4);
  if (rc != 4) {
    return NULL;
  }
  len = get_int(buff, 0);
  if (len <= 0 ) {
      return NULL;
}
  result = (char *)malloc(len+1);
  int needtoread = len;
  char * readpointer = result;
  do {
     rc = read(state->work_pipe, readpointer, needtoread); 
     if (rc < 0) {
        free(result);
        return NULL;
     }
     needtoread = needtoread - rc;
     readpointer = readpointer + rc;
  } while(needtoread != 0);

  result[len] = 0;
  return result;
}

/*
 * Report an error in performing work, and terminate the jvm
 */
void CJavaVM::WorkError(int x) 
{
  trace("CJavaVM::WorkError\n");

  if (tracing) 
    perror("Work error");
  
  fprintf(stderr, "%s (%d)\n",
	  DGETTEXT(JAVA_PLUGIN_DOMAIN, 
		   "Plugin: trouble with work request from child"),
	  x);
  Cleanup();
}

/*
 * Clean up the java process since an error has occurred 
 */
void CJavaVM::Cleanup(void) 
{
  trace("CJavaVM::Cleanup\n");

  fprintf(stderr, "%s\n",
	  DGETTEXT(JAVA_PLUGIN_DOMAIN,"Plugin: Plugin is not enabled or Java VM process has died."));
    	
  mStatus = nsJVMStatus_Failed;
  if (state->command_pipe >= 0) {
    char *buff;
    close(state->command_pipe);
    close(state->work_pipe);
    state->command_pipe = -1;
    state->work_pipe = -1;
    buff = (char *)malloc(140);
    sprintf(buff, "%s=XXX",stateENVName);
    putenv(buff);
    trace("Cleaned up child state\n");
  }
  if (state->child_pid != 0) {
    int stat = 0;
    int rc = waitpid(state->child_pid, &stat, WNOHANG);
    if (rc > 0) {
      if (WIFEXITED(stat)) {
	fprintf(stderr, "%s %d\n",
		DGETTEXT(JAVA_PLUGIN_DOMAIN, 
			 "plugin: java process exited with status"), 
		WEXITSTATUS(stat));
      } 
    }
    state->child_pid = 0; 
  }
}


/*
 * Define the variables needed by the child process. This merely initializes
 * the env_ variables. They are later putenv'd after the fork.
 */
void CJavaVM::SetupChildEnvironment(void) 
{
  trace("CJavaVM::SetupChildEnvironment\n");

  // Figure out which JRE to use. 
  char *jre = FindJRE();
  trace("Using JRE from %s\n", jre);

  char *ld_path = getenv("LD_LIBRARY_PATH");
  char *buff = (char *)malloc(300 + slen(state->java_dir) + 5*slen(jre)  
			      + 5*slen(LIBARCH) + slen(ld_path));

  // The ARCH macro is defined in the makefile.  E.g. -DARCH="sparc"    
  // Be careful of the LD_LIBRARY_PATH - beware the dummy libhpi 
  // and libjvm.so
  //   In 1.2 the non-dummy versions are in 
  //        <jre>/lib/sparc/<thread_type>/libhpi.so 
  //  If jre is the same as state->javadir (typically ~/.netscape/java)
  //  then if jre/lib is too early it ends up pointing to 
  // .netscape/java/lib
  //  and picks up the (wrong, dummy) version of libhpi etc which are
  //   only meant for 1.1. So put the thread specific directory first
  //   Also: The 1.2fcs build places libjvm in 
  //          jre/lib/<arch>/classic/libjvm.so
  // (to permit hotspot later).  So this path is only needed for 1.2

      sprintf(buff, "LD_LIBRARY_PATH=%s/lib/%s/client:%s/lib/%s",
	      jre, LIBARCH, 		// libjvm
	      jre, LIBARCH);		// libjava, libawt & friends

  if (slen(ld_path) > 0) {
    // Append the user's LD_LIBRARY_PATH 
    strcat(buff, ":");
    strcat(buff, ld_path);
  }
  trace("Library path is %s\n", buff);
  env_LD_LIBRARY_PATH = buff;
  
  buff =(char *) malloc(100 + slen(jre));
  sprintf(buff, "JAVA_HOME=%s", jre);
  
  trace(" JAVA_HOME is %s\n", buff);
  env_JAVA_HOME = buff;    
  
  buff =(char *) malloc(100 + slen(state->java_dir));
  sprintf(buff, "PLUGIN_HOME=%s", state->java_dir);

  trace(" PLUGIN_HOME is %s\n", state->java_dir);
  env_PLUGIN_HOME = buff;    
}

/*
 * Find the location of the JRE. Use either the .java/deployment/deployment.properties
 * file (javaplugin.jre.path=) or the standard jre directory that
 * came with the plugin stored in state->java_dir
 */
char *CJavaVM::FindJRE(void) {
    FILE *fin;
    char *home = NULL;
    char path[1024];
    char line[200];
    char jre[200];
    int rc;

    home  = getenv("USER_JPI_PROFILE");
    if(home == NULL)
    {
	home  = getenv("HOME");
    }

    // Look in the properties file
    sprintf(path, "%s/.java/deployment/deployment.properties", home);	

    fin = fopen(path, "r");
    if (fin == NULL) {
	return state->java_dir;
    }

    jre[0] = 0;
    while (fgets(line, sizeof(line), fin)) {
        trace("~/.java/deployment/deployment.properties:%s\n", line);
	sscanf(line, "javaplugin.jre.path=%s", jre);
    }
    fclose(fin);

    if (jre[0] != 0) {

        // The user has defined a JRE.  Check if it looks plausible. 
        // First check if the property is set to "Default" .
        // This whole thing should be internationalized, which will be
        // problematic. Maybe use a different flag (boolean valued)

	if ( strcmp(jre,"Default") == 0 ) {
	  return state->java_dir;
	}

	// Now check if that directory is reasonable 
        struct stat sbuf;
        sprintf(path, "%s/lib", jre);
        rc = stat(path, &sbuf);
        if (rc == 0) {
	    // We've been given a plausible JRE string 
	    int jrelen = slen(jre)+1;
	    char *result =(char *) malloc(jrelen);
	    memcpy(result, jre, jrelen + 1);
	    return result;
	}
    	fprintf(stderr, "%s %s\n",
		DGETTEXT(JAVA_PLUGIN_DOMAIN, 
			 "Java property javaplugin.jre.path defined as"),
		jre);
        fprintf(stderr, "%s\n",
		DGETTEXT(JAVA_PLUGIN_DOMAIN, 
			 "But that directory does not exist.")); 
        fprintf(stderr, "%s %s\n",
		DGETTEXT(JAVA_PLUGIN_DOMAIN, "Using JRE from"),
		state->java_dir);
    }
    return state->java_dir;
}



void CJavaVM::FindJavaDir(void) {
    Dl_info dlinfo;
    static int dummy;
    char buf[PATH_MAX];
    char real[PATH_MAX];

    // Now: Only use the JRE which came with the plugin.

    // Formerly:
    //   Use the NPX_PLUGIN_PATH to find the directories holding plugins
    //   Find the first of those that has a ../java directory.

    state->java_dir = NULL;
    dladdr((void *)&dummy, &dlinfo);
    strcpy(buf, dlinfo.dli_fname);

    if (realpath(buf, real) == NULL) {
	fprintf(stderr, "Error: realpath(`%s') failed.\n", buf);
	return;
    }
    
    *(strrchr(real, '/')) = '\0';  /* executable file      */
    *(strrchr(real, '/')) = '\0';  /* NS4 subdirectory     */
    *(strrchr(real, '/')) = '\0';  /* $(LIBARCH)		  */
    *(strrchr(real, '/')) = '\0';  /* plugin		  */

    state->java_dir = strdup(real);
}


/*
 * Create a new applet
 */
void CJavaVM::CreateApplet(const char *appletType, int appletNumber, 
			   int argc, char **argn, char **argv) {
    char *buff;
    char *bp;
    int i;
    int len = 16;

    
    for (i = 0; i < argc; i++) {
	char *s;
	len += 2;
	s = argn[i];
	len += slen(s);
	len += 2;
	s = argv[i];
	len += slen(s);
	trace("JavaVM creating applet[%d] %d %s %s \n", 
	      appletNumber, i, argn[i], argv[i]);
    }
    buff = (char *)malloc(len);
    put_int(buff, 0, JAVA_PLUGIN_NEW);
    put_int(buff, 4, appletNumber);
    if (strstr(appletType, "bean")) {
	// It's a bean 
        put_int(buff, 8, 1);
    } else {
	// Its a regular applet 
        put_int(buff, 8, 0);
    }
    put_int(buff, 12, argc);
    bp = buff + 16;
    for (i = 0; i < argc; i++) {
	char *s;
	int l;
	s = argn[i];
	l = slen(s);
	put_short(bp, 0, l);
	bp += 2;
	memcpy(bp, s, l);
	bp += l;

	s = argv[i];
	l = slen(s);
	put_short(bp, 0, l);
	bp += 2;
	memcpy(bp, s, l);
	bp += l;
    }
    SendRequest(buff, len, TRUE);
    free(buff);
}

/*
 * Send a request to the VM on the command pipe. Depending on
 * wait_for_reply either do or do not wait for an acknowledgement.
 */
void CJavaVM::SendRequest(char *buff, int len, int wait_for_reply)
{
    trace("CJavaVM::SendRequest\n");

    int rc;
    char rbuff[4];
    int reply;
    int offset;

    trace("Sending request %d %d %d \n", state->command_pipe, 
	  len, wait_for_reply);
    if (state->command_pipe < 0) {
	trace("Could not send request. Child VM probably dead\n");
	return;
    }

    offset = 0;
    while (offset < len) {
        rc = write(state->command_pipe, buff+offset, len-offset);
        if (rc <= 0) {
	    trace("Write failed. Child process has terminated!\n");
	    Cleanup();
	    return;
	}
	offset += rc;
    }

   trace("Wrote first part\n");
    if (wait_for_reply == 0) 
	return;

    rc = read(state->command_pipe, rbuff, 4);
    if (rc != 4) {
	fprintf(stderr, "Read of acknowledgement failed: %d\n", errno);
	Cleanup();
	return;
    }

    trace("Read acknowledgement\n");

    reply = get_int(rbuff, 0);
    if (reply == JAVA_PLUGIN_OK) {
	trace("Done with request");
	return;
    } else {
	fprintf(stderr, "Bad reply: %d \n", reply);
    }

    Cleanup();

    fprintf(stderr, "Error in sending request\n");

    return;
}

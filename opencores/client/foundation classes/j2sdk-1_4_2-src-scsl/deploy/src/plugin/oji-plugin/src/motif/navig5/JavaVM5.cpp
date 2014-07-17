/*
 * @(#)JavaVM5.cpp	1.90 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*  
 * The JavaVM5 class represents our connect to the actual VM.  It is
 * created, intialized and then started, which spawns off the VM
 * process. The basic execution structure is the same as that in the
 * original plugin 1.1.  BG Sept 98
 *                                      
 */

#include <sys/sem.h>
#include <sys/ipc.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <stropts.h>
#include <string.h>
#include <stdarg.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <stropts.h>
#include <dlfcn.h>
#include <limits.h>
#include <strings.h>
#include "jni.h"
#include "remotejni.h"
#include "nsprPrivate.h"
#include "private/pprio.h"

#include "CReadBuffer.h"

/* For htonl and htons */
#include <arpa/inet.h>

#ifdef LINUX
#include <linux/limits.h>
#include <linux/types.h>
#include <linux/dirent.h>
#define _DIRENTRY_H
#else
#include <ulimit.h>
#endif
#include <dirent.h>
#include <sys/param.h>          // For MAXPATHLEN

//#include "nsICapsManager.h"
#include "nsIJVMManager.h"
#include "Navig5.h"
#include "JavaVM5.h"
#include "pluginversion.h"
#include "CookieSupport.h"

#include "QueueRunnable.h"

#ifndef NO_MINIMAL_TRACE
#define TRACE(m) trace("JavaVM5 %s\n", m);
#define TRACE_INT(m, i) trace("JavaVM5 %s %X\n", m, i);
#define TRACE_STR(m, p) trace("JavaVM5 %s %s\n", m, p);
#else
#define TRACE(m) 
#define TRACE_INT(m, i) 
#define TRACE_STR(m, p) 
#endif

extern "C" void socket_cleanup()
{
    char path[PATH_MAX];
    bzero(path,PATH_MAX);
    pid_t pid = getpid();
    sprintf(path,"%s.%s.%d",JAVA_PLUGIN_SOCKFILE,PLUGIN_NODOTVERSION,(int) pid);
    unlink(path);
}

static NS_DEFINE_IID(kISupportsIID, NS_ISUPPORTS_IID);

void spont_queue_processor(void * data) {

        TRACE("In spont event handler");
        if(data != NULL)  {
                JavaVM5 *vm = (JavaVM5*) data;
                vm->ProcessSpontaneousQueue();
        }
}

void worker_queue_processor(void * data) {

        TRACE("In worker event handler");
        if(data != NULL)  {
                JavaVM5 *vm = (JavaVM5*) data;
                vm->ProcessWorkQueue();
        }
}

/*
 * Process work from the worker queue, coming from the out-of-proc
 * VM. Basically wait in an infinite loop on a timed select.
 * If we time out, return, since this may be called in the main
 * thread, and it is important that other work also get done
 * in the browser!
 */
void JavaVM5::ProcessWorkQueue(void) {    
    TRACE("ProcessWorkQueue");
    int wp = PRFileDesc_To_FD(state->work_pipe);
    if (wp < 0) {
        TRACE("work pipe is dead");
        return;
    }
    for (;;) {
        int rc;
        fd_set fs;
        struct timeval tv;
        tv.tv_sec =0;
        tv.tv_usec = 0;
        TRACE("Doing work");
        FD_ZERO(&fs);
        FD_SET(wp, &fs);
        rc = select(wp + 1, &fs, NULL, NULL, &tv);
        if (rc < 1) {
            TRACE("No work on work pipe");
            PR_EnterMonitor(workMonitor);
            workPipeClean = true;
            PR_NotifyAll(workMonitor);
            PR_ExitMonitor(workMonitor);
            break;
        }
        /* We have work in the pipe */
        DoWork();
    }
    TRACE("Done with processing work queue");
}


/*  
 * Create a new JavaVM, associated with the plugin factory 'plugin'
 * (an instance of NPIPlugin).  Also determine whether the VM should
 * use green or native threads based on the THREADS_FLAG.  Initialize
 * and then StartVM must be called.
 */
JavaVM5::JavaVM5(JavaPluginFactory5 *plugin) {
    TRACE("Creating JavaVM5");

    state = new LongTermState();
    sprintf(stateENVName,"%s%s","JAVA_PLUGIN_STATE",PLUGIN_NODOTVERSION);
    m_pPluginFactory = plugin;
    memset(state, 0, sizeof(struct LongTermState));
    jvm_status = nsJVMStatus_Enabled;
    workPipeClean = true;
    spontPipeClean = true;
    workMonitor = PR_NewMonitor();
    spontMonitor = PR_NewMonitor();
}


/*
 * Destroy the VM. In mozilla 5, since the factory is never deleted,
 * this should never be called
 */
JavaVM5::~JavaVM5() {
    TRACE("Destroying JavaVM");
    if(workMonitor != NULL) {
        PR_DestroyMonitor(workMonitor);
    }
    if(spontMonitor != NULL) {
        PR_DestroyMonitor(spontMonitor);
    }
}

/* Return whether the browser has enabled the  VM  (it may
 * be disabled even if it is running.
 */
PRBool JavaVM5::GetJVMEnabled(void) {
    TRACE("GetJVMEnabled");

    return PR_TRUE;
}


/*
 * Return the plugin factory associated with this VM
 */
JavaPluginFactory5* JavaVM5::GetPluginFactory(void) {
    TRACE("GetPluginFactory");
    return m_pPluginFactory;
}


/* Dup the fdarr to safety range */
void
dup2_to_safety(int nfds, int fdarr[], 
               int safe_range_start, int safe_range_end) {

    int i;

    /* Find the max fd to identify a fd range to store fds
       temporarily during duping */
    int tempfd = safe_range_end + 1;
    for(i = 0; i < nfds ; i++)
        tempfd = (fdarr[i] >= tempfd) ? fdarr[i] + 1 : tempfd;
    
    /* Dup the FDs out of the safe range */
    for(i = 0; i < nfds ; i++){
        int curfd = fdarr[i];
        /* If the FD is already in the safe range, dup it temporarily
           in the temporary range */
        if (curfd >= safe_range_start && curfd <= safe_range_end) {
            TRACE_INT("Conflict", i);
            TRACE_INT("fd", fdarr[i]);
            TRACE_INT("fd target", tempfd);
            fdarr[i] = wrap_dup2(curfd, tempfd);
            tempfd++;
        }
    }

    /* assert fdarr[i] not in safe_range */
    /* Dup the FDs into the safe range */
    int j;
    for(i = 0, j=safe_range_start; i < nfds; i++, j++) {
        fdarr[i] = wrap_dup2(fdarr[i], j);
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
int JavaVM5::StartJavaVM(const char *addClasspath)
{
    int rc;
    char *buff;
    char *env_JAVA_PLUGIN_AGENT;
    const char *progname;
    const char *agent = "No agent";
    nsresult res;

    UNUSED(addClasspath);

    TRACE("********************* StartJavaVM ***************************");

    /*
     * Check if we already have an Java child process.
     * We use the Unix environment to locate our saved state
     */
    LongTermState *pstateTmp;
    buff = getenv(stateENVName);
    if (buff != NULL) {
       //rc = sscanf(buff, "%X", &pstateTmp);
       rc = sscanf(buff, "%p", (void **) &pstateTmp);
       if (rc == 1 && pstateTmp != 0) {
           /* We already have an active connection. */
                 delete state;
	         state = pstateTmp;
                 TRACE("reusing child process");
                 jvm_status = nsJVMStatus_Running;
                 return NS_OK;
       }
    } 
        // We are first on the list
        /* Allocate and record a state struct.  This is used to retrieve
         * our state from the environment if the plugin is restarted 
         */
        buff = (char* )checked_malloc(140);
        //sprintf(buff, "%s=%X", stateENVName,(int) state);
        sprintf(buff, "%s=%p", stateENVName,(void *) state);
        putenv(buff);

    /* Get the browser's name (Mozilla-5) to store in the environment
     *  variable JAVA_PLUGIN_AGENT for the child to read to determine
     *  its version. 
     */
    res = m_pPluginFactory->GetPluginManager()->UserAgent(&agent);
    if (NS_OK != res) 
        return res;
    env_JAVA_PLUGIN_AGENT = (char *)checked_malloc(40 + slen(agent));
    sprintf(env_JAVA_PLUGIN_AGENT, "JAVA_PLUGIN_AGENT=%s", agent);

    /* Find directory where plugin JRE is installed */
    FindJavaDir();
    if (state->java_dir == NULL) {
        plugin_formal_error("Plugin: can't find plugins/../java directory");
        return NS_ERROR_NOT_AVAILABLE;
    }

    /* Set up the child environment variables. Wait till after the
     * fork to actually put them into the environment.
     */
    SetupChildEnvironment();


#if defined(DEBUG_VERSION)
    progname = "java_vm_g";
#else
    progname = "java_vm";
#endif

    /* Initialize the server socket for later use with attach_thread */
#if defined(USE_TCP_SOCKET)
    PRFileDesc* server_socket = PR_NewTCPSocket();
#else
    PRFileDesc* server_socket = PR_Socket(PR_AF_LOCAL, PR_SOCK_STREAM, 0);
#endif
    PRNetAddr net_address;

    int try_port = INITIAL_ATHREAD_PORT;
#if defined(USE_TCP_SOCKET)
    net_address.inet.family = AF_INET;
    net_address.inet.port = htons(try_port);
    net_address.inet.ip = htonl(INADDR_ANY);
    while (PR_Bind(server_socket, &net_address) != PR_SUCCESS) {
        /* Try higher number addresses */
        TRACE_INT("Binding of server socket failed", try_port);
        try_port++;
        net_address.inet.port = htons(try_port);
    }
#else
    pid_t spid = getpid();
    net_address.local.family = PR_AF_LOCAL;
    bzero(net_address.local.path,sizeof(net_address.local.path));
    sprintf(net_address.local.path,"%s.%s.%d",JAVA_PLUGIN_SOCKFILE,PLUGIN_NODOTVERSION,(int) spid);
    unlink(net_address.local.path);
    try_port = htons(spid);
    if(PR_Bind(server_socket, &net_address) != PR_SUCCESS) {
        TRACE_INT("Binding of server socket failed", try_port);
    }
    atexit(socket_cleanup);
#endif
    
    if (PR_Listen(server_socket, 3) != PR_SUCCESS)
        plugin_error("Listen on server socket failed");

    state->server_socket = server_socket;
    state->port = try_port;
    

    /* For the worker pipe communication */
    PRFileDesc *Work[2];
    wrap_PR_CreateSocketPair("work", Work);

    /* For the command pipe communication */
    PRFileDesc *Command[2];
    wrap_PR_CreateSocketPair("command", Command);

    /* For the other JNI/JS communication from JVM */
    PRFileDesc *Spontaneous[2];
    wrap_PR_CreateSocketPair("spontaneous", Spontaneous);

    /* For the Print communication */
    PRFileDesc* Printing[2];
    wrap_PR_CreateSocketPair("print", Printing);

    /* Determine the full executable path name */
    char *fullprogname = (char *) checked_malloc(100 + slen(state->java_dir));
    sprintf(fullprogname, "%s/bin/%s", state->java_dir, progname);

    int JVMWorkFD = PRFileDesc_To_FD(Work[1]);
    int JVMCommandFD = PRFileDesc_To_FD(Command[1]);
    int JVMSpontaneousFD = PRFileDesc_To_FD(Spontaneous[1]);
    int JVMPrintFD = PRFileDesc_To_FD(Printing[1]);

    int nfds = 4;
    int fdarr[4] = {JVMSpontaneousFD, 
                    JVMCommandFD, JVMWorkFD, JVMPrintFD};


    /* If the vm uses native threads make these fds be blocking.
       The FDs that mozilla returns are non-blocking, at
       least when created with a "local threads" nspr.
       If the VM uses green threads it requires blocking FDs */
    if (true) {
        for(int i=0; i < nfds; i++) {
            int origflags = fcntl(fdarr[i], F_GETFL);
            int newflags = origflags & (~O_NONBLOCK);
            fcntl(fdarr[i], F_SETFL, newflags);
            trace("native vm:%d Orig flags = %X New flags = %X \n", fdarr[i],
                  origflags, newflags);
        }
    }


    char env_MOZILLA_WORKAROUND[1024];
    sprintf(env_MOZILLA_WORKAROUND, "MOZILLA_WORKAROUND=true");

    TRACE("Ready to fork");


    /* Create our child java process */
#if defined(__linux__) 
    rc = fork();
#else
    rc = fork1();
#endif

    if (rc == -1) {
        plugin_error("Fork failed: %d\n", errno);
        return NS_ERROR_OUT_OF_MEMORY;
    }

    if (rc == 0) {
        /* Child process.  Prepare to exec. */
        int i, max;

        /* Must include javaplugin.jar */
        putenv(env_LD_LIBRARY_PATH);
        /* Used in call to sun.plugin.JavaRunTime.initEnvironment */
        putenv(env_JAVA_HOME);
        /* To derive the version of mozilla calling the plugin */
        putenv(env_JAVA_PLUGIN_AGENT);
	/* Workaround Mozilla bug */
	putenv(env_MOZILLA_WORKAROUND);

	putenv(env_PLUGIN_HOME);

        if (tracing) {
            /* If PLUGIN_LD_DEBUG is set to anything pass it as LD_DEBUG*/
            char* ld_flags = getenv("JAVA_PLUGIN_LD_DEBUG");
            if (ld_flags != NULL) {
                char* ldbuf = (char *) checked_malloc(strlen(ld_flags) + 11);
                sprintf(ldbuf, "LD_DEBUG=%s", ld_flags);
                putenv(ldbuf);
            }
        }
        /* Clear the close-on-exec flags for stdio and our pipe. */
        for (i = 0; i < 3; i++) {
            fcntl(i, F_SETFD, 0);
        }

        /* We need to dup the FDs into a lower range since under green
           threads monitors for only the first 16 FDs are
           pre-allocated. 
           */
        int safe_range_start = MOZ5_STARTSAFEFD;
        int safe_range_end   = MOZ5_ENDSAFEFD;
        dup2_to_safety(nfds, fdarr, safe_range_start, safe_range_end);

        /* Don't close on exec for our pipes */
        for(i = 0; i < nfds; i++)
            fcntl(fdarr[i], F_SETFD, 0);
        

        /* Close all other file descriptors. */
#ifndef __linux__
        max = ulimit(UL_GDESLIM);
#else
        max = sysconf(_SC_OPEN_MAX);
#endif       

        for (i = 3; i <= max; i++) {
            int do_close = 1;
            for(int j = 0; j < nfds; j++)
                if (i == fdarr[j] ) do_close = 0;
            if (do_close) close(i);
        }

        if (tracing) {
            execlp(fullprogname, progname, "-t", 0);
        } else {
            execlp(fullprogname, progname, "", 0);
        }

        plugin_error("Exec of \"%s\" failed: %d\n<", progname, errno);
        exit(6);
    }

    TRACE("Continuing in parent process....");
    state->child_pid = rc;
    state->command_pipe = Command[0];
    state->work_pipe = Work[0];
    state->spont_pipe = Spontaneous[0];
    state->print_pipe = Printing[0];

    PR_Close(Command[1]);
    PR_Close(Work[1]);
    PR_Close(Spontaneous[1]);
    PR_Close(Printing[1]);

    int resack = PR_Read(Command[0], buff, 1);
    if (resack != 1) {
	plugin_error("Could not read ack from child process");
    }

    {
        /* this is here because the PR_ stuff creates NONBlocking
           pipes and CReadbuffer doesn't like that.
        */
        int change_fd = PRFileDesc_To_FD(Command[0]);
     	int flags = fcntl(change_fd, F_GETFL);
	int newflags = flags & ~O_NONBLOCK;
	int res = fcntl(change_fd, F_SETFL, newflags);
	if (res == -1)
            TRACE("\n***CHANGING FLAGS DID NOT WORK ON BROWSER\n");
	flags = fcntl(change_fd, F_GETFL);
	TRACE_INT("nbrowser flags =", flags);
    }

    TRACE_INT("Parent OK, child pid\n", state->child_pid);

    jvm_status = nsJVMStatus_Running;

    spont_remotejni            = create_RemoteJNIEnv();

    CSecureJNIEnv* secjni        = new CSecureJNIEnv(NULL, spont_remotejni);

    JNIEnv* proxyJNI           = NULL;

    if (NS_SUCCEEDED((m_pPluginFactory->GetJVMManager())->CreateProxyJNI(secjni,&proxyJNI))){
        //fprintf(stderr, " Succeeding in creating proxy\n");
    }

    /* Register the mapping between proxy and remote jni */
    int spont_env_index =  m_pPluginFactory
        ->RegisterRemoteEnv(spont_remotejni, proxyJNI);

    /* Initialize the remote JNI Env with its pipe etc */
    init_RemoteJNIEnv(spont_remotejni, spont_env_index, state->spont_pipe);


    PRThread * tid = PR_GetCurrentThread();

    nsIThreadManager *tm = NULL;

    m_pPluginFactory->GetServiceProvider()->QueryService(
                                      nsIJVMManager::GetCID(),
                                      NS_GET_IID(nsIThreadManager),
                                      (nsISupports**)&tm);
   
    QueueRunnable * wqr = new QueueRunnable(this,
                                           worker_queue_processor,
                                           PRFileDesc_To_FD(state->work_pipe),
                                           (PRUint32)tid, 
                                           workMonitor, &workPipeClean,
                                           tm);

    QueueRunnable * sqr = new QueueRunnable(this,
                                           spont_queue_processor,
                                           PRFileDesc_To_FD(state->spont_pipe),
                                           (PRUint32)tid,
                                           spontMonitor,&spontPipeClean,
                                           tm);

    m_pPluginFactory->GetServiceProvider()->ReleaseService(
                                        nsIJVMManager::GetCID(),
                                        tm);

    PR_CreateThread( PR_USER_THREAD, QueueRunnable::threadEntry, 
                                  (void*) wqr, PR_PRIORITY_HIGH, 
                                  PR_GLOBAL_THREAD, PR_JOINABLE_THREAD, 0);

    PR_CreateThread( PR_USER_THREAD, QueueRunnable::threadEntry, 
                                  (void*) sqr, PR_PRIORITY_HIGH, 
                                  PR_GLOBAL_THREAD, PR_JOINABLE_THREAD, 0);

    return NS_OK;
}
/*
 * ReceivePrint polls the printpipe until it receives ok from
 * plugin
 */

void JavaVM5::ReceivePrinting(FILE *fp) {
    struct PRPollDesc pollDesc[2];
    PRIntervalTime timeout=PR_INTERVAL_NO_TIMEOUT;
    PRIntn npds= 2;
    int finished = 0;
    char *buf;
    int ok;
    int n;
    int oldsize=0;

    TRACE("Receiving printing ");
    pollDesc[0].fd = state->command_pipe;
    pollDesc[1].fd = state->print_pipe;
    pollDesc[0].in_flags = pollDesc[1].in_flags=PR_POLL_READ;
    
    for (;;) {
        if (PR_Poll(pollDesc, npds, timeout ) < 0 ) {
            continue;
        }
	if (pollDesc[1].out_flags & PR_POLL_READ) {
            n = PR_Available(pollDesc[1].fd);
            if ( n < 0 ) {
                fprintf(stderr, "Error reading print pipe %d\n",PR_GetError());
                break;
            }  
            if ( oldsize < n ) {
            buf = (char *) realloc(buf, n);
            oldsize = n;
            }
            n = PR_Read(pollDesc[1].fd, buf, n);
            if ( n > 0 ) {
                fwrite(buf, n, 1, fp);
            } else if ( n < 0 ) {
                fprintf(stderr, "Error reading print pipe %d\n", PR_GetError());
                break;
            }
        } else if (finished) break;

        if (pollDesc[0].out_flags & PR_POLL_READ) {
            CReadBuffer rb(PRFileDesc_To_FD(pollDesc[0].fd));
            if ((rb.getInt(&ok) > 0) && (JAVA_PLUGIN_OK == ok)) {
                finished = 1;
                timeout = 0;
            }
        }
    }
    if (buf != NULL) free(buf);
}

jobject
JavaVM5::GetJavaObjectForInstance(int plugin_number) {
    CWriteBuffer wb;
    wb.putInt(JAVA_PLUGIN_GET_INSTANCE_JAVA_OBJECT);
    wb.putInt(plugin_number);
    int res = SendBlockingRequest(wb);
    return (jobject) res;
}

/*
 * This is called when Netscape is about to unload the library.
 * All our plugin instances should be closed down at this point.
 * We keep our Java child process alive for reuse later.
 */
void
JavaVM5::ShutdownJavaVM(int RealShutdown)
{
    CWriteBuffer wb;
    TRACE("Shutdown");

    /* This is just advice, the java_vm process will actually stick around */
    wb.putInt(JAVA_PLUGIN_SHUTDOWN);
    SendRequest(wb, FALSE);

    // Clean off our work queue
    ProcessWorkQueue();

    if (RealShutdown) {
//        putenv((char *) "JAVA_PLUGIN_STATE=0x0");
//        close(state->command_pipe);
//        close(state->work_pipe);
//        free(state);
//        state = NULL;
    }
}

/* Handle requests on the spontaneous pipe. These requests are handled
   similarly to requests on */
void
JavaVM5::ProcessSpontaneousQueue() {

    int wp = PRFileDesc_To_FD(state->spont_pipe);
    if (wp < 0) {
        TRACE("spont pipe is dead");
        return;
    }
    for (;;) {
        int rc;
        fd_set fs;
        struct timeval tv;
        tv.tv_sec =0;
        tv.tv_usec = 0;
        TRACE("Doing spontainious work");
        FD_ZERO(&fs);
        FD_SET(wp, &fs);
        rc = select(wp + 1, &fs, NULL, NULL, &tv);
        if (rc < 1) {
            TRACE("No work on spont pipe");
            PR_EnterMonitor(spontMonitor);
            spontPipeClean = true;
            PR_NotifyAll(spontMonitor);
            PR_ExitMonitor(spontMonitor);

            break;
        }
        /* We have work in the pipe */
        int request_code = 0;

        TRACE("Spontaneous thread waiting for next request...");

        read_PR_fully("Spont Req", state->spont_pipe, (char *)&request_code, 4);

        TRACE_INT("Received request code: ", request_code);

        switch (request_code) {
        case JAVA_PLUGIN_REQUEST: 
            {
                /* This is a JS call from a spontaneous thread */
                TRACE("SPONTANEOUS CALL!!! (JAVA_PLUGIN_REQUEST)");
                JSHandler(spont_remotejni);
                break;
            }
        default:
            plugin_error("Did not understand spontaneous code %X\n", 
                         request_code);
        }
    }
}



/* 
 * Inform plugin to terminate current request without a reply
 */
void JavaVM5::TerminateRequestAbruptly(const char* infoMsg) {
    TRACE_STR("Abruptly terminating request", infoMsg);
   
    CWriteBuffer wb;
    wb.putInt(JAVA_PLUGIN_REQUEST_ABRUPTLY_TERMINATED);
    SendRequest(wb, FALSE);

}


/*
 * This method is called when we detect that our child process has 
 * sent us a work request that we ought to process.
 */
void JavaVM5::DoWork()
{
    int code; 
    short ix;
    JavaPluginInstance5* pluginInstance;

    if (state->work_pipe < 0) {
        plugin_error("JavaVM5::work pipe is dead");
        m_pPluginFactory->ExitMonitor("ProcessWorkQueue-1");
        return;
    }

    CReadBuffer rb(PRFileDesc_To_FD(state->work_pipe));
    
    // First read the request code. The special value of 0 is used
    // to indicate that the child is ready and is otherwise ignored.
    rb.getInt(&code);
    
    TRACE_INT("Obtained next work code code", code);

    rb.getShort(&ix);
    pluginInstance = m_pPluginFactory->GetInstance(ix);

    /* Now handle the normal cases */
    if (code == JAVA_PLUGIN_SHOW_STATUS) {
        char *mess;
        int rc = rb.getString(&mess);
        if (rc <= 0)
	    mess=strdup(" ");
        if (pluginInstance != NULL) {
            nsIPluginInstancePeer *peer = NULL;
            pluginInstance->GetPeer(&peer);
            if (peer != NULL) {
                peer->ShowStatus(mess);
                peer->Release();
            } else {
                plugin_error("No peer found for show status!");
            }
        }
        if (rc <= 0) 
            free(mess);
        else 
            rb.free(mess);
    } else if (code == JAVA_PLUGIN_SHOW_DOCUMENT) {
        char *url;
        char *target;
         rb.getString(&url);
         rb.getString(&target);
        if (url == NULL || target == NULL) {
            WorkError(4);
            return;
        }
        TRACE_STR("Show document URL", url);
        TRACE_STR("Show document target", target);
        if (pluginInstance != NULL) {
            nsIPluginManager *mgr = m_pPluginFactory->GetPluginManager();
            nsresult rv  = mgr->GetURL((nsIJVMPluginInstance*)pluginInstance, url, 
							target, NULL);
            TRACE_INT("Return from GetURL",rv);
        }
        rb.free(url);
        rb.free(target);
    } else if (code == JAVA_PLUGIN_FIND_PROXY) {
        char *url;
        char *host;
        rb.getString(&url);
        rb.getString(&host);
	if (url == NULL || host == NULL) {
	  WorkError(5);
	  return;
	}

	if (pluginInstance != NULL) {
	  m_pPluginFactory->GetProxySupport()->ProxmapFindProxy((nsIPluginInstance*) pluginInstance, url, host);
	} else {
	  TerminateRequestAbruptly("FindProxy");
	}
        rb.free(url);
        rb.free(host);
    } else if (code == JAVA_PLUGIN_FIND_COOKIE) {
        char *url;
        rb.getString(&url);
	if (pluginInstance != NULL) {
	    m_pPluginFactory->GetCookieSupport()->FindCookieForURL(pluginInstance,url);
	}
        rb.free(url);
    } else if (code == JAVA_PLUGIN_SET_COOKIE) {
        char *url;
        char *cookie;
        rb.getString(&url);
        rb.getString(&cookie);
	if (pluginInstance != NULL) {
	    m_pPluginFactory->GetCookieSupport()->SetCookieForURL(url, cookie);
	}
        rb.free(url);
        rb.free(cookie);
    } else {
        plugin_formal_error("Plugin: unexpected work request from child");
        plugin_error("Code = %0x", code);
    }
}

/*
 * Clean up the java process since an error has occurred 
 */
void JavaVM5::Cleanup(void) {

    plugin_formal_error("Plugin: Java VM process has died.");
        
    jvm_status = nsJVMStatus_Failed;
    /* Wipe out the JAVA_PLUGIN_STATE environment variable, close pipes */
    if (state->command_pipe >= 0) {
//        char *buff;
        PR_Close(state->command_pipe);
        PR_Close(state->work_pipe);
        state->command_pipe = 0;
        state->work_pipe = 0;
//        free(state);
//        state = NULL;
//       buff = (char *)checked_malloc(40);
//        sprintf(buff, "JAVA_PLUGIN_STATE=XXX");
//        putenv(buff);
        TRACE("Cleaned up child state");
    }
    TRACE("Waiting for child process to termiante ");
    /* Wait for the child process to terminate */
    if (state->child_pid != 0) {
        int stat = 0;
        int rc = waitpid(state->child_pid, &stat, WNOHANG);
        if (rc > 0) {
            if (WIFEXITED(stat)) {
                fprintf(stderr, "%s %d\n",
                        DGETTEXT(JAVA_PLUGIN_DOMAIN, 
                                 "plugin: java process exited with status"), 
                        WEXITSTATUS(stat));
            } else if (WIFSIGNALED(stat)) {
                fprintf(stderr,"%s %d\n", 
                        DGETTEXT(JAVA_PLUGIN_DOMAIN, 
                                 "plugin: java process died due to signal"), 
                        WTERMSIG(stat));
                if (WCOREDUMP(stat)) {
                    fprintf(stderr, "%s\n", 
                            DGETTEXT(JAVA_PLUGIN_DOMAIN, 
                                     "  a core file was generated"));
                }
            }
        }
        state->child_pid = 0; 
    }
}



/* Send the message to create a new applet */
void JavaVM5::CreateApplet(const char *appletType, int appletNumber, int argc,
                           char **argn, char **argv) {
    CWriteBuffer wb;
    int i;
    
    TRACE_INT("JavaVM JAVA_PLUGIN_NEW creating applet", appletNumber);
    wb.putInt(JAVA_PLUGIN_NEW);
    wb.putInt(appletNumber);

    if (strstr(appletType, "bean")) {
        /* It's a bean */
        wb.putInt(1);
    } else {
        /* Its a regular applet */
        wb.putInt(0);
    }

    wb.putInt(argc);
    for (i = 0; i < argc; i++) {
        const char *s;
        wb.putString(argn[i]);

        s = argv[i];
	if(strcasecmp("mayscript",argn[i]) == 0) {
		if(s == NULL || slen(s) < 1 || s[0] == ' ') {
			s = "true";
		}
	}
        wb.putString(s);
    }
    SendRequest(wb, TRUE);
}

/*
 * Send blocking request which waits for a reply
 */
int
JavaVM5::SendBlockingRequest(const CWriteBuffer& wb)
{
    int reply;

    /* Pipe monitor must be held */
    m_pPluginFactory->EnterMonitor("SendRequest");

    /* Check that the command pipe is alive */
    if (state->command_pipe <= 0) {
        plugin_error("SendRequest: Bad pipe. Dead VM?");
        return -1;
    }

    if (!wb.send(PRFileDesc_To_FD(state->command_pipe))) {
        /* Write the message to the command pipe */
        plugin_error("SendRequest: write failed. Dead VM? %d\n", 
                     errno);
        m_pPluginFactory->ExitMonitor("SendRequest-failedread");
        Cleanup();
        return -1;
    }
    PR_Sync(state->command_pipe);

    /* Get the response */
    CReadBuffer rb(PRFileDesc_To_FD(state->command_pipe));
    if (rb.getInt(&reply) <= 0) {
        plugin_error("SendBlockingRequest: Read of ack failed: %d\n", errno);
        m_pPluginFactory->ExitMonitor("SendRequest-replynotread");
        Cleanup();
        return -1;
    }

    m_pPluginFactory->ExitMonitor("SendRequest-ok");

    return reply;
}

/*
     * Send a request to the VM on the command pipe. Depending on
     * wait_for_reply either do or do not wait for an acknowledgement.
     */
void JavaVM5::SendRequest(const CWriteBuffer& wb, int wait_for_reply)
{
    int rc;
    int reply;
    static int request_id = 0;  // To identify requests

    /* Pipe monitor must be held */
    m_pPluginFactory->EnterMonitor("SendRequest");

    request_id++;
    TRACE("Sending command ");

    /* Check that the command pipe is alive */
    if (state->command_pipe <= 0) {
        plugin_error("SendRequest: Bad pipe. Dead VM?");
        m_pPluginFactory->ExitMonitor("SendRequest-badpipe");
        return;
    }

    if (!wb.send(PRFileDesc_To_FD(state->command_pipe))) {
	/* Write the message to the command pipe */
	plugin_error("SendRequest: write failed. Dead VM? %d\n",
			errno);
	m_pPluginFactory->ExitMonitor("SendRequest-failedread");
	Cleanup();
	return;
    }
    rc = PR_Sync(state->command_pipe);

    /* See if we need an ACK from the VM. Return, if not */
    if (wait_for_reply == 0) {
        TRACE("SendRequest: Wrote request. No reply needed.");
        m_pPluginFactory->ExitMonitor("SendRequest-noreply");
        return;
    }
    
    /* Get the ACK */
    CReadBuffer rb(PRFileDesc_To_FD(state->command_pipe));
    TRACE("SendRequest: Wrote request. Waiting for ack.");
    if (rb.getInt(&reply) <= 0) {
        plugin_error("SendRequest: Read of ack failed: %d\n", errno);
        m_pPluginFactory->ExitMonitor("SendRequest-replynotread");
        Cleanup();
        return;
    }

    /* Verify that the ack is OK */
    if (reply == JAVA_PLUGIN_OK) {
        TRACE_INT("SendRequest: Read OK acknowledgement", request_id);
    } else {
        Cleanup();
        plugin_formal_error("Java Plug-in ERROR");
        plugin_error("SendRequest: Got an erroneous ack. %d %d\n", request_id,
                     reply);
    }

    m_pPluginFactory->ExitMonitor("SendRequest-ok");
    return;
}


/* Create a new remote JNI Env for a given secure jni env  */
RemoteJNIEnv* JavaVM5::CreateRemoteJNIEnv(JNIEnv *proxyenv) {
    m_pPluginFactory->EnterMonitor("createenv");

    RemoteJNIEnv *env =  create_RemoteJNIEnv();
    
    int env_index = m_pPluginFactory->RegisterRemoteEnv(env, proxyenv);

    CWriteBuffer wb;
    wb.putInt(JAVA_PLUGIN_ATTACH_THREAD);
    wb.send(PRFileDesc_To_FD(state->command_pipe));

    write_PR_fully("SendingAttachPort", state->command_pipe, 
                   (char*)&(state->port), 4);

    PRNetAddr result_addr;

    PRFileDesc* fd = PR_Accept(state->server_socket, &result_addr, 1000000);
    
    if (fd == NULL) {
        plugin_error("Did not accept a connection");
    }

    int junk_ack;
    PR_Read(fd, &junk_ack, 4);

    TRACE("Read the initial ack");

    if (junk_ack != 5050) 
        plugin_error("Could not read initial ack over the new fd");

    junk_ack++;

    PR_Write(fd, &junk_ack, 4);

    TRACE("Wrote the initial ack");

    int change_fd = PRFileDesc_To_FD(fd);

    {
        int flags = fcntl(change_fd, F_GETFL);
        /*      int newflags = flags  | O_NONBLOCK; */
        int newflags = flags & ~O_NONBLOCK;
        int res = fcntl(change_fd, F_SETFL, newflags);
        if (res == -1)
            fprintf(stderr, "\n***CHANGING FLAGS DID NOT WORK ON BROWSER\n");
        flags = fcntl(change_fd, F_GETFL);
        TRACE_INT("nbrowser flags =", flags);
    }

    m_pPluginFactory->ExitMonitor("createenv");

    init_RemoteJNIEnv(env, env_index, fd);

    {      
        for(int i = 0; i < 2; i++) {
            env->FindClass("java/lang/System");
            env->ExceptionClear();
            env->ExceptionOccurred();
        }
    }
    return env;

}

/*
 * Define the variables needed by the child process. This merely initializes
 * the env_ variables. They are later putenv'd after the fork.
 */
void JavaVM5::SetupChildEnvironment(void) {
  TRACE("JavaVM5::SetupChildEnvironment");

  // Figure out which JRE to use.
  char *jre = FindJRE();
  TRACE_STR("Using JRE from", jre);

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
              jre, LIBARCH,                // libjvm
              jre, LIBARCH);               // libjava, libawt & friends

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

char *JavaVM5::FindJRE(void) {
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

    /* Look in the properties file for javaplugin.jre.path */
    sprintf(path, "%s/.java/deployment/deployment.properties", home); 

    fin = fopen(path, "r");
    if (fin == NULL) {
        return state->java_dir;
    }

    jre[0] = 0;
    while (fgets(line, sizeof(line), fin)) {
        trace_verbose("%s:%s\n", path,line);
        sscanf(line, "javaplugin.jre.path=%s", (char*) &jre);
    }
    fclose(fin);

    /* If there is a javaplugin.jre.path defined, verify that it is ok */
    if (jre[0] != 0) {
	int len = slen(jre)+1;
        /* First check if the property is set to "Default" .
         */
        if (len > 4) {
            const char* jrePrefix = "Default";
            int match = TRUE;
            for(int i = 0; i < 6; i++) {
                if (jre[i] != jrePrefix[i]) 
                    match = FALSE;
            }
            if (match) 
                return state->java_dir;
        }
	
	/* Now check if that directory is reasonable */
        struct stat sbuf;
        sprintf(path, "%s/lib", jre);
        rc = stat(path, &sbuf);
        if (rc == 0) {
            /* We've been given a plausible JRE string */
            int jrelen = slen(jre)+1;
            char *result =(char *) checked_malloc(jrelen);
            memcpy(result, jre, jrelen+1);
            return result;
        }
        plugin_formal_error("Java property javaplugin.jre.path defined as");
        plugin_raw_formal_error(jre);
        plugin_formal_error("But that directory does not exist."); 
        plugin_formal_error("Using JRE from");
        plugin_raw_formal_error(state->java_dir);
    }
    return state->java_dir;
}


void JavaVM5::FindJavaDir(void) {
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
    *(strrchr(real, '/')) = '\0';  // NS600 subdirector used to isolate ongoing version
    *(strrchr(real, '/')) = '\0';  /* $(LIBARCH)              */
    *(strrchr(real, '/')) = '\0';  /* plugin               */

    state->java_dir = strdup(real);
}


/*
 * Report an error in performing work, and terminate the jvm
 */
void JavaVM5::WorkError(int x) {
    perror("Plugin worker error");
    fprintf(stderr, "%s (%d)\n",
            DGETTEXT(JAVA_PLUGIN_DOMAIN, 
                     "Plugin: trouble with work request from child"),
            x);
    Cleanup();
}

PRFileDesc* JavaVM5::GetWorkPipe() {
    return state->work_pipe;
}

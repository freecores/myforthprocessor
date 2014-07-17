/*
 * @(#)JavaVM5.h	1.36 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef JAVAVM5_H
#define JAVAVM5_H

#include "jni.h"
#include "CWriteBuffer.h"

class JavaPluginInstance5;

class JavaPluginFactory5;

enum nsJVMStatus {
    nsJVMStatus_Enabled,  // but not Running
    nsJVMStatus_Disabled, // explicitly disabled
    nsJVMStatus_Running,  // enabled and started
    nsJVMStatus_Failed    // enabled but failed to start
};

struct LongTermState {

    /* Command pipe used for applet and thread initiation reqursts and resp */
    PRFileDesc* command_pipe;

    /* Worker pipe used to receive applet related requests */
    PRFileDesc* work_pipe;

    /* Java to Browser pipe used for spontaneous JS requests/responses */
    PRFileDesc* spont_pipe;

    /* Java to Browser pipe used for printing return data. 
     XXXFIX no printing support yet */
    PRFileDesc* print_pipe;

    /* Socket for accepting attach_thread requests */
    PRFileDesc* server_socket;

    /* Port for connecting to the server socket */
    int port;

    /* The directory the Java Plug-in JRE is installed in */
    char* java_dir;

    /* Process ID of child process */
    int child_pid;
};

class JavaVM5 {
public:
    /* Constructor and destructor */
    JavaVM5(JavaPluginFactory5 *plugin);
    virtual ~JavaVM5();

    /* Fork and exec the java_vm process, after setting up 
       the child environment*/
    int StartJavaVM(const char * addClasspath);
    /* Terminate the connection to the VM process */
    void ShutdownJavaVM(int RealShutdown);
    /* Create a new applet */
    void CreateApplet(const char *appletType, int appletNumber, 
				   int argc, char **argn, char **argv);

    /* Return the plugin factory */
    JavaPluginFactory5* GetPluginFactory(void);

    /* Long term state of the plugin */
    struct LongTermState *state;

    /* Send a request on the command pipe */
    void SendRequest(const CWriteBuffer& wb, int wait_for_reply);

    /* Return true if the VM is enabled */
    int GetJVMEnabled(void);

    /* Separate thread function that handles work requests */
    void ProcessWorkQueue(void); 

    /* Separate thread function that handles spontaneous requests */
    void ProcessSpontaneousQueue(void); 

    /* Create a remote jni  for a secure jni */
    RemoteJNIEnv* CreateRemoteJNIEnv(JNIEnv* proxyjni);

    /* Get the java object for the instance numbered 'plugin_number' */
    jobject GetJavaObjectForInstance(int plugin_number);
    
    /* Receive printing data from the Plugin. 
     */
    void ReceivePrinting(FILE* fp);

    PRFileDesc* GetWorkPipe(void);

protected:
    /* Send a blocking request on the command pipe, with an INT result */
    int SendBlockingRequest(const CWriteBuffer& wb);
    
    NS_IMETHOD_(void) TerminateRequestAbruptly(const char* msg);
    
    /* Actually do the work for a single request */
    void DoWork();

    /* An error occurred when reading on the worker pipe. */
    void WorkError(int x);

    void Cleanup(void);

    /* Setup the child process environment variables and write them into 
     * the static variables env_*
     * LD_LIBRARY_PATH etc.
     */
    void SetupChildEnvironment(void);

    /* Find the location of the JRE, starting with either the .java/deployment/deployment.properties
     * javaplugin.jre.path= or the location of the plugin i.e. the 
     * javadirectory obtained by FindJavaDir
     */
    char* FindJRE(void);

    /* Find the location of the directory where the plugin java code
     * might live, starting with NPX_PLUGIN_PATH and then 
     * HOME/.netscape/plugins 
     */
    void FindJavaDir(void);
 
private:


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
    char* env_LD_LIBRARY_PATH;
    char* env_JAVA_HOME;
    char* env_PLUGIN_HOME;
    char stateENVName[100];

    /* Remote JNIEnv for the current spontaneous call */
    RemoteJNIEnv* spont_remotejni;

    /* Current VM status - whether the VM is actually running */
    nsJVMStatus jvm_status;

    /* Pointer to plugin factory */
    JavaPluginFactory5* m_pPluginFactory;

    PRMonitor * workMonitor;
    bool workPipeClean;
    PRMonitor * spontMonitor;
    bool spontPipeClean;
};

#endif

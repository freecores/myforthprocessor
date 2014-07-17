/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-*/
/*
 * @(#)CJavaVM.h	1.24 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


enum nsJVMStatus {
    nsJVMStatus_Enabled,  // but not Running
    nsJVMStatus_Disabled, // explicitly disabled
    nsJVMStatus_Running,  // enabled and started
    nsJVMStatus_Failed    // enabled but failed to start
};

struct LongTermState {
    /* This pipe is used for sending commands to the Java process */
    int command_pipe;
    /* This pipe is used for receiving work from the Java process */
    int work_pipe;
    /* This pipe is used for receiving printing from the VM */
    int print_pipe;

    /* The directory the Java Plug-in JRE is installed in */
    char *java_dir;

    /* Process ID of child process */
    int child_pid;
    XtInputId inputid;
};

class CJavaVM {
public:
    CJavaVM(CJavaPluginFactory *plugin);
    /* Fork and exec the java_vm process, after setting up 
       the child environment*/
    NS_IMETHOD StartJavaVM(const char * addClasspath);
    NS_IMETHOD_(void) ShutdownJavaVM(int RealShutdown = 0);
    NS_IMETHOD_(void) ProcessWorkQueue(void);
    NS_IMETHOD_(void) SendRequest(char *buff, int len, int wait_for_reply);
    NS_IMETHOD_(void) CreateApplet(const char *appletType, int appletNumber, 
				   int argc, char **argn, char **argv);
    virtual ~CJavaVM();
    struct LongTermState *state;
    NS_IMETHOD_(void) ReceivePrinting(FILE *fp);
    NS_IMETHOD_(void) TerminateRequestAbruptly(const char* msg);

protected:
    NS_IMETHOD_(void) DoWork(void);
    NS_IMETHOD_(char *) ReadWorkString(void);
    NS_IMETHOD_(void) WorkError(int x);
    NS_IMETHOD_(void) Cleanup(void);
    NS_IMETHOD_(void) SetupChildEnvironment(void);
    NS_IMETHOD_(char *) FindJRE(void);
    NS_IMETHOD_(void) FindJavaDir(void);
    NS_IMETHOD_(void) ConnectPipeEvent(int);
    // Sets the variable

private:

    nsJVMStatus mStatus;
    CJavaPluginFactory  *mPlugin;

    char stateENVName[100];
};

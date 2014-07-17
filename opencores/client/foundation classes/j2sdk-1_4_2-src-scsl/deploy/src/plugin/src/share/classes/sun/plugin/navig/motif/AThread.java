/*
 * @(#)AThread.java	1.9 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/* This is the Java end of a thread attached from communicator */

package sun.plugin.navig.motif;

/* AThread represents the state associated with a particular Java
   thread which is receiving or making JNI/JS calls. In the case of a
   JS thread, the AThread represents the actual thread object, which
   is started up at the point of an attach thread.  The set of
   AThreads is maintained in the OJIPlugin class */
class AThread extends Thread {

    /* Called from server.c to create a new thread */
    private AThread(int pipe){
	initIfFirstCall();
	this.pipe = pipe;
    }

    public void run() {
	OJIPlugin.registerThread(this);
	JNIHandleLoop();
    }

    public int getPipe() {
      return pipe;
    }
    
    void JNIHandleLoop() {
	handleRequest(pipe);
    }

    /* Initialize the global method ids etc if this is the first call */
    private static synchronized void initIfFirstCall() {
	if (is_initialized) return;
	initGlobals();
	is_initialized = true;
    }


    private native void handleRequest(int pipe);
    private native static void initGlobals();

    private int pipe = -1;

    /* Need to perform static initialization, but may not want to do this
       too early since the shared library with the native code may not
       yet have been linked in */
    private static boolean is_initialized; 


}

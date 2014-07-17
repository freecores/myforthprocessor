/*
 * "@(#)LinuxThread.java	1.3 03/01/23 11:30:01"
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.debugger.linux;

import sun.jvm.hotspot.debugger.*;

class LinuxThread implements ThreadProxy {
    private LinuxDebugger debugger;
    private int           thread_id;

    /** The address argument must be the address of the _thread_id in the
        OSThread. 
        FIXME: we should also save the pthread_id !! */
    LinuxThread(LinuxDebugger debugger, Address addr) {
        this.debugger = debugger;
        // FIXME: size of data fetched here should be configurable.
        // However, making it so would produce a dependency on the "types"
        // package from the debugger package, which is not desired.
        this.thread_id = (int) addr.getCIntegerAt(0, 4, true);
    }

    LinuxThread(LinuxDebugger debugger, long id) {
        this.debugger = debugger;
        this.thread_id = (int) id;
    }

    public boolean equals(Object obj) {
        if ((obj == null) || !(obj instanceof LinuxThread)) {
            return false;
        }

        return (((LinuxThread) obj).thread_id == thread_id);
    }

    public int hashCode() {
        return thread_id;
    }

    public ThreadContext getContext() throws IllegalThreadStateException {
        long[] data = debugger.getThreadIntegerRegisterSet(thread_id);
        LinuxThreadContext context = new LinuxThreadContext(debugger);
        for (int i = 0; i < data.length; i++) {
            context.setRegister(i, data[i]);
        }
        return context;
    }

    public boolean canSetContext() throws DebuggerException {
        return false;
    }

    public void setContext(ThreadContext context)
      throws IllegalThreadStateException, DebuggerException {
        throw new DebuggerException("Unimplemented");
    }
}

/*
 * @(#)LinuxDebuggerLocal.java	1.5 03/01/23 11:29:56
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.debugger.linux;

import java.io.*;
import java.net.*;
import java.util.*;
import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.debugger.x86.*;
import sun.jvm.hotspot.debugger.cdbg.*;
import sun.jvm.hotspot.utilities.*;
import java.lang.reflect.*;

/** <P> An implementation of the JVMDebugger interface. The basic debug
    facilities are implemented through ptrace interface in the JNI code
    (libsa.so). Library maps and symbol table management are done in
    JNI. </P>

    <P> <B>NOTE</B> that since we have the notion of fetching "Java
    primitive types" from the remote process (which might have
    different sizes than we expect) we have a bootstrapping
    problem. We need to know the sizes of these types before we can
    fetch them. The current implementation solves this problem by
    requiring that it be configured with these type sizes before they
    can be fetched. The readJ(Type) routines here will throw a
    RuntimeException if they are called before the debugger is
    configured with the Java primitive type sizes. </P> */

public class LinuxDebuggerLocal extends DebuggerBase implements LinuxDebugger {
    private boolean attached;
    private long pid;

    // native methods

    private native static void init0() 
                                throws DebuggerException;
    private native void attach0(int pid) 
                                throws DebuggerException;
    private native void detach0() 
                                throws DebuggerException;
    private native long lookup0(String objectName, String symbol) 
                                throws DebuggerException;
    private native long[] getThreadIntegerRegisterSet0(int pid) 
                                throws DebuggerException;
    private native byte[] readBytesFromProcess0(long address, long numBytes) 
                                throws DebuggerException;

    // Note on Linux threads are really processes. When target process is
    // attached by a serviceability agent thread, only that thread can do
    // ptrace operations on the target. This is because from kernel's point
    // view, other threads are just separate processes and they are not 
    // attached to the target. When they attempt to make ptrace calls,
    // an ESRCH error will be returned as kernel believes target is not
    // being traced by the caller.
    // To work around the problem, we use a worker thread here to handle 
    // all JNI functions that are making ptrace calls.

    interface WorkerThreadTask {
       public void doit(LinuxDebuggerLocal debugger);
    }

    class LinuxDebuggerLocalWorkerThread extends Thread {
       LinuxDebuggerLocal debugger;
       WorkerThreadTask task;

       public LinuxDebuggerLocalWorkerThread(LinuxDebuggerLocal debugger) {
         this.debugger = debugger;
       }

       public void run() {
          synchronized (workerThread) {
             for (;;) {
                if (task != null) {
                   task.doit(debugger);
                   task = null;
                   workerThread.notifyAll();
                }

                try {
                   workerThread.wait();
                } catch (InterruptedException x) {}
             }
          }
       }

       public WorkerThreadTask execute(WorkerThreadTask task) {
          synchronized (workerThread) {
             this.task = task;
             workerThread.notifyAll();
             while (this.task != null)
                try {
                   workerThread.wait();
                } catch (InterruptedException x) {}
             return task;
          }
       }
    }

    private static LinuxDebuggerLocalWorkerThread workerThread = null;

    //----------------------------------------------------------------------
    // Implementation of Debugger interface
    //

    /** <P> machDesc may not be null. </P>

    <P> useCache should be set to true if debugging is being done
    locally, and to false if the debugger is being created for the
    purpose of supporting remote debugging. </P> */
    public LinuxDebuggerLocal(MachineDescription machDesc,
                              boolean useCache) throws DebuggerException {
        this.machDesc = machDesc;
        utils = new DebuggerUtilities(machDesc.getAddressSize(),
                                      machDesc.isBigEndian()) {
           public void checkAlignment(long address, long alignment) {
             // Need to override default checkAlignment because we need to
             // relax alignment constraints on Linux/x86
             if ( (address % alignment != 0)
                &&(alignment != 8 || address % 4 != 0)) {
                throw new UnalignedAddressException(
                        "Trying to read at address: " 
                      + addressValueToString(address)
                      + " with alignment: " + alignment,
                        address);
             }
           }
        };

        if (useCache) {
            // FIXME: re-test necessity of cache on Linux, where data
            // fetching is faster
            // Cache portion of the remote process's address space.
            // Fetching data over the socket connection to dbx is slow.
            // Might be faster if we were using a binary protocol to talk to
            // dbx, but would have to test. For now, this cache works best
            // if it covers the entire heap of the remote process. FIXME: at
            // least should make this tunable from the outside, i.e., via
            // the UI. This is a cache of 4096 4K pages, or 16 MB. The page
            // size must be adjusted to be the hardware's page size.
            // (FIXME: should pick this up from the debugger.)
            initCache(4096, parseCacheNumPagesProperty(4096));
        }

        workerThread = new LinuxDebuggerLocalWorkerThread(this);
        workerThread.start();
    }

    /** From the Debugger interface via JVMDebugger */
    public boolean hasProcessList() throws DebuggerException {
        return true;
    }

    /** From the Debugger interface via JVMDebugger */
    public List getProcessList() throws DebuggerException {
        throw new DebuggerException("getProcessList not implemented yet");
    }

    /** From the Debugger interface via JVMDebugger */
    public synchronized void attach(int processID) throws DebuggerException {
        if (attached) {
            throw new DebuggerException("Already attached to process " + pid);
        }

        class AttachTask implements WorkerThreadTask {
           int pid;
           public void doit(LinuxDebuggerLocal debugger) {
              debugger.attach0(pid);
              debugger.attached = true;
              debugger.pid = pid;
           }
        }

        AttachTask task = new AttachTask();
        task.pid = processID;
        workerThread.execute(task);
    }

    /** From the Debugger interface via JVMDebugger */
    public synchronized void attach(String executableName, String coreFileName)
            throws DebuggerException {
        throw new DebuggerException("Core files not yet supported on Linux");
    }

    /** From the Debugger interface via JVMDebugger */
    public synchronized boolean detach() {
        if (!attached) {
            return false;
        }

        class DetachTask implements WorkerThreadTask {
           boolean result = false;

           public void doit(LinuxDebuggerLocal debugger) {
              debugger.detach0();
              debugger.pid = 0;
              debugger.attached = false;
              result = true;
           }
        }

        DetachTask task = new DetachTask();
        workerThread.execute(task);
        return task.result;
    }

    /** From the Debugger interface via JVMDebugger */
    public Address parseAddress(String addressString)
            throws NumberFormatException {
        long addr = utils.scanAddress(addressString);
        if (addr == 0) {
            return null;
        }
        return new LinuxAddress(this, addr);
    }

    /** From the Debugger interface via JVMDebugger */
    public String getOS() {
        return PlatformInfo.getOS();
    }

    /** From the Debugger interface via JVMDebugger */
    public String getCPU() {
        return PlatformInfo.getCPU();
    }

    public boolean hasConsole() throws DebuggerException {
        return false;
    }

    public String consoleExecuteCommand(String cmd) throws DebuggerException {
        throw new DebuggerException("No debugger console available on Linux");
    }

    public String getConsolePrompt() throws DebuggerException {
        return null;
    }

    public CDebugger getCDebugger() {
        return null;
    }

    /** From the SymbolLookup interface via Debugger and JVMDebugger */
    public synchronized Address lookup(String objectName, String symbol) {
        if (!attached) {
            return null;
        }

        class LookupTask implements WorkerThreadTask {
           String objectName, symbol;
           Address result;

           public void doit(LinuxDebuggerLocal debugger) {
              long addr = debugger.lookup0(objectName, symbol);
              result = (addr == 0 ? null : new LinuxAddress(debugger, addr));
           }
        }

        LookupTask task = new LookupTask();
        task.objectName = objectName;
        task.symbol = symbol;
        workerThread.execute(task);
        return task.result;
    }

    /** From the SymbolLookup interface via Debugger and JVMDebugger */
    public synchronized OopHandle lookupOop(String objectName, String symbol) {
        Address addr = lookup(objectName, symbol);
        if (addr == null) {
            return null;
        }
        return addr.addOffsetToAsOopHandle(0);
    }

    /** From the Debugger interface */
    public MachineDescription getMachineDescription() {
        return machDesc;
    }

    //----------------------------------------------------------------------
    // Implementation of ThreadAccess interface
    //

    /** From the ThreadAccess interface via Debugger and JVMDebugger */
    public ThreadProxy getThreadForIdentifierAddress(Address addr) {
        return new LinuxThread(this, addr);
    }

    /** From the ThreadAccess interface via Debugger and JVMDebugger */
    public ThreadProxy getThreadForThreadId(long id) {
        return new LinuxThread(this, id);
    }

    //----------------------------------------------------------------------
    // Internal routines (for implementation of LinuxAddress).
    // These must not be called until the MachineDescription has been set up.
    //

    /** From the LinuxDebugger interface */
    public String addressValueToString(long address) {
        return utils.addressValueToString(address);
    }

    /** From the LinuxDebugger interface */
    public LinuxAddress readAddress(long address)
            throws UnmappedAddressException, UnalignedAddressException {
        long value = readAddressValue(address);
        return (value == 0 ? null : new LinuxAddress(this, value));
    }

    /** From the LinuxDebugger interface */
    public LinuxOopHandle readOopHandle(long address)
            throws UnmappedAddressException, UnalignedAddressException,
                NotInHeapException {
        long value = readAddressValue(address);
        return (value == 0 ? null : new LinuxOopHandle(this, value));
    }

    //----------------------------------------------------------------------
    // Thread context access
    //

    public synchronized long[] getThreadIntegerRegisterSet(int pid)
                                            throws DebuggerException {
        class GetThreadIntegerRegisterSetTask implements WorkerThreadTask {
           int pid;
           long[] result;
           public void doit(LinuxDebuggerLocal debugger) {
              result = debugger.getThreadIntegerRegisterSet0(pid);
           }
        }

        GetThreadIntegerRegisterSetTask task = new GetThreadIntegerRegisterSetTask();
        task.pid = pid;
        workerThread.execute(task);
        return task.result;
    }

    //----------------------------------------------------------------------
    // Address access. Can not be package private, but should only be
    // accessed by the architecture-specific subpackages.

    /** From the LinuxDebugger interface */
    public long getAddressValue(Address addr) {
      if (addr == null) return 0;
      return ((LinuxAddress) addr).getValue();
    }

    /** From the LinuxDebugger interface */
    public Address newAddress(long value) {
      if (value == 0) return null;
      return new LinuxAddress(this, value);
    }

    /** This reads bytes from the remote process. */
    public synchronized ReadResult readBytesFromProcess(long address,
            long numBytes) throws UnmappedAddressException, DebuggerException {

        class ReadBytesFromProcessTask implements WorkerThreadTask {
           long address, numBytes;
           ReadResult result;
           public void doit(LinuxDebuggerLocal debugger) {
              byte[] res = debugger.readBytesFromProcess0(address, numBytes);
              if (res != null)
                 result = new ReadResult(res);
              else
                 result = new ReadResult(address);
           }
        }

        ReadBytesFromProcessTask task = new ReadBytesFromProcessTask();
        task.address = address;
        task.numBytes = numBytes;
        workerThread.execute(task);
        return task.result;
    }

    public void writeBytesToProcess(long address, long numBytes, byte[] data)
        throws UnmappedAddressException, DebuggerException {
        // FIXME
        throw new DebuggerException("Unimplemented");
    }

    static {
        System.loadLibrary("sa");
        init0();
    }
}

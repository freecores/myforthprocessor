/*
 * @(#)WindbgDebuggerLocal.java	1.9 03/01/23 11:35:44
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.debugger.windbg;

import java.io.*;
import java.net.*;
import java.util.*;
import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.debugger.x86.*;
import sun.jvm.hotspot.debugger.win32.coff.*;
import sun.jvm.hotspot.debugger.cdbg.*;
import sun.jvm.hotspot.debugger.cdbg.basic.BasicDebugEvent;
import sun.jvm.hotspot.utilities.*;
import sun.jvm.hotspot.utilities.memo.*;

/** <P> An implementation of the JVMDebugger interface which talks to
    windbg and symbol table management is done in Java. </P>

    <P> <B>NOTE</B> that since we have the notion of fetching "Java
    primitive types" from the remote process (which might have
    different sizes than we expect) we have a bootstrapping
    problem. We need to know the sizes of these types before we can
    fetch them. The current implementation solves this problem by
    requiring that it be configured with these type sizes before they
    can be fetched. The readJ(Type) routines here will throw a
    RuntimeException if they are called before the debugger is
    configured with the Java primitive type sizes. </P> */

public class WindbgDebuggerLocal extends DebuggerBase implements WindbgDebugger {
  private PageCache cache;
  private boolean   attached;
  // Symbol lookup support
  // This is a map of library names to DLLs
  private Map nameToDllMap;

  // C/C++ debugging support
  private List/*<LoadObject>*/ loadObjects;
  private CDebugger cdbg;

  // thread access
  private Map threadIntegerRegisterSet;
  private List threadList;

  // windbg native interface pointers

  private long ptrIDebugClient;
  private long ptrIDebugControl;
  private long ptrIDebugDataSpaces;
  private long ptrIDebugOutputCallbacks;
  private long ptrIDebugRegisters;
  private long ptrIDebugSymbols;
  private long ptrIDebugSystemObjects;

  private void resetNativePointers() {
    ptrIDebugClient          = 0L;
    ptrIDebugControl         = 0L;
    ptrIDebugDataSpaces      = 0L;
    ptrIDebugOutputCallbacks = 0L;
    ptrIDebugRegisters       = 0L;
    ptrIDebugSymbols         = 0L;
    ptrIDebugSystemObjects   = 0L;
  }

  //--------------------------------------------------------------------------------
  // Implementation of Debugger interface
  //

  /** <P> machDesc may not be null. </P>

      <P> useCache should be set to true if debugging is being done
      locally, and to false if the debugger is being created for the
      purpose of supporting remote debugging. </P> */
  public WindbgDebuggerLocal(MachineDescription machDesc,
                            boolean useCache) throws DebuggerException {
    this.machDesc = machDesc;
    utils = new DebuggerUtilities(machDesc.getAddressSize(), machDesc.isBigEndian());
    if (useCache) {
      // Cache portion of the remote process's address space.
      // Fetching data over the socket connection to dbx is slow.
      // Might be faster if we were using a binary protocol to talk to
      // dbx, but would have to test. For now, this cache works best
      // if it covers the entire heap of the remote process. FIXME: at
      // least should make this tunable from the outside, i.e., via
      // the UI. This is a cache of 4096 4K pages, or 16 MB. The page
      // size must be adjusted to be the hardware's page size.
      // (FIXME: should pick this up from the debugger.)
      initCache(4096, 4096);
    }
    // FIXME: add instantiation of thread factory

  }

  /** From the Debugger interface via JVMDebugger */
  public boolean hasProcessList() throws DebuggerException {
    return false;
  }

  /** From the Debugger interface via JVMDebugger */
  public List getProcessList() throws DebuggerException {
    return null;
  }

  /** From the Debugger interface via JVMDebugger */
  public synchronized void attach(int processID) throws DebuggerException {
    throw new DebuggerException("Unimplemented, windbg debugger is only for Dr. Watson dumps!");
  }

  /** From the Debugger interface via JVMDebugger */
  public synchronized void attach(String executableName, String coreFileName) throws DebuggerException {
    if (attached) {
       throw new DebuggerException("alreay attached to a Dr. Watson dump!");
    }

    resetNativePointers();
    loadObjects = new ArrayList();
    nameToDllMap = new HashMap();
    threadIntegerRegisterSet = new HashMap();
    threadList = new ArrayList();
    attach0(executableName, coreFileName);
    attached = true;
  }

  public List getLoadObjectList() {
    return loadObjects;
  }

  /** From the Debugger interface via JVMDebugger */
  public synchronized boolean detach() {
    if ( ! attached) 
       return false;

    // Close all open DLLs
    if (nameToDllMap != null) {
      for (Iterator iter = nameToDllMap.values().iterator(); iter.hasNext(); ) {
        DLL dll = (DLL) iter.next();
        dll.close();
      }
      nameToDllMap = null;
      loadObjects = null;
    }

    cdbg = null;
    clearCache();

    threadIntegerRegisterSet = null;
    threadList = null;
    try {
       detach0();
    } finally {
       attached = false;
       resetNativePointers();
    }
    return true;
  }


  /** From the Debugger interface via JVMDebugger */
  public Address parseAddress(String addressString) throws NumberFormatException {
    return newAddress(utils.scanAddress(addressString));
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
    return true;
  }

  public String consoleExecuteCommand(String cmd) throws DebuggerException {
    if (! attached) {
       throw new DebuggerException("debugger not yet attached to a Dr. Watson dump!");
    }

    return consoleExecuteCommand0(cmd);
  }

  public String getConsolePrompt() throws DebuggerException {
    return "(windbg)";
  }

  public CDebugger getCDebugger() throws DebuggerException {
    if (cdbg == null) {
      cdbg = new WindbgCDebugger(this);
    }
    return cdbg;
  }

  /** From the SymbolLookup interface via Debugger and JVMDebugger */
  public synchronized Address lookup(String objectName, String symbol) {
    return newAddress(lookupInProcess(objectName, symbol));
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

  //--------------------------------------------------------------------------------
  // Implementation of ThreadAccess interface
  //


  /** From the ThreadAccess interface via Debugger and JVMDebugger */
  public ThreadProxy getThreadForIdentifierAddress(Address addr) {
    return new WindbgThread(this, addr);
  }

  public ThreadProxy getThreadForThreadId(long handle) {
    // with windbg we can't make out using handle
    throw new DebuggerException("Unimplemented!");
  }

  public long getThreadIdFromSysId(long sysId) throws DebuggerException {
    return getThreadIdFromSysId0(sysId);
  }

  //----------------------------------------------------------------------
  // Overridden from DebuggerBase because we need to relax alignment
  // constraints on x86

  public long readJLong(long address)
    throws UnmappedAddressException, UnalignedAddressException {
    checkJavaConfigured();
    // FIXME: allow this to be configurable. Undesirable to add a
    // dependency on the runtime package here, though, since this
    // package should be strictly underneath it.
    //    utils.checkAlignment(address, jlongSize);
    utils.checkAlignment(address, jintSize);
    byte[] data = readBytes(address, jlongSize);
    return utils.dataToJLong(data, jlongSize);
  }  

  //--------------------------------------------------------------------------------
  // Internal routines (for implementation of WindbgAddress).
  // These must not be called until the MachineDescription has been set up.
  //

  /** From the WindbgDebugger interface */
  public String addressValueToString(long address) {
    return utils.addressValueToString(address);
  }

  /** From the WindbgDebugger interface */
  public WindbgAddress readAddress(long address)
    throws UnmappedAddressException, UnalignedAddressException {
    return (WindbgAddress) newAddress(readAddressValue(address));
  }

  /** From the WindbgDebugger interface */
  public WindbgOopHandle readOopHandle(long address)
    throws UnmappedAddressException, UnalignedAddressException, NotInHeapException {
    long value = readAddressValue(address);
    return (value == 0 ? null : new WindbgOopHandle(this, value));
  }

  //--------------------------------------------------------------------------------
  // Thread context access
  //

  private synchronized void setThreadIntegerRegisterSet(long threadId, 
                                               long[] regs) {
    threadIntegerRegisterSet.put(new Long(threadId), regs);
  }

  private synchronized void addThread(long sysId) {
    threadList.add(new WindbgThread(this, sysId));
  }

  public synchronized long[] getThreadIntegerRegisterSet(long threadId)
    throws DebuggerException {
    return (long[]) threadIntegerRegisterSet.get(new Long(threadId));
  }
  
  public synchronized List getThreadList() throws DebuggerException {
    return threadList;
  }

  private String findFullPath(String file) {
    File f = new File(file);
    if (f.exists()) {
       return file;
    } else {
       // remove path part, if any.
       file = f.getName();
       StringTokenizer st = new StringTokenizer(imagePath, File.pathSeparator);
       while (st.hasMoreTokens()) {
          f = new File(st.nextToken(), file);
          if (f.exists()) {
             return f.getPath();
          }
       }
    }
    return null;
  }

  private synchronized void addLoadObject(String file, long size, long base) {
    String path = findFullPath(file);
    if (path != null) {
       DLL dll = new DLL(this, path, size,newAddress(base));
       loadObjects.add(dll);
       nameToDllMap.put(new File(file).getName(), dll);
    }
  }

  //--------------------------------------------------------------------------------
  // Address access
  //

  /** From the Debugger interface */
  public long getAddressValue(Address addr) {
    return ((WindbgAddress) addr).getValue();
  }

  /** From the WindbgDebugger interface */
  public Address newAddress(long value) {
    if (value == 0) return null;
    return new WindbgAddress(this, value);
  }

  //--------------------------------------------------------------------------------
  // Internals only below this point
  //

  /** Looks up an address in the remote process's address space.
      Returns 0 if symbol not found or upon error. Package private to
      allow WindbgDebuggerRemoteIntfImpl access. NOTE that this returns
      a long instead of an Address because we do not want to serialize
      Addresses. */
  synchronized long lookupInProcess(String objectName, String symbol) {
    DLL dll = (DLL) nameToDllMap.get(objectName);
    // The DLL can be null because we use this to search through known
    // DLLs in HotSpotTypeDataBase (for example)
    if (dll != null) {
      WindbgAddress addr = (WindbgAddress) dll.lookupSymbol(symbol);
      if (addr != null) {
        return addr.getValue();
      }
    }
    return 0;
  }

  /** This reads bytes from the remote process. */
  public synchronized ReadResult readBytesFromProcess(long address, long numBytes)
    throws UnmappedAddressException, DebuggerException {
    byte[] res = readBytesFromProcess0(address, numBytes);
    if(res != null)
       return new ReadResult(res);
    else
       return new ReadResult(address);
  }

    
  private DLL findDLLByName(String fullPathName) {
    for (Iterator iter = loadObjects.iterator(); iter.hasNext(); ) {
      DLL dll = (DLL) iter.next();
      if (dll.getName().equals(fullPathName)) {
        return dll;
      }
    }
    return null;
  }

  public void writeBytesToProcess(long address, long numBytes, byte[] data)
    throws UnmappedAddressException, DebuggerException {
    // FIXME
    throw new DebuggerException("Unimplemented");
  }

  private static String imagePath;

  static {
    System.loadLibrary("sawindbg");
    imagePath = System.getProperty("sun.jvm.hotspot.debugger.windbg.imagePath");
    if (imagePath == null) {
       imagePath = System.getProperty("os.path");
    }

    initIDs();
  }

  // native methods
  private static native void initIDs();
  private native void attach0(String executableName, String coreFileName);
  private native void detach0();
  private native byte[] readBytesFromProcess0(long address, long numBytes)
    throws UnmappedAddressException, DebuggerException;
  private native long getThreadIdFromSysId0(long sysId);
  private native synchronized String consoleExecuteCommand0(String cmd);
}

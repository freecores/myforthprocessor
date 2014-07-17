/*
 * @(#)ProcDebuggerLocal.java	1.9 03/01/23 11:30:39
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.debugger.proc;

import java.io.*;
import java.net.*;
import java.util.*;
import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.debugger.cdbg.*;
import sun.jvm.hotspot.debugger.proc.sparc.*;
import sun.jvm.hotspot.debugger.proc.x86.*;
import sun.jvm.hotspot.utilities.*;

/** <P> An implementation of the JVMDebugger interface which sits on
    top of proc and relies on the SA's proc import module for
    communication with the debugger. </P>

    <P> <B>NOTE</B> that since we have the notion of fetching "Java
    primitive types" from the remote process (which might have
    different sizes than we expect) we have a bootstrapping
    problem. We need to know the sizes of these types before we can
    fetch them. The current implementation solves this problem by
    requiring that it be configured with these type sizes before they
    can be fetched. The readJ(Type) routines here will throw a
    RuntimeException if they are called before the debugger is
    configured with the Java primitive type sizes. </P>
*/

public class ProcDebuggerLocal extends DebuggerBase implements ProcDebugger {
  protected boolean unalignedAccessesOkay;
  protected ProcThreadFactory threadFactory;
  protected long ps_prochandle_ptr;

  // libthread.so's dlopen handle and function pointers
  protected long tdb_handle;
  protected long p_td_init;
  protected long p_td_ta_new;
  protected long p_td_ta_delete;
  protected long p_td_ta_thr_iter;
  protected long p_td_thr_get_info;
  protected long p_td_thr_getgregs;

  // Part of workaround for 4705086.

  // libjvm[_g].so's file descriptor
  protected int  libjvm_fd;

  // starting address of libjvm[_g].so's text segment.
  protected long libjvm_text_start;

  // size of text in libjvm[_g].so.
  protected long libjvm_text_size;

  protected static final int cacheSize = 16 * 1024 * 1024; // 16 MB

  //--------------------------------------------------------------------------------
  // Implementation of Debugger interface
  //

  /** <P> machDesc may be null if it couldn't be determined yet; i.e.,
      if we're on SPARC, we need to ask the remote process whether
      we're in 32- or 64-bit mode. </P>

      <P> useCache should be set to true if debugging is being done
      locally, and to false if the debugger is being created for the
      purpose of supporting remote debugging. </P> */
  public ProcDebuggerLocal(MachineDescription machDesc,
                          boolean useCache) {
    this.machDesc = machDesc;
    int cacheNumPages;
    int cachePageSize;

    if (PlatformInfo.getCPU().equals("sparc")) {
      threadFactory = new ProcSPARCThreadFactory(this);
    } else if (PlatformInfo.getCPU().equals("x86")) {
      threadFactory = new ProcX86ThreadFactory(this);
      unalignedAccessesOkay = true;
    } else {
      throw new RuntimeException("Thread access for CPU architecture " + PlatformInfo.getCPU() + " not yet supported");
    }
     if (useCache) {
      // Cache portion of the remote process's address space.
      // For now, this cache works best if it covers the entire
      // heap of the remote process. FIXME: at least should make this
      // tunable from the outside, i.e., via the UI. This is a 16 MB
      // cache divided on SPARC into 2048 8K pages and on x86 into
      // 4096 4K pages; the page size must be adjusted to be the OS's
      // page size.

      cachePageSize = getPageSize();
      cacheNumPages = parseCacheNumPagesProperty(cacheSize / cachePageSize);
      initCache(cachePageSize, cacheNumPages);
    }

    resetNativePointers();
  }

  /** FIXME: implement this with a Runtime.exec() of ps followed by
      parsing of its output */
  public boolean hasProcessList() throws DebuggerException {
    return false;
  }

  public List getProcessList() throws DebuggerException {
    throw new DebuggerException("Not yet supported");
  }

  /** From the Debugger interface via JVMDebugger */
  public synchronized void attach(int processID) throws DebuggerException {
    attach0(new Integer(processID).toString());
  }

  /** From the Debugger interface via JVMDebugger */
  public synchronized void attach
	(String executableName, String coreFileName) throws DebuggerException {
    attach0(coreFileName);
  }

  /** From the Debugger interface via JVMDebugger */
  public synchronized boolean detach() {
    try {
      if (ps_prochandle_ptr == 0L) {
        return false;
      }
      detach0(); 
      clearCache();
      return true;
    } catch (Exception e) {
      e.printStackTrace();
      return false;
    } finally {
       resetNativePointers();
    }
  }

  private void resetNativePointers() {
       ps_prochandle_ptr = 0L;

       // reset thread_db pointers
       tdb_handle        = 0L;
       p_td_init         = 0L;
       p_td_ta_new       = 0L;
       p_td_ta_delete    = 0L;
       p_td_ta_thr_iter  = 0L;
       p_td_thr_get_info = 0L;
       p_td_thr_getgregs = 0L;

       // Part of workaround for 4705086.

       // uninitialize the workaround data.
       libjvm_fd = -1;
       libjvm_text_start = 0L;
       libjvm_text_size = 0L;
  }

  /** From the Debugger interface via JVMDebugger */
  public Address parseAddress(String addressString) throws NumberFormatException {
    long addr = utils.scanAddress(addressString);
    if (addr == 0) {
      return null;
    }
    return new ProcAddress(this, addr);
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
    throw new DebuggerException("Can't execute console commands");
  }

  public String getConsolePrompt() throws DebuggerException {
    return "";
  }

  public CDebugger getCDebugger() throws DebuggerException {
    return null;
  }

  /** From the SymbolLookup interface via Debugger and JVMDebugger */
  public synchronized Address lookup(String objectName, String symbol) {
    long addr = lookup0(objectName, symbol);
    if (addr == 0) {
      return null;
    }
    return new ProcAddress(this, addr);
  }

  /** From the SymbolLookup interface via Debugger and JVMDebugger */
  public synchronized OopHandle lookupOop(String objectName, String symbol) {
    long addr = lookup0(objectName, symbol);
    if (addr == 0) {
      return null;
    }
    return new ProcOopHandle(this, addr);
  }

  /** From the ProcDebugger interface */
  public MachineDescription getMachineDescription() {
    return machDesc;
  }

  /** Internal routine supporting lazy setting of MachineDescription,
      since on SPARC we will need to query the remote process to ask
      it what its data model is (32- or 64-bit).
  */

  public void setMachineDescription(MachineDescription machDesc) {
    this.machDesc = machDesc;
    setBigEndian(machDesc.isBigEndian());
    utils = new DebuggerUtilities(machDesc.getAddressSize(), machDesc.isBigEndian());
  }

  public int getRemoteProcessAddressSize() throws DebuggerException {
    if(ps_prochandle_ptr == 0L)
       throw new DebuggerException("remote process not attached yet!");

    return getRemoteProcessAddressSize0();
  }

  //--------------------------------------------------------------------------------
  // Implementation of ThreadAccess interface
  //

  /** From the ThreadAccess interface via Debugger and JVMDebugger */
  public ThreadProxy getThreadForIdentifierAddress(Address addr) {
    return threadFactory.createThreadWrapper(addr);
  }

  public ThreadProxy getThreadForThreadId(long id) {
    return threadFactory.createThreadWrapper(id);
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
    if (unalignedAccessesOkay) {
      utils.checkAlignment(address, jintSize);
    } else {
      utils.checkAlignment(address, jlongSize);
    }
    byte[] data = readBytes(address, jlongSize);
    return utils.dataToJLong(data, jlongSize);
  }

  //--------------------------------------------------------------------------------
  // Internal routines (for implementation of ProcAddress).
  // These must not be called until the MachineDescription has been set up.
  //

  /** From the ProcDebugger interface */
  public String addressValueToString(long address) {
    return utils.addressValueToString(address);
  }

  /** Need to override this to relax alignment checks on Solaris/x86. */
  public long readCInteger(long address, long numBytes, boolean isUnsigned)
    throws UnmappedAddressException, UnalignedAddressException {
    checkConfigured();
    if (!unalignedAccessesOkay) {
      utils.checkAlignment(address, numBytes);
    } else {
      // Only slightly relaxed semantics -- this is a hack, but is
      // necessary on Solaris/x86 where it seems the compiler is
      // putting some global 64-bit data on 32-bit boundaries
      if (numBytes == 8) {
        utils.checkAlignment(address, 4);
      } else {
        utils.checkAlignment(address, numBytes);
      }
    }
    byte[] data = readBytes(address, numBytes);
    return utils.dataToCInteger(data, isUnsigned);
  }

  /** From the ProcDebugger interface */
  public ProcAddress readAddress(long address)
    throws UnmappedAddressException, UnalignedAddressException {
    long value = readAddressValue(address);
    return (value == 0 ? null : new ProcAddress(this, value));
  }

  /** From the ProcDebugger interface */
  public ProcOopHandle readOopHandle(long address)
    throws UnmappedAddressException, UnalignedAddressException, NotInHeapException {
    long value = readAddressValue(address);
    return (value == 0 ? null : new ProcOopHandle(this, value));
  }

  public void writeBytesToProcess(long address, long numBytes, byte[] data)
    throws UnmappedAddressException, DebuggerException {
    // FIXME
    throw new DebuggerException("Unimplemented");
  }

  public synchronized ReadResult readBytesFromProcess(long address, long numBytes)
    throws DebuggerException {
    byte[] res = readBytesFromProcess0(address, numBytes);
    if(res != null)
       return new ReadResult(res);
    else
       return new ReadResult(address);
  }

  protected int getPageSize() {
    int pagesize = getPageSize0();
    if (pagesize == -1) {
       // return the hard coded default value.
       pagesize = (PlatformInfo.getCPU().equals("x86"))? 4096 : 8192;
    }
    return pagesize;
  }

  //--------------------------------------------------------------------------------
  // Thread context access. Can not be package private, but should
  // only be accessed by the architecture-specific subpackages.

  /** From the ProcDebugger interface. May have to redefine this later. */
  public long[] getThreadIntegerRegisterSet(int tid) {
    return (long[]) _threadIntegerRegisterSet.get(new Integer(tid));
  }

  //--------------------------------------------------------------------------------
  // Address access. Can not be package private, but should only be
  // accessed by the architecture-specific subpackages.

  /** From the ProcDebugger interface */
  public long getAddressValue(Address addr) {
    return ((ProcAddress) addr).getValue();
  }

  /** From the ProcDebugger interface */
  public Address newAddress(long value) {
    return new ProcAddress(this, value);
  }

  // native methods

  private native void attach0(String commandLine) throws DebuggerException;
  private native void detach0() throws DebuggerException;

  /** Looks up an address in the remote process's address space.
      Returns 0 if symbol not found or upon error.
  */
  private native long lookup0(String objectName, String symbol) throws DebuggerException;

  private native byte[] readBytesFromProcess0(long address, long numBytes) throws DebuggerException;
  private native int getRemoteProcessAddressSize0() throws DebuggerException;

  private native int getPageSize0() throws DebuggerException;

  private native static void initIDs() throws DebuggerException;

  private void setThreadIntegerRegisterSet(int tid, long[] regSet) {
     _threadIntegerRegisterSet.put(new Integer(tid), regSet);
  }

  private Map _threadIntegerRegisterSet = new HashMap();

  static {
     System.loadLibrary("saproc");
     initIDs();
  }

}

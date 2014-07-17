/*
 * @(#)RemoteDebuggerClient.java	1.1 02/09/12 18:17:46
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.debugger.remote;

import java.rmi.*;
import java.util.*;

import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.debugger.cdbg.*;
import sun.jvm.hotspot.debugger.remote.sparc.*;
import sun.jvm.hotspot.debugger.remote.x86.*;

/** An implementation of Debugger which wraps a
    RemoteDebugger, providing remote debugging via RMI.
    This implementation provides caching of the remote process's
    address space on the local machine where the user interface is
    running. */

public class RemoteDebuggerClient extends DebuggerBase implements JVMDebugger {
  private RemoteDebugger remoteDebugger;
  private RemoteThreadFactory threadFactory;

  private static final int cacheSize = 16 * 1024 * 1024; // 16 MB
  
  public RemoteDebuggerClient(RemoteDebugger remoteDebugger) throws DebuggerException {
    super();
    try {
      this.remoteDebugger = remoteDebugger;
      machDesc = remoteDebugger.getMachineDescription();
      utils = new DebuggerUtilities(machDesc.getAddressSize(), machDesc.isBigEndian());
      int cacheNumPages;
      int cachePageSize;
      String cpu = remoteDebugger.getCPU();
      // page size. (FIXME: should pick this up from the remoteDebugger.)
      if (cpu.equals("sparc")) {
        threadFactory = new RemoteSPARCThreadFactory(this);
        cachePageSize = 8192;
        cacheNumPages = parseCacheNumPagesProperty(cacheSize / cachePageSize);
      } else if (cpu.equals("x86")) {
        threadFactory = new RemoteX86ThreadFactory(this);
        cachePageSize = 4096;
        cacheNumPages = parseCacheNumPagesProperty(cacheSize / cachePageSize);
      } else {
        throw new RuntimeException("Thread access for CPU architecture " + cpu + " not yet supported");
      }

      // Cache portion of the remote process's address space.
      initCache(cachePageSize, cacheNumPages);

      jbooleanSize = remoteDebugger.getJBooleanSize();
      jbyteSize    = remoteDebugger.getJByteSize();
      jcharSize    = remoteDebugger.getJCharSize();
      jdoubleSize  = remoteDebugger.getJDoubleSize();
      jfloatSize   = remoteDebugger.getJFloatSize();
      jintSize     = remoteDebugger.getJIntSize();
      jlongSize    = remoteDebugger.getJLongSize();
      jshortSize   = remoteDebugger.getJShortSize();
      javaPrimitiveTypesConfigured = true;
    }
    catch (RemoteException e) {
      throw new DebuggerException(e.toString());
    }
  }

  public long[] getThreadIntegerRegisterSet(Address addr) {
    try {
      return remoteDebugger.getThreadIntegerRegisterSet(getAddressValue(addr), true);
    }
    catch (RemoteException e) {
      throw new DebuggerException(e.toString());
    }
  }

  public long[] getThreadIntegerRegisterSet(long id) {
    try {
      return remoteDebugger.getThreadIntegerRegisterSet(id, false);
    }
    catch (RemoteException e) {
      throw new DebuggerException(e.toString());
    }
  }

  /** Unimplemented in this class (remote remoteDebugger should already be attached) */
  public boolean hasProcessList() throws DebuggerException {
    throw new DebuggerException("Should not be called on RemoteDebuggerClient");
  }

  /** Unimplemented in this class (remote remoteDebugger should already be attached) */
  public List getProcessList() throws DebuggerException {
    throw new DebuggerException("Should not be called on RemoteDebuggerClient");
  }

  /** Unimplemented in this class (remote remoteDebugger should already be attached) */
  public void attach(int processID) throws DebuggerException {
    throw new DebuggerException("Should not be called on RemoteDebuggerClient");
  }

  /** Unimplemented in this class (remote remoteDebugger should already be attached) */
  public void attach(String executableName, String coreFileName) throws DebuggerException {
    throw new DebuggerException("Should not be called on RemoteDebuggerClient");
  }

  /** Unimplemented in this class (remote remoteDebugger can not be detached) */
  public boolean detach() {
    throw new DebuggerException("Should not be called on RemoteDebuggerClient");
  }

  public Address parseAddress(String addressString) throws NumberFormatException {
    long addr = utils.scanAddress(addressString);
    if (addr == 0) {
      return null;
    }
    return new RemoteAddress(this, addr);
  }

  public String getOS() throws DebuggerException {
    try {
      return remoteDebugger.getOS();
    }
    catch (RemoteException e) {
      throw new DebuggerException(e.toString());
    }
  }

  public String getCPU() {
    try {
      return remoteDebugger.getCPU();
    }
    catch (RemoteException e) {
      throw new DebuggerException(e.toString());
    }
  }

  public boolean hasConsole() throws DebuggerException {
    try {
       return remoteDebugger.hasConsole();
    } catch (RemoteException e) {
       throw new DebuggerException(e.toString());
    }
  }

  public String consoleExecuteCommand(String cmd) throws DebuggerException {
    try {
      return remoteDebugger.consoleExecuteCommand(cmd);
    }
    catch (RemoteException e) {
      throw new DebuggerException(e.toString());
    }
  }

  public String getConsolePrompt() throws DebuggerException {
    try {
      return remoteDebugger.getConsolePrompt();
    } catch (RemoteException e) {
      throw new DebuggerException(e.toString());
    }
  }

  public CDebugger getCDebugger() throws DebuggerException {
    return null;
  }

  //--------------------------------------------------------------------------------
  // Implementation of SymbolLookup interface

  public Address lookup(String objectName, String symbol) {
    try {
      long addr = remoteDebugger.lookupInProcess(objectName, symbol);
      if (addr == 0) {
        return null;
      }
      return new RemoteAddress(this, addr);
    }
    catch (RemoteException e) {
      throw new DebuggerException(e.toString());
    }
  }

  public OopHandle lookupOop(String objectName, String symbol) {
    try {
      long addr = remoteDebugger.lookupInProcess(objectName, symbol);
      if (addr == 0) {
        return null;
      }
      return new RemoteOopHandle(this, addr);
    }
    catch (RemoteException e) {
      throw new DebuggerException(e.toString());
    }
  }

  //--------------------------------------------------------------------------------
  // Implementation of JVMDebugger interface
  //

  /** Unimplemented in this class (remote remoteDebugger should already be configured) */
  public void configureJavaPrimitiveTypeSizes(long jbooleanSize,
                                              long jbyteSize,
                                              long jcharSize,
                                              long jdoubleSize,
                                              long jfloatSize,
                                              long jintSize,
                                              long jlongSize,
                                              long jshortSize) {
    throw new DebuggerException("Should not be called on RemoteDebuggerClient");
  }

  public void setMachineDescription(MachineDescription machDesc) {
    throw new DebuggerException("Should not be called on RemoteDebuggerClient");
  }
  
  public int getRemoteProcessAddressSize() {
    throw new DebuggerException("Should not be called on RemoteDebuggerClient");
  }

  public String addressValueToString(long addr) {
    return utils.addressValueToString(addr);
  }

  public long getAddressValue(Address addr) throws DebuggerException {
    return ((RemoteAddress) addr).getValue();
  }

  public Address newAddress(long value) {
    return new RemoteAddress(this, value);
  }

  RemoteAddress readAddress(long address)
    throws UnmappedAddressException, UnalignedAddressException {
    long value = readAddressValue(address);
    return (value == 0 ? null : new RemoteAddress(this, value));
  }

  RemoteOopHandle readOopHandle(long address)
    throws UnmappedAddressException, UnalignedAddressException, NotInHeapException {
    long value = readAddressValue(address);
    return (value == 0 ? null : new RemoteOopHandle(this, value));
  }

  boolean areThreadsEqual(Address addr1, Address addr2) {
    try {
       return remoteDebugger.areThreadsEqual(getAddressValue(addr1), true,
                                             getAddressValue(addr2), true);
    } catch (RemoteException e) {
    }
    return false;
  }

  boolean areThreadsEqual(long id1, long id2) {
    try {
       return remoteDebugger.areThreadsEqual(id1, false, id2, false);
    } catch (RemoteException e) {
    }
    return false;
  }

  boolean areThreadsEqual(Address addr1, long id2) {
    try {
       return remoteDebugger.areThreadsEqual(getAddressValue(addr1), true, id2, false);
    } catch (RemoteException e) {
    }
    return false;
  }

  boolean areThreadsEqual(long id1, Address addr2) {
    try {
       return remoteDebugger.areThreadsEqual(id1, false, getAddressValue(addr2), true);
    } catch (RemoteException e) {
    }
    return false;
  }

  int getThreadHashCode(Address a) {
    try {
       return remoteDebugger.getThreadHashCode(getAddressValue(a), true);
    } catch (RemoteException e) {
    }
    return a.hashCode();
  }

  int getThreadHashCode(long id) {
    try {
       return remoteDebugger.getThreadHashCode(id, false);
    } catch (RemoteException e) {
    }
    return (int) id;
  }

  public ThreadProxy getThreadForIdentifierAddress(Address addr) {
     return threadFactory.createThreadWrapper(addr);
  }

  public ThreadProxy getThreadForThreadId(long id) {
     return threadFactory.createThreadWrapper(id);
  }

  public MachineDescription getMachineDescription() throws DebuggerException {
     return machDesc;
  }

  /** This reads bytes from the remote process. */
  public ReadResult readBytesFromProcess(long address, long numBytes) {
    try {
      return remoteDebugger.readBytesFromProcess(address, numBytes);
    }
    catch (RemoteException e) {
      throw new DebuggerException(e.toString());
    }
  }

  public void writeBytesToProcess(long a, long b, byte[] c) {
     throw new DebuggerException("Unimplemented!");
  }
}

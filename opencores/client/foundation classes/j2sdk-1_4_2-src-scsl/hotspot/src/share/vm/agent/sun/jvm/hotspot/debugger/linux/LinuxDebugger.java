/*
 * "@(#)LinuxDebugger.java	1.2 03/01/23 11:29:53"
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.debugger.linux;

import sun.jvm.hotspot.debugger.*;

/** An extension of the JVMDebugger interface with a few additions to
    support 32-bit vs. 64-bit debugging as well as features required
    by the architecture-specific subpackages. */

public interface LinuxDebugger extends JVMDebugger {
  public String       addressValueToString(long address) throws DebuggerException;
  public boolean      readJBoolean(long address) throws DebuggerException;
  public byte         readJByte(long address) throws DebuggerException;
  public char         readJChar(long address) throws DebuggerException;
  public double       readJDouble(long address) throws DebuggerException;
  public float        readJFloat(long address) throws DebuggerException;
  public int          readJInt(long address) throws DebuggerException;
  public long         readJLong(long address) throws DebuggerException;
  public short        readJShort(long address) throws DebuggerException;
  public long         readCInteger(long address, long numBytes, boolean isUnsigned)
    throws DebuggerException;
  public LinuxAddress readAddress(long address) throws DebuggerException;
  public LinuxOopHandle readOopHandle(long address) throws DebuggerException;
  // On Windows the int is actually the value of a HANDLE which
  // currently must be read from the target process; that is, the
  // target process must maintain its own thread list, each element of
  // which holds a HANDLE to its underlying OS thread. FIXME: should
  // add access to the OS-level thread list, but there are too many
  // limitations imposed by Windows to usefully do so; see
  // src/os/win32/agent/README-commands.txt, command "duphandle".
  //
  // The returned array of register contents is guaranteed to be in
  // the same order as in the DbxDebugger for Solaris/x86; that is,
  // the indices match those in debugger/x86/X86ThreadContext.java.
  public long[]       getThreadIntegerRegisterSet(int threadHandleValue) throws DebuggerException;
  public long         getAddressValue(Address addr) throws DebuggerException;
  public Address      newAddress(long value) throws DebuggerException;

  // NOTE: this interface implicitly contains the following methods:
  // From the Debugger interface via JVMDebugger
  //   public void attach(int processID) throws DebuggerException;
  //   public void attach(String executableName, String coreFileName) throws DebuggerException;
  //   public boolean detach();
  //   public Address parseAddress(String addressString) throws NumberFormatException;
  //   public String getOS();
  //   public String getCPU();
  // From the SymbolLookup interface via Debugger and JVMDebugger
  //   public Address lookup(String objectName, String symbol);
  //   public OopHandle lookupOop(String objectName, String symbol);
  // From the JVMDebugger interface
  //   public void configureJavaPrimitiveTypeSizes(long jbooleanSize,
  //                                               long jbyteSize,
  //                                               long jcharSize,
  //                                               long jdoubleSize,
  //                                               long jfloatSize,
  //                                               long jintSize,
  //                                               long jlongSize,
  //                                               long jshortSize);
  // From the ThreadAccess interface via Debugger and JVMDebugger
  //   public ThreadProxy getThreadForIdentifierAddress(Address addr);
}


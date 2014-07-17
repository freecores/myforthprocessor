/*
 * @(#)Thread.java	1.12 03/01/23 11:45:56
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.runtime;

import java.util.*;
import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.types.*;

public class Thread extends VMObject {
  private static long tlabFieldOffset;
  private static AddressField activeHandlesField;
  private static AddressField highestLockField;
  private static AddressField currentPendingMonitorField;

  static {
    VM.registerVMInitializedObserver(new Observer() {
        public void update(Observable o, Object data) {
          initialize(VM.getVM().getTypeDataBase());
        }
      });
  }

  private static synchronized void initialize(TypeDataBase db) {
    Type type = db.lookupType("Thread");
    
    tlabFieldOffset    = type.getField("_tlab").getOffset();
    activeHandlesField = type.getAddressField("_active_handles");
    highestLockField   = type.getAddressField("_highest_lock");
    try {
      currentPendingMonitorField = type.getAddressField("_current_pending_monitor");
    } catch (RuntimeException t) { }
  }

  public Thread(Address addr) {
    super(addr);
  }
  
  public ThreadLocalAllocBuffer tlab() {
    return new ThreadLocalAllocBuffer(addr.addOffsetTo(tlabFieldOffset));
  }

  public JNIHandleBlock activeHandles() {
    Address a = activeHandlesField.getAddress(addr);
    if (a == null) {
      return null;
    }
    return new JNIHandleBlock(a);
  }

  public boolean   isVMThread()          { return false; }
  public boolean   isJavaThread()        { return false; }
  public boolean   isCompilerThread()    { return false; }
  public boolean   isJVMPIDaemonThread() { return false; }
  public boolean   isDebuggerThread()    { return false; }
  public boolean   isWatcherThread()     { return false; }

  /** Memory operations */
  public void oopsDo(AddressVisitor oopVisitor) {
    // FIXME: Empty for now; will later traverse JNI handles and
    // pending exception
  }

  public Address highestLock() {
    return highestLockField.getValue(addr);
  }

  public ObjectMonitor getCurrentPendingMonitor() {
    if (currentPendingMonitorField == null) {
      throw new RuntimeException("field \"_current_pending_monitor\" not found in type Thread");
    }
    Address monitorAddr = currentPendingMonitorField.getValue(addr);
    if (monitorAddr == null) {
      return null;
    }
    return new ObjectMonitor(monitorAddr);
  }

  public boolean isLockOwned(Address lock) {
    if (isInStack(lock)) return true;
    return false;
  }

  public boolean isInStack(Address a) {
    // In the Serviceability Agent we need access to the thread's
    // stack pointer to be able to answer this question. Since it is
    // only a debugging system at the moment we need access to the
    // underlying thread, which is only present for Java threads; see
    // JavaThread.java.
    return false;
  }

  /** Assistance for ObjectMonitor implementation */
  Address threadObjectAddress() { return addr; }
}

/*
 * @(#)Threads.java	1.14 03/01/23 11:46:02
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.runtime;

import java.util.*;
import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.types.*;
import sun.jvm.hotspot.runtime.solaris_sparc.SolarisSPARCJavaThreadPDAccess;
import sun.jvm.hotspot.runtime.solaris_x86.SolarisX86JavaThreadPDAccess;
import sun.jvm.hotspot.runtime.win32_x86.Win32X86JavaThreadPDAccess;
import sun.jvm.hotspot.runtime.linux_x86.LinuxX86JavaThreadPDAccess;
import sun.jvm.hotspot.utilities.*;

public class Threads {
  private static JavaThreadFactory threadFactory;
  private static AddressField      threadListField;
  private static VirtualConstructor virtualConstructor;
  private static JavaThreadPDAccess access;

  static {
    VM.registerVMInitializedObserver(new Observer() {
        public void update(Observable o, Object data) {
          initialize(VM.getVM().getTypeDataBase());
        }
      });
  }

  private static synchronized void initialize(TypeDataBase db) {
    Type type = db.lookupType("Threads");
    
    threadListField = type.getAddressField("_thread_list");
    
    // Instantiate appropriate platform-specific JavaThreadFactory
    String os  = VM.getVM().getOS();
    String cpu = VM.getVM().getCPU();

    if (os.equals("solaris") && cpu.equals("sparc")) {
	access = new SolarisSPARCJavaThreadPDAccess();
    } else if (os.equals("solaris") && cpu.equals("x86")) {
	access = new SolarisX86JavaThreadPDAccess();
    } else if (os.equals("win32") && cpu.equals("x86")) {
	access =  new Win32X86JavaThreadPDAccess();
    } else if (os.equals("linux") && cpu.equals("x86")) {
        access = new LinuxX86JavaThreadPDAccess();
    } else {
      throw new RuntimeException("OS/CPU combination " + os + "/" + cpu +
                                 " not yet supported");
    }

    virtualConstructor = new VirtualConstructor(db);
    // Add mappings for all known thread types
    virtualConstructor.addMapping("JavaThread", JavaThread.class);
    virtualConstructor.addMapping("CompilerThread", CompilerThread.class);
    virtualConstructor.addMapping("JVMPIDaemonThread", JVMPIDaemonThread.class);
    virtualConstructor.addMapping("DebuggerThread", DebuggerThread.class);
  }

  public Threads() {
  }
  
  /** NOTE: this returns objects of type JavaThread, CompilerThread,
      DebuggerThread, and JVMPIDaemonThread. The latter three are
      subclasses of the former. Most operations (fetching the top
      frame, etc.) are only allowed to be performed on a "pure"
      JavaThread. For this reason, {@link
      sun.jvm.hotspot.runtime.JavaThread#isJavaThread} has been
      changed from the definition in the VM (which returns true for
      all of these thread types) to return true for JavaThreads and
      false for the three subclasses. FIXME: should reconsider the
      inheritance hierarchy; see {@link
      sun.jvm.hotspot.runtime.JavaThread#isJavaThread}. */
  public JavaThread first() {
    Address threadAddr = threadListField.getValue();
    if (threadAddr == null) {
      return null;
    }

    return createJavaThreadWrapper(threadAddr);
  }

  /** Routine for instantiating appropriately-typed wrapper for a
      JavaThread. Currently needs to be public for OopUtilities to
      access it. */
  public JavaThread createJavaThreadWrapper(Address threadAddr) {
    try {
      JavaThread thread = (JavaThread)virtualConstructor.instantiateWrapperFor(threadAddr);
	thread.setThreadPDAccess(access);
	return thread;
    }
    catch (Exception e) {
      e.printStackTrace();
      throw new RuntimeException("Unable to deduce type of thread from address " + threadAddr +
                                 " (expected type JavaThread, CompilerThread, DebuggerThread, or JVMPIDaemonThread)");
    }
  }

  /** Memory operations */
  public void oopsDo(AddressVisitor oopVisitor) {
    // FIXME: add more of VM functionality
    for (JavaThread thread = first(); thread != null; thread = thread.next()) {
      thread.oopsDo(oopVisitor);
    }
  }

  public JavaThread owningThreadFromMonitor(ObjectMonitor monitor) {
    Address o = monitor.owner();
    if (o == null) return null;
    for (JavaThread thread = first(); thread != null; thread = thread.next()) {
      if (o.equals(thread.threadObjectAddress())) {
        return thread;
      }
    }

    long leastDiff = 0;
    boolean leastDiffInitialized = false;
    JavaThread theOwner = null;
    for (JavaThread thread = first(); thread != null; thread = thread.next()) {
      Address addr = thread.highestLock();
      if (addr == null || addr.lessThan(o)) continue;
      long diff = addr.minus(o);
      if (!leastDiffInitialized || diff < leastDiff) {
        leastDiffInitialized = true;
        leastDiff = diff;
        theOwner = thread;
      }
    }
    return theOwner;
  }

  // FIXME: add other accessors
}

/*
 * @(#)SolarisSPARCJavaThreadPDAccess.java	1.10 03/01/23 11:46:31
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.runtime.solaris_sparc;

import java.io.*;
import java.util.*;
import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.debugger.sparc.*;
import sun.jvm.hotspot.runtime.*;
import sun.jvm.hotspot.runtime.sparc.*;
import sun.jvm.hotspot.types.*;
import sun.jvm.hotspot.utilities.*;

public class SolarisSPARCJavaThreadPDAccess implements JavaThreadPDAccess {
  private static AddressField baseOfStackPointerField;
  private static AddressField postJavaStateField;
  private static AddressField osThreadField;
  private static int          isPC;
  private static int          spHasFlushed;
  private static int          pcHasFlushed;

  // Field from OSThread
  private static CIntegerField osThreadThreadIDField;

  static {
    VM.registerVMInitializedObserver(new Observer() {
        public void update(Observable o, Object data) {
          initialize(VM.getVM().getTypeDataBase());
        }
      });
  }

  private static synchronized void initialize(TypeDataBase db) {
    Type type = db.lookupType("JavaThread");
    Type anchorType = db.lookupType("JavaFrameAnchor");
    
    baseOfStackPointerField = type.getAddressField("_base_of_stack_pointer");
    postJavaStateField      = anchorType.getAddressField("_post_Java_state");
    osThreadField           = type.getAddressField("_osthread");
    isPC                    = db.lookupIntConstant("JavaFrameAnchor::is_pc").intValue();
    spHasFlushed            = db.lookupIntConstant("JavaFrameAnchor::sp_has_flushed").intValue();
    pcHasFlushed            = db.lookupIntConstant("JavaFrameAnchor::pc_has_flushed").intValue();
    
    type = db.lookupType("OSThread");
    osThreadThreadIDField   = type.getCIntegerField("_thread_id");
  }

  public    Address getLastJavaFP(Address addr) {
	return null;
    
  }

  public Address getBaseOfStackPointer(Address addr) {
    return baseOfStackPointerField.getValue(addr);
  }

  public Frame getLastFramePD(JavaThread thread, Address addr) {

    // This assert doesn't work in the debugging case for threads
    // which are running Java code and which haven't re-entered the
    // runtime (e.g., through a Method.invoke() or otherwise). They
    // haven't yet "decached" their last Java stack pointer to the
    // thread.

    //    if (Assert.ASSERTS_ENABLED) {
    //      Assert.that(hasLastJavaFrame(), "must have last_Java_sp() when suspended");
    //      // FIXME: add assertion about flushing register windows for runtime system
    //      // (not appropriate for debugging system, though, unless at safepoin t)
    //    }

    // FIXME: I don't think this is necessary, but might be useful
    // while debugging
    if (thread.getLastJavaSP() == null) {
      return null;
    }

    // postJavaState is part of a lazy window flush scheme. It flags whether the
    // value is an encoded pc, the windows have flushed, or the younger sp corresponding
    // to last_Java_SP (where we find the pc). In a relective system we'd have to
    // do something to force the thread to flush its windows and give us the younger sp.
    // In a debugger situation (process or core) the flush should have happened and
    // so if we don't have the younger sp we can find it
    //
    if (postJavaStateIsPC(addr)) {
      return new SPARCFrame(SPARCFrame.biasSP(thread.getLastJavaSP()), getPostJavaPC(addr));
    } else {
      if (getPostJavaSP(addr) == null) {
	Frame top = getCurrentFrameGuess(thread, addr);
	return new SPARCFrame(SPARCFrame.biasSP(thread.getLastJavaSP()), 
			      SPARCFrame.biasSP(SPARCFrame.findYoungerSP(top.getSP(), thread.getLastJavaSP())),
			      0, false);
      } else {
	return new SPARCFrame(SPARCFrame.biasSP(thread.getLastJavaSP()), SPARCFrame.biasSP(getPostJavaSP(addr)), 0, false);
      }
    }

    
  }

  public RegisterMap newRegisterMap(JavaThread thread, boolean updateMap) {
    return new SPARCRegisterMap(thread, updateMap);
  }

  public Frame getCurrentFrameGuess(JavaThread thread, Address addr) {
    ThreadProxy t = getThreadProxy(addr);
    SPARCThreadContext context = (SPARCThreadContext) t.getContext();
    // For now, let's see what happens if we do a similar thing to
    // what the runtime code does. I suspect this may cause us to lose
    // the top frame from the stack.
    Address sp = context.getRegisterAsAddress(SPARCThreadContext.R_SP);
    Address pc = context.getRegisterAsAddress(SPARCThreadContext.R_PC);

    if ((sp == null) || (pc == null)) {
      // Problems (have not hit this case so far, but would be bad to continue if we did)
      return null;
    }

    return new SPARCFrame(sp, pc);
  }


  public void printThreadIDOn(Address addr, PrintStream tty) {
    tty.print(getThreadProxy(addr));
  }

  public Address getLastSP(Address addr) {
    ThreadProxy t = getThreadProxy(addr);
    SPARCThreadContext context = (SPARCThreadContext) t.getContext();
    return SPARCFrame.unBiasSP(context.getRegisterAsAddress(SPARCThreadContext.R_SP));
  }

  public void printInfoOn(Address threadAddr, PrintStream tty) {
    tty.print("Thread id: ");
    printThreadIDOn(threadAddr, tty);
    tty.println("\nPostJavaState: " + getPostJavaState(threadAddr));

  }

  public ThreadProxy getThreadProxy(Address addr) {
    // Fetch the OSThread (for now and for simplicity, not making a
    // separate "OSThread" class in this package)
    Address osThreadAddr = osThreadField.getValue(addr);
    // Get the address of the thread ID from the OSThread
    Address tidAddr = osThreadAddr.addOffsetTo(osThreadThreadIDField.getOffset());

    JVMDebugger debugger = VM.getVM().getDebugger();
    return debugger.getThreadForIdentifierAddress(tidAddr);
  }

  private boolean postJavaStateIsPC(Address addr) {
    Address state = getPostJavaState(addr);
    if (state == null) {
      return false;
    }
    return (state.andWithMask(isPC) != null);
  }

  private Address getPostJavaPC(Address addr) {
    if (Assert.ASSERTS_ENABLED) {
      Assert.that(postJavaStateIsPC(addr), "must be a program counter");
    }
    long mask = isPC | pcHasFlushed;
    mask = ~mask;
    Address pc = getPostJavaState(addr);
    if (pc == null) {
      return null;
    }
    return pc.andWithMask(mask);
  }

  private Address getPostJavaState(Address addr) {
    return postJavaStateField.getValue(addr.addOffsetTo(sun.jvm.hotspot.runtime.JavaThread.getAnchorField().getOffset()));
  }

  // This returns an unbiased SP
  private Address getPostJavaSP(Address addr) {
    if (Assert.ASSERTS_ENABLED) {
      Assert.that(!postJavaStateIsPC(addr), "must be a stack pointer");
    }
    // Bug in C++ code? isPC shouldn't need to be or'ed in here.
    long mask = isPC | spHasFlushed;
    mask = ~mask;
    Address sp = getPostJavaState(addr);
    if (sp == null) {
      return null;
    }
    sp = sp.andWithMask(mask);
    if (sp.andWithMask(1) != null) {
      // value must be biased
      // handle either flavor of stack pointer: biased or unbiased
      sp = SPARCFrame.unBiasSP(sp);
      // sp = sp.addOffsetTo(VM.getVM().getStackBias());
    }
    return sp;
  }


}

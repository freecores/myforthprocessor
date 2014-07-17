/*
 * @(#)X86Frame.java	1.11 03/01/23 11:46:48
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.runtime.x86;

import java.util.*;
import sun.jvm.hotspot.asm.x86.*;
import sun.jvm.hotspot.code.*;
import sun.jvm.hotspot.compiler.*;
import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.oops.*;
import sun.jvm.hotspot.runtime.*;
import sun.jvm.hotspot.types.*;
import sun.jvm.hotspot.utilities.*;

/** Specialization of and implementation of abstract methods of the
    Frame class for the x86 family of CPUs. */

public class X86Frame extends Frame {
  private static final boolean DEBUG = false;

  // All frames
  private static final int LINK_OFFSET                =  0;
  private static final int RETURN_ADDR_OFFSET         =  1;
  private static final int SENDER_SP_OFFSET           =  2;

  // Interpreter frames
  private static final int INTERPRETER_FRAME_MIRROR_OFFSET    =  2; // for native calls only
  private static final int INTERPRETER_FRAME_SENDER_SP_OFFSET = -1;
  private static final int INTERPRETER_FRAME_METHOD_OFFSET    = INTERPRETER_FRAME_SENDER_SP_OFFSET - 1;
  private static       int INTERPRETER_FRAME_MDX_OFFSET;         // Non-core builds only
  private static       int INTERPRETER_FRAME_CACHE_OFFSET;
  private static       int INTERPRETER_FRAME_LOCALS_OFFSET;
  private static       int INTERPRETER_FRAME_BCX_OFFSET;
  private static       int INTERPRETER_FRAME_INITIAL_SP_OFFSET;
  private static       int INTERPRETER_FRAME_MONITOR_BLOCK_TOP_OFFSET;
  private static       int INTERPRETER_FRAME_MONITOR_BLOCK_BOTTOM_OFFSET;

  // Entry frames
  private static final int ENTRY_FRAME_CALL_WRAPPER_OFFSET   =  2;

  // Native frames
  private static final int NATIVE_FRAME_INITIAL_PARAM_OFFSET =  2;

  /** Size of methodOopDesc for computing BCI from BCP (FIXME: hack) */
  private static long    methodOopDescSize;

  static {
    VM.registerVMInitializedObserver(new Observer() {
        public void update(Observable o, Object data) {
          initialize(VM.getVM().getTypeDataBase());
        }
      });
  }

  private static synchronized void initialize(TypeDataBase db) {
    if (VM.getVM().isCore()) {
      INTERPRETER_FRAME_CACHE_OFFSET = INTERPRETER_FRAME_METHOD_OFFSET - 1;
    } else {
      INTERPRETER_FRAME_MDX_OFFSET   = INTERPRETER_FRAME_METHOD_OFFSET - 1;
      INTERPRETER_FRAME_CACHE_OFFSET = INTERPRETER_FRAME_MDX_OFFSET - 1;
    }
    INTERPRETER_FRAME_LOCALS_OFFSET               = INTERPRETER_FRAME_CACHE_OFFSET - 1;
    INTERPRETER_FRAME_BCX_OFFSET                  = INTERPRETER_FRAME_LOCALS_OFFSET - 1;
    INTERPRETER_FRAME_INITIAL_SP_OFFSET           = INTERPRETER_FRAME_BCX_OFFSET - 1;
    INTERPRETER_FRAME_MONITOR_BLOCK_TOP_OFFSET    = INTERPRETER_FRAME_INITIAL_SP_OFFSET;
    INTERPRETER_FRAME_MONITOR_BLOCK_BOTTOM_OFFSET = INTERPRETER_FRAME_INITIAL_SP_OFFSET;

    Type methodOopType = db.lookupType("methodOopDesc");
    // FIXME: not sure whether alignment here is correct or how to
    // force it (round up to address size?)
    methodOopDescSize = methodOopType.getSize();
  }

  // an additional field beyond sp and pc:
  Address raw_fp; // frame pointer
  
  private X86Frame() {
  }

  public X86Frame(Address raw_sp, Address raw_fp, Address pc) {
    this.raw_sp = raw_sp;
    this.raw_fp = raw_fp;
    this.pc = pc;
    if (DEBUG) {
      System.err.println("X86Frame(sp, fp, pc): " + this);
      dumpStack();
    }
  }

  public X86Frame(Address raw_sp, Address raw_fp) {
    this.raw_sp = raw_sp;
    this.raw_fp = raw_fp;
    this.pc = raw_sp.getAddressAt(-1 * VM.getVM().getAddressSize());
    // In case of native stubs, the pc retreived here might be 
    // wrong. (the _last_native_pc will have the right value)
    // So do not put add any asserts on the _pc here.
    if (DEBUG) {
      System.err.println("X86Frame(sp, fp): " + this);
      dumpStack();
    }
  }

  public Object clone() {
    X86Frame frame = new X86Frame();
    frame.raw_sp = raw_sp;
    frame.raw_fp = raw_fp;
    frame.pc = pc;
    return frame;
  }

  public boolean equals(Object arg) {
    if (arg == null) {
      return false;
    }

    if (!(arg instanceof X86Frame)) {
      return false;
    }

    X86Frame other = (X86Frame) arg;

    return (AddressOps.equal(getSP(), other.getSP()) &&
            AddressOps.equal(getFP(), other.getFP()) &&
            AddressOps.equal(getPC(), other.getPC()));
  }

  public int hashCode() {
    if (raw_sp == null) {
      return 0;
    }

    return raw_sp.hashCode();
  }

  public String toString() {
    return "sp: " + (getSP() == null? "null" : getSP().toString()) +
         ", fp: " + (getFP() == null? "null" : getFP().toString()) +
         ", pc: " + (pc == null? "null" : pc.toString());
  }

  // accessors for the instance variables
  public Address getFP() { return raw_fp; }
  public Address getSP() { return raw_sp; }
  public Address getID() { return raw_sp; }

  // FIXME: not implemented yet (should be done for Solaris/X86)
  public boolean isSignalHandlerFrameDbg() { return false; }
  public int     getSignalNumberDbg()      { return 0;     }
  public String  getSignalNameDbg()        { return null;  }

  // FIXME: do sanity checks
  public boolean isInterpretedFrameValid() {
    return true;
  }


  // FIXME: not applicable in current system
  //  void    patch_pc(Thread* thread, address pc);

  public Frame sender(RegisterMap regMap, CodeBlob cb) {
    X86RegisterMap map = (X86RegisterMap) regMap;

    if (Assert.ASSERTS_ENABLED) {
      Assert.that(map != null, "map must be set");
    }

    // Default is we done have to follow them. The sender_for_xxx will
    // update it accordingly
    map.setIncludeArgumentOops(false);

    if (isEntryFrame())       return senderForEntryFrame(map);
    if (isInterpretedFrame()) return senderForInterpreterFrame(map);
    if (VM.getVM().isClientCompiler()) {
      // Note: this test has to come before CodeCache::find_blob(pc())
      //       call since the code for osr adapter frames is contained
      //       in the code cache, too!
      if (isOSRAdapterFrame()) return senderForInterpreterFrame(map);
    }

    if (!VM.getVM().isCore()) {
      if(cb == null) {
        cb = VM.getVM().getCodeCache().findBlob(getPC());
      } else {
        if (Assert.ASSERTS_ENABLED) {
          Assert.that(cb.equals(VM.getVM().getCodeCache().findBlob(getPC())), "Must be the same");
        }
      }

      if (cb != null) {
        // Deoptimized frame?
        if (isDeoptimizedFrame()) {
          return senderForDeoptimizedFrame(map, cb);
        } else {
          // Returns adjusted pc if it was pointing into a temp. safepoint codebuffer.
          return senderForCompiledFrame(map, cb, true);
        }
      }
    }

    // Must be native-compiled frame, i.e. the marshaling code for native
    // methods that exists in the core system.
    return new X86Frame(getSenderSP(), getLink(), getSenderPC());
  }

  private Frame senderForEntryFrame(X86RegisterMap map) {
    if (Assert.ASSERTS_ENABLED) {
      Assert.that(map != null, "map must be set");
    }
    // Java frame called from C; skip all C frames and return top C
    // frame of that chunk as the sender
    X86JavaCallWrapper jcw = (X86JavaCallWrapper) getEntryFrameCallWrapper();
    if (Assert.ASSERTS_ENABLED) {
      Assert.that(!entryFrameIsFirst(), "next Java fp must be non zero");
      Assert.that(jcw.getLastJavaSP().greaterThan(getSP()), "must be above this frame on stack");  
    }
    X86Frame fr = new X86Frame(jcw.getLastJavaSP(), jcw.getLastJavaFP());
    map.clear(jcw.getNotAtCallID()); 
    if (Assert.ASSERTS_ENABLED) {
      Assert.that(map.getIncludeArgumentOops(), "should be set by clear");
    }
    return fr;
  }

  private Frame senderForInterpreterFrame(X86RegisterMap map) {
    Address sp = addressOfStackSlot(INTERPRETER_FRAME_SENDER_SP_OFFSET).getAddressAt(0);
    // We do not need to update the callee-save register mapping because above
    // us is either another interpreter frame or a converter-frame, but never
    // directly a compiled frame.
    return new X86Frame(sp, getLink(), getSenderPC());
  }

  private Frame senderForDeoptimizedFrame(X86RegisterMap map, CodeBlob cb) {
    // FIXME
    throw new RuntimeException("Deoptimized frames not handled yet");
  }

  private Frame senderForCompiledFrame(X86RegisterMap map, CodeBlob cb, boolean adjusted) {
    //
    // NOTE: some of this code is (unfortunately) duplicated in X86CurrentFrameGuess
    //

    if (Assert.ASSERTS_ENABLED) {
      Assert.that(map != null, "map must be set");
    }

    // frame owned by optimizing compiler 
    Address        sender_sp = null;

    if (VM.getVM().isClientCompiler()) {
      sender_sp        = addressOfStackSlot(SENDER_SP_OFFSET);
    } else {
      if (Assert.ASSERTS_ENABLED) {
        Assert.that(cb.getFrameSize() >= 0, "Compiled by Compiler1: do not use");
      }
      sender_sp = getSP().addOffsetTo(cb.getFrameSize());
      if( cb.isI2CAdapter() ||
          cb.isOSRMethod() ) {
        // Sender's SP is stored at the end of the frame
        sender_sp = sender_sp.getAddressAt(-1 * VM.getVM().getAddressSize()).
          addOffsetTo(VM.getVM().getAddressSize());
      }
    }

    // On Intel the return_address is always the word on the stack
    Address sender_pc = sender_sp.getAddressAt(-1 * VM.getVM().getAddressSize());

    if (map.getUpdateMap() && cb.getOopMaps() != null) {
      OopMapSet.updateRegisterMap(this, cb, map, true);
    }

    if (VM.getVM().isClientCompiler()) {
      // Move this here for C1 and collecting oops in arguments (According to Rene)
      map.setIncludeArgumentOops(cb.callerMustGCArguments(map.getThread()));
    }

    Address saved_fp = null;
    if (VM.getVM().isClientCompiler()) {
      saved_fp = getFP().getAddressAt(0);
    } else {
      int llink_offset = cb.getLinkOffset();
      if (llink_offset >= 0) {    
        // Restore base-pointer, since next frame might be an interpreter frame.    
        Address fp_addr = getSP().addOffsetTo(VM.getVM().getAddressSize() * llink_offset);
        saved_fp = fp_addr.getAddressAt(0);
      }
    }

    // Update sp (e.g. osr adapter needs to get saved_esp)
    if (cb.isOSRAdapter()) {
      // See in frame_i486.hpp the interpreter's frame layout.
      // Currently, sender's sp points just past the 'return pc' like normal.
      // We need to load up the real 'sender_sp' from the interpreter's frame.
      // This is different from normal, because the current interpreter frame
      // (which has been mangled into an OSR-adapter) pushed a bunch of space
      // on the caller's frame to make space for Java locals.  
      Address sp_addr = sender_sp.addOffsetTo(VM.getVM().getAddressSize() * 
                                              (INTERPRETER_FRAME_SENDER_SP_OFFSET - SENDER_SP_OFFSET));
      sender_sp = sp_addr.getAddressAt(0);
    }

    // FIXME: safepoint state not yet ported
    //    if (adjusted) {
    //      // Adjust the sender_pc if it points into a temporary codebuffer.      
    //      sender_pc = map->thread()->safepoint_state()->compute_adjusted_pc(sender_pc);
    //    }

    return new X86Frame(sender_sp, saved_fp, sender_pc);
  }

  protected boolean hasSenderPD() {
    // FIXME
    // Check for null ebp? Need to do some tests.
    return true;
  }

  public long frameSize() {
    return (getSenderSP().minus(getSP()) / VM.getVM().getAddressSize());
  }

  public Address getLink() {
    return addressOfStackSlot(LINK_OFFSET).getAddressAt(0);
  }

  // FIXME: not implementable yet
  //inline void      frame::set_link(intptr_t* addr)  { *(intptr_t **)addr_at(link_offset) = addr; }

  public Address getUnextendedSP() { return getSP(); }
  
  // Return address:
  public Address getSenderPCAddr() { return addressOfStackSlot(RETURN_ADDR_OFFSET); }
  public Address getSenderPC()     { return getSenderPCAddr().getAddressAt(0);      }

  // return address of param, zero origin index.
  public Address getNativeParamAddr(int idx) {
    return addressOfStackSlot(NATIVE_FRAME_INITIAL_PARAM_OFFSET + idx);
  }

  public Address getSenderSP()     { return addressOfStackSlot(SENDER_SP_OFFSET); }

  public long getOopMapOffsetAdjustmentPD() { return 0; }
  
  public Address compiledArgumentToLocationPD(VMReg reg, RegisterMap regMap, int argSize) {
    if (VM.getVM().isCore() || VM.getVM().isClientCompiler()) {
      throw new RuntimeException("Should not reach here");
    }
    
    return oopMapRegToLocation(reg, regMap);
  }

  public Address addressOfInterpreterFrameLocals() {
    return addressOfStackSlot(INTERPRETER_FRAME_LOCALS_OFFSET);
  }

  private Address addressOfInterpreterFrameBCX() {
    return addressOfStackSlot(INTERPRETER_FRAME_BCX_OFFSET);
  }

  public int getInterpreterFrameBCI() {
    // FIXME: this is not atomic with respect to GC and is unsuitable
    // for use in a non-debugging, or reflective, system. Need to
    // figure out how to express this.
    Address bcp = addressOfInterpreterFrameBCX().getAddressAt(0);
    // bcp will be null for interpreter native methods
    // in addition depending on where we catch the system the value can
    // be a bcp or a bci.
    if (bcp == null) return 0;

    OopHandle methodHandle = addressOfInterpreterFrameMethod().getOopHandleAt(0);
    Method method = (Method) VM.getVM().getObjectHeap().newOop(methodHandle);

    long bci = bcp.minus(null);
    if (bci >= 0 && bci < method.getCodeSize()) return (int) bci;
    return (int) (bcp.minus(methodHandle) - methodOopDescSize);
  }

  public Address addressOfInterpreterFrameMDX() {
    return addressOfStackSlot(INTERPRETER_FRAME_MDX_OFFSET);
  }

  // FIXME
  //inline int frame::interpreter_frame_monitor_size() {
  //  return BasicObjectLock::size();
  //}

  // expression stack
  // (the max_stack arguments are used by the GC; see class FrameClosure)

  public Address addressOfInterpreterFrameExpressionStack() {
    Address monitorEnd = interpreterFrameMonitorEnd().address();
    return monitorEnd.addOffsetTo(-1 * VM.getVM().getAddressSize());
  }

  public int getInterpreterFrameExpressionStackDirection() { return -1; }

  // top of expression stack
  public Address addressOfInterpreterFrameTOS() {
    return getSP();
  }
  
  /** Expression stack from top down */
  public Address addressOfInterpreterFrameTOSAt(int slot) {
    return addressOfInterpreterFrameTOS().addOffsetTo(slot * VM.getVM().getAddressSize());
  }
  
  public Address getInterpreterFrameSenderSP() {
    if (Assert.ASSERTS_ENABLED) {
      Assert.that(isInterpretedFrame(), "interpreted frame expected");
    }
    return addressOfStackSlot(INTERPRETER_FRAME_SENDER_SP_OFFSET).getAddressAt(0);
  }

  // Monitors
  public BasicObjectLock interpreterFrameMonitorBegin() {
    return new BasicObjectLock(addressOfStackSlot(INTERPRETER_FRAME_MONITOR_BLOCK_BOTTOM_OFFSET));
  }

  public BasicObjectLock interpreterFrameMonitorEnd() {
    Address result = addressOfStackSlot(INTERPRETER_FRAME_MONITOR_BLOCK_TOP_OFFSET).getAddressAt(0);
    if (Assert.ASSERTS_ENABLED) {
      // make sure the pointer points inside the frame
      Assert.that(AddressOps.gt(getFP(), result), "result must <  than frame pointer");
      Assert.that(AddressOps.lte(getSP(), result), "result must >= than stack pointer");
    }
    return new BasicObjectLock(result);
  }

  public int interpreterFrameMonitorSize() {
    return BasicObjectLock.size();
  }

  // Method
  public Address addressOfInterpreterFrameMethod() {
    return addressOfStackSlot(INTERPRETER_FRAME_METHOD_OFFSET);
  }

  // Constant pool cache
  public Address addressOfInterpreterFrameCPCache() {
    return addressOfStackSlot(INTERPRETER_FRAME_CACHE_OFFSET);
  }
  
  // Entry frames
  public JavaCallWrapper getEntryFrameCallWrapper() {
    return new X86JavaCallWrapper(addressOfStackSlot(ENTRY_FRAME_CALL_WRAPPER_OFFSET).getAddressAt(0));
  }

  protected Address addressOfSavedOopResult() {
    // offset is 2 for compiler2 and 3 for compiler1
    return getSP().addOffsetTo((VM.getVM().isClientCompiler() ? 2 : 3) *
                               VM.getVM().getAddressSize());
  }

  protected Address addressOfSavedReceiver() {
    return getSP().addOffsetTo(-4 * VM.getVM().getAddressSize());
  }

  private void dumpStack() {
    if (getFP() != null) {
      for (Address addr = getSP().addOffsetTo(-5 * VM.getVM().getAddressSize());
           AddressOps.lte(addr, getFP().addOffsetTo(5 * VM.getVM().getAddressSize()));
           addr = addr.addOffsetTo(VM.getVM().getAddressSize())) {
        System.out.println(addr + ": " + addr.getAddressAt(0));
      }
    } else {
      for (Address addr = getSP().addOffsetTo(-5 * VM.getVM().getAddressSize());
           AddressOps.lte(addr, getSP().addOffsetTo(20 * VM.getVM().getAddressSize()));
           addr = addr.addOffsetTo(VM.getVM().getAddressSize())) {
        System.out.println(addr + ": " + addr.getAddressAt(0));
      }
    }
  }
}

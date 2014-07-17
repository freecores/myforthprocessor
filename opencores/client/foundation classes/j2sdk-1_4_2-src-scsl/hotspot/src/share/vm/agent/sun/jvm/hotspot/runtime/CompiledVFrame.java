/*
 * @(#)CompiledVFrame.java	1.6 03/01/23 11:44:40
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.runtime;

import java.util.*;
import sun.jvm.hotspot.code.*;
import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.oops.*;
import sun.jvm.hotspot.utilities.*;

/** FIXME: missing many accessors; all we have right now is the method
    and BCI. NOTE that this has been modified from the VM's version to
    handle NULL ScopeDescs for the debugging case. This simplifies
    using code a great deal. */

public class CompiledVFrame extends JavaVFrame {
  private ScopeDesc scope;
  private boolean mayBeImprecise;

  public CompiledVFrame(Frame fr, RegisterMap regMap, JavaThread thread, ScopeDesc scope, boolean mayBeImprecise) {
    super(fr, regMap, thread);
    this.scope = scope;
    this.mayBeImprecise = mayBeImprecise;
    if (!VM.getVM().isDebugging()) {
      Assert.that(scope != null, "scope must be present");
    }
  }

  public boolean isTop() {
    if (VM.getVM().isDebugging()) {
      return (getScope() == null || getScope().isTop());
    } else {
      return getScope().isTop();
    }
  }

  public boolean isCompiledFrame() {
    return true;
  }

  public boolean mayBeImpreciseDbg() {
    return mayBeImprecise;
  }

  /** Returns the active method */
  public NMethod getCode() {
    return VM.getVM().getCodeCache().findNMethod(fr.getPC());
  }

  /** Returns the active method. Does not perform a guarantee
      regarding unloaded methods -- more suitable for debugging
      system. */
  public NMethod getCodeUnsafe() {
    return VM.getVM().getCodeCache().findNMethodUnsafe(fr.getPC());
  }

  /** Returns the ScopeDesc */
  public ScopeDesc getScope() {
    return scope;
  }

  public Method getMethod() {
    if (VM.getVM().isDebugging() && getScope() == null) {
      return getCodeUnsafe().getMethod();
    }
    return getScope().getMethod();
  }

  public StackValueCollection getLocals() {
    List scvList = getScope().getLocals();
    if (scvList == null) 
      return new StackValueCollection();

    // scvList is the list of ScopeValues describing the JVM stack state.
    // There is one scv_list entry for every JVM stack state in use.
    int length = scvList.size();
    StackValueCollection result = new StackValueCollection(length);
    for( int i = 0; i < length; i++ )
      result.add( createStackValue((ScopeValue) scvList.get(i)) );

    return result;
  }

  public StackValueCollection getExpressions() {
    List scvList = getScope().getExpressions();
    if (scvList == null) 
      return new StackValueCollection();

    // scvList is the list of ScopeValues describing the JVM stack state.
    // There is one scv_list entry for every JVM stack state in use.
    int length = scvList.size();
    StackValueCollection result = new StackValueCollection(length);
    for( int i = 0; i < length; i++ )
      result.add( createStackValue((ScopeValue) scvList.get(i)) );

    return result;
  }

  /** Returns List<MonitorInfo> */
  public List   getMonitors() {
    List monitors = getScope().getMonitors();
    if (monitors == null) {
      return new ArrayList();
    }
    List result = new ArrayList(monitors.size());
    for (int i = 0; i < monitors.size(); i++) {
      MonitorValue mv = (MonitorValue) monitors.get(i);
      StackValue ownerSV = createStackValue(mv.owner()); // it is an oop
      result.add(new MonitorInfo(ownerSV.getObject(), resolveMonitorLock(mv.basicLock())));
    }
    return result;
  }

  public int getBCI() {
    int raw = getRawBCI();
    return ((raw == DebugInformationRecorder.SYNCHRONIZATION_ENTRY_BCI) ? 0 : raw);
  }

  /** Returns SynchronizationEntryBCI or bci() (used for synchronization) */
  public int getRawBCI() {
    if (VM.getVM().isDebugging() && getScope() == null) {
      return 0; // No debugging information!
    }
    return getScope().getBCI();
  }

  public VFrame sender() {
    if (!VM.getVM().isDebugging()) {
      if (Assert.ASSERTS_ENABLED) {
        Assert.that(scope != null, "When new stub generator is in place, then scope can never be NULL");
      }
    }
    Frame f = (Frame) getFrame().clone();
    return (isTop()
              ? super.sender()
              : new CompiledVFrame(f, getRegisterMap(), getThread(), getScope().sender(), mayBeImprecise));
  }

  private StackValue createStackValue(ScopeValue sv) {
    if (sv.isLocation()) {
      // Stack or register value
      Location loc = ((LocationValue) sv).getLocation();

      loc.printOn(System.err);
      System.err.println();

      if (loc.isIllegal()) return new StackValue();

      // First find address of value
      Address valueAddr = loc.isRegister()
        // Value was in a callee-save register
        ? getRegisterMap().getLocation(new VMReg(loc.getRegisterNumber()))
        // Else value was directly saved on the stack. The frame's original stack pointer,
        // before any extension by its callee (due to Compiler1 linkage on SPARC), must be used.
        : ((Address)fr.getUnextendedSP()).addOffsetTo(loc.getStackOffset());

      // Then package it right depending on type
      if (loc.holdsFloat()) {    // Holds a float in a double register?
        // The callee has no clue whether the register holds a float,
        // double or is unused.  He always saves a double.  Here we know
        // a double was saved, but we only want a float back.  Narrow the
        // saved double to the float that the JVM wants.
        if (Assert.ASSERTS_ENABLED) {
          Assert.that( loc.isRegister(), "floats always saved to stack in 1 word" );
        }
        float value = (float) valueAddr.getJDoubleAt(0);
        return new StackValue(Float.floatToIntBits(value) & 0xFFFFFFFF); // 64-bit high half is stack junk
      } else if( loc.holdsOop() ) {  // Holds an oop?
        return new StackValue(valueAddr.getOopHandleAt(0));
      } else if( loc.holdsDouble() ) {
        // Double value in a single stack slot
        return new StackValue(valueAddr.getJIntAt(0) & 0xFFFFFFFF);
      } else if(loc.holdsAddr()) {
        if (Assert.ASSERTS_ENABLED) {
          Assert.that(!VM.getVM().isServerCompiler(), "No address type for locations with C2 (jsr-s are inlined)");
        }
        // FIXME: not yet implemented (no access to safepoint state yet)
        return new StackValue(0);
        //      intptr_t return_addr_tmp = *(intptr_t *)value_addr;
        //      int bci = -1;
        //      // Get the bci of the jsr that generated this returnAddress value.
        //      // If the destination of a jsr is a block that ends with a return or throw, then
        //      // the GraphBuilder converts the jsr into a direct goto.  This has the side effect that
        //      // the return address for the jsr gets emitted as a bci instead of as a real pc
        //      if (code()->contains((address)return_addr_tmp)) {
        //        address return_addr = thread()->safepoint_state()->compute_adjusted_pc((address)return_addr_tmp);
        //        ScopeDesc* scope = code()->scope_desc_at((address)(return_addr - jsr_call_offset), false);
        //        bci = scope->bci();
        //      } else {
        //        bci = (int)return_addr_tmp;
        //      }
        //      // no need to lock method as this happens at Safepoint
        //      assert (SafepointSynchronize::is_at_safepoint(), "must be at safepoint, otherwise lock method()");
        //      // make sure bci points to jsr
        //      Bytecode* bytecode = Bytecode_at(method()->bcp_from(bci));
        //      Bytecodes::Code bc = bytecode->code();     
        //      assert (bc == Bytecodes::_jsr || bc == Bytecodes::_jsr_w, "must be jsr");
        //      
        //      // the real returnAddress is the bytecode following the jsr
        //      return new StackValue((intptr_t)(bci + Bytecodes::length_for(bc)));
      } else if (VM.getVM().isLP64() && loc.holdsLong()) {
        if ( loc.isRegister() ) {
          // Long value in two registers, high half in the first, low in the second
          return new StackValue(((valueAddr.getJLongAt(0) & 0xFFFFFFFF) << 32) |
                                ((valueAddr.getJLongAt(8) & 0xFFFFFFFF)));
        } else {
          // Long value in a single stack slot
          return new StackValue(valueAddr.getJLongAt(0));
        }
      } else if( loc.isRegister() ) {
        // At the moment, all non-oop values in registers are 4 bytes,
        // including double and long halves (see Compile::FillLocArray() in
        // opto/output.cpp).  Haul them out as such and return a StackValue
        // containing an image of the value as it appears in a stack slot.
        // If this is a double or long half, the interpreter _must_ deal
        // with doubles and longs as entities split across two stack slots.
        // To change this so doubles and/or longs can live in one stack slot,
        // a StackValue will have to understand that it can contain an
        // undivided double or long, implying that a Location (and the debug
        // info mechanism) and FillLocArray() will also have to understand it.
        return new StackValue(valueAddr.getJIntAt(0) & 0xFFFFFFFF);
      } else {
        return new StackValue(valueAddr.getJIntAt(0) & 0xFFFFFFFF);
      }
    } else if (sv.isConstantInt()) {
      // Constant int: treat same as register int.
      return new StackValue(((ConstantIntValue) sv).getValue() & 0xFFFFFFFF);
    } else if (sv.isConstantOop()) {
      // constant oop        
      return new StackValue(((ConstantOopReadValue) sv).getValue());
    } else if (sv.isConstantDouble()) {
      // Constant double in a single stack slot
      double d = ((ConstantDoubleValue) sv).getValue();
      return new StackValue(Double.doubleToLongBits(d) & 0xFFFFFFFF);
    } else if (VM.getVM().isLP64() && sv.isConstantLong()) {
      // Constant long in a single stack slot
      return new StackValue(((ConstantLongValue) sv).getValue() & 0xFFFFFFFF);
    }

    // Unknown ScopeValue type
    Assert.that(false, "Should not reach here");
    return new StackValue(0);   // dummy  
  }

  private BasicLock resolveMonitorLock(Location location) {
    if (Assert.ASSERTS_ENABLED) {
      Assert.that(location.isStack(), "for now we only look at the stack");
    }
    int byteOffset = location.getStackOffset();
    // (stack picture)
    // high: [     ]  byte_offset + wordSize
    // low   [     ]  byte_offset
    //       
    // sp->  [     ]  0
    // the byte_offset is the distance from the stack pointer to the lowest address
    // The frame's original stack pointer, before any extension by its callee
    // (due to Compiler1 linkage on SPARC), must be used.
    return new BasicLock(getFrame().getUnextendedSP().addOffsetTo(byteOffset));
  }
}

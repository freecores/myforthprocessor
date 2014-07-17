#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)stubGenerator_ia64.cpp	1.47 03/03/31 12:16:34 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_stubGenerator_ia64.cpp.incl"

// Declaration and definition of StubGenerator (no .hpp file).
// For a more detailed description of the stub routine structure
// see the comment in stubRoutines.hpp

#define __ _masm->

// -------------------------------------------------------------------------------------------------------------------------
// Stub Code definitions

class NOT_YET_DONE {
public:
  static void throw_null_exception() { fatal("throw_null_exception"); }
};


extern "C" {
  void ia_64_verify_oop(const char* message, oop o) { 
    uintptr_t x = (uintptr_t) o;
    if ( x & 7) fatal(message);
    if (o != NULL) {
      x = (uintptr_t) o->klass();
      if ( x & 7) fatal(message);
    }
  }
}


class StubGenerator: public StubCodeGenerator {
private:

  //------------------------------------------------------------------------------------------------------------------------
  // Call stubs are used to call Java from C
  //
  // GR_I0 - call wrapper address     : address
  // GR_I1 - result                   : intptr_t*
  // GR_I2 - result type              : BasicType
  // GR_I3 - method                   : methodOop
  // GR_I4 - interpreter entry point  : address
  // GR_I5 - parameter block          : intptr_t*
  // GR_I6 - parameter count in words : int
  // GR_I7 - thread                   : Thread*
  //
  address generate_call_stub(address& return_address) {
    StubCodeMark mark(this, "StubRoutines", "call_stub");

    const Register result     = GR_I1;
    const Register type       = GR_I2;
    const Register method     = GR_I3;
    const Register entry_fd   = GR_I4;
    const Register parms      = GR_I5;
    const Register parm_count = GR_I6;
    const Register thread     = GR_I7;

    const Register parm_size = GR31_SCRATCH;
    const Register entry     = GR30_SCRATCH;
    const Register arg       = GR29_SCRATCH;

    const Register out_tos   = GR48; // Equivalent of GR_Otos
    const Register out_parms = GR49; // Equivalent of GR_Olocals (unused)

    const BranchRegister    entry_br = BR6_SCRATCH;
    const PredicateRegister no_args  = PR6_SCRATCH;

    address start = __ emit_fd();

    __ alloc(GR_Lsave_PFS, 8, 8, 2, 0);                     // save AR_PFS
    __ sxt4(parm_count, parm_count);                        // # of parms
    __ mov(GR_Lsave_SP, SP);                                // save caller's SP
    __ mov(GR_entry_frame_GR6, GR6_caller_BSP);
    __ mov(GR_entry_frame_GR7, GR7_reg_stack_limit);

    // We can not tolerate an eager RSE cpu. Itanium-1 & 2 do not support
    // this feature but we turn it off anyway 
    const Register RSC   = GR2_SCRATCH;
    __ mov(RSC, AR_RSC);
    __ and3(RSC, -4, RSC);	// Turn off two low bits
    __ mov(AR_RSC, RSC);        //  enforced lazy mode

    __ shladd(parm_size, parm_count, LogBytesPerWord, GR0); // size of stack space for the parms
    __ ld8(entry, entry_fd, sizeof(address));               // entry point
    __ mov(GR_Lsave_RP, RP);                                // save return address

    __ add(parm_size, parm_size, 15);                       // round up to multiple of 16 bytes.  we use
                                                            // caller's 16-byte scratch area for params,
                                                            // so no need to add 16 to the current frame size.
    __ mov(GR_Lsave_LC, AR_LC);                             // save AR_LC
    __ add(out_parms, SP, BytesPerWord);                    // caller's SP+8 is 1st parm addr == target method locals addr

    __ and3(parm_size, parm_size, -16);
    __ cmp4(PR0, no_args, 0, parm_count, Assembler::less);  // any parms?

    __ mov(GR_entry_frame_GR4, GR4_thread);                 // save GR4_thread: it's a preserved register
    __ sub(SP, SP, parm_size);                              // allocate the space for args + scratch
    __ mov(entry_br, entry);

    __ mov(GR27_method, method);                            // load method
    __ mov(GR4_thread, thread);                             // load thread
    __ sub(parm_count, parm_count, 1);                      // cloop counts down to zero

    // Initialize the register and memory stack limits for stack checking in compiled code
    __ add(GR7_reg_stack_limit, thread_(register_stack_limit));
    __ mov(GR6_caller_BSP, AR_BSP);                         // load register SP
    __ ld8(GR7_reg_stack_limit, GR7_reg_stack_limit);       // load register stack limit

    Label exit;

    __ mov(AR_LC, parm_count);
    __ mov(out_tos, out_parms);                             // out_tos = &out_parms[0]
    __ br(no_args, exit, Assembler::dpnt);

    // Reverse argument list and set up sender tos

    Label copy_word;
    __ bind(copy_word);

    __ ld8(arg, parms, BytesPerWord);                       // load *parms++
    __ st8(out_tos, arg, -BytesPerWord);                    // store *out_tos--
    __ cloop(copy_word, Assembler::sptk, Assembler::few);

    __ bind(exit);

    __ ld8(GP, entry_fd);
    __ mov(GR_entry_frame_TOS, out_tos);                    // so entry_frame_argument_at can find TOS

    // call interpreter frame manager

    __ call(entry_br);

    return_address = __ pc();

    // Store result depending on type.  Everything that is not
    // T_OBJECT, T_LONG, T_FLOAT, or T_DOUBLE is treated as T_INT.

    const PredicateRegister is_obj = PR6_SCRATCH;
    const PredicateRegister is_flt = PR7_SCRATCH;
    const PredicateRegister is_dbl = PR8_SCRATCH;
    const PredicateRegister is_lng = PR9_SCRATCH;

    __ cmp4(is_obj, PR0,    T_OBJECT, type, Assembler::equal);
    __ cmp4(is_flt, PR0,    T_FLOAT,  type, Assembler::equal);
    __ st4( result, GR_RET);

    __ st8( is_obj, result, GR_RET);
    __ stfs(is_flt, result, FR_RET);
    __ cmp4(is_dbl, PR0,    T_DOUBLE, type, Assembler::equal);

    __ stfd(is_dbl, result, FR_RET);
    __ cmp4(is_lng, PR0,    T_LONG,   type, Assembler::equal);
    __ mov(RP, GR_Lsave_RP);

    __ st8( is_lng, result, GR_RET);
    __ mov(GR4_thread, GR_entry_frame_GR4);

    __ mov(GR6_caller_BSP, GR_entry_frame_GR6);
    __ mov(GR7_reg_stack_limit, GR_entry_frame_GR7);
    __ mov(AR_PFS, GR_Lsave_PFS);

    __ mov(AR_LC, GR_Lsave_LC);
    __ mov(SP, GR_Lsave_SP);
    __ ret();

    return start;
  }


  //------------------------------------------------------------------------------------------------------------------------
  // Return point for a Java call if there's an exception thrown in Java code.
  // The exception is caught and transformed into a pending exception stored in
  // JavaThread that can be tested from within the VM.
  //
  address generate_catch_exception() {
    StubCodeMark mark(this, "StubRoutines", "catch_exception");

    address start = __ pc();

    // verify that thread corresponds
//  __ verify_thread();

    // set pending exception
//  __ verify_oop(GR8_exception, "generate_catch_exception");

    const Register pending_exception_addr   = GR2_SCRATCH;
    const Register exception_file_addr      = GR3_SCRATCH;
    const Register exception_line_addr      = GR31_SCRATCH;
    const Register exception_file           = GR30_SCRATCH;
    const Register exception_line           = GR29_SCRATCH;
    const Register call_stub_return_address = GR28_SCRATCH;

    const BranchRegister call_stub_return_address_br = BR6_SCRATCH;

    __ add(pending_exception_addr, thread_(pending_exception));
    __ mova(exception_file, (address)__FILE__);
    __ add(exception_file_addr, thread_(exception_file));
    __ mova(exception_line, (address)__LINE__);

    __ st8(pending_exception_addr, GR8_exception);
    __ st8(exception_file_addr, exception_file);
    __ add(exception_line_addr, thread_(exception_line));

    __ st8(exception_line_addr, exception_line);

    // complete return to VM
    assert(StubRoutines::_call_stub_return_address != NULL, "must have been generated before");

    __ mova(call_stub_return_address, StubRoutines::_call_stub_return_address);
    __ mov(call_stub_return_address_br, call_stub_return_address);
    __ br(call_stub_return_address_br);

    return start;
  }

  // JNI - call trampoline - jni function pointer in R8
  address generate_jni_trampoline() {
    StubCodeMark mark(this, "StubRoutines", "jni_trampoline");

    address start = __ pc();
    const Register save_PFS   = GR_L0;
    const Register save_RP    = GR_L1;
    const Register save_RSC   = GR_L2;
    const Register mod_RSC    = GR_L3;
    const Register save_UNAT  = GR_L4;
    // We only have 5 locals so the outs are different than standard 
    
    const Register out0   =     GR45;
    const Register out1   =     GR46;
    const Register out2   =     GR47;
    const Register out3   =     GR48;
    const Register out4   =     GR49;
    const Register out5   =     GR50;
    const Register out6   =     GR51;
    const Register out7   =     GR52;

    __ alloc(save_PFS, 8, 5, 8, 0);	// Allocate small register frame
    __ mov(save_RP, BR0);               // save return to jni transition frame
    __ mov(save_RSC, AR_RSC);           // save original value of RSC
    __ movl(mod_RSC, CONST64(0xFFFFFFFFC000FFFC));    // mask tear point to zero, rse to lazy

    // Shuffle all the ins to the outs

    __ mov(out0, GR_I0);
    __ mov(out1, GR_I1);
    __ mov(out2, GR_I2);
    __ mov(out3, GR_I3);
    __ mov(out4, GR_I4);
    __ mov(out5, GR_I5);
    __ mov(out6, GR_I6);
    __ mov(out7, GR_I7);

    __ and3(mod_RSC, mod_RSC, save_RSC);

    const Register flushed_windows_addr   = GR2_SCRATCH;
    const Register flushed                = GR3_SCRATCH;
    const Register native_method_addr     = GR_RET0;
    const Register native_method          = GR_RET1;

    __ add(flushed, GR0, 1);
    __ add(flushed_windows_addr, GR4_thread, in_bytes(JavaThread::flushed_windows_offset()));

    __ flushrs();

    // Make sure eager RSE is off while we are in native
    __ mov(AR_RSC, mod_RSC); 
    __ loadrs();					// invalidate lower frames

    // Now the caller's frame (and its call chain) have been invalidated and flushed.
    // We are now protected against Eager being enabled in native code.
    // Even if it goes eager the registers will be reloaded as clean
    // and we will invalidate after the call so no spurious flush should
    // be possible.

    __ st4(flushed_windows_addr, flushed, Assembler::ordered_release);
    __ mf();

    __ ld8(native_method, native_method_addr, in_bytes(FuncDesc::gp_offset()));
    __ ld8(GP, native_method_addr);                  // Native method GP

    // Call the native method
    const BranchRegister native_method_br = BR6_SCRATCH;
    __ mov(native_method_br, native_method);
    __ call(native_method_br);			// off we go

    const Register thread_state_addr   = GR2_SCRATCH;
    const Register thread_state        = GR3_SCRATCH;

    const Register sync_state         = GR_I0;
    const Register sync_state_addr    = sync_state;    // Address is dead after use
    const Register suspend_flags      = GR_I1;
    const Register suspend_flags_addr = suspend_flags; // Address is dead after use

    // Block, if necessary, before resuming in _thread_in_Java state.
    // In order for GC to work, don't clear the last_Java_sp until after blocking.

    // Switch thread to "native transition" state before reading the synchronization state.
    // This additional state is necessary because reading and testing the synchronization
    // state is not atomic w.r.t. GC, as this scenario demonstrates:
    //     Java thread A, in _thread_in_native state, loads _not_synchronized and is preempted.
    //     VM thread changes sync state to synchronizing and suspends threads for GC.
    //     Thread A is resumed to finish this native method, but doesn't block here since it
    //     didn't see any synchronization is progress, and escapes.

    __ add(thread_state_addr, thread_(thread_state));
    __ mov(thread_state, _thread_in_native_trans);
    __ st4(thread_state_addr, thread_state, Assembler::ordered_release);
    __ mova(sync_state_addr, SafepointSynchronize::address_of_state());

    // Fence because acquire is not ordered wrt release.
    __ mf();

    // Now invalidate the lower frames once again so that if Eager happened
    // behind our back we will still be safe and reload when we go back to
    // Java mode.

    __ mov(AR_RSC, mod_RSC); 
    __ loadrs();					// invalidate lower frames

    // Now before we return to java we must look for a current safepoint
    // (a new safepoint can not start since we entered native_trans).
    // We must check here because a current safepoint could be modifying
    // the callers registers right this moment.

    // Acquire isn't strictly necessary here because of the fence, but
    // sync_state is declared to be volatile, so we do it anyway.

    const PredicateRegister not_synced = PR6;

    __ ld4(sync_state, sync_state_addr, Assembler::acquired);
    __ ld4(suspend_flags, suspend_flags_addr, Assembler::acquired);

    Label sync_check_done;

    __ cmp(not_synced, PR0, SafepointSynchronize::_not_synchronized, sync_state, Assembler::equal, Assembler::Unc);
    if (!UseForcedSuspension || SafepointSynchronize::needs_deopt_suspend()) {
      __ cmp(not_synced, PR0, suspend_flags, GR0, Assembler::equal, Assembler::And_);
    }
    __ br(not_synced, sync_check_done);

    // Block.  We do the call directly and leave the current last_Java_frame setup undisturbed
    // We must save any possible native result acrosss the call. No oop is present

    __ mov(save_UNAT, AR_UNAT);         // save caller's UNAT

    __ sub(GR2, SP, 16);		// address of 1st spill
    __ sub(SP, SP, 16 + 32 + 32);	// 1 int register spill, 1 float spill, abi scratch area
    __ st8spill(GR2, GR_RET, -32);
    __ stfspill(GR2, FR_RET);

    __ mov(out0, GR4_thread);

    // Get target address and gp 
    address target = CAST_FROM_FN_PTR(FuncDesc*, JavaThread::check_safepoint_and_suspend_for_native_trans)->entry();
    address target_gp = CAST_FROM_FN_PTR(FuncDesc*, JavaThread::check_safepoint_and_suspend_for_native_trans)->gp();

    __ mova(GR_RET, target);
    __ mova(GP, target_gp);

    __ mov(BR6_SCRATCH, GR_RET);
    __ call(BR6_SCRATCH);

    // Restore any possible native result

    __ add(GR2, SP, 32);
    __ ldffill(FR_RET, GR2, 32);
    __ ld8fill(GR_RET, GR2);
    __ add(SP, SP, 16 + 32 + 32);       // 1 int register spill, 1 float spill, abi scratch area

    __ mov(AR_UNAT, save_UNAT);         // restore caller's UNAT

    __ bind(sync_check_done);

    // Put RSC back to original state. 
    __ mov(AR_RSC, save_RSC);

    __ mov(RP, save_RP);
    __ mov(AR_PFS, save_PFS);

    __ ret();

    return start;
  }


  //------------------------------------------------------------------------------------------------------------------------
  // Flush the register stack.
  //
  address generate_get_backing_store_pointer() {
    StubCodeMark mark(this, "StubRoutines", "get_backing_store_pointer");

    address start = __ emit_fd();

    __ mov(GR_RET, AR_BSP);
    __ ret();

    return start;
  }

  //------------------------------------------------------------------------------------------------------------------------

  //------------------------------------------------------------------------------------------------------------------------
  // Flush the register stack.
  //
  address generate_flush_register_stack() {
    StubCodeMark mark(this, "StubRoutines", "flush_register_stack");

    address start = __ emit_fd();

    const Register orig_RSC   = GR2_SCRATCH;
    const Register mod_RSC    = GR3_SCRATCH;
    __ mov(orig_RSC, AR_RSC);
    __ movl(mod_RSC, CONST64(0xFFFFFFFFC000FFFC));    // mask tear point to zero, rse to lazy

    __ flushrs();

    __ and3(mod_RSC, mod_RSC, orig_RSC);
    __ mov(AR_RSC, mod_RSC); 
    __ loadrs();		// Invalidate lower frames
    __ mov(AR_RSC, orig_RSC);   // restore tear point to original

    __ ret();

    return start;
  }

  //------------------------------------------------------------------------------------------------------------------------
  // Continuation point for runtime calls returning with a pending exception.
  // The pending exception check happened in the runtime or native call stub.
  // The pending exception in Thread is converted into a Java-level exception.
  //
  // Contract with Java-level exception handlers:
  //
  address generate_forward_exception() {
    StubCodeMark mark(this, "StubRoutines", "forward exception");

    address start = __ pc();

    // Upon entry, GR_Lsave_RP has the return address returning into Java
    // compiled code; i.e. the return address becomes the throwing pc.

    const Register pending_exception_addr = GR31_SCRATCH;
    const Register handler                = GR30_SCRATCH;

    const PredicateRegister is_not_null   = PR15_SCRATCH;
    const BranchRegister    handler_br    = BR6_SCRATCH;

    // Allocate abi scratch, since the compiler didn't allocate a memory frame.
    // pop_dummy_thin_frame will restore the caller's SP.
    __ sub(SP, SP, 16);

#ifdef ASSERT
    // Get pending exception oop.
    __ add(pending_exception_addr, thread_(pending_exception));
    __ ld8(GR8_exception, pending_exception_addr);

    // Make sure that this code is only executed if there is a pending exception.
    {
      Label not_null;
      __ cmp(is_not_null, PR0, 0, GR8_exception, Assembler::notEqual);
      __ br(is_not_null, not_null);
      __ stop("StubRoutines::forward exception: no pending exception (1)");
      __ bind(not_null);
    }

//  __ verify_oop(GR8_exception, "generate_forward_exception");
#endif

    // Find exception handler
    __ call_VM_leaf(CAST_FROM_FN_PTR(address, SharedRuntime::exception_handler_for_return_address), GR_Lsave_RP);

    __ mov(handler, GR_RET);

    // Load pending exception oop.
    __ add(pending_exception_addr, thread_(pending_exception));
    __ ld8(GR8_exception, pending_exception_addr);

    // The exception pc is the return address in the caller.
    __ mov(GR9_issuing_pc, GR_Lsave_RP);

    // Uses GR2, BR6
    __ pop_dummy_thin_frame();
    // Now in caller of native/stub register frame

#ifdef ASSERT
    // make sure exception is set
    {
      Label not_null;
      __ cmp(is_not_null, PR0, 0, GR8_exception, Assembler::notEqual);
      __ br(is_not_null, not_null);
      __ stop("StubRoutines::forward exception: no pending exception (2)");
      __ bind(not_null);
    }
#endif
    // clear pending exception
    __ st8(pending_exception_addr, GR0);

    // jump to exception handler
    __ mov(handler_br, handler);
    __ br(handler_br);

    return start;
  }


  //----------------------------------------------------------------------------------------------------
  // Support for jint atomic::exchange(jint exchange_value, jint* dest)
  // 
  // Arguments:
  //
  //     exchange_value - GR_I0
  //     dest           - GR_I1
  //
  // Results:
  //
  //     GR_RET - the value previously stored in dest
  //
  address generate_atomic_exchange() {
    StubCodeMark mark(this, "StubRoutines", "atomic_exchange");

    const Register exchange_value = GR_I0;
    const Register dest           = GR_I1;

    address start = __ emit_fd();

    __ mf();

    __ xchg4(GR_RET, dest, exchange_value);
    __ sxt4(GR_RET, GR_RET);

    __ ret();

    return start;
  }

  //----------------------------------------------------------------------------------------------------
  // Support for intptr_t atomic::exchange_ptr(intptr_t exchange_value, intptr_t* dest).
  //
  // Arguments:
  //
  //     exchange_value - GR_I0
  //     dest           - GR_I1
  //
  // Results:
  //
  //     GR_RET - the value previously stored in dest
  //
  address generate_atomic_exchange_ptr() {
    StubCodeMark mark(this, "StubRoutines", "atomic_exchange_ptr");

    const Register exchange_value = GR_I0;
    const Register dest           = GR_I1;

    address start = __ emit_fd();

    __ mf();

    __ xchg8(GR_RET, dest, exchange_value);

    __ ret();

    return start;
  }


  //----------------------------------------------------------------------------------------------------
  // Support for jint atomic::compare_and_exchange(jint exchange_value, jint* dest, jint compare_value)
  // 
  // Arguments:
  //
  //     exchange_value - GR_I0
  //     dest           - GR_I1
  //     compare_value  - GR_I2
  //
  // Results:
  //
  //     GR_RET - the value previously stored in dest
  //
  address generate_atomic_compare_and_exchange() {
    StubCodeMark mark(this, "StubRoutines", "atomic_compare_and_exchange");

    const Register exchange_value = GR_I0;
    const Register dest           = GR_I1;
    const Register compare_value  = GR_I2;

    address start = __ emit_fd();

    __ mf();

    __ zxt4(compare_value, compare_value);
    __ mov(AR_CCV, compare_value);
    __ cmpxchg4(GR_RET, dest, exchange_value, Assembler::acquire);
    __ sxt4(GR_RET, GR_RET);

    __ ret();

    return start;
  }


  // Support for intptr_t atomic::compare_and_exchange_ptr(intptr_t exchange_value, intptr_t* dest, intptr_t compare_value)
  //
  // Arguments:
  //
  //     exchange_value - GR_I0
  //     dest           - GR_I1
  //     compare_value  - GR_I2
  //
  // Results:
  //
  //     GR_RET - the value previously stored in dest
  //
  address generate_atomic_compare_and_exchange_ptr() { 
    StubCodeMark mark(this, "StubRoutines", "atomic_compare_and_exchange_ptr");

    const Register exchange_value = GR_I0;
    const Register dest           = GR_I1;
    const Register compare_value  = GR_I2;

    address start = __ emit_fd();

    __ mf();

    __ mov(AR_CCV, compare_value);
    __ cmpxchg8(GR_RET, dest, exchange_value, Assembler::acquire);

    __ ret();

    return start;
  }


  //----------------------------------------------------------------------------------------------------
  // Support for jint atomic::increment(jint inc, jint* dest).
  //
  // Arguments:
  //
  //     inc  - GR_I0 (e.g., +1 or -1)
  //     dest - GR_I1
  //
  // Results:
  //
  //     GR_RET - the new value stored in dest
  //
  //
  address generate_atomic_increment() {
    StubCodeMark mark(this, "StubRoutines", "atomic_increment");

    const Register inc  = GR_I0;
    const Register dest = GR_I1;

    address start = __ emit_fd();

    __ mf();

    // increment or decrement
    __ cmp4(PR6, PR7, 1, inc, Assembler::equal);

    __ fetchadd4(PR6, GR_RET, dest,  1, Assembler::acquire);
    __ fetchadd4(PR7, GR_RET, dest, -1, Assembler::acquire);

    // GR_RET contains result of the fetch, not the add
    __ sxt4(GR_RET, GR_RET);
    __ adds(PR6, GR_RET,  1, GR_RET);
    __ adds(PR7, GR_RET, -1, GR_RET);

    __ ret();

    return start;
  }


  //----------------------------------------------------------------------------------------------------
  // Support for intptr_t atomic::increment_ptr(intptr_t inc, intptr_t* dest).
  //
  // Arguments:
  //
  //     inc  - GR_I0 (e.g., +1 or -1)
  //     dest - GR_I1
  //
  // Results:
  //
  //     GR_RET - the new value stored in dest
  //
  address generate_atomic_increment_ptr() {
    StubCodeMark mark(this, "StubRoutines", "atomic_increment_ptr");

    const Register inc  = GR_I0;
    const Register dest = GR_I1;

    address start = __ emit_fd();

    __ mf();

    // increment or decrement
    __ cmp(PR6, PR7, 1, inc, Assembler::equal);

    __ fetchadd8(PR6, GR_RET, dest,  1, Assembler::acquire);
    __ fetchadd8(PR7, GR_RET, dest, -1, Assembler::acquire);

    // GR_RET contains result of the fetch, not the add
    __ adds(PR6, GR_RET,  1, GR_RET);
    __ adds(PR7, GR_RET, -1, GR_RET);

    __ ret();

    return start;
  }


  //----------------------------------------------------------------------------------------------------
  // Support for void atomic::membar().
  //
  address generate_atomic_membar() {
    StubCodeMark mark(this, "StubRoutines", "atomic_membar");

    address start = __ emit_fd();

    // severe overkill
    __ mf();
    __ ret();

    return start;
  }


  //----------------------------------------------------------------------------------------------------
  // Non-destructive plausibility checks for oops
  //  
  // Arguments:
  //     GR_I0 - oop to verify
  //
  address generate_verify_oop() {
    StubCodeMark mark(this, "StubRoutines", "verify_oop");

    address start = CAST_FROM_FN_PTR(address, ia_64_verify_oop);

    return start;
  }


  //----------------------------------------------------------------------------------------------------
  // The following routine generates a subroutine to throw a null pointer exception.
  // 
  // Arguments:
  //
  // Results:
  //
  //  throws a new exception object to the given return address
  //
  // catch exception if receiver is NULL
  //
  address generate_throw_null_exception() {
    StubCodeMark mark(this, "StubRoutines", "throw_null_exception");

    // not used
    address start = CAST_FROM_FN_PTR(address, NOT_YET_DONE::throw_null_exception);

    return start;
  }


  //-----------------------------------------------------------------
  // Called when an interpreter call needs an i2c adapter before
  // continuing in the compiled callee. Thi sis typically called from a vtable
  //
  // Arguments:
  //      methodOop - GR_I0
  // 
#ifdef COMPILER2
  address generate_lazy_i2c_adapter_entry() {
    StubCodeMark mark(this, "StubRoutines", "lazy_i2c_adapter_entry");

    address start = __ pc();

    return start;
  }
#endif


  //------------------------------------------------------------------------------------------------------------------------
  // Handler for implicit exceptions forwarded by the OS
  //
  // How it works: Implicit exceptions are forwarded by the OS to this
  // handler, as if by a simple branch.  All registers remain intact.
  // The trap handler sets JavaThread::saved_exception_pc() to show
  // where the trap came from.
  //
  // Note 1: This solution automatically handles cases where the underlying
  //         nmethod (if any) is deoptimized, since VM calls must be able to
  //         handle such a situation anyway.
  //
  // Results:
  //     initiates exception processing; does not return

  address generate_handler_for_implicit_exception(address throw_interpreted,
						  address throw_compiled,
						  address* filter,
						  bool is_implicit_null) {
    StubCodeMark mark(this, "StubRoutines", "handler_for_implicit_exception");

    address start = __ pc();

    const PredicateRegister is_non_interpreter = PR15_SCRATCH;
    const PredicateRegister is_caller          = PR15_SCRATCH;

    const BranchRegister    return_address_br  = BR7_SCRATCH;

    Label non_interpreter;
    Label use_caller_return_address;

    // 4826555: nsk test stack016 fails.  See os_linux_ia64.cpp.
    // Reload register stack limit because the Linux kernel
    // doesn't reload GR4-7 from the ucontext.
    __ add(GR7_reg_stack_limit, thread_(register_stack_limit));
    __ ld8(GR7_reg_stack_limit, GR7_reg_stack_limit);

    __ push_full_frame();
    __ push_scratch_regs();
//  __ verify_thread();

    // Check for interpreter case: at the moment, the C++ interpreter does no implicit
    // exception checking, so this code should never trigger (i.e., throw_interpreted == NULL).

    __ call_VM_leaf(CAST_FROM_FN_PTR(address, SharedRuntime::is_interpreter_exception));
    __ cmp(is_non_interpreter, PR0, 0, GR_RET, Assembler::equal);
    __ br(is_non_interpreter, non_interpreter);

    __ pop_scratch_regs();
    __ pop_full_frame();
    __ br(throw_interpreted);

    __ bind(non_interpreter);

    // Compiled code case - both implicit and inline-cache/vtable-stub
    // exception happen outside of interpreter => compute exception
    // return address, if any.  Must not blow any registers (except that
    // we have to blow a branch register in order to get to the handler);
    // the implicit exception acts as if it took a long-latency branch to
    // the final target.

    if (is_implicit_null) {
      __ mov(GR_O0, 1);
    } else {
      __ clr(GR_O0);
    }
    __ call_VM_leaf(CAST_FROM_FN_PTR(address, SharedRuntime::compute_exception_return_address), GR_O0);

    __ mov(return_address_br, GR_RET);
    __ cmp(is_caller, PR0, 0, GR_RET, Assembler::equal);
    __ br(is_caller, use_caller_return_address);

//  __ verify_thread();
    __ pop_scratch_regs();
    __ pop_full_frame();
    __ br(return_address_br);

    __ bind(use_caller_return_address);

    // vtable-stub or inline cache.

    // An implicit exception in compiled code may happen from the
    // vtable stub or inline cache check.  We handle it similiarly to
    // handle_wrong_method, except that we throw an exception.
    // The runtime code must verify that the exception happened in the stubs.

    if (throw_compiled == NULL) {
      // running with -Xint
      __ stop("wrong implicit exception");
    } else {
      // The exception PC which is the frame above vtable or unverified
      // entry point has been saved in the context save area
      // by the trap handler just before calling into the stub.
      __ pop_scratch_regs();
      __ pop_full_frame();
      __ br(throw_compiled);
    }

    return start;
  }


  // Support for uint StubRoutine::ia64::partial_subtype_check( Klass sub, Klass super );
  // Arguments :
  //      ret  : GR_RET, returned
  //      sub  : I0, argument
  //      super: I1, argument
  //
  address generate_partial_subtype_check() {
    StubCodeMark mark(this, "StubRoutines", "partial_subtype_check");
    address start = __ pc();

    Label loop, missed;

    const Register subklass   = GR_I0; // subklass
    const Register superklass = GR_I1; // superklass

    const Register length     = GR_L0; // cache array length
    const Register index      = GR_L1; // index into cache array
    const Register value      = GR_L2; // current value from cache array
    const Register save_PFS   = GR_L3;

    const PredicateRegister miss = PR6_SCRATCH;

    // Allocate a small frame for a leaf routine
    __ alloc(save_PFS, 8, 4, 0, 0);

    // Set up the input and local registers

    int source_offset = Klass::secondary_supers_offset_in_bytes();
    int target_offset = Klass::secondary_super_cache_offset_in_bytes();

    int length_offset = arrayOopDesc::length_offset_in_bytes();
    int base_offset   = arrayOopDesc::base_offset_in_bytes(T_OBJECT);

    __ add(subklass, sizeof(oopDesc) + source_offset, subklass);

    __ ld8(value, subklass, target_offset - source_offset);

    // Point to the length
    __ add(value, length_offset, value);

    // Load the length, set the pointer to the base, and clear the index
    __ ld2(length, value, base_offset - length_offset);
    __ clr(index);

    // Load the next pointer (which can run 1 past the end)
    // Exit the loop if the count is reached
    __ bind(loop);

    __ ld8(GR_RET, value, BytesPerWord);
    __ cmp(miss, PR0, index, length, Assembler::equal); 
    __ br(miss, missed, Assembler::spnt);

    // Increment the loop counter
    // Exit if this is a match
    __ cmp(miss, PR0, GR_RET, superklass, Assembler::notEqual);
                                  // Check for match
    __ add(index, 1, index);	  // Bump index
    __ br(miss, loop, Assembler::sptk);

    // Got a hit; return success (zero result); set cache.
    // Cache load doesn't happen here; for speed it is directly emitted by the compiler.
    __ st8(subklass, superklass); // Save result to cache
    __ mov(AR_PFS, save_PFS);
    __ clr(GR_RET);               // Set zero result
    __ ret();			  // Result in GR_RET is ok

    // Got a miss, return non-zero result
    __ bind(missed);

    __ mov(AR_PFS, save_PFS);
    __ mov(GR_RET, 1);		  // Set non-zero result
    __ ret();			  // Result in GR_RET is ok

    return start;
  }


  //---------------------------------------------------------------------------
  // Initialization
  
  void generate_initial() {
    // Generates all stubs and initializes the entry points

    //------------------------------------------------------------------------------------------------------------------------
    // entry points that exist in all platforms
    // Note: This is code that could be shared among different platforms - however the benefit seems to be smaller than
    //       the disadvantage of having a much more complicated generator structure. See also comment in stubRoutines.hpp.

    StubRoutines::_forward_exception_entry                = generate_forward_exception();    
    StubRoutines::_throw_null_exception_entry             = generate_throw_null_exception();
    StubRoutines::_call_stub_entry                        = generate_call_stub(StubRoutines::_call_stub_return_address);
    StubRoutines::_catch_exception_entry                  = generate_catch_exception();    

    COMPILER2_ONLY(StubRoutines::_lazy_i2c_adapter_entry  = generate_lazy_i2c_adapter_entry();)

    StubRoutines::_atomic_exchange_ptr_entry              = generate_atomic_exchange_ptr();
    StubRoutines::_atomic_compare_and_exchange_ptr_entry  = generate_atomic_compare_and_exchange_ptr();
    StubRoutines::_atomic_compare_and_exchange_long_entry = StubRoutines::_atomic_compare_and_exchange_ptr_entry;
    StubRoutines::_atomic_increment_ptr_entry             = generate_atomic_increment_ptr();

    StubRoutines::_atomic_exchange_entry                  = generate_atomic_exchange();
    StubRoutines::_atomic_compare_and_exchange_entry      = generate_atomic_compare_and_exchange();
    StubRoutines::_atomic_increment_entry                 = generate_atomic_increment();
    StubRoutines::_atomic_membar_entry                    = generate_atomic_membar();

    StubRoutines::ia64::_partial_subtype_check            = generate_partial_subtype_check();
    StubRoutines::ia64::_flush_register_stack_entry       = generate_flush_register_stack();
    StubRoutines::ia64::_jni_call_trampoline              = generate_jni_trampoline();
    StubRoutines::ia64::_get_backing_store_pointer        = generate_get_backing_store_pointer();
  }


  void generate_all() {
    // Generates all stubs and initializes the entry points
    
    //------------------------------------------------------------------------------------------------------------------------
    // entry points that are platform specific  

    StubRoutines::ia64::_handler_for_divide_by_zero_entry  =
      generate_handler_for_implicit_exception(
        NULL,
        NOT_COMPILER2(NULL) COMPILER2_ONLY(OptoRuntime::throw_div0_exception_stub()),
        &SharedRuntime::_implicit_div0_exception_filter,
        false
      );

    StubRoutines::ia64::_handler_for_null_exception_entry  =
      generate_handler_for_implicit_exception(
        NULL,
        NOT_COMPILER2(NULL) COMPILER2_ONLY(OptoRuntime::throw_null_exception_stub()),
        &SharedRuntime::_implicit_null_exception_filter,
        true
      );

    StubRoutines::ia64::_handler_for_stack_overflow_entry  =
      generate_handler_for_implicit_exception(
        NULL,
        NOT_COMPILER2(NULL) COMPILER2_ONLY(OptoRuntime::throw_stack_overflow_error_stub()),
        &SharedRuntime::_stack_ovrflow_exception_filter,
        false
      );
  }


public:
  StubGenerator(CodeBuffer* code, bool all) : StubCodeGenerator(code) { 
    if (all) {
      generate_all();
    } else {
      generate_initial();
    }
   __ flush_bundle();
   __ flush();
  }

}; // StubGenerator


void StubGenerator_generate(CodeBuffer* code, bool all) {
  StubGenerator g(code, all);
}

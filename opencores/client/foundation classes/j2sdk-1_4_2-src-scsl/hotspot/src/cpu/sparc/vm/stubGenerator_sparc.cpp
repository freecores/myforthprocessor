#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)stubGenerator_sparc.cpp	1.191 03/01/23 11:02:20 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_stubGenerator_sparc.cpp.incl"

// Declaration and definition of StubGenerator (no .hpp file).
// For a more detailed description of the stub routine structure
// see the comment in stubRoutines.hpp.

#define __ _masm->


// Note:  The register L7 is used as L7_thread_cache, and may not be used
//        any other way within this module.


static const Register& Lstub_temp = L2;

// -------------------------------------------------------------------------------------------------------------------------
// Stub Code definitions

static address handle_unsafe_access() {
  JavaThread* thread = JavaThread::current();
  address pc  = thread->saved_exception_pc();
  address npc = thread->saved_exception_npc();
  // pc is the instruction which we must emulate
  // doing a no-op is fine:  return garbage from the load

  // request an async exception
  thread->set_pending_unsafe_access_error();

  // return address of next instruction to execute
  return npc;
}

class StubGenerator: public StubCodeGenerator {
 private:

  // Load Oissuing_pc from the given register.
  void load_issuing_pc_from_return_addr(Register return_addr_reg) {
    const int pc_adj = frame::pc_return_offset;

    __ add(return_addr_reg, pc_adj, Oissuing_pc);
  }


  //----------------------------------------------------------------------------------------------------
  // Call stubs are used to call Java from C

  address StubGenerator::generate_call_stub(address& return_pc) {
    StubCodeMark mark(this, "StubRoutines", "call_stub");
    address start = __ pc();

    // Incoming arguments:
    //
    // o0         : call wrapper address
    // o1         : result (address)
    // o2         : result type
    // o3         : method
    // o4         : (interpreter) entry point
    // o5         : parameters (address)
    // [sp + 0x5c]: parameter size (in words)
    // [sp + 0x60]: thread
    //
    // +---------------+ <--- sp + 0
    // |               |
    // . reg save area .
    // |               |
    // +---------------+ <--- sp + 0x40
    // |               |
    // . extra 7 slots .
    // |               |
    // +---------------+ <--- sp + 0x5c
    // |  param. size  |
    // +---------------+ <--- sp + 0x60
    // |    thread     |
    // +---------------+
    // |               |

    // note: if the link argument position changes, adjust
    //       the code in frame::entry_frame_call_wrapper()

    const Argument link           = Argument(0, false); // used only for GC
    const Argument result         = Argument(1, false);
    const Argument result_type    = Argument(2, false);
    const Argument method         = Argument(3, false);
    const Argument entry_point    = Argument(4, false);
    const Argument parameters     = Argument(5, false);
    const Argument parameter_size = Argument(6, false);
    const Argument thread         = Argument(7, false);

    // setup thread register
    __ ld_ptr(thread.as_address(), G2_thread);

#ifdef ASSERT
    // make sure we have no pending exceptions
    { const Register t = G3_scratch;
      Label L;
      __ ld_ptr(G2_thread, in_bytes(Thread::pending_exception_offset()), t);
      __ br_null(t, false, Assembler::pt, L);
      __ delayed()->nop();
      __ stop("StubRoutines::call_stub: entered with pending exception");
      __ bind(L);
    }
#endif

    // create activation frame & allocate space for parameters
    { const Register t = G3_scratch;
      __ ld_ptr(parameter_size.as_address(), t);                // get parameter size (in words)
      __ add(t, frame::memory_parameter_word_sp_offset, t);     // add space for save area (in words)
      __ round_to(t, WordsPerLong);                             // make sure it is multiple of 2 (in words)
      __ sll(t, LogBytesPerWord, t);                            // compute number of bytes
      __ neg(t);                                                // negate so it can be used with save
      __ save(SP, t, SP);                                       // setup new frame
    }

    // +---------------+ <--- sp + 0
    // |               |
    // . reg save area .
    // |               |
    // +---------------+ <--- sp + 0x40
    // |               |
    // . extra 7 slots .
    // |               |
    // +---------------+ <--- sp + 0x5c
    // |  empty slot   |      (only if parameter size is even)
    // +---------------+
    // |               |
    // .  parameters   .
    // |               |
    // +---------------+ <--- fp + 0
    // |               |
    // . reg save area .
    // |               |
    // +---------------+ <--- fp + 0x40
    // |               |
    // . extra 7 slots .
    // |               |
    // +---------------+ <--- fp + 0x5c
    // |  param. size  |
    // +---------------+ <--- fp + 0x60
    // |    thread     |
    // +---------------+
    // |               |

    // pass parameters if any
    { const Register src = parameters.as_in().as_register();
      const Register dst = Lesp;
      const Register tmp = G3_scratch;
      const Register cnt = G4_scratch;

      // test if any parameters & setup of Lesp
      Label exit;
      __ ld_ptr(parameter_size.as_in().as_address(), cnt);      // parameter counter
      __ add( FP, STACK_BIAS, dst );
      __ tst(cnt);
      __ br(Assembler::zero, false, Assembler::pn, exit);
      __ delayed()->sub(dst, BytesPerWord, dst);                 // setup Lesp

      // copy parameters if any
      Label loop;
      __ bind(loop);
      __ ld_ptr(src, 0, tmp);
      __ add(src, BytesPerWord, src);
      __ st_ptr(tmp, dst, 0);
      __ deccc(cnt);
      __ br(Assembler::greater, false, Assembler::pt, loop);
      __ delayed()->sub(dst, BytesPerWord, dst);

      // done
      __ bind(exit);
    }

    // setup parameters, method & call Java function
#ifdef ASSERT
    const Register saved_SP = Lscratch;
    if (!UseC2CallingConventions) {
      __ mov(SP, saved_SP);                                     // keep track of SP before call
    }
#endif
    // setup parameters
    if (!UseC2CallingConventions) {
      const Register t = G3_scratch;
      __ ld_ptr(parameter_size.as_in().as_address(), t);            // get parameter size (in words)
      __ sll(t, LogBytesPerWord, t);                            // compute number of bytes
      __ sub(FP, t, Gargs);                                     // setup parameter pointer
#ifdef _LP64
      __ add( Gargs, STACK_BIAS, Gargs );			// Account for LP64 stack bias
#endif
    }
    // do the call
    //
    // the following register must be setup:
    //
    // G2_thread
    // G5_method
    // Gargs
    __ jmpl(entry_point.as_in().as_register(), G0, O7);
    __ delayed()->mov(method.as_in().as_register(), G5_method);   // setup method
    return_pc = __ pc();

#ifdef ASSERT
    // make sure callee didn't change SP
    // note: strictly speaking, this doesn't matter for java calls, because
    //       we return anyway - however, it is an indication that something
    //       goes wrong elsewhere
    if (!UseC2CallingConventions) {
      Label L;
      __ cmp(SP, saved_SP);
      __ br(Assembler::equal, false, Assembler::pt, L);
      __ delayed()->nop();
      __ stop("StubRoutines::call_stub: callee changed SP");
      __ bind(L);
    }
#endif

    // store result depending on type
    // (everything that is not T_OBJECT, T_LONG, T_FLOAT, or T_DOUBLE 
    //  is treated as T_INT)
    { const Register addr = result     .as_in().as_register();
      const Register type = result_type.as_in().as_register();
      Label is_long, is_float, is_double, is_object, exit;
      __            cmp(type, T_OBJECT);  __ br(Assembler::equal, false, Assembler::pn, is_object);
      __ delayed()->cmp(type, T_FLOAT);   __ br(Assembler::equal, false, Assembler::pn, is_float);
      __ delayed()->cmp(type, T_DOUBLE);  __ br(Assembler::equal, false, Assembler::pn, is_double);
      __ delayed()->cmp(type, T_LONG);    __ br(Assembler::equal, false, Assembler::pn, is_long);
      __ delayed()->nop();

      // store int result 
      __ st(O0, addr, G0);

      __ bind(exit);
      __ ret();
      __ delayed()->restore();
  
      __ bind(is_object);
      __ ba(false, exit);
      __ delayed()->st_ptr(O0, addr, G0);
  
      __ bind(is_float);
      __ ba(false, exit);
      __ delayed()->stf(FloatRegisterImpl::S, F0, addr, G0);
  
      __ bind(is_double);
      __ ba(false, exit);
      __ delayed()->stf(FloatRegisterImpl::D, F0, addr, G0);
  
      __ bind(is_long);
#ifdef _LP64
      __ ba(false, exit);
      __ delayed()->st_long(O0, addr, G0);	// store entire long
#else
      __ st(O1, addr, BytesPerInt); 		
      __ ba(false, exit);
      __ delayed()->st(O0, addr, G0); 		
#endif
     }
     return start;
  }


  //----------------------------------------------------------------------------------------------------
  // Return point for a Java call if there's an exception thrown in Java code.
  // The exception is caught and transformed into a pending exception stored in
  // JavaThread that can be tested from within the VM.
  //
  // Oexception: exception oop

  address StubGenerator::generate_catch_exception() {
    StubCodeMark mark(this, "StubRoutines", "catch_exception");

    address start = __ pc();
    // verify that thread corresponds
    __ verify_thread();

    const Register& temp_reg = Gtemp;
    Address pending_exception_addr    (G2_thread, 0, in_bytes(Thread::pending_exception_offset()));
    Address exception_file_offset_addr(G2_thread, 0, in_bytes(Thread::exception_file_offset   ()));
    Address exception_line_offset_addr(G2_thread, 0, in_bytes(Thread::exception_line_offset   ()));

    // set pending exception
    __ verify_oop(Oexception, "generate_catch_exception");
    __ st_ptr(Oexception, pending_exception_addr);
    __ set((intptr_t)__FILE__, temp_reg);
    __ st_ptr(temp_reg, exception_file_offset_addr);
    __ set((intptr_t)__LINE__, temp_reg);
    __ st(temp_reg, exception_line_offset_addr);

    // complete return to VM
    assert(StubRoutines::_call_stub_return_address != NULL, "must have been generated before");

    __ jump_to(Address(temp_reg, StubRoutines::_call_stub_return_address));
    __ delayed()->nop();

    return start;
  }


  //----------------------------------------------------------------------------------------------------
  // Continuation point for runtime calls returning with a pending exception
  // The pending exception check happened in the runtime or native call stub
  // The pending exception in Thread is converted into a Java-level exception
  //
  // Contract with Java-level exception handler: O0 = exception
  //                                  		 O1 = throwing pc

  address StubGenerator::generate_forward_exception() {
    StubCodeMark mark(this, "StubRoutines", "forward_exception");
    address start = __ pc();

    // Upon entry, O7 has the return address returning into Java
    // (interpreted or compiled) code; i.e. the return address
    // becomes the throwing pc.

    const Register& handler_reg = Gtemp;

    Address exception_addr (G2_thread, 0, in_bytes(Thread::pending_exception_offset()));

#ifdef ASSERT
    // make sure that this code is only executed if there is a pending exception
    { Label L;
      __ ld_ptr(exception_addr, Gtemp);
      __ br_notnull(Gtemp, false, Assembler::pt, L);
      __ delayed()->nop();
      __ stop("StubRoutines::forward exception: no pending exception (1)");
      __ bind(L);
    }
#endif

    // compute exception handler into handler_reg
    __ get_thread();
    __ ld_ptr(exception_addr, Oexception);
    __ verify_oop(Oexception, "generate_forward_exception");
    __ save_frame(0);             // compensates for compiler weakness
    __ add(O7->after_save(), frame::pc_return_offset, Lscratch); // save the issuing PC
    __ call_VM_leaf(L7_thread_cache, CAST_FROM_FN_PTR(address, SharedRuntime::exception_handler_for_return_address), Lscratch);
    __ mov(O0, handler_reg);
    __ restore();                 // compensates for compiler weakness

    __ ld_ptr(exception_addr, Oexception);
    __ add(O7, frame::pc_return_offset, Oissuing_pc); // save the issuing PC

#ifdef ASSERT
    // make sure exception is set
    { Label L;
      __ br_notnull(Oexception, false, Assembler::pt, L);
      __ delayed()->nop();
      __ stop("StubRoutines::forward exception: no pending exception (2)");
      __ bind(L);
    }
#endif
    // jump to exception handler
    __ jmp(handler_reg, 0);
    // clear pending exception
    __ delayed()->st_ptr(G0, exception_addr);

    return start;
  }


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
  // Note 2: In case of an exception in the interpreter, the Lbcp (L1) value
  //         has to be saved in the stack frame (calling convention for
  //         interpreter calls). This happens automatically as a result
  //         of register window saving.  However, if the call_VM_base
  //         method of InterpreterMacroAssembler were to perform explicit
  //         state saving, it would need to be emulated here also.
  //
  // Note 3: The previous value of O7 is saved in JavaThread::saved_accumulator().
  //         It must used if the null exception arose in the middle of
  //         a call (e.g., a null receiver getting its type checked).
  //
  // Arguments :
  //
  //      trapping PC:  O7 (may need to be adjusted if an inline cache check)
  //
  // Results:
  //     initiates exception processing; does not return
  //
  address generate_handler_for_implicit_exception(address throw_interpreted, address throw_compiled, address *filter, bool is_implicit_null) {
    StubCodeMark mark(this, "StubRoutines", "handler_for_implicit_exception");
    address start = __ pc();

    // In general we don't care about stack contents (or that tos
    // is saved in O0 for the interpreter) since the stack will
    // be reset when the exception is thrown. In particular, we
    // don't need to preserve call arguments accross a GC.

    Address saved_exception_pc(G2_thread, 0, in_bytes(JavaThread::saved_exception_pc_offset()));

    Label non_interp;
    Label retry;
    Label compiler;

    __ bind(retry);
    __ save_frame(0);         // do not blow any registers!
    __ mov(G1, L1);
    __ mov(G3, L3);
    __ mov(G4, L4);
    __ mov(G5, L5);
    __ verify_thread();
    
    // INTERPRETER CASE
    // decide if exception happened in interpreter
    __ call_VM_leaf(L7_thread_cache, CAST_FROM_FN_PTR(address, SharedRuntime::is_interpreter_exception));
    __ tst(O0);
    __ br(Assembler::zero, true, Assembler::pn, non_interp);
    __ delayed()->ld_ptr(saved_exception_pc, O0);
    __ set((intptr_t)throw_interpreted, O1);
#ifdef COMPILER2
    if (ProfileInterpreter && is_implicit_null) {
      Address is_throwing_null_ptr_exception(G2_thread, 0, in_bytes(JavaThread::is_throwing_null_ptr_exception_offset()));

      __ set(1, G1);
      __ st(G1, is_throwing_null_ptr_exception);
    }
#endif
    __ mov(L1, G1);
    __ mov(L3, G3);
    __ mov(L4, G4);
    __ mov(L5, G5);
    __ verify_thread();
    __ jmp(O1, 0);
    __ delayed()->restore();

    __ bind(non_interp);

    // check if this is an implicit null_check in the 
    // megamorphic_call stub. if so, we need to set the thread's
    // saved_exception_pc to the caller of the stub and remove 
    // the stub's activation frame. this can be found in I7.
    //__ set((intptr_t)StubRoutines::_megamorphic_null_exception_address, O1);
    //__ cmp(O0, O1);
    //__ brx(Assembler::notEqual, false, Assembler::pt, compiler);
    //__ delayed()->nop();
    //__ restore();
    //__ st_ptr(I7, saved_exception_pc, 0);
    //__ br(Assembler::always, false, Assembler::pt, retry);
    //__ delayed()->restore();

    __ bind(compiler);

    // JVMCI EXCEPTION FILTER
    // register values as at the exception point (except for eip & flags) -
    // call exception filter to filter out exceptions happening in external
    // compilers, returns with untouched register values if exception could
    // not be handled
    //
    // note: eventually, with a fully functional JVMCI we need to try out
    //       all 'installed' exception filters here (if we have 2 compilers
    //       in the system, we would have 2 exception filters to try)
    #ifdef COMPILER1
    {
      __ mov(L1, G1);
      __ mov(L3, G3);
      __ mov(L4, G4);
      __ mov(L5, G5);
      __ set((intx)filter, O1);
      __ ld_ptr(O1, 0, O1);
      __ callr(O1, 0);
      __ delayed()->nop();
      __ should_not_reach_here();
    }
    #endif


    // COMPILED CODE CASE - BOTH IMPLICIT AND INLINE-CACHE/VTABLE-STUB
    // exception happened outside of interpreter => compute exception
    // return address, if any.  Must not blow any registers; the implicit
    // exception acts as if it took a long-latency branch to the final target.

    { Label L;
      if (is_implicit_null) {
        __ mov(1, O0);
      } else {
        __ clr(O0);
      }
      __ call_VM_leaf(L7_thread_cache, CAST_FROM_FN_PTR(address, SharedRuntime::compute_exception_return_address), O0);
      __ br_null(O0, false, Assembler::pn, L);
      __ delayed()->nop();

      __ verify_thread();
      __ mov(L1, G1);
      __ mov(L3, G3);
      __ mov(L4, G4);
      __ mov(L5, G5);
      // take a branch (a very slow branch) to the indicated label
      __ jmp(O0, G0);
      __ delayed()->restore();

      __ bind(L);
    }

    // VTABLE-STUB OR INLINE_CACHE CASE

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
       // by the trap Handler just before calling into the stub, we restore
       // it in O7 so that the rest of the exception system can handle
       // the exception correctly
       // restoring %g's not strictly necessary
       __ mov(L1, G1);
       __ mov(L3, G3);
       __ mov(L4, G4);
       __ mov(L5, G5);
       __ set((intptr_t)throw_compiled, G3_scratch);
       __ jmp(G3_scratch, relocInfo::none);
       __ delayed()->restore();
    }

    return start;
  }


  #ifndef CORE
  //----------------------------------------------------------------------------------------------------
  // Explicit throwing of exceptions
  //
  // Arguments :
  //
  //         O7: stub address where called from (to locate the method)
  //         O0: offset of exception occurence (from begin of method code)
  // 
  // Results:
  //
  //         exception pc  in Oissuing_pc
  //         exception oop in Oexception
  //         jumps to exception handler (does not return to the caller)

  address generate_division_by_zero() {
    StubCodeMark mark(this, "StubRoutines", "division_by_zero");
    address start = __ pc();

    __ unimplemented("division_by_zero");
    return start;
  }
  #endif


  // Generate a routine that sets all the registers so we
  // can tell if the stop routine prints them correctly.
  address StubGenerator::generate_test_stop() {
    StubCodeMark mark(this, "StubRoutines", "test_stop");
    address start = __ pc();

    int i;

    __ save_frame(0);

    static jfloat zero = 0.0, one = 1.0;

    // put addr in L0, then load through L0 to F0
    __ set((intptr_t)&zero, L0);  __ ldf( FloatRegisterImpl::S, L0, 0, F0);
    __ set((intptr_t)&one,  L0);  __ ldf( FloatRegisterImpl::S, L0, 0, F1); // 1.0 to F1

    // use add to put 2..18 in F2..F18
    for ( i = 2;  i <= 18;  ++i ) {
      __ fadd( FloatRegisterImpl::S, F1, as_FloatRegister(i-1),  as_FloatRegister(i));
    }

    // Now put double 2 in F16, double 18 in F18
    __ ftof( FloatRegisterImpl::S, FloatRegisterImpl::D, F2, F16 );
    __ ftof( FloatRegisterImpl::S, FloatRegisterImpl::D, F18, F18 );

    // use add to put 20..32 in F20..F32 
    for (i = 20; i < 32; i += 2) {
      __ fadd( FloatRegisterImpl::D, F16, as_FloatRegister(i-2),  as_FloatRegister(i));
    }

    // put 0..7 in i's, 8..15 in l's, 16..23 in o's, 24..31 in g's
    for ( i = 0; i < 8; ++i ) {
      if (i < 6) {
	__ set(     i, as_iRegister(i));
	__ set(16 + i, as_oRegister(i));
	__ set(24 + i, as_gRegister(i));
      }
      __ set( 8 + i, as_lRegister(i));
    }

    __ stop("testing stop");


    __ ret();
    __ delayed()->restore();

    return start;
  }


  address StubGenerator::generate_stop_subroutine() {
    StubCodeMark mark(this, "StubRoutines", "stop_subroutine");
    address start = __ pc();

    __ stop_subroutine();

    return start;
  }

  address StubGenerator::generate_flush_callers_register_windows() {
    StubCodeMark mark(this, "StubRoutines", "flush_callers_register_windows");
    address start = __ pc();

    __ flush_windows();
    __ retl();
    __ delayed()->mov(FP, O0);
    // The returned value must be a stack pointer whose register save area
    // is flushed, and will stay flushed while the caller executes.

    return start;
  }

  // Helper functions for v8 atomic operations.
  // 
  void StubGenerator::get_v8_oop_lock_ptr(Register lock_ptr_reg, Register mark_oop_reg, Register scratch_reg) {
    if (mark_oop_reg == noreg) {
      address lock_ptr = (address)StubRoutines::sparc::atomic_memory_operation_lock_addr();
      __ set((intptr_t)lock_ptr, lock_ptr_reg);
    } else {
      assert(scratch_reg != noreg, "just checking");
      address lock_ptr = (address)StubRoutines::sparc::_v8_oop_lock_cache;
      __ set((intptr_t)lock_ptr, lock_ptr_reg);
      __ and3(mark_oop_reg, StubRoutines::sparc::v8_oop_lock_mask_in_place, scratch_reg);
      __ add(lock_ptr_reg, scratch_reg, lock_ptr_reg);
    }
  }

  void StubGenerator::generate_v8_lock_prologue(Register lock_reg, Register lock_ptr_reg, Register yield_reg, Label& retry, Label& dontyield, Register mark_oop_reg = noreg, Register scratch_reg = noreg) {

    get_v8_oop_lock_ptr(lock_ptr_reg, mark_oop_reg, scratch_reg);
    __ set(StubRoutines::sparc::locked, lock_reg);
    // Initialize yield counter
    __ mov(G0,yield_reg);

    __ bind(retry);
    __ cmp(yield_reg, V8AtomicOperationUnderLockSpinCount);
    __ br(Assembler::less, false, Assembler::pt, dontyield);
    __ delayed()->nop();

    // This code can only be called from inside the VM, this
    // stub is only invoked from os:atomic_increment(). We do
    // not want to use call_VM, because _last_java_sp and such
    // must already be set.
    // 
    // Save the regs and make space for a C call
    __ save(SP, -96, SP);
    __ save_all_globals_into_locals();
    __ call(CAST_FROM_FN_PTR(address, os::Solaris::naked_sleep));
    __ delayed()->nop();
    __ restore_globals_from_locals();
    __ restore();
    // reset the counter
    __ mov(G0,yield_reg);

    __ bind(dontyield);

    // try to get lock
    __ swap(lock_ptr_reg, 0, lock_reg);

    // did we get the lock?
    __ cmp(lock_reg, StubRoutines::sparc::unlocked);
    __ br(Assembler::notEqual, true, Assembler::pn, retry);
    __ delayed()->add(yield_reg,1,yield_reg);

    // yes, got lock. do the operation here.
  }

  void StubGenerator::generate_v8_lock_epilogue(Register lock_reg, Register lock_ptr_reg, Register yield_reg, Label& retry, Label& dontyield, Register mark_oop_reg = noreg, Register scratch_reg = noreg) {
    __ st(lock_reg, lock_ptr_reg, 0); // unlock
  }

  // Support for jint atomic::exchange(jint exchange_value, jint* dest).
  //
  // Arguments :
  //
  //      exchange_value: O0
  //      dest:           O1
  //
  // Results:
  //
  //     O0: the value previously stored in dest
  //
  address StubGenerator::generate_atomic_exchange() {
    StubCodeMark mark(this, "StubRoutines", "atomic_exchange");
    address start = __ pc();

    if (UseCASForSwap) {
      // Use CAS instead of swap, just in case the MP hardware
      // prefers to work with just one kind of synch. instruction.
      Label retry;
      __ bind(retry);
      __ mov(O0, O3);       // scratch copy of exchange value
      __ ld(O1, 0, O2);     // observe the previous value
      // try to replace O2 with O3
      __ cas_under_lock(O1, O2, O3, 
      (address)StubRoutines::sparc::atomic_memory_operation_lock_addr(),false);
      __ cmp(O2, O3);
      __ br(Assembler::notEqual, false, Assembler::pn, retry);
      __ delayed()->nop();

      __ retl();
      __ delayed()->mov(O2, O0);  // report previous value to caller

    } else {
      if (VM_Version::v9_instructions_work()) {
        __ retl();
        __ delayed()->swap(O1, 0, O0);
      } else {
        const Register& lock_reg = O2;
        const Register& lock_ptr_reg = O3;
        const Register& yield_reg = O4;
  
        Label retry;
        Label dontyield;

        generate_v8_lock_prologue(lock_reg, lock_ptr_reg, yield_reg, retry, dontyield);
        // got the lock, do the swap
        __ swap(O1, 0, O0);

        generate_v8_lock_epilogue(lock_reg, lock_ptr_reg, yield_reg, retry, dontyield);
        __ retl();
        __ delayed()->nop();
      }
    }

    return start;
  }


  // Support for jint atomic::compare_and_exchange(jint exchange_value, jint* dest, jint compare_value)
  //
  // Arguments :
  //
  //      exchange_value: O0
  //      dest:           O1
  //      compare_value:  O2
  //
  // Results:
  //
  //     O0: the value previously stored in dest
  //
  // Overwrites (v8): O3,O4,O5
  //
  address StubGenerator::generate_atomic_compare_and_exchange() {
    StubCodeMark mark(this, "StubRoutines", "atomic_compare_and_exchange");
    address start = __ pc();

    // compare_and_swap(dest, compare_value, exchange_value)
    __ cas_under_lock(O1, O2, O0,
      (address)StubRoutines::sparc::atomic_memory_operation_lock_addr(),false);
    __ retl();
    __ delayed()->nop();

    return start;
  }

  // Support for jlong atomic::compare_and_exchange(jlong exchange_value, jlong *dest, jlong compare_value)
  //
  // Arguments :
  //
  //      exchange_value: O1:O0
  //      dest:           O2
  //      compare_value:  O4:O3
  //
  // Results:
  //
  //     O1:O0: the value previously stored in dest
  //
  // This only works on V9, on V8 we don't generate any
  // code and just return NULL.
  //
  // Overwrites: G1,G2,G3
  //
  address StubGenerator::generate_atomic_compare_and_exchange_long() {
    StubCodeMark mark(this, "StubRoutines", "atomic_compare_and_exchange_long");
    address start = __ pc();

    if (!VM_Version::supports_cx8())
        return NULL;;
    __ sllx(O0, 32, O0);
    __ srl(O1, 0, O1);
    __ or3(O0,O1,O0);      // O0 holds 64-bit value from compare_value
    __ sllx(O3, 32, O3);
    __ srl(O4, 0, O4);
    __ or3(O3,O4,O3);     // O3 holds 64-bit value from exchange_value
    __ casx(O2, O3, O0);
    __ srl(O0, 0, O1);    // unpacked return value in O1:O0
    __ retl();
    __ delayed()->srlx(O0, 32, O0);

    return start;
  }


  // Support for jint atomic::add(jint inc, jint* dest).
  //
  // Arguments :
  //
  //      inc: O0   (e.g., +1 or -1)
  //      loc: O1
  //
  // Results:
  //
  //     O0: the new value stored in dest
  //
  // Overwrites (v9): O3
  // Overwrites (v8): O3,O4,O5
  //
  address StubGenerator::generate_atomic_increment() {
    StubCodeMark mark(this, "StubRoutines", "atomic_increment");
    address start = __ pc();
    __ bind(_atomic_increment_stub);

    if (VM_Version::v9_instructions_work()) {
      Label(retry);
      __ bind(retry);

      __ lduw(O1, 0, O2);
      __ add(O0,   O2, O3);
      __ cas(O1,   O2, O3);
      __ cmp(      O2, O3);
      __ br(Assembler::notEqual, false, Assembler::pn, retry);
      __ delayed()->nop();
      __ retl();
      __ delayed()->add(O0, O2, O0); // note that cas made O2==O3
    } else {
      const Register& lock_reg = O2;
      const Register& lock_ptr_reg = O3;
      const Register& value_reg = O4;
      const Register& yield_reg = O5;

      Label(retry);
      Label(dontyield);

      generate_v8_lock_prologue(lock_reg, lock_ptr_reg, yield_reg, retry, dontyield);
      // got lock, do the increment
      __ ld(O1, 0, value_reg);
      __ add(O0, value_reg, value_reg);
      __ st(value_reg, O1, 0);

      // %%% only for RMO and PSO
      __ membar(Assembler::StoreStore); 

      generate_v8_lock_epilogue(lock_reg, lock_ptr_reg, yield_reg, retry, dontyield);

      __ retl();
      __ delayed()->mov(value_reg, O0);
    }

    return start;
  }
  Label _atomic_increment_stub;  // called from other stubs


  // Support for void atomic::membar().
  //
  address StubGenerator::generate_atomic_membar() {
    StubCodeMark mark(this, "StubRoutines", "atomic_membar");
    address start = __ pc();

    __ membar(Assembler::Membar_mask_bits(Assembler::LoadLoad  | Assembler::LoadStore | 
					  Assembler::StoreLoad | Assembler::StoreStore));
    __ retl();
    __ delayed()->nop();

    return start;
  }


  // Support for address call_address()
  // 
  // This routine is used to find the address of the call instruction
  // used to call the current method.  Can be used only from routines
  // that have executed a save instruction.
  //
  address StubGenerator::generate_call_address() {
    StubCodeMark mark(this, "StubRoutines", "call_address");
    address start = __ pc();
    __ retl();
    __ delayed()->mov(I7, O0);
    return start;
  }



  // Support for void copy_words_aligned8_lower(HeapWord* from, HeapWord* to, size_t count)
  // 
  // Arguments:
  //     from:  O0
  //     to:    O1
  //     count: O2   must be non-zero
  //
  // Destroys O4 (v9) or F0 (v8) and F2-F7.
  //
  address StubGenerator::generate_copy_words_aligned8_lower() {
    StubCodeMark mark(this, "StubRoutines", "copy_words_aligned8_lower");
    Label(loop_start);
    Label(loop_end);

    __ align(32);
    address start = __ pc();

    if (HeapWordSize == sizeof(jint)*2) {
      __ add(O2, O2, O2);                                 // word count
    } else {
      assert(HeapWordSize == sizeof(jint), "HeapWordSize must be 4 or 8");
      __ add(O2, 1, O2);                                  // round up to multiple of 2
      __ andn(O2, 1, O2);
    }
    __ cmp(O2, 8);
    __ brx(Assembler::less, false, Assembler::pn, loop_end);
    if (VM_Version::has_v9()) {
      __ delayed()->ldx(O0, 0, O4);                       // load0
    } else {
      __ delayed()->ldf(FloatRegisterImpl::D, O0, 0, F0); // load0
    }

    __ align(16);
  __ bind(loop_start);
    __ ldf(FloatRegisterImpl::D, O0, 8, F2);              // load8
    __ add(O1, 32, O1);
    __ ldf(FloatRegisterImpl::D, O0, 16, F4);             // load16
    __ sub(O2, 8, O2);
    __ ldf(FloatRegisterImpl::D, O0, 24, F6);             // load24
    __ cmp(O2, 7);
    if (VM_Version::has_v9()) {
      __ stx(O4, O1, -32);                                // store0
    } else {
      __ stf(FloatRegisterImpl::D, F0, O1, -32);          // store0
    }
    __ add(O0, 32, O0);
    __ stf(FloatRegisterImpl::D, F2, O1, -24);            // store8
    __ stf(FloatRegisterImpl::D, F4, O1, -16);            // store16
    __ stf(FloatRegisterImpl::D, F6, O1, -8);             // store24
    __ brx(Assembler::greater, true, Assembler::pt, loop_start);
    if (VM_Version::has_v9()) {
      __ delayed()->ldx(O0, 0, O4);                       // load0
    } else {
      __ delayed()->ldf(FloatRegisterImpl::D, O0, 0, F0); // load0
    }

    __ cmp(O2, 0);
    __ brx(Assembler::greater, true, Assembler::pt, loop_end);
    if (VM_Version::has_v9()) {
      __ delayed()->ldx(O0, 0, O4);                       // load0
    } else {
      __ delayed()->ldf(FloatRegisterImpl::D, O0, 0, F0); // load0
    }

    __ retl();
    __ delayed()->nop();

    __ align(16);
  __ bind(loop_end);
    __ add(O0, 8, O0);
    __ add(O1, 8, O1);
    __ subcc(O2, 2, O2);
    if (VM_Version::has_v9()) {
      __ stx(O4, O1, -8);                                 // store0
    } else {
      __ stf(FloatRegisterImpl::D, F0, O1, -8);           // store0
    }
    __ brx(Assembler::greater, true, Assembler::pt, loop_end);
    if (VM_Version::has_v9()) {
      __ delayed()->ldx(O0, 0, O4);                       // load0
    } else {
      __ delayed()->ldf(FloatRegisterImpl::D, O0, 0, F0); // load0
    }

    __ retl();
    __ delayed()->nop();

    return start;
  }


  // Support for void copy_words_aligned8_higher(HeapWord* from, HeapWord* to, size_t count)
  // 
  // Arguments:
  //     from:  O0
  //     to:    O1
  //     count: O2   must be non-zero
  //
  // Destroys O4 (v9) or F0 (v8) and O3,F2-F7.
  //
  address StubGenerator::generate_copy_words_aligned8_higher() {
    StubCodeMark mark(this, "StubRoutines", "copy_words_aligned8_higher");
    Label(loop_start);
    Label(loop_end);

    __ align(32);
    address start = __ pc();

    __ sll(O2, LogHeapWordSize, O3);                       // byte count
    if (HeapWordSize == sizeof(jint)*2) {
      __ add(O2, O2, O2);                                  // word count
      __ add(O1, O3, O1);                                  // to + count
      __ add(O0, O3, O0);                                  // from + count
    } else {
      assert(HeapWordSize == sizeof(jint), "HeapWordSize must be 4 or 8");
      __ add(O2, 1, O2);                                   // round up to multiple of 2
      __ add(O1, O3, O1);                                  // to + count
      __ andn(O2, 1, O2);
      __ add(O0, O3, O0);                                  // from + count
    }
    __ cmp(O2, 8);
    __ brx(Assembler::less, false, Assembler::pn, loop_end);
    if (VM_Version::has_v9()) {
      __ delayed()->ldx(O0, -8, O4);                       // load0
    } else {
      __ delayed()->ldf(FloatRegisterImpl::D, O0, -8, F0); // load0
    }

    __ align(16);
  __ bind(loop_start);
    __ ldf(FloatRegisterImpl::D, O0, -16, F2);             // load8
    __ sub(O1, 32, O1);
    __ ldf(FloatRegisterImpl::D, O0, -24, F4);             // load16
    __ sub(O2, 8, O2);
    __ ldf(FloatRegisterImpl::D, O0, -32, F6);             // load24
    __ cmp(O2, 7);
    if (VM_Version::has_v9()) {
      __ stx(O4, O1, 24);                                  // store0
    } else {
      __ stf(FloatRegisterImpl::D, F0, O1, 24);            // store0
    }
    __ sub(O0, 32, O0);
    __ stf(FloatRegisterImpl::D, F2, O1, 16);              // store8
    __ stf(FloatRegisterImpl::D, F4, O1, 8);               // store16
    __ stf(FloatRegisterImpl::D, F6, O1, 0);               // store24
    __ brx(Assembler::greater, true, Assembler::pt, loop_start);
    if (VM_Version::has_v9()) {
      __ delayed()->ldx(O0, -8, O4);                       // load0
    } else {
      __ delayed()->ldf(FloatRegisterImpl::D, O0, -8, F0); // load0
    }

    __ cmp(O2, 0);
    __ brx(Assembler::greater, true, Assembler::pt, loop_end);
    if (VM_Version::has_v9()) {
      __ delayed()->ldx(O0, -8, O4);                       // load0
    } else {
      __ delayed()->ldf(FloatRegisterImpl::D, O0, -8, F0); // load0
    }

    __ retl();
    __ delayed()->nop();

    __ align(16);
  __ bind(loop_end);
    __ sub(O0, 8, O0);
    __ sub(O1, 8, O1);
    __ subcc(O2, 2, O2);
    if (VM_Version::has_v9()) {
      __ stx(O4, O1, 0);                                  // store0
    } else {
      __ stf(FloatRegisterImpl::D, F0, O1, 0);            // store0
    }
    __ brx(Assembler::greater, true, Assembler::pt, loop_end);
    if (VM_Version::has_v9()) {
      __ delayed()->ldx(O0, -8, O4);                       // load0
    } else {
      __ delayed()->ldf(FloatRegisterImpl::D, O0, -8, F0); // load0
    }

    __ retl();
    __ delayed()->nop();

    return start;
  }


  // Support for void set_words_aligned8(HeapWord* to, size_t count, junit value)
  // 
  // Arguments:
  //     to:    O0
  //     count: O1   must be non-zero
  //     value: O2
  //
  // Destroys O4 (v9) or F0 (v8) and O3,F2-F7.
  //
  address StubGenerator::generate_set_words_aligned8() {
    StubCodeMark mark(this, "StubRoutines", "set_words_aligned8");

    __ align(32);
    address start = __ pc();

    __ retl();
    __ delayed()->nop();

    return start;
  }


  // Support for void zero_words_aligned8(HeapWord* to, size_t count)
  // 
  // Arguments:
  //     to:    O0
  //     count: O1   must be non-zero
  //
  // Destroys O4 (v9) or F0 (v8) and O3,F2-F7.
  //
  address StubGenerator::generate_zero_words_aligned8() {
    StubCodeMark mark(this, "StubRoutines", "zero_words_aligned8");

    __ align(32);
    address start = __ pc();

    __ retl();
    __ delayed()->nop();

    return start;
  }


  //------------------------------------------------------------------------------------------------------------------------
  // The following routine generates a subroutine to throw an asynchronous
  // UnknownError when an unsafe access gets a fault that could not be
  // reasonably prevented by the programmer.  (Example: SIGBUS/OBJERR.)
  //
  // Arguments :
  //
  //      trapping PC:    O7
  //
  // Results:
  //     posts an asynchronous exception, skips the trapping instruction
  //

  address generate_handler_for_unsafe_access() {
    StubCodeMark mark(this, "StubRoutines", "handler_for_unsafe_access");
    address start = __ pc();

    const int preserve_register_words = (64 * 2);
    Address preserve_addr(FP, 0, (-preserve_register_words * wordSize) + STACK_BIAS);

    Register Lthread = L7_thread_cache;
    int i;

    __ save_frame(0);
    __ mov(G1, L1);
    __ mov(G2, L2);
    __ mov(G3, L3);
    __ mov(G4, L4);
    __ mov(G5, L5);
    for (i = 0; i < (VM_Version::v9_instructions_work() ? 64 : 32); i += 2) {
      __ stf(FloatRegisterImpl::D, as_FloatRegister(i), preserve_addr, i * wordSize);
    }
    
    address entry_point = CAST_FROM_FN_PTR(address, handle_unsafe_access);
    __ call(entry_point, relocInfo::runtime_call_type);
    __ delayed()->nop();

    __ mov(L1, G1);
    __ mov(L2, G2);
    __ mov(L3, G3);
    __ mov(L4, G4);
    __ mov(L5, G5);
    for (i = 0; i < (VM_Version::v9_instructions_work() ? 64 : 32); i += 2) {
      __ ldf(FloatRegisterImpl::D, preserve_addr, as_FloatRegister(i), i * wordSize);
    }

    __ verify_thread();

    __ jmp(O0, 0);
    __ delayed()->restore();

    return start;
  }


  // Support for uint StubRoutine::sparc::partial_subtype_check( Klass sub, Klass super );
  // Arguments :
  //
  //      ret  : O0, returned
  //      icc/xcc: set as O0
  //      sub  : O1, argument, not changed
  //      super: O2, argument, not changed
  //      raddr: O7, blown by call
  address generate_partial_subtype_check() {
    StubCodeMark mark(this, "StubRoutines", "partial_subtype_check");
    address start = __ pc();
    Label loop, miss;

#if defined(COMPILER2) && !defined(_LP64)
    // Do not use a 'save' because it blows the 64-bit O registers.
    __ add(SP,-4*wordSize,SP);  // Make space for 4 temps
    __ st_ptr(L0,SP,(frame::register_save_words+0)*wordSize);
    __ st_ptr(L1,SP,(frame::register_save_words+1)*wordSize);
    __ st_ptr(L2,SP,(frame::register_save_words+2)*wordSize);
    __ st_ptr(L3,SP,(frame::register_save_words+3)*wordSize);
    Register Rret   = O0;
    Register Rsub   = O1;
    Register Rsuper = O2;
#else
    __ save_frame(0);
    Register Rret   = I0;
    Register Rsub   = I1;
    Register Rsuper = I2;
#endif

    Register L0_ary_len = L0;
    Register L1_ary_ptr = L1;
    Register L2_super   = L2;
    Register L3_index   = L3;

    __ ld_ptr( Rsub, sizeof(oopDesc) + Klass::secondary_supers_offset_in_bytes(), L3 );
    __ lduw(L3,arrayOopDesc::length_offset_in_bytes(),L0_ary_len); 
    __ add(L3,arrayOopDesc::base_offset_in_bytes(T_OBJECT),L1_ary_ptr); 
    __ clr(L3_index);           // zero index
    // Load a little early; will load 1 off the end of the array.
    // Ok for now; revisit if we have other uses of this routine.
    __ ld_ptr(L1_ary_ptr,0,L2_super);// Will load a little early
    __ align(CodeEntryAlignment);

    // The scan loop
    __ bind(loop);
    __ add(L1_ary_ptr,wordSize,L1_ary_ptr); // Bump by OOP size
    __ cmp(L3_index,L0_ary_len); 
    __ br(Assembler::equal,false,Assembler::pn,miss);
    __ delayed()->inc(L3_index); // Bump index
    __ subcc(L2_super,Rsuper,Rret);   // Check for match; zero in Rret for a hit
    __ brx( Assembler::notEqual, false, Assembler::pt, loop );
    __ delayed()->ld_ptr(L1_ary_ptr,0,L2_super); // Will load a little early

    // Got a hit; report success; set cache.  Cache load doesn't
    // happen here; for speed it is directly emitted by the compiler.
    __ st_ptr( Rsuper, Rsub, sizeof(oopDesc) + Klass::secondary_super_cache_offset_in_bytes() );

#if defined(COMPILER2) && !defined(_LP64)
    __ ld_ptr(SP,(frame::register_save_words+0)*wordSize,L0);
    __ ld_ptr(SP,(frame::register_save_words+1)*wordSize,L1);
    __ ld_ptr(SP,(frame::register_save_words+2)*wordSize,L2);
    __ ld_ptr(SP,(frame::register_save_words+3)*wordSize,L3);
    __ retl();			// Result in Rret is ok; flags set to Z
    __ delayed()->add(SP,4*wordSize,SP);
#else
    __ ret();			// Result in Rret is ok; flags set to Z
    __ delayed()->restore();
#endif

    // Hit or miss falls through here
    __ bind(miss);
    __ addcc(G0,1,Rret);        // set NZ flags, NZ result

#if defined(COMPILER2) && !defined(_LP64)
    __ ld_ptr(SP,(frame::register_save_words+0)*wordSize,L0);
    __ ld_ptr(SP,(frame::register_save_words+1)*wordSize,L1);
    __ ld_ptr(SP,(frame::register_save_words+2)*wordSize,L2);
    __ ld_ptr(SP,(frame::register_save_words+3)*wordSize,L3);
    __ retl();			// Result in O0 is ok; flags set to NZ
    __ delayed()->add(SP,4*wordSize,SP);
#else
    __ ret();			// Result in Rret is ok; flags set to Z
    __ delayed()->restore();
#endif

    return start;
  }


  // Called from MacroAssembler::verify_oop
  //
  address generate_verify_oop_subroutine() {
    StubCodeMark mark(this, "StubRoutines", "verify_oop_stub");
     
    address start = __ pc();
    
    __ verify_oop_subroutine();
    
    return start;
  }


  //-----------------------------------------------------------------
  // Called when an interpreter call needs an i2c adapter before continuing in
  // the compiled callee.  This is typically called from a vtable.
  // 
  // ebx: methodOop
  //

#ifdef COMPILER2
  address generate_lazy_i2c_adapter_entry() {
    StubCodeMark mark(this, "StubRoutines", "lazy_i2c_adapter_entry");
    address start = __ pc();

    __ save_frame(0);
    __ mov(G5_method, O1);        // argument in O1
    __ set_last_Java_frame(FP, G0);
    __ save_thread(L7_thread_cache);
    __ call(CAST_FROM_FN_PTR(address, SharedRuntime::create_i2c_adapter), 
            relocInfo::runtime_call_type);
    __ delayed()->mov(L7_thread_cache, O0); // thread in O0
    __ restore_thread(L7_thread_cache);
    __ reset_last_Java_frame();
    __ get_vm_result(G5_method);
    // O0:      i2c adapter destination 
    // G5_method: methodOop
    __ jmp(O0, 0);
    __ delayed()->restore();

    return start;
  }
#endif // COMPILER2


  void generate_initial() {
    // Generates all stubs and initializes the entry points

    //------------------------------------------------------------------------------------------------------------------------
    // entry points that exist in all platforms
    // Note: This is code that could be shared among different platforms - however the benefit seems to be smaller than
    //       the disadvantage of having a much more complicated generator structure. See also comment in stubRoutines.hpp.
    StubRoutines::_forward_exception_entry                 = generate_forward_exception();

    StubRoutines::_call_stub_entry                         = generate_call_stub(StubRoutines::_call_stub_return_address);    
    StubRoutines::_catch_exception_entry                   = generate_catch_exception();

    COMPILER2_ONLY(StubRoutines::_lazy_i2c_adapter_entry   = generate_lazy_i2c_adapter_entry();)

    //------------------------------------------------------------------------------------------------------------------------
    // entry points that are platform specific
    StubRoutines::sparc::_test_stop_entry                  = generate_test_stop();

    StubRoutines::sparc::_stop_subroutine_entry            = generate_stop_subroutine();
    StubRoutines::sparc::_flush_callers_register_windows_entry = generate_flush_callers_register_windows();

#if !defined(COMPILER2) && !defined(_LP64)
    StubRoutines::_atomic_exchange_entry                       = generate_atomic_exchange();
    StubRoutines::_atomic_compare_and_exchange_entry           = generate_atomic_compare_and_exchange();
    StubRoutines::_atomic_increment_entry                      = generate_atomic_increment();
    StubRoutines::_atomic_exchange_ptr_entry                   = StubRoutines::_atomic_exchange_entry;
    StubRoutines::_atomic_compare_and_exchange_ptr_entry       = StubRoutines::_atomic_compare_and_exchange_entry;
    StubRoutines::_atomic_compare_and_exchange_long_entry      = generate_atomic_compare_and_exchange_long();
    StubRoutines::_atomic_increment_ptr_entry                  = StubRoutines::_atomic_increment_entry;
    StubRoutines::_atomic_membar_entry                         = generate_atomic_membar();
#endif  // COMPILER2 !=> _LP64

    StubRoutines::_call_address_entry                          = generate_call_address();

    StubRoutines::sparc::_copy_words_aligned8_lower_entry      = generate_copy_words_aligned8_lower();
    StubRoutines::sparc::_copy_words_aligned8_higher_entry     = generate_copy_words_aligned8_higher();
    StubRoutines::sparc::_set_words_aligned8_entry             = generate_set_words_aligned8();
    StubRoutines::sparc::_zero_words_aligned8_entry            = generate_zero_words_aligned8();
    StubRoutines::sparc::_partial_subtype_check                = generate_partial_subtype_check();
  }


  void generate_all() {
    // Generates all stubs and initializes the entry points

    //------------------------------------------------------------------------------------------------------------------------
    // entry points that are platform specific
    StubRoutines::sparc::_handler_for_divide_by_zero_entry =
      generate_handler_for_implicit_exception(
        Interpreter::throw_ArithmeticException_entry(),
        NOT_COMPILER2(NULL) COMPILER2_ONLY(OptoRuntime::throw_div0_exception_stub()),
        &SharedRuntime::_implicit_div0_exception_filter,
        false
      );
 
    StubRoutines::sparc::_handler_for_null_exception_entry =
      generate_handler_for_implicit_exception(
        Interpreter::throw_NullPointerException_entry(),
        NOT_COMPILER2(NULL) COMPILER2_ONLY(OptoRuntime::throw_null_exception_stub()),
        &SharedRuntime::_implicit_null_exception_filter,
        true
      );
 
    StubRoutines::sparc::_handler_for_stack_overflow_entry =
      generate_handler_for_implicit_exception(
        Interpreter::throw_StackOverflowError_entry(),
        NOT_COMPILER2(NULL) COMPILER2_ONLY(OptoRuntime::throw_stack_overflow_error_stub()),
        &SharedRuntime::_stack_ovrflow_exception_filter,
        false
      );

    StubRoutines::sparc::_handler_for_unsafe_access_entry  = generate_handler_for_unsafe_access();


    // support for verify_oop (must happen after universe_init)
    StubRoutines::_verify_oop_subroutine_entry	   = generate_verify_oop_subroutine();
  }


  class StubMacroAssembler: public MacroAssembler {
    StubGenerator* _sg;

   public:
    StubMacroAssembler(CodeBuffer* code, StubGenerator* sg) : MacroAssembler(code) {
      _sg = sg;
    }

    // Stub generator version of call_VM_base.
    // It differs in two ways from the standard one:
    // First, it uses this module's standard "L7_thread_cache".
    // Second, the last_Java_sp to record is actually the FP,
    // since all stubs save their own private register windows.
    virtual void call_VM_base(
      Register        oop_result,
      Register        java_thread,
      Register        last_java_sp,
      address         entry_point,
      int             number_of_arguments,
      bool	      check_exception
    ) {
      assert(!java_thread->is_valid(), "checking");
      java_thread = L7_thread_cache;

      assert(!last_java_sp->is_valid(), "checking");
      last_java_sp = FP;

      MacroAssembler::call_VM_base(oop_result, java_thread, last_java_sp, entry_point, number_of_arguments, check_exception);
    }
  };
  friend StubMacroAssembler;

 public:
  StubGenerator(CodeBuffer* code, bool all) : StubCodeGenerator(code) {
    // replace the standard masm with a special one:
    _masm = new StubMacroAssembler(code, this);

    _stub_count = !all ? 0x100 : 0x200;
    if (all) {
      generate_all();
    } else {
      generate_initial();
    }

    // make sure this stub is available for all local calls
    if (_atomic_increment_stub.is_unbound()) {
      // generate a second time, if necessary
      (void) generate_atomic_increment();
    }
  }


 private:
  int _stub_count;
  void stub_prolog(StubCodeDesc* cdesc) {
    # ifdef ASSERT
      // put extra information in the stub code, to make it more readable
#ifdef _LP64
// Write the high part of the address
// [RGV] Check if there is a dependency on the size of this prolog
      __ emit_data((intptr_t)cdesc >> 32,    relocInfo::none);
#endif
      __ emit_data((intptr_t)cdesc,    relocInfo::none);
      __ emit_data(++_stub_count, relocInfo::none);
    # endif
    align(true);
  }

  void align(bool at_header = false) {
    // %%%%% move this constant somewhere else
    // UltraSPARC cache line size is 8 instructions:
    const unsigned int icache_line_size = 32;
    const unsigned int icache_half_line_size = 16;

    if (at_header) {
      while ((intptr_t)(__ pc()) % icache_line_size != 0) {
	__ emit_data(0, relocInfo::none);
      }
    } else {
      while ((intptr_t)(__ pc()) % icache_half_line_size != 0) {
	__ nop();
      }
    }
  }

}; // end class declaration


void StubGenerator_generate(CodeBuffer* code, bool all) {
  StubGenerator g(code, all);
}


//Reconciliation History
// 1.11 97/07/03 14:03:24 stubRoutines_i486.cpp
// 1.17 97/07/14 16:43:44 stubRoutines_i486.cpp
// 1.7 97/08/06 11:02:34 stubGenerator_i486.cpp
// 1.15 97/08/20 03:20:39 stubGenerator_i486.cpp
// 1.25 97/09/11 17:20:56 stubGenerator_i486.cpp
// 1.26 97/09/11 18:14:09 stubGenerator_i486.cpp
// 1.30 97/10/06 16:08:07 stubGenerator_i486.cpp
// 1.37 97/10/16 15:47:08 stubGenerator_i486.cpp
// 1.42 97/10/21 11:54:32 stubGenerator_i486.cpp
// 1.54 97/10/28 17:55:26 stubGenerator_i486.cpp
// 1.63 97/11/06 08:49:24 stubGenerator_i486.cpp
// 1.68 97/11/11 14:54:45 stubGenerator_i486.cpp
// 1.81 97/11/19 17:27:45 stubGenerator_i486.cpp
// 1.75 97/12/03 18:51:59 stubGenerator_i486.cpp
// 1.77 97/12/08 10:50:13 stubGenerator_i486.cpp
// 1.92 98/01/06 15:24:49 stubGenerator_i486.cpp
// 1.97 98/01/14 11:14:58 stubGenerator_i486.cpp
// 1.101 98/01/16 23:04:07 stubGenerator_i486.cpp
// 1.112 98/01/28 12:13:26 stubGenerator_i486.cpp
// 1.119 98/02/05 13:40:24 stubGenerator_i486.cpp
// 1.140 98/02/26 18:46:40 stubGenerator_i486.cpp
// 1.146 98/03/12 16:19:59 stubGenerator_i486.cpp
// 1.147 98/03/17 11:33:10 stubGenerator_i486.cpp
// 1.149 98/03/19 18:59:29 stubGenerator_i486.cpp
// 1.151 98/03/26 10:37:35 stubGenerator_i486.cpp
// 1.152 98/03/27 09:49:48 stubGenerator_i486.cpp
// 1.154 98/04/07 15:43:24 stubGenerator_i486.cpp
// 1.155 98/04/09 16:49:29 stubGenerator_i486.cpp
// 1.161 98/04/24 12:52:59 stubGenerator_i486.cpp
// 1.163 98/05/05 14:42:07 stubGenerator_i486.cpp
// 1.168 98/05/11 12:59:19 stubGenerator_i486.cpp
// 1.174 98/05/19 16:56:57 stubGenerator_i486.cpp
// 1.179 98/05/22 11:38:48 stubGenerator_i486.cpp
// 1.182 98/05/26 17:33:00 stubGenerator_i486.cpp
// 1.184 98/06/01 11:28:51 stubGenerator_i486.cpp
// 1.185 98/06/03 09:33:43 stubGenerator_i486.cpp
// 1.194 98/06/17 08:19:08 stubGenerator_i486.cpp
// 1.198 98/06/19 10:07:35 stubGenerator_i486.cpp
// 1.206 98/07/02 13:50:58 stubGenerator_i486.cpp
// 1.209 98/07/24 15:30:08 stubGenerator_i486.cpp
// 1.210 98/08/13 16:30:08 stubGenerator_i486.cpp
// 1.211 98/08/17 17:03:34 stubGenerator_i486.cpp
// 1.215 98/08/28 11:31:05 stubGenerator_i486.cpp
// 1.218 98/09/02 12:58:26 stubGenerator_i486.cpp
// 1.220 98/10/02 15:34:01 stubGenerator_i486.cpp
// 1.222 98/10/06 14:41:06 stubGenerator_i486.cpp
// 1.228 98/10/21 18:36:46 stubGenerator_i486.cpp
// 1.229 98/10/22 17:38:15 stubGenerator_i486.cpp
// 1.230 98/10/27 13:56:30 stubGenerator_i486.cpp
// 1.232 98/11/17 14:00:24 stubGenerator_i486.cpp
// 1.235 98/11/23 10:31:18 stubGenerator_i486.cpp
// 1.236 98/11/24 10:30:45 stubGenerator_i486.cpp
// 1.238 98/12/10 16:47:33 stubGenerator_i486.cpp
// 1.239 99/01/06 16:19:00 stubGenerator_i486.cpp
// 1.240 99/01/08 09:04:34 stubGenerator_i486.cpp
// 1.242 99/01/15 16:45:34 stubGenerator_i486.cpp
// 1.244 99/01/18 17:43:18 stubGenerator_i486.cpp
// 1.247 99/01/25 11:50:15 stubGenerator_i486.cpp
// 1.248 99/03/02 16:52:12 stubGenerator_i486.cpp
// 1.250 99/03/30 14:59:45 stubGenerator_i486.cpp
// 1.252 99/04/06 01:07:28 stubGenerator_i486.cpp
// 1.254 99/06/28 09:59:23 stubGenerator_i486.cpp
// 1.251 99/04/01 16:53:04 stubGenerator_i486.cpp
// 1.252 99/04/06 01:07:28 stubGenerator_i486.cpp
// 1.255 99/07/06 16:03:00 stubGenerator_i486.cpp
// 1.257 99/07/20 10:58:46 stubGenerator_i486.cpp
// 1.2 99/07/30 13:53:10 stubGenerator_i486.cpp
// 1.4 99/08/16 13:11:52 stubGenerator_i486.cpp
//End

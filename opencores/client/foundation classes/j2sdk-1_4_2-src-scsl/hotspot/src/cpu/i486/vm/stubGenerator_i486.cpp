#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)stubGenerator_i486.cpp	1.38 03/01/23 10:55:47 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_stubGenerator_i486.cpp.incl"

// Declaration and definition of StubGenerator (no .hpp file).
// For a more detailed description of the stub routine structure
// see the comment in stubRoutines.hpp

#define __ _masm->

// -------------------------------------------------------------------------------------------------------------------------
// Stub Code definitions

static const double pi_4 = 0.7853981633974483;

static address handle_unsafe_access() {
  JavaThread* thread = JavaThread::current();
  address pc  = thread->saved_exception_pc();
  // pc is the instruction which we must emulate
  // doing a no-op is fine:  return garbage from the load
  // therefore, compute npc
  address npc = Assembler::locate_next_instruction(pc);

  // request an async exception
  thread->set_pending_unsafe_access_error();

  // return address of next instruction to execute
  return npc;
}

class StubGenerator: public StubCodeGenerator {
 private:

  //------------------------------------------------------------------------------------------------------------------------
  // Call stubs are used to call Java from C
  //
  //    [ return_from_Java     ] <--- esp
  //    [ argument word n      ]
  //      ...
  // -4 [ argument word 1      ]
  // -3 [ saved ebx            ] <--- esp_after_call
  // -2 [ saved esi            ]
  // -1 [ saved edi            ]
  //  0 [ saved ebp            ] <--- ebp
  //  1 [ return address       ]
  //  2 [ ptr. to call wrapper ]
  //  3 [ result               ]
  //  4 [ result_type          ]
  //  5 [ method               ]
  //  6 [ entry_point          ]
  //  7 [ parameters           ]
  //  8 [ parameter_size       ]
  //  9 [ thread               ]


  address generate_call_stub(address& return_address) {
    StubCodeMark mark(this, "StubRoutines", "call_stub");
    address start = __ pc();

    // stub code parameters / addresses
    assert(frame::entry_frame_call_wrapper_offset == 2, "adjust this code");
    const Address esp_after_call(ebp, -3 * wordSize); // same as in generate_catch_exception()!
    const Address result        (ebp,  3 * wordSize);
    const Address result_type   (ebp,  4 * wordSize);
    const Address method        (ebp,  5 * wordSize);
    const Address entry_point   (ebp,  6 * wordSize);
    const Address parameters    (ebp,  7 * wordSize);
    const Address parameter_size(ebp,  8 * wordSize);
    const Address thread        (ebp,  9 * wordSize); // same as in generate_catch_exception()!

    // stub code
    __ enter();    

    // save edi, esi, & ebx, according to C calling conventions
    __ pushl(edi);
    __ pushl(esi);
    __ pushl(ebx);

#ifdef ASSERT
    // make sure we have no pending exceptions
    { Label L;
      __ movl(ecx, thread);
      __ cmpl(Address(ecx, Thread::pending_exception_offset()), (int)NULL);
      __ jcc(Assembler::equal, L);
      __ stop("StubRoutines::call_stub: entered with pending exception");
      __ bind(L);
    }
#endif

    // pass parameters if any
    Label parameters_done;
    __ movl(ecx, parameter_size);  // parameter counter
    __ testl(ecx, ecx);
    __ jcc(Assembler::zero, parameters_done);

    // parameter passing loop

    Label loop;
    __ movl(edx, parameters);	       // parameter pointer
    __ movl(esi, ecx);                 // parameter counter is in esi now
    __ movl(ecx,  Address(edx));       // get first parameter in case it is a receiver

    __ bind(loop);
    __ movl(eax, Address(edx));	       // get parameter
    __ addl(edx, wordSize);            // advance to next parameter
    __ decl(esi);                      // decrement counter
    __ pushl(eax);                     // pass parameter
    __ jcc(Assembler::notZero, loop);

    // call Java function
    __ bind(parameters_done);
    __ movl(ebx, method);              // get methodOop
    __ movl(esi, entry_point);         // get entry_point
    __ call(esi, relocInfo::none);
    return_address = __ pc();

    // store result depending on type
    // (everything that is not T_LONG, T_FLOAT or T_DOUBLE is treated as T_INT)
    __ movl(edi, result);
    Label is_long, is_float, is_double, exit;
    __ movl(esi, result_type);
    __ cmpl(esi, T_LONG);
    __ jcc(Assembler::equal, is_long);
    __ cmpl(esi, T_FLOAT);
    __ jcc(Assembler::equal, is_float);
    __ cmpl(esi, T_DOUBLE);
    __ jcc(Assembler::equal, is_double);

    // handle T_INT case
    __ movl(Address(edi), eax);
    __ bind(exit);

    if (UseC2CallingConventions || UseFixedFrameSize) {
      // pop parameters
      __ movl(ecx, parameter_size);
      __ leal(esp, Address(esp, ecx, Address::times_4));
    }

    // check if parameters have been popped correctly
#ifdef ASSERT
      Label esp_wrong;
      __ leal(edi, esp_after_call);
      __ cmpl(esp, edi);
      __ jcc(Assembler::notEqual, esp_wrong);
#endif

    // restore edi & esi
    __ popl(ebx);
    __ popl(esi);
    __ popl(edi);    

    // return
    __ popl(ebp);
    __ ret(0);

    // handle return types different from T_INT
    __ bind(is_long);
    __ movl(Address(edi, 0 * wordSize), eax);
    __ movl(Address(edi, 1 * wordSize), edx);
    __ jmp(exit);

    __ bind(is_float);
    __ fstp_s(Address(edi));
    __ jmp(exit);

    __ bind(is_double);
    __ fstp_d(Address(edi));
    __ jmp(exit);

#ifdef ASSERT
      // stack pointer misadjusted
      __ bind(esp_wrong);
      __ stop("esp wrong after Java call");
#endif

    return start;
  }


  //------------------------------------------------------------------------------------------------------------------------
  // Return point for a Java call if there's an exception thrown in Java code.
  // The exception is caught and transformed into a pending exception stored in
  // JavaThread that can be tested from within the VM.
  //
  // Note: Usually the parameters are removed by the callee. In case of an exception
  //       crossing an activation frame boundary, that is not the case if the callee
  //       is compiled code => need to setup the esp.
  //
  // eax: exception oop

  address generate_catch_exception() {
    StubCodeMark mark(this, "StubRoutines", "catch_exception");
    const Address esp_after_call(ebp, -3 * wordSize); // same as in generate_call_stub()!
    const Address thread        (ebp,  9 * wordSize); // same as in generate_call_stub()!
    address start = __ pc();

    // get thread directly
    __ movl(ecx, thread);
#ifdef ASSERT
    // verify that threads correspond
    { Label L;
      __ get_thread(ebx);
      __ cmpl(ebx, ecx);
      __ jcc(Assembler::equal, L);
      __ stop("StubRoutines::catch_exception: threads must correspond");
      __ bind(L);
    }
#endif
    // set pending exception
    __ verify_oop(eax);
    __ movl(Address(ecx, Thread::pending_exception_offset()), eax          );
    __ movl(Address(ecx, Thread::exception_file_offset   ()), (int)__FILE__);
    __ movl(Address(ecx, Thread::exception_line_offset   ()),      __LINE__);
    // setup esp (remove parameters - see also note above)
    if (!UseC2CallingConventions && !UseFixedFrameSize) {
      __ leal(esp, esp_after_call);
    }
    // complete return to VM
    assert(StubRoutines::_call_stub_return_address != NULL, "_call_stub_return_address must have been generated before");
    __ jmp(StubRoutines::_call_stub_return_address, relocInfo::none);

    return start;
  }

  
  //------------------------------------------------------------------------------------------------------------------------
  // Continuation point for runtime calls returning with a pending exception.
  // The pending exception check happened in the runtime or native call stub.
  // The pending exception in Thread is converted into a Java-level exception.
  //
  // Contract with Java-level exception handlers:
  // eax: exception
  // edx: throwing pc
  //
  // NOTE: At entry of this stub, exception-pc must be on stack !!

  address generate_forward_exception() {
    StubCodeMark mark(this, "StubRoutines", "forward exception");
    address start = __ pc();

    // Upon entry, the sp points to the return address returning into Java
    // (interpreted or compiled) code; i.e., the return address becomes the
    // throwing pc.
    //
    // Arguments pushed before the runtime call are still on the stack but
    // the exception handler will reset the stack pointer -> ignore them.
    // A potential result in registers can be ignored as well.

#ifdef ASSERT
    // make sure this code is only executed if there is a pending exception
    { Label L;
      __ get_thread(ecx);
      __ cmpl(Address(ecx, Thread::pending_exception_offset()), (int)NULL);
      __ jcc(Assembler::notEqual, L);
      __ stop("StubRoutines::forward exception: no pending exception (1)");
      __ bind(L);
    }
#endif

    // compute exception handler into ebx
    __ movl(eax, Address(esp));
    __ call_VM_leaf(CAST_FROM_FN_PTR(address, SharedRuntime::exception_handler_for_return_address), eax);
    __ movl(ebx, eax);

    // setup eax & edx, remove return address & clear pending exception
    __ get_thread(ecx);
    __ popl(edx);
    __ movl(eax, Address(ecx, Thread::pending_exception_offset()));
    __ movl(Address(ecx, Thread::pending_exception_offset()), (int)NULL);

#ifdef ASSERT
    // make sure exception is set
    { Label L;
      __ testl(eax, eax);
      __ jcc(Assembler::notEqual, L);
      __ stop("StubRoutines::forward exception: no pending exception (2)");
      __ bind(L);
    }
#endif

    // continue at exception handler (return address removed)
    // eax: exception
    // ebx: exception handler
    // edx: throwing pc
    __ verify_oop(eax);
    __ jmp(ebx);

    return start;
  }
  

  //----------------------------------------------------------------------------------------------------
  // Support for jint atomic::exchange(jint exchange_value, jint* dest)
  // 
  // xchg exists as far back as 8086, lock needed for MP only
  // Stack layout immediately after call:
  //
  // 0 [ret addr ] <--- esp
  // 1 [  ex     ]
  // 2 [  dest   ] 
  //
  // Result:   *dest <- ex, return (old *dest)
  //
  // Note: win32 does not currently use this code

  address generate_atomic_exchange() {
    StubCodeMark mark(this, "StubRoutines", "atomic_exchange");
    address start = __ pc();

    __ pushl(edx);
    Address exchange(esp, 2 * wordSize);
    Address dest_addr(esp, 3 * wordSize);
    __ movl(eax, exchange);
    __ movl(edx, dest_addr);    
    __ xchg(eax, Address(edx, 0));
    __ popl(edx);
    __ ret(0);

    return start;
  }


  //----------------------------------------------------------------------------------------------------
  // Support for jint atomic::compare_and_exchange(jint exchange_value, jint* dest, jint compare_value)
  // 
  // TODO - cmpxchg does not exist prior to 486. 
  // Stack layout immediately after call:
  //
  // 0 [ret addr ] <--- esp
  // 1 [  ex     ]
  // 2 [  dest   ] 
  // 3 [  comp   ]
  //
  //  Result:
  //  If (comp == *dest) { *dest = ex} 
  //  return (old *dest)
  //
  // Note: win32 does not currently use this code

  address generate_atomic_compare_and_exchange() {
    StubCodeMark mark(this, "StubRoutines", "atomic_compare_and_exchange");
    address start = __ pc();

    __ pushl(edx);
    __ pushl(ebx);
    Address compare(esp, 5 * wordSize);
    Address dest(esp, 4 * wordSize);
    Address exchange(esp, 3 * wordSize);
    __ movl(eax, compare);
    __ movl(ebx, exchange);
    __ movl(edx, dest);
    if (AlwaysUseXchg || os::is_MP()) __ lock();
    __ cmpxchg(ebx, Address(edx,0));
    __ popl(ebx);
    __ popl(edx);
    __ ret(0);

    return start;
  }

  //
  // jlong atomic_compare_and_exchange_long(jlong exchange_value, jlong* dest, jlong compare_value)
  //
  address generate_atomic_compare_and_exchange_long() {
    StubCodeMark mark(this, "StubRoutines", "atomic_compare_and_exchange_long");
    address start = __ pc();

    __ pushl(ebx);
    __ pushl(edi);
    Address compare_hi(esp, 7 * wordSize);
    Address compare_lo(esp, 6 * wordSize);
    Address dest(esp, 5 * wordSize);
    Address destAddr(edi, 0);
    Address exchange_hi(esp, 4 * wordSize);
    Address exchange_lo(esp, 3 * wordSize);

    __ movl(edi, dest);
    __ movl(eax, compare_lo);
    __ movl(edx, compare_hi);
    __ movl(ebx, exchange_lo);
    __ movl(ecx, exchange_hi);
    if (AlwaysUseXchg || os::is_MP()) __ lock();
    __ cmpxchg8(destAddr);
    __ popl(edi);
    __ popl(ebx);
    __ ret(0);

    return start;
  }

  //----------------------------------------------------------------------------------------------------
  // Support for jint atomic::add(jint inc, jint* loc)
  // 
  // add exists as far back as 8086, lock needed for MP only
  // Stack layout immediately after call:
  //
  // 0 [ret addr ] <--- esp
  // 1 [  inc    ]
  // 2 [  loc    ] 
  //
  // Result:   return (*loc) += inc;
  //
  // Note: win32 does not currently use this code

  address generate_atomic_increment() {
    StubCodeMark mark(this, "StubRoutines", "atomic_increment");
    address start = __ pc();

    __ pushl(edx);
    __ pushl(ecx);
    Address inc(esp, 3 * wordSize);
    Address loc(esp, 4 * wordSize);
    __ movl(eax, inc);
    __ movl(edx, loc);
    __ movl(ecx, eax);
    if (AlwaysUseXchg || os::is_MP()) __ lock();
    __ xaddl(Address(edx, 0), eax);
    __ addl(eax, ecx);
    __ popl(ecx);
    __ popl(edx);
    __ ret(0);

    return start;
  }


  //----------------------------------------------------------------------------------------------------
  // Support for void atomic::membar().
  //
  // Note: win32 does not currently use this code

  address StubGenerator::generate_atomic_membar() {
    StubCodeMark mark(this, "StubRoutines", "atomic_membar");
    address start = __ pc();

    __ membar();
    __ ret(0);

    return start;
  }


  //----------------------------------------------------------------------------------------------------
  // Support for jint get_previous_fp()
  // 
  // This routine is used to find the previous frame pointer for
  // the caller (current_frame_guess). This is used as part of debugging
  // ps() is seemingly lost trying to find frames.
  // This code assumes that caller current_frame_guess) has a frame.

  address generate_get_previous_fp() {
    StubCodeMark mark(this, "StubRoutines", "get_previous_fp");
    const Address old_fp       (ebp,  0);
    const Address older_fp       (eax,  0);
    address start = __ pc();

    __ enter();    
    __ movl(eax, old_fp); // callers fp
    __ movl(eax, older_fp); // the frame for ps()
    __ popl(ebp);
    __ ret(0);

    return start;
  }
  

  //----------------------------------------------------------------------------------------------------
  // Support for address call_address()
  // 
  // This routine is used to find the address of the call instruction
  // used to call the current method.
  // The code assumes that the caller has a frame.

  address generate_call_address() {
    StubCodeMark mark(this, "StubRoutines", "call_address");
    const Address retaddr(ebp, 4);
    address start = __ pc();

    __ movl(eax, retaddr); // caller's return address
    __ subl(eax, NativeCall::instruction_size);
    __ ret(0);

    return start;
  }
  

  //---------------------------------------------------------------------------
  // Wrapper for slow-case handling of double-to-integer conversion 
  // d2i or f2i fast case failed either because it is nan or because
  // of under/overflow.
  // Input:  FPU TOS: float value
  // Output: eax (edx): integer (long) result

  address generate_d2i_wrapper( address fcn ) {
    StubCodeMark mark(this, "StubRoutines", "d2i_wrapper");
    address start = __ pc();

  // Capture info about frame layout
  enum layout { FPUState_off         = 0,
                ebp_off              = FPUStateSizeInWords,
                edi_off,         
                esi_off,
                ecx_off,
                ebx_off,
                saved_argument_off,
                saved_argument_off2, // 2nd half of double
	        framesize 
  };

  assert(FPUStateSizeInWords == 27, "update stack layout");

    // Save outgoing argument to stack across push_FPU_state()
    __ subl(esp, wordSize * 2);
    __ fstp_d(Address(esp));

    // Save CPU & FPU state
    __ pushl(ebx);
    __ pushl(ecx);
    __ pushl(esi);
    __ pushl(edi);
    __ pushl(ebp);
    __ push_FPU_state();

    // push_FPU_state() resets the FP top of stack 
    // Load original double into FP top of stack
    __ fld_d(Address(esp, saved_argument_off * wordSize));
    // Store double into stack as outgoing argument
    __ subl(esp, wordSize*2);
    __ fst_d(Address(esp));

    // Prepare FPU for doing math in C-land
    __ empty_FPU_stack();
    // Call the C code to massage the double.  Result in EAX
    __ call_VM_leaf( fcn, 2 );

    // Restore CPU & FPU state
    __ pop_FPU_state();
    __ popl(ebp);
    __ popl(edi);
    __ popl(esi);
    __ popl(ecx);
    __ popl(ebx);
    __ addl(esp, wordSize * 2);

    __ ret(0);

    return start;
  }


  //---------------------------------------------------------------------------
  // The following routine generates a subroutine to throw an asynchronous
  // UnknownError when an unsafe access gets a fault that could not be
  // reasonably prevented by the programmer.  (Example: SIGBUS/OBJERR.)
  address generate_handler_for_unsafe_access() {
    StubCodeMark mark(this, "StubRoutines", "handler_for_unsafe_access");
    address start = __ pc();

    __ pushl(0);                      // hole for return address-to-be
    __ pushad();                      // push registers
    Address next_pc(esp, RegisterImpl::number_of_registers * BytesPerWord);
    __ call(CAST_FROM_FN_PTR(address, handle_unsafe_access), relocInfo::runtime_call_type);
    __ movl(next_pc, eax);            // stuff next address 
    __ popad();
    __ ret(0);                        // jump to next address

    return start;
  }


  //----------------------------------------------------------------------------------------------------
  // Non-destructive plausibility checks for oops
  
  address generate_verify_oop() {
    StubCodeMark mark(this, "StubRoutines", "verify_oop");
    address start = __ pc();
    
    // Incoming arguments on stack after saving eax:
    //
    // [tos    ]: saved edx
    // [tos + 1]: return address
    // [tos + 2]: char* error message
    // [tos + 3]: oop   object to verify
    // [tos + 4]: saved eax - saved by caller and bashed
    
    Label exit, error;
    __ incl(Address((int)StubRoutines::verify_oop_count_addr(), relocInfo::none));
    __ pushl(edx);                               // save edx
    // make sure object is 'reasonable'
    __ movl(eax, Address(esp, 3 * wordSize));    // get object
    __ testl(eax, eax);
    __ jcc(Assembler::zero, exit);               // if obj is NULL it is ok
    
    // Check if the oop is in the right area of memory
    const int oop_mask = Universe::verify_oop_mask();
    const int oop_bits = Universe::verify_oop_bits();
    __ movl(edx, eax);
    __ andl(edx, oop_mask);
    __ cmpl(edx, oop_bits);
    __ jcc(Assembler::notZero, error);

    // make sure klass is 'reasonable'
    __ movl(eax, Address(eax, oopDesc::klass_offset_in_bytes())); // get klass
    __ testl(eax, eax);
    __ jcc(Assembler::zero, error);              // if klass is NULL it is broken

    // Check if the klass is in the right area of memory
    const int klass_mask = Universe::verify_klass_mask();
    const int klass_bits = Universe::verify_klass_bits();
    __ movl(edx, eax);
    __ andl(edx, klass_mask);
    __ cmpl(edx, klass_bits);
    __ jcc(Assembler::notZero, error);

    // make sure klass' klass is 'reasonable'
    __ movl(eax, Address(eax, oopDesc::klass_offset_in_bytes())); // get klass' klass
    __ testl(eax, eax);
    __ jcc(Assembler::zero, error);              // if klass' klass is NULL it is broken

    __ movl(edx, eax);
    __ andl(edx, klass_mask);
    __ cmpl(edx, klass_bits);
    __ jcc(Assembler::notZero, error);           // if klass not in right area
                                                 // of memory it is broken too.

    // return if everything seems ok
    __ bind(exit);
    __ movl(eax, Address(esp, 4 * wordSize));    // get saved eax back
    __ popl(edx);                                // restore edx
    __ ret(3 * wordSize);                        // pop arguments

    // handle errors
    __ bind(error);
    __ movl(eax, Address(esp, 4 * wordSize));    // get saved eax back
    __ popl(edx);                                // get saved edx back
    __ pushad();                                 // push registers (eip = return address & msg are already pushed)
    __ call(CAST_FROM_FN_PTR(address, MacroAssembler::debug), relocInfo::runtime_call_type);
    __ popad();
    __ ret(3 * wordSize);                        // pop arguments
    return start;
  }


  //----------------------------------------------------------------------------------------------------
  // The following routine generates a subroutine to throw a null pointer exception.
  // 
  //
  // Arguments :
  //
  //  return address in edx (call exception pc)
  //
  // Results:
  //
  //  throws a new exception object to the given return address

  // catch exception if receiver is NULL

  address generate_throw_null_exception() {
    StubCodeMark mark(this, "StubRoutines", "throw_null_exception");
    address start = __ pc();

    // generate NullException oop and compute the handler address 

    __ pushl (edx);    // return address for building last_java_fp/sp
    __ leal  (ebx, Address(esp, wordSize));
    __ call_VM (esi, ebx, CAST_FROM_FN_PTR(address, SharedRuntime::get_null_exception_entry_and_oop));
    // Result:
    //   esi: exception oop
    //   eax: handler address
    __ popl (edx);    // restore return address into pc

    __ movl (ecx, eax);
    __ movl (eax, esi);
    // eax is exception oop
    // edx is exception pc
    // ecx is handler address
    __ jmp  (ecx);

    return start;
  }


  //-----------------------------------------------------------------
  // Called when an interpreter call needs an i2c adapter before
  // continuing in the compiled callee. Thi sis typically called from a vtable
  //
  // 
  // ebx: methodOop
  // 
#ifndef CORE
  address generate_lazy_i2c_adapter_entry() {
    StubCodeMark mark(this, "StubRoutines", "lazy_i2c_adapter_entry");
    address start = __ pc();

    __ leal(eax, Address(esp, wordSize));
    __ call_VM(ebx, eax, CAST_FROM_FN_PTR(address, SharedRuntime::create_i2c_adapter), ebx);
    // eax: i2c adapter destination 
    // ebx: methodOop
    __ movl(ecx, eax);
    __ movl(eax, ebx);

    // must have methodOop in eax before entering the adapter
    __ jmp(ecx);

    return start;
  }
#endif


  //------------------------------------------------------------------------------------------------------------------------
  // Handler for implicit exceptions forwarded by the OS
  //
  // How it works: First we check if the exception happened in the interpreter.
  // If so, we jump directly to the corresponding interpreter entry point with
  // unchanged register contents. Otherwise, we check if a valid return address
  // is on the stack (that is the case if the exception happened in an inline
  // cache check). If not, we compute a valid fake return address and push it.
  // Afterwards we simply throw the corresponding exception via a VM call; i.e.,
  // from the Java execution point-of-view it looks like we called the VM to
  // explicitly throw the exception.
  //
  // Note: This solution automatically handles cases where the underlying
  //       nmethod (if any) is deoptimized, since VM calls must be able to
  //       handle such a situation anyway.

  address generate_handler_for_implicit_exception(address throw_interpreted, address throw_compiled, address* filter, bool is_implicit_null) {
    StubCodeMark mark(this, "StubRoutines", "handler_for_implicit_exception");
    address start = __ pc();

    // INTERPRETER CASE
    // register values as at the exception point (except for eip, of course)
    // decide if exception happened in interpreter
    __ pushad();                       // preserve registers (we don't care about flags)
    __ call_VM_leaf(CAST_FROM_FN_PTR(address, SharedRuntime::is_interpreter_exception));
    __ testl(eax, eax);                // true if exception happened in interpreter
#ifdef COMPILER2
    if (ProfileInterpreter && is_implicit_null) {
      Label L;

      __ jcc(Assembler::zero, L);
      __ get_thread(eax);
      __ movl(Address(eax, JavaThread::is_throwing_null_ptr_exception_offset() ), 1);
      __ popad();                        // restore registers (flags remain untouched)
      __ jmp(throw_interpreted, relocInfo::none);
      __ bind(L);
      __ popad();                        // restore registers (flags remain untouched)

    } else
#endif
    {
      __ popad();                        // restore registers (flags remain untouched)
      __ jcc(Assembler::notZero, throw_interpreted);
    }

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
#ifdef ASSERT
     { Label L;
       __ cmpl(Address((int)filter, relocInfo::none), 0);
       __ jcc(Assembler::notZero, L);
       __ stop("implicit exception filter not installed");
       __ bind(L);
     }
#endif
      __ call(Address((int)filter, relocInfo::none));
      __ should_not_reach_here();
#endif // COMPILER1

    // COMPILED CODE CASE - BOTH IMPLICIT AND INLINE-CACHE/VTABLE-STUB
    // exception happened outside of interpreter => compute exception
    // return address, if any.  Must not blow any registers; the implicit
    // exception acts as if it took a long-latency branch to the final target.
    { Label L;
      __ pushl(0);                      // hole for return address-to-be
      __ pushad();                      // preserve registers (we don't care about flags)
      if (is_implicit_null) {
        __ movl(eax, 1);
      } else {
        __ xorl(eax, eax);
      }
      // eax is flag saying if this is implicit null exception
      __ call_VM_leaf(CAST_FROM_FN_PTR(address, SharedRuntime::compute_exception_return_address), eax);
      __ movl(Address(esp, RegisterImpl::number_of_registers * BytesPerWord), eax); // stuff possible return address 
      __ testl(eax, eax);               // result was NULL
      __ popad();
      __ jcc(Assembler::zero, L);       // => return address already pushed (e.g., exception in method prolog)
      __ ret(0);

      __ bind(L);
      __ popl(eax);                     // tos pushed return address; not needed
    }

    // VTABLE-STUB OR INLINE_CACHE CASE
    if (throw_compiled == NULL) {
      // running with -Xint
      __ stop("wrong implicit exception");
    } else {
      __ jmp(throw_compiled, relocInfo::none);
      __ stop("ShouldNotReachHere: gen handler for implicit exception");
    }

    return start;
  }


  void create_control_words() {
    // Round to nearest, 53-bit mode, exceptions masked
    StubRoutines::_fpu_cntrl_wrd_std   = 0x027F;
    // Round to zero, 53-bit mode, exception mased
    StubRoutines::_fpu_cntrl_wrd_trunc = 0x0D7F;
    // Round to nearest, 24-bit mode, exceptions masked
    StubRoutines::_fpu_cntrl_wrd_24    = 0x007F;
    // Round to nearest, 64-bit mode, exceptions masked
    StubRoutines::_fpu_cntrl_wrd_64    = 0x037F;
    // Note: the following two constants are 80-bit values
    //       layout is critical for correct loading by FPU.
    // Bias for strict fp multiply/divide
    StubRoutines::_fpu_subnormal_bias1[0]= 0x00000000; // 2^(-15360) == 0x03ff 8000 0000 0000 0000
    StubRoutines::_fpu_subnormal_bias1[1]= 0x80000000;
    StubRoutines::_fpu_subnormal_bias1[2]= 0x03ff;
    // Un-Bias for strict fp multiply/divide
    StubRoutines::_fpu_subnormal_bias2[0]= 0x00000000; // 2^(+15360) == 0x7bff 8000 0000 0000 0000
    StubRoutines::_fpu_subnormal_bias2[1]= 0x80000000;
    StubRoutines::_fpu_subnormal_bias2[2]= 0x7bff;
  }

  //---------------------------------------------------------------------------
  // Initialization
  
  void generate_initial() {
    // Generates all stubs and initializes the entry points

    //------------------------------------------------------------------------------------------------------------------------
    // entry points that exist in all platforms
    // Note: This is code that could be shared among different platforms - however the benefit seems to be smaller than
    //       the disadvantage of having a much more complicated generator structure. See also comment in stubRoutines.hpp.

    // The following 2 routines must come first since their entry points are used later!
    StubRoutines::_forward_exception_entry                 = generate_forward_exception();    

    StubRoutines::_throw_null_exception_entry              = generate_throw_null_exception();

    StubRoutines::_call_stub_entry                         = generate_call_stub(StubRoutines::_call_stub_return_address);
    // is referenced by megamorphic call    
    StubRoutines::_catch_exception_entry                   = generate_catch_exception();    
    StubRoutines::_atomic_compare_and_exchange_long_entry    = generate_atomic_compare_and_exchange_long();

   // These are currently used by Solaris/Intel
   StubRoutines::_atomic_exchange_entry                    = generate_atomic_exchange();
   StubRoutines::_atomic_compare_and_exchange_entry        = generate_atomic_compare_and_exchange();
   StubRoutines::_atomic_increment_entry                   = generate_atomic_increment();
   StubRoutines::_atomic_exchange_ptr_entry                = StubRoutines::_atomic_exchange_entry;
   StubRoutines::_atomic_compare_and_exchange_ptr_entry    = StubRoutines::_atomic_compare_and_exchange_entry;
   StubRoutines::_atomic_increment_ptr_entry               = StubRoutines::_atomic_increment_entry;
   StubRoutines::_atomic_membar_entry                      = generate_atomic_membar();
   StubRoutines::_call_address_entry                       = generate_call_address();
  
   // platform dependent
   StubRoutines::i486::_handler_for_unsafe_access_entry    = generate_handler_for_unsafe_access();
   StubRoutines::i486::_get_previous_fp_entry              = generate_get_previous_fp();

   StubRoutines::_d2i_wrapper                              = generate_d2i_wrapper( CAST_FROM_FN_PTR(address, SharedRuntime::d2i) );
   StubRoutines::_d2l_wrapper                              = generate_d2i_wrapper( CAST_FROM_FN_PTR(address, SharedRuntime::d2l) );
   create_control_words();

   COMPILER2_ONLY(StubRoutines::_lazy_i2c_adapter_entry    = generate_lazy_i2c_adapter_entry();)
  }


  void generate_all() {
    // Generates all stubs and initializes the entry points
    
    //------------------------------------------------------------------------------------------------------------------------
    // entry points that are platform specific  

    StubRoutines::i486::_handler_for_divide_by_zero_entry  =
      generate_handler_for_implicit_exception(
        Interpreter::throw_ArithmeticException_entry(),
        NOT_COMPILER2(NULL) COMPILER2_ONLY(OptoRuntime::throw_div0_exception_stub()),
        &SharedRuntime::_implicit_div0_exception_filter,
        false
      );

    StubRoutines::i486::_handler_for_null_exception_entry  =
      generate_handler_for_implicit_exception(
        Interpreter::throw_NullPointerException_entry(),
        NOT_COMPILER2(NULL) COMPILER2_ONLY(OptoRuntime::throw_null_exception_stub()),
        &SharedRuntime::_implicit_null_exception_filter,
        true
      );

    StubRoutines::i486::_handler_for_stack_overflow_entry  =
      generate_handler_for_implicit_exception(
        Interpreter::throw_StackOverflowError_entry(),
        NOT_COMPILER2(NULL) COMPILER2_ONLY(OptoRuntime::throw_stack_overflow_error_stub()), 
        &SharedRuntime::_stack_ovrflow_exception_filter,
        false
      );

    // support for verify_oop (must happen after universe_init)
    StubRoutines::_verify_oop_subroutine_entry	   = generate_verify_oop();
  }


 public:
  StubGenerator(CodeBuffer* code, bool all) : StubCodeGenerator(code) { 
    if (all) {
      generate_all();
    } else {
      generate_initial();
    }
  }
}; // end class declaration


void StubGenerator_generate(CodeBuffer* code, bool all) {
  StubGenerator g(code, all);
}


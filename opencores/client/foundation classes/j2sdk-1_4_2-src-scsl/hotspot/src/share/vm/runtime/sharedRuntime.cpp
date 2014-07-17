#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)sharedRuntime.cpp	1.297 03/02/28 16:51:01 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_sharedRuntime.cpp.incl"
#include <math.h>


// Implementation of SharedRuntime

SharedRuntime::StrictMathFunction_DD_D SharedRuntime::java_lang_strict_math_pow = NULL;

address SharedRuntime::_implicit_null_exception_filter = NULL;
address SharedRuntime::_implicit_div0_exception_filter = NULL;
address SharedRuntime::_stack_ovrflow_exception_filter = NULL;


void SharedRuntime::lookup_function_DD_D(StrictMathFunction_DD_D& f, const char* fname) {
  if (f == NULL) f = CAST_TO_FN_PTR(StrictMathFunction_DD_D, NativeLookup::base_library_lookup("java/lang/StrictMath", fname, "(DD)D"));
  assert(f != NULL, "lookup failed");
}

void SharedRuntime::initialize_StrictMath_entry_points() {
  assert(!Universe::is_jdk12x_version(), "entry points do not exist for JDK 1.2");
  lookup_function_DD_D(java_lang_strict_math_pow, "pow");
}

JRT_LEAF(jdouble, SharedRuntime::dpow(jdouble x, jdouble y))
  assert(java_lang_strict_math_pow != NULL, "pow entry point must exist");
  return (*java_lang_strict_math_pow)(NULL, NULL, x, y);
JRT_END


JRT_LEAF(jlong, SharedRuntime::lmul(jlong y, jlong x))
  return x * y;
JRT_END


JRT_LEAF(jlong, SharedRuntime::ldiv(jlong y, jlong x))
  if (x == min_jlong && y == CONST64(-1)) {
    return x;
  } else {
    return x / y;
  }
JRT_END


JRT_LEAF(jlong, SharedRuntime::lrem(jlong y, jlong x))
  if (x == min_jlong && y == CONST64(-1)) {
    return 0;
  } else {
    return x % y;
  }
JRT_END


JRT_LEAF(jfloat, SharedRuntime::frem(jfloat  x, jfloat  y))
  return ((jfloat)fmod((double)x,(double)y));
JRT_END


JRT_LEAF(jdouble, SharedRuntime::drem(jdouble x, jdouble y))
  return ((jdouble)fmod((double)x,(double)y));
JRT_END


JRT_LEAF(jint, SharedRuntime::f2i(jfloat  x))
  if (g_isnan(x)) {return 0;}
  jlong lltmp = (jlong)x;
  jint ltmp   = (jint)lltmp;
  if (ltmp == lltmp) {
    return ltmp;
  } else {
    if (x < 0) {
      return min_jint;
    } else {
      return max_jint;
    }
  }
JRT_END


JRT_LEAF(jlong, SharedRuntime::f2l(jfloat  x))  
  if (g_isnan(x)) {return 0;}
  jlong lltmp = (jlong)x;
  if (lltmp != min_jlong) {
    return lltmp;
  } else {
    if (x < 0) {
      return min_jlong;
    } else {
      return max_jlong;
    }
  }
JRT_END


JRT_LEAF(jint, SharedRuntime::d2i(jdouble x))
  if (g_isnan(x)) {return 0;}
  jlong lltmp = (jlong)x;
  jint ltmp   = (jint)lltmp;
  if (ltmp == lltmp) {
    return ltmp;
  } else {
    if (x < 0) {
      return min_jint;
    } else {
      return max_jint;
    }
  }
JRT_END


JRT_LEAF(jlong, SharedRuntime::d2l(jdouble x))
  if (g_isnan(x)) {return 0;}
  jlong lltmp = (jlong)x;
  if (lltmp != min_jlong) {
    return lltmp;
  } else {
    if (x < 0) {
      return min_jlong;
    } else {
      return max_jlong;
    }
  }
JRT_END


JRT_LEAF(jfloat, SharedRuntime::d2f(jdouble x))
  return (jfloat)x;
JRT_END


JRT_LEAF(jfloat, SharedRuntime::l2f(jlong x))
  return (jfloat)x;
JRT_END


JRT_LEAF(jdouble, SharedRuntime::l2d(jlong x))
  return (jdouble)x;
JRT_END

// Exception handling accross interpreter/compiler boundaries
//
// exception_handler_for_return_address(...) returns the continuation address.
// The continuation address is the entry point of the exception handler of the
// previous frame depending on the return address.

address SharedRuntime::raw_exception_handler_for_return_address(address return_address) {


  assert(frame::verify_return_pc(return_address), "must be a return pc");

#ifdef COMPILER1
  // the fastest case first
  CodeBlob* blob = CodeCache::find_blob(return_address);
  if (blob != NULL && blob->is_nmethod()) {
    nmethod* code = (nmethod*)blob;
    assert(code != NULL, "nmethod must be present");
    assert(code->header_begin() != code->exception_begin(), "no exception handler");
    return code->exception_begin();
  } 
#endif // COMPILER1

  
  // Entry code
  if (StubRoutines::returns_to_call_stub(return_address)) {
    return StubRoutines::catch_exception_entry();
  }
  // Interpreted code
  if (Interpreter::contains(return_address)) {
    return Interpreter::rethrow_exception_entry();
  }
#ifdef COMPILER1
  // Handle runtime stubs before compiled methods

  // OSR adapter
  if (Runtime1::returns_to_osr_adapter(return_address)) {
    return Runtime1::entry_for(Runtime1::osr_unwind_exception_id);
  }

  // alignment adapter
  if (Runtime1::entry_for(Runtime1::alignment_frame_return_id) == return_address) {
    return Runtime1::entry_for(Runtime1::unwind_exception_id);
  }

  // Deoptimized frames
  DeoptimizationBlob* deopt_blob = (DeoptimizationBlob*) Runtime1::blob_for(Runtime1::deoptimization_handler_id);
  if (deopt_blob != NULL) {
    if (deopt_blob->exception_address_is_unpack_entry(return_address)) {
      return deopt_blob->unpack_with_exception();
    }
  }
#endif // COMPILER1
#ifdef COMPILER2
  // Unpacking code for deoptimized frames
  if (OptoRuntime::deoptimization_blob() != NULL) {
    if (OptoRuntime::deoptimization_blob()->exception_address_is_unpack_entry(return_address)) {
      return OptoRuntime::deoptimization_blob()->unpack_with_exception();
    }
  }
#endif // COMPILER2
#ifndef CORE
  // Compiled code
  if (CodeCache::contains(return_address)) {
    CodeBlob* blob = CodeCache::find_blob(return_address);
    if (blob->is_nmethod()) {
      nmethod* code = (nmethod*)blob;
      assert(code != NULL, "nmethod must be present");
      assert(code->header_begin() != code->exception_begin(), "no exception handler");
      return code->exception_begin();
    } 
    if (blob->is_runtime_stub()) {
      ShouldNotReachHere();   // callers are responsible for skipping runtime stub frames
    }
#ifdef COMPILER2
    if (blob->is_c2i_adapter()) {
      return OptoRuntime::exception_blob()->instructions_begin();
    } 
    if (blob->is_i2c_adapter()) {
      return OptoRuntime::exception_blob()->instructions_begin();
    } 
    if (blob->is_osr_adapter()) {
      return OptoRuntime::exception_blob()->instructions_begin();
    }
#endif // COMPILER2
  }
  guarantee(!VtableStubs::contains(return_address), "NULL exceptions in vtables should have been handled already!");
#endif // CORE
#ifndef PRODUCT
  { ResourceMark rm;
    tty->print_cr("No exception handler found for exception at " INTPTR_FORMAT " - potential problems:", return_address);
    tty->print_cr("a) exception happened in (new?) code stubs/buffers that is not handled here");
    tty->print_cr("b) should have called exception_handler_for_address instead");
    tty->print_cr("c) other problem");
  }
#endif // PRODUCT
  ShouldNotReachHere();
  return NULL;
}


JRT_LEAF(address, SharedRuntime::exception_handler_for_return_address(address return_address))
  JavaThread* thread = JavaThread::current();
#ifndef CORE
  address ra = thread->safepoint_state()->compute_adjusted_pc(return_address);  
#else
  address ra = return_address;  
#endif
  return raw_exception_handler_for_return_address(ra);
JRT_END


oop SharedRuntime::retrieve_receiver( symbolHandle sig, frame caller ) {
  assert(caller.is_interpreted_frame(), "");
  int args_size = ArgumentSizeComputer(sig).size() + 1;
  assert(args_size <= caller.interpreter_frame_expression_stack_size(), "receiver must be on interpreter stack");
  oop result = (oop) caller.interpreter_frame_tos_at(args_size - 1);
  assert(Universe::heap()->is_in(result) && result->is_oop(), "receiver must be an oop");
  return result;
}

JRT_ENTRY(address, SharedRuntime::create_i2c_adapter(JavaThread* thread, methodOop method))
  COMPILER1_ONLY(ShouldNotReachHere();) // no adapters with C1
  if (TraceAdapterGeneration && Verbose) tty->print_cr("create_i2c_adapter");
#ifndef CORE
  methodHandle calleeMethod (thread,  method);
  // Verify that frame and arguments are correct
  debug_only(verify_caller_frame(thread->last_frame(), calleeMethod));  

  address i2c_entry = calleeMethod->code()->interpreter_entry_point();
  thread->set_vm_result(calleeMethod());
  return i2c_entry;
#else
  ShouldNotReachHere();
  return NULL;
#endif
JRT_END



JRT_LEAF(int, SharedRuntime::is_interpreter_exception())
  // Caution: This routine must be a leaf routine (no GCs. no locking!)
  JavaThread* thread = JavaThread::current();
  address pc = thread->saved_exception_pc();
  return (int)Interpreter::contains(pc);
JRT_END

// Method needs to be cleaned up !!
// ifdef CORE does not belong to sharedRuntime.cpp; the method has been
// designed to handle only implicit null exceptions, but there are two more
// implicit exception types (arithmetic and stack overflow)
// The method contains a cumbersome fix by using the flag is_implicit_null
JRT_LEAF(address, SharedRuntime::compute_exception_return_address(jint is_implicit_null))
#ifndef COMPILER2
  ShouldNotReachHere();
  return NULL;
#else   
  // Compute the (corrected) issuing pc for an exception pc.
  // Caution: This routine must be a leaf routine (no GCs. no locking!)
  //          because we are in the middle of exception resolving.
  //          before GC can happen we have to set the exception pc 
  //          correctly.
  //
  // Note: The pc is __not__ a conventional next_pc value (as from
  // frame::pc), but the real address of the trapping instruction.
  
  // compute the corrected pc (if the exception happened in a temporary code
  // buffer used for safepoint synchronization, we need to adjust it first)
  assert(!(bool)is_interpreter_exception(), "interpreter exceptions must be handled already");
  JavaThread* thread = JavaThread::current();
  address pc = thread->safepoint_state()->compute_adjusted_pc(thread->saved_exception_pc());
 
  if (VtableStubs::contains(pc)) {
    // exception happened in vtable stub
    // => must be a NULL receiver => use return address
    // pushed by caller => don't push another return address
    if (TraceExceptions) {
      if (is_implicit_null) {
	tty->print_cr("Implicit NULL in VtableStub at " INTPTR_FORMAT, pc);
      } else {
	tty->print_cr("Implicit exception in VtableStub at " INTPTR_FORMAT, pc);
      }
    }
    return NULL;
  }

  CodeBlob* cb = CodeCache::find_blob(pc);

  guarantee(cb != NULL, "exception happened outside interpreter, nmethods and vtable stubs (1)");
  
  // Exception happened in CodeCache. Must be either:
  // 1. Inline-cache check in C2I adapter,
  // 2. Inline-cache check in nmethod, or
  // 3. Implict null exception in nmethod, or
  // 4. Divide-by-zero in nmethod, or
  // 5. Stack overflow just about anywhere.

  if (!cb->is_nmethod()) {
    bool is_stack_overflow = !is_implicit_null;

    // (Note:  This path would also be taken for divide-by-zero
    // exceptions, if they could possibly arise from adapters.
    // But they cannot.)

    guarantee(is_stack_overflow ||
	      (cb->is_c2i_adapter() && ((C2IAdapter*)cb)->inlinecache_check_contains(pc)),
	      "exception happened outside interpreter, nmethods and vtable stubs (2)");

    // There is no handler here, so we will simply unwind.
    return NULL;
  }
  // Otherwise, it's an nmethod.  Consult its exception handlers.
    
  // compute return address if needed    
  nmethod* nm = (nmethod*)cb;

  // exception happened inside an nmethod
  if (nm->inlinecache_check_contains(pc)) {
    // exception happened inside inline-cache check code
    // => the nmethod is not yet active (i.e., the frame
    // is not set up yet) => use return address pushed by
    // caller => don't push another return address
    return NULL;
  }
  
  if (is_implicit_null == 1) {
#ifndef PRODUCT
    extern int implicit_null_throws;
    implicit_null_throws++;
#endif
    // Exception happened outside inline-cache check code => we are inside
    // an active nmethod => use cpc to determine a return address
    int exception_offset = pc - nm->instructions_begin();
    int cont_offset = ImplicitNullCheckTable(nm).at( exception_offset );
#ifdef ASSERT
    if (cont_offset == 0) {
      ResetNoHandleMark rnm; // Might be called from LEAF/QUICK ENTRY
      HandleMark hm(thread);
      ResourceMark rm(thread);
      CodeBlob* cb = CodeCache::find_blob(pc);
      assert(cb != NULL, "");
      tty->print_cr("implicit exception happened at " INTPTR_FORMAT, pc);
      cb->print();
      if (cb->is_nmethod()) {
        ((nmethod*)cb)->method()->print_codes();
        ((nmethod*)cb)->print_code();
        ((nmethod*)cb)->print_pcs();
      }
    }
#endif
    guarantee(cont_offset != 0, "null exception in compiled code");
    int offset_from_exception_pc = cont_offset - exception_offset;
    address target_pc  = thread->saved_exception_pc() + offset_from_exception_pc;
  
    // A codebuffer for this exception might have been created while inside the exception
    // handler.  (The OS might lie about the exact pc for a thread.)  In this case,
    // we make sure to redirect the target pc into this codebuffer.
    target_pc = thread->safepoint_state()->maybe_capture_pc(target_pc);
    assert(thread->safepoint_state()->code_buffer() != NULL || CodeCache::contains(target_pc), "bad target_pc (1)");
    assert(thread->safepoint_state()->code_buffer() == NULL || thread->safepoint_state()->code_buffer()->contains(target_pc), "bad target_pc (2)");

    Events::log("Implicit null exception at " INTPTR_FORMAT " to " INTPTR_FORMAT, thread->saved_exception_pc(), target_pc);
    return target_pc;       
  } else {
    return NULL;
  }
#endif // COMPILER2
JRT_END


JRT_ENTRY(address, SharedRuntime::handle_array_index_check(JavaThread* thread, jint index))
  // Note: method returns an oop through the thread local storage  
  address pc = thread->last_frame().pc();
  address handler_address = raw_exception_handler_for_return_address(pc);

  char buf[40];
  sprintf(buf, "%d", index);
  Handle exception = Exceptions::new_exception(thread, vmSymbols::java_lang_ArrayIndexOutOfBoundsException(), buf);
  thread->set_vm_result(exception()); 
  return handler_address;
JRT_END


JNI_ENTRY(void, throw_unsatisfied_link_error(JNIEnv* env, ...))
{
  THROW(vmSymbols::java_lang_UnsatisfiedLinkError());
}
JNI_END


address SharedRuntime::native_method_throw_unsatisfied_link_error_entry() {
  return CAST_FROM_FN_PTR(address, &throw_unsatisfied_link_error);
}


JRT_ENTRY(int, SharedRuntime::trace_bytecode(JavaThread* thread, intptr_t preserve_this_value, intptr_t tos, intptr_t tos2))
  const frame f = thread->last_frame();
  assert(f.is_interpreted_frame(), "must be an interpreted frame");
  BytecodeTracer::trace(f.interpreter_frame_method(), f.interpreter_frame_bcp(), tos, tos2);
  if (EnableJVMPIInstructionStartEvent && jvmpi::is_event_enabled(JVMPI_EVENT_INSTRUCTION_START)) {
    jvmpi::post_instruction_start_event(f);
  }
  return preserve_this_value;
JRT_END


JRT_ENTRY(void, SharedRuntime::yield_all(JavaThread* thread, int attempts))
  os::yield_all(attempts);
JRT_END

// returns NULL exception oop and sets the correct handler_entry for given return address
JRT_ENTRY(address, SharedRuntime::get_null_exception_entry_and_oop (JavaThread *thread))
  address exc_start = raw_exception_handler_for_return_address(thread->last_frame().pc());
  Handle exception = Exceptions::new_exception(thread, vmSymbols::java_lang_NullPointerException(), NULL);    
  thread->set_vm_result(exception());
  return exc_start;
JRT_END

//------ simple_array_copy helpers--------

inline bool check_length_of_type_array(oop arr, int start, int copy_lng) {
  assert (arr->is_array(), "");
  int array_length = arrayOop(arr)->length();
  return (unsigned int) array_length >= (unsigned int)start + (unsigned int)copy_lng;
}


// returns T_VOID if cannot handle the array
inline BasicType get_array_klass_type(klassOop klass) {
  Klass* k = klass->klass_part();
  if (k->oop_is_objArray()) {
    return T_OBJECT; // disable fast object copying (need to set RememberedSet)
  } else {
    if (k->oop_is_typeArray()) {
      return ((typeArrayKlass*)k)->type();
    } else if (k->oop_is_symbol()) {
      return T_CHAR;
    }
  }
  return T_VOID;
}


// we assume that all necessary checks have been done in compiled code already
JRT_LEAF(int, SharedRuntime::simple_array_copy_no_checks(int length, int dst_pos, oop dst, int src_pos, oop src))
#ifdef CORE
  ShouldNotReachHere();
  return 0; 
#else
#ifdef COMPILER2
  ShouldNotReachHere();
  return 0; 
#else
  if (length == 0) return 0; // done
  assert(src != NULL && dst != NULL, "illegal arrays");
  assert(check_length_of_type_array(dst, dst_pos, length) && check_length_of_type_array(src, src_pos, length), "wrong parameters");
  assert(dst->is_array() && src->is_array(), "");
  BasicType type = get_array_klass_type(src->klass()); // this checks also if the oops are arrays
  if (type == T_OBJECT) {
    klassOop bound = objArrayKlass::cast(dst->klass())->element_klass();
    klassOop stype = objArrayKlass::cast(src->klass())->element_klass();
    if (stype == bound || Klass::cast(stype)->is_subtype_of(bound)) {
      oop* src_addr = objArrayOop(src)->obj_at_addr(src_pos);
      oop* dst_addr = objArrayOop(dst)->obj_at_addr(dst_pos);
      // elements are guaranteed to be subtypes, so no check necessary
      assert(length > 0, "sanity check");
      Memory::copy_oops_overlapping_atomic(src_addr, dst_addr, length);
      BarrierSet* bs = Universe::heap()->barrier_set();
      assert(bs->has_write_ref_array_opt(), "assuming this for now");
      bs->write_ref_array(MemRegion((HeapWord*)dst_addr, (HeapWord*)(dst_addr + length)));
      return 0;
    } 
  } else if (type != T_VOID) {
    if (src->klass() != dst->klass()) {
      // this can happen for symbol vs char array...
      BasicType type_2 = get_array_klass_type(dst->klass());
      if (type_2 != type) return -1;
    }
    // do the copy
    unsigned int bytes_per_elem = type2aelembytes[type];
    char* src_a = (char*) arrayOop(src)->base(type) + (src_pos * bytes_per_elem);
    char* dst_a = (char*) arrayOop(dst)->base(type) + (dst_pos * bytes_per_elem);
    unsigned int nof_bytes = length * bytes_per_elem;
    // Potential problem: memmove is not guaranteed to be word atomic
    // Revisit in Merlin
    memmove(dst_a, src_a, nof_bytes); // use memmove as the areas may overlap
    return 0;      
  }
  return -1; // array was not copied
#endif
#endif
JRT_END


// ---------------------------------------------------------------------------------------------------------
// Non-product code
#ifndef PRODUCT

void SharedRuntime::verify_caller_frame(frame caller_frame, methodHandle callee_method) {
  ResourceMark rm;  
  assert (caller_frame.is_interpreted_frame(), "sanity check");
  assert (callee_method->has_compiled_code(), "callee must be compiled");  
  methodHandle caller_method (Thread::current(), caller_frame.interpreter_frame_method());
  jint bci = caller_frame.interpreter_frame_bci();
  methodHandle method = find_callee_method_inside_interpreter(caller_frame, caller_method, bci);
  assert (callee_method == method, "incorrect method");
}

methodHandle SharedRuntime::find_callee_method_inside_interpreter(frame caller_frame, methodHandle caller_method, int bci) {
  EXCEPTION_MARK;
  Bytecode_invoke* bytecode = Bytecode_invoke_at(caller_method, bci);
  methodHandle staticCallee = bytecode->static_target(CATCH); // Non-product code
  
  bytecode = Bytecode_invoke_at(caller_method, bci);
  int bytecode_index = bytecode->index();
  Bytecodes::Code bc = bytecode->adjusted_invoke_code();      

  Handle receiver;
  if (bc == Bytecodes::_invokeinterface ||
      bc == Bytecodes::_invokevirtual ||
      bc == Bytecodes::_invokespecial) {
    symbolHandle signature (THREAD, staticCallee->signature());
    receiver = Handle(THREAD, retrieve_receiver(signature, caller_frame));
  } else {
    receiver = Handle();
  }
  CallInfo result;
  constantPoolHandle constants (THREAD, caller_method->constants());
  LinkResolver::resolve_invoke(result, receiver, constants, bytecode_index, bc, CATCH); // Non-product code
  methodHandle calleeMethod = result.selected_method();
  return calleeMethod;
}

#endif 

// This is factored because it is also called from Runtime1.
void SharedRuntime::jvmpi_method_entry_work(JavaThread* thread, methodOop method, oop receiver) {
  GC_locker::lock();
  if (jvmpi::is_event_enabled(JVMPI_EVENT_METHOD_ENTRY2)) {
    jvmpi::post_method_entry2_event(method, receiver);
  } 
  if (jvmpi::is_event_enabled(JVMPI_EVENT_METHOD_ENTRY)) {
    jvmpi::post_method_entry_event(method);
  } 
  GC_locker::unlock();
}


// Must be entry as it may lock when acquring the jni_id of the method
JRT_ENTRY (void,  SharedRuntime::jvmpi_method_entry(JavaThread* thread, methodOop method, oop receiver))
  jvmpi_method_entry_work(thread, method, receiver);
JRT_END


// Must be entry as it may lock when acquring the jni_id of the method
JRT_ENTRY (void, SharedRuntime::jvmpi_method_exit(JavaThread* thread, methodOop method))
#ifdef COMPILER1
#ifdef ASSERT
  frame stub_fr = thread->last_frame();
  CodeBlob* stub_cb = CodeCache::find_blob(stub_fr.pc());  
  bool stub_is_jvmpi_method_exit = stub_cb != NULL &&
                                   stub_cb->instructions_begin() ==
                                   Runtime1::entry_for(Runtime1::jvmpi_method_exit_id);
  if (stub_is_jvmpi_method_exit) {
    CodeBlob* caller = CodeCache::find_blob(stub_fr.frameless_stub_return_addr());
    assert(caller != NULL && caller->is_nmethod(), "should only be called from an nmethod");
  }
#endif
#endif
  GC_locker::lock();
  if (jvmpi::is_event_enabled(JVMPI_EVENT_METHOD_EXIT)) {
    jvmpi::post_method_exit_event(method);
  }
  GC_locker::unlock();
JRT_END


#ifndef CORE

// Resets a call-site in compiled code so it will get resolved again. This routines handle 
// both virtual call sites, optimized virual call sites, and static call sites. Typically used
// to change a call sites destination from interpreter to compiled. The 'caller' frame's pc must
// point after call to update
void SharedRuntime::reresolve_call_site(frame caller) {  
  assert(caller.is_compiled_frame(), "must be compiled");     
  address pc = caller.pc();
  Events::log("update call-site at pc %#x", pc);
  
  address call_addr;
  {
    // Get call instruction under lock because another thread may be
    // busy patching it.
    MutexLockerEx ml_patch(Patching_lock, Mutex::_no_safepoint_check_flag);
    // Location of call instruction 
    NativeCall *ncall = nativeCall_before(pc);
    call_addr = ncall->instruction_address();   
  }


  // Check for static or virtual call       
  bool is_static_call = false;
  { nmethod* caller_nm = CodeCache::find_nmethod(pc);     
    RelocIterator iter(caller_nm, call_addr, call_addr+1);
    int ret = iter.next(); // Get item
    assert(ret == 1, "relocation info. must exist for this address");
    assert(iter.addr() == call_addr, "sanity check");
#ifdef COMPILER1
    if (iter.type() == relocInfo::static_call_type || iter.type() == relocInfo::opt_virtual_call_type) {
      is_static_call = true; 
    } else {
      assert(iter.type() == relocInfo::virtual_call_type 
            , "unexpected relocInfo. type");
    }
#else
    if (iter.type() == relocInfo::static_call_type) {
      is_static_call = true; 
    } else {
      assert(iter.type() == relocInfo::virtual_call_type ||
             iter.type() == relocInfo::opt_virtual_call_type
            , "unexpected relocInfo. type");
    }
#endif
  }

  // Cleaning the inline cache will force a new resolve. This is more robust than 
  // directly setting it to the new destination, since resolving of calls is always
  // done through the same code path. (experience shows that it leads to very hard to
  // track down bugs, if an inline cache gets update to a wrong method). It should not be 
  // performance critical, since the resolve is only done once.
  
  MutexLocker ml(CompiledIC_lock);    
  // Check if deoptimization happend while we were waiting for the lock.   
  if (!caller.is_deoptimized_frame()) {     
    if (is_static_call) {         
      CompiledStaticCall* ssc= compiledStaticCall_at(call_addr);
      ssc->set_to_clean();
    } else {
      // compiled, dispatched call (which used to call an interpreted method)      
      CompiledIC* inline_cache = CompiledIC_at(call_addr);
      inline_cache->set_to_clean();
    }
  }
}
#endif // !CORE


JRT_LEAF(void, SharedRuntime::reguard_yellow_pages())
  (void) JavaThread::current()->reguard_stack();
JRT_END



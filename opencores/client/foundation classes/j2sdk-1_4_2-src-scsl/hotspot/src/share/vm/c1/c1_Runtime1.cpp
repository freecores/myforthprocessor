#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)c1_Runtime1.cpp	1.181 03/02/05 11:12:26 JVM"
#endif
//
// Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
//

#include "incls/_precompiled.incl"
#include "incls/_c1_Runtime1.cpp.incl"


// Implementation of StubAssembler

StubAssembler::StubAssembler(CodeBuffer* code) : C1_MacroAssembler(code) {
  _name = NULL;
  _must_gc_arguments = false;
}


void StubAssembler::set_info(const char* name, bool must_gc_arguments) {
  _name = name;
  _must_gc_arguments = must_gc_arguments;
}


// Implementation of iEntries

iEntries::iEntries() {
  _static_call_offset    = 0;
  _optimized_call_offset = 0;
  _virtual_call_offset   = 0;
  _virtual_final_call_offset  = 0;
}


iEntries::iEntries(int static_call_offset, int optimized_call_offset, int virtual_call_offset, int virtual_final_call_offset) {
  _static_call_offset    = static_call_offset;
  _optimized_call_offset = optimized_call_offset;
  _virtual_call_offset   = virtual_call_offset;
  _virtual_final_call_offset  = virtual_final_call_offset;
}


void iEntries::set_base(address base) {
  assert(base != NULL, "base must exist");
  _base = base;
}


// Implementation of Runtime1

bool      Runtime1::_is_initialized = false;
CodeBlob* Runtime1::_blobs[Runtime1::number_of_ids];
const char *Runtime1::_blob_names[] = {
  RUNTIME1_STUBS(STUB_NAME, LAST_STUB_NAME)
};
iEntries  Runtime1::_ientries[AbstractInterpreter::number_of_method_entries];

#ifndef PRODUCT
// statistics
int Runtime1::_resolve_invoke_cnt = 0;
int Runtime1::_handle_wrong_method_cnt = 0;
int Runtime1::_ic_miss_cnt = 0;
int Runtime1::_checkcast_cnt = 0;
int Runtime1::_storecheck_cnt = 0;
#endif

int Runtime1::_code_buffer_size = 0;
int Runtime1::_locs_buffer_size = 0;
BufferBlob* Runtime1::_code_buffer      = NULL;
address Runtime1::_locs_buffer      = NULL;


CodeBuffer* Runtime1::new_code_buffer() {
  CodeBuffer* code = new CodeBuffer(_code_buffer_size, _locs_buffer_size, 0, 0, 0, true,
                                    _code_buffer, (relocInfo *)_locs_buffer, NULL, false, 
                                    NULL, "Compiler1 temporary CodeBuffer");
  // Allocate code buffer space only once
  if (_code_buffer == NULL ) _code_buffer = code->get_blob();
  return code;
}


void Runtime1::generate_blob_for(StubID id) {
  assert(0 <= id && id < number_of_ids, "illegal stub id");
  ResourceMark rm;
  // create code buffer for code storage
#ifdef _LP64
  int code_size = CodeBuffer::insts_memory_size(VerifyOops ? 11*K : 7*K);
#else
  int code_size = CodeBuffer::insts_memory_size(VerifyOops ? 8*K : 4*K);
#endif
  int locs_size = CodeBuffer::locs_memory_size(1*K);

  CodeBuffer* code = new_code_buffer();

  // create assembler for code generation
  StubAssembler* sasm = new StubAssembler(code);
  // generate code for runtime stub
  int frame_size, unpack_exception_offset, unpack_reexecute_offset; // for deoptimization blob
  OopMapSet* oop_maps;
  oop_maps = generate_code_for(id, sasm, &unpack_exception_offset, &frame_size, &unpack_reexecute_offset);
  // align so printing shows nop's instead of random code at the end (SimpleStubs are aligned)
  sasm->align(BytesPerWord);
  // make sure all code is in code buffer
  sasm->flush();
  // create blob - distinguish a few special cases
  CodeBlob* blob = NULL;
  switch (id) {
    case illegal_instruction_handler_id:
      blob = SafepointBlob::create(code, oop_maps, no_frame_size);
      break;

    case deoptimization_handler_id:
      blob = DeoptimizationBlob::create(code, oop_maps, 0, unpack_exception_offset, unpack_reexecute_offset, frame_size);
      break;

    default:
      {
        RuntimeStub* stub = RuntimeStub::new_runtime_stub(name_for(id), code, no_frame_size, oop_maps, sasm->must_gc_arguments());
        blob = stub;
      }
      break;
  }
  // install blob
  assert(blob != NULL, "blob must exist");
  _blobs[id] = blob;
}


void Runtime1::initialize() {
  // Warning: If we have more than one compilation running in parallel, we
  //          need a lock here with the current setup (lazy initialization).
  if (!is_initialized()) {
    _is_initialized = true;

    // setup CodeBuffer
    assert(_code_buffer == NULL, "initialization should happen only once");
    _code_buffer_size = CodeBuffer::insts_memory_size(desired_max_code_buffer_size);
    _locs_buffer_size = CodeBuffer::locs_memory_size (desired_max_locs_buffer_size);
    _code_buffer = NULL;
    _locs_buffer = NEW_C_HEAP_ARRAY(u_char, _locs_buffer_size);

    // platform-dependent initialization
    initialize_pd();
    // generate stubs
    for (int id = 0; id < number_of_ids; id++) generate_blob_for((StubID)id);
    // 'relocate' interpreter entries
    address base = blob_for(interpreter_entries_id)->instructions_begin();
    for (int j = 0; j < AbstractInterpreter::number_of_method_entries; j++) {
      _ientries[j].set_base(base);
    }
    // printing
    if (PrintSimpleStubs) {
      tty->print_cr("PrintSimpleStub is not implemented");
    }
  }
}


CodeBlob* Runtime1::blob_for(StubID id) {
  assert(0 <= id && id < number_of_ids, "illegal stub id");
  if (!is_initialized()) initialize();
  return _blobs[id];
}


const char* Runtime1::name_for(StubID id) {
  assert(0 <= id && id < number_of_ids, "illegal stub id");
  return _blob_names[id];
}


iEntries* Runtime1::ientries_for(methodHandle m) {
  if (!is_initialized()) initialize();
  AbstractInterpreter::MethodKind kind = AbstractInterpreter::method_kind(m);
  assert(0 <= kind && kind < AbstractInterpreter::number_of_method_entries, "illegal index");
  return &_ientries[kind];
}


// runtime entry points

// Finds receiver, CallInfo (i.e. receiver method), and calling bytecode)
// for a call current in progress, i.e., arguments has been pushed on stack
// put callee has not been invoked yet.  Used by: resolve virtual/static,
// vtable updates, etc.  Caller frame must be compiled
static Handle compute_call_info(JavaThread* thread, Bytecodes::Code& invoke_code, CallInfo& callinfo, TRAPS) {
  Handle receiver;
  ResourceMark rm(THREAD);

  // last java frame on stack (which includes native call frames)
  vframeStream vfst(thread, true);  // Do not skip and javaCalls
  assert(!vfst.at_end(), "Java frame must exist");

  // get caller and bci
  methodHandle caller (THREAD, vfst.method());
  int bci = vfst.bci();

  // get invoke bytecode and index
  Bytecode_invoke* invoke = Bytecode_invoke_at(caller, bci);
  invoke_code = invoke->adjusted_invoke_code();
  int invoke_index = invoke->index();

  // get receiver, if any
  if (invoke_code != Bytecodes::_invokestatic) {
    // This register map must be update since we need to find the receiver for
    // compiled frames. The receiver might be in a register.
    RegisterMap reg_map(thread);
    frame stubFrame   = thread->last_frame();
    frame caller_frame = stubFrame.real_sender(&reg_map);

    methodHandle callee = invoke->static_target(CHECK_NH);
    if (callee.is_null()) {
      THROW_(vmSymbols::java_lang_NoSuchMethodException(), callee);
    }  
#ifdef SPARC
    receiver = Handle(THREAD, caller_frame.saved_receiver());
#else
    symbolHandle signature(THREAD, callee->signature());
    receiver = caller_frame.interpreter_callee_receiver(signature);
#endif
    if (receiver.is_null()) {
      THROW_(vmSymbols::java_lang_NullPointerException(), receiver);
    }
  }

  // Resolve method. This is parameterized by bytecode.  
  constantPoolHandle constants(THREAD, caller->constants());
  assert (receiver.is_null() || receiver->is_oop(), "wrong receiver");
  LinkResolver::resolve_invoke(callinfo, receiver, constants, invoke_index, invoke_code, CHECK_NH);

#ifdef ASSERT
  // Check that the receiver klass is of the right subtype and that it is initialized for virtual calls
  if (invoke_code != Bytecodes::_invokestatic) {
    assert(receiver.not_null(), "should have thrown exception");
    KlassHandle receiver_klass (THREAD, receiver->klass());
    klassOop k = constants->klass_ref_at(invoke_index, CHECK_NH);
                                                    // klass is already loaded
    KlassHandle static_receiver_klass (THREAD, k);
    assert(receiver_klass->is_subtype_of(static_receiver_klass()), "actual receiver must be subclass of static receiver klass");
    if (receiver_klass->oop_is_instance()) {
      if (instanceKlass::cast(receiver_klass())->is_not_initialized()) {
        tty->print_cr("ERROR: Klass not yet initialized!!");
        receiver_klass.print();
      }
      assert (!instanceKlass::cast(receiver_klass())->is_not_initialized(), "receiver_klass must be initialized");
    }
  }
#endif

  return receiver;
}


static bool is_native_static(address pc) {
  pc = pc - NativeCall::instruction_size;
  nmethod* nm = CodeCache::find_nmethod(pc);
  RelocIterator iter(nm, pc, pc+1);
  bool ret = iter.next();
  assert(ret == true, "relocInfo must exist at this address");
  assert(iter.addr() == pc, "must find call");
  if (iter.type() == relocInfo::virtual_call_type) {
    return false;
  } else {
    assert(iter.type() == relocInfo::opt_virtual_call_type, "must be a virtual call");
    return true;
  }
}


JRT_ENTRY(address, Runtime1::resolve_invoke(JavaThread* thread, oop receiver))
  ResourceMark rm(thread);
#ifdef ASSERT
  _resolve_invoke_cnt++;
  RegisterMap debug_cbl_map(thread, false);
  frame debug_caller = thread->last_frame().sender(&debug_cbl_map);
  assert(thread->vm_result() == NULL, "who did it?")
  assert(thread->vm_result_2() == NULL, "who did it?")
#endif
  Handle recv(thread, receiver);

  RegisterMap cbl_map(thread, false);
  address caller_pc = thread->last_frame().sender(&cbl_map).pc();

  // It's possible that deoptimization can occur at a call site which hasn't
  // been resolved yet, in which case this function will appear to be called
  // from the deoptimization stub.  If that happens then the top vframeArray
  // will have the original call pc so that method resolution can proceed.
  CodeBlob* cb = CodeCache::find_blob(caller_pc);
  if (cb->is_deoptimization_stub()) {
    vframeArray* array = thread->vframe_array_head();
    caller_pc = array->original_pc();
    cb = CodeCache::find_blob(caller_pc);
  }
  guarantee(cb != NULL && cb->is_nmethod(), "must be called from nmethod");
  // make sure caller is not getting deoptimized
  // and removed before we are done with it.
  nmethodLocker caller_lock((nmethod*)cb);

  // determine call info & receiver
  // note: a) receiver is NULL for static calls
  //       b) an exception is thrown if receiver is NULL for non-static calls
  CallInfo call_info;
  Bytecodes::Code invoke_code = Bytecodes::_illegal;
  { // computes invoke_code and call_info
    Handle r = compute_call_info(thread, invoke_code, call_info, CHECK_0);
    assert(r() == recv(), "receivers must be identical");
  }
  methodHandle callee_method = call_info.selected_method();
  bool is_virtual = (invoke_code == Bytecodes::_invokevirtual || invoke_code == Bytecodes::_invokeinterface);
  // for now: we do not optimize any optimizable virtual calls
  // bool is_optimized = Klass::can_be_statically_bound(call_info.resolved_method()());

  // The only possible optimization is currently for finals
  bool is_optimized = call_info.resolved_method()->is_final_method();

  // Note: the following problem exists with Compiler1:
  //       - at compile time we may or may not know if the destination is final
  //       - if we know that the destination is final, we will emit an optimized
  //         virtual call (no inline cache), and need a methodOop to make a call to
  //         interpreter
  //       - if we do not know if the destination is final, we emit a standard virtual
  //         call, and use CompiledICHolder to call interpreted code (no static call
  //         stub has been generated)
  //       That is why we need to determine if the call is static in its native form.

  // static_call is true for invokestatic, invokespecial and invokevirtuals to known final methods
  // static call does not have an inline cache
  bool static_call = is_virtual ? is_native_static(caller_pc) : true;

  // tracing/debugging
#ifndef PRODUCT
  if (TraceCallFixup) {
    ResourceMark rm(thread);
    tty->print("resolving %s to: ", Bytecodes::name(invoke_code));
    callee_method->print_short_name(tty);
    tty->print_cr(" (nmethod = 0x%x)", callee_method->code());
  }
#endif

  // Compute entry points. This might require generation of C2I converter
  // frames, so we cannot be holding any locks here. Furthermore, the
  // computation of the entry points is independent of patching the call.  We
  // always return the entry-point, but we only patch the stub if the call has
  // not been deoptimized.  Return values: For a virtual call this is an
  // (cached_oop, destination address) pair. For a static call/optimized
  // virtual this is just a destination address.

  StaticCallInfo static_call_info;
  CompiledICInfo virtual_call_info;
  address  entry = NULL;
  Handle   oop_result;

  // Make sure the callee nmethod does not get deoptimized and removed before
  // we are done patching the code. callee_nm is NULL if we are calling an
  // interpreted method
  nmethodLocker nl_callee(callee_method->code());

#ifdef ASSERT
  nmethod* nm = callee_method->code();
  address dest_entry_point = (nm == NULL) ? 0 : nm->entry_point(); // used below
#endif

  if (is_virtual && !static_call) {
    // We cannot compute this info for optimized virtual calls (needs a methodOop in
    // info instead of the CompiledICHolder)
    assert(recv.not_null(), "sanity check");
    // for now: we do not optimize any optimizable virtual calls
    bool static_bound = is_optimized; //Klass::can_be_statically_bound(call_info.resolved_method()());
    KlassHandle h_klass(THREAD, recv->klass());
    CompiledIC::compute_monomorphic_entry(callee_method, h_klass, is_optimized, static_bound, virtual_call_info, CHECK_0);
    entry = virtual_call_info.entry();
    oop_result = virtual_call_info.cached_oop();
  } else {
    // static call or optimized virtual calls (use methodOop in info)
    CompiledStaticCall::compute_entry(callee_method, static_call_info);
    entry = static_call_info.entry();
    oop_result = Handle(THREAD, callee_method()); // Only used when calling interpreter
  }

  // grab lock, check for deoptimization and potentially patch caller
  { MutexLocker ml_patch(CompiledIC_lock);
    // Reread frame since deoptimization could have happened while we
    // waited for the lock
    RegisterMap map(thread, false);
    frame caller_frame = thread->last_frame().sender(&map);

#ifdef ASSERT
    // We must not try to patch to jump to an already unloaded method.

    if (dest_entry_point != 0) {
      assert(CodeCache::find_blob(dest_entry_point) != NULL, "should not unload nmethod while locked");
    }
#endif

    if (!caller_frame.is_deoptimized_frame()) {
      if (is_virtual && !static_call) {
        CompiledIC* inline_cache = CompiledIC_before(caller_frame.pc());
        if (inline_cache->is_clean()) inline_cache->set_to_monomorphic(virtual_call_info);
      } else {
        CompiledStaticCall* ssc = compiledStaticCall_before(caller_frame.pc());
        if (ssc->is_clean()) ssc->set(static_call_info);
      }
    }
  }

  // return callee methodOop, recv and interpreter or verified entry point
  // (resolution was exact)
  thread->set_vm_result(callee_method());
  thread->set_vm_result_2(recv());
  return methodOopDesc::verified_code_entry(callee_method);
JRT_END


JRT_ENTRY(address, Runtime1::handle_ic_miss(JavaThread* thread, oop receiver))
  ResourceMark rm(thread);
  Handle recv(thread, receiver);

  CallInfo call_info;
  Bytecodes::Code bc;

  // receiver is NULL for static calls. An exception is thrown for NULL receivers for non-static calls
  { Handle r = compute_call_info(thread, bc, call_info, CHECK_0);
    assert(r() == recv(), "receivers must be identical");
  }
  methodHandle callee_method = call_info.selected_method();

#ifndef PRODUCT
  // Statistics & Tracing
  _ic_miss_cnt++;
  if (TraceICmisses) { tty->print_cr("IC miss"); }
#endif

  // Update inline cache to megamorphic. Skip update if caller has been deoptimized or we
  // are called from interpreted)
  { MutexLocker ml_patch (CompiledIC_lock);
    RegisterMap reg_map(thread, false);
    frame caller_frame = thread->last_frame().sender(&reg_map);
    assert(!caller_frame.is_interpreted_frame(), "interpreted methods never call to inline cache entry (without deoptimization)");
    if (caller_frame.is_compiled_frame()) {
      // Not a deoptimized frame, so find inline_cache
      CompiledIC* inline_cache = CompiledIC_before(caller_frame.pc());
      if (!inline_cache->is_megamorphic() && !inline_cache->is_clean()) {
        // Change to megamorphic
        inline_cache->set_to_megamorphic(&call_info, bc, CHECK_0);
      } else {
        // Either clean or megamorphic
      }
    }
  } // Release CompiledIC_lock

  // Return methodOop & recv through TLS
  thread->set_vm_result(callee_method());
  thread->set_vm_result_2(recv());
  // Return address to jump to
  return methodOopDesc::verified_code_entry(callee_method);
JRT_END


// Note: There are no c2r, c2i or i2c adapters; this code should be cleaned up.
JRT_ENTRY(address, Runtime1::handle_wrong_method(JavaThread* thread, oop receiver))
  ResourceMark rm(thread);
  Handle recv(thread, receiver);
  RegisterMap reg_map(thread, false);
  // Check if the caller is a compiled method or an I2C adapter
  // Note: We can get an deoptimized frame on the stack - if we first get
  // into a lazy_c2i_adapter stub - that will deoptimize the calling frame.
  //assert(check_compiled_frame(thread, true, false, true), "incorrect caller");
  frame c2r_frame = thread->last_frame();
  frame caller_frame = c2r_frame.sender(&reg_map);

  //Force resolving of caller (if we called from compiled frame)
  if (caller_frame.is_compiled_frame()) {
    SharedRuntime::reresolve_call_site(caller_frame);
  }
#ifndef PRODUCT
  _handle_wrong_method_cnt++;
#endif

  methodHandle callee_method;

  // We need first to check if any Java activations (compiled, interpreted, deoptimized)
  // exist on the stack since last JavaCall. If not, we need to get the target method
  // and receiver from the JavaCall wrapper.
  vframeStream vfst(thread, true);  // Do not skip and javaCalls
  if (vfst.at_end()) {
    // No Java frames was found on stack since we did the JavaCall.
    // Hence, the stack can only contain an entry_frame and possible some
    // and i2c_adapter. We need to find the target method and receviver from
    // the stub frame
    RegisterMap reg_map(thread, false);
    frame fr = thread->last_frame();
    assert(fr.is_c2runtime_frame(), "must be a runtimeStub");
    fr = fr.sender(&reg_map);
    while(!fr.is_entry_frame())  {
      assert(fr.is_c2i_frame() || fr.is_i2c_frame(), "sanity check");
      fr = fr.sender(&reg_map);
    }
    // fr is now pointing to the entry frame.
    callee_method = methodHandle(THREAD, fr.entry_frame_call_wrapper()->callee_method());
    { Handle r(THREAD, fr.entry_frame_call_wrapper()->receiver());
      assert(r() == recv(), "receivers must be identical");
    }
  } else {
    Bytecodes::Code bc;
    CallInfo callinfo;
    { Handle r = compute_call_info(thread, bc, callinfo, CHECK_0);
      assert(r() == recv(), "receivers must be identical");
    }
    callee_method = callinfo.selected_method();
  }
  assert(recv.is_null() || !callee_method->is_static(), "non-null receiver for static call?");

  thread->set_vm_result(callee_method());
  thread->set_vm_result_2(recv());

  // Return address to jump to
  return methodOopDesc::verified_code_entry(callee_method);
JRT_END


JRT_ENTRY(void, Runtime1::new_instance(JavaThread* thread, klassOop klass))
  assert(oop(klass)->is_klass(), "not a class");
  instanceKlassHandle h(thread, klass);
  h->check_valid_for_instantiation(true, CHECK);
  // make sure klass is initialized
  h->initialize(CHECK);
  // allocate instance and return via TLS
  oop obj = h->allocate_instance(CHECK);
  thread->set_vm_result(obj);
JRT_END


JRT_ENTRY(void, Runtime1::new_type_array(JavaThread* thread, BasicType elt_type, jint length))
  oop obj = oopFactory::new_typeArray(elt_type, length, CHECK);
  thread->set_vm_result(obj);
JRT_END


JRT_ENTRY(void, Runtime1::new_object_array(JavaThread* thread, klassOop array_klass, jint length))
  // Note: no handle for klass needed since they are not used
  //       anymore after new_objArray() and no GC can happen before.
  //       (This may have to change if this code changes!)
  assert(oop(array_klass)->is_klass(), "not a class");
  klassOop elem_klass = objArrayKlass::cast(array_klass)->element_klass();
  objArrayOop obj = oopFactory::new_objArray(elem_klass, length, CHECK);
  thread->set_vm_result(obj);
JRT_END


JRT_ENTRY(void, Runtime1::new_multi_array(JavaThread* thread, klassOop klass, int rank, jint* dims))
  assert(oop(klass)->is_klass(), "not a class");
  assert(rank >= 1, "rank must be nonzero");
#ifdef _LP64
// In 64 bit mode, the sizes are stored in the top 32 bits
// of each 64 bit stack entry.
// dims is actually an intptr_t * because the arguments
// are pushed onto a 64 bit stack.
// We must create an array of jints to pass to multi_allocate.
// We reuse the current stack because it will be popped
// after this bytecode is completed.
  if ( rank > 1 ) {
    int index;
    for ( index = 1; index < rank; index++ ) {  // First size is ok
        dims[index] = dims[index*2];
    }
  }
#endif
  oop obj = arrayKlass::cast(klass)->multi_allocate(rank, dims, 1, CHECK);
  thread->set_vm_result(obj);
JRT_END


JRT_ENTRY(void, Runtime1::unimplemented_entry(JavaThread* thread, StubID id))
  tty->print_cr("Runtime1::entry_for(%d) returned unimplemented entry point", id);
JRT_END


JRT_ENTRY(void, Runtime1::throw_array_store_exception(JavaThread* thread))
  THROW(vmSymbolHandles::java_lang_ArrayStoreException());
JRT_END


JRT_ENTRY(void, Runtime1::post_jvmdi_exception_throw(JavaThread* thread, oop exception))
  if (jvmdi::enabled()) {
    vframeStream vfst(thread, true);
    address bcp = vfst.method()->bcp_from(vfst.bci());
    jvmdi::post_exception_throw_event(thread, vfst.method(), bcp, exception);
  }
JRT_END

static void set_pc_not_at_call_for_caller(JavaThread* thread) {
  // if we throw an implicit exception we need to set the
  // 'pc_not_at_call' bit for the caller frame, as the pc
  // information doesn't correspond to a (assembly level)
  // call instruction but another instruction.
  if (thread->is_handling_implicit_exception()) {
    RegisterMap reg_map(thread);
    frame stub_frame = thread->last_frame();
    frame caller_frame = stub_frame.real_sender(&reg_map);
    thread->set_pc_not_at_call_for_frame(caller_frame.id());
  }
}

static address fast_find_exception_handler(JavaThread* thread, Handle exception, address pc) {
  assert(!thread->is_handling_implicit_exception() && UseFastExceptionHandling, "should not enter");
  address adjust_pc = thread->safepoint_state()->compute_adjusted_pc(pc);
  nmethod* nm = CodeCache::find_nmethod(adjust_pc);
  assert(nm != NULL, "this is not an nmethod");
  address continuation = nm->handler_for_exception_and_pc(exception, adjust_pc);
  return continuation;
}


extern void vm_exit(int code);

// Enter this method from compiled code only if there is a Java exception handler
// in the method handling the exception

JRT_ENTRY(address, Runtime1::exception_handler_for_pc(JavaThread* thread, oop ex, address pc))
  Handle exception(thread, ex);
#ifdef ASSERT
  assert(exception.not_null(), "NULL exceptions should be handled by throw_exception");
  assert(exception->is_oop(), "just checking");
  // Check that exception is a subclass of Throwable, otherwise we have a VerifyError
  if (!(exception->is_a(SystemDictionary::throwable_klass()))) {
    if (ExitVMOnVerifyError) vm_exit(-1);
    ShouldNotReachHere();
  }
#endif
  bool entered_as_implicit = thread->is_handling_implicit_exception();
  pc = thread->safepoint_state()->compute_adjusted_pc(pc);
  // too noisy event
  // Events::log("exception_handler_for_pc @ %#x oop %#x impl %d", pc, ex, entered_as_implicit);
  // ExceptionCache is used only for exceptions at call and not for implicit exceptions

  // Check the stack guard pages and reenable them if necessary and there is
  // enough space on the stack to do so.  Use fast exceptions only if the guard
  // pages are enabled.
  bool guard_pages_enabled = thread->stack_yellow_zone_enabled();
  if (!guard_pages_enabled) guard_pages_enabled = thread->reguard_stack();
  if (!entered_as_implicit && UseFastExceptionHandling && guard_pages_enabled && !jvmdi::enabled()) {
    Handle h_ex(thread, ex);
    address fast_continuation = fast_find_exception_handler(thread, h_ex, pc);
    if (fast_continuation != NULL) {
      if (fast_continuation == ExceptionCache::unwind_handler()) fast_continuation = NULL;
      thread->set_vm_result(exception());
      return fast_continuation;
    }
  }

  set_pc_not_at_call_for_caller(thread);             // needed for implicit exceptions
  thread->set_is_handling_implicit_exception(false); // reset implicit exception flag

  address continuation = NULL;
  // JVMDI only
  address handler_bcp;
  methodHandle method;

    // If the stack guard pages are enabled, check whether there is a handler in
    // the current method.  Otherwise (guard pages disabled), force an unwind and
    // skip the exception cache update (i.e., just leave continuation==NULL).
    if (guard_pages_enabled) {

      // New exception handling mechanism can support inlined methods
      // with exception handlers since the mappings are from PC to PC

      CodeBlob* cb = CodeCache::find_blob(pc);
      guarantee(cb != NULL && cb->is_nmethod(), "exception happened outside interpreter, nmethods and vtable stubs");
      nmethod*  nm = (nmethod*) cb;

      // debugging support
      // tracing
      if (TraceExceptions) {
        ThreadCritical tc;
        ResourceMark rm;
        tty->print_cr("Exception <%s> (0x%x) thrown in compiled method <%s> for thread 0x%x",
                      exception->print_value_string(), exception(), nm->method()->print_value_string(), thread);
      }

      int pco = ExceptionRangeTable::compute_modified_at_call_pco(pc - nm->instructions_begin(), !entered_as_implicit);
      ExceptionRangeTable* table = nm->exception_range_table();
      assert(table != NULL, "");
      int idx = table->entry_index_for_pco(pco);
      if (idx != -1) {
        ResourceMark rm(thread);
        ScopeDesc* sd = nm->scope_desc_at(pc, !entered_as_implicit);
        int scope_count = 0;
        assert(sd != NULL, "must have scope description at exception sites");
        while (idx < table->length()) {
          ExceptionRangeEntry* entry = table->entry_at(idx);
          if (entry->covers(pco)) {
            assert(entry->scope_count() >= scope_count,
                   "scopes must be added to ExceptionRangeTable in inner-to-outer order");
            while (scope_count < entry->scope_count()) {
              sd = sd->sender();
              assert(sd != NULL, "out of scopes - bug in ExceptionRangeTable generation");
              ++scope_count;
            }
            KlassHandle klass;
            if (entry->exception_type() != 0) {
              klassOop k = sd->method()->constants()->klass_at(entry->exception_type(), CHECK_0);
              klass = KlassHandle(thread, k);
            }
            if (entry->exception_type() == 0 || exception()->is_a(klass())) {
              continuation = nm->instructions_begin() + entry->handler_pco();
              if (jvmdi::enabled()) {
                handler_bcp = sd->method()->bcp_from(entry->handler_bci());
                method = sd->method();
              }
#ifdef ASSERT
              KlassHandle ek (thread, exception->klass());
              int i_bci = sd->method()->fast_exception_handler_bci_for(ek,
                                                    sd->bci(), true, CHECK_(0));
              assert(entry->handler_bci() == i_bci, "dispatching to the wrong exception handler");
#endif // ASSERT
              break;
            }
          } else {
            if (entry->start_pco() > pco) break;
          }
          ++idx;
        }
      }

      if (UseFastExceptionHandling && !entered_as_implicit && !jvmdi::enabled()) {
        // the exception cache is used only by non-implicit exceptions
        if (continuation == NULL) {
          nm->add_handler_for_exception_and_pc(exception, pc, ExceptionCache::unwind_handler());
        } else {
          nm->add_handler_for_exception_and_pc(exception, pc, continuation);
        }
      }
    }

  // notify debugger of an exception catch
  // (this is good for exceptions caught in native methods as well)
  if (jvmdi::enabled() && continuation != NULL) {
    //Pass true for in_handler_frame because the handler frame is known by C1 at this point.
    jvmdi::notice_unwind_due_to_exception(thread, method(), handler_bcp, exception(), true);
  }

  thread->set_vm_result(exception());
  return continuation;
JRT_END


JRT_LEAF(address, Runtime1::return_address_for_exception())
  // Compute the (corrected) issuing pc for an exception pc.
  // Caution: This routine must be a leaf routine (no GCs. no locking!)
  //          because we are in the middle of exception resolving
  //
  // Note: The pc is __not__ a conventional next_pc value (as from
  // frame::pc), but the real address of the trapping instruction.
  //
  // compute the corrected pc (if the exception happened in a temporary code
  // buffer used for safepoint synchronization, we need to adjust it first)
  JavaThread* thread = JavaThread::current();
#ifdef CC_INTERP
  // QQQ the assert below is not correct for C++ Interpreter (too weak)
#endif
  assert(!AbstractInterpreter::contains(thread->saved_exception_pc()), "interpreter exceptions must be handled already");
  assert(!thread->is_handling_implicit_exception(), "implicit exception handling bit should not be set");
  address pc = thread->safepoint_state()->compute_adjusted_pc(thread->saved_exception_pc());
  // check if exception happened outside nmethods
  if (VtableStubs::contains(pc) || blob_for(interpreter_entries_id)->contains(pc)) {
    // exception happened in vtable stub or interpreter entry code
    // => must be a NULL receiver => use return address
    // pushed by caller => don't push another return address
    return NULL;
  }
  // exception must have happened inside an nmethod
  CodeBlob* cb = CodeCache::find_blob(pc);
  guarantee(cb != NULL && cb->is_nmethod(), "exception happened outside interpreter, nmethods and vtable stubs");

  nmethod* nm = (nmethod*)cb;
  if (nm->inlinecache_check_contains(pc)) {
    // exception happened inside inline-cache check code
    // => the nmethod is not yet active (i.e., the frame
    // is not set up yet) => use return address pushed by
    // caller => don't push another return address
    return NULL;
  }
  // the exception happened inside an nmethod and is
  // not at a call => set implicit exception bit and
  // push exception pc as return address
  //
  // note: the implicit exception bit is reset when
  //       the exception is handled
  thread->set_is_handling_implicit_exception(true);
  return pc;
JRT_END


JRT_LEAF(address, Runtime1::compute_exception_pc(int pc_offset, address pc_addr))
  // compute the corrected pc (if the exception happened in a temporary code
  // buffer used for safepoint synchronization, we need to adjust it first)
  JavaThread* thread = JavaThread::current();
  address pc_adj = thread->safepoint_state()->compute_adjusted_pc(pc_addr);
  // compute exception pc
  nmethod* nm = CodeCache::find_nmethod(pc_adj);
  assert(nm != NULL, "nmethod not found");
  return nm->instructions_begin() + pc_offset;
JRT_END

static void throw_and_post_jvmdi_exception(JavaThread *thread, Handle h_exception) {
  if (jvmdi::enabled() && FullSpeedJVMDI) {
    vframeStream vfst(thread, true);
    methodHandle method = methodHandle(thread, vfst.method());
    address bcp = method()->bcp_from(vfst.bci());
    jvmdi::post_exception_throw_event(thread, method(), bcp, h_exception());
  }
  Exceptions::_throw(thread, __FILE__, __LINE__, h_exception);
}

static void throw_and_post_jvmdi_exception(JavaThread *thread, symbolOop name, const char *message = NULL) {
  Handle h_exception = Exceptions::new_exception(thread, name, message);
  throw_and_post_jvmdi_exception(thread, h_exception);
}


JRT_ENTRY(void, Runtime1::throw_range_check_exception(JavaThread* thread, int index))
  Events::log("throw_range_check");
  char message[jintAsStringSize];
  sprintf(message, "%d", index);
  throw_and_post_jvmdi_exception(thread, vmSymbols::java_lang_ArrayIndexOutOfBoundsException(), message);
JRT_END


JRT_ENTRY(void, Runtime1::throw_index_exception(JavaThread* thread, int index))
  Events::log("throw_index");
  char message[16];
  sprintf(message, "%d", index);
  throw_and_post_jvmdi_exception(thread, vmSymbols::java_lang_IndexOutOfBoundsException(), message);
JRT_END


JRT_ENTRY(void, Runtime1::throw_div0_exception(JavaThread* thread))
  throw_and_post_jvmdi_exception(thread, vmSymbols::java_lang_ArithmeticException(), "/ by zero");
JRT_END


JRT_ENTRY(void, Runtime1::implicit_throw_div0_exception(JavaThread* thread))
  set_pc_not_at_call_for_caller(thread); // needed for implicit div0 exceptions
  throw_and_post_jvmdi_exception(thread, vmSymbols::java_lang_ArithmeticException(), "/ by zero");
JRT_END


// Used only by implicit null exceptions
JRT_ENTRY(void, Runtime1::implicit_throw_null_exception(JavaThread* thread))
  set_pc_not_at_call_for_caller(thread); // needed for implicit null exceptions
  throw_and_post_jvmdi_exception(thread, vmSymbols::java_lang_NullPointerException());
JRT_END


JRT_ENTRY(void, Runtime1::throw_stack_overflow(JavaThread* thread))
  // Creating a new stack overflow object here will cause javaCalls::call()
  // to try to throw stack overflow exception again, and end up back here.
  // Use the preallocated StackOverflowError class.  See 
  // InterpreterRuntime::create_StackOverflowError
  // 
  klassOop k = SystemDictionary::StackOverflowError_klass();
  oop exception = instanceKlass::cast(k)->allocate_instance(CHECK);
  Handle h_exception = Handle(thread, exception);
  throw_and_post_jvmdi_exception(thread, h_exception);
JRT_END


JRT_ENTRY(void, Runtime1::throw_abstract_method_error(JavaThread* thread))
  throw_and_post_jvmdi_exception(thread, vmSymbols::java_lang_AbstractMethodError());
JRT_END


JRT_ENTRY(void, Runtime1::throw_class_cast_exception(JavaThread* thread, oop object))
  ResourceMark rm(thread);
  Handle obj(thread, object);
  throw_and_post_jvmdi_exception(thread, vmSymbols::java_lang_ClassCastException(), Klass::cast(obj->klass())->external_name());
JRT_END


JRT_ENTRY_FOR_MONITORENTER(void, Runtime1::monitorenter(JavaThread* thread, oop obj, BasicObjectLock* lock))
  Handle h_obj(thread, obj);
  assert(h_obj()->is_oop(), "must be NULL or an object");
  if (UseFastLocking) {
    // When using fast locking, the compiled code has already tried the fast case
    assert(obj == lock->obj(), "must match");
    ObjectSynchronizer::slow_enter(h_obj, lock->lock(), THREAD);
  } else {
    lock->set_obj(obj);
    ObjectSynchronizer::fast_enter(h_obj, lock->lock(), THREAD);
  }
JRT_END


JRT_LEAF(void, Runtime1::monitorexit(JavaThread* thread, BasicObjectLock* lock))
  assert(thread == JavaThread::current(), "threads must correspond");
  assert(thread->last_Java_sp(), "last_Java_sp must be set");
  // monitorexit is non-blocking (leaf routine) => no exceptions can be thrown
  EXCEPTION_MARK;

  oop obj = lock->obj();
  assert(obj->is_oop(), "must be NULL or an object");
  if (UseFastLocking) {
    // When using fast locking, the compiled code has already tried the fast case
    ObjectSynchronizer::slow_exit(obj, lock->lock(), THREAD);
  } else {
    ObjectSynchronizer::fast_exit(obj, lock->lock(), THREAD);
  }
JRT_END


static int resolve_field_return_offset(methodHandle caller, int bci, TRAPS) {
  Bytecode_field* field_access = Bytecode_field_at(caller(), caller->bcp_from(bci));
  // This can be static or non-static field access
  Bytecodes::Code code       = field_access->code();

  // We must load class, initialize class and resolvethe field
  FieldAccessInfo result; // initialize class if needed
  constantPoolHandle constants(THREAD, caller->constants());
  LinkResolver::resolve_field(result, constants, field_access->index(), Bytecodes::java_code(code), false, CHECK_0);
  return result.field_offset();
}


static klassOop resolve_field_return_klass(methodHandle caller, int bci, TRAPS) {
  Bytecode_field* field_access = Bytecode_field_at(caller(), caller->bcp_from(bci));
  // This can be static or non-static field access
  Bytecodes::Code code       = field_access->code();

  // We must load class, initialize class and resolvethe field
  FieldAccessInfo result; // initialize class if needed
  constantPoolHandle constants(THREAD, caller->constants());
  LinkResolver::resolve_field(result, constants, field_access->index(), Bytecodes::java_code(code), false, CHECK_0);
  return result.klass()();
}


//
// This routine patches sites where a class wasn't loaded or
// initialized at the time the code was generated.  It handles
// references to classes, fields and forcing of initialization.  Most
// of the cases are straightforward and involving simply forcing
// resolution of a class, rewriting the instruction stream with the
// needed constant and replacing the call in this function with the
// patched code.  The case for static field is more complicated since
// the thread which is in the process of initializing a class can
// access it's static fields but other threads can't so the code
// either has to deoptimize when this case is detected or execute a
// check that the current thread is the initializing thread.  The
// current
//
// Patches basically look like this:
//
//
// patch_site: jmp patch stub     ;; will be patched
// continue:   ...
//             ...
//             ...
//             ...
//
// They have a stub which looks like this:
//
//             ;; patch body
//             movl <const>, reg           (for class constants)
//        <or> movl [reg1 + <const>], reg  (for field offsets)
//        <or> movl reg, [reg1 + <const>]  (for field offsets)
//             <being_init offset> <bytes to copy> <bytes to skip>
// patch_stub: call Runtime1::patch_code (through a runtime stub)
//             jmp patch_site
//
// 
// A normal patch is done by rewriting the patch body, usually a move,
// and then copying it into place over top of the jmp instruction
// being careful to flush caches and doing it in an MP-safe way.  The
// constants following the patch body are used to find various pieces
// of the patch relative to the call site for Runtime1::patch_code.
// The case for getstatic and putstatic is more complicated because
// getstatic and putstatic have special semantics when executing while
// the class is being initialized.  getstatic/putstatic on a class
// which is being_initialized may be executed by the initializing
// thread but other threads have to block when they execute it.  This
// is accomplished in compiled code by executing a test of the current
// thread against the initializing thread of the class.  It's emitted
// as boilerplate in their stub which allows the patched code to be
// executed before it's copied back into the main body of the nmethod.
//
// being_init: get_thread(<tmp reg>
//             cmpl [reg1 + <init_thread_offset>], <tmp reg>
//             jne patch_stub
//             movl [reg1 + <const>], reg  (for field offsets)  <or>
//             movl reg, [reg1 + <const>]  (for field offsets)
//             jmp continue
//             <being_init offset> <bytes to copy> <bytes to skip>
// patch_stub: jmp Runtim1::patch_code (through a runtime stub)
//             jmp patch_site
//
// If the class is being initialized the patch body is rewritten and
// the patch site is rewritten to jump to being_init, instead of
// patch_stub.  Whenever this code is executed it checks the current
// thread against the intializing thread so other threads will enter
// the runtime and end up blocked waiting the class to finish
// initializing inside the calls to resolve_field below.  The
// initializing class will continue on it's way.  Once the class is
// fully_initialized, the intializing_thread of the class becomes
// NULL, so the next thread to execute this code will fail the test,
// call into patch_code and complete the patching process by copying
// the patch body back into the main part of the nmethod and resume
// executing.
//
//


void Runtime1::patch_code(JavaThread* thread, StubID stub_id, TRAPS) {
  ResourceMark rm(thread);
  RegisterMap reg_map(thread, false);
  frame runtime_frame = thread->last_frame();
  frame caller_frame = runtime_frame.sender(&reg_map);

  // last java frame on stack
  vframeStream vfst(thread, true);
  assert(!vfst.at_end(), "Java frame must exist");

  methodHandle caller_method(THREAD, vfst.method());
  // Note that caller_method->code() may not be same as caller_code because of OSR's
  // Note also that in the presence of inlining it is not guaranteed
  // that caller_method() == caller_code->method()
  nmethod* caller_code = CodeCache::find_nmethod(caller_frame.pc());
  assert(caller_code != NULL, "nmethod not found");
  // make sure the nmethod doesn't get swept before we're done with it
  nmethodLocker locker(caller_code);

  int bci = vfst.bci();

  Events::log("patch_code @ %#x", caller_frame.pc());

  Bytecodes::Code code = Bytecode_at(caller_method->bcp_from(bci))->java_code();

#ifndef PRODUCT
  // this is used by assertions in the init_check_patching_id
  BasicType patch_field_type = T_ILLEGAL;
#endif // PRODUCT
  int patch_field_offset = -1;
  KlassHandle klass = KlassHandle(THREAD, klassOop(NULL));
  if (stub_id == init_check_patching_id) {

    Bytecode_field* field_access = Bytecode_field_at(caller_method(), caller_method->bcp_from(bci));
    FieldAccessInfo result; // initialize class if needed
    Bytecodes::Code code = field_access->code();
    constantPoolHandle constants(THREAD, caller_method->constants());
    LinkResolver::resolve_field(result, constants, field_access->index(), Bytecodes::java_code(code), false, CHECK);
    patch_field_offset = result.field_offset();
    klass = result.klass();
#ifndef PRODUCT
    patch_field_type = result.field_type();
#endif
  } else if (stub_id == load_klass_patching_id) {
    klassOop k;
    switch (code) {
      case Bytecodes::_putstatic:
      case Bytecodes::_getstatic:
        { k = resolve_field_return_klass(caller_method, bci, CHECK);
        }
        break;
      case Bytecodes::_new:
        { Bytecode_new* bnew = Bytecode_new_at(caller_method->bcp_from(bci));
          k = caller_method->constants()->klass_at(bnew->index(), CHECK);
        }
        break;
      case Bytecodes::_multianewarray:
        { Bytecode_multianewarray* mna = Bytecode_multianewarray_at(caller_method->bcp_from(bci));
          k = caller_method->constants()->klass_at(mna->index(), CHECK);
        }
        break;
      case Bytecodes::_instanceof:
        { Bytecode_instanceof* io = Bytecode_instanceof_at(caller_method->bcp_from(bci));
          k = caller_method->constants()->klass_at(io->index(), CHECK);
        }
        break;
      case Bytecodes::_checkcast:
        { Bytecode_checkcast* cc = Bytecode_checkcast_at(caller_method->bcp_from(bci));
          k = caller_method->constants()->klass_at(cc->index(), CHECK);
        }
        break;
      case Bytecodes::_anewarray:
        { Bytecode_anewarray* anew = Bytecode_anewarray_at(caller_method->bcp_from(bci));
          klassOop ek = caller_method->constants()->klass_at(anew->index(), CHECK);
          k = Klass::cast(ek)->array_klass(CHECK);
        }
        break;
      default: Unimplemented();
    }
    // convert to handle
    klass = KlassHandle(THREAD, k);
  } else {
    ShouldNotReachHere();
  }

  // Now copy code back

  {
    MutexLockerEx ml_patch (Patching_lock, Mutex::_no_safepoint_check_flag);
    NativeGeneralJump* jump = nativeGeneralJump_at(caller_frame.pc());
    address instr_pc = jump->jump_destination();
    // recompute caller frame, as deoptimization may have happened during the Patching_lock
    caller_frame = runtime_frame.sender(&reg_map);
    NativeInstruction* ni = nativeInstruction_at(instr_pc);
    if (ni->is_jump() && !caller_frame.is_deoptimized_frame()) {
      // the jump has not been patched yet
      // The jump destination is slow case and therefore not part of the stubs
      // (stubs are only for StaticCalls)

      // format of buffer
      //    ....
      //    instr byte 0     <-- copy_buff
      //    instr byte 1
      //    ..
      //    instr byte n-1
      //      n
      //    ....             <-- call destination

      address stub_location = caller_frame.pc() + PatchingStub::patch_info_offset();
      unsigned char* byte_count = (unsigned char*) (stub_location - 1);
      unsigned char* byte_skip = (unsigned char*) (stub_location - 2);
      unsigned char* being_initialized_entry_offset = (unsigned char*) (stub_location - 3);
      address copy_buff = stub_location - *byte_skip - *byte_count;
      address being_initialized_entry = stub_location - *being_initialized_entry_offset;
      if (TracePatching) {
        tty->print_cr(" Patching %s at bci %d at address 0x%x  (%s)", Bytecodes::name(code), bci,
                      instr_pc, (stub_id == init_check_patching_id) ? "field" : "klass");
        OopMap* map = caller_code->oop_map_for_return_address(caller_frame.pc(), true);
        assert(map != NULL, "null check");
        map->print();
        tty->cr();
      }
      // depending on the code below, do_patch says whether to copy the patch body back into the nmethod
      bool do_patch = true;
      if (stub_id == init_check_patching_id) {
        do_patch = instanceKlass::cast(klass())->is_initialized() ||
          (code != Bytecodes::_getstatic && code != Bytecodes::_putstatic);
        NativeGeneralJump* jump = nativeGeneralJump_at(instr_pc);
        if (jump->jump_destination() == being_initialized_entry) {
          assert(do_patch == true, "initialization must be complete at this point");
        } else {
          // The offset may not be correct if the class was not loaded at code generation time.
          // Set it now.
          NativeMovRegMem* n_move = nativeMovRegMem_at(copy_buff);
          assert(n_move->offset() == 0 || (n_move->offset() == 4 && (patch_field_type == T_DOUBLE || patch_field_type == T_LONG)), "illegal offset for type");
          assert(patch_field_offset >= 0, "illegal offset");
          n_move->add_offset_in_bytes(patch_field_offset);
        }
      } else if (stub_id == load_klass_patching_id) {
        // patch the instruction <move reg, klass>
        NativeMovConstReg* n_copy = nativeMovConstReg_at(copy_buff);
        assert(n_copy->data() == 0, "illegal init value");
        assert(klass() != NULL, "klass not set");
        n_copy->set_data((intx) (klass()));

        // update relocInfo to oop
        nmethod* nm = CodeCache::find_nmethod(instr_pc);
        assert(nm != NULL, "invalid nmethod_pc");

        RelocIterator iter(nm, (address)instr_pc, (address)(instr_pc + 1));
        relocInfo::change_reloc_info_for_address(&iter, (address) instr_pc, relocInfo::none, relocInfo::oop_type);
#ifdef SPARC
        { address instr_pc2 = instr_pc + NativeMovConstReg::add_offset;
          RelocIterator iter2(nm, instr_pc2, instr_pc2 + 1);
          relocInfo::change_reloc_info_for_address(&iter2, (address) instr_pc2, relocInfo::none, relocInfo::oop_type);
        }
#endif
      } else {
        ShouldNotReachHere();
      }
      if (do_patch) {
        // replace instructions
        // first replace the tail, then the call
        for (int i = NativeCall::instruction_size; i < *byte_count; i++) {
          address ptr = copy_buff + i;
          int a_byte = (*ptr) & 0xFF;
          address dst = instr_pc + i;
          *(unsigned char*)dst = (unsigned char) a_byte;
        }
        ICache::invalidate_range(instr_pc, *byte_count);
        NativeGeneralJump::replace_mt_safe(instr_pc, copy_buff);
#ifdef SPARC
        if (stub_id == load_klass_patching_id) {
          // Factor this out!
          // update relocInfo to oop
          nmethod* nm = CodeCache::find_nmethod(instr_pc);
          assert(nm != NULL, "invalid nmethod_pc");

          RelocIterator oops(nm, instr_pc, instr_pc + 1);
          bool found = false;
          while (oops.next() && !found) {
            if (oops.type() == relocInfo::oop_type) {
              oop_Relocation* r = oops.oop_reloc();
              oop* oop_adr = r->oop_addr();
              *oop_adr = klass();
              r->fix_oop_relocation();
              found = true;
            }
          }
          assert(found, "the oop must exist!");
        }
#endif
      } else {
        ICache::invalidate_range(copy_buff, *byte_count);
        NativeGeneralJump::insert_unconditional(instr_pc, being_initialized_entry);
      }
    }
  }
}

JRT_ENTRY(void, Runtime1::move_klass_patching(JavaThread* thread))
  patch_code(thread, load_klass_patching_id, CHECK);
JRT_END


JRT_ENTRY(void, Runtime1::init_check_patching(JavaThread* thread))
  patch_code(thread, init_check_patching_id, CHECK);
JRT_END


JRT_LEAF(void, Runtime1::trace_method_entry(jint v1, jint v2))
  tty->print_cr("Entering a method 0x%x 0x%x", v1, v2);
JRT_END


JRT_LEAF(void, Runtime1::trace_method_exit(jint value))
  Unimplemented();
JRT_END


JRT_LEAF(void, Runtime1::trace_block_entry(jint block_id))
  // for now we just print out the block id
  tty->print("%d ", block_id);
JRT_END


// fast and direct copy of arrays; returning -1, means that an exception may be thrown
// and we did not copy anything
JRT_LEAF(int, Runtime1::arraycopy(int length, int dst_pos, oop dst, int src_pos, oop src))
  enum {
    ac_failed = -1, // arraycopy failed
    ac_ok = 0       // arraycopy succeeded
  };

  if (!dst->is_array() || !src->is_array()) return ac_failed;
  assert(dst->is_array() && src->is_array(), "wrong objects");
  assert (src != NULL && dst != NULL &&  src_pos >= 0 && dst_pos >= 0 && length >= 0, "prechecks failed");
  assert((unsigned int) arrayOop(src)->length() >= (unsigned int)src_pos + (unsigned int)length, "precheck failed");
  assert((unsigned int) arrayOop(dst)->length() >= (unsigned int)dst_pos + (unsigned int)length, "precheck failed");

  if (length == 0) return ac_ok;
  if (src->is_typeArray()) {
    const klassOop klass_oop = src->klass();
    if (klass_oop != dst->klass()) return ac_failed;
    typeArrayKlass* klass = typeArrayKlass::cast(klass_oop);
    const int sc  = klass->scale();
    const int ihs = klass->array_header_in_bytes() / wordSize;
    char* src_addr = (char*) ((oop*)src + ihs) + (src_pos * sc);
    char* dst_addr = (char*) ((oop*)dst + ihs) + (dst_pos * sc);
    // Potential problem: memmove is not guaranteed to be word atomic
    // Revisit in Merlin
    memmove(dst_addr, src_addr, length * sc);
    return ac_ok;
  } else {
    assert(src->is_objArray(), "what array type is it else?");
    oop* src_addr = objArrayOop(src)->obj_at_addr(src_pos);
    oop* dst_addr = objArrayOop(dst)->obj_at_addr(dst_pos);
    // For performance reasons, we assume we are using a card marking write
    // barrier. The assert will fail if this is not the case.
    // Note that we use the non-virtual inlineable variant of write_ref_array.
    BarrierSet* bs = Universe::heap()->barrier_set();
    if (src == dst) {
      // same object, no check
      Memory::copy_oops_overlapping_atomic(src_addr, dst_addr, length);
      bs->write_ref_array(MemRegion((HeapWord*)dst_addr,
				    (HeapWord*)(dst_addr + length)));
      return ac_ok;
    } else {
      klassOop bound = objArrayKlass::cast(dst->klass())->element_klass();
      klassOop stype = objArrayKlass::cast(src->klass())->element_klass();
      if (stype == bound || Klass::cast(stype)->is_subtype_of(bound)) {
        // Elements are guaranteed to be subtypes, so no check necessary
        Memory::copy_oops_overlapping_atomic(src_addr, dst_addr, length);
        bs->write_ref_array(MemRegion((HeapWord*)dst_addr,
				      (HeapWord*)(dst_addr + length)));
        return ac_ok;
      }
    }
  }
  return ac_failed;
JRT_END


JRT_ENTRY(void, Runtime1::jvmpi_method_entry_after_deopt(JavaThread* thread, oop receiver))
  // Perform JVMPI method entry notification for a synchronized method activation
  // that was deoptimized after its monitorenter operation.  First, get the method:
  vframeStream vfst(thread, true);  // Do not skip and javaCalls
  assert(!vfst.at_end(), "Java frame must exist");
  methodHandle method (THREAD, vfst.method());
#ifdef ASSERT
  vframeArray* array = thread->vframe_array_head();
  address caller_pc = array->original_pc();
  CodeBlob* cb = CodeCache::find_blob(caller_pc);
  assert(cb != NULL && cb->is_nmethod(), "must be called from nmethod");
  assert(((nmethod*)cb)->method() == method(), "wrong method");
#endif
  if (method()->is_static()) {
    // clear the "receiver", which is really the method's class that was passed to monitorenter
    receiver = NULL;
  }
  SharedRuntime::jvmpi_method_entry_work(thread, method(), receiver);
JRT_END


#ifndef PRODUCT
void Runtime1::print_statistics() {
  tty->print_cr("C1 Runtime statistics:");
  tty->print_cr("    _resolve_invoke_cnt:      %d", _resolve_invoke_cnt);
  tty->print_cr("    _handle_wrong_method_cnt: %d", _handle_wrong_method_cnt);
  tty->print_cr("    _ic_miss_cnt:             %d", _ic_miss_cnt);
  tty->print_cr("  _checkcast_cnt:             %d", _checkcast_cnt);
  tty->print_cr(" _storecheck_cnt:             %d", _storecheck_cnt);
  tty->cr();
}
#endif // PRODUCT

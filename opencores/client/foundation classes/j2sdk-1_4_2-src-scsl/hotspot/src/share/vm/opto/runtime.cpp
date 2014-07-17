#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)runtime.cpp	1.400 03/03/19 10:06:11 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_runtime.cpp.incl"


// For debugging purposes:
//  To force FullGCALot inside a runtime function, add the following two lines
//
//  Universe::release_fullgc_alot_dummy();
//  MarkSweep::invoke(0, "Debugging");
//
// At command line specify the parameters: -XX:+FullGCALot -XX:FullGCALotStart=100000000




// Compiled code entry points
address OptoRuntime::_resolve_virtual_call_Java                   = NULL;
address OptoRuntime::_resolve_opt_virtual_call_Java               = NULL;
address OptoRuntime::_resolve_static_call_Java                    = NULL;
address OptoRuntime::_new_Java                                    = NULL;
address OptoRuntime::_new_typeArray_Java                          = NULL;
address OptoRuntime::_new_objArray_Java                           = NULL;
address OptoRuntime::_multianewarray1_Java                        = NULL;
address OptoRuntime::_multianewarray2_Java                        = NULL;
address OptoRuntime::_multianewarray3_Java                        = NULL;
address OptoRuntime::_multianewarray4_Java                        = NULL;
address OptoRuntime::_multianewarray5_Java                        = NULL;
address OptoRuntime::_handle_ic_miss_Java                         = NULL;
address OptoRuntime::_handle_wrong_method_Java                    = NULL;
address OptoRuntime::_lazy_c2i_adapter_Java                       = NULL;
address OptoRuntime::_vtable_must_compile_Java                    = NULL;
address OptoRuntime::_complete_monitor_locking_Java               = NULL;
address OptoRuntime::_handle_abstract_method_Java                 = NULL;
address OptoRuntime::_throw_null_exception_Java                   = NULL;
address OptoRuntime::_throw_stack_overflow_error_Java             = NULL;
address OptoRuntime::_throw_div0_exception_Java                   = NULL;
address OptoRuntime::_throw_exception_Java                        = NULL;
address OptoRuntime::_handle_exception_Java                       = NULL;
address OptoRuntime::_rethrow_Java                                = NULL;

address OptoRuntime::_jvmpi_method_entry_Java                     = NULL;
address OptoRuntime::_jvmpi_method_exit_Java                      = NULL;

# ifdef ENABLE_ZAP_DEAD_LOCALS
address OptoRuntime::_zap_dead_Java_locals_Java                   = NULL;
address OptoRuntime::_zap_dead_native_locals_Java                 = NULL;
# endif


// This should be called in an assertion at the start of OptoRuntime routines
// which are entered from compiled code (all of them)
#ifndef PRODUCT
static bool check_compiled_frame(JavaThread* thread, bool allow_i2c, bool allow_c2i, bool allow_deopt) {  
  assert(thread->last_frame().is_glue_frame(), "cannot call runtime directly from compiled code");
#ifdef ASSERT
  RegisterMap map(thread, false);
  frame caller = thread->last_frame().sender(&map);
  assert(   caller.is_compiled_frame()
         || (allow_i2c && caller.is_i2c_frame())
         || (allow_c2i && caller.is_c2i_frame())
         || (allow_deopt && caller.is_deoptimized_frame()),
         "not being called from compiled like code");  
#endif 
  return true;
}
#endif


#define gen(env, var, type_func_gen, c_func, fancy_jump, pass_tls, save_arg_regs, return_pc) \
  var = generate_stub(env, type_func_gen, CAST_FROM_FN_PTR(address, c_func), #var, fancy_jump, pass_tls, save_arg_regs, return_pc)

void OptoRuntime::generate(ciEnv* env) {
  
  // Reserve room for exception blob. Creates entry-point
  setup_exception_blob();  

  // Note: tls: Means fetching the return oop out of the thread-local storage
  //
  //   variable/name                       type-function-gen              , runtime method                  ,fncy_jp, tls,save_args,retpc
  // -------------------------------------------------------------------------------------------------------------------------------
  gen(env, _handle_ic_miss_Java            , resolve_call_Type            , handle_wrong_method_ic_miss     ,    1 , true , true , false); 
  gen(env, _handle_wrong_method_Java       , resolve_call_Type            , handle_wrong_method             ,    1 , true , true , false);
  gen(env, _new_Java                       , new_Type                     , new_C                           ,    0 , true , false, false);
  gen(env, _new_typeArray_Java             , new_typeArray_Type           , new_typeArray_C                 ,    0 , true , false, false);
  gen(env, _new_objArray_Java              , new_objArray_Type            , new_objArray_C                  ,    0 , true , false, false);
  gen(env, _multianewarray1_Java           , multianewarray1_Type         , multianewarray1_C               ,    0 , true , false, false);  
  gen(env, _multianewarray2_Java           , multianewarray2_Type         , multianewarray2_C               ,    0 , true , false, false);
  gen(env, _multianewarray3_Java           , multianewarray3_Type         , multianewarray3_C               ,    0 , true , false, false);
  gen(env, _multianewarray4_Java           , multianewarray4_Type         , multianewarray4_C               ,    0 , true , false, false);
  gen(env, _multianewarray5_Java           , multianewarray5_Type         , multianewarray5_C               ,    0 , true , false, false);
  gen(env, _resolve_static_call_Java       , resolve_call_Type            , resolve_static_call_C           ,    1 , true , true , false);
  gen(env, _resolve_virtual_call_Java      , resolve_call_Type            , resolve_virtual_call_C          ,    1 , true , true , false);
  gen(env, _resolve_opt_virtual_call_Java  , resolve_call_Type            , resolve_opt_virtual_call_C      ,    1 , true , true , false);
  gen(env, _lazy_c2i_adapter_Java          , resolve_call_Type            , lazy_c2i_adapter_generation_C   ,    1 , true , true , false);     
  gen(env, _complete_monitor_locking_Java  , complete_monitor_enter_Type  , complete_monitor_locking_C      ,    0 , false, false, false);
  gen(env, _handle_abstract_method_Java    , resolve_call_Type            , handle_abstract_method_C        ,    0 , false, false, false);
  gen(env, _throw_null_exception_Java      , resolve_call_Type            , throw_null_exception_C          ,    0 , false, false, false);  
  gen(env, _throw_stack_overflow_error_Java, resolve_call_Type            , throw_stack_overflow_error_C    ,    0 , false, false, false);  
  gen(env, _throw_div0_exception_Java      , resolve_call_Type            , throw_div0_exception_C          ,    0 , false, false, false);  
  gen(env, _throw_exception_Java           , athrow_Type                  , throw_exception_C               ,    0 , false, false, false);  
  gen(env, _handle_exception_Java          , handle_exception_Type        , handle_exception_C              ,    0 , false, false, false);
  gen(env, _rethrow_Java                   , rethrow_Type                 , rethrow_C                       ,    2 , true , false, true );

  // JVMPI support
  gen(env, _jvmpi_method_entry_Java        , jvmpi_method_entry_Type      , jvmpi_method_entry_C            ,    0 , false, false, false);  
  gen(env, _jvmpi_method_exit_Java         , jvmpi_method_exit_Type       , jvmpi_method_exit_C             ,    0 , false, false, false);  
  
# ifdef ENABLE_ZAP_DEAD_LOCALS                                                                                              
  gen(env, _zap_dead_Java_locals_Java      , zap_dead_locals_Type         , zap_dead_Java_locals_C          ,    0 , false, true , false );
  gen(env, _zap_dead_native_locals_Java    , zap_dead_locals_Type         , zap_dead_native_locals_C        ,    0 , false, true , false );
# endif
  
  // Blobs
  generate_deopt_blob();
  generate_uncommon_trap_blob();
  generate_illegal_instruction_handler_blob();
  fill_in_exception_blob(); // Generate code in preallocated space
} 

#undef gen


// Helper method to do generation of RunTimeStub's
address OptoRuntime::generate_stub( ciEnv* env,
				    TypeFunc_generator gen, address C_function, 
                                    const char *name, int is_fancy_jump, 
                                    bool pass_tls,
                                    bool save_argument_registers,
                                    bool return_pc ) {
  ResourceMark rm;
  Compile C( env, gen, C_function, name, is_fancy_jump, pass_tls, save_argument_registers, return_pc );
  return  C.stub_entry_point();
}

const char* OptoRuntime::stub_name(address entry) { 
#ifndef PRODUCT  
  CodeBlob* cb = CodeCache::find_blob(entry);
  RuntimeStub* rs =(RuntimeStub *)cb;  
  assert(rs != NULL && rs->is_runtime_stub(), "not a runtime stub");
  return rs->name();
#else
  // Fast implementation for product mode (maybe it should be inlined too)
  return "runtime stub";
#endif
}


//=============================================================================
// Opto compiler runtime routines
//=============================================================================


//=============================allocation======================================
// We failed the fast-path allocation.  Now we need to do a scavenge or GC
// and try allocation again.

// object allocation
JRT_ENTRY(void, OptoRuntime::new_C( klassOop klass, JavaThread* thread ))
#ifndef PRODUCT
  _new_ctr++;                 // 'new' object requires GC
#endif
  assert(check_compiled_frame(thread,false,false,false), "incorrect caller");

  // Scavenge and allocate an instance.
  oop result = instanceKlass::cast(klass)->allocate_instance(THREAD);
  // Pass oops back through thread local storage.  Our apparent type to Java 
  // is that we return an oop, but we can block on exit from this routine and
  // a GC can trash the oop in C's return register.  The generated stub will
  // fetch the oop from TLS after any possible GC.
  deoptimize_caller_frame(thread, HAS_PENDING_EXCEPTION);
  thread->set_vm_result(result);
JRT_END


// typeArray allocation
JRT_ENTRY(void, OptoRuntime::new_typeArray_C(BasicType elem_type, int len, JavaThread *thread))
#ifndef PRODUCT
  _new_type_ctr++;              // 'new' array requires GC
#endif
  assert(check_compiled_frame(thread,false,false,false), "incorrect caller");

  // Scavenge and allocate an instance.
  oop result = oopFactory::new_typeArray(elem_type, len, THREAD);
  // Pass oops back through thread local storage.  Our apparent type to Java 
  // is that we return an oop, but we can block on exit from this routine and
  // a GC can trash the oop in C's return register.  The generated stub will
  // fetch the oop from TLS after any possible GC.
  deoptimize_caller_frame(thread, HAS_PENDING_EXCEPTION);
  thread->set_vm_result(result);
JRT_END


// objArray allocation
JRT_ENTRY(void, OptoRuntime::new_objArray_C(klassOop elem_type, int len, JavaThread *thread))
#ifndef PRODUCT
  _new_obj_ctr++;               // 'new' object array requires GC
#endif
  assert(check_compiled_frame(thread,false,false,false), "incorrect caller");

  // Scavenge and allocate an instance.
  oop result = oopFactory::new_objArray(elem_type, len, THREAD);
  // Pass oops back through thread local storage.  Our apparent type to Java 
  // is that we return an oop, but we can block on exit from this routine and
  // a GC can trash the oop in C's return register.  The generated stub will
  // fetch the oop from TLS after any possible GC.
  deoptimize_caller_frame(thread, HAS_PENDING_EXCEPTION);
  thread->set_vm_result(result);
JRT_END

// multianewarray for one dimension
JRT_ENTRY(void, OptoRuntime::multianewarray1_C(klassOop elem_type, int len1, JavaThread *thread))
#ifndef PRODUCT
  _multi1_ctr++;                // multianewarray for 1 dimension
#endif
  assert(check_compiled_frame(thread,false,false,false), "incorrect caller");
  assert(oop(elem_type)->is_klass(), "not a class");
  jint dims[1];
  dims[0] = len1;
  oop obj = arrayKlass::cast(elem_type)->multi_allocate(1, dims, +1, THREAD);
  deoptimize_caller_frame(thread, HAS_PENDING_EXCEPTION);
  thread->set_vm_result(obj);
JRT_END

// multianewarray for 2 dimensions
JRT_ENTRY(void, OptoRuntime::multianewarray2_C(klassOop elem_type, int len1, int len2, JavaThread *thread))
#ifndef PRODUCT
  _multi2_ctr++;                // multianewarray for 1 dimension
#endif
  assert(check_compiled_frame(thread,false,false,false), "incorrect caller");
  assert(oop(elem_type)->is_klass(), "not a class");
  jint dims[2];
  dims[0] = len1;
  dims[1] = len2;
  oop obj = arrayKlass::cast(elem_type)->multi_allocate(2, dims, +1, THREAD);
  deoptimize_caller_frame(thread, HAS_PENDING_EXCEPTION);
  thread->set_vm_result(obj);
JRT_END

// multianewarray for 3 dimensions
JRT_ENTRY(void, OptoRuntime::multianewarray3_C(klassOop elem_type, int len1, int len2, int len3, JavaThread *thread))
#ifndef PRODUCT
  _multi3_ctr++;                // multianewarray for 1 dimension
#endif
  assert(check_compiled_frame(thread,false,false,false), "incorrect caller");
  assert(oop(elem_type)->is_klass(), "not a class");
  jint dims[3];
  dims[0] = len1;
  dims[1] = len2;
  dims[2] = len3;
  oop obj = arrayKlass::cast(elem_type)->multi_allocate(3, dims, +1, THREAD);
  deoptimize_caller_frame(thread, HAS_PENDING_EXCEPTION);
  thread->set_vm_result(obj);
JRT_END

// multianewarray for 4 dimensions
JRT_ENTRY(void, OptoRuntime::multianewarray4_C(klassOop elem_type, int len1, int len2, int len3, int len4, JavaThread *thread))
#ifndef PRODUCT
  _multi4_ctr++;                // multianewarray for 1 dimension
#endif
  assert(check_compiled_frame(thread,false,false,false), "incorrect caller");
  assert(oop(elem_type)->is_klass(), "not a class");
  jint dims[4];
  dims[0] = len1;
  dims[1] = len2;
  dims[2] = len3;
  dims[3] = len4;
  oop obj = arrayKlass::cast(elem_type)->multi_allocate(4, dims, +1, THREAD);
  deoptimize_caller_frame(thread, HAS_PENDING_EXCEPTION);
  thread->set_vm_result(obj);
JRT_END

// multianewarray for 5 dimensions
JRT_ENTRY(void, OptoRuntime::multianewarray5_C(klassOop elem_type, int len1, int len2, int len3, int len4, int len5, JavaThread *thread))
#ifndef PRODUCT
  _multi5_ctr++;                // multianewarray for 1 dimension
#endif
  assert(check_compiled_frame(thread,false,false,false), "incorrect caller");
  assert(oop(elem_type)->is_klass(), "not a class");
  jint dims[5];
  dims[0] = len1;
  dims[1] = len2;
  dims[2] = len3;
  dims[3] = len4;
  dims[4] = len5;
  oop obj = arrayKlass::cast(elem_type)->multi_allocate(5, dims, +1, THREAD);
  deoptimize_caller_frame(thread, HAS_PENDING_EXCEPTION);
  thread->set_vm_result(obj);
JRT_END

const TypeFunc *OptoRuntime::new_Type() {
  // create input type (domain)
  const Type **fields = TypeTuple::fields(1);
  fields[TypeFunc::Parms+0] = TypeInstPtr::NOTNULL; // Klass to be allocated
  const TypeTuple *domain = TypeTuple::make(TypeFunc::Parms+1, fields);

  // create result type (range)
  fields = TypeTuple::fields(1);
  fields[TypeFunc::Parms+0] = TypeRawPtr::NOTNULL; // Returned oop

  const TypeTuple *range = TypeTuple::make(TypeFunc::Parms+1, fields);

  return TypeFunc::make(domain, range);
}


const TypeFunc *OptoRuntime::athrow_Type() {
  // create input type (domain)
  const Type **fields = TypeTuple::fields(1);
  fields[TypeFunc::Parms+0] = TypeInstPtr::NOTNULL; // Klass to be allocated
  const TypeTuple *domain = TypeTuple::make(TypeFunc::Parms+1, fields);

  // create result type (range)
  fields = TypeTuple::fields(0);

  const TypeTuple *range = TypeTuple::make(TypeFunc::Parms+0, fields);

  return TypeFunc::make(domain, range);
}


const TypeFunc *OptoRuntime::new_typeArray_Type() {
  // create input type (domain)
  const Type **fields = TypeTuple::fields(2);
  fields[TypeFunc::Parms+0] = TypeInt::INT;       // element type
  fields[TypeFunc::Parms+1] = TypeInt::INT;       // array size
  const TypeTuple *domain = TypeTuple::make(TypeFunc::Parms+2, fields);

  // create result type (range)
  fields = TypeTuple::fields(1);
  fields[TypeFunc::Parms+0] = TypeRawPtr::NOTNULL; // Returned oop

  const TypeTuple *range = TypeTuple::make(TypeFunc::Parms+1, fields);

  return TypeFunc::make(domain, range);
}

const TypeFunc *OptoRuntime::new_objArray_Type() {
  // create input type (domain)
  const Type **fields = TypeTuple::fields(2);
  fields[TypeFunc::Parms+0] = TypeInstPtr::NOTNULL;   // element klass
  fields[TypeFunc::Parms+1] = TypeInt::INT;       // array size
  const TypeTuple *domain = TypeTuple::make(TypeFunc::Parms+2, fields);

  // create result type (range)
  fields = TypeTuple::fields(1);
  fields[TypeFunc::Parms+0] = TypeRawPtr::NOTNULL; // Returned oop

  const TypeTuple *range = TypeTuple::make(TypeFunc::Parms+1, fields);

  return TypeFunc::make(domain, range);
}

const TypeFunc *OptoRuntime::multianewarray_Type(int ndim) {
  // create input type (domain)
  const int nargs = ndim + 1;
  const Type **fields = TypeTuple::fields(nargs);
  fields[TypeFunc::Parms+0] = TypeInstPtr::NOTNULL;   // element klass
  for( int i = 1; i < nargs; i++ )
    fields[TypeFunc::Parms + i] = TypeInt::INT;       // array size
  const TypeTuple *domain = TypeTuple::make(TypeFunc::Parms+nargs, fields);

  // create result type (range)
  fields = TypeTuple::fields(1);
  fields[TypeFunc::Parms+0] = TypeRawPtr::NOTNULL; // Returned oop
  const TypeTuple *range = TypeTuple::make(TypeFunc::Parms+1, fields);

  return TypeFunc::make(domain, range);
}

const TypeFunc *OptoRuntime::multianewarray1_Type() {
  return multianewarray_Type(1);
}

const TypeFunc *OptoRuntime::multianewarray2_Type() {
  return multianewarray_Type(2);
}

const TypeFunc *OptoRuntime::multianewarray3_Type() {
  return multianewarray_Type(3);
}

const TypeFunc *OptoRuntime::multianewarray4_Type() {
  return multianewarray_Type(4);
}

const TypeFunc *OptoRuntime::multianewarray5_Type() {
  return multianewarray_Type(5);
}

const TypeFunc *OptoRuntime::uncommon_trap_Type() {
  // create input type (domain)
  const Type **fields = TypeTuple::fields(1);
  // symbolOop name of class to be loaded
  fields[TypeFunc::Parms+0] = TypeInt::INT; 
  const TypeTuple *domain = TypeTuple::make(TypeFunc::Parms+1, fields);

  // create result type (range)
  fields = TypeTuple::fields(0);
  const TypeTuple *range = TypeTuple::make(TypeFunc::Parms+0, fields);

  return TypeFunc::make(domain, range);
}

//------------------------------resolving of calls ----------------------------
// Generalized resolve method for static, virtual, and optimized virtual calls.
//

// Inline caches exist only in compiled code
JRT_ENTRY(address, OptoRuntime::handle_wrong_method_ic_miss(JavaThread* thread))
#ifndef PRODUCT
  _ic_miss_ctr++;               // inline cache miss in compiled code
#endif
  assert(check_compiled_frame(thread, true,false,false), "incorrect caller");
  ResourceMark rm(thread);  
  CallInfo call_info;
  Bytecodes::Code bc;  

  // receiver is NULL for static calls. An exception is thrown for NULL receivers for non-static calls
  Handle receiver = find_callee_info(thread, bc, call_info, CHECK_0);  
  methodHandle callee_method = call_info.selected_method();

#ifndef PRODUCT
  // Statistics & Tracing
  if (TraceCallFixup) tty->print_cr(" %d handle_wrong_method_ic_miss", _ic_miss_ctr);

  if (CountICMisses) {
    MutexLocker m(VMStatistic_lock);
    _nof_ic_misses++;
  }
  if (TraceICmisses) { tty->print_cr("%d. IC miss: ", _nof_ic_misses); }

  if (ICMissHistogram) {
    RegisterMap reg_map(thread, false);
    frame f = thread->last_frame().real_sender(&reg_map);// skip runtime stub
    trace_ic_miss(f.pc());
  }
#endif

  // Update inline cache to megamorphic. Skip update if caller has been deoptimized or we 
  // are called from interpreted)
  { MutexLocker ml_patch (CompiledIC_lock);
    RegisterMap reg_map(thread, false);
    frame caller_frame = thread->last_frame().sender(&reg_map);    
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

  // Return methodOop through TLS  
  thread->set_vm_result(callee_method());
  // Return address to jump to
  return methodOopDesc::verified_code_entry(callee_method);
JRT_END


JRT_ENTRY(address, OptoRuntime::handle_wrong_method(JavaThread* thread))    
#ifndef PRODUCT
  _wrong_method_ctr++;          // uninitialized call site
  if (TraceCallFixup) { 
    tty->print_cr("%d handle_wrong_method", _wrong_method_ctr);
  }
#endif
  ResourceMark rm(thread);  
  RegisterMap reg_map(thread, false);
  // Check if the caller is a compiled method or an I2C adapter
  // Note: We can get an deoptimized frame on the stack - if we first get
  // into a lazy_c2i_adapter stub - that will deoptimize the calling frame.
  assert(check_compiled_frame(thread, true, false, true), "incorrect caller");
  frame c2r_frame = thread->last_frame();  
  frame caller_frame = c2r_frame.sender(&reg_map);
  
  //Force resolving of caller (if we called from compiled frame)        
  if (caller_frame.is_compiled_frame()) {
    SharedRuntime::reresolve_call_site(caller_frame);   
  }
  
  methodHandle callee_method = find_callee_method(thread, CHECK_0);

  thread->set_vm_result(callee_method());  
  // Return address to jump to
  return methodOopDesc::verified_code_entry(callee_method);
JRT_END


void OptoRuntime::inner_resolve_helper(JavaThread *thread, bool is_virtual, bool is_optimized, methodHandle &callee_method, TRAPS) {
  
  CallInfo call_info;
  Bytecodes::Code bc;

  // receiver is NULL for static calls. An exception is thrown for NULL
  // receivers for non-static calls
  Handle receiver = find_callee_info(thread, bc, call_info, CHECK);
  callee_method = call_info.selected_method();
  
  assert((!is_virtual && bc == Bytecodes::_invokestatic) ||
         ( is_virtual && bc != Bytecodes::_invokestatic), "inconsistent bytecode");

#ifndef PRODUCT
  if (TraceCallFixup) {
    ResourceMark rm(thread);
    tty->print("resolving %s%s call to", (is_optimized) ? "optimized " : "", (is_virtual) ? "virtual" : "static");
    callee_method->print_short_name(tty);
    tty->print_cr(" code: " INTPTR_FORMAT, callee_method->code());                  
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
  
  if (is_virtual) {
    // virtual call
    assert(receiver.not_null(), "sanity check");
    bool static_bound = Klass::can_be_statically_bound(call_info.resolved_method()());
    KlassHandle kh (THREAD, receiver->klass());
    CompiledIC::compute_monomorphic_entry(callee_method, kh, is_optimized, static_bound, virtual_call_info, CHECK);
    entry = virtual_call_info.entry();
    oop_result = virtual_call_info.cached_oop();
  } else {
    // static call
    CompiledStaticCall::compute_entry(callee_method, static_call_info);    
    entry = static_call_info.entry();
    oop_result = Handle(THREAD, callee_method()); // Only used when calling interpreter
  }      

  // Make sure the callee nmethod does not get deoptimized and removed before
  // we are done patching the code. callee_nm is NULL if we are calling an 
  // interpreted method
  nmethodLocker nl_callee(callee_method->code());

  // grab lock, check for deoptimization and potentially patch caller
  { MutexLocker ml_patch(CompiledIC_lock);
    // Reread frame since deoptimization could have happened while we
    // waited for the lock
    // If the frame has been deoptimized, or was initially deoptimized,
    // then this paragraph of code is a big no-op.
    RegisterMap map(thread, false);
    frame caller_frame = thread->last_frame().sender(&map);  
    if (!caller_frame.is_deoptimized_frame()) { 
      if (is_virtual) {
        CompiledIC* inline_cache = CompiledIC_before(caller_frame.pc());  
        if (inline_cache->is_clean()) inline_cache->set_to_monomorphic(virtual_call_info);
      } else {
        CompiledStaticCall* ssc = compiledStaticCall_before(caller_frame.pc());            
        if(ssc->is_clean())  ssc->set(static_call_info);      
      }
    }
  }
}

address OptoRuntime::resolve_helper(JavaThread *thread, bool is_virtual, bool is_optimized, TRAPS) {
  ResourceMark rm(thread);
  assert(!(!is_virtual && is_optimized), "cannot be optimized static call");
  
  // Make sure the calling nmethod is not getting deoptimized and removed 
  // before we are done with it.
  RegisterMap cbl_map(thread, false);
  frame caller = thread->last_frame().sender(&cbl_map);  

  methodHandle callee_method;
  if (caller.is_deoptimized_frame()) {
    // DeoptimizeALot in conjunction with safepoints causes this occasionally
    // The caller is no longer an nmethod, but the deopt_blob.
    // The inner_resolve_helper code recognizes this case safely.
    inner_resolve_helper(thread, is_virtual, is_optimized, callee_method, CHECK_0);
  } else {
    // usual case:  we must resolve the callee AND patch the caller
    assert(check_compiled_frame(thread,false,false,false), "incorrect caller");
    // We must lock it down the caller (a normal nmethod) while we patch it.
    // Otherwise it might deoptimize under us.
    nmethodLocker nm_caller(caller.pc());
    inner_resolve_helper(thread, is_virtual, is_optimized, callee_method, CHECK_0);
  }

  // Call to verified entrypoint (we done an exact resolving)  
  thread->set_vm_result(callee_method());  
  // Return address to jump to
  return methodOopDesc::verified_code_entry(callee_method);
}


// resolve a static call and patch code
JRT_ENTRY(address, OptoRuntime::resolve_static_call_C(JavaThread *thread ))  
#ifndef PRODUCT
  _resolve_static_ctr++;        // uninitialized static call site
#endif
  return resolve_helper(thread, false, false, CHECK_0);
JRT_END


// resolve virtual call and update inline cache to monomorphic
JRT_ENTRY(address, OptoRuntime::resolve_virtual_call_C(JavaThread *thread ))
#ifndef PRODUCT
  _resolve_virtual_ctr++;       // uninitialized virtual call site
#endif
  return resolve_helper(thread, true, false, CHECK_0);
JRT_END


// Resolve a virtual call that can be statically bound (e.g., always
// monomorphic, so it has no inline cache).  Patch code to resolved target.
JRT_ENTRY(address, OptoRuntime::resolve_opt_virtual_call_C(JavaThread *thread ))  
#ifndef PRODUCT
  _resolve_opt_virtual_ctr++;   // uninitialized optimized virtual call site
#endif
  return resolve_helper(thread, true, true, CHECK_0);
JRT_END


// Type used for stub generation for resolve_static_call_C,
// resolve_virtual_call_C, and resolve_opt_virtual_call_C
const TypeFunc *OptoRuntime::resolve_call_Type() {
  // create input type (domain)
  const Type **fields = TypeTuple::fields(0);
  const TypeTuple *domain = TypeTuple::make(TypeFunc::Parms,fields);

  // create result type (range)
  fields = TypeTuple::fields(1);
  fields[TypeFunc::Parms+0] = TypeRawPtr::BOTTOM; // jump target
  const TypeTuple *range = TypeTuple::make(TypeFunc::Parms+1,fields);

  return TypeFunc::make(domain,range);
}


# ifdef ENABLE_ZAP_DEAD_LOCALS
// Type used for stub generation for zap_dead_locals.
// No inputs or outputs
const TypeFunc *OptoRuntime::zap_dead_locals_Type() {
  // create input type (domain)
  const Type **fields = TypeTuple::fields(0);
  const TypeTuple *domain = TypeTuple::make(TypeFunc::Parms,fields);

  // create result type (range)
  fields = TypeTuple::fields(0);
  const TypeTuple *range = TypeTuple::make(TypeFunc::Parms,fields);

  return TypeFunc::make(domain,range);
}
# endif


//-----------------------------------------------------------------------------
// Monitor Handling
const TypeFunc *OptoRuntime::complete_monitor_enter_Type() {
  // create input type (domain)
  const Type **fields = TypeTuple::fields(2);
  fields[TypeFunc::Parms+0] = TypeInstPtr::NOTNULL;  // Object to be Locked
  fields[TypeFunc::Parms+1] = TypeRawPtr::BOTTOM;   // Address of stack location for lock
  const TypeTuple *domain = TypeTuple::make(TypeFunc::Parms+2,fields);

  // create result type (range)
  fields = TypeTuple::fields(0);

  const TypeTuple *range = TypeTuple::make(TypeFunc::Parms+0,fields);

  return TypeFunc::make(domain,range);
}

// Handles the uncommon case in locking, i.e., contention or an inflated lock.
JRT_ENTRY_FOR_MONITORENTER(void, OptoRuntime::complete_monitor_locking_C(oop obj, BasicLock* lock, JavaThread* thread))
#ifndef PRODUCT
  _mon_enter_ctr++;             // monitor enter slow
#endif
  Handle h_obj(THREAD, obj);
  ObjectSynchronizer::slow_enter(h_obj, lock, THREAD);

  deoptimize_caller_frame(thread, HAS_PENDING_EXCEPTION);
JRT_END


//-----------------------------------------------------------------------------
const TypeFunc *OptoRuntime::complete_monitor_exit_Type() {
  // create input type (domain)
  const Type **fields = TypeTuple::fields(2);
  fields[TypeFunc::Parms+0] = TypeInstPtr::NOTNULL;  // Object to be Locked
  fields[TypeFunc::Parms+1] = TypeRawPtr::BOTTOM;   // Address of stack location for lock
  const TypeTuple *domain = TypeTuple::make(TypeFunc::Parms+2,fields);

  // create result type (range)
  fields = TypeTuple::fields(0);

  const TypeTuple *range = TypeTuple::make(TypeFunc::Parms+0,fields);

  return TypeFunc::make(domain,range);
}

// Handles the uncommon cases of monitor unlocking in compiled code
JRT_LEAF(void, OptoRuntime::complete_monitor_unlocking_C(oop obj, BasicLock* lock))  
#ifndef PRODUCT
  _mon_exit_ctr++;              // monitor exit slow
#endif
  // Save and restore any pending_exception around the exception mark.
  // While the slow_exit must not throw an exception, we could come into
  // this routine with one set.
  Thread* THREAD = JavaThread::current();
  oop pending_excep = NULL;
  const char* pending_file;
  int pending_line;
  if (HAS_PENDING_EXCEPTION) {
    pending_excep = PENDING_EXCEPTION;
    pending_file  = THREAD->exception_file();
    pending_line  = THREAD->exception_line();
    CLEAR_PENDING_EXCEPTION;
  }

  {
    // Exit must be non-blocking, and therefore no exceptions can be thrown.
    EXCEPTION_MARK;
    ObjectSynchronizer::slow_exit(obj, lock, THREAD);
  }

  if (pending_excep != NULL) {
    THREAD->set_pending_exception(pending_excep, pending_file, pending_line);
  }
JRT_END


const TypeFunc* OptoRuntime::flush_windows_Type() {
  // create input type (domain)
  const Type** fields = TypeTuple::fields(1);
  fields[TypeFunc::Parms+0] = NULL; // void 
  const TypeTuple *domain = TypeTuple::make(TypeFunc::Parms, fields);

  // create result type
  fields = TypeTuple::fields(1);
  fields[TypeFunc::Parms+0] = NULL; // void 
  const TypeTuple *range = TypeTuple::make(TypeFunc::Parms, fields);

  return TypeFunc::make(domain, range);
}


const TypeFunc* OptoRuntime::modf_Type() {
  const Type **fields = TypeTuple::fields(2);
  fields[TypeFunc::Parms+0] = Type::FLOAT;
  fields[TypeFunc::Parms+1] = Type::FLOAT;
  const TypeTuple *domain = TypeTuple::make(TypeFunc::Parms+2, fields);

  // create result type (range)
  fields = TypeTuple::fields(1);
  fields[TypeFunc::Parms+0] = Type::FLOAT;

  const TypeTuple *range = TypeTuple::make(TypeFunc::Parms+1, fields);

  return TypeFunc::make(domain, range);
}

const TypeFunc *OptoRuntime::Math_D_D_Type() {
  // create input type (domain)
  const Type **fields = TypeTuple::fields(2);
  // symbolOop name of class to be loaded
  fields[TypeFunc::Parms+0] = Type::DOUBLE; 
  fields[TypeFunc::Parms+1] = Type::HALF; 
  const TypeTuple *domain = TypeTuple::make(TypeFunc::Parms+2, fields);

  // create result type (range)
  fields = TypeTuple::fields(2);
  fields[TypeFunc::Parms+0] = Type::DOUBLE;
  fields[TypeFunc::Parms+1] = Type::HALF;
  const TypeTuple *range = TypeTuple::make(TypeFunc::Parms+2, fields);

  return TypeFunc::make(domain, range);
}

const TypeFunc* OptoRuntime::Math_DD_D_Type() {
  const Type **fields = TypeTuple::fields(4);
  fields[TypeFunc::Parms+0] = Type::DOUBLE;
  fields[TypeFunc::Parms+1] = Type::HALF;
  fields[TypeFunc::Parms+2] = Type::DOUBLE;
  fields[TypeFunc::Parms+3] = Type::HALF;
  const TypeTuple *domain = TypeTuple::make(TypeFunc::Parms+4, fields);

  // create result type (range)
  fields = TypeTuple::fields(2);
  fields[TypeFunc::Parms+0] = Type::DOUBLE;
  fields[TypeFunc::Parms+1] = Type::HALF;
  const TypeTuple *range = TypeTuple::make(TypeFunc::Parms+2, fields);

  return TypeFunc::make(domain, range);
}


//-------------- arraycopy

const TypeFunc* OptoRuntime::primitive_arraycopy_Type() {
  // create input type (domain)
#ifdef _LP64
  int argcnt = 4;
#else
  int argcnt = 3;
#endif
  const Type **fields = TypeTuple::fields(argcnt);
  fields[TypeFunc::Parms+0] = TypeInstPtr::NOTNULL;   // src
  fields[TypeFunc::Parms+1] = TypeInstPtr::NOTNULL;   // dest
  fields[TypeFunc::Parms+2] = TypeX_X; // size in bytes
#ifdef _LP64
  fields[TypeFunc::Parms+3] = Type::HALF; // Other half of long length
#endif
  const TypeTuple *domain = TypeTuple::make(TypeFunc::Parms+argcnt, fields);

  // create result type
  fields = TypeTuple::fields(1);
  fields[TypeFunc::Parms+0] = NULL; // void 
  const TypeTuple *range = TypeTuple::make(TypeFunc::Parms, fields);
  return TypeFunc::make(domain, range);
}

const TypeFunc* OptoRuntime::oop_arraycopy_Type() {
  // create input type (domain)
  const Type **fields = TypeTuple::fields(3);          
  fields[TypeFunc::Parms+0] = TypeInstPtr::NOTNULL;   // src
  fields[TypeFunc::Parms+1] = TypeInstPtr::NOTNULL;   // dest
  fields[TypeFunc::Parms+2] = TypeInt::INT;       // size in oops
  const TypeTuple *domain = TypeTuple::make(TypeFunc::Parms+3, fields);

  // create result type
  fields = TypeTuple::fields(1);
  fields[TypeFunc::Parms+0] = NULL; // void 
  const TypeTuple *range = TypeTuple::make(TypeFunc::Parms, fields);
  return TypeFunc::make(domain, range);  
}

JRT_LEAF(void, OptoRuntime::primitive_arraycopy(HeapWord* src, HeapWord* dest, intptr_t len))
#ifndef PRODUCT
  _prim_array_copy_ctr++;       // Slow-path primitive array copy
#endif
  if (len == 0) {
    // Bailout early for zero length copies.
    return;
  }

  // memmove is not guaranteed to be word atomic, but that doesn't matter
  // for anything but an oop array, which is covered by oop_arraycopy.
  Memory::copy_bytes_overlapping(src, dest, len);
JRT_END

JRT_LEAF(void, OptoRuntime::oop_arraycopy(HeapWord* src, HeapWord* dest,
					  jint num))
#ifndef PRODUCT
  _oop_array_copy_ctr++;        // Slow-path oop array copy
#endif
  if (num == 0) {
    // Bailout early for zero length copies, as in objArrayKlass::copy_array().
    return;
  }
  Memory::copy_oops_overlapping_atomic((oop*) src, (oop*) dest, num);
  BarrierSet* bs = Universe::heap()->barrier_set();
  bs->write_ref_array(MemRegion(dest, dest + num));
JRT_END


//------------- Interpreter state access for on stack replacement
const TypeFunc* OptoRuntime::fetch_monitor_Type() {
  // create input type (domain)
  const Type **fields = TypeTuple::fields(3);
  fields[TypeFunc::Parms+0] = TypeInt::INT;       // index
  fields[TypeFunc::Parms+1] = TypeRawPtr::BOTTOM; // address of compiler lock
  fields[TypeFunc::Parms+2] = TypeRawPtr::BOTTOM; // address of interpreter lock array
  const TypeTuple *domain = TypeTuple::make(TypeFunc::Parms+3, fields);

  // create result type
  fields = TypeTuple::fields(1);
  fields[TypeFunc::Parms+0] = TypeInstPtr::NOTNULL; // locked oop
  const TypeTuple *range = TypeTuple::make(TypeFunc::Parms+1, fields);
  return TypeFunc::make(domain, range);  
}


JRT_LEAF(oop, OptoRuntime::fetch_monitor(int index, BasicLock *compile_lock, address locks_addrs))
  BasicObjectLock *lock;
  while(1) {
    lock = (BasicObjectLock*)locks_addrs;
    if (lock->obj()) {       // Avoid 'holes' in the monitor array
      if( !index ) break;
      index--;
    }
    locks_addrs = locks_addrs - wordSize * frame::interpreter_frame_monitor_size();
  }
  lock->lock()->move_to(lock->obj(),compile_lock);
  return lock->obj();  
JRT_END

methodHandle OptoRuntime::find_callee_method(JavaThread* thread, TRAPS) {
  ResourceMark rm(THREAD);
  // We need first to check if any Java activations (compiled, interpeted, deoptimized)
  // exist on the stack since last JavaCall.  If not, we need to get the target method 
  // from the JavaCall wrapper.    
  vframeStream vfst(thread, true);  // Do not skip any javaCalls
  if (vfst.at_end()) {    
    // No Java frames were found on stack since we did the JavaCall.
    // Hence the stack can only contain an entry_frame and possibly an 
    // i2c_adapter.  We need to find the target method from the stub frame.
    RegisterMap reg_map(thread, false);
    frame fr = thread->last_frame();
    assert(fr.is_c2runtime_frame(), "must be a runtimeStub");
    fr = fr.sender(&reg_map);
    while(!fr.is_entry_frame())  {
      assert(fr.is_c2i_frame() || fr.is_i2c_frame(), "sanity check");
      fr = fr.sender(&reg_map);
    }
    // fr is now pointing to the entry frame.    
    methodHandle callee_method = methodHandle(THREAD, fr.entry_frame_call_wrapper()->callee_method());        
    assert(fr.entry_frame_call_wrapper()->receiver() == NULL || !callee_method->is_static(), "non-null receiver for static call??");
    return callee_method;
  } else {
    Bytecodes::Code bc;
    CallInfo callinfo;
    find_callee_info_helper(thread, vfst, bc, callinfo, CHECK_(methodHandle()));
    return callinfo.selected_method();
  }
}

// Finds receiver, CallInfo (i.e. receiver method), and calling bytecode)
// for a call current in progress, i.e., arguments has been pushed on stack
// put callee has not been invoked yet.  Caller frame must be compiled.
Handle OptoRuntime::find_callee_info_helper(JavaThread* thread, vframeStream& vfst, Bytecodes::Code& bc, CallInfo& callinfo, TRAPS) {
  Handle receiver;
  Handle nullHandle;  //create a handy null handle for exception returns

  assert(!vfst.at_end(), "Java frame must exist");
  
  // Find caller and bci from vframe
  methodHandle caller (THREAD, vfst.method());
  int          bci    = vfst.bci();
  
  // Find bytecode
  Bytecode_invoke* bytecode = Bytecode_invoke_at(caller, bci);
  bc = bytecode->adjusted_invoke_code();
  int bytecode_index = bytecode->index();

  // Find receiver for non-static call    
  if (bc != Bytecodes::_invokestatic) {
    // This register map must be update since we need to find the receiver for
    // compiled frames. The receiver might be in a register.
    RegisterMap reg_map2(thread); 
    frame stubFrame   = thread->last_frame();
    // Caller-frame is either a compiled frame or an I2C adapter
    frame callerFrame = stubFrame.sender(&reg_map2); 

    methodHandle callee = bytecode->static_target(CHECK_(nullHandle));
    if (callee.is_null()) {
      THROW_(vmSymbols::java_lang_NoSuchMethodException(), nullHandle);
    }  
    // Retrieve from either a compiled frame or an I2C adapter
    receiver = Handle(THREAD, retrieve_receiver(callerFrame, &reg_map2));

    if (receiver.is_null()) {
      THROW_(vmSymbols::java_lang_NullPointerException(), nullHandle);
    }
  }      

  // Resolve method. This is parameterized by bytecode.  
  constantPoolHandle constants (THREAD, caller->constants());
  assert (receiver.is_null() || receiver->is_oop(), "wrong receiver");
  LinkResolver::resolve_invoke(callinfo, receiver, constants, bytecode_index, bc, CHECK_(nullHandle));

  #ifdef ASSERT
  // Check that the receiver klass is of the right subtype and that it is initialized for virtual calls  
  if (bc != Bytecodes::_invokestatic) {    
    assert(receiver.not_null(), "should have thrown exception");
    KlassHandle receiver_klass (THREAD, receiver->klass());
    klassOop rk = constants->klass_ref_at(bytecode_index, CHECK_(nullHandle));
                            // klass is already loaded
    KlassHandle static_receiver_klass (THREAD, rk);
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

// Finds receiver, CallInfo (i.e. receiver method), and calling bytecode)
// for a call current in progress, i.e., arguments has been pushed on stack
// put callee has not been invoked yet.  Used by: resolve virtual/static,
// vtable updates, etc.  Caller frame must be compiled.
Handle OptoRuntime::find_callee_info(JavaThread* thread, Bytecodes::Code& bc, CallInfo& callinfo, TRAPS) {
  ResourceMark rm(THREAD);

  // last java frame on stack (which includes native call frames)
  vframeStream vfst(thread, true);  // Do not skip and javaCalls

  return find_callee_info_helper(thread, vfst, bc, callinfo, CHECK_(Handle()));
}

// Suck receiver out of callers frame, i.e. find parameter 0 in callers 
// frame.  Consult ADLC for where parameter 0 is to be found.  Then 
// check local reg_map for it being a callee-save register or argument
// register, both of which are saved in the local frame.  If not found
// there, it must be an in-stack argument of the caller. 
// Note: caller.sp() points to callee-arguments
oop OptoRuntime::retrieve_receiver(frame caller, RegisterMap* reg_map) {
  assert(caller.is_i2c_frame() || caller.is_compiled_frame() || caller.is_deoptimized_frame(), "only works for compiled frames");

  // First consult the ADLC on where it puts parameter 0 for this signature.
  VMReg::Name reg = Matcher::find_receiver(true);
  oop r = *caller.oopmapreg_to_location(reg, reg_map);  
  assert( Universe::heap()->is_in_or_null(r), "bad receiver" );
  return r;
}


// creates c2i adapter and jumps at adapter.  Used by vtables
JRT_ENTRY(address, OptoRuntime::lazy_c2i_adapter_generation_C(JavaThread* thread))
#ifndef PRODUCT
  _c2i_adapter_ctr++;           // c2i adapter for vtables
#endif
  // It is possible to have the caller frame deoptimized.  Example:
  // _invokeinterface calls into megamorphic stub which enters lookup in the
  // VM.  On exit the thread may block and the caller may be deoptimized; the
  // megamorphic stub continues and calls lazy_c2i_adapter stub
  assert(check_compiled_frame(thread, true, false, true), "incorrect caller");
  
  methodHandle calleeMethod = find_callee_method(thread, CHECK_0);
  
  // Update to either c2i or nmethod entry (i.e., replace lazy stub with real stub)  
  thread->set_vm_result(calleeMethod());    
  // Return address to jump to
  return methodOopDesc::verified_code_entry(calleeMethod);
JRT_END

//-----------------------------------------------------------------------------
// implicit exception support. 

static void report_null_exception_in_code_cache(address exception_pc) {
  ResourceMark rm;
  CodeBlob* n = CodeCache::find_blob(exception_pc);
  if (n != NULL) {
    tty->print_cr("#");
    tty->print_cr("# HotSpot Runtime Error, null exception in generated code");
    tty->print_cr("#");
    tty->print_cr("# pc where exception happened = " INTPTR_FORMAT, exception_pc);

    if (n->is_nmethod()) {
      methodOop method = ((nmethod*)n)->method();
      tty->print_cr("# Method where it happened %s.%s ", Klass::cast(method->method_holder())->name()->as_C_string(), method->name()->as_C_string());
      tty->print_cr("#");
      if (ShowMessageBoxOnError && UpdateHotSpotCompilerFileOnError) { 
        const char* title    = "HotSpot Runtime Error";
        const char* question = "Do you want to exclude compilation of this method in future runs?";
        if (os::message_box(title, question)) {
          CompilerOracle::append_comment_to_file("");
          CompilerOracle::append_comment_to_file("Null exception in compiled code resulted in the following exclude");
          CompilerOracle::append_comment_to_file("");
          CompilerOracle::append_exclude_to_file(method);
          tty->print_cr("#");
          tty->print_cr("# %s has been updated to exclude the specified method", CompileCommandFile);
          tty->print_cr("#");
        }
      }
      fatal("Implicit null exception happened in compiled method");
    } else {
      n->print(); 
      fatal("Implicit null exception happened in generated stub");
    }
  }
  fatal("Implicit null exception at wrong place");
}


// The implicit null exception can happen only in vtablestub or in inline
// cache check of the compiled code. The caller must be compiled code as
// interpreter is doing explicit checks itself.
extern "C" void find(int x);

JRT_ENTRY(void, OptoRuntime::throw_null_exception_C(JavaThread* thread))
#ifndef PRODUCT
  _throw_null_ctr++;            // implicit null throw
#endif
  address exception_pc = thread->saved_exception_pc(); 
  VtableStub* vtstub = VtableStubs::stub_containing(exception_pc);
  if (vtstub != NULL && vtstub->is_abstract_method_error(exception_pc)) {
#ifdef ASSERT
    if (WizardMode) tty->print_cr("Implicit AbstractMethodError in %s stub", (vtstub->is_itable_stub()? "Itable": "Vtable"));
#endif
    THROW(vmSymbols::java_lang_AbstractMethodError());
  }
#ifdef ASSERT
  bool ok =  false;
  if (vtstub != NULL) {
    // happy
    ok = vtstub->is_null_pointer_exception(exception_pc);
    if (WizardMode) tty->print_cr("Implicit NULL exception in %s stub", (vtstub->is_itable_stub()? "Itable": "Vtable"));
  } else if (CodeCache::contains(exception_pc)) {
    // only inline cache handled
    CodeBlob* cb = CodeCache::find_blob(exception_pc);
    assert (cb != NULL, "it should still be there");
    if (cb->is_nmethod()) {
      ok = ((nmethod*)cb)->inlinecache_check_contains(exception_pc);  
    } else if (cb->is_c2i_adapter()) {
      ok = ((C2IAdapter*)cb)->inlinecache_check_contains(exception_pc);  
    } else {
      // Got null pointer exception at wrong place
      ok = false;
    }              
    if (WizardMode) tty->print_cr("Implicit NULL exception in inline cache");
  }
  if (!ok) {
    report_null_exception_in_code_cache(exception_pc);
  }
  // Allow compiled and c2i adapters
  assert(check_compiled_frame(thread, false, true, false) || VtableStubs::contains(exception_pc), "incorrect caller");
#endif
  THROW(vmSymbols::java_lang_NullPointerException());
JRT_END


JRT_ENTRY(void, OptoRuntime::throw_exception_C(oop obj, JavaThread* thread))
  // Note: For now we re-create NullPointer & Arithmetic
  //       exceptions in case they are identical with the
  //       constant 'signal' values in Universe. This is
  //       only a fix for the case where we actually end
  //       up here with such an exception. Eventually this
  //       routine should go away and the general solution
  //       (implicit exceptions) should do everything right.
  if (obj == Universe::null_ptr_exception_instance()) {
    THROW(vmSymbols::java_lang_NullPointerException());
  } else if (obj == Universe::arithmetic_exception_instance()) {
    THROW_MSG(vmSymbols::java_lang_ArithmeticException(), "/ by zero");
  } else {
    THROW_OOP(obj);
  }
JRT_END

// --- Currently not used by Compiler2 (but referenced in StubGenerator)

JRT_ENTRY(void, OptoRuntime::throw_div0_exception_C(JavaThread* thread))  
  ShouldNotReachHere();
  THROW_MSG(vmSymbols::java_lang_ArithmeticException(), "/ by zero");
JRT_END


JRT_ENTRY(void, OptoRuntime::throw_stack_overflow_error_C(JavaThread* thread)) 
  Exceptions::throw_stack_overflow_exception(THREAD_AND_LOCATION);
JRT_END

//-------------------------------------------------------------------------------------
// vtable support

JRT_ENTRY(void, OptoRuntime::handle_abstract_method_C(JavaThread* thread))
  warning("OptoRuntime::handle_abstract_method_C: untested code");
  assert(check_compiled_frame(thread, true,false,false), "incorrect caller");
#ifdef ASSERT  
  methodHandle calleeMethod = OptoRuntime::find_callee_method(thread, CHECK);
  assert(calleeMethod->is_abstract(), "must be an abstract method");
#endif
  THROW(vmSymbols::java_lang_AbstractMethodError());
JRT_END


bool OptoRuntime::is_callee_saved_register(MachRegisterNumbers reg) {
  assert(reg >= 0 && reg < _last_Mach_Reg, "must be a machine register");
  switch (register_save_policy[reg]) {
    case 'C': return false; //SOC
    case 'E': return true ; //SOE
    case 'N': return false; //NS
    case 'A': return false; //AS
  }
  ShouldNotReachHere();
  return false;
}

//-----------------------------------------------------------------------
// Exceptions
// 

void trace_exception(oop exception_oop, address exception_pc, const char* msg) PRODUCT_RETURN;

const TypeFunc *OptoRuntime::handle_exception_Type() {
  // create input type (domain)
  const Type **fields = TypeTuple::fields(0);  
  const TypeTuple *domain = TypeTuple::make(TypeFunc::Parms+0,fields);

  // create result type (range)
  fields = TypeTuple::fields(1);
  fields[TypeFunc::Parms+0] = TypeRawPtr::BOTTOM; // Returned result 
  const TypeTuple *range = TypeTuple::make(TypeFunc::Parms+1, fields);

  return TypeFunc::make(domain, range);
}

address compute_compiled_exc_handler(nmethod* nm, address ret_pc, Handle exception, bool force_unwind);

// We are entering here from exception_blob
// If there is a compiled exception handler in this method, we will continue there;
// otherwise we will unwind the stack and continue at the caller of top frame method
JRT_ENTRY(address, OptoRuntime::handle_exception_C(JavaThread* thread))
#ifndef PRODUCT
  _find_handler_ctr++;          // find exception handler
#endif

  // Do not confuse exception_oop with pending_exception. The exception_oop
  // is only used to pass arguments into the method. Not for general
  // exception handling.  DO NOT CHANGE IT to use pending_exception, since
  // the runtime stubs checks this on exit.
  assert(thread->exception_oop() != NULL, "exception oop is found");  

  Handle exception(thread, thread->exception_oop());

  if (TraceExceptions) {
    ThreadCritical tc;
    trace_exception(exception(), thread->exception_pc(), ""); 
  }

  #ifdef ASSERT
    if (!(exception->is_a(SystemDictionary::throwable_klass()))) {
      // should throw an exception here
      ShouldNotReachHere();
    }
  #endif


  // new exception handling: this method is entered only from adapters
  // exceptions from compiled java methods are handled in compiled code
  // using rethrow node

  address pc = thread->safepoint_state()->compute_adjusted_pc(thread->exception_pc());
  CodeBlob* cb = CodeCache::find_blob(pc);
  address handler_address = NULL;
  if (cb->is_i2c_adapter() || cb->is_c2i_adapter() || cb->is_osr_adapter() || cb->is_native_method()) {
    // Setup exception info. in thread. Used by stub
    RegisterMap rm(thread, false); // do not update map; need map in case that caller is deoptimized
    frame exception_blob_frame = thread->last_frame();
    frame caller_frame = exception_blob_frame.sender(&rm,OptoRuntime::exception_blob());  

    // there is no exception handler in adapter frames
    // Setup for stack unwinding the callee-save registers

    RegisterMap reg_map(thread);
    unwind_stack(thread, cb, caller_frame, &reg_map);

    // unwinding has setup all exception related TLS for adapters;
    handler_address = NULL; // means stack unwinding done in runtime, leave this frame
  } else {
    assert (cb->is_nmethod(), "");
    // we are switching to old paradigm: search for exception handler in caller_frame
    // instead in exception handler of caller_frame.sender()
    nmethod* nm = (nmethod*)cb;

    if (jvmdi::enabled() && FullSpeedJVMDI) {
      // "Full-speed catching" is not necessary here,
      // since we're notifying the VM on every catch.
      address badPC = (address)-1;
      thread->set_last_exception_pc(badPC);
      thread->set_exception_pc(badPC);
      thread->set_exception_handler_pc(badPC);
      thread->set_exception_stack_size(0);
      thread->set_exception_oop(exception());
      deoptimize_caller_frame(thread, true);
      // Return an arbitrary non-null value to the exception blob.
      // (Don't return NULL, or it will unwind rather than catch.)
      return badPC;
    }

    // Check the stack guard pages.  If enabled, look for handler in this frame;
    // otherwise, forcibly unwind the frame.
    //
    // 4826555: use default current sp for reguard_stack instead of &nm: it's more accurate.
    bool force_unwind = !thread->reguard_stack();
    handler_address =
      force_unwind ? NULL : nm->handler_for_exception_and_pc(exception, pc);

    if (handler_address == NULL) {
      handler_address = compute_compiled_exc_handler(nm, pc, exception, force_unwind);
      assert (handler_address != NULL, "must have compiled handler");
      // Update the exception cache only when the unwind was not forced.
      if (!force_unwind) {
        nm->add_handler_for_exception_and_pc(exception,pc,handler_address);
      }
    } else {
      assert(handler_address == compute_compiled_exc_handler(nm, pc, exception, force_unwind), "Must be the same");
    }

    // if the exception was thrown within a temporary codebuffer, keep it inside:
    handler_address = thread->safepoint_state()->maybe_capture_pc(handler_address);
    thread->set_last_exception_pc(pc);    
    thread->set_exception_pc(pc);
    thread->set_exception_handler_pc(handler_address);
    thread->set_exception_stack_size(0);
  }

  // Restore correct return pc.  Was saved above.
  thread->set_exception_oop(exception());

  return handler_address;

JRT_END


// ret_pc points into caller; we are returning caller's exception handler
// for given exception
address compute_compiled_exc_handler(nmethod* nm, address ret_pc, Handle exception, bool force_unwind) {
  assert(nm != NULL, "must exist");  
  ResourceMark rm;
  
  SimpleScopeDesc ssd(nm,ret_pc,true);

  // determine handler bci, if any
  EXCEPTION_MARK;
  int handler_bci = -1;
  if (!force_unwind) {
    bool should_repeat;
    int bci = ssd.bci();
    do {
      should_repeat = false;
      // exception handler lookup
      KlassHandle ek (THREAD, exception->klass());
      handler_bci = ssd.method()->fast_exception_handler_bci_for(ek, bci, true, THREAD);
      if (HAS_PENDING_EXCEPTION) {
  	// We threw an exception while trying to find the exception handler.
  	// Transfer the new exception to the exception handle which will
  	// be set into thread local storage, and do another lookup for an
  	// exception handler for this exception, this time starting at the
  	// BCI of the exception handler which caused the exception to be
  	// thrown (bugs 4307310 and 4546590).
  	exception = Handle(THREAD, PENDING_EXCEPTION);
  	CLEAR_PENDING_EXCEPTION;
  	if (handler_bci >= 0) {
  	  bci = handler_bci;
  	  should_repeat = true;
  	}
      }
    } while (should_repeat == true);
  }

  // found handling method => lookup exception handler
  int catch_pco = ret_pc - nm->instructions_begin();

  ExceptionHandlerTable table(nm);
  HandlerTableEntry *t = table.entry_for(catch_pco, handler_bci);
  if (t == NULL && handler_bci != -1) {
    // Allow abbreviated catch tables, for now
    // The idea is to allow a method to materialize its exceptions
    // without committing to the exact routing of exceptions.
    t = table.entry_for(catch_pco, -1);
  }
#ifdef ASSERT
  if (t == NULL) {
    tty->print_cr("MISSING EXCEPTION HANDLER for pc " INTPTR_FORMAT " and handler bci %d", ret_pc, handler_bci);
    tty->print_cr("   Exception:");
    exception->print();
    tty->cr();
    tty->print_cr(" Compiled exception table :");
    table.print();
    nm->print_code();
  }
#endif
  assert( t, "missing exception handler" );

  return nm->instructions_begin() + t->pco();
}

//------------------------------rethrow----------------------------------------
// We get here after compiled code has executed a 'RethrowNode'.  The callee 
// is either throwing or rethrowing an exception.  The callee-save registers
// have been restored, synchronized objects have been unlocked and the callee
// stack frame has been removed.  The return address was passed in.
// Exception oop is passed as the 1st argument.  This routine is then called 
// from the stub.  On exit, we know where to jump in the caller's code.  
// After this C code exits, the stub will pop his frame and end in a jump 
// (instead of a return).  We enter the caller's default handler.
//
// This must be JRT_LEAF: 
//     - caller will not change its state as we cannot block on exit,
//       therefore raw_exception_handler_for_return_address is all it takes
//       to handle deoptimized blobs
//
// However, there needs to be a safepoint check in the middle!  So compiled
// safepoints are completely watertight.
//
// Thus, it cannot be a leaf since it contains the No_GC_Verifier. 
//  
// *THIS IS NOT RECOMMENDED PROGRAMMING STYLE*
// 
address OptoRuntime::rethrow_C(oop exception, JavaThread* thread, address ret_pc) {
#ifndef PRODUCT
  _rethrow_ctr++;               // count rethrows
#endif
  assert (exception != NULL, "should have thrown a NULLPointerException");
#ifdef ASSERT
  if (!(exception->is_a(SystemDictionary::throwable_klass()))) {
    // should throw an exception here
    ShouldNotReachHere();
  }
#endif

  // Force safepoint check.  Suppose we are trying to reach a safepoint, and
  // we have put illegal-instructions at all safepoints in the caller, and we
  // are rolling forwards to a safepoint.  We can escape the caller by calling
  // a Rethrow - which puts us here.
  if (SafepointSynchronize::do_call_back()) {
    HandleMark hm(thread);
    // If we're going to block, we need to preserve the exception oop
    Handle save_exception(thread, exception);

    SafepointSynchronize::block(thread);
    // Get pc again, since a deoptimization may have happened
    RegisterMap reg_map(thread, false); // do not update map; need map in case that caller
                                        // is deoptimized
    frame stub_frame = thread->last_frame();
    frame caller_frame = stub_frame.sender(&reg_map);  
    ret_pc = caller_frame.pc();
    exception = save_exception();
  }

  thread->set_vm_result(exception);

  // Frame not compiled (handles deoptimization blob)
  return SharedRuntime::raw_exception_handler_for_return_address(ret_pc);
}


const TypeFunc *OptoRuntime::rethrow_Type() {
  // create input type (domain)
  const Type **fields = TypeTuple::fields(1);  
  fields[TypeFunc::Parms+0] = TypeInstPtr::NOTNULL; // Exception oop
  const TypeTuple *domain = TypeTuple::make(TypeFunc::Parms+1,fields);

  // create result type (range)
  fields = TypeTuple::fields(1);
  fields[TypeFunc::Parms+0] = TypeInstPtr::NOTNULL; // Exception oop
  const TypeTuple *range = TypeTuple::make(TypeFunc::Parms+1, fields);

  return TypeFunc::make(domain, range);
}


void OptoRuntime::deoptimize_caller_frame(JavaThread *thread, bool doit) {
  // Deoptimize frame
  if (doit) {
    // Called from within the owner thread, so no need for safepoint
    RegisterMap reg_map(thread);
    frame stub_frame = thread->last_frame();
    assert(stub_frame.is_c2runtime_frame() || exception_blob()->contains(stub_frame.pc()), "sanity check");
    frame caller_frame = stub_frame.sender(&reg_map); 

    VM_DeoptimizeFrame deopt(thread, caller_frame.id());
    VMThread::execute(&deopt);
  }
}



// Called by handle_exception_C. Unwind stack one frame
// Description:
//    This is invoked only for i2c and c2i adapter until the rethrow handler
//    for the adapters is implemented.
//  - Search the code blob for the adapter; frame belongs to the adapter
//  - Get the sender of the adapter and get all the registers addresses that he has defined
//    (typically saved inside the adapter frame. i.e. addresses poiting into the adapter frame)
//  - The last frame belongs to the exception blob
//  - We copy all the found registers locations into the exception blob frame,
//    because we are going to restore callee saved registers out of 
//    exception blob instead out of adapter frame; the exception blob is using the address
//    of each callee saved register to locate the value of that register
//  (Basically we are copying register values from adapter into the exception blob)
void OptoRuntime::unwind_stack(JavaThread *thread, CodeBlob *code, frame fr, RegisterMap* reg_map) { 
  EXCEPTION_MARK;
  ResourceMark rm(THREAD);
  address exception_pc = thread->exception_pc();   
    
  // Unlock synchronized methods
  assert(code == CodeCache::find_blob(exception_pc),"Must be the same");
  assert (code != NULL, "cannot find nmethod for given pc");
  assert(code->is_i2c_adapter() || code->is_c2i_adapter() || code->is_nmethod() || code->is_osr_adapter(), "sanity check");  

  frame sender_frame = fr.sender(reg_map,code);

  pd_unwind_stack(thread, fr, reg_map);
  
  // Set exception_pc and handler_pc for next frame. Stack size for
  // current frame. Used by stub
  address caller_return_address  = sender_frame.pc();
  address caller_handler_address = SharedRuntime::raw_exception_handler_for_return_address(caller_return_address);
  
  // We need to keep track of the exception_pc for the current frame,
  // so we can do the right stack traversal.  The return_pc will point
  // to the ExceptionBlob(), but the frame belongs to the
  // last_exception_pc()
  thread->set_last_exception_pc(exception_pc);    
  thread->set_exception_pc(caller_return_address);
  thread->set_exception_handler_pc(caller_handler_address);
  assert(code->frame_size() >= 0, "Compiled by C1: do not use");

  // Do not use 'code->frame_size()' because I2C adapter frames have a
  // variable size.  The correct sender is computed in fr.sender above.
  int frame_size_in_words = sender_frame.sp() - fr.sp();
  frame_size_in_words = fr.pd_compute_variable_size(frame_size_in_words,code);
  thread->set_exception_stack_size(frame_size_in_words * wordSize);  

}


//-----------------------------------------------------------------------------
// JVMPI support
const TypeFunc *OptoRuntime::jvmpi_method_entry_Type() {
  // create input type (domain)
  const Type **fields = TypeTuple::fields(2);
  fields[TypeFunc::Parms+0] = TypeInstPtr::NOTNULL;  // methodOop;    Method we are entering
  fields[TypeFunc::Parms+1] = TypeInstPtr::BOTTOM;   // oop;          Receiver
  // // The JavaThread* is passed to each routine as the last argument
  // fields[TypeFunc::Parms+2] = TypeRawPtr::NOTNULL;  // JavaThread *; Executing thread
  const TypeTuple *domain = TypeTuple::make(TypeFunc::Parms+2,fields);

  // create result type (range)
  fields = TypeTuple::fields(0);

  const TypeTuple *range = TypeTuple::make(TypeFunc::Parms+0,fields);

  return TypeFunc::make(domain,range);
}

// Make the signature of jvmpi_method_entry_C and jvmpi_method_exit_C the same for simplicity and consistency.
const TypeFunc *OptoRuntime::jvmpi_method_exit_Type() {
  // create input type (domain)
  const Type **fields = TypeTuple::fields(2);
  fields[TypeFunc::Parms+0] = TypeInstPtr::NOTNULL;  // methodOop;    Method we are exiting
  fields[TypeFunc::Parms+1] = TypeInstPtr::BOTTOM;   // oop;          dummy
  const TypeTuple *domain = TypeTuple::make(TypeFunc::Parms+2,fields);

  // create result type (range)
  fields = TypeTuple::fields(0);

  const TypeTuple *range = TypeTuple::make(TypeFunc::Parms+0,fields);

  return TypeFunc::make(domain,range);
}

// TEMPORARY: jvmpi_method_entry_C and jvmpi_method_exit_C was cloned from sharedRuntime.cpp
// but with different signature
// Must be entry as it may lock when acquring the jni_id of the method
JRT_ENTRY (void, OptoRuntime::jvmpi_method_entry_C(methodOop method, oop receiver, JavaThread* thread))
  GC_locker::lock();
  if (jvmpi::is_event_enabled(JVMPI_EVENT_METHOD_ENTRY2)) {
    jvmpi::post_method_entry2_event(method, receiver);
  } 
  if (jvmpi::is_event_enabled(JVMPI_EVENT_METHOD_ENTRY)) {
    jvmpi::post_method_entry_event(method);
  } 
  GC_locker::unlock();
JRT_END

// Must be entry as it may lock when acquring the jni_id of the method
// Make the signature of jvmpi_method_entry_C and jvmpi_method_exit_C the same for simplicity and consistency.
JRT_ENTRY (void, OptoRuntime::jvmpi_method_exit_C(methodOop method, oop dummy, JavaThread* thread))
  GC_locker::lock();
  if (jvmpi::is_event_enabled(JVMPI_EVENT_METHOD_EXIT)) {
    jvmpi::post_method_exit_event(method);
  } 
  GC_locker::unlock();
JRT_END


//-----------------------------------------------------------------------------
// Non-product code
#ifndef PRODUCT

// Statistics 
int     OptoRuntime::_nof_ic_misses                   = 0;
int     OptoRuntime::_ICmiss_index                    = 0;
int     OptoRuntime::_ICmiss_count[OptoRuntime::maxICmiss_count];
address OptoRuntime::_ICmiss_at[OptoRuntime::maxICmiss_count];
int OptoRuntime::_nof_normal_calls = 0; 
int OptoRuntime::_nof_optimized_calls = 0; 
int OptoRuntime::_nof_inlined_calls = 0; 
int OptoRuntime::_nof_megamorphic_calls = 0; 
int OptoRuntime::_nof_static_calls = 0;
int OptoRuntime::_nof_inlined_static_calls = 0;
int OptoRuntime::_nof_interface_calls = 0; 
int OptoRuntime::_nof_optimized_interface_calls = 0;
int OptoRuntime::_nof_inlined_interface_calls = 0;
int OptoRuntime::_nof_megamorphic_interface_calls = 0;
int OptoRuntime::_nof_removable_exceptions = 0;

int OptoRuntime::_new_ctr=0;                     
int OptoRuntime::_new_type_ctr=0;                
int OptoRuntime::_new_obj_ctr=0;                 
int OptoRuntime::_multi1_ctr=0;
int OptoRuntime::_multi2_ctr=0;
int OptoRuntime::_multi3_ctr=0;
int OptoRuntime::_multi4_ctr=0;
int OptoRuntime::_multi5_ctr=0;
int OptoRuntime::_ic_miss_ctr=0;                 
int OptoRuntime::_wrong_method_ctr=0;          
int OptoRuntime::_resolve_static_ctr=0;        
int OptoRuntime::_resolve_virtual_ctr=0;       
int OptoRuntime::_resolve_opt_virtual_ctr=0;   
int OptoRuntime::_mon_enter_stub_ctr=0;        
int OptoRuntime::_mon_exit_stub_ctr=0;         
int OptoRuntime::_mon_enter_ctr=0;             
int OptoRuntime::_mon_exit_ctr=0;              
int OptoRuntime::_vtable_c2c_ctr=0;            
int OptoRuntime::_prim_array_copy_ctr=0;
int OptoRuntime::_oop_array_copy_ctr=0;
int OptoRuntime::_c2i_adapter_ctr=0;           
int OptoRuntime::_throw_null_ctr=0;            
int OptoRuntime::_find_handler_ctr=0;          
int OptoRuntime::_rethrow_ctr=0;
int OptoRuntime::_compress_i2c2i_ctr=0;

void OptoRuntime::trace_ic_miss(address at) {
  int i;
  for (i = 0; i < _ICmiss_index; i++) {
    if (_ICmiss_at[i] == at) {
      _ICmiss_count[i]++; 
      return;
    }
  }
  i = _ICmiss_index++; 
  if (_ICmiss_index >= maxICmiss_count) _ICmiss_index = maxICmiss_count - 1;
  _ICmiss_at[i] = at; 
  _ICmiss_count[i] = 1;
}


void OptoRuntime::print_statistics() {
  if (CountICMisses) {
    tty->print_cr ("OptoRuntime statistics (#total) :");
    if (ICMissHistogram) {
      tty->print_cr ("IC missed at:");
      int tot_misses = 0;
      for (int i = 0; i < _ICmiss_index; i++) {
        tty->print_cr("  at: %p  nof: %d", _ICmiss_at[i], _ICmiss_count[i]);
        tot_misses += _ICmiss_count[i];
      }
      tty->print_cr ("Total IC misses: %7d", tot_misses);
    }
    if (CountICMisses) {
      if (_nof_ic_misses > 0) {
        tty->print_cr ("Compiled IC misses:    %7d", _nof_ic_misses);
      }
    }
  }

  if (CountRemovableExceptions) {
    if (_nof_removable_exceptions > 0) {
      Unimplemented(); // this counter is not yet incremented
      tty->print_cr("Removable exceptions: %d", _nof_removable_exceptions);
    }
  }

  // Dump the JRT_ENTRY counters
  if( _new_ctr ) tty->print_cr("%5d 'new' object requires GC", _new_ctr);
  if( _new_type_ctr ) tty->print_cr("%5d 'new' array requires GC", _new_type_ctr);
  if( _new_obj_ctr ) tty->print_cr("%5d 'new' object array requires GC", _new_obj_ctr);
  if( _multi1_ctr ) tty->print_cr("%5d multianewarray 1 dim", _multi1_ctr);
  if( _multi2_ctr ) tty->print_cr("%5d multianewarray 2 dim", _multi2_ctr);
  if( _multi3_ctr ) tty->print_cr("%5d multianewarray 3 dim", _multi3_ctr);
  if( _multi4_ctr ) tty->print_cr("%5d multianewarray 4 dim", _multi4_ctr);
  if( _multi5_ctr ) tty->print_cr("%5d multianewarray 5 dim", _multi5_ctr);
  if( _ic_miss_ctr ) tty->print_cr("%5d inline cache miss in compiled", _ic_miss_ctr );
  if( _wrong_method_ctr ) tty->print_cr("%5d wrong method", _wrong_method_ctr );
  if( _resolve_static_ctr ) tty->print_cr("%5d unresolved static call site", _resolve_static_ctr );
  if( _resolve_virtual_ctr ) tty->print_cr("%5d unresolved virtual call site", _resolve_virtual_ctr );
  if( _resolve_opt_virtual_ctr ) tty->print_cr("%5d unresolved opt virtual call site", _resolve_opt_virtual_ctr );
  if( _mon_enter_stub_ctr ) tty->print_cr("%5d monitor enter stub", _mon_enter_stub_ctr );
  if( _mon_exit_stub_ctr ) tty->print_cr("%5d monitor exit stub", _mon_exit_stub_ctr );
  if( _mon_enter_ctr ) tty->print_cr("%5d monitor enter slow", _mon_enter_ctr );
  if( _mon_exit_ctr ) tty->print_cr("%5d monitor exit slow", _mon_exit_ctr );
  if( _vtable_c2c_ctr ) tty->print_cr("%5d vtable compiled-to-compiled", _vtable_c2c_ctr );
  if( _prim_array_copy_ctr ) tty->print_cr("%5d primitive array copies", _prim_array_copy_ctr );
  if( _oop_array_copy_ctr ) tty->print_cr("%5d oop array copies", _oop_array_copy_ctr );
  if( _c2i_adapter_ctr ) tty->print_cr("%5d c2i adapter for vtables", _c2i_adapter_ctr );
  if( _throw_null_ctr ) tty->print_cr("%5d implicit null throw", _throw_null_ctr );
  if( _find_handler_ctr ) tty->print_cr("%5d find exception handler", _find_handler_ctr );
  if( _rethrow_ctr ) tty->print_cr("%5d rethrow handler", _rethrow_ctr );
  if( _compress_i2c2i_ctr ) tty->print_cr("%5d removed paired I2C/C2I adapters during deopt", _compress_i2c2i_ctr );
}

inline double percent(int x, int y) {
  return 100.0 * x / MAX2(y, 1);
}

class MethodArityHistogram {
 public:
  enum { MAX_ARITY = 256 };
 private:
  static int _arity_histogram[MAX_ARITY];     // histogram of #args
  static int _size_histogram[MAX_ARITY];      // histogram of arg size in words
  static int _max_arity;                      // max. arity seen
  static int _max_size;                       // max. arg size seen

  static void add_method_to_histogram(nmethod* nm) {
    methodOop m = nm->method();
    ArgumentCount args(m->signature());
    int arity   = args.size() + (m->is_static() ? 0 : 1);
    int argsize = m->size_of_parameters();
    arity   = MIN2(arity, MAX_ARITY-1);
    argsize = MIN2(argsize, MAX_ARITY-1);
    int count = nm->method()->compiled_invocation_count();
    _arity_histogram[arity]  += count;
    _size_histogram[argsize] += count;
    _max_arity = MAX2(_max_arity, arity);
    _max_size  = MAX2(_max_size, argsize);
  }

  void print_histogram_helper(int n, int* histo, const char* name) {
    const int N = MIN2(5, n);
    tty->print_cr("\nHistogram of call arity (incl. rcvr, calls to compiled methods only):");
    double sum = 0;
    double weighted_sum = 0;
    int i;
    for (i = 0; i <= n; i++) { sum += histo[i]; weighted_sum += i*histo[i]; }
    double rest = sum;
    double percent = sum / 100;
    for (i = 0; i <= N; i++) {
      rest -= histo[i];
      tty->print_cr("%4d: %7d (%5.1f%%)", i, histo[i], histo[i] / percent);
    }
    tty->print_cr("rest: %7d (%5.1f%%))", (int)rest, rest / percent);
    tty->print_cr("(avg. %s = %3.1f, max = %d)", name, weighted_sum / sum, n);
  }

  void print_histogram() {
    tty->print_cr("\nHistogram of call arity (incl. rcvr, calls to compiled methods only):");
    print_histogram_helper(_max_arity, _arity_histogram, "arity");
    tty->print_cr("\nSame for parameter size (in words):");
    print_histogram_helper(_max_size, _size_histogram, "size");
    tty->cr();
  }

 public:
  MethodArityHistogram() {
    MutexLockerEx mu(CodeCache_lock, Mutex::_no_safepoint_check_flag);
    _max_arity = _max_size = 0;
    for (int i = 0; i < MAX_ARITY; i++) _arity_histogram[i] = _size_histogram [i] = 0;
    CodeCache::nmethods_do(add_method_to_histogram);
    print_histogram();
  }
};

int MethodArityHistogram::_arity_histogram[MethodArityHistogram::MAX_ARITY];
int MethodArityHistogram::_size_histogram[MethodArityHistogram::MAX_ARITY];
int MethodArityHistogram::_max_arity;
int MethodArityHistogram::_max_size;


int trace_exception_counter = 0;
void trace_exception(oop exception_oop, address exception_pc, const char* msg) {
  trace_exception_counter++;
  tty->print("%d [Exception (%s): ", trace_exception_counter, msg);
  exception_oop->print_value();
  tty->print(" in ");
  CodeBlob* blob = CodeCache::find_blob(exception_pc);
  if (blob->is_nmethod()) {
    ((nmethod*)blob)->method()->print_value();
  } else if (blob->is_i2c_adapter()) {
    tty->print("<i2c-adapter>");
  } else if (blob->is_c2i_adapter()) {
    tty->print("<c2i-adapter>");
  } else if (blob->is_runtime_stub()) {
    tty->print("<runtime-stub>");
  } else if (blob->is_osr_adapter()) {
    tty->print("<osr-adapter>");
  } else {
    tty->print("<unknown>");
  }
  tty->print(" at " INTPTR_FORMAT,  exception_pc);
  tty->print_cr("]");
}

void OptoRuntime::print_call_statistics(int comp_total) {
  tty->print_cr("Calls from compiled code:");
  int total  = _nof_normal_calls + _nof_interface_calls + _nof_static_calls;
  int mono_c = _nof_normal_calls - _nof_optimized_calls - _nof_megamorphic_calls;
  int mono_i = _nof_interface_calls - _nof_optimized_interface_calls - _nof_megamorphic_interface_calls;
  tty->print_cr("\t%9d   (%4.1f%%) total non-inlined   ", total, percent(total, total));
  tty->print_cr("\t%9d   (%4.1f%%) virtual calls       ", _nof_normal_calls, percent(_nof_normal_calls, total));
  tty->print_cr("\t  %9d  (%3.0f%%)   inlined          ", _nof_inlined_calls, percent(_nof_inlined_calls, _nof_normal_calls));
  tty->print_cr("\t  %9d  (%3.0f%%)   optimized        ", _nof_optimized_calls, percent(_nof_optimized_calls, _nof_normal_calls));
  tty->print_cr("\t  %9d  (%3.0f%%)   monomorphic      ", mono_c, percent(mono_c, _nof_normal_calls));
  tty->print_cr("\t  %9d  (%3.0f%%)   megamorphic      ", _nof_megamorphic_calls, percent(_nof_megamorphic_calls, _nof_normal_calls));
  tty->print_cr("\t%9d   (%4.1f%%) interface calls     ", _nof_interface_calls, percent(_nof_interface_calls, total));
  tty->print_cr("\t  %9d  (%3.0f%%)   inlined          ", _nof_inlined_interface_calls, percent(_nof_inlined_interface_calls, _nof_interface_calls));
  tty->print_cr("\t  %9d  (%3.0f%%)   optimized        ", _nof_optimized_interface_calls, percent(_nof_optimized_interface_calls, _nof_interface_calls));
  tty->print_cr("\t  %9d  (%3.0f%%)   monomorphic      ", mono_i, percent(mono_i, _nof_interface_calls));
  tty->print_cr("\t  %9d  (%3.0f%%)   megamorphic      ", _nof_megamorphic_interface_calls, percent(_nof_megamorphic_interface_calls, _nof_interface_calls));
  tty->print_cr("\t%9d   (%4.1f%%) static/special calls", _nof_static_calls, percent(_nof_static_calls, total));
  tty->print_cr("\t  %9d  (%3.0f%%)   inlined          ", _nof_inlined_static_calls, percent(_nof_inlined_static_calls, _nof_static_calls));
  tty->cr();
  tty->print_cr("Note 1: counter updates are not MT-safe.");
  tty->print_cr("Note 2: %% in major categories are relative to total non-inlined calls;");
  tty->print_cr("        %% in nested categories are relative to their category");
  tty->print_cr("        (and thus add up to more than 100%% with inlining)");
  tty->cr();

  MethodArityHistogram h;
}

#endif  // PRODUCT


# ifdef ENABLE_ZAP_DEAD_LOCALS
// Called from call sites in compiled code with oop maps (actually safepoints)
// Zaps dead locals in first java frame.
// Is entry because may need to lock to generate oop maps
// Currently, only used for compiler frames, but someday may be used 
// for interpreter frames, too.

int OptoRuntime::ZapDeadCompiledLocals_count = 0;

// avoid pointers to member funcs with these helpers
static bool is_java_frame(  frame* f) { return f->is_java_frame();   }
static bool is_native_frame(frame* f) { return f->is_native_frame(); }


void OptoRuntime::zap_dead_java_or_native_locals(JavaThread* thread, 
                                                bool (*is_this_the_right_frame_to_zap)(frame*)) {
  assert(JavaThread::current() == thread, "is this needed?");
 
  if ( !ZapDeadCompiledLocals )  return;

  bool skip = false;

       if ( ZapDeadCompiledLocalsFirst  ==  0  ) ; // nothing special
  else if ( ZapDeadCompiledLocalsFirst  >  ZapDeadCompiledLocals_count )  skip = true;
  else if ( ZapDeadCompiledLocalsFirst  == ZapDeadCompiledLocals_count )
    warning("starting zapping after skipping");

       if ( ZapDeadCompiledLocalsLast  ==  -1  ) ; // nothing special
  else if ( ZapDeadCompiledLocalsLast  <   ZapDeadCompiledLocals_count )  skip = true;
  else if ( ZapDeadCompiledLocalsLast  ==  ZapDeadCompiledLocals_count )
    warning("about to zap last zap");

  ++ZapDeadCompiledLocals_count; // counts skipped zaps, too

  if ( skip )  return;

  // find java frame and zap it

  for (StackFrameStream sfs(thread);  !sfs.is_done();  sfs.next()) {
    if (is_this_the_right_frame_to_zap(sfs.current()) ) {
      sfs.current()->zap_dead_locals(thread, sfs.register_map());
      return;
    }
  }
  warning("no frame found to zap in zap_dead_Java_locals_C");
}

JRT_LEAF(void, OptoRuntime::zap_dead_Java_locals_C(JavaThread* thread))
  zap_dead_java_or_native_locals(thread, is_java_frame);
JRT_END

// The following does not work because for one thing, the
// thread state is wrong; it expects java, but it is native.
// Also, the invarients in a native stub are different and
// I'm not sure it is safe to have a MachCalRuntimeDirectNode
// in there.
// So for now, we do not zap in native stubs.

JRT_LEAF(void, OptoRuntime::zap_dead_native_locals_C(JavaThread* thread))
  zap_dead_java_or_native_locals(thread, is_native_frame);
JRT_END

# endif

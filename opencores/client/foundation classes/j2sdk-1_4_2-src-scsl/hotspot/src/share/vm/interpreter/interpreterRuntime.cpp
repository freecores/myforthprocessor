#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)interpreterRuntime.cpp	1.423 03/05/08 14:38:54 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_interpreterRuntime.cpp.incl"

class UnlockFlagSaver {
  private:
    JavaThread* _thread;
    bool _do_not_unlock;
  public:
    UnlockFlagSaver(JavaThread* t) {
      _thread = t;
      _do_not_unlock = t->do_not_unlock_if_synchronized();
      t->set_do_not_unlock_if_synchronized(false);
    }
    ~UnlockFlagSaver() {
      _thread->set_do_not_unlock_if_synchronized(_do_not_unlock);
    }
};

//------------------------------------------------------------------------------------------------------------------------
// State accessors

void InterpreterRuntime::set_bcp_and_mdp(address bcp, JavaThread *thread) {
  last_frame(thread).interpreter_frame_set_bcp(bcp);
#ifndef CORE
  methodDataOop mdo = last_frame(thread).interpreter_frame_method()->method_data();
  if (mdo != NULL) {
    NEEDS_CLEANUP;
    last_frame(thread).interpreter_frame_set_mdp(mdo->bci_to_dp(last_frame(thread).interpreter_frame_bci()));
  }
#endif // !CORE
}

//------------------------------------------------------------------------------------------------------------------------
// Constants


IRT_ENTRY(void, InterpreterRuntime::ldc(JavaThread* thread))
  // access constant pool
  constantPoolOop pool      = method(thread)->constants();
  int index = one_byte_index(thread);

#ifdef ASSERT
  constantTag     tag       = pool->tag_at(index);
  // If we entered this runtime routine, we believed the tag contained
  // an unresolved string. However, another thread could have resolved
  // it by the time we go there.
  assert(tag.is_unresolved_string() || tag.is_string(), "expected string");
#endif
  pool->string_at(index, CHECK);
IRT_END


IRT_ENTRY(void, InterpreterRuntime::ldc_w(JavaThread* thread))
  // access constant pool
  constantPoolOop pool      = method(thread)->constants();
  int index = two_byte_index(thread);

#ifdef ASSERT
  constantTag     tag       = pool->tag_at(index);
  // If we entered this runtime routine, we believed the tag contained
  // an unresolved string. However, another thread could have resolved
  // it by the time we go there.
  assert(tag.is_unresolved_string() || tag.is_string(), "expected string");
#endif
  pool->string_at(index, CHECK);
IRT_END


//------------------------------------------------------------------------------------------------------------------------
// Allocation

IRT_ENTRY(void, InterpreterRuntime::_new(JavaThread* thread, constantPoolOop pool, int index))
  klassOop k_oop = pool->klass_at(index, CHECK);
  instanceKlassHandle klass (THREAD, k_oop);

  // Make sure we are not instantiating an abstract klass
  klass->check_valid_for_instantiation(true, CHECK);

  // Make sure klass is initialized
  klass->initialize(CHECK);    

  // At this point the class may not be fully initialized
  // because of recursive initialization. If it is fully
  // initialized & has_finalized is not set, we rewrite
  // it into its fast version (Note: no locking is needed
  // here since this is an atomic byte write and can be
  // done more than once).
  //
  // Note: In case of classes with has_finalized we don't
  //       rewrite since that saves us an extra check in
  //       the fast version which then would call the
  //       slow version anyway (and do a call back into
  //       Java).
  //       If we have a breakpoint, then we don't rewrite
  //       because the _breakpoint bytecode would be lost.
  oop obj = klass->allocate_instance(CHECK);
  thread->set_vm_result(obj);  
IRT_END


IRT_ENTRY(void, InterpreterRuntime::newarray(JavaThread* thread, BasicType type, jint size))
  oop obj = oopFactory::new_typeArray(type, size, CHECK);
  thread->set_vm_result(obj);
IRT_END


IRT_ENTRY(void, InterpreterRuntime::anewarray(JavaThread* thread, constantPoolOop pool, int index, jint size))
  // Note: no oopHandle for pool & klass needed since they are not used
  //       anymore after new_objArray() and no GC can happen before.
  //       (This may have to change if this code changes!)       
  klassOop  klass = pool->klass_at(index, CHECK);    
  objArrayOop obj = oopFactory::new_objArray(klass, size, CHECK);
  thread->set_vm_result(obj);
IRT_END


IRT_ENTRY(void, InterpreterRuntime::multianewarray(JavaThread* thread, jint* first_size_address))
  // We may want to pass in more arguments - could make this slightly faster
  constantPoolOop constants = method(thread)->constants();
  int          i = two_byte_index(thread);
  klassOop klass = constants->klass_at(i, CHECK);
  int   nof_dims = number_of_dimensions(thread);
  assert(oop(klass)->is_klass(), "not a class");
  assert(nof_dims >= 1, "multianewarray rank must be nonzero");
#ifdef _LP64
// In 64 bit mode, the sizes are stored in the top 32 bits of each 64 bit stack entry.  
// first_size_address is actually an intptr_t *
// We must create an array of jints to pass to multi_allocate.
  if ( nof_dims > 1 ) {
    int index;
    for ( index = 1; index < nof_dims; index++ ) {  // First size is ok
	first_size_address[-index] = first_size_address[-index*2];
    }
  }
#endif
  oop obj = arrayKlass::cast(klass)->
              multi_allocate(nof_dims, 
                             first_size_address, 
                             frame::interpreter_frame_expression_stack_direction(),
                             CHECK);
  thread->set_vm_result(obj);
IRT_END


// Quicken instance-of and check-cast bytecodes
IRT_ENTRY(void, InterpreterRuntime::quicken_io_cc(JavaThread* thread))
  // Force resolving; quicken the bytecode
  int which = two_byte_index(thread);
  constantPoolOop cpool = method(thread)->constants();
  // We'd expect to assert that we're only here to quicken bytecodes, but in a multithreaded
  // program we might have seen an unquick'd bytecode in the interpreter but have another
  // thread quicken the bytecode before we get here.
  // assert( cpool->tag_at(which).is_unresolved_klass(), "should only come here to quicken bytecodes" );
  klassOop klass = cpool->klass_at(which, CHECK);
  thread->set_vm_result(klass);
IRT_END


//------------------------------------------------------------------------------------------------------------------------
// Exceptions

// Special handling for stack overflow: since we don't have any (java) stack
// space left we use the pre-allocated & pre-initialized StackOverflowError
// klass to create an stack overflow error instance.  We do not call its
// constructor for the same reason (it is empty, anyway).

IRT_ENTRY(void, InterpreterRuntime::create_StackOverflowError(JavaThread* thread))
  // get klass
  instanceKlass* klass = instanceKlass::cast(SystemDictionary::StackOverflowError_klass());
  assert(klass->is_initialized(), "StackOverflowError klass should have been initialized during VM initialization");
  // create instance - do not call constructor since we have no (java) stack space left
  Handle exception (thread, klass->allocate_instance(thread));
  // check if another exception happened in the meantime
  if (thread->has_pending_exception()) {
    // another exception happened => use that one
    exception = Handle(thread, thread->pending_exception());
    thread->clear_pending_exception();
  } else {
    // no other exception happened => fill in stack trace if needed
    if (StackTraceInThrowable) {
      java_lang_Throwable::fill_in_stack_trace(exception);
    }
  }
  // return exception
  thread->set_vm_result(exception());
IRT_END


IRT_ENTRY(void, InterpreterRuntime::create_exception(JavaThread* thread, char* name, char* message))
  // lookup exception klass
  symbolOop s = oopFactory::new_symbol(name, CHECK);
  // create exception 
  Handle exception = Exceptions::new_exception(thread, s, message);  
  thread->set_vm_result(exception());
IRT_END


IRT_ENTRY(void, InterpreterRuntime::create_ArrayIndexOutOfBounds_exception(JavaThread* thread, char* name, jint index))
  char message[jintAsStringSize];
  // lookup exception klass
  symbolOop s = oopFactory::new_symbol(name, CHECK);
  // create exception 
  sprintf(message, "%d", index);
  Handle exception = Exceptions::new_exception(thread, s, message);  
  thread->set_vm_result(exception());
IRT_END


// exception_handler_for_exception(...) returns the continuation address,
// the exception oop (via TLS) and sets the bci/bcp for the continuation.
// The exception oop is returned to make sure it is preserved over GC (it
// is only on the stack if the exception was thrown explicitly via athrow).
// During this operation, the expression stack contains the values for the
// bci where the exception happened. If the exception was propagated back
// from a call, the expression stack contains the values for the bci at the
// invoke w/o arguments (i.e., as if one were inside the call).
IRT_ENTRY(address, InterpreterRuntime::exception_handler_for_exception(JavaThread* thread, oop exception))

  // Need to do this check first since when _do_not_unlock_if_synchronized
  // is set, we don't want to trigger any classloading which may make calls
  // into java, or surprisingly find a matching exception handler for bci 0
  // since at this moment the method hasn't been "officially" entered yet.
  if (thread->do_not_unlock_if_synchronized()) {
    thread->set_vm_result(exception);
#ifdef CC_INTERP
    return (address) -1;
#else
    return Interpreter::remove_activation_entry();
#endif
  }

  Handle             h_exception(thread, exception);
  methodHandle       h_method   (thread, method(thread));
  constantPoolHandle h_constants(thread, h_method->constants());
  typeArrayHandle    h_extable  (thread, h_method->exception_table());
  bool               should_repeat;
  int                handler_bci;
  int                current_bci = bcp(thread) - h_method->code_base();
  
#ifdef COMPILER2
  if (ProfileInterpreter && thread->is_throwing_null_ptr_exception() ) {
    methodDataOop mdo = h_method->method_data();

    thread->set_is_throwing_null_ptr_exception(false);
    if (mdo == NULL) {
      methodOopDesc::build_interpreter_method_data(h_method, THREAD);
      if (HAS_PENDING_EXCEPTION) {
        assert((PENDING_EXCEPTION->is_a(SystemDictionary::OutOfMemoryError_klass())), "we expect only an OOM error here");
        CLEAR_PENDING_EXCEPTION;
      }
      mdo = h_method->method_data();
      // and fall through...
    }

    if (mdo != NULL)
      mdo->incr_null_ptr_count();
  }
#endif

  do {
    should_repeat = false;

    // assertions
#ifdef ASSERT
    assert(h_exception.not_null(), "NULL exceptions should be handled by athrow");
    assert(h_exception->is_oop(), "just checking");
    // Check that exception is a subclass of Throwable, otherwise we have a VerifyError
    if (!(h_exception->is_a(SystemDictionary::throwable_klass()))) {
      if (ExitVMOnVerifyError) vm_exit(-1);
      ShouldNotReachHere();
    }
#endif

    // tracing
    if (TraceExceptions) {
      ThreadCritical tc;
      ResourceMark rm(thread);
      tty->print_cr("Exception <%s> (" INTPTR_FORMAT ")", h_exception->print_value_string(), h_exception());
      tty->print_cr(" thrown in interpreter method <%s>", h_method->print_value_string());
      tty->print_cr(" at bci %d for thread " INTPTR_FORMAT, current_bci, thread);
    } else if (h_extable->length() == 0) {
      // disabled for now - interpreter is not using shortcut yet
      // (shortcut is not to call runtime if we have no exception handlers)
      // warning("performance bug: should not call runtime if method has no exception handlers");
    }

    // exception handler lookup
    KlassHandle h_klass(THREAD, h_exception->klass());
    handler_bci = h_method->fast_exception_handler_bci_for(h_klass, current_bci, true, THREAD);
    if (HAS_PENDING_EXCEPTION) {
      // We threw an exception while trying to find the exception handler.
      // Transfer the new exception to the exception handle which will
      // be set into thread local storage, and do another lookup for an
      // exception handler for this exception, this time starting at the
      // BCI of the exception handler which caused the exception to be
      // thrown (bug 4307310).
      h_exception = Handle(THREAD, PENDING_EXCEPTION);
      CLEAR_PENDING_EXCEPTION;
      if (handler_bci >= 0) {
	current_bci = handler_bci;
	should_repeat = true;
      }
    }
  } while (should_repeat == true);

  // notify jvmdi of an exception throw; jvmdi will detect if this is a first 
  // time throw or a stack unwinding throw and accordingly notify the debugger
  if (jvmdi::enabled()) {
    jvmdi::post_exception_throw_event(thread, h_method(), bcp(thread), h_exception());
  }

#ifdef CC_INTERP
  address continuation = (address) handler_bci;
#else
  address continuation = NULL;
#endif
  address handler_pc = NULL;
  if (handler_bci < 0 || !thread->reguard_stack((address) &continuation)) {
    // Forward exception to callee (leaving bci/bcp untouched) because (a) no
    // handler in this method, or (b) after a stack overflow there is not yet
    // enough stack space available to reprotect the stack.
#ifndef CC_INTERP
    continuation = Interpreter::remove_activation_entry();
#endif 
    // Count this for compilation purposes
    COMPILER2_ONLY(h_method->interpreter_throwout_increment());
  } else {
    // handler in this method => change bci/bcp to handler bci/bcp and continue there
    handler_pc = h_method->code_base() + handler_bci;
#ifndef CC_INTERP
    set_bcp_and_mdp(handler_pc, thread);
    continuation = Interpreter::dispatch_table(vtos)[*handler_pc];
#endif
  }
  // notify debugger of an exception catch 
  // (this is good for exceptions caught in native methods as well)
  if (jvmdi::enabled()) {
    jvmdi::notice_unwind_due_to_exception(thread, h_method(), handler_pc, h_exception(), (handler_pc != NULL));
  }

  thread->set_vm_result(h_exception());
  return continuation;
IRT_END


IRT_ENTRY(void, InterpreterRuntime::throw_pending_exception(JavaThread* thread))  
  assert(thread->has_pending_exception(), "must only ne called if there's an exception pending");
  // nothing to do - eventually we should remove this code entirely (see comments @ call sites)
IRT_END


IRT_ENTRY(void, InterpreterRuntime::throw_abstract_method_error(JavaThread* thread))          
  THROW(vmSymbols::java_lang_AbstractMethodError());
IRT_END


IRT_ENTRY(void, InterpreterRuntime::throw_incompatible_class_change_error(JavaThread* thread))          
  THROW(vmSymbols::java_lang_IncompatibleClassChangeError());
IRT_END


//------------------------------------------------------------------------------------------------------------------------
// Fields
//

IRT_ENTRY(void, InterpreterRuntime::resolve_get_put(JavaThread* thread, Bytecodes::Code bytecode))
  // resolve field
  FieldAccessInfo info;
  constantPoolHandle pool(thread, method(thread)->constants());
  bool is_static = (bytecode == Bytecodes::_getstatic || bytecode == Bytecodes::_putstatic);
  bool single_step_hidden = false;
  if (jvmdi::enabled()) {
    single_step_hidden = jvmdi::hide_single_stepping(thread);
  }
  LinkResolver::resolve_field(info, pool, two_byte_index(thread), bytecode, false, CHECK);
  if (single_step_hidden) {
    jvmdi::expose_single_stepping(thread);
  }  

  // check if link resolution caused cpCache to be updated
  if (already_resolved(thread)) return;

  // compute auxiliary field attributes
  TosState state  = as_TosState(info.field_type());

  // We need to delay resolving put instructions on final fields
  // until we actually invoke one. This is required so we throw
  // exceptions at the correct place. If we do not resolve completely
  // in the current pass, leaving the put_code set to zero will
  // cause the next put instruction to reresolve.
  bool is_put = (bytecode == Bytecodes::_putfield ||
                 bytecode == Bytecodes::_putstatic);
  Bytecodes::Code put_code = (Bytecodes::Code)0;

  // We also need to delay resolving getstatic instructions until the
  // class is intitialized.  This is required so that access to the static
  // field will call the initialization function every time until the class
  // is completely initialized ala. in 2.17.5 in JVM Specification.
  instanceKlass *klass = instanceKlass::cast(info.klass()->as_klassOop());
  bool uninitialized_static = ((bytecode == Bytecodes::_getstatic || bytecode == Bytecodes::_putstatic) &&
                               !klass->is_initialized());
  Bytecodes::Code get_code = (Bytecodes::Code)0;


  if (!uninitialized_static) {
    get_code = ((is_static) ? Bytecodes::_getstatic : Bytecodes::_getfield);
    if (is_put || !info.access_flags().is_final()) {
      put_code = ((is_static) ? Bytecodes::_putstatic : Bytecodes::_putfield);
    }
  }

  cache_entry(thread)->set_field(
    get_code,
    put_code,
    info.klass(),
    info.field_index(),
    info.field_offset(),
    state,
    info.access_flags().is_final(),
    info.access_flags().is_volatile()
  );
IRT_END


//------------------------------------------------------------------------------------------------------------------------
// Synchronization
//
// The interpreter's synchronization code is factored out so that it can
// be shared by method invocation and synchronized blocks.
//%note synchronization_3

static void trace_locking(Handle& h_locking_obj, bool is_locking) {
  NOT_CORE(ObjectSynchronizer::trace_locking(h_locking_obj, false, true, is_locking);)
}


//%note monitor_1
IRT_ENTRY_FOR_MONITORENTER_OR_EXIT(void, InterpreterRuntime::monitorenter(JavaThread* thread, BasicObjectLock* elem))
#ifdef ASSERT
  thread->last_frame().interpreter_frame_verify_monitor(elem);
#endif
  Handle h_obj(thread, elem->obj());  
  assert(Universe::heap()->is_in_or_null(h_obj()), "must be NULL or an object");
  ObjectSynchronizer::slow_enter(h_obj, elem->lock(), CHECK);
  assert(Universe::heap()->is_in_or_null(elem->obj()), "must be NULL or an object");
#ifdef ASSERT
  thread->last_frame().interpreter_frame_verify_monitor(elem);
#endif
IRT_END


//%note monitor_1
IRT_ENTRY_FOR_MONITORENTER_OR_EXIT(void, InterpreterRuntime::monitorexit(JavaThread* thread, BasicObjectLock* elem))
#ifdef ASSERT
  thread->last_frame().interpreter_frame_verify_monitor(elem);
#endif
  Handle h_obj(thread, elem->obj());  
  assert(Universe::heap()->is_in_or_null(h_obj()), "must be NULL or an object");
  if (elem == NULL || h_obj()->is_unlocked()) {
    THROW(vmSymbols::java_lang_IllegalMonitorStateException());
  }
  ObjectSynchronizer::slow_exit(h_obj(), elem->lock(), thread);
  // Free entry. This must be done here, since a pending exception might be installed on
  // exit. If it is not cleared, the exception handling code will try to unlock the monitor again.
  elem->set_obj(NULL); 
#ifdef ASSERT
  thread->last_frame().interpreter_frame_verify_monitor(elem);
#endif
IRT_END


IRT_ENTRY(void, InterpreterRuntime::throw_illegal_monitor_state_exception(JavaThread* thread))  
  THROW(vmSymbols::java_lang_IllegalMonitorStateException());
IRT_END


IRT_ENTRY(void, InterpreterRuntime::new_illegal_monitor_state_exception(JavaThread* thread))
  // Returns an illegal exception to install into the current thread. The pending_exception
  // flag is cleared so normal exception handling does not trigger. Any current installed
  // exception will be overwritten. This method will be called during an exception unwind.  
  assert(!HAS_PENDING_EXCEPTION, "no pending exception");
  Handle exception(thread, thread->vm_result());
  assert(exception() != NULL, "vm result should be set");
  thread->set_vm_result(NULL); // clear vm result before continuing (may cause memory leaks and assert failures)
  if (!exception->is_a(SystemDictionary::threaddeath_klass())) {        
    exception = Exceptions::new_exception(thread, 
                     vmSymbols::java_lang_IllegalMonitorStateException(),
                     NULL);
  }
  thread->set_vm_result(exception());
IRT_END


//------------------------------------------------------------------------------------------------------------------------
// Invokes

IRT_ENTRY(Bytecodes::Code, InterpreterRuntime::get_original_bytecode_at(JavaThread* thread, methodOop method, address bcp))
  return method->orig_bytecode_at(method->bci_from(bcp));
IRT_END

IRT_ENTRY(void, InterpreterRuntime::set_original_bytecode_at(JavaThread* thread, methodOop method, address bcp, Bytecodes::Code new_code))
  method->set_orig_bytecode_at(method->bci_from(bcp), new_code);
IRT_END

IRT_ENTRY(void, InterpreterRuntime::_breakpoint(JavaThread* thread, methodOop method, address bcp))
  JvmdiThreadState *state = thread->jvmdi_thread_state();
  state->compare_and_set_current_location(method, bcp, JVMDI_EVENT_BREAKPOINT);
  if (!state->breakpoint_posted()) {
    ThreadState old_state = thread->osthread()->get_state();
    thread->osthread()->set_state(BREAKPOINTED);
    jvmdi::post_breakpoint_event(thread, method, bcp);
    thread->osthread()->set_state(old_state);
  }
IRT_END

IRT_ENTRY(void, InterpreterRuntime::resolve_invoke(JavaThread* thread, Bytecodes::Code bytecode))  
  // extract receiver from the outgoing argument list if necessary
  Handle receiver(thread, NULL);  
  if (bytecode == Bytecodes::_invokevirtual || bytecode == Bytecodes::_invokeinterface) {
    ResourceMark rm(thread);
    methodHandle m (thread, method(thread));
    int bci = m->bci_from(bcp(thread));    
    Bytecode_invoke* call = Bytecode_invoke_at(m, bci);    
    symbolHandle signature (thread, call->signature());
    receiver = Handle(thread,
                  thread->last_frame().interpreter_callee_receiver(signature));
    assert(Universe::heap()->is_in_or_null(receiver()), "sanity check");    
    assert(receiver.is_null() || Universe::heap()->is_in(receiver->klass()), "sanity check");
  }  

  // resolve method
  CallInfo info;
  constantPoolHandle pool(thread, method(thread)->constants());

  bool single_step_hidden = false;
  if (jvmdi::enabled()) {
    single_step_hidden = jvmdi::hide_single_stepping(thread);
  }
  LinkResolver::resolve_invoke(info, receiver, pool, two_byte_index(thread), bytecode, CHECK);
  if (single_step_hidden) {
    jvmdi::expose_single_stepping(thread);
  }
  // check if link resolution caused cpCache to be updated
  if (already_resolved(thread)) return;

  if (bytecode == Bytecodes::_invokeinterface) {    

    if (TraceItables && Verbose) {
      ResourceMark rm(thread);
      tty->print_cr("Resolving: klass: %s to method: %s", info.resolved_klass()->name()->as_C_string(), info.resolved_method()->name()->as_C_string());
    }
    if (info.resolved_method()->method_holder() ==
                                            SystemDictionary::object_klass()) {
      // NOTE: THIS IS A FIX FOR A CORNER CASE in the JVM spec
      // (see also cpCacheOop.cpp for details)
      assert(info.has_vtable_index(), "should have been set already");
      cache_entry(thread)->set_method(
        bytecode,
        info.resolved_method(),
        info.vtable_index()
      ); 
    } else {          
      // Setup itable entry      
      int index = klassItable::compute_itable_index(info.resolved_method()());
      cache_entry(thread)->set_interface_call(info.resolved_method(), index);
    }
  } else {    
    cache_entry(thread)->set_method(
      bytecode,
      info.resolved_method(),
      info.vtable_index());     
  }
IRT_END


#ifndef CORE
IRT_ENTRY(address, InterpreterRuntime::nmethod_entry_point(JavaThread* thread, methodOop method, nmethod* nm))  
  methodHandle m(thread, method);    
  // This is a particularly tricky entry point because the interpreter has not set up
  // a new frame and must do some tricks to get the environment setup correct to make this
  // call. Make it even more stressful by triggering a safepoint with stress options.
  if (SafepointALot || ScavengeALot) {
      VM_ForceSafepoint vfs;
      VMThread::execute(&vfs);
  }
  { debug_only(nmethod* nm2 = m->code());
    // Note: nm2 could be null, if an uncommon trap for the method happened right before we entered here
    assert(nm != NULL && (nm2 == NULL || nm == nm2), "nmethods must match");
  }
  // Sets the interpreter_entry_point as a side effect.
  address i2c_entry = nm->interpreter_entry_point();
  thread->set_vm_result(m());
  return i2c_entry;
IRT_END
#endif


//------------------------------------------------------------------------------------------------------------------------
// Miscellaneous


#ifndef CORE
#ifndef PRODUCT
static void trace_frequency_counter_overflow(methodHandle m, int bci, address branch_bcp) {
  if (TraceInvocationCounterOverflow) {
    InvocationCounter* c = m->invocation_counter();
    ResourceMark rm;
    const char* msg =
      branch_bcp == NULL
      ? "comp-policy cntr ovfl @ %d in entry of "
      : "comp-policy cntr ovfl @ %d in loop of ";
    tty->print(msg, bci);
    m->print_value();
    tty->cr();
    c->print();
  }
}

static void trace_osr_request(methodHandle method, nmethod* osr, int bci) {
  if (TraceOnStackReplacement) {
    ResourceMark rm;
    tty->print(osr != NULL ? "Reused OSR entry for " : "Requesting OSR entry for ");
    method->print_short_name(tty);
    tty->print_cr(" at bci %d", bci);
  }    
}
#endif // !PRODUCT

IRT_ENTRY(InterpreterRuntime::IcoResult,
          InterpreterRuntime::frequency_counter_overflow(JavaThread* thread, address branch_bcp))
  // use UnlockFlagSaver to clear and restore the _do_not_unlock_if_synchronized
  // flag, in case this method triggers classloading which will call into Java.
  UnlockFlagSaver fs(thread);

  frame fr = thread->last_frame();
  assert(fr.is_interpreted_frame(), "must come from interpreter");
  methodHandle method(thread, fr.interpreter_frame_method());  
  const int bci = method->bci_from(fr.interpreter_frame_bcp());
  NOT_PRODUCT(trace_frequency_counter_overflow(method, bci, branch_bcp);)

  if (jvmdi::enabled() && FullSpeedJVMDI) {
    if (thread->jvmdi_thread_state()->is_interp_only_mode()) {
      // If a breakpoint or frame pop event is requested then the
      // thread is forced to remain in interpreted code. This is
      // implemented partly by a check in the run_compiled_code
      // section of the interpreter whether we should skip running
      // compiled code, and partly by skipping OSR compiles for
      // interpreted-only threads.
      if (branch_bcp != NULL) {
        CompilationPolicy::policy()->reset_counter_for_back_branch_event(method);
        return makeIcoResult(NULL);
      }
    }
  }

  if (branch_bcp == NULL) {
    // when code cache is full, compilation gets switched off, UseCompiler
    // is set to false
    if (!method->has_compiled_code() && UseCompiler) {
      CompilationPolicy::policy()->method_invocation_event(method, CHECK_0);
    } else {
      // Force counter overflow on method entry, even if no compilation
      // happened.  (The method_invocation_event call does this also.)
      CompilationPolicy::policy()->reset_counter_for_invocation_event(method);
    }
    nmethod* nm = method->code();
    return makeIcoResult(nm != NULL ? nm->verified_entry_point() : NULL);

  } else {
    // counter overflow in a loop => try to do on-stack-replacement
    nmethod* osr_nm = method->lookup_osr_nmethod_for(bci);
    NOT_PRODUCT(trace_osr_request(method, osr_nm, bci);)
    // when code cache is full, we should not compile any more...
    if (osr_nm == NULL && UseCompiler) {
      const int branch_bci = method->bci_from(branch_bcp);
      CompilationPolicy::policy()->method_back_branch_event(method, branch_bci, bci, CHECK_0);
      osr_nm = method->lookup_osr_nmethod_for(bci);
    }
    if (osr_nm == NULL) {
      CompilationPolicy::policy()->reset_counter_for_back_branch_event(method);
      return makeIcoResult((address)NULL);
    } else {
      // continue w/ on-stack-replacement code
#ifdef COMPILER1
      // no osr_adapter_frame_return_address needed
      address osr_adapter_frame_return_address = NULL;
#else // COMPILER2
      // compute osr_adapter_frame_return_address
      int osr_frame_size = fr.frame_size() + method->size_of_parameters();
      OSRAdapter* osr_return_adapter = OnStackReplacement::get_osr_adapter(osr_frame_size,method->is_returning_fp());
      address osr_adapter_frame_return_address = osr_return_adapter->instructions_begin();
#endif // COMPILER1
      // return 2 values in registers
      return makeIcoResult(osr_adapter_frame_return_address, osr_nm);
    }
  }
IRT_END

IRT_ENTRY(jint, InterpreterRuntime::profile_method(JavaThread* thread, address cur_bcp))
  // use UnlockFlagSaver to clear and restore the _do_not_unlock_if_synchronized
  // flag, in case this method triggers classloading which will call into Java.
  UnlockFlagSaver fs(thread);

  assert(ProfileInterpreter, "must be profiling interpreter");
  frame fr = thread->last_frame();
  assert(fr.is_interpreted_frame(), "must come from interpreter");
  methodHandle method(thread, fr.interpreter_frame_method());
  int bci = method->bci_from(cur_bcp);
  methodOopDesc::build_interpreter_method_data(method, THREAD);
  if (HAS_PENDING_EXCEPTION) {
    assert((PENDING_EXCEPTION->is_a(SystemDictionary::OutOfMemoryError_klass())), "we expect only an OOM error here");
    CLEAR_PENDING_EXCEPTION;
    // and fall through...
  }
  methodDataOop mdo = method->method_data();
  if (mdo == NULL)  return 0;
  return mdo->bci_to_di(bci);
IRT_END


#ifdef ASSERT
IRT_LEAF(void, InterpreterRuntime::verify_mdp(methodOop method, address bcp, address mdp))
  assert(ProfileInterpreter, "must be profiling interpreter");

  methodDataOop mdo = method->method_data();
  assert(mdo != NULL, "must not be null");

  int bci = method->bci_from(bcp);

  address mdp2 = mdo->bci_to_dp(bci);
  if (mdp != mdp2) {
    ResourceMark rm;
    ResetNoHandleMark rnm; // In a LEAF entry.
    HandleMark hm;
    tty->print_cr("FAILED verify : actual mdp %p   expected mdp %p @ bci %d", mdp, mdp2, bci);
    int current_di = mdo->dp_to_di(mdp);
    int expected_di  = mdo->dp_to_di(mdp2);
    tty->print_cr("  actual di %d   expected di %d", current_di, expected_di);
    int expected_approx_bci = mdo->data_at(expected_di)->bci();
    int approx_bci = -1;
    if (current_di >= 0) {
      approx_bci = mdo->data_at(current_di)->bci();
    }
    tty->print_cr("  actual bci is %d  expected bci %d", approx_bci, expected_approx_bci);
    mdo->print_on(tty);
    method->print_codes();
  }
  assert(mdp == mdp2, "wrong mdp");
IRT_END
#endif // ASSERT

IRT_ENTRY(void, InterpreterRuntime::update_mdp_for_ret(JavaThread* thread, int return_bci))
  assert(ProfileInterpreter, "must be profiling interpreter");
  ResourceMark rm(thread);
  HandleMark hm(thread);
  frame fr = thread->last_frame();
  assert(fr.is_interpreted_frame(), "must come from interpreter");
  methodDataHandle h_mdo(thread, fr.interpreter_frame_method()->method_data());

  // Grab a lock to ensure atomic access to setting the return bci and
  // the displacement.  This can block and GC, invalidating all naked oops.
  MutexLocker ml(RetData_lock);

  // ProfileData is essentially a wrapper around a derived oop, so we
  // need to take the lock before making any ProfileData structures.
  ProfileData* data = h_mdo->data_at(h_mdo->dp_to_di(fr.interpreter_frame_mdp()));
  RetData* rdata = data->as_RetData();
  address new_mdp = rdata->fixup_ret(return_bci, h_mdo);
  fr.interpreter_frame_set_mdp(new_mdp);
IRT_END

#endif // CORE


IRT_ENTRY(void, InterpreterRuntime::at_safepoint(JavaThread* thread))
  // We used to need an explict preserve_arguments here for invoke bytecodes. However,
  // stack traversal automatically takes care of preserving arguments for invoke, so
  // this is no longer needed.

  // IRT_END does an implicit safepoint check, hence we are guaranteed to block
  // if this is called during a safepoint

  if (jvmdi::enabled() &&
      (JvmdiEventDispatcher::is_enabled(thread, JVMDI_EVENT_SINGLE_STEP) ||
      (!UseFastBreakpoints && jvmdi::is_bytecode_stepping()))) {
    // We are called during regular safepoints and when the VM is
    // single stepping. If this thread is marked for single stepping,
    // then we have JVM/DI work to do.
    // If we aren't using fast breakpoints, then we have JVM/DI work
    // to do for every thread.
    jvmdi::at_single_stepping_point(thread, method(thread), bcp(thread));
  }
IRT_END

IRT_ENTRY(void, InterpreterRuntime::post_field_access(JavaThread *thread, oop obj,
ConstantPoolCacheEntry *cp_entry))

  // check the access_flags for the field in the klass
  instanceKlass* ik = instanceKlass::cast((klassOop)cp_entry->f1());
  typeArrayOop fields = ik->fields();
  int index = cp_entry->holder_index();
  assert(index < fields->length(), "holder index is out of range");
  // bail out if field accesses are not watched
  if ((fields->ushort_at(index) & JVM_ACC_FIELD_ACCESS_WATCHED) == 0) return;

  bool is_static = false;

  switch(cp_entry->flag_state()) {
    case btos:    // fall through
    case ctos:    // fall through
    case stos:    // fall through
    case itos:    // fall through
    case ftos:    // fall through
    case ltos:    // fall through
    case dtos:    // fall through
    case atos: break;
    default: ShouldNotReachHere(); return;
  }
  is_static = (cp_entry->bytecode_1() == Bytecodes::_getstatic);

  HandleMark hm(thread);

  Handle h_obj;
  if (!is_static) {
    // non-static field accessors have an object, but we need a handle
    assert(obj != NULL, "non-static needs an object");
    h_obj = Handle(thread, obj);
  }
  instanceKlassHandle h_cp_entry_f1(thread, (klassOop)cp_entry->f1());
  jvmdi::post_field_access_event(thread, method(thread), bcp(thread), h_cp_entry_f1, h_obj,
    jfieldIDWorkaround::to_jfieldID(h_cp_entry_f1, cp_entry->f2(), is_static));
IRT_END

IRT_ENTRY(void, InterpreterRuntime::post_field_modification(JavaThread *thread,
  oop obj, ConstantPoolCacheEntry *cp_entry, jvalue *value))

  klassOop k = (klassOop)cp_entry->f1();

  // check the access_flags for the field in the klass
  instanceKlass* ik = instanceKlass::cast(k);
  typeArrayOop fields = ik->fields();
  int index = cp_entry->holder_index();
  assert(index < fields->length(), "holder index is out of range");
  // bail out if field modifications are not watched
  if ((fields->ushort_at(index) & JVM_ACC_FIELD_MODIFICATION_WATCHED) == 0) return;

  bool   is_static = false;
  char   sig_type = NULL;

  switch(cp_entry->flag_state()) {
    case btos: sig_type = 'Z'; break;
    case ctos: sig_type = 'C'; break;
    case stos: sig_type = 'S'; break;
    case itos: sig_type = 'I'; break;
    case ftos: sig_type = 'F'; break;
    case ltos: sig_type = 'J'; break;
    case dtos: sig_type = 'D'; break;
    case atos: sig_type = 'L'; break;
    default:  ShouldNotReachHere(); return;
  }
  is_static = (cp_entry->bytecode_2() == Bytecodes::_putstatic);

  HandleMark hm(thread);
  instanceKlassHandle h_klass(thread, k);
  jfieldID fid = jfieldIDWorkaround::to_jfieldID(h_klass, cp_entry->f2(), is_static);
  jvalue fvalue = *value;

  if (sig_type == 'I' || sig_type == 'Z' || sig_type == 'C' || sig_type == 'S') {
    // 'I' instructions are used for byte, char, short and int.
    // determine which it really is, and convert
    fieldDescriptor fd;
    bool found = JvmdiInternal::get_field_descriptor(k, fid, &fd);
    // should be found (if not, leave as is)
    if (found) {
      jint ival = value->i;
      // convert value from int to appropriate type
      switch (fd.field_type()) {   
      case T_BOOLEAN:
        sig_type = 'Z';
        fvalue.i = 0; // clear it
        fvalue.z = (jboolean)ival;
        break;
      case T_BYTE:
        sig_type = 'B';
        fvalue.i = 0; // clear it
        fvalue.b = (jbyte)ival;
        break;
      case T_CHAR:
        sig_type = 'C';
        fvalue.i = 0; // clear it
        fvalue.c = (jchar)ival;
        break;
      case T_SHORT:
        sig_type = 'S';
        fvalue.i = 0; // clear it
        fvalue.s = (jshort)ival;
        break;
      case T_INT:
        // nothing to do
        break;
      default:
        // this is an integer instruction, should be one of above
        ShouldNotReachHere();
        break;
      }
    }
  }

  // convert oop to JNI handle.
  if (sig_type == 'L') {
    fvalue.l = (jobject)JNIHandles::make_local(thread, (oop)value->l);
  }

  Handle h_obj;
  if (!is_static) {
    // non-static field accessors have an object, but we need a handle
    assert(obj != NULL, "non-static needs an object");
    h_obj = Handle(thread, obj);
  }
  jvmdi::post_field_modification_event(thread, method(thread), bcp(thread), h_klass, h_obj,
                                       fid, sig_type, &fvalue);

  // Destroy the JNI handle allocated above.
  if (sig_type == 'L') {
    JNIHandles::destroy_local(fvalue.l);
  }

IRT_END

IRT_ENTRY(void, InterpreterRuntime::post_method_entry(JavaThread *thread))
  jvmdi::post_method_entry_event(thread, InterpreterRuntime::method(thread), InterpreterRuntime::last_frame(thread));
IRT_END


IRT_ENTRY(void, InterpreterRuntime::post_method_exit(JavaThread *thread))
  jvmdi::post_method_exit_event(thread, InterpreterRuntime::method(thread), InterpreterRuntime::last_frame(thread));
IRT_END

IRT_LEAF(int, InterpreterRuntime::interpreter_contains(address pc))
{
  return (Interpreter::contains(pc) ? 1 : 0);
}
IRT_END


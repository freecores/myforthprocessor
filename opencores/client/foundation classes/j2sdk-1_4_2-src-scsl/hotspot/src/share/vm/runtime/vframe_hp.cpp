#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)vframe_hp.cpp	1.137 03/01/23 12:26:44 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_vframe_hp.cpp.incl"


// ------------- deoptimizedVFrame --------------

deoptimizedVFrame::deoptimizedVFrame(const frame* fr, const RegisterMap* reg_map, JavaThread* thread)
: javaVFrame(fr, reg_map, thread) {
  assert(fr, "valid frame must be present");
  _array = thread->vframe_array_for(fr);
  _index = vframeArray::first_index();
}


deoptimizedVFrame::deoptimizedVFrame(const frame* fr, const RegisterMap* reg_map, JavaThread* thread, int index)
: javaVFrame(fr, reg_map, thread) {
  assert(fr, "valid frame must be present");
  _array = thread->vframe_array_for(fr);
  _index = index;
}


deoptimizedVFrame::deoptimizedVFrame(vframeArray* array, int index) 
  : javaVFrame(NULL, array->owner_thread()) {
  _array = array;
  _index = index;
}


deoptimizedVFrame::deoptimizedVFrame(const frame *fr, vframeArray* array, int index) 
  : javaVFrame(fr, array->owner_thread()) {
  _array = array;
  _index = index;
}


deoptimizedVFrame::deoptimizedVFrame(vframeArray* array) 
  : javaVFrame(NULL, array->owner_thread()) {
  _array = array;
  _index = vframeArray::first_index();
}

deoptimizedVFrame::deoptimizedVFrame(const frame* fr, const RegisterMap* reg_map, vframeArray* array, int index)
: javaVFrame(fr, reg_map,  array->owner_thread()) {
  assert(fr, "valid frame must be present");
  _array = array;
  _index = index;
}


bool deoptimizedVFrame::is_top() const {
  int sender_index = array()->sender_index_for(index());
  return sender_index >= array()->length();
}


StackValueCollection* deoptimizedVFrame::locals() const {  
  int idx    = index();
  int length = array()->locals_size_for(idx);  
  assert(array()->method_for(idx)->max_locals() == length, "sanity check");

  StackValueCollection* result = new StackValueCollection(length);
  for(int i = 0; i < length; i++) {    
    StackValue *sv;
    if (array()->locals_is_obj_for(idx, i)) {
      oop value = array()->locals_obj_for(idx, i);
      Handle h(value);
      sv = new StackValue(h);
    } else {
      intptr_t value = array()->locals_int_for(idx, i);                   
      sv = new StackValue(value);
    }              
    assert(sv != NULL, "sanity check");
    result->add(sv);
  }
  return result;
}


void deoptimizedVFrame::set_locals(StackValueCollection* values) const {
  if (values == NULL || values->size() == 0) return;
  int idx    = index();
  int length = array()->locals_size_for(idx);  
  assert(array()->method_for(idx)->max_locals() == length, "sanity check");
  assert(length == values->size(), "Mismatch between actual stack format and supplied data");

  for(int i = 0; i < length; i++) {    
    StackValue *sv = values->at(i);
    assert(sv != NULL, "sanity check");
    if (array()->locals_is_obj_for(idx, i)) {
      array()->set_locals_obj_for(idx, i, (sv->get_obj())());
    } else {
      array()->set_locals_int_for(idx, i, sv->get_int());
    }              
  }
}



StackValueCollection* deoptimizedVFrame::expressions() const {  
  int idx    = index();
  int length = array()->expression_stack_size_for(idx);

  StackValueCollection* result = new StackValueCollection(length);

  for(int i = 0; i < length; i++) {    
    StackValue *sv;
    if (array()->expression_stack_is_obj_for(idx, i)) {
      oop value = array()->expression_stack_obj_for(idx, i);
      Handle h(value);
      sv = new StackValue(h);
    } else {
      intptr_t value = array()->expression_stack_int_for(idx, i);
      sv = new StackValue(value);
    }              
    assert(sv != NULL, "sanity check");
    result->add(sv);
  }
  return result;
}


methodOop deoptimizedVFrame::method() const {
  return array()->method_for(index());
}


int deoptimizedVFrame::raw_bci() const {
  return array()->bci_for(index());
}


int deoptimizedVFrame::bci() const {
  int raw = raw_bci();
  return raw == SynchronizationEntryBCI ? 0 : raw;
}


GrowableArray<MonitorInfo*>* deoptimizedVFrame::monitors() const {
  MonitorArray* arr = array()->monitor_array_for(index());
  if (arr == NULL) return new GrowableArray<MonitorInfo*>(0);
  return arr->as_monitor_list();
}


vframe* deoptimizedVFrame::sender() const {
  // Find sender in vframe array. 
  vframe* f = deoptimized_sender_or_null();
  if (f != NULL)
    return f;
  assert(fr().sp() != NULL, "should not call sender() for deoptimizedVFrame created only to walk vframeArray with null SP");
  // Sender not found in vframe array. So find sender in next physical stack frame.
  return vframe::sender();
}

// Function Name: deoptimized_sender_or_null.
// This function should be called only to walk frames in vframeArray.
// Return sender in a deoptimized vframe array. Return NULL if it is a
// last vframe in the array.
deoptimizedVFrame* deoptimizedVFrame::deoptimized_sender_or_null() const {
  int sender_index = array()->sender_index_for(index());
  if (sender_index < array()->length()) {
    return new deoptimizedVFrame(&_fr, array(), sender_index);
  }
  return NULL;
}


int deoptimizedVFrame::on_stack_size(int callee_parameters,
                                     int callee_locals,
                                     int popframe_extra_stack_expression_els,
                                     bool is_top_frame) const {
  assert(method()->max_locals() == array()->locals_size_for(index()), "just checking");
  int locks = monitors()->length();
  int temps = array()->expression_stack_size_for(index());
  return Interpreter::size_activation(method(), 
                                      temps + callee_parameters + popframe_extra_stack_expression_els,
                                      locks,
                                      callee_parameters,
                                      callee_locals,
                                      is_top_frame);
}


int unpack_counter = 0;

void deoptimizedVFrame::unpack_vframe_on_stack(int callee_parameters,
						int callee_locals,
						frame* caller,
						frame* interpreter_frame,
						bool is_top_frame,
						int exec_mode) {
#if  defined(CC_INTERP) && !defined(IA64)
  ShouldNotReachHere();
#else
  int locals_size = array()->locals_size_for(index());

  // Look at bci and decide on bcp and continuation pc
  address bcp;
  // C++ interpreter doesn't need a pc since it will figure out what to do when it
  // begins execution
  address pc;
  bool use_next_mdp; // true iff we should use the mdp associated with the next bci
                     // rather than the one associated with bcp
  if (raw_bci() == SynchronizationEntryBCI) {
    // We are deoptimizing while hanging in prologue code for synchronized method
    bcp = method()->bcp_from(0); // first byte code
    pc  = Interpreter::deopt_entry(vtos, 0); // step = 0 since we don't skip current bytecode
    use_next_mdp = false;
  } else {
    bcp = method()->bcp_from(bci());
    pc  = Interpreter::continuation_for(method(), bcp, callee_parameters, is_top_frame, use_next_mdp);
  }
  assert(Bytecodes::is_defined(*bcp), "must be a valid bytecode");

  // Monitorenter and pending exceptions:
  //
  // For Compiler2, there should be no pending exception when deoptimizing at monitorenter
  // because there is no safepoint at the null pointer check (it is either handled explicitly
  // or prior to the monitorenter) and asynchronous exceptions are not made "pending" by the
  // runtime interface for the slow case (see JRT_ENTRY_FOR_MONITORENTER).  If an asynchronous 
  // exception was processed, the bytecode pointer would have to be extended one bytecode beyond
  // the monitorenter to place it in the proper exception range.
  //
  // For Compiler1, deoptimization can occur while throwing a NullPointerException at monitorenter,
  // in which case bcp should point to the monitorenter since it is within the exception's range.

  assert(*bcp != Bytecodes::_monitorenter || is_top_frame, "a _monitorenter must be a top frame");
  COMPILER2_ONLY(guarantee(*bcp != Bytecodes::_monitorenter || exec_mode != Deoptimization::Unpack_exception,
                           "shouldn't get exception during monitorenter");)

  int popframe_preserved_args_size_in_bytes = 0;
  int popframe_preserved_args_size_in_words = 0;
  if (is_top_frame) {
#ifdef HOTSWAP
    if (jvmdi::enabled() && FullSpeedJVMDI &&
        (thread()->has_pending_popframe() || thread()->popframe_forcing_deopt_reexecution())) {
      if (thread()->has_pending_popframe()) {
        // Pop top frame after deoptimization
        pc = Interpreter::remove_activation_preserving_args_entry();
      } else {
        // Reexecute invoke in top frame
        pc = Interpreter::deopt_entry(vtos, 0);
        popframe_preserved_args_size_in_bytes = in_bytes(thread()->popframe_preserved_args_size());
        // Note: the PopFrame-related extension of the expression stack size is done in
        // Deoptimization::fetch_unroll_info_helper
        popframe_preserved_args_size_in_words = in_words(thread()->popframe_preserved_args_size_in_words());
      }
    } else {
#endif HOTSWAP
      // Possibly override the previous pc computation of the top (youngest) frame
      switch (exec_mode) {
      case Deoptimization::Unpack_deopt:
        // use what we've got
        break;
      case Deoptimization::Unpack_exception:
        // exception is pending
        pc = SharedRuntime::raw_exception_handler_for_return_address(pc);
        // [phh] We're going to end up in some handler or other, so it doesn't
        // matter what mdp we point to.  See exception_handler_for_exception()
        // in interpreterRuntime.cpp.
        break;
      case Deoptimization::Unpack_uncommon_trap:
      case Deoptimization::Unpack_reexecute:
        // redo last byte code
        pc  = Interpreter::deopt_entry(vtos, 0);
        use_next_mdp = false;
        break;
      default:
        ShouldNotReachHere();
      }
#ifdef HOTSWAP
    }
#endif HOTSWAP
  }

  // Setup the interpreter frame
  GrowableArray<MonitorInfo*>* list = monitors();

  assert(method() != NULL, "method must exist");
  assert (!method()->is_synchronized() || list->length() > 0, "synchronized methods must have monitors");
  int temps = array()->expression_stack_size_for(index());

  Interpreter::layout_activation(method(),
                                 temps + popframe_preserved_args_size_in_words + callee_parameters,
				 list->length(),
				 callee_parameters,
				 callee_locals,
				 caller,
				 interpreter_frame,
				 is_top_frame);

  // Overwrite the temporary pc we put in the skeletal frame
  // This only writes the pc into the frame object but does 
  // not update the actual return address. A later set_sender_pc would
  // do the real work. Now with fully walkable frames we can do both at once.

  // Update the actual pc in memory and in the frame object.
  interpreter_frame->patch_pc(Thread::current(), pc);

  BasicObjectLock* top = interpreter_frame->interpreter_frame_monitor_begin();
  for (int index = 0; index < list->length(); index++) {
    top = interpreter_frame->previous_monitor_in_interpreter_frame(top);
    MonitorInfo* src = list->at(index);
    top->set_obj(src->owner());
    src->lock()->move_to(src->owner(), top->lock());
  }
#ifndef CORE
  if (ProfileInterpreter) {
    interpreter_frame->interpreter_frame_set_mdx(0); // clear out the mdp.
  }
#endif // CORE
  interpreter_frame->interpreter_frame_set_bcx((intptr_t)bcp); // cannot use bcp because frame is not initialized yet
#ifndef CORE
  if (ProfileInterpreter) {
    methodDataOop mdo = method()->method_data();
    if (mdo != NULL) {
      int bci = interpreter_frame->interpreter_frame_bci();
      if (use_next_mdp) ++bci;
      address mdp = mdo->bci_to_dp(bci);
      interpreter_frame->interpreter_frame_set_mdp(mdp);
    }
  }
#endif // CORE

  // Unpack expression stack  
  // If this is an intermediate frame (i.e. not top frame) then this
  // only unpacks the part of the expression stack not used by callee
  // as parameters. The callee parameters are unpacked as part of the
  // callee locals.
  StackValueCollection *exprs = expressions();
  int i;
  for(i = 0; i < exprs->size(); i++) {
    StackValue *value = exprs->at(i);
    intptr_t*   addr  = &interpreter_frame->interpreter_frame_expression_stack_at(i);
    switch(value->type()) {
      case T_OBJECT: 
        *(oop*)addr = value->get_obj()();
        assert(*(oop*)addr == NULL || (*(oop*)addr)->is_oop(), "sanity check");
        break;
      case T_INT:
        *addr = value->get_int();
        break;
      default:
        ShouldNotReachHere();
    }
  }

  // Unpack the locals
  StackValueCollection *locs = locals();
  for(i = 0; i < locs->size(); i++) {
    StackValue *value = locs->at(i);
    intptr_t*       addr  = &interpreter_frame->interpreter_frame_local_at(i);
    switch(value->type()) {      
      case T_OBJECT:
        *(oop*)addr = value->get_obj()();
        assert(*(oop*)addr == 0 || (*(oop*)addr)->is_oop(), "sanity check");
        break;
      case T_INT:
        *addr = value->get_int();
        break;
      case T_CONFLICT:
        *addr = badHeapOopVal;
        break;
      default:
        ShouldNotReachHere();
    }
  }

#ifdef HOTSWAP
  if (is_top_frame && jvmdi::enabled() && FullSpeedJVMDI && thread()->popframe_forcing_deopt_reexecution()) {
    // An interpreted frame was popped but it returns to a deoptimized
    // frame. The incoming arguments to the interpreted activation
    // were preserved in thread-local storage by the
    // remove_activation_preserving_args_entry in the interpreter; now
    // we put them back into the just-unpacked interpreter frame.
    // Note that this assumes that the locals arena grows toward lower
    // addresses.
    if (popframe_preserved_args_size_in_words != 0) {
      void* saved_args = thread()->popframe_preserved_args();
      assert(saved_args != NULL, "must have been saved by interpreter");
      assert(popframe_preserved_args_size_in_words <= interpreter_frame->interpreter_frame_expression_stack_size(), "expression stack size should have been extended");
      int top_element = interpreter_frame->interpreter_frame_expression_stack_size() - 1;
      intptr_t* base;
      if (frame::interpreter_frame_expression_stack_direction() < 0) {
        base = &interpreter_frame->interpreter_frame_expression_stack_at(top_element);
      } else {
        base = interpreter_frame->interpreter_frame_expression_stack();
      }
      Memory::copy_bytes_overlapping(saved_args,
                                     base,
                                     popframe_preserved_args_size_in_bytes);
      thread()->popframe_free_preserved_args();
    }
  }
#endif HOTSWAP

#ifndef PRODUCT
  if (TraceDeoptimization && Verbose) {
    tty->print_cr("[%d Interpreted Frame]", ++unpack_counter);
    interpreter_frame->print_on(tty);
    RegisterMap map(thread());
    vframe* f = vframe::new_vframe(interpreter_frame, &map, thread());
    f->print();
    interpreter_frame->interpreter_frame_print_on(tty);
   
    tty->print_cr("locals size     %d", locs->size());
    tty->print_cr("expression size %d", exprs->size());
    
    method()->print_value();
    tty->cr();
    // method()->print_codes();
  } else if (TraceDeoptimization) {
    tty->print("     ");
    method()->print_value();
    Bytecodes::Code code = Bytecodes::java_code(Bytecodes::cast(*bcp));
    int bci = method()->bci_from(bcp);
    tty->print(" - %s", Bytecodes::name(code));
    tty->print(" @ bci %d ", bci);
    tty->print_cr("sp = " PTR_FORMAT, interpreter_frame->sp());
  }
#endif // PRODUCT

#endif /* !CC_INTERP */
}


// ------------- compiledVFrame --------------

StackValueCollection* compiledVFrame::locals() const {  
  GrowableArray<ScopeValue*>*  scv_list = scope()->locals();
  if (scv_list == NULL) return new StackValueCollection(0);

  // scv_list is the list of ScopeValues describing the JVM stack state.
  // There is one scv_list entry for every JVM stack state in use.
  int length = scv_list->length();
  StackValueCollection* result = new StackValueCollection(length);
  for( int i = 0; i < length; i++ )
    result->add( create_stack_value(scv_list->at(i)) );

  return result;
}


void compiledVFrame::set_locals(StackValueCollection* values) const {
  if (values == NULL || values->size() == 0) return;

  GrowableArray<ScopeValue*>*  scv_list = scope()->locals();
  assert(scv_list != NULL && scv_list->length() == values->size(),
         "Mismatch between actual stack format and supplied data"); 

  for (int i = 0; i < scv_list->length(); i++) {
    write_stack_value(scv_list->at(i), values->at(i));
  }
}


StackValueCollection* compiledVFrame::expressions() const {
  GrowableArray<ScopeValue*>*  scv_list = scope()->expressions();
  if (scv_list == NULL) return new StackValueCollection(0);

  // scv_list is the list of ScopeValues describing the JVM stack state.
  // There is one scv_list entry for every JVM stack state in use.
  int length = scv_list->length();
  StackValueCollection* result = new StackValueCollection(length);
  for( int i = 0; i < length; i++ )
    result->add( create_stack_value(scv_list->at(i)) );

  return result;
}

StackValue *compiledVFrame::create_stack_value(ScopeValue *sv) const {
  if (sv->is_location()) {
    // Stack or register value
    Location loc = ((LocationValue *)sv)->location();

#ifdef SPARC
    // %%%%% Callee-save floats will NOT be working on a Sparc until we
    // handle the case of a 2 floats in a single double register.
    assert( !(loc.is_register() && loc.type() == Location::float_in_dbl), "Sparc does not handle callee-save floats yet" );
#endif // SPARC

    // First find address of value

    address value_addr = loc.is_register()
      // Value was in a callee-save register
      ? register_map()->location(VMReg::Name(loc.register_number()))
      // Else value was directly saved on the stack. The frame's original stack pointer,
      // before any extension by its callee (due to Compiler1 linkage on SPARC), must be used.
      : ((address)_fr.unextended_sp()) + loc.stack_offset();

    // Then package it right depending on type
    // Note: the transfer of the data is thru a union that contains
    // an intptr_t. This is because an interpreter stack slot is
    // really an intptr_t. The use of a union containing an intptr_t
    // ensures that on a 64 bit platform we have proper alignment
    // and that we store the value where the interpreter will expect
    // to find it (i.e. proper endian). Similarly on a 32bit platform
    // using the intptr_t ensures that when a value is larger than
    // a stack slot (jlong/jdouble) that we capture the proper part
    // of the value for the stack slot in question.
    //
    switch( loc.type() ) {
    case Location::float_in_dbl: { // Holds a float in a double register?
      // The callee has no clue whether the register holds a float,
      // double or is unused.  He always saves a double.  Here we know
      // a double was saved, but we only want a float back.  Narrow the
      // saved double to the float that the JVM wants.
      assert( loc.is_register(), "floats always saved to stack in 1 word" );
      union { intptr_t p; jfloat jf; } value;
      value.p = (intptr_t) CONST64(0xDEADDEAFDEADDEAF);
      value.jf = (jfloat) *(jdouble*) value_addr;
      return new StackValue(value.p); // 64-bit high half is stack junk
    }
    case Location::int_in_long: { // Holds an int in a long register?
      // The callee has no clue whether the register holds an int,
      // long or is unused.  He always saves a long.  Here we know
      // a long was saved, but we only want an int back.  Narrow the
      // saved long to the int that the JVM wants.
      assert( loc.is_register(), "ints always saved to stack in 1 word" );
      union { intptr_t p; jint ji;} value;
      value.p = (intptr_t) CONST64(0xDEADDEAFDEADDEAF);
      value.ji = (jint) *(jlong*) value_addr;
      return new StackValue(value.p); // 64-bit high half is stack junk
    }
#ifdef _LP64
    case Location::dbl:
      // Double value in an aligned adjacent pair
      return new StackValue(*(intptr_t*)value_addr);
    case Location::lng:
      // Long   value in an aligned adjacent pair
      return new StackValue(*(intptr_t*)value_addr);
#endif
    case Location::oop: {
      Handle h(*(oop *)value_addr); // Wrap a handle around the oop
      return new StackValue(h);
    }
    case Location::addr: {
      ShouldNotReachHere(); // both C1 and C2 now inline jsrs
    }
    case Location::normal: {
      // Just copy all other bits straight through
      union { intptr_t p; jint ji;} value;
      value.p = (intptr_t) CONST64(0xDEADDEAFDEADDEAF);
      value.ji = *(jint*)value_addr;
      return new StackValue(value.p);
    }
    case Location::invalid:
      return new StackValue();
    default:
      ShouldNotReachHere();
    }

  } else if (sv->is_constant_int()) {
    // Constant int: treat same as register int.
    union { intptr_t p; jint ji;} value;
    value.p = (intptr_t) CONST64(0xDEADDEAFDEADDEAF);
    value.ji = (jint)((ConstantIntValue*)sv)->value();
    return new StackValue(value.p); 
  } else if (sv->is_constant_oop()) {
    // constant oop        
    return new StackValue(((ConstantOopReadValue *)sv)->value());
#ifdef _LP64
  } else if (sv->is_constant_double()) {
    // Constant double in a single stack slot
    union { intptr_t p; double d; } value;
    value.p = (intptr_t) CONST64(0xDEADDEAFDEADDEAF);
    value.d = ((ConstantDoubleValue *)sv)->value();
    return new StackValue(value.p);
  } else if (sv->is_constant_long()) {
    // Constant long in a single stack slot
    union { intptr_t p; jlong jl; } value;
    value.p = (intptr_t) CONST64(0xDEADDEAFDEADDEAF);
    value.jl = ((ConstantLongValue *)sv)->value();
    return new StackValue(value.p);
#endif
  }

  // Unknown ScopeValue type
  ShouldNotReachHere();    
  return new StackValue((intptr_t) 0);   // dummy  
}


void compiledVFrame::write_stack_value(ScopeValue *scope_val, StackValue *stack_val) const {
  // JVMDI Full-speed debugging always deopts
  ShouldNotReachHere();
}


BasicLock* compiledVFrame::resolve_monitor_lock(Location location) const {
  assert(location.is_stack(), "for now we only look at the stack");
  int word_offset = location.stack_offset() / wordSize;
  // (stack picture)
  // high: [     ]  word_offset + 1
  // low   [     ]  word_offset
  //       
  // sp->  [     ]  0
  // the word_offset is the distance from the stack pointer to the lowest address
  // The frame's original stack pointer, before any extension by its callee
  // (due to Compiler1 linkage on SPARC), must be used.
  return (BasicLock*) (fr().unextended_sp() + word_offset);
}


GrowableArray<MonitorInfo*>* compiledVFrame::monitors() const {
  GrowableArray<MonitorValue*>* monitors = scope()->monitors();
  if (monitors == NULL) {
    return new GrowableArray<MonitorInfo*>(0);
  }
  GrowableArray<MonitorInfo*>* result = new GrowableArray<MonitorInfo*>(monitors->length());
  for (int index = 0; index < monitors->length(); index++) {
    MonitorValue* mv = monitors->at(index);
    StackValue *owner_sv = create_stack_value(mv->owner()); // it is an oop
    result->push(new MonitorInfo(owner_sv->get_obj()(), resolve_monitor_lock(mv->basic_lock())));
  }
  return result;
}


compiledVFrame::compiledVFrame(const frame* fr, const RegisterMap* reg_map, JavaThread* thread, ScopeDesc* scope)
: javaVFrame(fr, reg_map, thread) {
  _scope  = scope;
  guarantee(_scope != NULL, "scope must be present");
}


bool compiledVFrame::is_top() const {
  // FIX IT: Remove this when new native stubs are in place
  if (scope() == NULL) return true;
  return scope()->is_top();
}


nmethod* compiledVFrame::code() const {
  return CodeCache::find_nmethod(_fr.pc());
}


methodOop compiledVFrame::method() const {
  return scope()->method()();
}


int compiledVFrame::bci() const {
  int raw = raw_bci();
  return raw == SynchronizationEntryBCI ? 0 : raw;
}


int compiledVFrame::raw_bci() const {
  return scope()->bci();
}


vframe* compiledVFrame::sender() const {
  assert( scope(), "When new stub generator is in place, then scope() can never be NULL" );
  const frame f = fr();
  return scope()->is_top() 
    ? vframe::sender()
    : new compiledVFrame(&f, register_map(), thread(), scope()->sender());
}


#ifndef PRODUCT
void compiledVFrame::verify() const {
  Unimplemented();
}
#endif // PRODUCT

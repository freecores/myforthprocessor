#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)parseHelper.cpp	1.171 03/02/28 09:18:13 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_parseHelper.cpp.incl"

//------------------------------make_jvmpi_method_entry------------------------
// JVMPI -- record entry to a method if compiled while JVMPI is turned on
void GraphKit::make_jvmpi_method_entry() {
  const TypeFunc *call_type    = OptoRuntime::jvmpi_method_entry_Type();
  address         call_address = OptoRuntime::jvmpi_method_entry_Java();
  const char     *call_name    = OptoRuntime::stub_name( call_address );
  assert(bci() == InvocationEntryBci, "must be outside all blocks");
  const TypeInstPtr *method_type = TypeInstPtr::make(TypePtr::Constant, method()->klass(), true, method(), 0);
  Node *methodOop_node = _gvn.transform( new ConPNode(method_type) );
  Node *receiver_node  = (method() && !method()->is_static()) // IF  (virtual call)
    ? map()->in(TypeFunc::Parms)                              // THEN 'this' pointer, receiver,
    : null();                                                 // ELSE NULL

  kill_dead_locals();
  make_slow_call( call_type, call_address, NULL, control(), methodOop_node, receiver_node );
}

//------------------------------make_jvmpi_method_exit-------------------------
// JVMPI -- record entry to a method if compiled while JVMPI is turned on
void GraphKit::make_jvmpi_method_exit(ciMethod* method) {
  const TypeFunc *call_type    = OptoRuntime::jvmpi_method_exit_Type();
  address         call_address = OptoRuntime::jvmpi_method_exit_Java(); // CAST_FROM_FN_PTR(address, SharedRuntime::jvmpi_method_exit); // OptoRuntime::jvmpi_method_exit_Java();
  const char     *call_name    = "jvmpi_method_exit"; // OptoRuntime::stub_name( call_address );
  // assert triggers on exception exits with other BCIs
  // assert(bci() == InvocationEntryBci, "must be outside all blocks");
  const TypeInstPtr* method_type = TypeInstPtr::make(TypePtr::Constant, method->klass(), true, method, 0);
  Node *method_node = _gvn.transform( new ConPNode(method_type) );

  kill_dead_locals();
  make_slow_call( call_type, call_address, NULL, control(), method_node, null() );
}

//=============================================================================
//------------------------------do_checkcast-----------------------------------
void Parse::do_checkcast() {
  bool will_link;
  ciKlass* klass = iter().get_klass(will_link);

  Node *obj = peek();

  // Throw uncommon trap if class is not loaded or the value we are casting
  // _from_ is not loaded, and value is not null.  If the value _is_ NULL,
  // then the checkcast does nothing.
  const TypeInstPtr *tp = _gvn.type(obj)->isa_instptr();
  if (!will_link || (tp && !tp->is_loaded())) {
    if (C->log() != NULL) {
      if (!will_link) {
        C->log()->elem("assert_null reason='checkcast' klass='%d'",
                       C->log()->identify(klass));
      }
      if (tp && !tp->is_loaded()) {
        // %%% Cannot happen?
        C->log()->elem("assert_null reason='checkcast source' klass='%d'",
                       C->log()->identify(tp->klass()));
      }
    }
    do_null_assert(obj, T_OBJECT);
    assert( stopped() || (peek() == null()), "what's left behind is null" );
    return;
  }

  Node *res = gen_checkcast(obj, makecon(TypeKlassPtr::make(klass)) );

  // Pop from stack AFTER gen_checkcast because it can uncommon trap and
  // the debug info has to be correct.
  pop();
  push(res);
}


//------------------------------do_instanceof----------------------------------
void Parse::do_instanceof() {
  if (stopped())  return;
  // We would like to return false if class is not loaded, emitting a
  // dependency, but Java requires instanceof to load its operand.

  // Throw uncommon trap if class is not loaded
  bool will_link;
  ciKlass* klass = iter().get_klass(will_link);

  if (!will_link) {
    if (C->log() != NULL) {
      C->log()->elem("assert_null reason='instanceof' klass='%d'",
                     C->log()->identify(klass));
    }
    do_null_assert(peek(), T_OBJECT);
    assert(stopped() || (peek() == null()), "null is left behind");
    if (!stopped()) {
      // The object is now known to be null.
      // Shortcut the effect of gen_instanceof and return "false" directly.
      pop();                   // pop the null
      push(_gvn.intcon(0));    // push false answer
    }
    return;
  }

  // Push the bool result back on stack
  push( gen_instanceof( pop(), makecon(TypeKlassPtr::make(klass)) ) );
}

//------------------------------array_store_check------------------------------
// pull array from stack and check that the store is valid
void Parse::array_store_check() {

  // Shorthand access to array store elements
  Node *obj = stack(_sp-1);
  Node *idx = stack(_sp-2);
  Node *ary = stack(_sp-3);

  // Extract the array klass type
  int klass_offset = oopDesc::klass_offset_in_bytes();
  Node* p = basic_plus_adr( ary, ary, klass_offset );
  // p's type is array-of-OOPS plus klass_offset
  Node* array_klass = _gvn.transform(new (3) LoadKlassNode(0, memory(TypeInstPtr::KLASS), p, TypeInstPtr::KLASS));
  // Get the array klass
  const TypeKlassPtr *tak = _gvn.type(array_klass)->is_klassptr();

  // array_klass's type is generally INexact array-of-oop.  Heroically
  // cast the array klass to EXACT array and uncommon-trap if the cast
  // fails.
  ciMethodData *md = method()->method_data();
  if( UncommonNullCast &&       // Cutout for this technique
      !C->has_array_check_deopt() && // No repeated array-store-check failures?
      // No failure seen at this bytecode?
      (md->is_empty() || !md->bci_to_data(bci())->as_BitData()->was_heroic_opt_failure_seen()) ) {
    // If the array klass is exactly it's defined type
    if( !tak->klass_is_exact() ) {
      // Make a constant out of the inexact array klass
      const TypeKlassPtr *extak = tak->cast_to_exactness(true)->is_klassptr();
      Node *con = makecon(extak);
      Node *cmp = _gvn.transform(new (3) CmpPNode( array_klass, con ));
      Node *bol = _gvn.transform(new (2) BoolNode( cmp, BoolTest::eq ));
      Node *ctrl= control();
      uncommon_trap_unless(bol,Deoptimization::Deopt_array_store_check, tak->klass());
      if( stopped() ) {         // MUST uncommon-trap?
        set_control(ctrl);      // Then Don't Do It, just fall into the normal checking
      } else {                  // Cast array klass to exactness: 
        // Use the exact constant value we know it is.
        replace_in_map(array_klass,con);
        CompileLog* log = C->log();
        if (log != NULL) {
          log->elem("cast_up reason='monomorphic_array' from='%d' to='(exact)'",
                    log->identify(tak->klass()));
        }
        array_klass = con;      // Use cast value moving forward
      }
    }
  }

  // Come here for polymorphic array klasses

  // Extract the array element class
  int element_klass_offset = objArrayKlass::element_klass_offset_in_bytes() + sizeof(oopDesc);
  Node *p2 = basic_plus_adr(array_klass, array_klass, element_klass_offset);
  Node *a_e_klass = _gvn.transform(new (3) LoadKlassNode(0, memory(tak), p2, tak));

  // Check (the hard way) and throw if not a subklass.
  // Result is ignored, we just need the CFG effects.
  gen_checkcast( obj, a_e_klass );
}


//------------------------------do_new-----------------------------------------
void Parse::do_new() {
  kill_dead_locals();

  // The allocator will coalesce int->oop copies away.  See comment in
  // coalesce.cpp about how this works.  It depends critically on the exact
  // code shape produced here, so if you are changing this code shape
  // make sure the GC info for the heap-top is correct in and around the
  // slow-path call.

  bool will_link;
  ciInstanceKlass* klass = iter().get_klass(will_link)->as_instance_klass();
  assert(will_link, "_new: typeflow responsibility");

  // Should initialize, or throw an InstantiationError?
  if (!klass->is_initialized() ||
      klass->is_abstract() || klass->is_interface() ||
      klass->name() == ciSymbol::java_lang_Class()) {
    uncommon_trap(Deoptimization::Deopt_uninitialized, klass);
    return;
  }

  Node* obj = new_instance(klass);

  // Push resultant oop onto stack
  push(obj);
}

#ifndef PRODUCT
//------------------------------dump_map_adr_mem-------------------------------
// Debug dump of the mapping from address types to MergeMemNode indices.
void Parse::dump_map_adr_mem() const {
  tty->print_cr("--- Mapping from address types to memory Nodes ---");
  MergeMemNode *mem = map() == NULL ? NULL : map()->memory()->is_MergeMem();
  for (uint i = 0; i < (uint)C->num_alias_types(); i++) {
    C->alias_type(i)->print_on(tty);
    tty->print("\t");
    // Node mapping, if any
    if (mem && i < mem->req() && mem->in(i) && mem->in(i) != mem->empty_memory()) {
      mem->in(i)->dump();
    } else {
      tty->cr();
    }
  }
}

#endif


//=============================================================================
//
// parser methods for profiling


//--------------------------increment_invocation_counter-----------------------
void Parse::increment_and_test_invocation_counter() {
  if (!count_invocations()) return;

  // Get the methodOop node.
  // Possible alternative code:
  //   const TypeInstPtr *method_type = TypeInstPtr::make(TypePtr::Constant, method()->klass(), method(), 0);
  //   Node *methodOop_node = _gvn.transform( new ConPNode(method_type) );
  const TypePtr* adr_type = TypeOopPtr::make_from_constant(method());
  Node *methodOop_node = makecon(adr_type);

  // Load the interpreter_invocation_counter from the methodOop.
  int offset = methodOopDesc::interpreter_invocation_counter_offset_in_bytes();
  Node* adr_node = basic_plus_adr(methodOop_node, methodOop_node, offset);
  Node* cnt = make_load(NULL, adr_node, TypeInt::INT, T_INT, adr_type);

  // Test the counter against the threshold...
  // This code is largely copied from the range check code in 
  // array_addressing()
  {
    // Test invocation count vs threshold
    Node *threshold = makecon(TypeInt::make(Tier2CompileThreshold));
    Node *chk   = _gvn.transform( new (3) CmpUNode( cnt, threshold) );
    BoolTest::mask btest = BoolTest::lt;
    Node *tst   = _gvn.transform( new (2) BoolNode( chk, btest) );
    // Branch to failure if threshold exceeded
    IfNode *iff = create_and_map_if( control(), tst, 0.999999f, -1.0f );
    Node *iffalse = _gvn.transform( new (1) IfFalseNode(iff) );
    // Add exit path for recompilation - just uncommon trap
    set_control(iffalse);
    { 
      PreserveJVMState pjvms(this);
      // End the current control-flow path; ensure that the bci is 0.
      // set_bci(0);
      uncommon_trap(Deoptimization::Deopt_tier1_counter);
    }

    // Else fall through
    Node *iftrue = _gvn.transform( new (1) IfTrueNode(iff) );
    set_control(iftrue);
  }

  // Add one to the counter and store
  Node* incr = _gvn.transform(new (3) AddINode(cnt, _gvn.intcon(1)));
  store_to_memory( NULL, adr_node, incr, T_INT, adr_type );
}

//-------------------------increment_profiling_counter-------------------------
void Parse::increment_profiling_counter(Node* counter_addr) {
  const TypePtr* adr_type = TypeOopPtr::BOTTOM;
  Node* cnt  = make_load(NULL, counter_addr, TypeInt::INT, T_INT, adr_type);
  Node* incr = _gvn.transform(new (3) AddINode(cnt, _gvn.intcon(1)));
  store_to_memory( NULL, counter_addr, incr, T_INT, adr_type );
}


//-------------------------increment_method_data_at----------------------------
void Parse::increment_method_data_at(ciMethodData* md, ciProfileData* data, ByteSize counter_offset) {
  // Get offset within methodDataOop of the data array
  ByteSize data_offset = methodDataOopDesc::data_offset();
  
  // Get cell offset of the ProfileData within data array
  int cell_offset = md->dp_to_di(data->dp());

  // Add in counter_offset, the # of bytes in ProfileData of counter
  int offset = in_bytes(data_offset) + cell_offset + in_bytes(counter_offset);

  Node* mdo_node = makecon(TypeOopPtr::make_from_constant(md));
  Node* adr_node = basic_plus_adr(mdo_node, mdo_node, offset);
  increment_profiling_counter(adr_node);
}

//----------------------------profile_taken_branch-----------------------------
void Parse::profile_taken_branch(int bci) {
  if (!method_data_update()) return;

  ciMethodData* md = method()->method_data();
  assert(md != NULL, "expected valid ciMethodData");
  ciProfileData* data = md->bci_to_data(bci);
  assert(data->is_JumpData(), "need JumpData for taken branch");
  increment_method_data_at(md, data, JumpData::taken_offset());
}

//--------------------------profile_not_taken_branch---------------------------
void Parse::profile_not_taken_branch(int bci) {
  if (!method_data_update()) return;

  ciMethodData* md = method()->method_data();
  assert(md != NULL, "expected valid ciMethodData");
  ciProfileData* data = md->bci_to_data(bci);
  assert(data->is_BranchData(), "need BranchData for not taken branch");
  increment_method_data_at(md, data, BranchData::not_taken_offset());
}

//---------------------------------profile_call--------------------------------
void Parse::profile_call(int bci) {
  if( method_data_update() ) {
    switch (bc()) {
    case Bytecodes::_invokevirtual:
    case Bytecodes::_invokeinterface:
      profile_virtual_call(bci);
      break;
    case Bytecodes::_invokestatic:  
    case Bytecodes::_invokespecial:
      profile_static_call(bci);
      break;
    default: fatal("unexpected call bytecode");
    }
  }
}

//------------------------------profile_static_call----------------------------
void Parse::profile_static_call(int bci) {
  assert(method_data_update(), "must be generating profile code");

  ciMethodData* md = method()->method_data();
  assert(md != NULL, "expected valid ciMethodData");
  ciProfileData* data = md->bci_to_data(bci);
  assert(data->is_CounterData(), "need CounterData for not taken branch");
  increment_method_data_at(md, data, CounterData::count_offset());
}

//------------------------------profile_virtual_call---------------------------
void Parse::profile_virtual_call(int bci) {
  assert(method_data_update(), "must be generating profile code");

  ciMethodData* md = method()->method_data();
  assert(md != NULL, "expected valid ciMethodData");
}

//---------------------------------profile_ret---------------------------------
void Parse::profile_ret(int bci) {
  if (!method_data_update()) return;

  ciMethodData* md = method()->method_data();
  assert(md != NULL, "expected valid ciMethodData");
}

//-----------------------------profile_checkcast-------------------------------
void Parse::profile_checkcast(int bci) {
  if (!method_data_update()) return;

  ciMethodData* md = method()->method_data();
  assert(md != NULL, "expected valid ciMethodData");
}

//-----------------------------profile_switch_case-----------------------------
void Parse::profile_switch_case(int bci, int table_index) {
  if (!method_data_update()) return;

  ciMethodData* md = method()->method_data();
  assert(md != NULL, "expected valid ciMethodData");

  ciProfileData* data = md->bci_to_data(bci);
  assert(data->is_MultiBranchData(), "need MultiBranchData for switch case");
  if (table_index < 0) {
    increment_method_data_at(md, data, MultiBranchData::default_count_offset());
  } else {
    increment_method_data_at(md, data, MultiBranchData::case_count_offset(table_index));
  }
}



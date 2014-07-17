#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)parse2.cpp	1.323 03/01/23 12:18:21 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_parse2.cpp.incl"

extern int explicit_null_checks_inserted, 
           explicit_null_checks_elided;

                                   

//---------------------------------array_load----------------------------------
void Parse::array_load(BasicType elem_type) {
  const Type* elem = Type::TOP;
  Node* adr = array_addressing(elem_type, 0, &elem);
  if (stopped())  return;     // guarenteed null or range check
  _sp -= 2;                   // Pop array and index
  const TypeAryPtr* adr_type = TypeAryPtr::get_array_body_type(elem_type);
  Node* ld = make_load(control(), adr, elem, elem_type, adr_type);
  push(ld);
}


//--------------------------------array_store----------------------------------
void Parse::array_store(BasicType elem_type) {
  Node* adr = array_addressing(elem_type, 1);
  if (stopped())  return;     // guarenteed null or range check
  Node* val = pop();
  _sp -= 2;                   // Pop array and index
  const TypeAryPtr* adr_type = TypeAryPtr::get_array_body_type(elem_type);
  store_to_memory(control(), adr, val, elem_type, adr_type);
}


//------------------------------array_addressing-------------------------------
// Pull array and index from the stack.  Compute pointer-to-element.
Node* Parse::array_addressing(BasicType type, int vals, const Type* *result2) {
  uint stride = type2aelembytes[type];
  Node *idx   = peek(0+vals);   // Get from stack without popping
  Node *ary   = peek(1+vals);   // in case of exception

  // Null check the array base, with correct stack contents
  ary = do_null_check(ary, T_ARRAY);
  // Compile-time detect of null-exception?
  if (stopped())  return top();

  const TypeAryPtr* arytype  = _gvn.type(ary)->is_aryptr();
  const TypeInt*    sizetype = arytype->size();
  const Type*       elemtype = arytype->elem();

  if (UseUniqueSubclasses && result2 != NULL) {
    const TypeInstPtr* toop = elemtype->isa_instptr();
    if (toop) {
      if (toop->klass()->as_instance_klass()->unique_concrete_subklass()) {
	// If we load from "AbstractClass[]" we must see "ConcreteSubClass".
	const Type* subklass = Type::get_const_type(toop->klass());
	elemtype = subklass->join(elemtype);
      }
    }
  }

  // Check for big class initializers with all constant offsets 
  // feeding into a known-size array.
  const TypeInt* idxtype = _gvn.type(idx)->is_int();
  // See if the highest idx value is less than the lowest array bound,
  // and if the idx value cannot be negative:
  bool need_range_check = true;
  if (idxtype->_hi < sizetype->_lo && idxtype->_lo >= 0) {
    need_range_check = false;
    if (C->log() != NULL)   C->log()->elem("observe that='!need_range_check'");
  }

  if (!arytype->klass()->is_loaded()) {
    // Only fails for some -Xcomp runs
    uncommon_trap(Deoptimization::Deopt_uninitialized, arytype->klass(), "!loaded array");
    return top();
  }

  // Do the range check
  if (GenerateRangeChecks && need_range_check) {
    // Range is constant in array-oop, so we can use the original state of mem
    Node *len   = array_length(ary);
    // Test length vs index (standard trick using unsigned compare)
    Node *chk   = _gvn.transform( new (3) CmpUNode( idx, len) );
    BoolTest::mask btest = BoolTest::lt;
    Node *tst   = _gvn.transform( new (2) BoolNode( chk, btest) );
    // Branch to failure if out of bounds
    uncommon_trap_unless(tst, Deoptimization::Deopt_range_check);
  }
  // Check for always knowing you are throwing a range-check exception
  if (stopped())  return top();

  // must be correct type for alignment purposes
  Node *base  = basic_plus_adr( ary, ary, arrayOopDesc::base_offset_in_bytes(type)); 
  Node *c4    = _gvn.MakeConX(stride);
#ifdef _LP64
  // The scaled index operand to AddP must be a clean 64-bit value.
  // Java allows a 32-bit int to be incremented to a negative
  // value, which appears in a 64-bit register as a large
  // positive number.  Using that large positive number as an
  // operand in pointer arithmetic has bad consequences.
  // On the other hand, 32-bit overflow is rare, and the possibility
  // can often be excluded, if we annotate the ConvI2L node with
  // a type assertion that its value is known to be a small positive
  // number.  (The prior range check has ensured this.)
  // This assertion is used by ConvI2LNode::Ideal.
  const TypeLong* lidxtype = TypeLong::make(0, sizetype->_hi-1);
  idx = _gvn.transform( new (2) ConvI2LNode(idx, lidxtype) );
#endif
  Node *scale = _gvn.transform( new (3) MulXNode( idx, c4 ) );
  Node *ptr   = _gvn.transform( new (4) AddPNode( ary, base, scale ) );

  if (result2 != NULL)  *result2 = elemtype;
  return ptr;
}


// returns IfNode
IfNode* Parse::jump_if_fork_int(Node* a, Node* b, BoolTest::mask mask) {
  Node   *cmp = _gvn.transform( new (3) CmpINode( a, b)); // two cases: shiftcount > 32 and shiftcount <= 32
  Node   *tst = _gvn.transform( new (2) BoolNode( cmp, mask));
  IfNode *iff = create_and_map_if( control(), tst, ((mask == BoolTest::eq) ? 0.1f : 0.5f), -1.0f );
  return iff;
}

// return Region node
Node* Parse::jump_if_join(Node* iffalse, Node* iftrue) {
  Node *region  = new RegionNode(3); // 2 results
  record_for_igvn(region);
  region->set_req(1, iffalse);
  region->set_req(2, iftrue );
  _gvn.set_type(region, Type::CONTROL);
  region = _gvn.transform(region);
  set_control (region);
  return region;
}


//------------------------------helper for tableswitch-------------------------
void Parse::jump_if_true_fork(IfNode *iff, int dest_bci_if_true) {
  // True branch, use existing map info
  { PreserveJVMState pjvms(this);
    Node *iftrue  = _gvn.transform( new (1) IfTrueNode (iff) );
    set_control( iftrue );
    merge_new_path(dest_bci_if_true);
  }

  // False branch
  Node *iffalse = _gvn.transform( new (1) IfFalseNode(iff) );
  set_control( iffalse );
}

void Parse::jump_if_false_fork(IfNode *iff, int dest_bci_if_true) {
  // True branch, use existing map info
  { PreserveJVMState pjvms(this);
    Node *iffalse  = _gvn.transform( new (1) IfFalseNode (iff) );
    set_control( iffalse );
    merge_new_path(dest_bci_if_true);
  }

  // False branch
  Node *iftrue = _gvn.transform( new (1) IfTrueNode(iff) );
  set_control( iftrue );
}

void Parse::jump_if_always_fork(int dest_bci) {
  // False branch, use existing map and control()
  merge_new_path(dest_bci);
}


extern "C" {
  static int jint_cmp(const void *i, const void *j) {
    int a = *(jint *)i;
    int b = *(jint *)j;
    return a > b ? 1 : a < b ? -1 : 0;
  }
}


class SwitchRange : public StackObj {
  // a range of integers coupled with a bci destination
  jint _lo;			// inclusive lower limit
  jint _hi;			// inclusive upper limit
  int _dest;
  int _table_index;		// index into method data table

public:
  jint lo() const              { return _lo;   }
  jint hi() const              { return _hi;   }
  int  dest() const            { return _dest; }
  int  table_index() const     { return _table_index; }
  bool is_singleton() const    { return _lo == _hi; }

  void setRange(jint lo, jint hi, int dest, int table_index) {
    assert(lo <= hi, "must be a non-empty range");
    _lo = lo, _hi = hi; _dest = dest; _table_index = table_index;
  }
  bool adjoinRange(jint lo, jint hi, int dest) {
    assert(lo <= hi, "must be a non-empty range");
    if (lo == _hi+1 && dest == _dest) {
      _hi = hi;
      return true;
    }
    return false;
  }

  void set (jint value, int dest, int table_index) {           
    setRange(value, value, dest, table_index); 
  }
  bool adjoin(jint value, int dest) { 
    return adjoinRange(value, value, dest); 
  }

  void print(ciEnv* env) {
    if (is_singleton())
      tty->print(" {%d}=>%d", lo(), dest());
    else if (lo() == min_jint)
      tty->print(" {..%d}=>%d", hi(), dest());
    else if (hi() == max_jint)
      tty->print(" {%d..}=>%d", lo(), dest());
    else
      tty->print(" {%d..%d}=>%d", lo(), hi(), dest());
  }
};


//-------------------------------do_tableswitch--------------------------------
void Parse::do_tableswitch() {
  Node* lookup = pop();

  // Get information about tableswitch
  int default_dest = iter().get_dest_table(0);
  int lo_index     = iter().get_int_table(1);
  int hi_index     = iter().get_int_table(2);
  int len          = hi_index - lo_index + 1;

  if (len < 1) {
    // If this is a backward branch, add safepoint
    maybe_add_safepoint(default_dest);
    merge(default_dest);
    return;
  }

  // generate decision tree, using trichotomy when possible
  int rnum = len+2;
  bool makes_backward_branch = false;
  SwitchRange* ranges = NEW_RESOURCE_ARRAY(SwitchRange, rnum);
  int rp = -1;
  if (lo_index-1 != min_jint) {
    ranges[++rp].setRange(min_jint, lo_index-1, default_dest, -1);
  }
  for (int j = 0; j < len; j++) {
    jint match_int = lo_index+j;
    int  dest      = iter().get_dest_table(j+3);
    makes_backward_branch |= (dest <= bci());
    if (rp < 0 || !ranges[rp].adjoin(match_int, dest)) {
      ranges[++rp].set(match_int, dest, j);
    }
  }
  jint highest = lo_index+(len-1);
  assert(ranges[rp].hi() == highest, "");
  if (highest != max_jint 
      && !ranges[rp].adjoinRange(highest+1, max_jint, default_dest)) {
    ranges[++rp].setRange(highest+1, max_jint, default_dest, -1);
  }
  assert(rp < len+2, "not too many ranges");

  // Safepoint in case if backward branch observed
  if( makes_backward_branch && UseLoopSafepoints )	
    add_safepoint();

  jump_switch_ranges(lookup, &ranges[0], &ranges[rp]);
}


//------------------------------do_lookupswitch--------------------------------
void Parse::do_lookupswitch() {
  Node *lookup = pop();		// lookup value
  // Get information about lookupswitch
  int default_dest = iter().get_dest_table(0);
  int len          = iter().get_int_table(1);

  if (len < 1) {
    // If this is a backward branch, add safepoint
    maybe_add_safepoint(default_dest);
    merge(default_dest);
    return;
  }

  // generate decision tree, using trichotomy when possible
  jint* table = NEW_RESOURCE_ARRAY(jint, len*2);
  {
    for( int j = 0; j < len; j++ ) {
      table[j+j+0] = iter().get_int_table(2+j+j);
      table[j+j+1] = iter().get_dest_table(2+j+j+1);
    }
    qsort( table, len, 2*sizeof(table[0]), jint_cmp );
  }

  int rnum = len*2+1;
  bool makes_backward_branch = false;
  SwitchRange* ranges = NEW_RESOURCE_ARRAY(SwitchRange, rnum);
  int rp = -1;
  for( int j = 0; j < len; j++ ) {
    jint match_int = table[j+j+0];
    int  dest      = table[j+j+1];
    int next_lo    = rp < 0 ? min_jint : ranges[rp].hi()+1;
    makes_backward_branch |= (dest <= bci());
    if( match_int != next_lo ) {
      ranges[++rp].setRange(next_lo, match_int-1, default_dest, -1);
    }
    if( rp < 0 || !ranges[rp].adjoin(match_int, dest) ) {
      ranges[++rp].set(match_int, dest, j);
    }
  }
  jint highest = table[2*(len-1)];
  assert(ranges[rp].hi() == highest, "");
  if( highest != max_jint 
      && !ranges[rp].adjoinRange(highest+1, max_jint, default_dest) ) {
    ranges[++rp].setRange(highest+1, max_jint, default_dest, -1);
  }
  assert(rp < rnum, "not too many ranges");

  // Safepoint in case backward branch observed
  if( makes_backward_branch && UseLoopSafepoints )	
    add_safepoint();

  jump_switch_ranges(lookup, &ranges[0], &ranges[rp]);
}

//----------------------------jump_switch_ranges-------------------------------
void Parse::jump_switch_ranges(Node* key_val, SwitchRange *lo, SwitchRange *hi, int switch_depth) {
  Block* switch_block = block();

  if (switch_depth == 0) {
    // Do special processing for the top-level call.
    assert(lo->lo() == min_jint, "initial range must exhaust Type::INT");
    assert(hi->hi() == max_jint, "initial range must exhaust Type::INT");

    // Decrement pred-numbers for the unique set of nodes.
#ifdef ASSERT
    // Ensure that the block's successors are a (duplicate-free) set.
    int successors_counted = 0;  // block occurrences in [hi..lo]
    int unique_successors = switch_block->num_successors();
    for (int i = 0; i < unique_successors; i++) {
      Block* target = switch_block->successor_at(i);

      // Check that the set of successors is the same in both places.
      int successors_found = 0;
      for (SwitchRange* p = lo; p <= hi; p++) {
	if (p->dest() == target->start())  successors_found++;
      }
      assert(successors_found > 0, "successor must be known");
      successors_counted += successors_found;
    }
    assert(successors_counted == (hi-lo)+1, "no unexpected successors");
#endif

    // Maybe prune the inputs, based on the type of key_val.
    jint min_val = min_jint;
    jint max_val = max_jint;
    const TypeInt* ti = key_val->bottom_type()->isa_int();
    if (ti != NULL) {
      min_val = ti->_lo;
      max_val = ti->_hi;
      assert(min_val <= max_val, "invalid int type");
    }
    while (lo->hi() < min_val)  lo++;
    if (lo->lo() < min_val)  lo->setRange(min_val, lo->hi(), lo->dest(), lo->table_index());
    while (hi->lo() > max_val)  hi--;
    if (hi->hi() > max_val)  hi->setRange(hi->lo(), max_val, hi->dest(), hi->table_index());
  }

#ifndef PRODUCT
  if (switch_depth == 0) {
    _max_switch_depth = 0;
    _est_switch_depth = log2_intptr((hi-lo+1)-1)+1;
  }
#endif

  assert(lo <= hi, "must be a non-empty set of ranges");
  if (lo == hi) {
    jump_if_always_fork(lo->dest());
  } else {
    assert(lo->hi() == (lo+1)->lo()-1, "contiguous ranges");
    assert(hi->lo() == (hi-1)->hi()+1, "contiguous ranges");
    int nr = hi - lo + 1;

    SwitchRange* mid = lo + nr/2;
    // if there is an easy choice, pivot at a singleton:
    if (nr > 3 && !mid->is_singleton() && (mid-1)->is_singleton())  mid--;

    assert(lo < mid && mid <= hi, "good pivot choice");
    assert(nr != 2 || mid == hi,   "should pick higher of 2");
    assert(nr != 3 || mid == hi-1, "should pick middle of 3");

    Node *test_val = _gvn.intcon(mid->lo());

    if (mid->is_singleton()) {
      IfNode *iff_ne = jump_if_fork_int(key_val, test_val, BoolTest::ne);
      jump_if_false_fork(iff_ne, mid->dest());

      // Special Case:  If there are exactly three ranges, and the high
      // and low range each go to the same place, omit the "gt" test,
      // since it will not discriminate anything.
      bool eq_test_only = (hi == lo+2 && hi->dest() == lo->dest());
      if (eq_test_only) {
	assert(mid == hi-1, "");
      }

      // if there is a higher range, test for it and process it:
      if (mid < hi && !eq_test_only) {
	// two comparisons of same values--should enable 1 test for 2 branches
        // Use BoolTest::le instead of BoolTest::gt
	IfNode *iff_le  = jump_if_fork_int(key_val, test_val, BoolTest::le);
	Node   *iftrue  = _gvn.transform( new (1) IfTrueNode(iff_le) );
	Node   *iffalse = _gvn.transform( new (1) IfFalseNode(iff_le) );
	{ PreserveJVMState pjvms(this);
	  set_control(iffalse);
	  jump_switch_ranges(key_val, mid+1, hi, switch_depth+1);
	}
	set_control(iftrue);
      }

    } else {
      // mid is a range, not a singleton, so treat mid..hi as a unit
      IfNode *iff_ge = jump_if_fork_int(key_val, test_val, BoolTest::ge);

      // if there is a higher range, test for it and process it:
      if (mid == hi) {
	jump_if_true_fork(iff_ge, mid->dest());
      } else {
	Node *iftrue  = _gvn.transform( new (1) IfTrueNode(iff_ge) );
	Node *iffalse = _gvn.transform( new (1) IfFalseNode(iff_ge) );
	{ PreserveJVMState pjvms(this);
	  set_control(iftrue);
	  jump_switch_ranges(key_val, mid, hi, switch_depth+1);
	}
	set_control(iffalse);
      }
    }

    // in any case, process the lower range
    jump_switch_ranges(key_val, lo, mid-1, switch_depth+1);
  }

  // Decrease pred_count for each successor after all is done.
  if (switch_depth == 0) {
    int unique_successors = switch_block->num_successors();
    for (int i = 0; i < unique_successors; i++) {
      Block* target = switch_block->successor_at(i);
      // Throw away the pre-allocated path for each unique successor.
      target->next_path_num();
    }
  }

#ifndef PRODUCT
  _max_switch_depth = MAX2(switch_depth, _max_switch_depth);
  if (TraceOptoParse && Verbose && WizardMode && switch_depth == 0) {
    SwitchRange* r;
    int nsing = 0;
    for( r = lo; r <= hi; r++ ) {
      if( r->is_singleton() )  nsing++;
    }
    tty->print(">>> ");
    _method->print_short_name();
    tty->print_cr(" switch decision tree");
    tty->print_cr("    %d ranges (%d singletons), max_depth=%d, est_depth=%d",
		  hi-lo+1, nsing, _max_switch_depth, _est_switch_depth);
    if (_max_switch_depth > _est_switch_depth) {
      tty->print_cr("******** BAD SWITCH DEPTH ********");
    }
    tty->print("   ");
    for( r = lo; r <= hi; r++ ) {
      r->print(env());
    }
    tty->print_cr("");
  }
#endif
}

void Parse::modf() {
  Node *f2 = pop();
  Node *f1 = pop();
  CallRuntimeNode *call = new CallLeafNode(OptoRuntime::modf_Type(), CAST_FROM_FN_PTR(address, SharedRuntime::frem), "frem");

  set_predefined_input_for_runtime_call(call);
  call->set_req( TypeFunc::Parms+0, f1 );
  call->set_req( TypeFunc::Parms+1, f2 );

  Node* c = _gvn.transform(call);

  // Set fixed predefined return values
  set_predefined_output_for_runtime_call(c);
  Node* res = _gvn.transform(new (1) ProjNode(c, TypeFunc::Parms + 0));

  push(res);
}

void Parse::modd() {
  Node *d2 = pop_pair();
  Node *d1 = pop_pair();
  CallRuntimeNode *call = new CallLeafNode(OptoRuntime::Math_DD_D_Type(), CAST_FROM_FN_PTR(address, SharedRuntime::drem), "drem");

  set_predefined_input_for_runtime_call(call);
  call->set_req( TypeFunc::Parms+0, d1 );
  call->set_req( TypeFunc::Parms+1, top() );
  call->set_req( TypeFunc::Parms+2, d2 );
  call->set_req( TypeFunc::Parms+3, top() );

  Node* c = _gvn.transform(call);

  // Set fixed predefined return values
  set_predefined_output_for_runtime_call(c);
  Node* res_d   = _gvn.transform(new (1) ProjNode(c, TypeFunc::Parms + 0));

#ifdef ASSERT
  Node* res_top = _gvn.transform(new (1) ProjNode(c, TypeFunc::Parms + 1));
  assert(res_top == top(), "second value must be top");
#endif

  push_pair(res_d);
}

void Parse::do_irem() {
  // Must keep both values on the expression-stack during null-check
  do_null_check(peek(), T_INT); 
  // Compile-time detect of null-exception?
  if (stopped())  return;

  Node* b = pop();
  Node* a = pop();  

  const Type *t = _gvn.type(b);
  if (t != Type::TOP) {
    const TypeInt *ti = t->is_int();
    if (ti->is_con()) {
      int divisor = ti->get_con();
      // check for positive power of 2
      if (divisor > 0 &&
	  (divisor & ~(divisor-1)) == divisor) {
	// yes !
	Node *mask = _gvn.intcon((divisor - 1));
	// Sigh, must handle negative dividends
	Node *zero = _gvn.intcon(0);
	IfNode *ifff = jump_if_fork_int(a, zero, BoolTest::lt);
	Node *iff = _gvn.transform( new (1) IfFalseNode(ifff) );
	Node *ift = _gvn.transform( new (1) IfTrueNode (ifff) );
	Node *reg = jump_if_join(ift, iff);
	Node *phi = PhiNode::make(reg, NULL, TypeInt::INT);
	// Negative path; negate/and/negate
	Node *neg = _gvn.transform( new (3) SubINode(zero, a) );
	Node *andn= _gvn.transform( new (3) AndINode(neg, mask) );
	Node *negn= _gvn.transform( new (3) SubINode(zero, andn) );
	phi->set_req(1, negn);
	// Fast positive case
	Node *andx = _gvn.transform( new (3) AndINode(a, mask) );
	phi->set_req(2, andx);
	// Push the merge
	push( _gvn.transform(phi) );
	return;
      }
    }
  }
  // Default case
  push( _gvn.transform( new (3) ModINode(control(),a,b) ) );
}

// Handle jsr and jsr_w bytecode
void Parse::do_jsr() {
  assert(bc() == Bytecodes::_jsr || bc() == Bytecodes::_jsr_w, "wrong bytecode");

  // Store information about current state, tagged with new _jsr_bci
  int return_bci = iter().next_bci();
  int jsr_bci    = (bc() == Bytecodes::_jsr) ? iter().get_dest() : iter().get_far_dest();

  // Update method data
  profile_taken_branch(bci());

  // The way we do things now, there is only one successor block
  // for the jsr, because the target code is cloned by ciTypeFlow.
  Block* target = successor_for_bci(jsr_bci);

  // What got pushed?
  const Type* ret_addr = target->peek();
  assert(ret_addr->singleton(), "must be a constant (cloned jsr body)");

  // Effect on jsr on stack
  push(_gvn.makecon(ret_addr));

  // Flow to the jsr.
  merge(jsr_bci);
}

// Handle ret bytecode
void Parse::do_ret() {
  // Find to whom we return.
#if 0 // %%%% MAKE THIS WORK
  Node* con = local();
  const TypePtr* tp = con->bottom_type()->isa_ptr();
  assert(tp && tp->singleton(), "");
  int return_bci = (int) tp->get_con();
  merge(return_bci);
#else
  assert(block()->num_successors() == 1, "a ret can only go one place now");
  Block* target = block()->successor_at(0);
  assert(!target->is_ready(), "our arrival must be expected");
  int pnum = target->next_path_num();
  merge_common(target, pnum);
#endif
}

#define PROB_MIN  0.000001f
#define PROB_MAX  0.999999f

//------------------------------branch_prediction------------------------------
// Try to gather dynamic branch prediction behavior.  Return a probability
// of the branch being taken and set the "cnt" field.  Returns a -1.0
// if we need to use static prediction for some reason.
float Parse::branch_prediction(float &cnt) {
  ResourceMark rm;

  cnt  = -1.0f;

  // Use MethodData information if it is available
  // FIXME: free the ProfileData structure
  ciMethodData* methodData = method()->method_data();
  if (methodData->is_empty())  return -1.0f;  // unknown
  ciProfileData* data = methodData->bci_to_data(bci());
  if (!data->is_JumpData())  return -1.0f;  // unknown

  // get taken and not taken values
  int     taken = data->as_JumpData()->taken();
  int not_taken = 0;
  if (data->is_BranchData()) {
    not_taken = data->as_BranchData()->not_taken();
  }

  // Give up if too few counts to be meaningful
  if (taken + not_taken < 40) {
    if (C->log() != NULL) {
      C->log()->elem("branch taken='%d' not_taken='%d'", taken, not_taken);
    }
    return -1.0f;
  }

  // Compute frequency that we arrive here
  int sum = taken + not_taken;
  // Adjust, if this block is a cloned private block but the
  // Jump counts are shared.  Taken the private counts for
  // just this path instead of the shared counts.
  if( block()->count() > 0 )
    sum = block()->count();
  cnt = (float)sum / (float)FreqCountInvocations;

  // Pin probability to sane limits
  float prob;
  if( !taken )
    prob = (0+PROB_MIN) / 2;
  else if( !not_taken )
    prob = (1+PROB_MAX) / 2;
  else {                         // Compute probability of true path
    prob = (float)taken / (float)(taken + not_taken);
    if (prob > PROB_MAX)  prob = PROB_MAX;
    if (prob < PROB_MIN)  prob = PROB_MIN;
  }

  assert((cnt > 0.0f) && (prob > 0.0f),
         "Bad frequency assignment in if");

  if (C->log() != NULL) {
    const char* prob_str = NULL;
    if (prob >= PROB_MAX)  prob_str = (prob == PROB_MAX) ? "max" : "always";
    if (prob <= PROB_MIN)  prob_str = (prob == PROB_MIN) ? "min" : "never";
    char prob_str_buf[30];
    if (prob_str == NULL) {
      sprintf(prob_str_buf, "%g", prob);
      prob_str = prob_str_buf;
    }
    C->log()->elem("branch taken='%d' not_taken='%d' cnt='%g' prob='%s'",
                   taken, not_taken, cnt, prob_str);
  }
  return prob;
}

// The magic constants are chosen so as to match the output of
// branch_prediction() when the profile reports a zero taken count.
// It is important to distinguish zero counts unambiguously, because
// some branches (e.g., _213_javac.Assembler.eliminate) validly produce
// very small but nonzero probabilities, which if confused with zero
// counts would keep the program recompiling indefinitely.
static inline bool seems_never_taken(float prob) {
  return prob < PROB_MIN;
}
static inline bool seems_always_taken(float prob) {
  return prob > PROB_MAX;
}

static inline void repush_if_args(Parse* p, Node* a, Node* b) {
  if (PrintOpto && WizardMode) {
    tty->print("defending against excessive implicit null exceptions on %s @%d in ",
	       Bytecodes::name(p->iter().cur_bc()), p->iter().cur_bci());
    p->method()->print_name(); tty->cr();
  }
  int bc_depth = Bytecodes::depth(p->iter().cur_bc());
  if (bc_depth == -2) {
    p->push(b); p->push(a);
  } else {
    assert(bc_depth == -1, "only two kinds of branches");
    p->push(b);
  }
}

//----------------------------------do_ifnull----------------------------------
void Parse::do_ifnull(BoolTest::mask btest) {
  int target_bci = iter().get_dest();

  float cnt;
  float prob = branch_prediction(cnt);

  // If this is a backwards branch in the bytecodes, add Safepoint
  maybe_add_safepoint(target_bci);

  explicit_null_checks_inserted++;
  Node* a = null();
  Node* b = pop();  
  Node* c = _gvn.transform( new (3) CmpPNode(b, a) );

  // Make a cast-away-nullness that is control dependent on the test
  const Type *t = _gvn.type(b);
  const Type *t_not_null = t->join(TypePtr::NOTNULL);
  Node *cast = new (2) CastPPNode(b,t_not_null);

  // Generate real control flow
  Node   *tst = _gvn.transform( new (2) BoolNode( c, btest ) );

  // If prob is -1.0f, switch to static prediction
  if( prob == -1.0f )
    prob =  (btest == BoolTest::eq) ? 0.1f : 0.9f;

  // Sanity check the probability value
  assert(prob > 0.0f,"Bad probability in Parser");
 // Need xform to put node in hash table
  IfNode *iff = create_and_xform_if( control(), tst, prob, cnt );
  assert(iff->_prob > 0.0f,"Optimizer made bad probability in parser");
  // True branch
  { PreserveJVMState pjvms(this);
    Node *iftrue  = _gvn.transform( new (1) IfTrueNode (iff) );
    set_control( iftrue );

    if( iftrue == top() ) {    // Path is dead?
      explicit_null_checks_elided++;
    }

    if (!stopped()
	&& btest == BoolTest::eq && seems_never_taken(prob)) {
      // If the branch has never yet been taken, make an uncommon trap.
      // This branch is likely to be encoded as an implicit null check,
      // so if the program changes phase and starts executing this path,
      // we need to recompile, since implicit null checks are very slow.
      repush_if_args(this, a, b);
      uncommon_trap(Deoptimization::Deopt_uninitialized, NULL, "taken never");
    }

    if (!stopped()) {         // Path is live.
      // Update method data
      profile_taken_branch(bci());

      Node *checked_value;
      if( btest == BoolTest::eq ) {
	checked_value = null();
      } else {
	cast->set_req(0,control());
	checked_value = _gvn.transform(cast);
      }
      // Make instances of value not-null on this path
      replace_in_map(b, checked_value);

      merge(target_bci);
    }
  }

  // False branch
  Node *iffalse = _gvn.transform( new (1) IfFalseNode(iff) );
  set_control( iffalse );
  
  if( iffalse == top() ) {     // Path is dead?
    explicit_null_checks_elided++;
  }

  // (See comments above.)
  if (!stopped()
      && btest != BoolTest::eq && seems_always_taken(prob)) {
    repush_if_args(this, a, b);
    uncommon_trap(Deoptimization::Deopt_uninitialized, NULL, "taken always");
  }

  if (!stopped()) {         // Path is live.
    // Update method data
    profile_not_taken_branch(bci());

    if( btest == BoolTest::eq ) {
      cast->set_req(0,control());
      cast = _gvn.transform(cast);
    } else {
      cast = null();
    }
    // Make instances of value not-null on this path
    replace_in_map(b, cast);
  }
}

//------------------------------------do_if------------------------------------
void Parse::do_if(Node* a, Node* b, BoolTest::mask btest, Node* c) {
  int target_bci = iter().get_dest();

  float cnt;
  float prob = branch_prediction(cnt);

  // If prob is -1.0f, switch to static prediction
  if (prob == -1.0f) {
    prob = 0.5f;                // Set default value
    if (btest == BoolTest::eq)  // Exactly equal test?
      prob = 0.1f;              // Assume its relatively infrequent
    else if (btest == BoolTest::ne)
      prob = 0.9f;              // Assume its relatively frequent

    // If this is a conditional test guarding a backwards branch,
    // assume its a loop-back edge.  Make it a likely taken branch.
    if( target_bci < bci() ) {
      if( is_osr_parse() ) {    // Could be a hot OSR'd loop; force deopt
        // Since it's an OSR, we "know" we have profile data.
        // Access it directly (asserts built in so not repeated here)
        assert( !method()->method_data()->is_empty(), "expected to have some data here" );
        ciProfileData* data = method()->method_data()->bci_to_data(bci());
        // Only stop for truely zero counts.  branch_prediction reports no
        // data if you have less than 40 counts.  If you have ANY counts, then
        // this loop is 'cold' relative to the OSR loop and NOT 'unknown:
        // deopt to gather more stats'.
        if( data->as_BranchData()->taken() +
            data->as_BranchData()->not_taken() == 0 ) {
#ifndef PRODUCT
          if( PrintOpto && Verbose ) 
            tty->print_cr("Never-taken backedge stops compilation at bci %d",bci());
#endif
          repush_if_args(this, a, b); // to gather stats on loop
          uncommon_trap(Deoptimization::Deopt_uninitialized, NULL, "cold");
          return;
        }
      }
      prob = 0.9f;              // Likely to take backwards branch
    }
  }

  // Sanity check the probability value
  assert(0.0f < prob && prob < 1.0f,"Bad probability in Parser");

  bool taken_if_true = true;
  // Convert BoolTest to canonical form:
  {
    BoolTest bool_test(btest);
    if (!bool_test.is_canonical()) {
      btest         = bool_test.negate();
      taken_if_true = false;
      // prob is NOT updated here; it remains the probability of the taken 
      // path (as opposed to the prob of the path guarded by an 'IfTrueNode').
    }
  }
  assert(btest != BoolTest::eq, "!= is the only canonical exact test");

  // Generate real control flow
  Node *tst = _gvn.transform( new (2) BoolNode( c, btest ) );
  IfNode *iff = create_and_map_if(control(), tst, taken_if_true ? prob : 1.0-prob, cnt);
  assert(iff->_prob > 0.0f,"Optimizer made bad probability in parser");
  Node *taken_branch  = new (1) IfTrueNode(iff);
  Node *untaken_branch = new (1) IfFalseNode(iff);
  if (!taken_if_true) {  // Finish conversion to canonical form
    Node *tmp      = taken_branch;
    taken_branch   = untaken_branch;
    untaken_branch = tmp;
  }

  // Branch is taken:
  { PreserveJVMState pjvms(this);
    taken_branch = _gvn.transform(taken_branch);
    set_control(taken_branch);

    // If this might possibly turn into an implicit null check,
    // and the branch has never yet been taken, we need to generate
    // an uncommon trap, so as to recompile instead of suffering
    // with very slow branches.  See comments above in do_ifnull.
    // We do not compare c1 and c2 against null, since they may
    // optimize to null later on.
    if (!stopped() &&
	seems_never_taken(prob) &&
	c->Opcode() == Op_CmpP) {
      repush_if_args(this, a, b);
      uncommon_trap(Deoptimization::Deopt_uninitialized, NULL, "taken never");
    }

    if (!stopped()) {
      // Update method data
      profile_taken_branch(bci());

      // Check for equal-to-constant case, and "know" that the compared
      // value is a constant on the taken branch.
      if (btest == BoolTest::ne && !taken_if_true && c->is_Cmp()) {
	Node* c1 = c->in(1);
	Node* c2 = c->in(2);
        if( c2->is_Con() ) {
          c1 = c->in(2);        // Swap, so constant is in c1
          c2 = c->in(1);
        }
        if (c1->is_Con()) {     // Constant test?
          const Type *t = c1->bottom_type();
          const TypeF *tf = t->isa_float_constant();
          const TypeD *td = t->isa_double_constant();
          // Exclude tests vs float/double 0 as these could be
          // either +0 or -0.  Just because you are equal to +0
          // doesn't mean you ARE +0!
          if( (!tf || tf->_f != 0.0) &&
              (!td || td->_d != 0.0) )
            replace_in_map(c2, c1); // replace non-constant c2 by c1
        }
      }
      merge(target_bci);
    }
  }


  untaken_branch = _gvn.transform(untaken_branch);
  set_control(untaken_branch);

  if (!stopped() &&
      seems_always_taken(prob) &&
      c->Opcode() == Op_CmpP) {
    repush_if_args(this, a, b);
    uncommon_trap(Deoptimization::Deopt_uninitialized, NULL, "taken always");
  }

  // Branch not taken.
  if (!stopped()) {
    // Update method data
    profile_not_taken_branch(bci());

    // Check for not-equal-to-constant case, and "know" that the compared
    // value is a constant on the not-taken branch.
    if (btest == BoolTest::ne && taken_if_true && c->is_Cmp()) {
      Node* c1 = c->in(1);
      Node* c2 = c->in(2);
      if( c2->is_Con() ) {
        c1 = c->in(2);          // Swap, so constant is in c1
        c2 = c->in(1);
      }
      if (c1->is_Con()) {       // Constant test?
        const Type *t = c1->bottom_type();
        const TypeF *tf = t->isa_float_constant();
        const TypeD *td = t->isa_double_constant();
        // Exclude tests vs float/double 0 as these could be
        // either +0 or -0.  Just because you are equal to +0
        // doesn't mean you ARE +0!
        if( (!tf || tf->_f != 0.0) &&
            (!td || td->_d != 0.0) )
          replace_in_map(c2, c1); // replace non-constant c2 by c1
      }
    }
  }
}


//------------------------------do_one_bytecode--------------------------------
// Parse this bytecode, and alter the Parsers JVM->Node mapping
void Parse::do_one_bytecode() {
  Node *a, *b, *c, *d;          // Handy temps
  BoolTest::mask btest;
  int i;

  assert(!has_exceptions(), "bytecode entry state must be clear of throws");

  if (C->has_root() && C->unique() > 60000) {
    set_parser_bailout(Bailout_node_limit_exceeded);
    return;
  }

#ifdef ASSERT
  // for setting breakpoints
  if( TraceOptoParse ) {
    tty->print("  ");
    dump_bci(bci());
    tty->cr();
  }
#endif

  switch (bc()) {
  case Bytecodes::_nop:
    // do nothing
    break;
  case Bytecodes::_lconst_0:
    push_pair(makecon(TypeLong::ZERO));
    break;

  case Bytecodes::_lconst_1:
    push_pair(makecon(TypeLong::ONE));
    break;

  case Bytecodes::_fconst_0:
    push(zerocon(T_FLOAT));
    break;

  case Bytecodes::_fconst_1:
    push(makecon(TypeF::ONE));
    break;

  case Bytecodes::_fconst_2:
    push(makecon(TypeF::make(2.0f)));
    break;

  case Bytecodes::_dconst_0:
    push_pair(zerocon(T_DOUBLE));
    break;

  case Bytecodes::_dconst_1:
    push_pair(makecon(TypeD::ONE));
    break;

  case Bytecodes::_iconst_m1:push(intcon(-1)); break;
  case Bytecodes::_iconst_0: push(intcon( 0)); break;
  case Bytecodes::_iconst_1: push(intcon( 1)); break;
  case Bytecodes::_iconst_2: push(intcon( 2)); break;
  case Bytecodes::_iconst_3: push(intcon( 3)); break;
  case Bytecodes::_iconst_4: push(intcon( 4)); break;
  case Bytecodes::_iconst_5: push(intcon( 5)); break;
  case Bytecodes::_bipush:   push(intcon( iter().get_byte())); break;
  case Bytecodes::_sipush:   push(intcon( iter().get_short())); break;
  case Bytecodes::_aconst_null: push(null());  break;
  case Bytecodes::_ldc:
  case Bytecodes::_ldc_w:
  case Bytecodes::_ldc2_w:
    if (iter().is_unresolved_string()) {
      uncommon_trap(Deoptimization::Deopt_uninitialized, NULL, "unresolved_string");
      break;
    }
    push_constant(iter().get_constant());
    break;

  case Bytecodes::_aload_0:
    push( local(0) );
    break;
  case Bytecodes::_aload_1:
    push( local(1) );
    break;
  case Bytecodes::_aload_2:
    push( local(2) );
    break;
  case Bytecodes::_aload_3:
    push( local(3) );
    break;
  case Bytecodes::_aload:
    push( local(iter().get_index()) );
    break;

  case Bytecodes::_fload_0:
  case Bytecodes::_iload_0:
    push( local(0) );
    break;
  case Bytecodes::_fload_1:
  case Bytecodes::_iload_1:
    push( local(1) );
    break;
  case Bytecodes::_fload_2:
  case Bytecodes::_iload_2:
    push( local(2) );
    break;
  case Bytecodes::_fload_3:
  case Bytecodes::_iload_3:
    push( local(3) );
    break;
  case Bytecodes::_fload:
  case Bytecodes::_iload:    
    push( local(iter().get_index()) );
    break;
  case Bytecodes::_lload_0:
    push_pair_local( 0 );
    break;
  case Bytecodes::_lload_1:
    push_pair_local( 1 );
    break;
  case Bytecodes::_lload_2:
    push_pair_local( 2 );
    break;
  case Bytecodes::_lload_3:
    push_pair_local( 3 );
    break;
  case Bytecodes::_lload:
    push_pair_local( iter().get_index() );
    break;

  case Bytecodes::_dload_0:
    push_pair_local(0);
    break;
  case Bytecodes::_dload_1:
    push_pair_local(1);
    break;
  case Bytecodes::_dload_2:
    push_pair_local(2);
    break;
  case Bytecodes::_dload_3:
    push_pair_local(3);
    break;
  case Bytecodes::_dload:
    push_pair_local(iter().get_index());
    break;
  case Bytecodes::_fstore_0:
  case Bytecodes::_istore_0:
  case Bytecodes::_astore_0:
    set_local( 0, pop() );
    break;
  case Bytecodes::_fstore_1:
  case Bytecodes::_istore_1:
  case Bytecodes::_astore_1:
    set_local( 1, pop() );
    break;
  case Bytecodes::_fstore_2:
  case Bytecodes::_istore_2:
  case Bytecodes::_astore_2:
    set_local( 2, pop() );
    break;
  case Bytecodes::_fstore_3:
  case Bytecodes::_istore_3:
  case Bytecodes::_astore_3:
    set_local( 3, pop() );
    break; 
  case Bytecodes::_fstore:
  case Bytecodes::_istore:
  case Bytecodes::_astore:
    set_local( iter().get_index(), pop() );
    break; 
  // long stores 
  case Bytecodes::_lstore_0:
    set_pair_local( 0, pop_pair() );
    break;
  case Bytecodes::_lstore_1:
    set_pair_local( 1, pop_pair() );
    break;
  case Bytecodes::_lstore_2:
    set_pair_local( 2, pop_pair() );
    break;
  case Bytecodes::_lstore_3:
    set_pair_local( 3, pop_pair() );
    break;
  case Bytecodes::_lstore:
    set_pair_local( iter().get_index(), pop_pair() );
    break;

  // double stores 
  case Bytecodes::_dstore_0:
    set_pair_local( 0, dstore_rounding(pop_pair()) );
    break;
  case Bytecodes::_dstore_1:
    set_pair_local( 1, dstore_rounding(pop_pair()) );
    break;
  case Bytecodes::_dstore_2:
    set_pair_local( 2, dstore_rounding(pop_pair()) );
    break;
  case Bytecodes::_dstore_3:
    set_pair_local( 3, dstore_rounding(pop_pair()) );
    break;
  case Bytecodes::_dstore:
    set_pair_local( iter().get_index(), dstore_rounding(pop_pair()) );
    break;

  case Bytecodes::_pop:  _sp -= 1;   break;
  case Bytecodes::_pop2: _sp -= 2;   break;
  case Bytecodes::_swap: 
    a = pop();
    b = pop();
    push(a);
    push(b);
    break;
  case Bytecodes::_dup:
    a = pop();
    push(a);
    push(a);
    break;
  case Bytecodes::_dup_x1: 
    a = pop();
    b = pop();
    push( a );
    push( b );
    push( a );
    break;
  case Bytecodes::_dup_x2: 
    a = pop();
    b = pop();
    c = pop();
    push( a );
    push( c );
    push( b );
    push( a );
    break;
  case Bytecodes::_dup2: 
    a = pop();
    b = pop();
    push( b );
    push( a );
    push( b );
    push( a );
    break;

  case Bytecodes::_dup2_x1:
    // before: .. c, b, a
    // after:  .. b, a, c, b, a
    // not tested
    a = pop();
    b = pop();
    c = pop();
    push( b );
    push( a );
    push( c );
    push( b );
    push( a );
    break;
  case Bytecodes::_dup2_x2:
    // before: .. d, c, b, a
    // after:  .. b, a, d, c, b, a
    // not tested
    a = pop();
    b = pop();
    c = pop();
    d = pop();
    push( b );
    push( a );
    push( d );
    push( c );
    push( b );
    push( a );
    break;

  case Bytecodes::_arraylength: {
    // Must do null-check with value on expression stack
    Node *ary = do_null_check(peek(), T_ARRAY); 
    // Compile-time detect of null-exception?
    if (stopped())  return;
    a = pop();
    push(array_length(a)); 
    break;
  } 

  case Bytecodes::_baload: array_load(T_BYTE);   break;
  case Bytecodes::_caload: array_load(T_CHAR);   break;
  case Bytecodes::_iaload: array_load(T_INT);    break;
  case Bytecodes::_saload: array_load(T_SHORT);  break;
  case Bytecodes::_faload: array_load(T_FLOAT);  break;
  case Bytecodes::_aaload: array_load(T_OBJECT); break;
  case Bytecodes::_laload: {
    a = array_addressing(T_LONG, 0);
    if (stopped())  return;     // guarenteed null or range check
    _sp -= 2;			// Pop array and index
    push_pair( make_load(control(), a, TypeLong::LONG, T_LONG, TypeAryPtr::LONGS));
    break;
  }
  case Bytecodes::_daload: {
    a = array_addressing(T_DOUBLE, 0);
    if (stopped())  return;     // guarenteed null or range check
    _sp -= 2;			// Pop array and index
    push_pair( make_load(control(), a, Type::DOUBLE, T_DOUBLE, TypeAryPtr::DOUBLES));
    break;
  }
  case Bytecodes::_bastore: array_store(T_BYTE);  break;
  case Bytecodes::_castore: array_store(T_CHAR);  break;
  case Bytecodes::_iastore: array_store(T_INT);   break;
  case Bytecodes::_sastore: array_store(T_SHORT); break;
  case Bytecodes::_fastore: array_store(T_FLOAT); break;
  case Bytecodes::_aastore: {
    a = array_addressing(T_OBJECT, 1);
    if (stopped())  return;     // guarenteed null or range check
    array_store_check();
    c = pop();			// Oop to store
    _sp -= 2;			// Pop array and index
    const TypeAryPtr* adr_type = TypeAryPtr::OOPS;
    Node* store = store_to_memory(control(), a, c, T_OBJECT, adr_type);
    store_barrier(store, a, c);                                                        
    break;
  }
  case Bytecodes::_lastore: {
    a = array_addressing(T_LONG, 2);
    if (stopped())  return;     // guarenteed null or range check
    c = pop_pair();
    _sp -= 2;			// Pop array and index
    store_to_memory(control(), a, c, T_LONG, TypeAryPtr::LONGS);
    break;
  }
  case Bytecodes::_dastore: {
    a = array_addressing(T_DOUBLE, 2);
    if (stopped())  return;     // guarenteed null or range check
    c = pop_pair();
    _sp -= 2;			// Pop array and index
    c = dstore_rounding(c);
    store_to_memory(control(), a, c, T_DOUBLE, TypeAryPtr::DOUBLES);
    break;
  }
  case Bytecodes::_getfield:
    do_getfield();
    break;

  case Bytecodes::_getstatic:
    do_getstatic();
    break;

  case Bytecodes::_putfield:
    do_putfield();
    break;

  case Bytecodes::_putstatic:
    do_putstatic();
    break;

  case Bytecodes::_irem:
    do_irem();
    break;
  case Bytecodes::_idiv:
    // Must keep both values on the expression-stack during null-check
    do_null_check(peek(), T_INT); 
    // Compile-time detect of null-exception?
    if (stopped())  return;
    b = pop();
    a = pop();  
    push( _gvn.transform( new (3) DivINode(control(),a,b) ) );
    break;
  case Bytecodes::_imul:
    b = pop(); a = pop();  
    push( _gvn.transform( new (3) MulINode(a,b) ) );
    break;
  case Bytecodes::_iadd:
    b = pop(); a = pop();  
    push( _gvn.transform( new (3) AddINode(a,b) ) );
    break;
  case Bytecodes::_ineg:
    a = pop();
    push( _gvn.transform( new (3) SubINode(_gvn.intcon(0),a)) );
    break;
  case Bytecodes::_isub:
    b = pop(); a = pop();  
    push( _gvn.transform( new (3) SubINode(a,b) ) );
    break;
  case Bytecodes::_iand:
    b = pop(); a = pop();  
    push( _gvn.transform( new (3) AndINode(a,b) ) );
    break;
  case Bytecodes::_ior:
    b = pop(); a = pop();  
    push( _gvn.transform( new (3) OrINode(a,b) ) );
    break;
  case Bytecodes::_ixor:
    b = pop(); a = pop();  
    push( _gvn.transform( new (3) XorINode(a,b) ) );
    break;
  case Bytecodes::_ishl:
    b = pop(); a = pop();
    push( _gvn.transform( new (3) LShiftINode(a,b) ) );
    break;
  case Bytecodes::_ishr:
    b = pop(); a = pop();
    push( _gvn.transform( new (3) RShiftINode(a,b) ) );
    break;
  case Bytecodes::_iushr:
    b = pop(); a = pop();
    push( _gvn.transform( new (3) URShiftINode(a,b) ) );
    break;

  case Bytecodes::_fneg:  
    a = pop();
    b = _gvn.transform(new (2) NegFNode (a));
    push(b);
    break;

  case Bytecodes::_fsub:
    b = pop();  
    a = pop();  
    c = _gvn.transform( new (3) SubFNode(a,b) );
    d = precision_rounding(c);
    push( d );
    break;

  case Bytecodes::_fadd:
    b = pop();  
    a = pop();  
    c = _gvn.transform( new (3) AddFNode(a,b) );
    d = precision_rounding(c);
    push( d );
    break;

  case Bytecodes::_fmul:
    b = pop();  
    a = pop();  
    c = _gvn.transform( new (3) MulFNode(a,b) );
    d = precision_rounding(c);
    push( d );
    break;

  case Bytecodes::_fdiv:
    b = pop();
    a = pop();  
    c = _gvn.transform( new (3) DivFNode(0,a,b) );
    d = precision_rounding(c);
    push( d );
    break;

  case Bytecodes::_frem:
    if (Matcher::modFSupported()) {
      // Generate a ModF node.
      b = pop();
      a = pop();  
      c = _gvn.transform( new (3) ModFNode(0,a,b) );
      d = precision_rounding(c);
      push( d );
    }
    else {
      // Generate a call.
      modf();
    }
    break;
    
  case Bytecodes::_fcmpl:
    b = pop();
    a = pop();
    c = _gvn.transform( new (3) CmpF3Node( a, b)); 
    push(c);
    break;
  case Bytecodes::_fcmpg:
    b = pop();
    a = pop();

    // Same as fcmpl but need to flip the unordered case.  Swap the inputs,
    // which negates the result sign except for unordered.  Flip the unordered
    // as well by using CmpF3 which implements unordered-lesser instead of
    // unordered-greater semantics.  Finally, commute the result bits.  Result
    // is same as using a CmpF3Greater except we did it with CmpF3 alone.
    c = _gvn.transform( new (3) CmpF3Node( b, a)); 
    c = _gvn.transform( new (3) SubINode(_gvn.intcon(0),c) );
    push(c);
    break;

  case Bytecodes::_f2i:
    a = pop();
    push(_gvn.transform(new (2) ConvF2INode(a)));
    break;

  case Bytecodes::_d2i:
    a = pop_pair();
    b = _gvn.transform(new (2) ConvD2INode(a));
    push( b );
    break;

  case Bytecodes::_f2d:
    a = pop();
    b = _gvn.transform( new (2) ConvF2DNode(a));
    push_pair( b );
    break;

  case Bytecodes::_d2f:
    a = pop_pair();
    b = _gvn.transform( new (2) ConvD2FNode(a));
    // This breaks _227_mtrt (speed & correctness) and _222_mpegaudio (speed)
    //b = _gvn.transform(new (2) RoundFloatNode(0, b) );
    push( b );
    break;

  case Bytecodes::_l2f:
    a = pop_pair(); 
    b = _gvn.transform( new (2) ConvL2FNode(a));
    // For i486.ad, FILD doesn't restrict precision to 24 or 53 bits.
    // Rather than storing the result into an FP register then pushing
    // out to memory to round, the machine instruction that implements
    // ConvL2D is responsible for rounding.
    // c = precision_rounding(b);
    c = _gvn.transform(b);
    push(c);
    break;

  case Bytecodes::_l2d:
    a = pop_pair(); 
    b = _gvn.transform( new (2) ConvL2DNode(a));
    // For i486.ad, rounding is always necessary (see _l2f above).
    // c = dprecision_rounding(b);
    c = _gvn.transform(b);
    push_pair(c);
    break;

  case Bytecodes::_f2l:
    a = pop();
    b = _gvn.transform( new (2) ConvF2LNode(a));
    push_pair(b);
    break;

  case Bytecodes::_d2l:
    a = pop_pair();
    b = _gvn.transform( new (2) ConvD2LNode(a));
    push_pair(b);
    break;

  case Bytecodes::_dsub:
    b = pop_pair();  
    a = pop_pair();  
    c = _gvn.transform( new (3) SubDNode(a,b) );
    d = dprecision_rounding(c);
    push_pair( d );
    break;

  case Bytecodes::_dadd:
    b = pop_pair();
    a = pop_pair();
    c = _gvn.transform( new (3) AddDNode(a,b) );
    d = dprecision_rounding(c);
    push_pair( d );
    break;

  case Bytecodes::_dmul:
    b = pop_pair();  
    a = pop_pair();  
    c = _gvn.transform( new (3) MulDNode(a,b) );
    d = dprecision_rounding(c);
    push_pair( d );
    break;

  case Bytecodes::_ddiv:
    b = pop_pair();
    a = pop_pair();  
    c = _gvn.transform( new (3) DivDNode(0,a,b) );
    d = dprecision_rounding(c);
    push_pair( d );
    break;

  case Bytecodes::_dneg: 
    a = pop_pair();
    b = _gvn.transform(new (2) NegDNode (a));
    push_pair(b);
    break;

  case Bytecodes::_drem:
    // %%%% EXPIRATION 9/07/98 move this transformation downstream
    if (Matcher::modDSupported()) {
      // Generate a ModD node.
      b = pop_pair();
      a = pop_pair();  
      // a % b
    
      c = _gvn.transform( new (3) ModDNode(0,a,b) );
      d = dprecision_rounding(c);
      push_pair( d );
    }
    else {
      // Generate a call.
      modd();
    }
    break;
    
  case Bytecodes::_dcmpl:
    b = pop_pair();
    a = pop_pair();
    c = _gvn.transform( new (3) CmpD3Node( a, b)); 
    push(c);
    break;

  case Bytecodes::_dcmpg:
    b = pop_pair();
    a = pop_pair();
    // Same as dcmpl but need to flip the unordered case.
    // Commute the inputs, which negates the result sign except for unordered.
    // Flip the unordered as well by using CmpD3 which implements 
    // unordered-lesser instead of unordered-greater semantics.
    // Finally, negate the result bits.  Result is same as using a 
    // CmpD3Greater except we did it with CmpD3 alone.
    c = _gvn.transform( new (3) CmpD3Node( b, a)); 
    c = _gvn.transform( new (3) SubINode(_gvn.intcon(0),c) );
    push(c);
    break;

   
    // Note for longs -> lo word is on TOS, hi word is on TOS - 1
  case Bytecodes::_land:
    b = pop_pair();
    a = pop_pair();
    c = _gvn.transform( new (3) AndLNode(a,b) );
    push_pair(c);
    break;
  case Bytecodes::_lor:
    b = pop_pair();
    a = pop_pair();
    c = _gvn.transform( new (3) OrLNode(a,b) );
    push_pair(c);
    break;
  case Bytecodes::_lxor:
    b = pop_pair();
    a = pop_pair();
    c = _gvn.transform( new (3) XorLNode(a,b) );
    push_pair(c);
    break;

  case Bytecodes::_lshl:
    b = pop();			// the shift count
    a = pop_pair();		// value to be shifted
    c = _gvn.transform( new (3) LShiftLNode(a,b) );
    push_pair(c);
    break;
  case Bytecodes::_lshr:
    b = pop();			// the shift count
    a = pop_pair();		// value to be shifted
    c = _gvn.transform( new (3) RShiftLNode(a,b) );
    push_pair(c);
    break;
  case Bytecodes::_lushr: 
    b = pop();			// the shift count
    a = pop_pair();		// value to be shifted
    c = _gvn.transform( new (3) URShiftLNode(a,b) );
    push_pair(c);
    break;
  case Bytecodes::_lmul: 
    b = pop_pair();
    a = pop_pair();
    c = _gvn.transform( new (3) MulLNode(a,b) );
    push_pair(c);
    break;

  case Bytecodes::_lrem: 
    // Must keep both values on the expression-stack during null-check
    assert(peek(0) == top(), "long word order");
    do_null_check(peek(1), T_LONG); 
    // Compile-time detect of null-exception?
    if (stopped())  return;
    b = pop_pair();
    a = pop_pair();
    c = _gvn.transform( new (3) ModLNode(control(),a,b) );
    push_pair(c);
    break;

  case Bytecodes::_ldiv: 
    // Must keep both values on the expression-stack during null-check
    assert(peek(0) == top(), "long word order");
    do_null_check(peek(1), T_LONG); 
    // Compile-time detect of null-exception?
    if (stopped())  return;
    b = pop_pair();
    a = pop_pair();
    c = _gvn.transform( new (3) DivLNode(control(),a,b) );
    push_pair(c);
    break;

  case Bytecodes::_ladd: 
    b = pop_pair();
    a = pop_pair();
    c = _gvn.transform( new (3) AddLNode(a,b) );
    push_pair(c);
    break;
  case Bytecodes::_lsub: 
    b = pop_pair();
    a = pop_pair();
    c = _gvn.transform( new (3) SubLNode(a,b) );
    push_pair(c);
    break;
  case Bytecodes::_lcmp: 
    // Safepoints are now inserted _before_ branches.  The long-compare
    // bytecode painfully produces a 3-way value (-1,0,+1) which requires a
    // slew of control flow.  These are usually followed by a CmpI vs zero and
    // a branch; this pattern then optimizes to the obvious long-compare and
    // branch.  However, if the branch is backwards there's a Safepoint
    // inserted.  The inserted Safepoint captures the JVM state at the
    // pre-branch point, i.e. it captures the 3-way value.  Thus if a
    // long-compare is used to control a loop the debug info will force
    // computation of the 3-way value, even though the generated code uses a
    // long-compare and branch.  We try to rectify the situation by inserting
    // a SafePoint here and have it dominate and kill the safepoint added at a
    // following backwards branch.  At this point the JVM state merely holds 2
    // longs but not the 3-way value.
    if( UseLoopSafepoints ) {
      switch( iter().next_bc() ) {
      case Bytecodes::_ifgt:
      case Bytecodes::_iflt:
      case Bytecodes::_ifge:
      case Bytecodes::_ifle:
      case Bytecodes::_ifne:
      case Bytecodes::_ifeq:
        // No hook for getting dest of next bytecode so we unconditionally
        // insert Safepoint.  Optimizer will remove extras.
        add_safepoint();        
      }
    }
    b = pop_pair();
    a = pop_pair();
    c = _gvn.transform( new (3) CmpL3Node( a, b )); 
    push(c);
    break;

  case Bytecodes::_lneg: 
    a = pop_pair();
    b = _gvn.transform( new (3) SubLNode(_gvn.makecon(TypeLong::ZERO),a));
    push_pair(b);
    break;
  case Bytecodes::_l2i:
    a = pop_pair();
    push( _gvn.transform( new (2) ConvL2INode(a)));
    break;
  case Bytecodes::_i2l:
    a = pop();
    b = _gvn.transform( new (2) ConvI2LNode(a));
    push_pair(b);
    break;
  case Bytecodes::_i2b:
    // Sign extend
    a = pop();
    a = _gvn.transform( new (3) LShiftINode(a,_gvn.intcon(24)) );
    a = _gvn.transform( new (3) RShiftINode(a,_gvn.intcon(24)) );
    push( a );
    break;
  case Bytecodes::_i2s:
    a = pop();
    a = _gvn.transform( new (3) LShiftINode(a,_gvn.intcon(16)) );
    a = _gvn.transform( new (3) RShiftINode(a,_gvn.intcon(16)) );
    push( a );
    break;
  case Bytecodes::_i2c:
    a = pop();
    push( _gvn.transform( new (3) AndINode(a,_gvn.intcon(0xFFFF)) ) );
    break;

  case Bytecodes::_i2f:
    a = pop();
    b = _gvn.transform( new (2) ConvI2FNode(a) ) ;
    c = precision_rounding(b);
    push (b);
    break;

  case Bytecodes::_i2d:
    a = pop();
    b = _gvn.transform( new (2) ConvI2DNode(a));
    push_pair(b);
    break;

  case Bytecodes::_iinc:        // Increment local
    i = iter().get_index();     // Get local index
    set_local( i, _gvn.transform( new (3) AddINode( _gvn.intcon(iter().get_iinc_con()), local(i) ) ) );
    break;

  // Exit points of synchronized methods must have an unlock node
  case Bytecodes::_return:
    return_current(NULL);
    break;

  case Bytecodes::_ireturn:
  case Bytecodes::_areturn:
  case Bytecodes::_freturn:
    return_current(pop());
    break;
  case Bytecodes::_lreturn:
    return_current(pop_pair());
    break;
  case Bytecodes::_dreturn:
    return_current(pop_pair());
    break;

  case Bytecodes::_athrow:
    // null exception oop throws NULL pointer exception
    do_null_check(peek(), T_OBJECT); 
    if (stopped())  return;
    do_athrow(Deoptimization::Deopt_athrow);
    break;

  case Bytecodes::_goto:   // fall through
  case Bytecodes::_goto_w: {
    int target_bci = (bc() == Bytecodes::_goto) ? iter().get_dest() : iter().get_far_dest();

    // If this is a backwards branch in the bytecodes, add Safepoint
    maybe_add_safepoint(target_bci);

    // Update method data
    profile_taken_branch(bci());

    // Merge the current control into the target basic block
    merge(target_bci);

    // See if we can get some profile data and hand it off to the next block
    Block *target_block = block()->successor_for_bci(target_bci);
    if( target_block->pred_count() != 1 ) break;
    ciMethodData* methodData = method()->method_data();
    if( methodData->is_empty() ) break;
    ciProfileData* data = methodData->bci_to_data(bci());
    assert( data->is_JumpData(), "" );
    target_block->set_count( ((ciJumpData*)data)->taken() );
    break;
  }

  case Bytecodes::_ifnull:
    do_ifnull(BoolTest::eq);
    break;
  case Bytecodes::_ifnonnull:
    do_ifnull(BoolTest::ne);
    break;

  case Bytecodes::_if_acmpeq: btest = BoolTest::eq; goto handle_if_acmp;
  case Bytecodes::_if_acmpne: btest = BoolTest::ne; goto handle_if_acmp;
  handle_if_acmp:
    // If this is a backwards branch in the bytecodes, add Safepoint
    maybe_add_safepoint(iter().get_dest());
    a = pop(); 
    b = pop();  
    c = _gvn.transform( new (3) CmpPNode(b, a) );
    do_if(a, b, btest, c);
    break;

  case Bytecodes::_ifeq: btest = BoolTest::eq; goto handle_ifxx;
  case Bytecodes::_ifne: btest = BoolTest::ne; goto handle_ifxx;
  case Bytecodes::_iflt: btest = BoolTest::lt; goto handle_ifxx;
  case Bytecodes::_ifle: btest = BoolTest::le; goto handle_ifxx;
  case Bytecodes::_ifgt: btest = BoolTest::gt; goto handle_ifxx;
  case Bytecodes::_ifge: btest = BoolTest::ge; goto handle_ifxx;
  handle_ifxx:
    // If this is a backwards branch in the bytecodes, add Safepoint
    maybe_add_safepoint(iter().get_dest());
    a = _gvn.intcon(0);
    b = pop();  
    c = _gvn.transform( new (3) CmpINode(b, a) );
    do_if(a, b, btest, c);
    break;

  case Bytecodes::_if_icmpeq: btest = BoolTest::eq; goto handle_if_icmp;
  case Bytecodes::_if_icmpne: btest = BoolTest::ne; goto handle_if_icmp;
  case Bytecodes::_if_icmplt: btest = BoolTest::lt; goto handle_if_icmp;
  case Bytecodes::_if_icmple: btest = BoolTest::le; goto handle_if_icmp;
  case Bytecodes::_if_icmpgt: btest = BoolTest::gt; goto handle_if_icmp;
  case Bytecodes::_if_icmpge: btest = BoolTest::ge; goto handle_if_icmp;
  handle_if_icmp: 
    // If this is a backwards branch in the bytecodes, add Safepoint
    maybe_add_safepoint(iter().get_dest());
    a = pop(); 
    b = pop();  
    c = _gvn.transform( new (3) CmpINode( b, a ) );
    do_if(a, b, btest, c);
    break;

  case Bytecodes::_tableswitch:
    do_tableswitch();
    break;

  case Bytecodes::_fast_binaryswitch:
  case Bytecodes::_fast_linearswitch:
  case Bytecodes::_lookupswitch:
    do_lookupswitch();
    break;

  case Bytecodes::_invokestatic:
  case Bytecodes::_invokespecial:
  case Bytecodes::_invokevirtual: 
  case Bytecodes::_invokeinterface:
    do_call();
    break;
  case Bytecodes::_checkcast:
    do_checkcast();
    break;
  case Bytecodes::_instanceof:
    do_instanceof();
    break;
  case Bytecodes::_anewarray: 
    do_anewarray();
    break;
  case Bytecodes::_newarray:
    do_newarray((BasicType)iter().get_index());
    break;
  case Bytecodes::_multianewarray:
    do_multianewarray();
    break;
  case Bytecodes::_new:
    do_new();
    break;

  case Bytecodes::_jsr:
  case Bytecodes::_jsr_w:
    do_jsr();
    break;
      
  case Bytecodes::_ret:
    do_ret();
    break;


  case Bytecodes::_monitorenter:
    do_monitor_enter();
    break;

  case Bytecodes::_monitorexit:
    do_monitor_exit();
    break;

  case Bytecodes::_breakpoint:
    // Breakpoint set concurrently to compile
    set_parser_bailout(Bailout_unhandled_bytecode);
    return;

  default:
#ifndef PRODUCT
    map()->dump(99);
#endif
    tty->print("\nUnhandled bytecode %s\n", Bytecodes::name(bc()) );
    ShouldNotReachHere();
  }
}

#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)methodDataOop.cpp	1.30 03/01/23 12:13:27 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_methodDataOop.cpp.incl"

// ==================================================================
// DataLayout
//
// Overlay for generic profiling data.

// Some types of data layouts need a length field.
bool DataLayout::needs_array_len(u1 tag) {
  return (tag == multi_branch_data_tag);
}

// Perform generic initialization of the data.  More specific
// initialization occurs in overrides of ProfileData::post_initialize.
void DataLayout::initialize(u1 tag, u2 bci, int cell_count) {
  _header = (intptr_t)0;
  _struct._tag = tag;
  _struct._bci = bci;
  for (int i = 0; i < cell_count; i++) {
    set_cell_at(i, (intptr_t)0);
  }
  if (needs_array_len(tag)) {
    set_cell_at(ArrayData::array_len_off_set, cell_count - 1); // -1 for header.
  }
}

// ==================================================================
// ProfileData
//
// A ProfileData object is created to refer to a section of profiling
// data in a structured way.

// Constructor for invalid ProfileData.
ProfileData::ProfileData() {
  _data = NULL;
}

#ifndef PRODUCT
void ProfileData::print_shared(outputStream* st, const char* name) {
  st->print("bci: %d", bci());
  st->fill_to(tab_width_one);
  st->print("%s", name);
  tab(st);
}

void ProfileData::tab(outputStream* st) {
  st->fill_to(tab_width_two);
}
#endif // !PRODUCT

// ==================================================================
// BitData
//
// A BitData corresponds to a one-bit flag.  This is used to indicate
// whether a checkcast bytecode has seen a null value.


#ifndef PRODUCT
void BitData::print_data_on(outputStream* st) {
  print_shared(st, "BitData");
  st->print_cr("heroic_opt_failure_seen(%s)", was_heroic_opt_failure_seen() ? "true" : "false");
}
#endif // !PRODUCT

// ==================================================================
// CounterData
//
// A CounterData corresponds to a simple counter.

#ifndef PRODUCT
void CounterData::print_data_on(outputStream* st) {
  print_shared(st, "CounterData");
  st->print_cr("count(%u)", count());
}
#endif // !PRODUCT

// ==================================================================
// JumpData
//
// A JumpData is used to access profiling information for a direct
// branch.  It is a counter, used for counting the number of branches,
// plus a data displacement, used for realigning the data pointer to
// the corresponding target bci.

void JumpData::post_initialize(BytecodeStream* stream, methodDataOop mdo) {
  assert(stream->bci() == bci(), "wrong pos");
  int target;
  Bytecodes::Code c = stream->code();
  if (c == Bytecodes::_goto_w || c == Bytecodes::_jsr_w) {
    target = stream->dest_w();
  } else {
    target = stream->dest();
  }
  int my_di = mdo->dp_to_di(dp());
  int target_di = mdo->bci_to_di(target);
  int offset = target_di - my_di;
  set_displacement(offset);
}

#ifndef PRODUCT
void JumpData::print_data_on(outputStream* st) {
  print_shared(st, "JumpData");
  st->print_cr("taken(%u) displacement(%d)", taken(), displacement());
}
#endif // !PRODUCT

// ==================================================================
// VirtualCallData
//
// A VirtualCallData is used to access profiling information about a
// virtual call.  It consists of a counter which counts the total times
// that the call is reached, and a series of (klassOop, count) pairs
// which are used to store a type profile for the virtual call.

void VirtualCallData::follow_contents() {
  for (uint row = 0; row < row_limit(); row++) {
    if (receiver(row) != NULL) {
      MarkSweep::mark_and_push(adr_receiver(row));
    }
  }
}

void VirtualCallData::oop_iterate(OopClosure* blk) {
  for (uint row = 0; row < row_limit(); row++) {
    if (receiver(row) != NULL) {
      blk->do_oop(adr_receiver(row));
    }
  }
}  

void VirtualCallData::oop_iterate_m(OopClosure* blk, MemRegion mr) {
  for (uint row = 0; row < row_limit(); row++) {
    if (receiver(row) != NULL) {
      oop* adr = adr_receiver(row);
      if (mr.contains(adr)) {
	blk->do_oop(adr);
      }
    }
  }
}  

void VirtualCallData::adjust_pointers() {
  for (uint row = 0; row < row_limit(); row++) {
    if (receiver(row) != NULL) {
      MarkSweep::adjust_pointer(adr_receiver(row));
    }
  }
}

#ifndef PRODUCT
void VirtualCallData::print_data_on(outputStream* st) {
  print_shared(st, "VirtualCallData");
  uint row;
  int entries = 0;
  for (row = 0; row < row_limit(); row++) {
    if (receiver(row) != NULL)  entries++;
  }
  st->print_cr("count(%u) entries(%u)", count(), entries);
  for (row = 0; row < row_limit(); row++) {
    if (receiver(row) != NULL) {
      tab(st);
      receiver(row)->print_value_on(st);
      st->print_cr("(%u)", receiver_count(row));
    }
  }
}
#endif // !PRODUCT

// ==================================================================
// RetData
//
// A RetData is used to access profiling information for a ret bytecode.
// It is composed of a count of the number of times that the ret has
// been executed, followed by a series of triples of the form
// (bci, count, di) which count the number of times that some bci was the
// target of the ret and cache a corresponding displacement.
  
void RetData::post_initialize(BytecodeStream* stream, methodDataOop mdo) {
  for (uint row = 0; row < row_limit(); row++) {
    set_bci(row, no_bci);
    set_bci_displacement(row, -1);
  }
}

// This routine needs to atomically update the RetData structure, so the
// caller needs to hold the RetData_lock before it gets here.  Since taking
// the lock can block (and allow GC) and since RetData is a ProfileData is a
// wrapper around a derived oop, taking the lock in _this_ method will
// basically cause the 'this' pointer's _data field to contain junk after the
// lock.  We require the caller to take the lock before making the ProfileData
// structure.  Currently the only caller is InterpreterRuntime::update_mdp_for_ret
address RetData::fixup_ret(int return_bci, methodDataHandle h_mdo) {

  // First find the mdp which corresponds to the return bci.
  address mdp = h_mdo->bci_to_dp(return_bci);

  // Now check to see if any of the cache slots are open.
  for (uint row = 0; row < row_limit(); row++) {
    if (bci(row) == no_bci) {
      set_bci_displacement(row, mdp - dp());
      set_bci_count(row, DataLayout::counter_increment);
      // Barrier to ensure displacement is written before the bci; allows
      // the interpreter to read displacement without fear of race condition.
      atomic::membar();
      set_bci(row, return_bci);
      break;
    }
  }
  return mdp;
}


#ifndef PRODUCT
void RetData::print_data_on(outputStream* st) {
  print_shared(st, "RetData");
  uint row;
  int entries = 0;
  for (row = 0; row < row_limit(); row++) {
    if (bci(row) != no_bci)  entries++;
  }
  st->print_cr("count(%u) entries(%u)", count(), entries);
  for (row = 0; row < row_limit(); row++) {
    if (bci(row) != no_bci) {
      tab(st);
      st->print_cr("bci(%d: count(%u) displacement(%d))",
		   bci(row), bci_count(row), bci_displacement(row));
    }
  }
}
#endif // !PRODUCT

// ==================================================================
// BranchData
//
// A BranchData is used to access profiling data for a two-way branch.
// It consists of taken and not_taken counts as well as a data displacement
// for the taken case.

void BranchData::post_initialize(BytecodeStream* stream, methodDataOop mdo) {
  assert(stream->bci() == bci(), "wrong pos");
  int target = stream->dest();
  int my_di = mdo->dp_to_di(dp());
  int target_di = mdo->bci_to_di(target);
  int offset = target_di - my_di;
  set_displacement(offset);
}

#ifndef PRODUCT
void BranchData::print_data_on(outputStream* st) {
  print_shared(st, "BranchData");
  st->print_cr("taken(%u) displacement(%d)",
	       taken(), displacement());
  tab(st);
  st->print_cr("not taken(%u)", not_taken());
}
#endif

// ==================================================================
// MultiBranchData
//
// A MultiBranchData is used to access profiling information for
// a multi-way branch (*switch bytecodes).  It consists of a series
// of (count, displacement) pairs, which count the number of times each
// case was taken and specify the data displacment for each branch target.

int MultiBranchData::compute_cell_count(BytecodeStream* stream) {
  int cell_count = 0;
  if (stream->code() == Bytecodes::_tableswitch) {
    Bytecode_tableswitch* sw = Bytecode_tableswitch_at(stream->bcp());
    cell_count = 1 + per_case_cell_count * (1 + sw->length()); // 1 for default
  } else {
    Bytecode_lookupswitch* sw = Bytecode_lookupswitch_at(stream->bcp());
    cell_count = 1 + per_case_cell_count * (sw->number_of_pairs() + 1); // 1 for default
  }
  return cell_count;
}

void MultiBranchData::post_initialize(BytecodeStream* stream,
				      methodDataOop mdo) {
  assert(stream->bci() == bci(), "wrong pos");
  int target;
  int my_di;
  int target_di;
  int offset;
  if (stream->code() == Bytecodes::_tableswitch) {
    Bytecode_tableswitch* sw = Bytecode_tableswitch_at(stream->bcp());
    int len = sw->length();
    assert(array_len() == per_case_cell_count * (len + 1), "wrong len");
    for (int count = 0; count < len; count++) {
      target = sw->dest_offset_at(count) + bci();
      my_di = mdo->dp_to_di(dp());
      target_di = mdo->bci_to_di(target);
      offset = target_di - my_di;
      set_displacement_at(count, offset);
    }         
    target = sw->default_offset() + bci();
    my_di = mdo->dp_to_di(dp());
    target_di = mdo->bci_to_di(target);
    offset = target_di - my_di;
    set_default_displacement(offset);

  } else {
    Bytecode_lookupswitch* sw = Bytecode_lookupswitch_at(stream->bcp());
    int npairs = sw->number_of_pairs();
    assert(array_len() == per_case_cell_count * (npairs + 1), "wrong len");
    for (int count = 0; count < npairs; count++) {
      LookupswitchPair *pair = sw->pair_at(count);
      target = pair->offset() + bci();
      my_di = mdo->dp_to_di(dp());
      target_di = mdo->bci_to_di(target);
      offset = target_di - my_di;
      set_displacement_at(count, offset);
    }
    target = sw->default_offset() + bci();
    my_di = mdo->dp_to_di(dp());
    target_di = mdo->bci_to_di(target);
    offset = target_di - my_di;
    set_default_displacement(offset);
  }
}

#ifndef PRODUCT
void MultiBranchData::print_data_on(outputStream* st) {
  print_shared(st, "MultiBranchData");
  st->print_cr("default_count(%u) displacement(%d)",
	       default_count(), default_displacement());
  int cases = number_of_cases();
  for (int i = 0; i < cases; i++) {
    tab(st);
    st->print_cr("count(%u) displacement(%d)",
		 count_at(i), displacement_at(i));
  }
}
#endif

// ==================================================================
// methodDataOop
//
// A methodDataOop holds information which has been collected about
// a method.

int methodDataOopDesc::bytecode_cell_count(Bytecodes::Code code) {
  switch (code) {
  case Bytecodes::_checkcast:
  case Bytecodes::_aastore:
    return BitData::static_cell_count();
  case Bytecodes::_invokespecial: 
  case Bytecodes::_invokestatic:
    return CounterData::static_cell_count();
  case Bytecodes::_goto: 
  case Bytecodes::_goto_w: 
  case Bytecodes::_jsr:
  case Bytecodes::_jsr_w:
    return JumpData::static_cell_count();
  case Bytecodes::_invokevirtual: 
  case Bytecodes::_invokeinterface:
    return VirtualCallData::static_cell_count();
  case Bytecodes::_ret:
    return RetData::static_cell_count();
  case Bytecodes::_ifeq:
  case Bytecodes::_ifne:
  case Bytecodes::_iflt:
  case Bytecodes::_ifge:
  case Bytecodes::_ifgt:
  case Bytecodes::_ifle:
  case Bytecodes::_if_icmpeq:
  case Bytecodes::_if_icmpne:
  case Bytecodes::_if_icmplt:
  case Bytecodes::_if_icmpge:
  case Bytecodes::_if_icmpgt:
  case Bytecodes::_if_icmple:
  case Bytecodes::_if_acmpeq:
  case Bytecodes::_if_acmpne:
  case Bytecodes::_ifnull:
  case Bytecodes::_ifnonnull:
    return BranchData::static_cell_count();
  case Bytecodes::_lookupswitch:
  case Bytecodes::_fast_linearswitch:
  case Bytecodes::_fast_binaryswitch:
  case Bytecodes::_tableswitch:
    return variable_cell_count;
  }
  return no_profile_data;
}

// Compute the size of the profiling information corresponding to
// the current bytecode.
int methodDataOopDesc::compute_data_size(BytecodeStream* stream) {
  int cell_count = bytecode_cell_count(stream->code());
  if (cell_count == no_profile_data) {
    return 0;
  }
  if (cell_count == variable_cell_count) {
    cell_count = MultiBranchData::compute_cell_count(stream);
  }
  // Note:  cell_count might be zero, meaning that there is just
  //        a DataLayout header, with no extra cells.
  assert(cell_count >= 0, "sanity");
  return DataLayout::compute_size_in_bytes(cell_count);
}

// Compute the size of the methodDataOop necessary to store
// profiling information about a given method.  Size is in bytes.
int methodDataOopDesc::compute_allocation_size_in_bytes(methodOop method) {
  int data_size = 0;
  BytecodeStream stream(method);
  Bytecodes::Code c;
  while ((c = stream.next()) >= 0) {
    int size_in_bytes = compute_data_size(&stream);
    data_size += size_in_bytes;
  }
  int size = in_bytes(data_offset()) + data_size;
  return size;
}

// Compute the size of the methodDataOop necessary to store
// profiling information about a given method.  Size is in words
int methodDataOopDesc::compute_allocation_size_in_words(methodOop method) {
  int byte_size = compute_allocation_size_in_bytes(method);
  int word_size = align_size_up(byte_size, BytesPerWord) / BytesPerWord;
  return align_object_size(word_size);
}

// Initialize an individual data segment.  Returns the size of
// the segment in bytes.
int methodDataOopDesc::initialize_data(BytecodeStream* stream,
				       int data_index) {
  int cell_count = -1;
  int tag = DataLayout::no_tag;
  DataLayout* data_layout = data_layout_at(data_index);
  Bytecodes::Code c = stream->code();
  switch (c) {
  case Bytecodes::_checkcast:
  case Bytecodes::_aastore:
    cell_count = BitData::static_cell_count();
    tag = DataLayout::bit_data_tag;
    break;
  case Bytecodes::_invokespecial:
  case Bytecodes::_invokestatic:
    cell_count = CounterData::static_cell_count();
    tag = DataLayout::counter_data_tag;
    break;
  case Bytecodes::_goto:
  case Bytecodes::_goto_w:
  case Bytecodes::_jsr:
  case Bytecodes::_jsr_w:
    cell_count = JumpData::static_cell_count();
    tag = DataLayout::jump_data_tag;
    break;
  case Bytecodes::_invokevirtual:
  case Bytecodes::_invokeinterface:
    cell_count = VirtualCallData::static_cell_count();
    tag = DataLayout::virtual_call_data_tag;
    break;
  case Bytecodes::_ret:
    cell_count = RetData::static_cell_count();
    tag = DataLayout::ret_data_tag;
    break;
  case Bytecodes::_ifeq:
  case Bytecodes::_ifne:
  case Bytecodes::_iflt:
  case Bytecodes::_ifge:
  case Bytecodes::_ifgt:
  case Bytecodes::_ifle:
  case Bytecodes::_if_icmpeq:
  case Bytecodes::_if_icmpne:
  case Bytecodes::_if_icmplt:
  case Bytecodes::_if_icmpge:
  case Bytecodes::_if_icmpgt:
  case Bytecodes::_if_icmple:
  case Bytecodes::_if_acmpeq:
  case Bytecodes::_if_acmpne:
  case Bytecodes::_ifnull:
  case Bytecodes::_ifnonnull:
    cell_count = BranchData::static_cell_count();
    tag = DataLayout::branch_data_tag;
    break;
  case Bytecodes::_lookupswitch:
  case Bytecodes::_fast_linearswitch:
  case Bytecodes::_fast_binaryswitch:
  case Bytecodes::_tableswitch:
    cell_count = MultiBranchData::compute_cell_count(stream);
    tag = DataLayout::multi_branch_data_tag;
    break;
  }
  assert(tag == DataLayout::multi_branch_data_tag ||
         cell_count == bytecode_cell_count(c), "cell counts must agree");
  if (cell_count >= 0) {
    assert(tag != DataLayout::no_tag, "bad tag");
    assert(bytecode_has_profile(c), "agree w/ BHP");
    data_layout->initialize(tag, stream->bci(), cell_count);
    return DataLayout::compute_size_in_bytes(cell_count);
  } else {
    assert(!bytecode_has_profile(c), "agree w/ !BHP");
    return 0;
  }
}

// Get the data at an arbitrary (sort of) data index.
ProfileData* methodDataOopDesc::data_at(int data_index) {
  if (out_of_bounds(data_index)) {
    return NULL;
  }
  DataLayout* data_layout = data_layout_at(data_index);
  
  switch (data_layout->tag()) {
  case DataLayout::no_tag:
  default:
    ShouldNotReachHere();
    return NULL;
  case DataLayout::bit_data_tag:
    return new BitData(data_layout);
  case DataLayout::counter_data_tag:
    return new CounterData(data_layout);
  case DataLayout::jump_data_tag:
    return new JumpData(data_layout);
  case DataLayout::virtual_call_data_tag:
    return new VirtualCallData(data_layout);
  case DataLayout::ret_data_tag:
    return new RetData(data_layout);
  case DataLayout::branch_data_tag:
    return new BranchData(data_layout);
  case DataLayout::multi_branch_data_tag:
    return new MultiBranchData(data_layout);
  };
}

// Iteration over data.
ProfileData* methodDataOopDesc::next_data(ProfileData* current) {
  int current_index = dp_to_di(current->dp());
  int next_index = current_index + current->size_in_bytes();
  ProfileData* next = data_at(next_index);
  return next;
}

// Give each of the data entries a chance to perform specific
// data initialization.
void methodDataOopDesc::post_initialize(BytecodeStream* stream) {
  ResourceMark rm;
  ProfileData* data;
  for (data = first_data(); is_valid(data); data = next_data(data)) {
    stream->set_start(data->bci());
    stream->next();
    data->post_initialize(stream, this);
  }
}

// Initialize the methodDataOop corresponding to a given method.
void methodDataOopDesc::initialize(methodOop method) {
  ResourceMark rm;

  // Set the method back-pointer.
  _method = method;
  // Initial flags
  _flags = 0;
  // initial null pointer exception count
  _null_ptr_count = 0;

  // Go through the bytecodes and allocate and initialize the
  // corresponding data cells.
  int data_size = 0;
  BytecodeStream stream(method);
  Bytecodes::Code c;
  while ((c = stream.next()) >= 0) {
    int size_in_bytes = initialize_data(&stream, data_size);
    data_size += size_in_bytes;
  }
  _size = in_bytes(data_offset()) + data_size;

  // Set an initial hint. Don't use set_hint_di() because
  // first_di() may be out of bounds if data_size is 0.
  // In that situation, _hint_di is never used, but at
  // least well-defined.
  _hint_di = first_di();

  post_initialize(&stream);
  set_object_is_parsable();
}

// Translate a bci to its corresponding data index (di).
address methodDataOopDesc::bci_to_dp(int bci) {
  ResourceMark rm;
  ProfileData* data = data_before(bci);
  ProfileData* prev = NULL;
  for ( ; is_valid(data); data = next_data(data)) {
    if (data->bci() >= bci) {
      if (data->bci() == bci)  set_hint_di(dp_to_di(data->dp()));
      else if (prev != NULL)   set_hint_di(dp_to_di(prev->dp()));
      return data->dp();
    }
    prev = data;
  }
  return (address)limit_data_position();
}

// Translate a bci to its corresponding data, or NULL.
ProfileData* methodDataOopDesc::bci_to_data(int bci) {
  ProfileData* data = data_before(bci);
  for ( ; is_valid(data); data = next_data(data)) {
    if (data->bci() == bci) {
      set_hint_di(dp_to_di(data->dp()));
      return data;
    } else if (data->bci() > bci) {
      return NULL;
    }
  }
  return NULL;
}

#ifndef PRODUCT
void methodDataOopDesc::print_data_on(outputStream* st) {
  ResourceMark rm;
  ProfileData* data = first_data();
  for ( ; is_valid(data); data = next_data(data)) {
    st->print("%d", dp_to_di(data->dp()));
    st->fill_to(6);
    data->print_data_on(st);
  }
}

void methodDataOopDesc::verify_data_on(outputStream* st) {
  NEEDS_CLEANUP;
  // not yet implemented.
}
#endif

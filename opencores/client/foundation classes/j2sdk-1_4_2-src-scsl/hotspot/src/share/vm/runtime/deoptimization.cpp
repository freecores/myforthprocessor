#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)deoptimization.cpp	1.229 03/05/08 16:02:24 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_deoptimization.cpp.incl"

bool DeoptimizationMarker::_is_active = false;

Deoptimization::UnrollBlock::UnrollBlock(int  size_of_deoptimized_frame,
                                         int  adapter_size,
					 int  new_adapter,
                                         int  caller_adjustment,
                                         int  number_of_frames,
                                         intptr_t* frame_sizes,
                                         address* frame_pcs,
                                         BasicType return_type) {
  _size_of_deoptimized_frame = size_of_deoptimized_frame;
  _adapter_size              = adapter_size;
  _new_adapter               = new_adapter;
  _caller_adjustment         = caller_adjustment;
  _number_of_frames          = number_of_frames;
  _frame_sizes               = frame_sizes;
  _frame_pcs                 = frame_pcs;
  _register_block            = NEW_C_HEAP_ARRAY(intptr_t, RegisterMap::reg_count * 2);
  _return_type               = return_type;
  // PD (x86 only)
  _counter_temp              = 0;
  _initial_fp                = 0;
  _unpack_kind               = 0;
  _sender_sp_temp            = 0;
}


Deoptimization::UnrollBlock::~UnrollBlock() {
  FREE_C_HEAP_ARRAY(intptr_t, _frame_sizes);
  FREE_C_HEAP_ARRAY(intptr_t, _frame_pcs);
  FREE_C_HEAP_ARRAY(intptr_t, _register_block);
}


intptr_t* Deoptimization::UnrollBlock::value_addr_at(int register_number) const {
  assert(register_number < RegisterMap::reg_count, "checking register number");
  return &_register_block[register_number * 2];
}



int Deoptimization::UnrollBlock::size_of_frames() const {
  // Acount first for the adjustment of the initial frame
  int result = _caller_adjustment;
  for (int index = 0; index < number_of_frames(); index++) {
    result += frame_sizes()[index];
  }
  return result;
}


void Deoptimization::UnrollBlock::print() {
  ttyLocker ttyl;
  tty->print_cr("UnrollBlock");
  tty->print_cr("  size_of_deoptimized_frame = %d", _size_of_deoptimized_frame);
  tty->print(   "  frame_sizes: ");
  for (int index = 0; index < number_of_frames(); index++) {
    tty->print("%d ", frame_sizes()[index]);
  }
  tty->cr();
}


JRT_LEAF(Deoptimization::UnrollBlock*, Deoptimization::fetch_unroll_info(JavaThread* thread))
  // It is actually ok to allocate handles in a leaf method. It causes no safepoints,
  // but makes the entry a little slower. There is however a little dance we have to
  // do in debug mode to get around the NoHandleMark code in the JRT_LEAF macro
  ResetNoHandleMark rnhm; // No-op in release/product versions  
  HandleMark hm;
  ResourceMark rm;
  COMPILER1_ONLY(JavaThread::current()->set_is_handling_implicit_exception(false);) // in case that an implicit exception in compiled code happened
  return fetch_unroll_info_helper(thread);
JRT_END


// This is factored, since it is both called from a JRT_LEAF (deoptimization) and a JRT_ENTRY (uncommon_trap)
Deoptimization::UnrollBlock* Deoptimization::fetch_unroll_info_helper(JavaThread* thread) {
  // Since the frame to unpack is the top frame of this thread, the vframe_array_head
  // must point to the vframeArray for the unpack frame.
  vframeArray* array = thread->vframe_array_head();
  frame stub_frame = thread->last_frame();	// makes stack walkable
  RegisterMap map(thread, false);

  // Compute the caller frame based on the sender sp of stub_frame and stored frame sizes info.
  CodeBlob* cb = CodeCache::find_blob(stub_frame.pc());
  // Verify we have the right vframeArray
  assert(cb->frame_size() >= 0, "Unexpected frame size");
  intptr_t* unpack_sp = stub_frame.sp() + cb->frame_size();

#ifdef ASSERT
  assert(cb->is_deoptimization_stub() || cb->is_uncommon_trap_stub(), "just checking");
#ifdef COMPILER1
#ifdef ASSERT
  if (array->unextended_sp() != unpack_sp) {
    tty->print_cr("array->unextended_sp() = "INTPTR_FORMAT "\n", array->unextended_sp());
    tty->print_cr("unpack_sp = " INTPTR_FORMAT "\n", unpack_sp);
    tty->print_cr("original pc = " INTPTR_FORMAT "\n", array->original_pc());
    nmethod* nm = (nmethod*)CodeCache::find_blob_unsafe(array->original_pc());
    nm->print();
  }
#endif // ASSERT
#endif // COMPILER1
  Events::log("fetch unroll sp " INTPTR_FORMAT, unpack_sp);
#endif
  // This is a guarantee instead of an assert because if vframe doesn't match
  // we will unpack the wrong deoptimized frame and wind up in strange places
  // where it will be very difficult to figure out what went wrong. Better
  // to die an early death here than some very obscure death later when the
  // trail is cold.
  // Note: on ia64 this guarantee can be fooled by frames with no memory stack
  // in that it will fail to detect a problem when there is one. This needs
  // more work in tiger timeframe.
  guarantee(array->unextended_sp() == unpack_sp, "vframe_array_head must contain the vframeArray to unpack");

  // Convert the representation of monitors from on-stack values (MonitorArray) to off-stack values (MonitorChunk).
  array->migrate_monitors_off_stack();

  // Collect the frames in the vframeArray.  The first frame stored in the array is the top virtual activation
  // on the stack, while the last frame is the root of the sequence of virtual activations comprising the
  // compiled frame being deoptimized.
  ResourceMark rm;
  GrowableArray<deoptimizedVFrame*>* vframes = new GrowableArray<deoptimizedVFrame*>(10);
  // Find sender within vframe array.
  for (deoptimizedVFrame* vf = new deoptimizedVFrame(array); vf != NULL; vf = vf->deoptimized_sender_or_null()) {
    vframes->push(vf);
  }

  int number_of_frames = vframes->length();
  int need_new_adapter = 0;
#ifdef COMPILER2
  // If the caller of the deoptimized frame is compiled, a new C2I adapter has been 
  // allocated and must be included in the vframe count.
  // Otherwise if the caller was not a compiled frame then a now useless I2C or
  // OSR adapter frame will be popped.

  C2IAdapter* new_adapter = array->new_adapter();
  need_new_adapter = new_adapter != NULL ? 1 : 0;
  number_of_frames += need_new_adapter;
#endif

  // Compute the vframes' sizes.  Note that frame_sizes[] entries are ordered from outermost to innermost
  // virtual activation, which is the reverse of the elements in the vframes array.
  intptr_t* frame_sizes = NEW_C_HEAP_ARRAY(intptr_t, number_of_frames);
  // +1 because we always have an interpreter return address for the final slot.
  address* frame_pcs = NEW_C_HEAP_ARRAY(address, number_of_frames + 1);
  int callee_parameters = 0;
  int callee_locals = 0;
  int popframe_extra_args = 0;
  // Create an interpreter return address for the stub to use as its return
  // address so the skeletal frames are perfectly walkable
  frame_pcs[number_of_frames] = AbstractInterpreter::deopt_entry(vtos, 0);

#ifdef HOTSWAP
  // PopFrame requires that the preserved incoming arguments from the recently-popped topmost
  // activation be put back on the expression stack of the caller for reexecution
  if (jvmdi::enabled() && FullSpeedJVMDI && thread->popframe_forcing_deopt_reexecution()) {
    popframe_extra_args = in_words(thread->popframe_preserved_args_size_in_words());
  }
#endif HOTSWAP

  //
  // frame_sizes/frame_pcs[0] oldest frame (int or c2i)
  // frame_sizes/frame_pcs[1] next oldest frame (int)
  // frame_sizes/frame_pcs[n] youngest frame (int)
  //
  // Now a pc in frame_pcs is actually the return address to the frame's caller (a frame
  // owns the space for the return address to it's caller).  Confusing ain't it.
  // 
  for (int index = 0; index < vframes->length(); index++) {
    frame_sizes[number_of_frames - 1 - index] = BytesPerWord *
						vframes->at(index)->on_stack_size(callee_parameters, 
										  callee_locals,
                                                                                  popframe_extra_args,
										  index == 0);
    // This pc doesn't have to be perfect just good enough to identify the frame
    // as interpreted so the skeleton frame will be walkable
    // The correct pc will be set when the skeleton frame is completely filled out
    // The final pc we store in the loop is wrong and will be overwritten below
    frame_pcs[number_of_frames - 1 - index] = AbstractInterpreter::deopt_entry(vtos, 0) - frame::pc_return_offset;

    callee_parameters = vframes->at(index)->method()->size_of_parameters();
    callee_locals = vframes->at(index)->method()->max_locals();
    popframe_extra_args = 0;
  }

  // Compute whether the root vframe returns a float or double value.
  BasicType return_type;
  {
    HandleMark hm;
    deoptimizedVFrame* vf = vframes->at(0);
    methodHandle method(thread, vf->method());
    Bytecode_invoke* invoke = Bytecode_invoke_at_check(method, vf->bci());
    return_type = (invoke != NULL) ? invoke->result_type(thread) : T_ILLEGAL;
  }

  // Compute information for handling adapters and adjusting the frame size of the caller.
  int adapter_size = 0;
  int caller_adjustment = 0;

  // If we are popping an adapter compute its size. 
  // Strange but true: you'd think that this size would simply be frame_size in the
  // CodeBlob but you'd be wrong because the adapter frame can be variable sized.
  // So we have to calculate its size here./ So rather than this:
  //
  // if (array->old_adapter() != NULL)
  //   adapter_size = array->old_adapter()->frame_size() * BytesPerWord;
  //
  // We have:
  //
  if (array->old_adapter() != NULL) 
    adapter_size = (array->adapter_caller().sp() - (array->sp() + array->frame_size())) * BytesPerWord;

#ifdef COMPILER1
  if (callee_locals > callee_parameters) {
    // The caller frame may need extending to accommodate non-parameter locals of the first unpacked interpreted
    // frame.  Compute that adjustment.
    caller_adjustment = array->extend_caller_frame(callee_locals, callee_parameters);
  }
#endif
#ifdef COMPILER2

  // Compute the size that the first interpreter frame will have to adjust on the callers frame
  // The caller could be a c2i we create here or if we are popping an i2c then the
  // caller is an interpreter frame.
  caller_adjustment = last_frame_adjust(callee_parameters, callee_locals);

  if (new_adapter != NULL) {
    assert(array->adapter_caller().sp() == NULL, "should not be set");
    // Compute the size of a new C2I adapter frame that will preceed the unrolled frames.
    frame_sizes[0] = new_adapter->frame_size() * BytesPerWord;
    // The first interpreter frame returns to the adapter not to what we just set in place
    frame_pcs[1] = new_adapter->return_from_interpreter() - frame::pc_return_offset;
  } else {
    // We're removing an old adapter
    assert(array->adapter_caller().sp() != NULL, "must have caller of adapter to be popped");
    assert(array->old_adapter() != NULL, "must have an old adapter to be popped");
  }
#endif
  // Find the current pc for sender of the deoptee. Since the sender may have been deoptimized
  // itself since the deoptee vframeArray was created we must get a fresh value of the pc rather
  // than simply use array->sender.pc(). This requires us to walk the current set of frames
  //
  frame deopt_sender = stub_frame.sender(&map); // First is the deoptee frame
  deopt_sender = deopt_sender.sender(&map);     // Now deoptee caller
  // Is deoptee caller and adapter?
  if (array->old_adapter() != NULL)
    deopt_sender = deopt_sender.sender(&map);

  // Hate this subtraction
  frame_pcs[0] = deopt_sender.pc() - frame::pc_return_offset;

  assert(CodeCache::find_blob_unsafe(frame_pcs[0]) != NULL, "bad pc");

  UnrollBlock* info = new UnrollBlock(array->frame_size() * BytesPerWord, 
				      adapter_size,
				      need_new_adapter,
				      caller_adjustment * BytesPerWord,
                                      number_of_frames,
				      frame_sizes,
				      frame_pcs,
				      return_type);
#ifdef COMPILER2
  // We need a way to pass fp to the unpacking code so the skeletal frames
  // come out correct. This is only needed for x86 because of c2 using ebp
  // as an allocatable register. So this update is useless (and harmless)
  // on the other platforms. It would be nice to do this in a different
  // way but even the old style deoptimization had a problem with deriving
  // this value. NEEDS_CLEANUP
  intptr_t** fp_addr = (intptr_t**) (((address)info) + info->initial_fp_offset_in_bytes());
  *fp_addr = array->sender().fp(); // was adapter_caller
#endif

  array->set_unroll_block(info);
  return info;
}

// Return BasicType of value being returned
JRT_LEAF(BasicType, Deoptimization::unpack_frames(JavaThread* thread, int exec_mode))
  ResourceMark rm;

  // It is actually ok to allocate handles in a leaf method. It causes no safepoints,
  // but makes the entry a little slower. There is however a little dance we have to
  // do in debug mode to get around the NoHandleMark code in the JRT_LEAF macro
  ResetNoHandleMark rnhm; // No-op in release/product versions
  HandleMark hm;

  frame stub_frame = thread->last_frame();

  // Since the frame to unpack is the top frame of this thread, the vframe_array_head
  // must point to the vframeArray for the unpack frame.
  vframeArray* array = thread->vframe_array_head();

#ifndef PRODUCT
  if (TraceDeoptimization) {
    tty->print_cr("DEOPT UNPACKING thread 0x%lx vframeArray 0x%lx", thread, array);
  }
#endif

  UnrollBlock* info = array->unroll_block();

  // Remove the vframeArray from its thread
  // It is crucial that this happens before we unpack the new frames
  // so that the frame::sender... code can recognize the deopt frame
  // as the stub and original deopt frame and walk the stack appropriately.
  // It would be much better if the sender code could do this distinguishment
  // without this by instead examing the pc. NEEDS_CLEANUP
  //
  thread->set_vframe_array_head(array->next());

  // Unpack the interpreter frames and any adapter frame (c2 only) we might create.
  array->unpack_to_stack(stub_frame, exec_mode);


  BasicType bt = info->return_type();

  // If we have an exception pending, claim that the return type is an oop 
  // so the deopt_blob does not overwrite the exception_oop. 

  if (exec_mode == Unpack_exception)
    bt = T_OBJECT;


  // Free the previous UnrollBlock
  vframeArray* old_array = thread->vframe_array_last();
  thread->set_vframe_array_last(array);

  if (old_array != NULL) {
    UnrollBlock* old_info = old_array->unroll_block();
    old_array->set_unroll_block(NULL);
    delete old_info;
    delete old_array;
  }

#ifdef HOTSWAP
  if (jvmdi::enabled() && FullSpeedJVMDI) {
    // Regardless of whether we entered this routine with the pending
    // popframe condition bit set, we should always clear it now
    thread->clear_popframe_condition();
  }
#endif HOTSWAP
  return bt;
JRT_END


int Deoptimization::deoptimize_dependents() {
  Threads::deoptimized_wrt_marked_nmethods();
  return 0;
}

vframeArray* Deoptimization::create_vframeArray(JavaThread* thread, frame fr, RegisterMap *reg_map) {
  // Create a growable array of VFrames where each VFrame represents an inlined
  // Java frame.  This storage is allocated with the usual system arena.
  assert(fr.is_compiled_frame(), "Wrong frame type");
  GrowableArray<compiledVFrame*>* chunk = new GrowableArray<compiledVFrame*>(10);  
  vframe* vf = vframe::new_vframe(&fr, reg_map, thread);        
  while (!vf->is_top()) {
    assert(vf->is_compiled_frame(), "Wrong frame type");
    chunk->push(compiledVFrame::cast(vf));
    vf = vf->sender();
  }
  assert(vf->is_compiled_frame(), "Wrong frame type");
  chunk->push(compiledVFrame::cast(vf));

#ifndef PRODUCT
  if (TraceDeoptimization) {
    ttyLocker ttyl;
    tty->print("DEOPT PACKING thread 0x%lx ", thread);
    fr.print_on(tty);
    tty->print_cr("     Virtual frames (innermost first):");
    for (int index = 0; index < chunk->length(); index++) {
      compiledVFrame* vf = chunk->at(index);
      tty->print("       %2d - ", index);
      vf->print_value();
      int bci = chunk->at(index)->raw_bci();
      const char* code_name;
      if (bci == SynchronizationEntryBCI) {
        code_name = "sync entry";
      } else {
        Bytecodes::Code code = Bytecodes::code_at(vf->method(), bci);
        code_name = Bytecodes::name(code);
      }
      tty->print(" - %s", code_name);
      tty->print_cr(" @ bci %d ", bci);
      if (Verbose) {
        vf->print();
        tty->cr();
      }
    }
  }
#endif

  // Register map for next frame (used for stack crawl).  We capture
  // the state of the deopt'ing frame's caller.  Thus if we need to
  // stuff a C2I adapter we can properly fill in the callee-save
  // register locations.
  frame caller = fr.sender(reg_map);
  int frame_size = caller.sp() - fr.sp();

  CodeBlob* old_adapter = NULL;

  // If the caller of the deoptimized frame is an I2C or OSR adapter, it will be removed to achieve an
  // I2I transition.  Otherwise, the caller must be a compiled frame and for Compiler2, a C2I adapter
  // will be created when the vframeArray is allocated below.
  frame adapter_caller;  // treated as null unless its sp() gets set
  frame sender = caller;
#ifdef COMPILER1
  if (caller.is_osr_adapter_frame() ||
      caller.pc() == Runtime1::entry_for(Runtime1::alignment_frame_return_id)) {
    old_adapter = CodeCache::find_blob(caller.pc());
    assert(old_adapter, "Must have adapter to remove");
    adapter_caller = caller.sender(reg_map);
    sender = adapter_caller;
  }
#endif
#ifdef COMPILER2
  CodeBlob *cb = CodeCache::find_blob(caller.pc());
  if (cb && (cb->is_osr_adapter() || cb->is_i2c_adapter())) {
    // Capture the I2C frame; this holds info we need to populate the last
    // interpreted frame with for when it unwinds.  This kills callee-save
    // values for the caller BUT the caller's caller was the interpreter
    // and he has no callee-save values.
    old_adapter = cb;
    adapter_caller = caller.sender(reg_map);
    sender = adapter_caller;
    NOT_PRODUCT(OptoRuntime::_compress_i2c2i_ctr++;)
  }
#endif
 
  // Since the Java thread being deoptimized will eventually adjust it's own stack,
  // the vframeArray containing the unpacking information is allocated in the C heap.
  // For Compiler1, the caller of the deoptimized frame is saved for use by unpack_frames().
  vframeArray* array = vframeArray::allocate(thread, frame_size, chunk, reg_map, sender, caller, fr, adapter_caller);
  array->set_old_adapter(old_adapter);
#ifdef COMPILER2
  assert(fr.unextended_sp() == fr.sp(), "callee must be C2I adapter; does not change caller's SP");
  array->set_adjust_adapter_caller(cb && cb->is_i2c_adapter());
#endif

  // Compare the vframeArray to the collected vframes
  assert(array->structural_compare(thread, chunk), "just checking");
  Events::log("# vframes = %d", chunk->length());

#ifndef PRODUCT
  if (TraceDeoptimization) {
    ttyLocker ttyl;
    tty->print_cr("     Created vframeArray 0x%lx", array);
    if (Verbose) {
      int count = 0;
      for (deoptimizedVFrame* vf = new deoptimizedVFrame(array); vf; vf = vf->deoptimized_sender_or_null()) {
        vf->print_activation(count++);
      }
    }
  }
#endif // PRODUCT

  return array;
}


void Deoptimization::deoptimize(JavaThread* thread, frame fr, frame callee_fr, RegisterMap *reg_map) {
  ResourceMark rm;
  assert(fr.can_be_deoptimized(), "checking frame type");

  NOT_PRODUCT(_nof_deoptimizations++); // For statistics
  EventMark m("Deoptimization (pc=0x%lx, sp=0x%lx)", fr.pc(), fr.id());

  // Gather information from the compiled frame so that its execution can be resumed by the interpreter.
  vframeArray* array = create_vframeArray(thread, fr, reg_map);
  thread->insert_vframe_array(array);

  // Make the callee of the deoptimized frame return to the runtime routine that will
  // use the vframeArray to transform the compiled frame into interpreted frames.
  COMPILER1_ONLY(fr.patch_for_deoptimization(thread, callee_fr, Runtime1::entry_for(Runtime1::deoptimization_handler_id));)
  COMPILER2_ONLY(fr.patch_pc(thread, OptoRuntime::deoptimize_repack_stack());)
  if (SafepointSynchronize::needs_deopt_suspend(thread)) {
    MutexLockerEx ml(thread->SR_lock(), Mutex::_no_safepoint_check_flag);
    thread->set_deopt_suspended();
  }
}


void Deoptimization::deoptimize_frame(JavaThread* thread, intptr_t* id) {
  // Compute frame and register map based on thread and sp.
  RegisterMap reg_map(thread);
  frame fr = thread->last_frame();
  frame callee_fr = fr;
  while (fr.id() != id) {
    fr = fr.sender(&reg_map);
    callee_fr = fr;
  }

  // Deoptimize only if the frame comes from compile code
  if (fr.is_compiled_frame()) {        
    ResourceMark rm;
    DeoptimizationMarker dm;    
    deoptimize(thread, fr, callee_fr, &reg_map);
  }
}


#ifdef HOTSWAP
JRT_LEAF(void, Deoptimization::popframe_preserve_args(JavaThread* thread, int bytes_to_save, void* start_address))
{
  thread->popframe_preserve_args(in_ByteSize(bytes_to_save), start_address);
}
JRT_END

#endif HOTSWAP


#ifdef COMPILER2
void Deoptimization::load_class_by_index(constantPoolHandle constant_pool, int index, TRAPS) {
  // in case of an unresolved klass entry, load the class.
  if (constant_pool->tag_at(index).is_unresolved_klass()) {
    klassOop tk = constant_pool->klass_at(index, CHECK);
    return;
  }

  if (!constant_pool->tag_at(index).is_symbol()) return;

  Handle class_loader (THREAD, instanceKlass::cast(constant_pool->pool_holder())->class_loader());
  symbolHandle symbol (THREAD, constant_pool->symbol_at(index));

  // class name?
  if (symbol->byte_at(0) != '(') {
    Handle protection_domain (THREAD, Klass::cast(constant_pool->pool_holder())->protection_domain());
    SystemDictionary::resolve_or_null(symbol, class_loader, protection_domain, CHECK);
    return;
  }

  // then it must be a signature!
  for (SignatureStream ss(symbol); !ss.is_done(); ss.next()) {
    if (ss.is_object()) {
      symbolOop s = ss.as_symbol(CHECK);
      symbolHandle class_name (THREAD, s);
      Handle protection_domain (THREAD, Klass::cast(constant_pool->pool_holder())->protection_domain());
      SystemDictionary::resolve_or_null(class_name, class_loader, protection_domain, CHECK);
    }
  }
}


void Deoptimization::load_class_by_index(constantPoolHandle constant_pool, int index) {
  EXCEPTION_MARK;
  load_class_by_index(constant_pool, index, THREAD);
  if (PENDING_EXCEPTION) {
    // Exception happened during classloading. We ignore the exception here, since it
    // is going to be rethrown since the current activation is going to be deoptimzied and
    // the interpreter will re-execute the bytecode. 
    CLEAR_PENDING_EXCEPTION;
  }
}

#ifndef PRODUCT
static int uc_count=0;
static int uc_count_stop = 0;
#endif

JRT_ENTRY(Deoptimization::UnrollBlock*, Deoptimization::uncommon_trap(JavaThread* thread, jint unloaded_class_index))
  ResourceMark rm;
  HandleMark hm;
  
  RegisterMap reg_map(thread, false); // Do not need to update callee-saved registers
  frame stub_frame = thread->last_frame();
  frame fr = stub_frame.sender(&reg_map);

  // Make sure the calling nmethod is not getting deoptimized and removed 
  // before we are done with it.
  nmethodLocker nl(fr.pc());

  Events::log("Uncommon trap occurred @" INTPTR_FORMAT " unloaded_class_index = %d", fr.pc(), unloaded_class_index);
  gather_uncommon_trap_statistics(unloaded_class_index);
  
  vframe * vf  = vframe::new_vframe(&fr, &reg_map, thread);
  nmethod* nm = compiledVFrame::cast(vf)->code();

  if (TraceDeoptimization || LogCompilation) {
    ResourceMark rm;
    ttyLocker ttyl;
    if (xtty != NULL) {
      const char* trap_kind = deopt_info_name(unloaded_class_index > 0 ? 0 : unloaded_class_index);
      xtty->begin_head("uncommon_trap thread='%d' kind='%s'",
                       (int) os::current_thread_id(), trap_kind);
      if (unloaded_class_index > 0)  xtty->print(" index='%d'", unloaded_class_index);
      xtty->print(" compile_id='%d'", nm->compile_id());
      const char* nm_kind = nm->compile_kind();
      if (nm_kind != NULL)  xtty->print(" compile_kind='%s'", nm_kind);
    }
    symbolHandle class_name;
    bool unresolved = false;
    if (unloaded_class_index >=0) {
      constantPoolHandle constants (THREAD, compiledVFrame::cast(vf)->method()->constants());
      if (constants->tag_at(unloaded_class_index).is_unresolved_klass()) {
        class_name = constants->klass_name_at(unloaded_class_index);
        unresolved = true;
        if (xtty != NULL)
          xtty->print(" unresolved='1'");
      } else if (constants->tag_at(unloaded_class_index).is_symbol()) {
        class_name = constants->symbol_at(unloaded_class_index);
      }
      if (xtty != NULL)
        xtty->name(class_name);
    }
    if (xtty != NULL) {
      xtty->stamp();
      xtty->end_head();
    }
    if (TraceDeoptimization) {  // make noise on the tty
      tty->print("Uncommon trap (%d) occurred in", NOT_PRODUCT(++uc_count)+0);
      nm->method()->print_short_name(tty);
      tty->print(" (@0x%lx) unloaded_class_index = %d, thread = %d", fr.pc(), unloaded_class_index, Thread::current()->osthread()->thread_id());
      if (class_name.not_null()) {
        tty->print(unresolved ? " unresolved class: " : " symbol: ");
        class_name->print_symbol_on(tty);
      }
      tty->cr();
    }
    if (xtty != NULL) {
      // Log the precise location of the trap.
      for (ScopeDesc* sd = compiledVFrame::cast(vf)->scope(); ; sd = sd->sender()) {
        xtty->begin_elem("jvms bci='%d'", sd->bci());
        xtty->method(sd->method());
        xtty->end_elem();
        if (sd->is_top())  break;
      }
      xtty->tail("uncommon_trap");
    }
  }
#ifndef PRODUCT
  if ((uc_count_stop != 0) && (uc_count == uc_count_stop)) os::breakpoint();
#endif // PRODUCT



  if (BailoutToInterpreterForThrows && unloaded_class_index == Deopt_athrow) {
    // Keep method interpreted from now on
    nm->make_not_entrant();
    nm->method()->set_not_compilable();    
  }

  constantPoolHandle constants;
  if (unloaded_class_index >= Deoptimization::Deopt_unloaded) {
    constants = constantPoolHandle(THREAD, compiledVFrame::cast(vf)->method()->constants());

    // Reset invocation counter for outer most method. 
    ScopeDesc* sd = compiledVFrame::cast(vf)->scope();
    while (!sd->is_top()) sd = sd->sender();
    InvocationCounter *c = sd->method()->invocation_counter();
    if( nm->is_osr_method() ) {
      c->set(c->state(), CompileThreshold);
    } else {
      c->reset();
    }
  }

  // Load class is necessary 
  if (constants.not_null()) {
    load_class_by_index(constants, unloaded_class_index);
  }

  VM_DeoptimizeFrame deopt(thread, fr.id());
  VMThread::execute(&deopt);
  // An exception might have been installed at this point, e.g., ThreadDeath

  { // %%% Temporary Kludge.
    if (unloaded_class_index == Deoptimization::Deopt_unexpected_klass) {
      if (++_unexpected_klass_traps >= TypeProfileFailureLimit)
        _unexpected_klass_traps_enabled = false;
    }
  }

  // Make them not entrant, so next time they are called they get
  // recompiled.  Unloaded classes are loaded now so recompile before next
  // time they are called.  Same for uninitialized.  The interpreter will
  // load the array class.
  if( unloaded_class_index == Deoptimization::Deopt_uninitialized ||
      unloaded_class_index == Deoptimization::Deopt_unloaded_array_class ||
      unloaded_class_index == Deoptimization::Deopt_unexpected_klass ||
      unloaded_class_index == Deoptimization::Deopt_tier1_counter ||
      unloaded_class_index >= Deoptimization::Deopt_unloaded ) {
    nm->make_not_entrant();
#ifndef PRODUCT
    if (TraceDeoptimization) {
      tty->print_cr(" - turned (nmethod*)0x%lx into a non_entrant nmethod",nm);
    }
#endif
  }

  // If we see an unexpected null at a check-cast we record it and force a
  // recompile; the offending check-cast will be compiled to handle NULLs.
  // If we see more than one offending BCI, then all checkcasts in the
  // method will be compiled to handle NULLs.
  if( unloaded_class_index == Deoptimization::Deopt_unexpected_null_cast ||
      // We might deopt for a range check 'speculatively', if we've
      // widened some check in the method.  Otherwise, we might come here
      // if the method in question is actually throwing range check
      // exceptions frequently.
      unloaded_class_index == Deoptimization::Deopt_range_check ||
      // We come here we tried to cast an oop array to it's declared
      // type and that cast failed.
      unloaded_class_index == Deoptimization::Deopt_array_store_check ||
      // Math.pow intrinsic returned a NaN, which requires StrictMath.pow 
      // to handle.  Recompile without intrinsifying Math.pow.
      unloaded_class_index == Deoptimization::Deopt_math_pow_check ) {

    ScopeDesc* sd = compiledVFrame::cast(vf)->scope();
    int fail_bci = sd->bci();
    methodHandle m = sd->method();
    bool make_not_entrant = true;
#ifdef IA64
    // 4852688: ProfileInterpreter is off by default for ia64.  The result is
    // infinite heroic-opt-uncommon-trap/deopt/recompile cycles, since the
    // recompile relies on a methodDataOop to record heroic opt failures.
    // So on ia64 we always build a dummy mdo when deopting due to a heroic
    // opt failure.  If we ever support ProfileInterpreter for ia64, this
    // hack can go away.
#else
    if (ProfileInterpreter)
#endif
    {
      methodDataOop mdo = m()->method_data();
      if (mdo == NULL && !HAS_PENDING_EXCEPTION) {
        // Build an MDO.  Ignore errors like OutOfMemory;
        // that simply means we won't have an MDO to update.
        methodOopDesc::build_interpreter_method_data(m, THREAD);
        if (HAS_PENDING_EXCEPTION) {
          assert((PENDING_EXCEPTION->is_a(SystemDictionary::OutOfMemoryError_klass())), "we expect only an OOM error here");
          CLEAR_PENDING_EXCEPTION;
        }
        mdo = m()->method_data();
      }
      if (mdo != NULL) {          // Have an MDO to update?
        // Check-cast & array-store bytecodes have a BitData structure in the MDO
        if( unloaded_class_index == Deoptimization::Deopt_unexpected_null_cast ) {
  	  BitData* bd = mdo->bci_to_data(fail_bci)->as_BitData();
  	  if (bd != NULL) {
  	    bd->set_null_seen();
  	  } else {
  	    make_not_entrant = false;
  	  }
        } else if( unloaded_class_index == Deoptimization::Deopt_array_store_check ) {
  	  BitData* bd = mdo->bci_to_data(fail_bci)->as_BitData();
  	  if (bd != NULL) {
  	    bd->set_heroic_opt_failure_seen();
  	  } else {
  	    make_not_entrant = false;
  	  }
          mdo->set_array_check_deopt();
        } else if( unloaded_class_index == Deoptimization::Deopt_math_pow_check ) {
  	  CounterData* cd = mdo->bci_to_data(fail_bci)->as_CounterData();
  	  if (cd != NULL) {
  	    cd->set_math_pow_failure_seen();
  	  } else {
  	    make_not_entrant = false;
  	  }
        } else {                  // Else it's a range check deopt
          // Set the seen-range-check-deopt (which doubles as the disable-widening flag)
          assert( unloaded_class_index == Deoptimization::Deopt_range_check, "" );
          // If we have already recompiled with the range-check-widening
          // heroic optimization turned off, then we must really be throwing
          // range check exceptions.  Do not force a recompile in that case,
          // just suffer along in the interpreter.
          if( mdo->get_range_check_deopt() )// Already disabled heroic opt?
            make_not_entrant = false; // Then do not force a recompile
          else                  // Else set the disable-heroic-opt and
            mdo->set_range_check_deopt(); // force a recompile
        }
      }
    }
    // Recompile
    if( make_not_entrant ) nm->make_not_entrant();
  }

  return fetch_unroll_info_helper(thread);
JRT_END
#endif // COMPILER2


//-----------------------------------------------------------------------------

const char* Deoptimization::deopt_info_name(int deopt_info) {
  if (deopt_info > Deopt_unloaded)  return NULL;
  switch (deopt_info) {
  case Deopt_unloaded:              return "unloaded";
  case Deopt_athrow:                return "athrow";
  case Deopt_null_check:            return "null_check";
  case Deopt_div0_check:            return "div0_check";
  case Deopt_range_check:           return "range_check";
  case Deopt_cast_check:            return "cast_check";
  case Deopt_array_store_check:     return "array_store_check";
  case Deopt_uninitialized:         return "uninitialized";
  case Deopt_unloaded_array_class:  return "unloaded_array_class";
  case Deopt_unexpected_klass:      return "unexpected_klass";
  case Deopt_unexpected_null_cast:  return "unexpected_null_cast";
  case Deopt_unhandled_bytecode:    return "unhandled_bytecode";
  case Deopt_math_pow_check:        return "math_pow_check";
  case Deopt_tier1_counter:         return "tier1_counter";
  }
  ShouldNotReachHere();
  return "?";
}

int  Deoptimization::_unexpected_klass_traps = 0;
bool Deoptimization::_unexpected_klass_traps_enabled = true;

#ifndef PRODUCT

long Deoptimization::_nof_deoptimizations        = 0;
long Deoptimization::_nof_unloaded               = 0;
long Deoptimization::_nof_athrow                 = 0;
long Deoptimization::_nof_null_check             = 0;
long Deoptimization::_nof_div0_check             = 0;
long Deoptimization::_nof_negative_array_length  = 0;
long Deoptimization::_nof_range_check            = 0;
long Deoptimization::_nof_cast_check             = 0;
long Deoptimization::_nof_array_store_check      = 0;
long Deoptimization::_nof_uninitialized          = 0;
long Deoptimization::_nof_unloaded_array_class   = 0;
long Deoptimization::_nof_unexpected_klass       = 0;
long Deoptimization::_nof_unexpected_null_cast   = 0;
long Deoptimization::_nof_unhandled_bytecode     = 0;
long Deoptimization::_nof_math_pow_check         = 0;
long Deoptimization::_nof_tier1_counter          = 0;


void Deoptimization::gather_uncommon_trap_statistics(int unloaded_class_index) {
  switch(unloaded_class_index) {
     case Deopt_athrow:                 ++_nof_athrow;                break;
     case Deopt_null_check:             ++_nof_null_check;            break;
     case Deopt_div0_check:             ++_nof_div0_check;            break;
     case Deopt_range_check:            ++_nof_range_check;           break;
     case Deopt_cast_check:             ++_nof_cast_check;            break;
     case Deopt_array_store_check:      ++_nof_array_store_check;     break;
     case Deopt_uninitialized:          ++_nof_uninitialized;         break;
     case Deopt_unloaded_array_class:   ++_nof_unloaded_array_class;  break;
     case Deopt_unexpected_klass:       ++_nof_unexpected_klass;      break;
     case Deopt_unexpected_null_cast:   ++_nof_unexpected_null_cast;  break;
     case Deopt_unhandled_bytecode:     ++_nof_unhandled_bytecode;    break;
     case Deopt_math_pow_check:         ++_nof_math_pow_check;        break;
     case Deopt_tier1_counter:          ++_nof_tier1_counter;         break;
     default:
       assert(unloaded_class_index >= 0, "wrong index");
       _nof_unloaded++;
       break;
  }
}

  
int Deoptimization::print_stat_line(const char *name, long r) { 
  assert(_nof_deoptimizations != 0, "sanity check");
  if (r != 0) {
    tty->print_cr("  %s: %Ld (%d%%)", name, r, (r * 100) / _nof_deoptimizations);
  }
  return 0;
}


void Deoptimization::print_statistics() {
  if (_nof_deoptimizations != 0) {
    ttyLocker ttyl;
    tty->print_cr("Total number of deoptimizations: %Ld", _nof_deoptimizations);
    print_stat_line("unloaded classes"       , _nof_unloaded);
    print_stat_line("athrow"                 , _nof_athrow);
    print_stat_line("null check"             , _nof_null_check);
    print_stat_line("div0 check"             , _nof_div0_check);
    print_stat_line("array length"           , _nof_negative_array_length);
    print_stat_line("range_check"            , _nof_range_check);
    print_stat_line("checkcast"              , _nof_cast_check);
    print_stat_line("array store"            , _nof_array_store_check);
    print_stat_line("uninitialized"          , _nof_uninitialized);
    print_stat_line("unloaded array classes" , _nof_unloaded_array_class);
    print_stat_line("unexpected classes"     , _nof_unexpected_klass);
    print_stat_line("unexpected null cast"   , _nof_unexpected_null_cast);
    print_stat_line("unhandled bytecode"     , _nof_unhandled_bytecode);
    print_stat_line("math pow check"         , _nof_math_pow_check);
    print_stat_line("tier1 counter"          , _nof_tier1_counter);
  }
}
#endif // !PRODUCT

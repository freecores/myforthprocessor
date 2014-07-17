#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)c1_Runtime1_sparc.cpp	1.103 03/01/23 11:00:19 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_c1_Runtime1_sparc.cpp.incl"
#include <v9/sys/psr_compat.h>


// Implementation of StubAssembler

int StubAssembler::call_RT(Register oop_result1, Register oop_result2, address entry_point, int number_of_arguments) {
  assert_not_delayed();
  // bang stack before going to runtime
  set(-os::vm_page_size() + STACK_BIAS, G3_scratch);
  st(G0, SP, G3_scratch);

  // debugging support
  assert(number_of_arguments >= 0   , "cannot have negative number of arguments");

  set_last_Java_frame(SP, G0);
  if (VerifyThread)  mov(G2_thread, O0); // about to be smashed; pass early
  save_thread(L7_thread_cache);
  // do the call
  call(entry_point, relocInfo::runtime_call_type);
  if (!VerifyThread) {
    delayed()->mov(G2_thread, O0);  // pass thread as first argument
  } else {
    delayed()->nop();             // (thread already passed)
  }
  int call_offset = offset();  // offset of return address
  restore_thread(L7_thread_cache);
  reset_last_Java_frame();

  // check for pending exceptions
  { Label L;
    Address exception_addr(G2_thread, 0, in_bytes(Thread::pending_exception_offset()));
    ld_ptr(exception_addr, Gtemp);
    br_null(Gtemp, false, pt, L);
    delayed()->nop();
    Address vm_result_addr(G2_thread, 0, in_bytes(JavaThread::vm_result_offset()));
    st_ptr(G0, vm_result_addr);
    Address vm_result_addr_2(G2_thread, 0, in_bytes(JavaThread::vm_result_2_offset()));
    st_ptr(G0, vm_result_addr_2);

    // we use O7 linkage so that forward_exception_entry has the issuing PC
    call(StubRoutines::forward_exception_entry(), relocInfo::runtime_call_type);
    delayed()->restore();
    bind(L);
  }

  // get oop result if there is one and reset the value in the thread
  if (oop_result1->is_valid()) {                    // get oop result if there is one and reset it in the thread
    get_vm_result  (oop_result1);
  } else {
    // be a little paranoid and clear the result
    Address vm_result_addr(G2_thread, 0, in_bytes(JavaThread::vm_result_offset()));
    st_ptr(G0, vm_result_addr);
  }

  if (oop_result2->is_valid()) {
    get_vm_result_2(oop_result2);
  } else {
    // be a little paranoid and clear the result
    Address vm_result_addr_2(G2_thread, 0, in_bytes(JavaThread::vm_result_2_offset()));
    st_ptr(G0, vm_result_addr_2);
  }

  return call_offset;
}


int StubAssembler::call_RT(Register oop_result1, Register oop_result2, address entry, Register arg1) {
  // O0 is reserved for the thread
  mov(arg1, O1);
  return call_RT(oop_result1, oop_result2, entry, 1);
}


int StubAssembler::call_RT(Register oop_result1, Register oop_result2, address entry, Register arg1, Register arg2) {
  // O0 is reserved for the thread
  mov(arg1, O1);
  mov(arg2, O2); assert(arg2 != O1, "smashed argument");
  return call_RT(oop_result1, oop_result2, entry, 2);
}


int StubAssembler::call_RT(Register oop_result1, Register oop_result2, address entry, Register arg1, Register arg2, Register arg3) {
  // O0 is reserved for the thread
  mov(arg1, O1);
  mov(arg2, O2); assert(arg2 != O1,               "smashed argument");
  mov(arg3, O3); assert(arg3 != O1 && arg3 != O2, "smashed argument");
  return call_RT(oop_result1, oop_result2, entry, 3);
}

// Implementation of StubFrame

class StubFrame: public StackObj {
 private:
  StubAssembler* _sasm;
  int            _args_size;

 public:
  StubFrame(StubAssembler* sasm, const char* name, bool must_gc_arguments);

  ~StubFrame();
};


#define __ _sasm->
StubFrame::StubFrame(StubAssembler* sasm, const char* name, bool must_gc_arguments) {
  _sasm = sasm;
  _args_size = 0;
  __ set_info(name, must_gc_arguments);
  __ save_frame(0);
}

StubFrame::~StubFrame() {
  __ ret();
  __ delayed()->restore();
}
#undef __


// Implementation of Runtime1

#define __ sasm->

static int cpu_reg_save_offsets[FrameMap::nof_cpu_regs];
static int fpu_reg_save_offsets[FrameMap::nof_fpu_regs];
static int reg_save_size;
static OopMap* save_caller_save_registers(StubAssembler* sasm, int frame_size);
static void restore_caller_save_registers(StubAssembler* sasm);


static OopMap* save_caller_save_registers(StubAssembler* sasm, int frame_size) {
  // Record volatile registers as callee-save values in an OopMap so their save locations will be
  // propagated to the caller frame's RegisterMap during StackFrameStream construction (needed for
  // deoptimization; see compiledVFrame::create_stack_value).  The caller's I, L and O registers
  // are saved in register windows - I's and L's in the caller's frame and O's in the stub frame
  // (as the stub's I's) when the runtime routine called by the stub creates its frame.
  int i;
  OopMap* oop_map = new OopMap(frame_size, 0);
  for (i = 0; i < FrameMap::nof_cpu_regs; i++) {
    Register r = as_Register(i);
    if (r->is_global() && r != G0) {
      int sp_offset = cpu_reg_save_offsets[i];
      __ st_ptr(r, SP, (sp_offset * BytesPerWord) + STACK_BIAS);
      oop_map->set_callee_saved(OptoReg::Name(SharedInfo::stack0 + sp_offset), frame_size, 0, FrameMap::cpu_regname(r->encoding()));
    }
  }

  for (i = 0; i < FrameMap::nof_fpu_regs; i++) {
    FloatRegister r = as_FloatRegister(i);
    int sp_offset = fpu_reg_save_offsets[i];
    __ stf(FloatRegisterImpl::S, r, SP, (sp_offset * BytesPerWord) + STACK_BIAS);
    oop_map->set_callee_saved(OptoReg::Name(SharedInfo::stack0 + sp_offset), frame_size, 0,
                              FrameMap::fpu_regname(r->encoding(FloatRegisterImpl::S)));
  }
  return oop_map;
}


static void restore_caller_save_registers(StubAssembler* sasm) {
  int i;

  for (i = 0; i < FrameMap::nof_cpu_regs; i++) {
    Register r = as_Register(i);
    if (r->is_global() && r != G0) {
      __ ld_ptr(SP, (cpu_reg_save_offsets[i] * BytesPerWord) + STACK_BIAS, r);
    }
  }

  for (i = 0; i < FrameMap::nof_fpu_regs; i++) {
    FloatRegister r = as_FloatRegister(i);
    __ ldf(FloatRegisterImpl::S, SP, (fpu_reg_save_offsets[i] * BytesPerWord) + STACK_BIAS, r);
  }
}


void Runtime1::initialize_pd() {
  // compute word offsets from SP at which caller-save registers are captured by stub routines
  int i;
  int sp_offset = round_to(frame::memory_parameter_word_sp_offset, 2); //  start doubleword aligned

  // only G int registers are saved explicitly; others are found in register windows
  for (i = 0; i < FrameMap::nof_cpu_regs; i++) {
    Register r = as_Register(i);
    if (r->is_global() && r != G0) {
      cpu_reg_save_offsets[i] = sp_offset;
      sp_offset++;
    }
  }

  // all float registers are saved explicitly
  assert(FrameMap::nof_fpu_regs == 32, "double registers not handled here");
  for (i = 0; i < FrameMap::nof_fpu_regs; i++) {
    fpu_reg_save_offsets[i] = sp_offset;
    sp_offset++;
  }
  reg_save_size = sp_offset - frame::memory_parameter_word_sp_offset;
}


OopMapSet* Runtime1::generate_exception_throw(StubAssembler* sasm, address target, Register arg1) {
  // make a frame and preserve the caller's caller-save registers
  int frame_size = __ total_frame_size_in_bytes(reg_save_size);
  __ save_frame_c1(frame_size);
  OopMap* oop_map = save_caller_save_registers(sasm, frame_size);

  int call_offset;
  if (arg1 == noreg) {
    call_offset = __ call_RT(noreg, noreg, target);
  } else {
    call_offset = __ call_RT(noreg, noreg, target, arg1);
  }
  OopMapSet* oop_maps = new OopMapSet();
  oop_maps->add_gc_map(call_offset, true, oop_map);

  __ should_not_reach_here();
  return oop_maps;
}


OopMapSet* Runtime1::generate_illegal_instruction_handler(StubAssembler* sasm, address target) {
  address start = __ pc();

  // make a frame and preserve the caller's caller-save registers
  int frame_size = __ total_frame_size_in_bytes(reg_save_size);
  __ save_frame_c1(frame_size);
  OopMap* oop_map = save_caller_save_registers(sasm, frame_size);

  // setup last_Java_sp
  __ set_last_Java_frame(SP, G0);

  // Cannot use call_VM_leaf, because a GC map is needed at this call site.
  NEEDS_CLEANUP // can just use call_RT(noreg, noreg, target) here?
  __ mov(G2_thread, O0);
  __ save_thread(L7_thread_cache);
  __ call(CAST_FROM_FN_PTR(address, SafepointSynchronize::handle_illegal_instruction_exception));
  __ delayed()->nop();
  int call_offset = __ pc() - start;
  OopMapSet* oop_maps = new OopMapSet();;
  oop_maps->add_gc_map(call_offset, true, oop_map);
  __ restore_thread(L7_thread_cache);

  // clear last_Java_sp
  __ reset_last_Java_frame();

  { // check for pending exceptions
    Label L;
    Address exception_addr(G2_thread, 0, in_bytes(Thread::pending_exception_offset()));
    __ ld_ptr(exception_addr, Gtemp);
    __ br_null(Gtemp, false, Assembler::pt, L);
    __ delayed()->nop();

    // restore saved state from thread local storage 
    // NOTE: floating point registers are not restored (no need, it's an exception)!
    __ restore();
    const Register temp_reg = G3_scratch;

    Address exc(temp_reg, StubRoutines::forward_exception_entry());
    __ jump_to(exc, 0);
    __ delayed()->nop();
    __ bind(L);
  }

  Label negativeL;
  __ tst(O0);
  __ brx(Assembler::negative, false, Assembler::pt, negativeL);
  __ delayed()->nop();

  restore_caller_save_registers(sasm);

  // resume nmethod execution at the point where this trap occurred in its ThreadCodeBuffer copy;
  // CompiledCodeSafepointHandler has already set the return address
  __ ret();
  __ delayed()->restore();

  // deoptimization happened at a compiled safepoint at a call; jump to destination
  // instead of skipping the call in the blocked method; we do not have debug information
  // to reexecute the call
  __ bind(negativeL);
  restore_caller_save_registers(sasm);
  __ restore();
  __ mov(O7, G3_scratch); // destination of the call
  DeoptimizationBlob* deopt_blob = (DeoptimizationBlob*) Runtime1::blob_for(Runtime1::deoptimization_handler_id);
  assert(deopt_blob != NULL, "deoptimization blob must have been created");
  __ load_address(Address(O7, deopt_blob->unpack() -  frame::pc_return_offset));
  // __ breakpoint_trap();
  __ jmpl(G3_scratch, frame::pc_return_offset, G0);
  __ delayed()->nop();

  return oop_maps;
}


OopMapSet* Runtime1::generate_patching(StubAssembler* sasm, address target) {
  // make a frame and preserve the caller's caller-save registers
  int frame_size = __ total_frame_size_in_bytes(reg_save_size);
  __ save_frame_c1(frame_size);
  OopMap* oop_map = save_caller_save_registers(sasm, frame_size);

  // have to make sure the stack copy of I7 has a valid value, otherwise the check
  // for deoptimization following the call can fail
  __ st_ptr(I7, SP, (I7->sp_offset_in_saved_window()*BytesPerWord) + STACK_BIAS);

  // call the runtime patching routine
  int call_offset = __ call_RT(noreg, noreg, target);
  OopMapSet* oop_maps = new OopMapSet();
  oop_maps->add_gc_map(call_offset, true, oop_map);

  // re-execute the patched instruction or, if the frame was deoptmized, return to the
  // deoptimization handler entry that will cause re-execution of the current bytecode
  DeoptimizationBlob* deopt_blob = (DeoptimizationBlob*) Runtime1::blob_for(Runtime1::deoptimization_handler_id);
  assert(deopt_blob != NULL, "deoptimization blob must have been created");
  Label no_deopt;
  // if the compiled frame was deoptimized, the return address of this frame
  // was changed to be the deoptimization handler's normal entry point
  __ ld_ptr(SP, (I7->sp_offset_in_saved_window()*BytesPerWord) + STACK_BIAS, L0);        
  __ add(L0, frame::pc_return_offset, L1);
  __ load_address(Address(L2, deopt_blob->unpack()));
  __ cmp(L1, L2);
  __ brx(Assembler::notEqual, false, Assembler::pt, no_deopt);
  __ delayed()->nop();
  // return to the deoptimization handler entry for unpacking
  __ load_address(Address(I7, deopt_blob->unpack_with_reexecution()));
  __ ret(0);
  __ delayed()->restore();

  __ bind(no_deopt);
  restore_caller_save_registers(sasm);
  __ ret();
  __ delayed()->restore();

  return oop_maps;
}


OopMapSet* Runtime1::generate_deoptimization_handler(StubAssembler* sasm, int* exception_offset, int* frame_size, int* reexecute_offset) {
  FloatRegister   Freturn0           = F0;
  Register        Oreturn0           = O0;
  Register        Oreturn1           = O1;
  Register        G3pcs              = G3_scratch;
  Register        OUnrollBlock       = O2;
  Register        Oframe_size        = O2;
  Register        Oadjust            = O3;
  Register        Oarray             = O3;
  Register        Oarray_size        = O4;
  Register        Oexception_tmp     = O5;
  Register        G1exception_tmp    = G1_scratch;
  int             unpack_frame_words = sizeof(double)/wordSize;
  Address         saved_Freturn0_addr(FP, 0, -sizeof(double) + STACK_BIAS);
  Label           cont, loop, adjust_caller_frame, done_adjusting_caller_frame;

  //
  // This is the entry point for code which is returning to a de-optimized
  // frame.
  // The steps taken by this frame are as follows:
  //   - push a dummy "unpack_frame" and save the return values (O0, O1, F0)
  //     which are coming in
  //   - call the C routine: Deoptimization::fetch_unroll_info (this function
  //     returns information about the number and size of interpreter frames
  //     which are equivalent to the frame which is being deoptimized)
  //   - deallocate the unpack frame
  //   - deallocate the deoptimization frame
  //   - in a loop using the information returned in the previous step
  //     push new interpreter frames (take care to propagate the return
  //     values through each new frame pushed)
  //   - create a dummy "unpack_frame" and save the return values (O0, O1, F0)
  //   - call the C routine: Deoptimization::unpack_frames (this function
  //     lays out values on the interpreter frame which was just created)
  //   - deallocate the dummy unpack_frame
  //   - ensure that all the return values are correctly set and then do
  //     a return to the interpreter entry point
  //
  // Refer to the following methods for more information:
  //   - Deoptimization::fetch_unroll_info
  //   - Deoptimization::unpack_frames

  *frame_size = __ total_frame_size_in_bytes(unpack_frame_words)/wordSize;
  address start = __ pc();

  // Add empty oopmap at begining. This can both be at_call and not_at_call,
  // Normally it would be at call. However, it could be at not_at_call, if the
  // thread happended to be blocked at safepoint_node in the frame that got
  // deoptimized.
  OopMapSet *oop_maps = new OopMapSet();
  oop_maps->add_gc_map( 0, true,  new OopMap(*frame_size, 0));
  oop_maps->add_gc_map( 0, false, new OopMap(*frame_size, 0));

  __ ba(false, cont);
  __ delayed()->mov(Deoptimization::Unpack_deopt, Oexception_tmp);
  *exception_offset = __ pc() - start;
  __ ba(false, cont);
  __ delayed()->mov(Deoptimization::Unpack_exception, Oexception_tmp);;
  *reexecute_offset = __ pc() - start;
  __ mov(Deoptimization::Unpack_reexecute, Oexception_tmp);

  __ bind(cont);
  // push a dummy "unpack_frame" taking care to preserve float return values
  // and call Deoptimization::fetch_unroll_info to get the UnrollBlock
  __ save_frame(unpack_frame_words);
  __ stf(FloatRegisterImpl::D, Freturn0, saved_Freturn0_addr);
  __ set_last_Java_frame(SP, G0);
  __ call_VM_leaf(L7_thread_cache, CAST_FROM_FN_PTR(address, Deoptimization::fetch_unroll_info), G2_thread);

  __ reset_last_Java_frame();
  __ mov(O0, OUnrollBlock->after_save());
  __ ldf(FloatRegisterImpl::D, saved_Freturn0_addr, Freturn0);
  __ restore();

  // deallocate the deoptimization frame taking care to preserve the return values
  __ mov(Oreturn0,     Oreturn0->after_save());
  __ mov(Oreturn1,     Oreturn1->after_save());
  __ mov(OUnrollBlock, OUnrollBlock->after_save());
  __ mov(Oexception_tmp, G1exception_tmp);
  __ restore();

  // If an OSR adapter frame is present, pop it.  Otherwise, extend the caller's frame
  // to make space for outermost interpreted frame's java locals.
  __ ld(Address(OUnrollBlock, 0, Deoptimization::UnrollBlock::adapter_size_offset_in_bytes()), Oadjust);
  __ tst(Oadjust);
  __ br(Assembler::zero, false, Assembler::pt, adjust_caller_frame);
  __ delayed()-> nop();
  __ mov(Oreturn0,     Oreturn0->after_save());
  __ mov(Oreturn1,     Oreturn1->after_save());
  __ mov(OUnrollBlock, OUnrollBlock->after_save());
  __ restore();                        // Actually pop I2C frame here

  // NOTE: this restore leaves us with the extended sp for the caller (i.e. accomodates
  // the extra locals). So we skip the adjustment below since it would be redundant.

  __ br(Assembler::always, false, Assembler::pt, done_adjusting_caller_frame);
  __ delayed()-> nop();

  __ bind(adjust_caller_frame);
  __ ld(Address(OUnrollBlock, 0, Deoptimization::UnrollBlock::caller_adjustment_offset_in_bytes()), Oadjust);

  const Register OsavedSP = O5; // turns into IsavedSP after save for C1
  __ mov( SP, OsavedSP );

  __ sub(SP, Oadjust, SP);
  __ bind(done_adjusting_caller_frame);

  // loop through the UnrollBlock info and create interpreter frames

  __ ld(Address(OUnrollBlock, 0, Deoptimization::UnrollBlock::number_of_frames_offset_in_bytes()), Oarray_size);
  __ ld_ptr(Address(OUnrollBlock, 0, Deoptimization::UnrollBlock::frame_sizes_offset_in_bytes()), Oarray);
  __ ld_ptr(Address(OUnrollBlock, 0, Deoptimization::UnrollBlock::frame_pcs_offset_in_bytes()), G3pcs);

  #ifdef ASSERT
  // make sure that there is at least one entry in the array
  __ tst(Oarray_size);
  __ breakpoint_trap(Assembler::zero);
  #endif

  // Push all the new interpreter frames in a loop

  __ bind(loop);

  __ ld_ptr(G3pcs, 0, O7);                      // load frame's new pc
  __ add(G3pcs, wordSize, G3pcs);
  __ ld_ptr(Oarray, 0, Oframe_size);
  __ sub(G0, Oframe_size, Oframe_size);
  __ save(SP, Oframe_size, SP);

  #ifdef ASSERT
  // make sure that the frames are aligned properly
  #ifndef _LP64
  __ btst(wordSize*2-1, SP);
  __ breakpoint_trap(Assembler::notZero);
  #endif
  #endif

  __ mov(Oarray_size->after_save(), Oarray_size);
  __ mov(Oreturn0->after_save(), Oreturn0);
  __ sub(Oarray_size, 1, Oarray_size);
  __ mov(Oreturn1->after_save(), Oreturn1);
  __ mov(Oarray->after_save(), Oarray);

  #ifdef ASSERT
  // trash registers to show a clear pattern in backtraces
  __ set(0xDEAD0000, I0);
  __ add(I0,  2, I1);
  __ add(I0,  4, I2);
  __ add(I0,  6, I3);
  __ add(I0,  8, I4);
  // Don't touch I5 as it is savedSP value on transition to interpreted.
  __ set(0xDEADBEEF, L0);
  __ mov(L0, L1);
  __ mov(L0, L2);
  __ mov(L0, L3);
  __ mov(L0, L4);
  __ mov(L0, L5);

  #endif
  __ mov( G0, OsavedSP );	// savedsp is only important for the code->interp transition frame 

  __ tst(Oarray_size);
  __ br(Assembler::notZero, false, Assembler::pn, loop);
  __ delayed()->add(Oarray, wordSize, Oarray);
  __ ld_ptr(G3pcs, 0, O7);                      // load final frame new pc

  // push a dummy "unpack_frame" taking care of float return values and
  // call Deoptimization::unpack_frames to have the unpacker layout
  // information in the interpreter frames just created and then return
  // to the interpreter entry point
  __ save_frame(unpack_frame_words);
  __ stf(FloatRegisterImpl::D, Freturn0, saved_Freturn0_addr);
  __ set_last_Java_frame(SP, G0);
  __ call_VM_leaf(L7_thread_cache, CAST_FROM_FN_PTR(address, Deoptimization::unpack_frames), G2_thread, G1exception_tmp);
  __ reset_last_Java_frame();
  __ ldf(FloatRegisterImpl::D, saved_Freturn0_addr, Freturn0);

  __ ret();
  __ delayed()->restore();     

  return oop_maps;
}


// Normally the result is returned in register "r" but if deoptimization happened,
// it must be returned to the deoptimization handler in O0 (O0 is TOS for interpreter).
void Runtime1::deoptimization_check_return(StubAssembler* sasm, Register r) {
  Label no_deopt;
  DeoptimizationBlob* deopt_blob = (DeoptimizationBlob*) Runtime1::blob_for(Runtime1::deoptimization_handler_id);
  assert(deopt_blob != NULL, "deoptimization blob must have been created");
  __ ld_ptr(SP, (I7->sp_offset_in_saved_window()*BytesPerWord) + STACK_BIAS, L0);        
  __ add(L0, frame::pc_return_offset, L1);
  __ load_address(Address(L2, deopt_blob->unpack()));
  __ cmp(L1, L2);
  __ brx(Assembler::notEqual, false, Assembler::pt, no_deopt);
  __ delayed()->nop();
  __ mov(r, I0);

  __ bind(no_deopt);
  __ ret();
  __ delayed()->restore();
}


OopMapSet* Runtime1::generate_code_for(StubID id, StubAssembler* sasm, int* unpack_exception_offset, int* frame_size, int* unpack_reexecute_offset) {

  OopMapSet* oop_maps = NULL;
  // for better readability
  const bool must_gc_arguments = true;
  const bool dont_gc_arguments = false;

  // stub code & info for the different stubs
  switch (id) {
    case new_instance_id:
      { StubFrame f(sasm, "new_instance", dont_gc_arguments);
        // G4: klassOop
        __ call_RT(I0, noreg, CAST_FROM_FN_PTR(address, new_instance), G4);
        // I0->O0: new instance
      }

      break;

    case new_type_array_id:
      { StubFrame f(sasm, "new_type_array", dont_gc_arguments);
        // have to make sure the stack copy of I7 has a valid value, otherwise the check
        // for deoptimization following the call can fail
        __ st_ptr(I7, SP, (I7->sp_offset_in_saved_window()*BytesPerWord) + STACK_BIAS);
        // G4: elt_type
        // G5: length   
        __ call_RT(G4, noreg, CAST_FROM_FN_PTR(address, new_type_array), G4, G5);
        // G4: new type array
        deoptimization_check_return(sasm, G4);
      }
      break;

    case new_object_array_id:
      { // G4: klass
        // G5: length
        __ set_info("new_object_array", dont_gc_arguments);
        __ save_frame(0);
        // have to make sure the stack copy of I7 has a valid value, otherwise the check
        // for deoptimization following the call can fail
        __ st_ptr(I7, SP, (I7->sp_offset_in_saved_window()*BytesPerWord) + STACK_BIAS);
        __ call_RT(G4, noreg, CAST_FROM_FN_PTR(address, new_object_array), G4, G5);
        // G4: new object array
        deoptimization_check_return(sasm, G4);
      }
      break;

    case new_multi_array_id:
      { // O0: klass
        // O1: rank
        // O2: address of 1st dimension
        __ set_info("new_multi_array", dont_gc_arguments);
        __ save_frame(0);
        __ call_RT(I0, noreg, CAST_FROM_FN_PTR(address, new_multi_array), I0, I1, I2);
        __ ret();
        __ delayed()->restore();
        // I0 -> O0: new multi array
      }
      break;

    case resolve_invokestatic_id:
      { 
        __ set_info("resolve_invokestatic", must_gc_arguments);
        __ save_frame(0);
        __ call_RT(G5_method, noreg, CAST_FROM_FN_PTR(address, resolve_invoke), G0); 
        __ jmp(O0, 0);
        __ delayed()->restore();
      }
      break;

    case resolve_invoke_id:
      {
        __ set_info("resolve_invoke", must_gc_arguments);
        __ save_frame(0);
        __ call_RT(G5_method, noreg, CAST_FROM_FN_PTR(address, resolve_invoke), I0); 
        __ jmp(O0, 0);
        __ delayed()->restore();
      }
      break;

    case handle_ic_miss_id:
      {
        __ set_info("handle_ic_miss", must_gc_arguments);
        // G5_inline_cache_reg : ic klass
        // O0 : receiver
        __ save_frame(0);
        __ call_RT(G5_method, I0, CAST_FROM_FN_PTR(address, handle_ic_miss), I0);
        // O0: interpreter or verified entry point
        // G5_method: callee method (always)
        // I0: receiver (becomes O0 upon restore)
        __ jmp(O0, 0);
        __ delayed()->restore();
      }
      break;

    case handle_wrong_static_method_id:
      {
        __ set_info("handle_wrong_static_method", must_gc_arguments);
        __ save_frame(0);
        __ call_RT(G5_method, noreg, CAST_FROM_FN_PTR(address, handle_wrong_method), G0);
        // O0: interpreter or verfied entry point
        // G5_method: callee method (always)
        __ jmp(O0, 0);
        __ delayed()->restore();
      }
      break;

    case handle_wrong_method_id:
      {
        __ set_info("handle_wrong_method", must_gc_arguments);
        __ save_frame(0);
        __ call_RT(G5_method, I0, CAST_FROM_FN_PTR(address, handle_wrong_method), I0);
        // O0: interpreter or verfied entry point
        // G5_method: callee method (always)
        // I0: receiver (becomes O0 upon restore)
        __ jmp(O0, 0);
        __ delayed()->restore();
      }
      break;

    case entering_non_entrant_id:
      { __ set_info("entering_non_entrant", must_gc_arguments);
        __ stop ("we entered a non entrant method: internal error");
      }
      break;

    case throw_abstract_method_error_id:
      { StubFrame f(sasm, "throw_abstract_method_exception", dont_gc_arguments); // arguments will be discarded
        __ call_RT(noreg, noreg, CAST_FROM_FN_PTR(address, throw_abstract_method_error));
        __ should_not_reach_here();
      }
      break;

    case range_check_failed_id:
      { __ set_info("range_check_failed", dont_gc_arguments); // arguments will be discarded
        // G4: index
        oop_maps = generate_exception_throw(sasm, CAST_FROM_FN_PTR(address, throw_range_check_exception), G4);
      }
      break;

    case throw_index_exception_id:
      { __ set_info("index_range_check_failed", dont_gc_arguments); // arguments will be discarded
        // G4: index
        oop_maps = generate_exception_throw(sasm, CAST_FROM_FN_PTR(address, throw_index_exception), G4);
      }
      break;

    case throw_div0_exception_id:
      { __ set_info("throw_div0_exception", dont_gc_arguments);
        oop_maps = generate_exception_throw(sasm, CAST_FROM_FN_PTR(address, throw_div0_exception));
      }
      break;

    case handle_exception_id:
      {
        Label no_handler;
        Label no_deopt;
        DeoptimizationBlob* deopt_blob = (DeoptimizationBlob*) Runtime1::blob_for(Runtime1::deoptimization_handler_id);
        assert(deopt_blob != NULL, "deoptimization blob must have been created");
        // O0: exception
        // O1: issuing pc
        __ set_info("handle_exception", dont_gc_arguments);
        __ save_frame(0);
        // have to make sure the stack copy of I7 has a valid value, otherwise the check
        // for deoptimization following the call can fail
        __ st_ptr(I7, SP, (I7->sp_offset_in_saved_window()*BytesPerWord) + STACK_BIAS);
        // save the real return address and use the throwing pc as the return address to lookup (has bci & oop map)
        __ mov(I7, L0);
        __ sub(Oissuing_pc->after_save(), frame::pc_return_offset, I7);
        __ call_RT(Oexception->after_save(), noreg, CAST_FROM_FN_PTR(address, exception_handler_for_pc),
                   Oexception->after_save(), Oissuing_pc->after_save());
        __ verify_oop(Oexception->after_save());
        __ mov(L0, I7); // restore return address

        // if the exception handling frame was deoptimized, the return address of this frame
        // has been changed to be the deoptimization handler; check for that situation
        __ ld_ptr(SP, (I7->sp_offset_in_saved_window()*BytesPerWord) + STACK_BIAS, L0);        
        __ add(L0, frame::pc_return_offset, L1);
        __ load_address(Address(L2, deopt_blob->unpack()));
        __ cmp(L1, L2);
        __ brx(Assembler::notEqual, false, Assembler::pt, no_deopt);
        __ delayed()->nop();
        // return to the deoptimization handler for unpacking; then the interpreter will handle the exception
        __ load_address(Address(I7, deopt_blob->unpack_with_exception()));
        __ ret(0);
        __ delayed()->restore();

        __ bind(no_deopt);
        __ tst(O0);
        __ br(Assembler::zero, false, Assembler::pn, no_handler);
        __ delayed()->nop();
        __ jmp(O0, 0);
        __ delayed()->restore();

        __ bind(no_handler);
        __ ret();
        __ delayed()->restore();
      }
      break;

    case unwind_exception_id:
      {
        // I0: exception
        // I7: address of call to this method

        __ set_info("unwind_exception", dont_gc_arguments); 
        __ add(I7, frame::pc_return_offset, Oissuing_pc->after_save());
        __ call_VM_leaf(L7_thread_cache, CAST_FROM_FN_PTR(address, SharedRuntime::exception_handler_for_return_address),
                        Oissuing_pc->after_save()); 

        __ jmp(O0, 0);
        __ delayed()->restore();
      }
      break;

    case throw_array_store_exception_id:
      {
        __ set_info("throw_array_store_exception", dont_gc_arguments);
        oop_maps = generate_exception_throw(sasm, CAST_FROM_FN_PTR(address, throw_array_store_exception));
      }
      break;

    case throw_class_cast_exception_id:
      {
        // G4: object
        __ set_info("throw_class_cast_exception", dont_gc_arguments);
        oop_maps = generate_exception_throw(sasm, CAST_FROM_FN_PTR(address, throw_class_cast_exception), G4);
      }
      break;

    case slow_subtype_check_id:
      { // Support for uint StubRoutine::partial_subtype_check( Klass sub, Klass super );
        // Arguments :
        //
        //      ret  : G3
        //      sub  : G3, argument, destroyed
        //      super: G1, argument, not changed
        //      raddr: O7, blown by call
        Label loop, miss;
        
        __ save_frame(0);		// Blow no registers!
        
        __ ld_ptr( G3, sizeof(oopDesc) + Klass::secondary_supers_offset_in_bytes(), L3 );
        __ lduw(L3,arrayOopDesc::length_offset_in_bytes(),L0); // length in l0
        __ add(L3,arrayOopDesc::base_offset_in_bytes(T_OBJECT),L1); // ptr into array
        __ clr(L4);			// Index
        // Load a little early; will load 1 off the end of the array.
        // Ok for now; revisit if we have other uses of this routine.
        __ ld_ptr(L1,0,L2);		// Will load a little early
        
        // The scan loop
        __ bind(loop);
        __ add(L1,wordSize,L1);	// Bump by OOP size
        __ cmp(L4,L0); 
        __ br(Assembler::equal,false,Assembler::pn,miss);
        __ delayed()->inc(L4);	// Bump index
        __ subcc(L2,G1,L3);		// Check for match; zero in L3 for a hit
        __ brx( Assembler::notEqual, false, Assembler::pt, loop );
        __ delayed()->ld_ptr(L1,0,L2); // Will load a little early
        
        // Got a hit; report success; set cache
        __ st_ptr( G1, G3, sizeof(oopDesc) + Klass::secondary_super_cache_offset_in_bytes() );
        
        __ mov(1, G3);
        __ ret();			// Result in G5 is ok; flags set
        __ delayed()->restore();	// free copy or add can go here

        __ bind(miss);
        __ mov(0, G3);
        __ ret();			// Result in G5 is ok; flags set
        __ delayed()->restore();	// free copy or add can go here
      }

    case monitorenter_id:
      { // O0: object
        // O1: lock address
        __ set_info("monitorenter", dont_gc_arguments);
        __ save_frame(0);
        __ call_RT(noreg, noreg, CAST_FROM_FN_PTR(address, monitorenter), I0, I1);
        __ ret();
        __ delayed()->restore();
      }
      break;

    case monitorenter_with_jvmpi_id:
      { // This is used for slow-case synchronization at method entry when JVMPI method entry events are enabled.
        // If the compiled activation has been deoptimized after the monitorenter, then jvmpi notification is done
        // here since execution will resume in the interpreter at the first bytecode.
        // O0: object
        // O1: lock address
        __ set_info("monitorenter", dont_gc_arguments);
        __ save_frame(0);
        // have to make sure the stack copy of I7 has a valid value, otherwise the check
        // for deoptimization following the call can fail
        __ st_ptr(I7, SP, (I7->sp_offset_in_saved_window()*BytesPerWord) + STACK_BIAS);

        // Preserve the method's receiver in case its needed for jvmpi method entry
        Address vm_result_addr(G2_thread, 0, in_bytes(JavaThread::vm_result_offset()));
        __ st_ptr(I0, vm_result_addr);
        __ call_RT(I0, noreg, CAST_FROM_FN_PTR(address, monitorenter), I0, I1);

        // If the compiled frame was deoptimized, the return address of this frame
        // was changed to be the deoptimization handler's normal entry point
        DeoptimizationBlob* deopt_blob = (DeoptimizationBlob*) Runtime1::blob_for(Runtime1::deoptimization_handler_id);
        assert(deopt_blob != NULL, "deoptimization blob must have been created");
        Label no_deopt;
        __ ld_ptr(SP, (I7->sp_offset_in_saved_window()*BytesPerWord) + STACK_BIAS, L0);        
        __ add(L0, frame::pc_return_offset, L1);
        __ load_address(Address(L2, deopt_blob->unpack()));
        __ cmp(L1, L2);
        __ brx(Assembler::notEqual, false, Assembler::pt, no_deopt);
        __ delayed()->nop();   
        __ call_RT(noreg, noreg, CAST_FROM_FN_PTR(address, jvmpi_method_entry_after_deopt), I0);

        __ bind(no_deopt);
        __ ret();
        __ delayed()->restore();
      }
      break;

    case monitorexit_id:
      { // G4: lock address
        // note: really a leaf routine but must setup last java sp
        //       => use call_RT for now (speed can be improved by
        //       doing last java sp setup manually)
        __ set_info("monitorexit", dont_gc_arguments);
        __ save_frame(0);
        __ call_RT(noreg, noreg, CAST_FROM_FN_PTR(address, monitorexit), G4);
        __ ret();
        __ delayed()->restore();
      }
      break;

    case implicit_div0_exception_id: // fall through
      {
        // we are in the exception handler stub frame
        // O7: filter return address
        __ set_info("implicit_div0_exception", dont_gc_arguments);
        __ call_VM_leaf(L7_thread_cache, CAST_FROM_FN_PTR(address, return_address_for_exception));
        __ sub(O0, frame::pc_return_offset, I7);
        __ call_RT(noreg, noreg, CAST_FROM_FN_PTR(address, implicit_throw_div0_exception));
        __ should_not_reach_here();
      }
      break;

    case implicit_null_exception_id:
      {
        // we are in the exception handler stub frame
        // O7: filter return address
        Label L;
        __ set_info("implicit_null_exception", dont_gc_arguments);
        __ call_VM_leaf(L7_thread_cache, CAST_FROM_FN_PTR(address, return_address_for_exception));
        __ br_notnull(O0, true, Assembler::pt, L);
        __ delayed()->sub(O0, frame::pc_return_offset, I7);
        __ bind(L);
        __ call_RT(noreg, noreg, CAST_FROM_FN_PTR(address, implicit_throw_null_exception));
        __ should_not_reach_here();
      }
      break;

    case interpreter_entries_id:
      { __ set_info("interpreter_entries", must_gc_arguments);
        __ stop("do not call interpreter entries entry directly");
        // generate entries for all method kinds
        for (int i = 0; i < AbstractInterpreter::number_of_method_entries; i++) {
          AbstractInterpreter::MethodKind kind = (AbstractInterpreter::MethodKind)i;

          Register recv       = O0;
          Register icache     = G5;
          //       Gargs      = G4; 

          //--------------------------------------------------------------------------------
          // entry point to methods that are final
          // (icache contains IC holder oop => get methodOop)
          int virtual_final_call_offset = __ offset();
          Label optimized_call;
          __ verify_oop(recv);
          __ verify_oop(icache);
          __ br(Assembler::always, false, Assembler::pt, optimized_call);
          __ delayed()->ld_ptr(icache, compiledICHolderOopDesc::holder_method_offset(), icache);


          //--------------------------------------------------------------------------------
          // entry point for virtual calls
          // (icache contains IC holder oop => test & get methodOop)
          int virtual_call_offset = __ offset();
          { const Register klass1 = G1;
            const Register klass2 = G3;
            assert_different_registers(recv, icache, klass1, klass2);

            __ verify_oop(recv);
            __ verify_oop(icache);
            __ ld_ptr(recv  , oopDesc::klass_offset_in_bytes              (), klass1);
            __ ld_ptr(icache, compiledICHolderOopDesc::holder_klass_offset(), klass2);
            __ verify_oop(klass1);
            __ verify_oop(klass2);
            __ cmp(klass1, klass2);
            __ brx(Assembler::equal, false, Assembler::pt, optimized_call);
            __ delayed()->ld_ptr(icache, compiledICHolderOopDesc::holder_method_offset(), icache);
            __ jump_to(Address(klass1, entry_for(handle_ic_miss_id)));
            __ delayed()->nop();
          }

          //--------------------------------------------------------------------------------
          // entry point for optimized virtual calls
          // (icache contains methodOop => test if target is compiled)
          int optimized_call_offset = __ offset();
          Label setup_parameters;
          __ bind(optimized_call);
          { const Register tmp = G1;
            assert_different_registers(recv, icache, tmp);
            
            Label L;
            __ verify_oop(recv);
            __ verify_oop(icache);
            __ ld_ptr(icache, in_bytes(methodOopDesc::compiled_code_offset()), tmp);
            __ br_null(tmp, false, Assembler::pt, setup_parameters);
            __ delayed()->nop();
            __ jump_to(Address(tmp, entry_for(handle_wrong_method_id)));
            __ delayed()->nop();
          }

          //--------------------------------------------------------------------------------
          // entry point for static calls
          // (icache contains methodOop)
          int static_call_offset = __ offset();
          { const Register tmp = G1;
            assert_different_registers(recv, icache, tmp);
            
            Label L;
            __ verify_oop(icache);
            __ ld_ptr(icache, in_bytes(methodOopDesc::compiled_code_offset()), tmp);
            __ br_null(tmp, false, Assembler::pt, setup_parameters);
            __ delayed()->nop();
            __ jump_to(Address(tmp, entry_for(handle_wrong_static_method_id)));
            __ delayed()->nop();
          }

          //--------------------------------------------------------------------------------
          // copy parameters on stack if they are passed in registers
          { Label setup_parameter_map;
            const Register map = Gargs;
            const Register dst = G3;
            assert_different_registers(recv, icache, map, dst);

            // Note: set_info is setup at the beginning of the interpreter entries!
            __ bind(setup_parameter_map);
            __ save_frame(0);
            __ call_RT(icache, noreg, CAST_FROM_FN_PTR(address, prepare_interpreter_call), icache);
            __ restore();

            __ bind(setup_parameters);
            // setup registers
            __ lduh(icache, in_bytes(methodOopDesc::size_of_parameters_offset()), dst);
            __ lduh(icache, in_bytes(methodOopDesc::parameter_info_offset())    , map);
            __ sll(dst, LogBytesPerWord, dst);      // parameter size in bytes
            __ add(SP, dst, dst);
            // make sure parameter map is setup
            __ tst(map);
            __ br(Assembler::zero, false, Assembler::pn, setup_parameter_map);
            // copy parameters
            assert( SPARC_ARGS_IN_REGS_NUM <= 6, "Can't use btst");
            const int n = SPARC_ARGS_IN_REGS_NUM;   // number of register that can be used for parameter passing
            const int average_parameter_size = 2;   // average no. of parameters - for branch prediction only
            __   delayed()->btst(1 << 0, map);
            for (int i = 1; i <= n; i++) {
              Label L;
              __ br(Assembler::zero, false, i <= average_parameter_size ? Assembler::pn : Assembler::pt, L);
#ifdef _LP64
              __ delayed()->btst(1 << ((i-1)*2)+1, map); // Object or int?
              // Assume object is in arg 0
              __ br(Assembler::zero, true, (i - 1) == 0 ? Assembler::pn : Assembler::pt, L);
              __ delayed()->st(as_oRegister(i - 1), dst, 
                   ((frame::memory_parameter_word_sp_offset - i) * wordSize) + STACK_BIAS);
              __ st_ptr(as_oRegister(i - 1), dst, 
                   ((frame::memory_parameter_word_sp_offset - i) * wordSize) + STACK_BIAS);
              __ bind(L);
              if ( i == n ) __ nop();          // Don't overflow simm13 with btst
              else __ btst(1 << (i*2), map);   // Test next parameter
#else
              if ( i == n ) __ delayed()->nop();
              else __ delayed()->btst(1 << (i*2), map);     // for i == n, this test is not needed, but it doesn't matter
              __ st_ptr(as_oRegister(i - 1), dst, 
                   ((frame::memory_parameter_word_sp_offset - i) * wordSize) + STACK_BIAS);
              __ bind(L);
#endif
            }
          }

          // continue in interpreter
          { const Register tmp = G3;
            assert_different_registers(recv, icache, Gargs, tmp);
            __ jump_to(Address(tmp, AbstractInterpreter::entry_for_kind(kind)));
            __ delayed()->add(SP, (frame::memory_parameter_word_sp_offset * wordSize) + STACK_BIAS, Gargs);
          }

          //--------------------------------------------------------------------------------
          // setup entries
          _ientries[i] = iEntries(static_call_offset, optimized_call_offset, virtual_call_offset, virtual_final_call_offset);
        }
      }
      break;

    case stack_overflow_exception_id:
      { __ set_info("stack_overflow_exception", dont_gc_arguments);
        __ call_RT(noreg, noreg, CAST_FROM_FN_PTR(address, throw_stack_overflow));
        __ should_not_reach_here();
      }
      break;

    case init_check_patching_id:
      { __ set_info("init_check_patching", dont_gc_arguments);
        oop_maps = generate_patching(sasm, CAST_FROM_FN_PTR(address, init_check_patching));
      }
      break;

    case load_klass_patching_id:
      { __ set_info("load_klass_patching", dont_gc_arguments);
        oop_maps = generate_patching(sasm, CAST_FROM_FN_PTR(address, move_klass_patching));
      }
      break;

    case osr_frame_return_id:
      { __ set_info("osr_frame_return", dont_gc_arguments);
        // Continuation point after returning from osr compiled method.
        // Position a potential integer result for returning from the original interpreted activation.
        __ mov(O0, I0);
        __ mov(O1, I1);
        // Return, restoring the caller's register window and the calling activation's SP to
        // its value before it was changed to accommodate the method's non-parameter locals.
        // The SP adjustment was calculated in Lscratch by the osr entry code - see CodeEmitter::emit_osr_entry().
        __ ret();
        __ delayed()->restore(FP, Lscratch, SP);   // callee's FP + Lscratch -> caller's SP
      }
      break;

    case osr_unwind_exception_id:
      { __ set_info("osr_unwind_exception", dont_gc_arguments);
        // Upon entry, Oexception holds the exception, and I7 holds the PC of the call that created this activation.
        // Position the exception oop and calling PC for the unwinding restore instruction, which also protects them
        // from the ensuing VM call.
        __ mov(Oexception, Oexception->after_save());
        __ add(I7, frame::pc_return_offset, Oissuing_pc->after_save());

        // Get handler address in O0.
        __ call_VM_leaf(L7_thread_cache, CAST_FROM_FN_PTR(address, SharedRuntime::exception_handler_for_return_address),
                        Oissuing_pc->after_save()); 

        // Return, restoring the caller's register window and the calling activation's SP to
        // its value before it was changed to accomodate the method's non-parameter locals.
        // The SP adjustment was calculated in Lscratch by the osr entry code - see CodeEmitter::emit_osr_entry().
        __ jmp(O0, 0);
        __ delayed()->restore(FP, Lscratch, SP);   // callee's FP + Lscratch -> caller's SP
      }
      break;

    case illegal_instruction_handler_id:
      { __ set_info("illegal_instruction_handler", dont_gc_arguments);
        oop_maps = generate_illegal_instruction_handler(sasm, CAST_FROM_FN_PTR(address, SafepointSynchronize::handle_illegal_instruction_exception));
      }
      break;

    case deoptimization_handler_id:
      { __ set_info("deoptimization_handler", dont_gc_arguments);
        oop_maps = generate_deoptimization_handler(sasm, unpack_exception_offset, frame_size, unpack_reexecute_offset);
      }
      break;

    case check_safepoint_and_suspend_for_native_trans_id:
      __ unimplemented("check safepoint and suspend id is not implemented");
      break;

    case jvmdi_exception_throw_id:
      { // Oexception : exception
        // Oissuing_pc: pc
        __ set_info("jvmdi_exception_throw", dont_gc_arguments);
        __ save_frame(0);
        __ call_RT(noreg, noreg, CAST_FROM_FN_PTR(address, Runtime1::post_jvmdi_exception_throw), I0);
        __ ret();
        __ delayed()->restore();
      }
      break;

    case jvmpi_method_entry_id:
      { // O0: methodOop
        // O1: receiver or NULL
        __ set_info("jvmpi_method_entry", dont_gc_arguments);
        __ save_frame(0);
        __ call_RT(noreg, noreg, CAST_FROM_FN_PTR(address, SharedRuntime::jvmpi_method_entry), I0, I1);
        __ ret();
        __ delayed()->restore();
      }
      break;

    case jvmpi_method_exit_id:
      { // O0: methodOop
        // O1: method's oop result or null
        int     frame_words = sizeof(double)/wordSize;
        Address saved_result_addr(FP, 0, -sizeof(double) + STACK_BIAS);
        Address vm_result_addr(G2_thread, 0, in_bytes(JavaThread::vm_result_offset()));
        // This stub is frameless so that its compiled call site doesn't need debug info and
        // can't be deoptimized, since the compiled frame will be viewed as a stub frame.
        __ set_info("jvmpi_method_exit", dont_gc_arguments);
        // Preserve this stub's return address since it will be overwritten by call_RT (use I5 - see
        // frame::patch_frameless_stub_return_addr, used to update the return address after a safepoint)
        __ mov(O7, I5); 
        // Preserve any floating-point result.
        __ stf(FloatRegisterImpl::D, F0, saved_result_addr);
        // Preserve the method's oop result, or null if the result is not an oop, and have call_RT
        // restore the value in I2, rather than I0, so that a non-oop integer result isn't overwritten.
        __ st_ptr(O1, vm_result_addr);
        __ call_RT(I2, noreg, CAST_FROM_FN_PTR(address, SharedRuntime::jvmpi_method_exit), O0);
        // Restore potential floating-point result and the return address.
        __ ldf(FloatRegisterImpl::D, saved_result_addr, F0);
        __ mov(I5, O7);
        // Frameless return
        __ retl();
        __ delayed()->nop();
      }
      break;

    default:
      { __ set_info("unimplemented entry", dont_gc_arguments);
        __ save_frame(0);
        __ set((int)id, O0);
        __ call_RT(noreg, noreg, CAST_FROM_FN_PTR(address, unimplemented_entry), I0);
        __ should_not_reach_here();
      }
      break;
  }
  return oop_maps;
}

#undef __


class ParameterMapper: public SignatureIterator {
 protected:
  uint64_t _fingerprint;
  int _recv_slot;
  // bitmap has 2 bits per argument.  
  //   00: No argument  (or double, long, float)
  //   01: jint sized argument (bool, char, byte, short , int)
  //   10: not used
  //   11: object or array argument
  int _map;                              

  void copy( bool object ) {
    const int i = _recv_slot + parameter_index();
    if (i < SPARC_ARGS_IN_REGS_NUM) _map |= ( (object?3:1) << (i*2));
  }

 public:
  // Constructor
  ParameterMapper(methodHandle method, symbolHandle signature) :
                                   SignatureIterator(signature) {
    _recv_slot = method->is_static() ? 0 : 1;
    _map       = 0;
    _fingerprint = Fingerprinter(method).fingerprint();
  }

  // Accessors
  int map() {
    // compute _map lazily
    if (_map == 0) {
      // set 1st argument if we have a receiver and set 'termination' bit to make sure _map != 0
      _map = (_recv_slot?3:0) | (1 << (SPARC_ARGS_IN_REGS_NUM*2));
      // set bit for remaining parameters passed in registers
      iterate_parameters( _fingerprint );
    }
    return _map;
  }

  // Basic types
  virtual void do_bool  ()                       { copy( false ); }
  virtual void do_char  ()                       { copy( false ); }
  virtual void do_float ()                       { /* ignore */ }
  virtual void do_double()                       { /* ignore */ }
  virtual void do_byte  ()                       { copy( false ); }
  virtual void do_short ()                       { copy( false ); }
  virtual void do_int   ()                       { copy( false ); }
  virtual void do_long  ()                       { /* ignore */ }
  virtual void do_void  ()                       { ShouldNotReachHere(); }

  // Object types (begin indexes the first character of the entry, end indexes the first character after the entry)
  virtual void do_object(int begin, int end)     { copy( true ); }
  virtual void do_array (int begin, int end)     { copy( true ); }
};


JRT_ENTRY(void, Runtime1::prepare_interpreter_call(JavaThread* thread, methodOop method))
  // Note: This function may be called by more than one thread simultaneously.
  //       However, no locking is required since all threads will compute the
  //       same result.
  thread->set_vm_result(method);
  if (method->parameter_info() == 0) {
    // only compute parameter info if it has not been computed
    // before and no thread has won the race in the meantime
    methodHandle mh(thread, method);
    symbolHandle signature(thread, method->signature());
    method->set_parameter_info(ParameterMapper(mh, signature).map());
  }
JRT_END


// Implementation of GC_Support

class FindSignatureTypes: public SignatureInfo {
 private:
  BasicTypeArray* _types;
  int  _index;
  bool _is_static;

 public: 
  FindSignatureTypes(symbolHandle signature, bool is_static, BasicTypeArray* types): SignatureInfo(signature) {
    _types = types;
    _index = 0;
    _is_static = is_static;
  }

  void set(int size, BasicType type) {
    _types->at_put(_index, type);
    if (size == 2) _types->at_put(_index+1, type);
    _index += size;

  }

  void find() {
    if (!_is_static) {
      _types->at_put(0, T_OBJECT);
      _index ++;
    }
    iterate_parameters();
  }
};


class C1_ArgumentOopFinder: public SignatureInfo {
 private:
  OopClosure*         _f;           // oop visitor
  bool                _is_static;   // true if the callee is a static method
  const frame         _fr;
  CallingConvention*  _calling_convention;
  int                 _idx;

  void set(int size, BasicType type) {
    if (type == T_OBJECT || type == T_ARRAY) {
      oop_arg_do();
    }
    _idx += size;
  }

  void oop_arg_do() {
    ArgumentLocation location = _calling_convention->arg_at(_idx);
    oop* addr = (oop*)GC_Support::get_argument_addr_at(_fr, location);
    _f->do_oop(addr);
  }

 public:
  C1_ArgumentOopFinder(symbolHandle signature,
                       bool is_static,
                       const frame fr,
                       OopClosure* f)
    : SignatureInfo(signature),
      _f(f),
      _is_static(is_static),
      _fr(fr) {
    _idx = 0;
    int args_size = ArgumentSizeComputer(signature).size() + (is_static ? 0 : 1);
    BasicTypeArray sig_types(args_size, T_ILLEGAL);
    FindSignatureTypes type_finder(signature, is_static, &sig_types);
    type_finder.find();
    _calling_convention = FrameMap::calling_convention(is_static, sig_types);
  }

  void oops_do() {
    _idx = 0;
    if (!_is_static) {
      oop_arg_do();
      _idx++;
    }
    iterate_parameters();
  }
};


void GC_Support::preserve_callee_argument_oops(const frame fr, symbolHandle signature, bool is_static, OopClosure* f) {
  C1_ArgumentOopFinder finder(signature, is_static, fr, f);
  finder.oops_do();
}


intx* GC_Support::get_argument_addr_at(const frame fr, ArgumentLocation location) {
  if (location.is_stack_arg()) {
    int offset_from_sp_in_words = location.stack_offset_in_words();
    return get_stack_argument_addr_at(fr, offset_from_sp_in_words);
  } else {
    assert(location.is_register_arg(), "sanity check");
    Register r = location.outgoing_reg_location().as_register();
    return get_register_argument_addr_at(fr, r);
  }
}


intx* GC_Support::get_stack_argument_addr_at(const frame fr, int offset_from_sp_in_words) {
  return fr.sp_addr_at(offset_from_sp_in_words);
}


intx* GC_Support::get_register_argument_addr_at(const frame fr, Register r) {
  frame next_younger = fr.after_save();
  return next_younger.register_addr(r->after_save());
}


bool DeoptimizationBlob::exception_address_is_unpack_entry(address pc) const {
  DeoptimizationBlob* deopt_blob = (DeoptimizationBlob*) Runtime1::blob_for(Runtime1::deoptimization_handler_id);
  address unpack_pc = deopt_blob->unpack();
  return (pc == unpack_pc || (pc + frame::pc_return_offset) == unpack_pc);
}


int vframeArray::extend_caller_frame(int callee_locals, int callee_parameters) {
  // compute amount to extend SP of caller to accomodate interpreted callee's locals
  int adjustment = round_to((callee_locals - callee_parameters), WordsPerLong);
  return adjustment;
}

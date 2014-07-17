 #ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)runtime_sparc.cpp	1.105 03/04/30 22:28:40 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_runtime_sparc.cpp.incl"

// Deoptimization

#define __ masm->

UncommonTrapBlob   *OptoRuntime::_uncommon_trap_blob;
DeoptimizationBlob *OptoRuntime::_deopt_blob;
ExceptionBlob      *OptoRuntime::_exception_blob;
SafepointBlob      *OptoRuntime::_illegal_instruction_handler_blob;

int handle_exception_deopt_offset = 0;
int handle_exception_deopt_exception_offset = 0;
int handle_exception_deopt_offset_2 = 0;
int handle_exception_deopt_exception_offset_2 = 0;
int handle_exception_call_pc_offset = 0;

static const int exception_blob_words = 0;

//------------------------------setup_exception_blob---------------------------
// creates exception blob at the end
// Using exception blob, this code is jumped from a compiled method.
// (see emit_exception_handler in sparc.ad file)
// It is also the default exception destination for c2i and i2c adapters
// 
// Given an exception pc at a call, we may unwind the stack and jump to
// caller's exception handler. We may also continue into the handler
// This code is entered with a jmp.
// 
// Arguments:
//   O0: exception oop
//   O1: exception pc
//
// Results:
//   O0: exception oop
//   O1: exception pc in caller or ???
//   destination: exception handler of caller
// 
// Note: the exception pc MUST be at a call (precise debug information)
//
void OptoRuntime::setup_exception_blob() {
  // allocate space for code
  ResourceMark rm;

  // setup code generation tools
  CodeBuffer*     buffer   = new CodeBuffer(512, 512, 0, 0, 0, false);
  MacroAssembler* masm     = new MacroAssembler(buffer);

  Label exception_handler_found;
  Label L;

  int start = __ offset();

  __ verify_thread();
  __ st_ptr(Oexception,  Address(G2_thread, 0, in_bytes(JavaThread::exception_oop_offset())));
  __ st_ptr(Oissuing_pc, Address(G2_thread, 0, in_bytes(JavaThread::exception_pc_offset())));

  // This call does all the hard work. It checks if an exception handler
  // exists in the method.  
  // If so, it returns the handler address.
  // If not, it prepares for stack-unwinding, restoring the callee-save 
  // registers of the frame being removed.
  //
  __ save_frame(exception_blob_words);
  __ mov(G2_thread, O0);
  __ set_last_Java_frame(SP, G0);
  __ save_thread(L7_thread_cache);

  __ call(CAST_FROM_FN_PTR(address, OptoRuntime::handle_exception_C), relocInfo::runtime_call_type);
  __ delayed()->mov(L7_thread_cache, O0);
  handle_exception_call_pc_offset = __ offset() - start;

  __ bind(L);
  __ restore_thread(L7_thread_cache);
  __ reset_last_Java_frame();
  __ mov(O0, G3_scratch);             // Move handler address to temp
  __ restore();

  __ br_notnull(G3_scratch, false, Assembler::pn, exception_handler_found);
  __ delayed()->nop();

  // We have to check if the caller frame has been deoptimized;
  // If the caller frame has been deoptimized then we must continue
  // at DeoptimizationBlob::unpack_with_exception address; 
  // If the caller frame is not deoptimized, then we continue at
  // JavaThread::exception_handler_pc_offset_in_bytes.
  // The caller frame state is defined by its return address.

  __ ld_ptr(Address(G2_thread, 0, in_bytes(JavaThread::exception_oop_offset())),
	Oexception  ->after_save());
  __ ld_ptr(Address(G2_thread, 0, in_bytes(JavaThread::exception_pc_offset())),
	Oissuing_pc ->after_save());

  Label endL;

  __ ld_ptr(Address(G2_thread, 0, in_bytes(JavaThread::exception_handler_pc_offset())), G3_scratch);
  handle_exception_deopt_offset = __ offset() - start;
  __ set((uintptr_t)-1>>1, O5);    // will be filled in later with fill_in_exception_blob
  __ inc(O5, -frame::pc_return_offset);
  __ cmp(I7, O5);                 // get return address to check if the caller frame is deoptimized
  __ brx(Assembler::notEqual, false, Assembler::pn, endL);
  __ delayed()->nop();

  // deoptimizing this frame:  set G3_scratch to point to a stub
  handle_exception_deopt_exception_offset = __ offset() - start;
  __ set((uintptr_t)-1>>1, G3_scratch);  // will be filled in later with fill_in_exception_blob

  __ bind (endL);
  // G3_scratch is handler-pc of caller is where we continue or the deoptimization blob address
  // Oexception is live
#ifdef ASSERT
  __ st_ptr(G0, Address(G2_thread, 0, in_bytes(JavaThread::exception_oop_offset())));
  __ st_ptr(G0, Address(G2_thread, 0, in_bytes(JavaThread::exception_handler_pc_offset())));
  __ st_ptr(G0, Address(G2_thread, 0, in_bytes(JavaThread::exception_pc_offset())));
#endif
  __ jmpl(G3_scratch, G0, G0);
  __ delayed()->restore();               // remove frame

  // Exception handler found.
  __ bind(exception_handler_found);

  Label endL_2;

  // check if top frame has been deoptimized; if yes, exception handler must be
  // deoptimized blob
  // address to check is in Oissuing_pc
  handle_exception_deopt_offset_2 = __ offset() - start;
  __ set((uintptr_t)-1>>1, O5);  // will be filled in later with fill_in_exception_blob
  __ inc(O5, -frame::pc_return_offset);
  __ cmp(O7, O5);
  __ brx(Assembler::notEqual, false, Assembler::pn, endL_2);
  __ delayed()->nop();

  handle_exception_deopt_exception_offset_2 = __ offset() - start;
  __ set((uintptr_t)-1>>1, G3_scratch);  // will be filled in later with fill_in_exception_blob

  __ bind (endL_2);

  // G3_scratch contains handler address
  assert(Assembler::is_simm13(in_bytes(JavaThread::exception_oop_offset())), "exception offset overflows simm13, following ld instruction cannot be in delay slot");
#ifdef ASSERT
  __ ld_ptr(Address(G2_thread, 0, in_bytes(JavaThread::exception_oop_offset())), Oexception);
  __ st_ptr(G0, Address(G2_thread, 0, in_bytes(JavaThread::exception_handler_pc_offset())));
  __ st_ptr(G0, Address(G2_thread, 0, in_bytes(JavaThread::exception_pc_offset())));
  __ st_ptr(G0, Address(G2_thread, 0, in_bytes(JavaThread::exception_oop_offset())));
  __ jmpl(G3_scratch, G0, G0);
  __ delayed()->nop();
#else
  __ jmpl(G3_scratch, G0, G0);
  __ delayed()->ld_ptr(Address(G2_thread, 0, in_bytes(JavaThread::exception_oop_offset())), Oexception);
#endif

  // -------------
  // make sure all code is generated
  masm->flush();

  // Fill out other meta info
  OopMapSet *oop_maps = NULL; // will be set later; currently the register stuff is not yet initialized!
  _exception_blob = ExceptionBlob::create(buffer, oop_maps,  __ total_frame_size_in_bytes(0)/wordSize);
  address xxxyyy = _exception_blob->instructions_begin();
}

//------------------------------fill_in_exception_blob-------------------------
void OptoRuntime::fill_in_exception_blob() {
  int framesize;
  { ResourceMark rm;
    MacroAssembler* masm = new MacroAssembler(new CodeBuffer(NULL, NULL));
    framesize = __ total_frame_size_in_bytes(exception_blob_words)/wordSize;
  }

   // Patch blob
  assert(handle_exception_stub() != NULL, "exception stub must have been generated");
  assert(handle_exception_call_pc_offset != 0, "");
  assert(handle_exception_deopt_exception_offset != 0, "");
  assert(handle_exception_deopt_offset != 0, "");
  assert(handle_exception_deopt_exception_offset_2 != 0, "");
  assert(handle_exception_deopt_offset_2 != 0, "");

  {
  NativeMovConstReg* move = nativeMovConstReg_at(exception_blob()->instructions_begin() + handle_exception_deopt_offset);
  move->set_data((intptr_t)OptoRuntime::deoptimization_blob()->unpack());

  NativeMovConstReg* move2 = nativeMovConstReg_at(exception_blob()->instructions_begin() + handle_exception_deopt_exception_offset);
  move2->set_data((intptr_t)OptoRuntime::deoptimization_blob()->unpack_with_exception());
  }
  {
  NativeMovConstReg* move = nativeMovConstReg_at(exception_blob()->instructions_begin() + handle_exception_deopt_offset_2);
  move->set_data((intptr_t)OptoRuntime::deoptimization_blob()->unpack());

  NativeMovConstReg* move2 = nativeMovConstReg_at(exception_blob()->instructions_begin() + handle_exception_deopt_exception_offset_2);
  move2->set_data((intptr_t)OptoRuntime::deoptimization_blob()->unpack_with_exception());
  }
  // Set an oopmap for the call site.  This oopmap will only be used if we
  // are unwinding the stack.  Hence, all locations will be dead.
  // Callee-saved registers will be the same as the frame above (i.e.,
  // handle_exception_stub), since they were restored when we got the
  // exception.
  ResourceMark rm;
  OopMapSet *oop_maps = new OopMapSet();
  // No callee-save registers to save here.
  oop_maps->add_gc_map( handle_exception_call_pc_offset, true, new OopMap(framesize, 0));
  exception_blob()->set_oop_maps(oop_maps);

}

static void gen_new_frame(MacroAssembler* masm, bool deopt) {
//
// Common out the new frame generation for deopt and uncommon trap
//
  Register        G3pcs              = G3_scratch; // Array of new pcs (input)
  Register        Oreturn0           = O0;
  Register        Oreturn1           = O1;
  Register        O2UnrollBlock      = O2;
  Register        O3array            = O3;	   // Array of frame sizes (input)
  Register        O4array_size       = O4;	   // number of frames (input)
  Register        O7frame_size       = O7;	   // number of frames (input)

  __ ld_ptr(O3array, 0, O7frame_size);
  __ sub(G0, O7frame_size, O7frame_size);
  __ save(SP, O7frame_size, SP);
  __ ld_ptr(G3pcs, 0, I7);			// load frame's new pc

  #ifdef ASSERT
  // make sure that the frames are aligned properly
#ifndef _LP64
  __ btst(wordSize*2-1, SP);
  __ breakpoint_trap(Assembler::notZero);
#endif
  #endif

  // Deopt needs to pass some extra live values from frame to frame

  if (deopt) {
    __ mov(Oreturn0->after_save(), Oreturn0);
    __ mov(Oreturn1->after_save(), Oreturn1);
  }

  __ mov(O4array_size->after_save(), O4array_size);
  __ sub(O4array_size, 1, O4array_size);
  __ mov(O3array->after_save(), O3array);
  __ mov(O2UnrollBlock->after_save(), O2UnrollBlock);
  __ add(G3pcs, wordSize, G3pcs);		// point to next pc value

  #ifdef ASSERT
  // trash registers to show a clear pattern in backtraces
  __ set(0xDEAD0000, I0);
  __ add(I0,  2, I1);
  __ add(I0,  4, I2);
  __ add(I0,  6, I3);
  __ add(I0,  8, I4);
  // Don't touch I5 could have valuable savedSP
  __ set(0xDEADBEEF, L0);
  __ mov(L0, L1);
  __ mov(L0, L2);
  __ mov(L0, L3);
  __ mov(L0, L4);
  __ mov(L0, L5);

  // trash the return value as there is nothing to return yet
  __ set(0xDEAD0001, O7);
  #endif
}

static void make_new_frames(MacroAssembler* masm, bool deopt) {
// 
// loop through the UnrollBlock info and create new frames
//
  Register        G3pcs              = G3_scratch;
  Register        Oreturn0           = O0;
  Register        Oreturn1           = O1;
  Register        O2UnrollBlock      = O2;
  Register        O3array            = O3;
  Register        O4array_size       = O4;
  Register        O5savedSP          = O5;
  Label           loop;

  __ ld(Address(O2UnrollBlock, 0, Deoptimization::UnrollBlock::number_of_frames_offset_in_bytes()), O4array_size);
  __ ld_ptr(Address(O2UnrollBlock, 0, Deoptimization::UnrollBlock::frame_pcs_offset_in_bytes()), G3pcs);

  __ ld_ptr(Address(O2UnrollBlock, 0, Deoptimization::UnrollBlock::frame_sizes_offset_in_bytes()), O3array);

  // At this point we either have an interpreter frame or an compiled frame on the
  // the stack. If it is a compiled frame we push a new c2i adpater here
  Label adapter_done;
  __ ld(Address(O2UnrollBlock, 0, Deoptimization::UnrollBlock::new_adapter_offset_in_bytes()), O7);
  __ tst(O7);
  __ br(Assembler::zero, false, Assembler::pt, adapter_done);
  __ delayed()-> nop();

  // Allocate the frame, setup the registers

  gen_new_frame(masm, deopt);		           // Allocate the i2c adapter

  __ add(O3array, wordSize, O3array);        // point to next frame size

  __ bind(adapter_done);
  
  // Adjust old interpreter frame (or new c2i) to make space for new frame's extra java locals
  // 
  // We capture the original sp for the transition frame only because it is needed in
  // order to properly calculate interpreter_sp_adjustment. Even though in real life
  // every interpreter frame captures a savedSP it is only needed at the transition
  // (fortunately). If we had to have it correct everywhere then we would need to
  // be told the sp_adjustment for each frame we create. If the frame size array
  // were to have twice the frame count entries then we could have pairs [sp_adjustment, frame_size]
  // for each frame we create and keep up the illusion every where. This would als
  // 
  
  __ mov( SP, O5savedSP );	// remember initial sender's original sp before adjustment
  __ ld(Address(O2UnrollBlock, 0, Deoptimization::UnrollBlock::caller_adjustment_offset_in_bytes()), O7);
  __ sub(SP, O7, SP);

  #ifdef ASSERT
  // make sure that there is at least one entry in the array
  __ tst(O4array_size);
  __ breakpoint_trap(Assembler::zero);
  #endif

  // Now push the new interpreter frames
  //

  __ bind(loop);

  // allocate a new frame, fillin the registers

  gen_new_frame(masm, deopt);        // allocate an interpreter frame

  __ mov( G0, O5savedSP );	// make it clear there is no savedSP after the first one

  __ tst(O4array_size);
  __ br(Assembler::notZero, false, Assembler::pn, loop);
  __ delayed()->add(O3array, wordSize, O3array);
  __ ld_ptr(G3pcs, 0, O7);                      // load final frame new pc

}


//------------------------------generate_deopt_blob----------------------------
// Ought to generate an ideal graph & compile, but here's some SPARC ASM
// instead.
void OptoRuntime::generate_deopt_blob() {
  // allocate space for the code
  ResourceMark rm;
  // setup code generation tools
  int pad = VerifyThread ? 256 : 0;// Extra slop space for more verify code
#ifdef _LP64
  CodeBuffer*     buffer             = new CodeBuffer(1280+pad, 512, 0, 0, 0, false);
#else
  CodeBuffer*     buffer             = new CodeBuffer(768+pad, 512, 0, 0, 0, false);
#endif
  MacroAssembler* masm               = new MacroAssembler(buffer);
  FloatRegister   Freturn0           = F0;
  Register        Greturn1           = G1;
  Register        Oreturn0           = O0;
  Register        Oreturn1           = O1;
  Register        O2UnrollBlock      = O2;
  Register        O3tmp              = O3;
  Register        O5exception_tmp    = O5;
  Register        G4exception_tmp    = G4_scratch;
  int             unpack_frame_words = (sizeof(double)+sizeof(jlong))/wordSize;
  int             framesize          = __ total_frame_size_in_bytes(unpack_frame_words)/wordSize;
  Address         saved_Freturn0_addr(FP, 0, -sizeof(double) + STACK_BIAS);
  Address         saved_Greturn1_addr(FP, 0, -sizeof(double) -sizeof(jlong) + STACK_BIAS);
  Label           cont, skip_i2c;

  OopMapSet *oop_maps = new OopMapSet();

  //
  // This is the entry point for code which is returning to a de-optimized
  // frame.
  // The steps taken by this frame are as follows:
  //   - push a dummy "unpack_frame" and save the return values (O0, O1, F0/F1, G1)
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

  int start = __ offset();

  // Add empty oopmap at begining. This can both be at_call and not_at_call,
  // Normally it would be at call. However, it could be at not_at_call, if the
  // thread happended to be blocked at safepoint_node in the frame that got
  // deoptimized.
  oop_maps->add_gc_map( 0, true, new OopMap( framesize, 0) );
  oop_maps->add_gc_map( 0, false, new OopMap( framesize, 0) );

  __ ba(false, cont);
  __ delayed()->mov(Deoptimization::Unpack_deopt, O5exception_tmp);

  int exception_offset = __ offset() - start;
  __ mov(Deoptimization::Unpack_exception, O5exception_tmp);

  __ bind(cont);
  // push a dummy "unpack_frame" taking care to preserve float return values
  // and call Deoptimization::fetch_unroll_info to get the UnrollBlock
  __ save_frame(unpack_frame_words);
  __ stf(FloatRegisterImpl::D, Freturn0, saved_Freturn0_addr);
#ifndef _LP64
  // 32-bit 1-register longs return longs in G1
  __ stx(Greturn1, saved_Greturn1_addr);
#endif
  __ set_last_Java_frame(SP, G0);
  __ call_VM_leaf(L7_thread_cache, CAST_FROM_FN_PTR(address, Deoptimization::fetch_unroll_info), G2_thread);
  // Set an oopmap for the call site   
  //%%% This is not the right place.  Is it really needed?
  //oop_maps->add_gc_map( __ offset()-start, true, new OopMap( framesize, 0) );

  __ reset_last_Java_frame();
  __ mov(O0, O2UnrollBlock->after_save());
  __ ldf(FloatRegisterImpl::D, saved_Freturn0_addr, Freturn0);
#ifndef _LP64
  // 32-bit 1-register longs return longs in G1
  __ ldx(saved_Greturn1_addr, Greturn1);
#endif
  __ restore();

  // %%%%%%% may have to setup some oop maps for the call site if G/O
  //         registers are used in code

  // deallocate the deoptimization frame taking care to preserve the return values
  __ mov(Oreturn0,     Oreturn0->after_save());
  __ mov(Oreturn1,     Oreturn1->after_save());
  __ mov(O2UnrollBlock, O2UnrollBlock->after_save());
  __ mov(O5exception_tmp, G4exception_tmp);
  __ restore();

  // Pop an I2C adapter if desired
  __ ld(Address(O2UnrollBlock, 0, Deoptimization::UnrollBlock::adapter_size_offset_in_bytes()), O3tmp);
  __ tst(O3tmp);
  __ br(Assembler::zero, false, Assembler::pt, skip_i2c);
  __ delayed()-> nop();
  __ mov(Oreturn0,     Oreturn0->after_save());
  __ mov(Oreturn1,     Oreturn1->after_save());
  __ mov(O2UnrollBlock, O2UnrollBlock->after_save());
  __ restore();			// Actually pop I2C frame here
  //
  // We are now in a frame that either called an I2C or an OSR
  // In either case IsavedSP has the stack pointer before the
  // callee (I2C/OSR) adjusted it. Remove that adjustment now
  // (it will be added back in when new interpreter frames are
  // built below). This assumes C2CallingConventions
  //
  __ sub(IsavedSP, STACK_BIAS, SP);


  __ bind(skip_i2c);

  // Allocate new interpreter frame(s) and possible c2i adapter frame

  make_new_frames(masm, true);

  // push a dummy "unpack_frame" taking care of float return values and
  // call Deoptimization::unpack_frames to have the unpacker layout
  // information in the interpreter frames just created and then return
  // to the interpreter entry point
  __ save_frame(unpack_frame_words);
  __ stf(FloatRegisterImpl::D, Freturn0, saved_Freturn0_addr);
#if !defined(_LP64)
  // 32-bit 1-register longs return longs in G1
  __ stx(Greturn1, saved_Greturn1_addr);
  __ set_last_Java_frame(SP, G0);
  __ call_VM_leaf(L7_thread_cache, CAST_FROM_FN_PTR(address, Deoptimization::unpack_frames), G2_thread, G4exception_tmp);
#else
  // LP64 uses g4 in set_last_Java_frame
  __ mov(G4exception_tmp, O1);
  __ set_last_Java_frame(SP, G0);
  __ call_VM_leaf(L7_thread_cache, CAST_FROM_FN_PTR(address, Deoptimization::unpack_frames), G2_thread, O1);
#endif
  __ reset_last_Java_frame();
  __ ldf(FloatRegisterImpl::D, saved_Freturn0_addr, Freturn0);
#if !defined(_LP64)
  // 32-bit 1-register longs return longs in G1, but interpreter wants them 
  // in O0/O1
  Label not_long;
  __ cmp(O0,T_LONG);
  __ br(Assembler::notEqual, false, Assembler::pt, not_long);
  __ delayed()->nop();
  __ ldd(saved_Greturn1_addr,I0);
  __ bind(not_long);
#endif
  __ ret();
  __ delayed()->restore();

  masm->flush();
  _deopt_blob = DeoptimizationBlob::create(buffer, oop_maps, 0, exception_offset, 0, framesize);
}


//------------------------------generate_uncommon_trap_blob--------------------
// Ought to generate an ideal graph & compile, but here's some SPARC ASM
// instead.
void OptoRuntime::generate_uncommon_trap_blob() {
  // allocate space for the code
  ResourceMark rm;
  // setup code generation tools
  OopMapSet *oop_maps          = NULL;
  int pad = VerifyThread ? 256 : 0;
#ifdef _LP64
  CodeBuffer*   buffer               = new CodeBuffer(1024+pad, 512, 0, 0, 0, false);
#else
  CodeBuffer*   buffer               = new CodeBuffer(768+pad, 512, 0, 0, 0, false);
#endif
  MacroAssembler* masm               = new MacroAssembler(buffer);
  Register        O2UnrollBlock      = O2;
  Register        O3tmp              = O3;
  Register        O2klass_index      = O2;
  Label           skip_i2c;

  //
  // This is the entry point for all traps the compiler takes when it thinks
  // it cannot handle further execution of compilation code. The frame is
  // deoptimized in these cases and converted into interpreter frames for
  // execution
  // The steps taken by this frame are as follows:
  //   - push a fake "unpack_frame"
  //   - call the C routine Deoptimization::uncommon_trap (this function
  //     packs the current compiled frame into vframe arrays and returns
  //     information about the number and size of interpreter frames which 
  //     are equivalent to the frame which is being deoptimized)
  //   - deallocate the "unpack_frame"
  //   - deallocate the deoptimization frame
  //   - If removing a paired I2C/C2I, deallocate the I2C frame.
  //   - in a loop using the information returned in the previous step
  //     push interpreter frames; includes a C2I frame if needed.
  //   - create a dummy "unpack_frame"
  //   - call the C routine: Deoptimization::unpack_frames (this function
  //     lays out values on the interpreter frame which was just created)
  //   - deallocate the dummy unpack_frame
  //   - return to the interpreter entry point
  //
  //  Refer to the following methods for more information:
  //   - Deoptimization::uncommon_trap
  //   - Deoptimization::unpack_frame

  // the unloaded class index is in O0 (first parameter to this blob)

  // push a dummy "unpack_frame"
  // and call Deoptimization::uncommon_trap to pack the compiled frame into
  // vframe array and return the UnrollBlock information
  __ save_frame(0);
  __ set_last_Java_frame(SP, G0);
  __ mov(I0, O2klass_index);
  __ call_VM_leaf(L7_thread_cache, CAST_FROM_FN_PTR(address, Deoptimization::uncommon_trap), G2_thread, O2klass_index);
  __ reset_last_Java_frame();
  __ mov(O0, O2UnrollBlock->after_save());
  __ restore();

  // %%%%%%% may have to setup some oop maps for the call site if G/O
  //         registers are used in code

  // deallocate the deoptimized frame taking care to preserve the return values
  __ mov(O2UnrollBlock, O2UnrollBlock->after_save());
  __ restore();

  // Pop an I2C adapter if desired
  __ ld(Address(O2UnrollBlock, 0, Deoptimization::UnrollBlock::adapter_size_offset_in_bytes()), O3tmp);
  __ tst(O3tmp);
  __ br(Assembler::zero, false, Assembler::pt, skip_i2c); // boolean (not pointer) check
  __ delayed()->nop();
  __ mov(O2UnrollBlock, O2UnrollBlock->after_save());
  __ restore();			// Actually pop I2C frame here
  //
  // We are now in a frame that either called an I2C or an OSR
  // In either case IsavedSP has the stack pointer before the
  // callee (I2C/OSR) adjusted it. Remove that adjustment now
  // (it will be added back in when new interpreter frames are
  // built below). This assumes C2CallingConventions
  //
  __ sub(IsavedSP, STACK_BIAS, SP);


  __ bind(skip_i2c);

  // Allocate new interpreter frame(s) and possible c2i adapter frame

  make_new_frames(masm, false);

  // push a dummy "unpack_frame" taking care of float return values and
  // call Deoptimization::unpack_frames to have the unpacker layout
  // information in the interpreter frames just created and then return
  // to the interpreter entry point
  __ save_frame(0);
  __ set_last_Java_frame(SP, G0);
  __ mov(Deoptimization::Unpack_uncommon_trap, O3); // indicate it is the uncommon trap case
  __ call_VM_leaf(L7_thread_cache, CAST_FROM_FN_PTR(address, Deoptimization::unpack_frames), G2_thread, O3);
  __ reset_last_Java_frame();
  __ ret();
  __ delayed()->restore();

  masm->flush();
  _uncommon_trap_blob = UncommonTrapBlob::create(buffer, oop_maps, __ total_frame_size_in_bytes(0)/wordSize);
}

//------------------------------generate_illegal_instruction_handler_blob------
//
// Generate a special Compile2Runtime blob that saves all registers, and sets
// up an OopMap.
//
// This blob is jumped to (via a breakpoint and the signal handler) from a
// safepoint in a thread code buffer.  On entry to this blob, O7 contains the
// address in the original nmethod at which we should resume normal execution.
// Thus, this blob looks like a subroutine which must preserve lots of
// registers and return normally.  Note that O7 is never register-allocated,
// so it is guaranteed to be free here.
//

// The hardest part of what this blob must do is to save the 64-bit %o
// registers in the 32-bit build.  A simple 'save' turn the %o's to %i's and
// an interrupt will chop off their heads.  Making space in the caller's frame
// first will let us save the 64-bit %o's before save'ing, but we cannot hand
// the adjusted FP off to the GC stack-crawler: this will modify the caller's
// SP and mess up HIS OopMaps.  So we first adjust the caller's SP, then save
// the 64-bit %o's, then do a save, then fixup the caller's SP (our FP).
// Tricky, tricky, tricky...

void OptoRuntime::generate_illegal_instruction_handler_blob() {
  assert (StubRoutines::forward_exception_entry() != NULL, "must be generated before");  
  Label no_pending_exception_L;
  Label deoptimizing_L;

  // allocate space for the code
  ResourceMark rm;
  // setup code generation tools  
  CodeBuffer*     buffer              = new CodeBuffer(1536, 1536, 0, 0, 0, false);
  MacroAssembler* masm                = new MacroAssembler(buffer);
  
  int start = __ offset();

  // Lots of registers to save.  For all builds, a window save will preserve
  // the %i and %l registers.  For the 32-bit longs-in-two entries and 64-bit
  // builds a window-save will preserve the %o registers.  In the LION build
  // we need to save the 64-bit %o registers which requires we save them
  // before the window-save (as then they become %i registers and get their
  // heads chopped off on interrupt).  We have to save some %g registers here
  // as well.
  enum { 
    // This frame's save area.  Includes extra space for the native call:
    // vararg's layout space and the like.  Briefly holds the caller's
    // register save area.
    window_save_offset = 0,
    call_args_area = window_save_offset+frame::memory_parameter_word_sp_offset*wordSize,
#ifdef _LP64
    start_of_extra_save_area = call_args_area+8, // pad frame_size to 16-byte
#else
    start_of_extra_save_area = call_args_area+4, // pad frame_size to 8-byte
#endif
    g1_offset = start_of_extra_save_area, // g-regs needing saving
    g3_offset = g1_offset+8,
    g4_offset = g3_offset+8,
    g5_offset = g4_offset+8,
    start_of_flags_save_area = g5_offset+8,
    fsr_offset = start_of_flags_save_area,
    d00_offset = fsr_offset+8,  // Start of float save area
    frame_size = d00_offset+8*32
  };

  // __ breakpoint_trap();

  // Force frame flush so it's visible during debug printing
  /*
  __ st_ptr(L0,SP, 0*wordSize+STACK_BIAS);
  __ st_ptr(L1,SP, 1*wordSize+STACK_BIAS);
  __ st_ptr(L2,SP, 2*wordSize+STACK_BIAS);
  __ st_ptr(L3,SP, 3*wordSize+STACK_BIAS);
  __ st_ptr(L4,SP, 4*wordSize+STACK_BIAS);
  __ st_ptr(L5,SP, 5*wordSize+STACK_BIAS);
  __ st_ptr(L6,SP, 6*wordSize+STACK_BIAS);
  __ st_ptr(L7,SP, 7*wordSize+STACK_BIAS);
  __ st_ptr(I0,SP, 8*wordSize+STACK_BIAS);
  __ st_ptr(I1,SP, 9*wordSize+STACK_BIAS);
  __ st_ptr(I2,SP,10*wordSize+STACK_BIAS);
  __ st_ptr(I3,SP,11*wordSize+STACK_BIAS);
  __ st_ptr(I4,SP,12*wordSize+STACK_BIAS);
  __ st_ptr(I5,SP,13*wordSize+STACK_BIAS);
  __ st_ptr(I6,SP,14*wordSize+STACK_BIAS);
  __ st_ptr(I7,SP,15*wordSize+STACK_BIAS);
  */

#if !defined(_LP64)
  // Save 64-bit O registers; they will get their heads chopped off on a 'save'.
  // We got no spares to work with, so be careful!

  // This thread can't run any recursive Java code from here because it's just
  // stopped for GC or some safepoint.  Thus we can't recursively invoke this
  // code and stomp over the 64-bit O's, so we don't need to copy the O's to
  // this stack frame.  We actually only need the high bits.  The low bits are
  // tucked away in register-window where they'll get GC'd and modified.
  // We'll have to fold the low and high bits back together again later.  Note
  // that we can't just store down the full 64-bits and GC them in place,
  // because the stack crawler "knows" where the O regs live (in the register
  // window) and ignores other advice from the OopMap.
  __ stx(O0,G2_thread,JavaThread::o_reg_temps_offset_in_bytes()+0*8);
  __ stx(O1,G2_thread,JavaThread::o_reg_temps_offset_in_bytes()+1*8);
  __ stx(O2,G2_thread,JavaThread::o_reg_temps_offset_in_bytes()+2*8);
  __ stx(O3,G2_thread,JavaThread::o_reg_temps_offset_in_bytes()+3*8);
  __ stx(O4,G2_thread,JavaThread::o_reg_temps_offset_in_bytes()+4*8);
  __ stx(O5,G2_thread,JavaThread::o_reg_temps_offset_in_bytes()+5*8);
#endif

  // NOW push a register window, lopping the heads off of all the LION
  // build's old O-registers.
  __ save(SP,-frame_size,SP);

  // Save the G's
  __ stx(G1,SP,g1_offset+STACK_BIAS);
  __ stx(G3,SP,g3_offset+STACK_BIAS);
  __ stx(G4,SP,g4_offset+STACK_BIAS);
  __ stx(G5,SP,g5_offset+STACK_BIAS);
  // Save the flags
  __ rdccr( G5 );               // Save int flags in TLS
  __ stx(G5,G2_thread,JavaThread::ccr_temp_offset_in_bytes());
  __ stxfsr(SP,fsr_offset+STACK_BIAS);
  // Save all the FP registers
  for( int i=0; i<64; i+=2 )
    __ stf(FloatRegisterImpl::D, as_FloatRegister(i),SP,d00_offset+i*sizeof(float)+STACK_BIAS);

  // Force frame flush so it's visible during debug printing
#ifdef ASSERT
  /*
  __ st_ptr(L0,SP, 0*wordSize+STACK_BIAS);
  __ st_ptr(L1,SP, 1*wordSize+STACK_BIAS);
  __ st_ptr(L2,SP, 2*wordSize+STACK_BIAS);
  __ st_ptr(L3,SP, 3*wordSize+STACK_BIAS);
  __ st_ptr(L4,SP, 4*wordSize+STACK_BIAS);
  __ st_ptr(L5,SP, 5*wordSize+STACK_BIAS);
  __ st_ptr(L6,SP, 6*wordSize+STACK_BIAS);
  __ st_ptr(L7,SP, 7*wordSize+STACK_BIAS);
  __ st_ptr(I0,SP, 8*wordSize+STACK_BIAS);
  __ st_ptr(I1,SP, 9*wordSize+STACK_BIAS);
  __ st_ptr(I2,SP,10*wordSize+STACK_BIAS);
  __ st_ptr(I3,SP,11*wordSize+STACK_BIAS);
  __ st_ptr(I4,SP,12*wordSize+STACK_BIAS);
  __ st_ptr(I5,SP,13*wordSize+STACK_BIAS);
  __ st_ptr(I6,SP,14*wordSize+STACK_BIAS);
  __ st_ptr(I7,SP,15*wordSize+STACK_BIAS);
  */
  // Slather some dirty bits over the argv area
  for( int i=16*wordSize; i<start_of_extra_save_area; i+=wordSize ) {
    __ set(i,G1);
    __ st_ptr(G1,SP,i+STACK_BIAS);
  }
#endif

  // setup last_Java_sp (blows G4)
  __ set_last_Java_frame(SP, G0);

  // call into the runtime to handle illegal instructions exception
  // Do not use call_VM_leaf, because we need to make a GC map at this call site.
  __ mov(G2_thread, O0);
  __ save_thread(L7_thread_cache);
  __ call(CAST_FROM_FN_PTR(address, SafepointSynchronize::handle_illegal_instruction_exception));
  __ delayed()->nop();
  int call_offset = __ offset();
  __ restore_thread(L7_thread_cache);

  // clear last_Java_sp
  __ reset_last_Java_frame();

  // Restore all the FP registers
  for( int i=0; i<64; i+=2 )
    __ ldf(FloatRegisterImpl::D, SP,d00_offset+i*sizeof(float)+STACK_BIAS, as_FloatRegister(i));
  // Restore FP (not INT) flags
  __ ldxfsr(SP,fsr_offset+STACK_BIAS);
  // Restore the G's
  __ ldx(SP,g1_offset+STACK_BIAS,G1);
  __ ldx(SP,g3_offset+STACK_BIAS,G3);
  __ ldx(SP,g4_offset+STACK_BIAS,G4);
  __ ldx(SP,g5_offset+STACK_BIAS,G5);

  // Test the VM's return result.  This makes the int flags live and NOT yet
  // restored.  When we issue a restore, the VM's return result in O0 is
  // blown.
  __ tst(O0);

  // Unwind the register window-save, restoring the 32-bit o's, i's & l's to
  // their former glory.  Only some G's are spare now, along with the
  // head-lopped o's.  O7 once again holds the issuing PC.
  __ restore();

#if !defined(_LP64)
  // Restore the 64-bit O's.  Store down the low bits.
  __ st (O0,G2_thread,JavaThread::o_reg_temps_offset_in_bytes()+0*8+4);
  __ st (O1,G2_thread,JavaThread::o_reg_temps_offset_in_bytes()+1*8+4);
  __ st (O2,G2_thread,JavaThread::o_reg_temps_offset_in_bytes()+2*8+4);
  __ st (O3,G2_thread,JavaThread::o_reg_temps_offset_in_bytes()+3*8+4);
  __ st (O4,G2_thread,JavaThread::o_reg_temps_offset_in_bytes()+4*8+4);
  __ st (O5,G2_thread,JavaThread::o_reg_temps_offset_in_bytes()+5*8+4);
  // Suck up the combined 64-bits except for O0, which is needed as a temp.
  // __ ldx(G2_thread,JavaThread::o_reg_temps_offset_in_bytes()+0*8,O0);
  __ ldx(G2_thread,JavaThread::o_reg_temps_offset_in_bytes()+1*8,O1);
  __ ldx(G2_thread,JavaThread::o_reg_temps_offset_in_bytes()+2*8,O2);
  __ ldx(G2_thread,JavaThread::o_reg_temps_offset_in_bytes()+3*8,O3);
  __ ldx(G2_thread,JavaThread::o_reg_temps_offset_in_bytes()+4*8,O4);
  __ ldx(G2_thread,JavaThread::o_reg_temps_offset_in_bytes()+5*8,O5);
#else
  // Save O0 in thread-local storage for use as a temp
  __ stx(O0, G2_thread,JavaThread::o_reg_temps_offset_in_bytes()+0*8);
#endif

  // All registers now restored except for O0 and int flags.  The 64-bit O0
  // value and caller's int flags are in TLS.  Current int flags hold the VM's
  // return result.  Current O0 will be used as a temp.

  // ------------
  // check for pending exceptions
  __ ld_ptr(G2_thread, in_bytes(Thread::pending_exception_offset()), O0);
  // Even if !_LP64, it is OK to look at all 64 bits of register contents,
  // because the ld_ptr instruction (lduw in this case) zero-extends to 64 bits.
  // The BPR lets us keep the VM's return result live in the int flags.
  __ bpr(Assembler::rc_z, false, Assembler::pt, O0, no_pending_exception_L);
  __ delayed()->ldx(G2_thread,JavaThread::ccr_temp_offset_in_bytes(),O0);

  // Now that we know we have a pending exception, the VM's return result is
  // dead and we can restore the caller's int flags.
  __ wrccr(O0);

  // Tail-call forward_exception_entry, with the issuing PC in O7,
  // so it looks like the original nmethod called forward_exception_entry.
  __ set((intptr_t)StubRoutines::forward_exception_entry(), O0);
  __ jmpl(O0, G0, G0);
  // Restore O0 from TLS on the way out.  Note that I cannot use a restore
  // here in the delay slot because it will chop the heads off of my 64-bit 
  // %o registers in the 32-bit build.
  __ delayed()->ldx(G2_thread,JavaThread::o_reg_temps_offset_in_bytes(),O0);


  // ------------
  __ bind(no_pending_exception_L);
  // Finally use those int flags kept alive so long!
  __ br(Assembler::negative, false, Assembler::pn, deoptimizing_L);
  // We just tested the int flags, and the deopt-flag is now dead.
  // Recover the old int flags
  __ delayed()->wrccr( O0 );    // Restore int flags

  // Return to the original nmethod, as identified by the O7 value
  __ retl();
  __ delayed()->ldx(G2_thread,JavaThread::o_reg_temps_offset_in_bytes(),O0);


  // ------------
  // Deoptimization happened at a compiled safepoint at a call; jump to
  // destination instead of skipping the call in the blocked method; we do not
  // have debug information to reexecute the call
  __ bind(deoptimizing_L);

  __ mov(O7, O0);               // destination of the call
  DeoptimizationBlob* deopt_blob = OptoRuntime::deoptimization_blob();
  assert(deopt_blob != NULL, "deoptimization blob must have been created");
  __ load_address(Address(O7, deopt_blob->unpack() -  frame::pc_return_offset));
  __ jmpl(O0, frame::pc_return_offset, G0);
  // Recover faithful ol' O0 on the way out
  __ delayed()->ldx(G2_thread,JavaThread::o_reg_temps_offset_in_bytes(),O0);

  // Set an oopmap for the call site.
  // We need this not only for callee-saved registers, but also for volatile
  // registers that the compiler might be keeping live across a safepoint.
  OopMapSet *oop_maps = new OopMapSet();
  OopMap* map =  new OopMap( frame_size, 0 );

#define CALLEE(off,num) map->set_callee_saved(SharedInfo::stack2reg((off)>>2), frame_size, 0, OptoReg::Name(num))
  CALLEE(g1_offset+0,R_G1H_num);
  CALLEE(g1_offset+4,R_G1_num );
  CALLEE(g3_offset+0,R_G3H_num);
  CALLEE(g3_offset+4,R_G3_num );
  CALLEE(g4_offset+0,R_G4H_num);
  CALLEE(g4_offset+4,R_G4_num );
  CALLEE(g5_offset+0,R_G5H_num);
  CALLEE(g5_offset+4,R_G5_num );

  static const MachRegisterNumbers F_Reg[64] = { 
    // First OopMap for float registers
    R_F0_num,R_F1_num,R_F2_num,R_F3_num,
    R_F4_num,R_F5_num,R_F6_num,R_F7_num, 
    R_F8_num,R_F9_num,R_F10_num,R_F11_num,
    R_F12_num,R_F13_num,R_F14_num,R_F15_num, 
    R_F16_num,R_F17_num,R_F18_num,R_F19_num,
    R_F20_num,R_F21_num,R_F22_num,R_F23_num, 
    R_F24_num,R_F25_num,R_F26_num,R_F27_num,
    R_F28_num,R_F29_num,R_F30_num,R_F31_num,
    // Additional v9 double registers
    R_D32_num,R_D32x_num,
    R_D34_num,R_D34x_num,
    R_D36_num,R_D36x_num,
    R_D38_num,R_D38x_num,
    R_D40_num,R_D40x_num,
    R_D42_num,R_D42x_num,
    R_D44_num,R_D44x_num,
    R_D46_num,R_D46x_num,
    R_D48_num,R_D48x_num,
    R_D50_num,R_D50x_num,
    R_D52_num,R_D52x_num,
    R_D54_num,R_D54x_num,
    R_D56_num,R_D56x_num,
    R_D58_num,R_D58x_num,
    R_D60_num,R_D60x_num,
    R_D62_num,R_D62x_num
  };
  for( int i=0; i<64; i++ ) {
    CALLEE(d00_offset+i*sizeof(float), F_Reg[i]);
  }
#undef CALLEE

  oop_maps->add_gc_map( call_offset - start, true, map);

  // -------------
  // make sure all code is generated
  masm->flush();  

  // Set exception blob
  _illegal_instruction_handler_blob = SafepointBlob::create(buffer, oop_maps, frame_size);
}


//------------------------------exception_address_is_unpack_entry--------------
bool DeoptimizationBlob::exception_address_is_unpack_entry(address pc) const {
  address unpack_pc = OptoRuntime::deoptimization_blob()->unpack();
  return (pc == unpack_pc ||
          (pc + frame::pc_return_offset) == unpack_pc);
}


void OptoRuntime::pd_unwind_stack(JavaThread *thread, frame fr, RegisterMap* reg_map) {
}

//
// Generate the on-stack replacement stub, that is used to replace the
// interpreter frame
//
OSRAdapter* OptoRuntime::generate_osr_blob(int frame_size, bool returning_fp) {
  ResourceMark rm;
  
  // setup code generation tools
  CodeBuffer*       cb = new CodeBuffer(128, 128, 0, 0, 0, false);
  MacroAssembler* masm = new MacroAssembler(cb);    
  
  OopMapSet *oop_maps = new OopMapSet();
  OopMap* map =  new OopMap(frame_size, 0 );
  oop_maps->add_gc_map(0, true, map);
  
  __ mov(O0,I0);
  __ ret();
  __ delayed()->restore(O1,G0,O1);
  
  return OSRAdapter::new_osr_adapter(cb, oop_maps, frame_size);
}

//Reconciliation History
// 1.2 98/06/11 10:29:32 runtime_i486.cpp
// 1.3 98/06/25 16:44:37 runtime_i486.cpp
// 1.6 98/07/24 15:30:03 runtime_i486.cpp
// 1.7 98/08/06 11:43:15 runtime_i486.cpp
// 1.8 98/08/17 10:45:15 runtime_i486.cpp
// 1.12 98/08/28 11:30:59 runtime_i486.cpp
// 1.13 98/09/02 12:58:19 runtime_i486.cpp
// 1.14 98/09/03 20:05:49 runtime_i486.cpp
// 1.23 98/10/02 12:37:40 runtime_i486.cpp
// 1.26 98/10/06 16:50:52 runtime_i486.cpp
// 1.28 98/10/07 11:36:46 runtime_i486.cpp
// 1.30 98/10/09 17:17:32 runtime_i486.cpp
// 1.32 98/10/15 12:51:04 runtime_i486.cpp
// 1.33 98/10/27 13:56:13 runtime_i486.cpp
// 1.35 98/11/09 09:26:27 runtime_i486.cpp
// 1.36 98/11/13 14:04:16 runtime_i486.cpp
// 1.37 99/01/06 16:18:57 runtime_i486.cpp
// 1.40 99/01/21 16:59:49 runtime_i486.cpp
// 1.44 99/05/26 19:34:52 runtime_i486.cpp
// 1.47 99/06/24 14:24:51 runtime_i486.cpp
// 1.42 99/03/16 10:37:21 runtime_i486.cpp
// 1.43 99/04/13 11:57:13 runtime_i486.cpp
// 1.50 99/08/13 17:48:48 runtime_i486.cpp
// 1.52 99/08/26 16:29:56 runtime_i486.cpp
//End

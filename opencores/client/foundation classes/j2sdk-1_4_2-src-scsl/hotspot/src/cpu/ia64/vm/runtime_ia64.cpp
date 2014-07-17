#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)runtime_ia64.cpp	1.16 03/03/31 12:16:34 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_runtime_ia64.cpp.incl"

// Deoptimization

#define __ masm.

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
#if 0

//---------------------------------RegisterSaver-------------------------------
class RegisterSaver {
 private:
  // the offsets (or -1) where the registers are spilled
  int _saved_reg_set[_last_Mach_Reg];

  int _first_saved_reg_offset;
  int _limit_saved_reg_offset;

  MacroAssembler* masm;

  void move_registers(bool is_save);

 public:
  RegisterSaver(MacroAssembler* masm_, bool (*save_this_register)(int reg)) {
    masm = masm_;

    // evaluate the predicate
    for (int reg = 0; reg < _last_Mach_Reg; reg++) {
      _saved_reg_set[reg] = (int) save_this_register(reg);
    }
  }

  int allocate_offsets(int first_saved_reg_offset) {
    _first_saved_reg_offset = first_saved_reg_offset;
    int saved_reg_offset    = first_saved_reg_offset;
    for (int reg = 0; reg < _last_Mach_Reg; reg++) {
      if ((bool) _saved_reg_set[reg]) {
	// align, patch, and increment the offset
	int reg_size = regSize[reg];
	assert(reg_size == 1 || reg_size == 2, "sane regSize");
	saved_reg_offset = align_size_up(saved_reg_offset, reg_size);
	_saved_reg_set[reg] = saved_reg_offset;
	saved_reg_offset += reg_size;
      } else {
	_saved_reg_set[reg] = -1;
      }
    }
    _limit_saved_reg_offset = saved_reg_offset;
    return _limit_saved_reg_offset;
  }

  int num_saved_reg_words() {
    return _limit_saved_reg_offset - _first_saved_reg_offset;
  }

  void save_registers() {
    move_registers(true);
  }

  void restore_registers() {
    move_registers(false);
  }

  void set_callee_saved(OopMap* map, int framesize) {
    for (int reg = 0; reg < _last_Mach_Reg; reg++) {
      int sp_offset = _saved_reg_set[reg];
      if (sp_offset >= 0) {
	map->set_callee_saved(SharedInfo::stack2reg(sp_offset), framesize, 0, OptoReg::Name(reg));
      }
    }
  }
};

void RegisterSaver::move_registers(bool is_save) {
  // save or restore the saved registers
  // (no scratch register worries:  we have all the %o and %l to play with)

#ifdef ASSERT
  Register      ijunk = noreg;
  FloatRegister fjunk = fnoreg;
#endif

  // save the selected registers
  for (int reg = 0; reg < _last_Mach_Reg; reg++) {
    int sp_offset_in_bytes = _saved_reg_set[reg] * wordSize;
    if (sp_offset_in_bytes >= 0) {
      switch (register_save_type[reg]) {
      case Op_RegI:
	{
	  Register ireg = as_Register((int)Matcher::_regEncode[reg]);
	  if (is_save) {
	    __ st_ptr(ireg->after_save(), SP, sp_offset_in_bytes + STACK_BIAS);
	    #ifdef ASSERT
	    // deliberately trash the register
	    if (!ijunk->is_valid()) {
	      __ set(0xbaadd00dbaadd00d, ireg->after_save());
	      ijunk = ireg;
	    } else {
	      __ mov(ijunk, ireg->after_save());
	    }
	    #endif
	  } else {
	    __ ld_ptr(SP, sp_offset_in_bytes + STACK_BIAS, ireg->after_save());
	  }
	  break;
	}

      case Op_RegF:
	{
	  FloatRegister freg = as_SingleFloatRegister((int)Matcher::_regEncode[reg]);
	  if (is_save) {
	    __ stf(FloatRegisterImpl::S, freg, SP, sp_offset_in_bytes + STACK_BIAS);
	    #ifdef ASSERT
	    // deliberately trash the register
	    if (!fjunk->is_valid()) {
	      __ st(ijunk, FP, -4 + STACK_BIAS);
	      __ ldf(FloatRegisterImpl::S, FP, -4 + STACK_BIAS, freg);
	      fjunk = freg;
	    } else {
	      __ fmov(FloatRegisterImpl::S, fjunk, freg);
	    }
	    #endif
	  } else {
	    __ ldf(FloatRegisterImpl::S, SP, sp_offset_in_bytes + STACK_BIAS, freg);
	  }
	  break;
	}

      case Op_RegD:
	{
	  FloatRegister dreg = as_DoubleFloatRegister((int)Matcher::_regEncode[reg]);
	  if (is_save) {
	    __ stf(FloatRegisterImpl::D, dreg, SP, sp_offset_in_bytes + STACK_BIAS);
	  } else {
	    __ ldf(FloatRegisterImpl::D, SP, sp_offset_in_bytes + STACK_BIAS, dreg);
	  }
	  break;
	}

      case Op_RegFlags:
        if( !is_save )		// Load old flags into O0
          __ ld_ptr( SP, sp_offset_in_bytes + STACK_BIAS, O0 );
        if( !strcmp( SharedInfo::regName[reg], "CCR" ) ) { // Check for 'CCR' vs 'FCC'
          if( is_save ) __ rdccr ( O0 ); // Read CCR's and save
          else          __ wrccr ( O0 ); // Restore CCR's
        } else if( !strcmp( SharedInfo::regName[reg], "FCC0" ) ) {
          if( is_save ) __ rdfprs( O0 ); // Read FCC0's and save
          else          __ wrfprs( O0 ); // Restore FCC0's
        } else if( !strcmp( SharedInfo::regName[reg], "FCC1" ) ) {
          if( is_save ) {
            __ rdfprs( O0 );	// Read all FCC's
            __ srlx( O0, 32, O0 ); // Move high-order bits down low
          } else {
            __ rdfprs( O1 );	// Read all FCC0 into O1
            __ srl( O1, 0, O1 ); // Zero FCC1-3 bits
            __ sllx( O0, 32, O0 ); // Move old FCC1-3 bits up hi
            __ or3( O0, O1, O0 ); // Combine
            __ wrfprs( O0 ); // Restore ALL FCC's
          }
        } else {		// Save FCC2-FCC3
          // FCC2 and FCC3 are saved along with FCC1 above
        }
        if( is_save )		// Save flags into stack
          __ st_ptr( O0, SP, sp_offset_in_bytes + STACK_BIAS);
	break;

      default:
	ShouldNotReachHere();
      }
    }
  }
}


// predicate for RegisterSaver constructor
// selects all registers which are:
//  - not saved in any register window
//  - used by compiled Java code
//  - not the TLS register
// This amounts to 4 %g registers and all the %f/%d registers
static bool all_volatile_registers(int reg) {
  // special cases for particular registers:
  switch (reg) {
  case R_SP_num:
  case R_FP_num:
  case R_I7_num:
  case R_G2_num:  // handled by a different means, below
    // do not preserve these; the stack walker needs them to stay put
    return false;
  }

  // weed out %i/%l/%o registers and %d<odd> pseudo-regs
  // (Note:  we might be saving %o registers, because they can be SOE.)
  switch (register_save_type[reg]) {
  case Op_RegI:
    {
      assert(regSize[reg] == 1, "just checking");
      int enc = (int)Matcher::_regEncode[reg];
      if( enc >= 128 ) return false; // Sparc.ad uses 255 for high-half of int regs
      Register ireg = as_Register(enc);
      if (ireg->is_in() || ireg->is_local() || ireg->is_out())  return false;
      break;
    }

  case Op_RegF:
    assert(regSize[reg] == 1, "just checking");
    break;

  case Op_RegD:
    assert(regSize[reg] == 2, "just checking");
    if (is_odd(reg)) {
      // ignore the high-half registers
      assert(Matcher::_regEncode[reg] == 255, "sparc.ad uses 255 for uncodeable regs");
      return false;
    }
    if (VM_Version::v8_instructions_work()) { // No upper 32 float registers on V8
      return false;
    }
    break;
  case Op_RegFlags:
    return true;
  default:
    ShouldNotReachHere();
  }

  // weed out "always-save" registers
  switch (register_save_policy[reg]) {
  case 'E': // save on entry (callee-saved)
  case 'C': // save on call (caller-saved)
  case 'A': // always save
    // Because we are forcing a control transfer to happen in a place
    // where calls do not ordinarily occur, we must save all registers
    // except %i and %l registers, and certain "don't touch" registers
    // like %g7.
    return true;

  case 'N': // not saved
    return false;
  }

  ShouldNotReachHere();
  return false;
}
#endif


//------------------------------setup_exception_blob---------------------------
// creates exception blob at the end
// Using exception blob, this code is jumped from a compiled method.
// (see emit_exception_handler in ia64.ad file)
// It is also the default exception destination for c2i and i2c adapters
// 
// Given an exception pc at a call, we may unwind the stack and jump to
// caller's exception handler. We may also continue into the handler
// This code is entered with a jmp.
// 
// Arguments:
//   GR_exception: exception oop (R8)
//   GR_issuing_pc: exception pc (R9)
//
// Results:
//   GR_exception: exception oop
//   O1: exception pc in caller or ??? QQQ
//   destination: exception handler of caller
// 
// Note: the exception pc MUST be at a call (precise debug information)
//
// NOTE: NO Registers except the above mention argument/results can be modified.
//
void OptoRuntime::setup_exception_blob() {
  // allocate space for code
  ResourceMark rm;

  // setup code generation tools
  CodeBuffer*    buffer   = new CodeBuffer(4096, 512, 0, 0, 0, false);
  MacroAssembler masm(buffer);

  Label exception_handler_found;
  Label L;

  address start = __ pc();

//  __ verify_thread();

  // We enter using the window of the compiled code that has just
  // generated an exception. We have just completed a call so any
  // SOC registers are fair game as temps here (just like other
  // platforms)

  // This dummy frame must store the link to the "caller" 
  // QQQ dependent on calling conventions.

  __ push_dummy_full_frame(GR9_issuing_pc); // Get us a new window

  const Register exception_oop_addr        = GR31_SCRATCH;
  const Register exception_pc_addr         = GR30_SCRATCH;
 
  __ add(exception_oop_addr, GR4_thread, in_bytes(JavaThread::exception_oop_offset()));
  __ add(exception_pc_addr,  GR4_thread, in_bytes(JavaThread::exception_pc_offset()));

  __ st8(exception_oop_addr, GR8_exception);
  __ st8(exception_pc_addr, GR9_issuing_pc);

  // This call does all the hard work. It checks if an exception handler
  // exists in the method.  
  // If so, it returns the handler address.
  // If not, it prepares for stack-unwinding, restoring the callee-save 
  // registers of the frame being removed.
  //

  handle_exception_call_pc_offset = __ set_last_Java_frame(SP);

  // Ought to be just a simple call

  __ call_VM_leaf(CAST_FROM_FN_PTR(address, OptoRuntime::handle_exception_C), GR4_thread);
  __ flush_bundle();

  // Reload GR7 which holds the register stack limit. 
  // We might be processing a register stack overflow and raised
  // the limit while we handle the exception.  If we've unwound
  // enough, the reguard_stack routine will set the limit back to
  // normal.  This is where we update the global register.
  __ add(GR7_reg_stack_limit, thread_(register_stack_limit));
  __ ld8(GR7_reg_stack_limit, GR7_reg_stack_limit);

  //  handle_exception_call_pc_offset = __ pc() - start;

  __ bind(L);
  __ reset_last_Java_frame();

  // We are still in our new frame
  __ pop_dummy_full_frame();

  // Back to original frame. Only use SOC registers here as scratch.

  __ cmp(PR6, PR0, GR_RET, GR0, Assembler::notEqual);
  __ br(PR6, exception_handler_found, Assembler::spnt);

  // No handler found in this frame
  // This is incomplete for returning to a deoptimized frame
  //
  __ add(exception_oop_addr, GR4_thread, in_bytes(JavaThread::exception_oop_offset()));
  __ add(exception_pc_addr,  GR4_thread, in_bytes(JavaThread::exception_pc_offset()));

  __ ld8(GR8_exception, exception_oop_addr);
  __ ld8(GR9_issuing_pc, exception_pc_addr);

  // No handler was found for the current frame so we will pop this frame
  // and try again at thread->exception_handler_pc() assuming the original
  // address we were returning to was not deoptimized frame. If it was
  // a deoptimized frame then we return to the deopt blob pending exception
  // entry instead.

  __ flush_bundle();
  handle_exception_deopt_offset = __ pc() - start;

 const Register deopt_addr               = GR30_SCRATCH;
 const Register exception_handler_pc_addr = exception_oop_addr;

  __ movl(deopt_addr, (intptr_t)-1);    // will be filled in later by fill_in_exception_blob
  __ cmp(PR6, PR7, deopt_addr, GR_Lsave_RP, Assembler::notEqual);
  // PR6 == not deopting, PR7 == deopting
  __ add(PR6, exception_handler_pc_addr, GR4_thread, in_bytes(JavaThread::exception_handler_pc_offset()));
  __ ld8(PR6, GR_Lsave_RP, exception_handler_pc_addr);

  __ flush_bundle();
  handle_exception_deopt_exception_offset = __ pc() - start;

  __ movl(PR7, GR_Lsave_RP, (intptr_t)-1);    // will be filled in later by fill_in_exception_blob

  __ pop_thin_frame();
  __ ret();

  // Exception handler found.
  __ bind(exception_handler_found);

  // check if current frame has been deoptimized; if yes, exception handler must be
  // deoptimized blob
  // address to check is in Oissuing_pc
  handle_exception_deopt_offset_2 = __ pc() - start;

  __ movl(deopt_addr, (intptr_t)-1);    // will be filled in later by fill_in_exception_blob
  __ flush_bundle();

  // The push_dummy frame that called hadle_exception placed the issuing pc as the
  // return address in the dummy frame. If the current frame was deopt then that
  // value was changed. The pop dummy frame that we did after the call has the
  // issuing pc (or now the deopt blob) in RP. Check to see if the current
  // frame was deopted 
  __ mov(GR2_SCRATCH, RP);
  __ cmp(PR6, PR0, deopt_addr, GR2_SCRATCH, Assembler::equal); // was O7
  __ flush_bundle();

  handle_exception_deopt_exception_offset_2 = __ pc() - start;
  __ movl(PR6, GR_RET, (intptr_t)-1);    // will be filled in later by fill_in_exception_blob

  // GR_RET contains handler address (or deopt exception unpack blob)

  const BranchRegister handler_br = BR6_SCRATCH;

  __ mov(handler_br, GR_RET);

  __ add(exception_oop_addr, GR4_thread, in_bytes(JavaThread::exception_oop_offset()));
  __ add(exception_pc_addr,  GR4_thread, in_bytes(JavaThread::exception_pc_offset()));

  __ ld8(GR8_exception, exception_oop_addr);
  __ ld8(GR9_issuing_pc, exception_pc_addr);

#ifdef ASSERT
  __ st8(exception_oop_addr, GR0);
  const Register exception_handler_pc_addr2 = exception_oop_addr;
 
  __ add(exception_handler_pc_addr2, GR4_thread, in_bytes(JavaThread::exception_handler_pc_offset()));

  __ st8(exception_handler_pc_addr, GR0);
  __ st8(exception_pc_addr, GR0);
#endif

  __ br(handler_br);

  // -------------
  // make sure all code is generated

  __ flush();

  // Fill out other meta info
  OopMapSet *oop_maps = NULL; // will be set later; currently the register stuff is not yet initialized!
  // _exception_blob = ExceptionBlob::create(buffer, oop_maps,  __ total_frame_size_in_bytes(0)/wordSize);
  _exception_blob = ExceptionBlob::create(buffer, oop_maps,  0);
  address xxxyyy = _exception_blob->instructions_begin();
}

//------------------------------fill_in_exception_blob-------------------------
void OptoRuntime::fill_in_exception_blob() {
  int framesize;
  { ResourceMark rm;
    MacroAssembler masm(new CodeBuffer(NULL, NULL));
//    framesize = __ total_frame_size_in_bytes(exception_blob_words)/wordSize;
    framesize = 0;
  }

   // Patch blob
  assert(handle_exception_stub() != NULL, "exception stub must have been generated");
  assert(handle_exception_call_pc_offset != 0, "");

  assert(handle_exception_deopt_exception_offset != 0, "");
  assert(handle_exception_deopt_offset != 0, "");
  address e_blob = exception_blob()->instructions_begin();
  {
    NativeMovConstReg* move = nativeMovConstReg_at(e_blob + handle_exception_deopt_offset);
    move->set_data((intptr_t)OptoRuntime::deoptimization_blob()->unpack());

    NativeMovConstReg* move2 = nativeMovConstReg_at(e_blob + handle_exception_deopt_exception_offset);
    move2->set_data((intptr_t)OptoRuntime::deoptimization_blob()->unpack_with_exception());
  }

  assert(handle_exception_deopt_exception_offset_2 != 0, "");
  assert(handle_exception_deopt_offset_2 != 0, "");
  {
    NativeMovConstReg* move = nativeMovConstReg_at(e_blob + handle_exception_deopt_offset_2);
    move->set_data((intptr_t)OptoRuntime::deoptimization_blob()->unpack());

    NativeMovConstReg* move2 = nativeMovConstReg_at(e_blob + handle_exception_deopt_exception_offset_2);
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

//------------------------------generate_deopt_blob----------------------------
// Ought to generate an ideal graph & compile, but here's some IA64 ASM
// instead.
void OptoRuntime::generate_deopt_blob() {
  // allocate space for the code
  ResourceMark rm;
  // setup code generation tools
  CodeBuffer*     buffer             = new CodeBuffer(4096, 512, 0, 0, 0, false);
  MacroAssembler  masm(buffer);
  Register        GR10_exception_tmp     = GR10;
  Label           cont, loop, skip_i2c;

  OopMapSet *oop_maps = new OopMapSet();

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

  address start = __ pc();

  // space to save live return results and abi scratch area
  int framesize = (round_to(sizeof(double)*2, 16) + round_to(sizeof(jlong)*2, 16))/wordSize + 2;

  // How does this oopmap work???  This oopmap exists because the pc is stashed on
  // the stack and a gc might see it. However how in the world does it use these
  // oopmaps when it really ought to be using the original oopmap for the original
  // return address. Surely there is cross tracking to the original oopmap somehow
  // but then what would the point of these oopmaps be? I'm confused... QQQ

  // Add empty oopmap at beginning. This can both be at_call and not_at_call,
  // Normally it would be at call. However, it could be at not_at_call, if the
  // thread happended to be blocked at safepoint_node in the frame that got
  // deoptimized.
  oop_maps->add_gc_map( 0, true, new OopMap( framesize, 0) );
  oop_maps->add_gc_map( 0, false, new OopMap( framesize, 0) );

  __ mov(GR10_exception_tmp, Deoptimization::Unpack_deopt);
  __ br(cont);
  __ flush_bundle();

  int exception_offset = __ pc() - start;
  __ mov(GR10_exception_tmp, Deoptimization::Unpack_exception);

  __ bind(cont);
  //
  // frames: deoptee, possible_i2c, caller_of_deoptee

  // push a dummy "unpack_frame" taking care to preserve float return values
  // and call Deoptimization::fetch_unroll_info to get the UnrollBlock
  
  const Register dummy  = GR3_SCRATCH;

  __ mov(dummy, RP);
  __ push_dummy_full_frame(dummy);

  // frames: unpack frame, deoptee, possible_i2c, caller_of_deoptee

  __ mov(GR_L8, GR10_exception_tmp);                    // Save this where it is safe (istate)
  __ sub(SP, SP, round_to(sizeof(double)*2, 16));
  __ stfspill(SP, FR_RET);
  __ sub(SP, SP, round_to(sizeof(jlong)*2, 16));
  __ st8spill(SP, GR_RET);
  __ sub(SP, SP, 2*wordSize);				// ABI scratch

  // What about things like LC?  Saved/restored by push/pop_dummy_full_frame

  // Local register assignment

  const Register GR15_frame_sizes  = GR15;   // first the address, then the value
  const Register GR16_frame_count  = GR16;   // first the address, then the value
  const Register GR17_adapter_size = GR17;   // first the address, then the value
  const Register GR18_frame_pcs    = GR18;   // first the address, then the value

  // We need to set last_Java_frame because fetch_unroll_info will call last_Java_frame()
  // However we can't block and no GC will occur so we don't need an oopmap and the
  // value of the pc in the frame is not particularly important (just identify the blob)

  __ set_last_Java_frame(SP);
  __ call_VM_leaf(CAST_FROM_FN_PTR(address, Deoptimization::fetch_unroll_info), GR4_thread);
  __ reset_last_Java_frame();

  __ add(GR17_adapter_size, GR_RET /* info */, Deoptimization::UnrollBlock::adapter_size_offset_in_bytes());
  __ add(GR18_frame_pcs,    GR_RET /* info */, Deoptimization::UnrollBlock::frame_pcs_offset_in_bytes());
  __ add(GR15_frame_sizes,  GR_RET /* info */, Deoptimization::UnrollBlock::frame_sizes_offset_in_bytes());
  __ add(GR16_frame_count,  GR_RET /* info */, Deoptimization::UnrollBlock::number_of_frames_offset_in_bytes());

  __ mov(GR10_exception_tmp, GR_L8);                    // and restore to where we keep it
  __ add(SP, SP, 2*wordSize);				// ABI area
  __ ld8fill(GR_RET, SP);
  __ add(SP, SP, round_to(sizeof(jlong)*2, 16));
  __ ldffill(FR_RET, SP);
  __ add(SP, SP, round_to(sizeof(double)*2, 16));

  __ pop_dummy_full_frame();

  // frames: deoptee, possible_i2c, caller_of_deoptee

  __ ld4(GR17_adapter_size, GR17_adapter_size);
  __ ld8(GR15_frame_sizes,  GR15_frame_sizes);
  __ ld8(GR18_frame_pcs,    GR18_frame_pcs);
  __ ld4(GR16_frame_count,  GR16_frame_count);

  // Now we must pop the frame we are deoptimizing

  __ pop_dummy_thin_frame();  // Not really a dummy, but junk now.

  // frames: possible_i2c, caller_of_deoptee

  // We now have in LC and UNAT the values that we there on entry to the deoptimized frame
  // We will propagate them to all the frames we create only the original caller of
  // the deoptimized frame will care (and if that caller is an i2c we will pop it and
  // used its LC and UNAT instead).
  // Seems like things like r4-r7 are ok because Java never touches them so they should
  // already be saved/restored by non-Java code and their values here should be correct
  // already.

  __ cmp4(PR6, PR0, 0, GR17_adapter_size, Assembler::equal);
  __ br(PR6, skip_i2c, Assembler::dptk);
  __ pop_dummy_thin_frame();  // Pop the adapter frame

  __ bind(skip_i2c);

  // frames: caller_of_deoptee

  // loop through the UnrollBlock info and create interpreter frames
  // Everything must be done in global registers because the windows
  // will be changing constantly

  #ifdef ASSERT
  // make sure that there is at least one entry in the array
  __ cmp4(PR6, PR0, 0, GR16_frame_count, Assembler::equal);
  __ breaki(PR6, 1);
  #endif

  // QQQ what about adjusting the caller's stack and getting the sender_sp correct!!!

  // We could use LC but then we'd have to shuffle it to save the preserved value.
  // Doesn't seem to be worth the bother

  const Register GR19_pc  = GR19;

  __ bind(loop);

  __ ld8(GR2, GR15_frame_sizes, sizeof(intptr_t));      // Get size of next memory frame to allocate
  __ ld8(GR19_pc, GR18_frame_pcs, sizeof(address));     // Get size of next return address to store
  __ sub(GR16_frame_count, GR16_frame_count, 1);

  __ push_dummy_full_frame(GR19_pc);                     // Allocate register stack side of the frame

  #ifdef ASSERT
    // trash local registers to show a clear pattern in backtraces
    // __ mov(GR_L0, 0xDEAD00); GR_Lsave_SP
    // __ mov(GR_L1, 0xDEAD01); GR_Lsave_BSP
    // __ mov(GR_L2, 0xDEAD02); GR_Lsave_PFS
    // __ mov(GR_L3, 0xDEAD03); GR_Lsave_RP
    // __ mov(GR_L4, 0xDEAD04); GR_Lsave_LC
    // __ mov(GR_L5, 0xDEAD05); GR_Lsave_GP
    // __ mov(GR_L6, 0xDEAD06); GR_Lsave_UNAT
    // __ mov(GR_L7, 0xDEAD07); GR_Lsave_PR
    __ mov(GR_L8, 0xDEAD08);  //GR_Lstate
    __ mov(GR_L9, 0xDEAD09);  //GR_Lscratch0
    __ mov(GR_L10, 0xDEAD10); //GR_Lmethod_addr
    __ mov(GR_L11, 0xDEAD11);
    __ mov(GR_L12, 0xDEAD12);
    __ mov(GR_L13, 0xDEAD13);
    __ mov(GR_L14, 0xDEAD14);
    __ mov(GR_L15, 0xDEAD15);

  #endif
  __ sub(SP, SP, GR2);                                  // Allocate memory stack side of the frame

  __ cmp4(PR0, PR6, 0, GR16_frame_count, Assembler::equal);
  __ br(PR6, loop, Assembler::dptk);

  // frames: new skeletal interpreter frame(s), possible_c2_i,  caller_of_deoptee

  // push a dummy "unpack_frame" taking care of float/int return values and
  // call Deoptimization::unpack_frames to have the unpacker layout
  // information in the interpreter frames just created and then return
  // to the interpreter entry point

  __ ld8(GR19_pc, GR18_frame_pcs, sizeof(address));     // Get return address we want to store in the new frame

  __ push_dummy_full_frame(GR19_pc);                     // this will be a return to the frame manager (somewhere)

  // frames: unpack_frame, new skeletal interpreter frame(s), possible_c2_i,  caller_of_deoptee

  // Flush the windows to the stack. For Itanium2 we will have to also turn off eager filling
  // so that the windows don't get filled while we're in the middle of changing them!

  // We must be sure that the current windows flush so flushrs inline
  // will not be sufficient. The fact that the current window refills
  // is not important since we just want to be certain that the
  // stack is walkable and it will be.

  __ sub(SP, SP, round_to(sizeof(double)*2, 16));
  __ stfspill(SP, FR_RET);
  __ sub(SP, SP, round_to(sizeof(jlong)*2, 16));
  __ st8spill(SP, GR_RET);
  __ sub(SP, SP, 2*wordSize);				// ABI

  __ set_last_Java_frame(SP);
  __ call_VM_leaf((address)StubRoutines::ia64::flush_register_stack());

  __ call_VM_leaf(CAST_FROM_FN_PTR(address, Deoptimization::unpack_frames), GR4_thread, GR10_exception_tmp);
  __ reset_last_Java_frame();

  __ add(SP, SP, 2*wordSize);				// ABI
  __ ld8fill(GR_RET, SP);
  __ add(SP, SP, round_to(sizeof(jlong)*2, 16));
  __ ldffill(FR_RET, SP);
  __ add(SP, SP, round_to(sizeof(double)*2, 16));

  // We should be in the newly constructed top frame of the interpreter (frame_manager)

  __ pop_full_frame();

  // frames: new skeletal interpreter frame(s), possible_c2_i,  caller_of_deoptee
  __ ret();

  __ flush();
  _deopt_blob = DeoptimizationBlob::create(buffer, oop_maps, 0, exception_offset, 0, framesize);
}


//------------------------------generate_uncommon_trap_blob--------------------
// Ought to generate an ideal graph & compile, but here's some IA64 ASM
// instead.
void OptoRuntime::generate_uncommon_trap_blob() {
  // allocate space for the code
  ResourceMark rm;
  // setup code generation tools
  CodeBuffer*     buffer             = new CodeBuffer(2048, 512, 0, 0, 0, false);
  MacroAssembler  masm(buffer);
  Label           cont, loop, skip_i2c;

  const Register GR15_klass_index     = GR15;
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

  enum frame_layout {
    F8_off,
    F8_nat_off,
    framesize
  };

  // the unloaded class index is in O0 (first parameter to this blob)

  // push a dummy "unpack_frame"
  // and call Deoptimization::uncommon_trap to pack the compiled frame into
  // vframe array and return the UnrollBlock information

  // push a dummy "unpack_frame" taking care to preserve float return values
  // and call Deoptimization::fetch_unroll_info to get the UnrollBlock

  // Weird on sparc this frame isn't really a dummy at all

  
  const Register dummy  = GR3_SCRATCH;

  // This is lame but true. We need to allocate space on the stack so that deoptimization
  // will walk passed this frame and find the stack space above this one because
  // the code for deopt does a while loop looking for an sp that matches and then expects
  // the pc to be for the frame we want deoptimized. Since on ia64 we can have a frame
  // that uses no stack space (but register stack space) the code fails. Simplest thing
  // is to just allocate some dummy space and go with the flow.

  __ mov(dummy, RP);            // Make the return address be the sender
  __ save_full_frame(framesize * sizeof(intptr_t));

  // Spill the live registers

  __ stfspill(SP, FR8);		// ACK QQQ in ABI area...

  // Set an oopmap for the call site
  OopMapSet *oop_maps = new OopMapSet();
  OopMap* map =  new OopMap( framesize, 0 );

  int offset;
  offset = __ set_last_Java_frame(SP);

  __ mov(GR15_klass_index, GR_I0);
  __ call_VM_leaf(CAST_FROM_FN_PTR(address, Deoptimization::uncommon_trap), GR4_thread, GR15_klass_index);
  __ reset_last_Java_frame();

  // Add an oopmap that describes the above call

  map->set_callee_saved( SharedInfo::stack2reg(F8_off   ), framesize,0, OptoReg::Name(FR8_num  ) );
  oop_maps->add_gc_map(offset, true, map);

  // Local register assignment

  const Register GR15_frame_sizes   = GR15;   // first the address, then the value
  const Register GR16_frame_count   = GR16;   // first the address, then the value
  const Register GR17_adapter_size  = GR17;   // first the address, then the value
  const Register GR18_frame_pcs     = GR18;   // first the address, then the value

  __ add(GR17_adapter_size, GR_RET /* info */, Deoptimization::UnrollBlock::adapter_size_offset_in_bytes());
  __ add(GR15_frame_sizes,  GR_RET /* info */, Deoptimization::UnrollBlock::frame_sizes_offset_in_bytes());
  __ add(GR18_frame_pcs,    GR_RET /* info */, Deoptimization::UnrollBlock::frame_pcs_offset_in_bytes());
  __ add(GR16_frame_count,  GR_RET /* info */, Deoptimization::UnrollBlock::number_of_frames_offset_in_bytes());
  __ pop_dummy_full_frame();
  // our frame is gone now.

  __ ld4(GR17_adapter_size, GR17_adapter_size);
  __ ld8(GR15_frame_sizes, GR15_frame_sizes);
  __ ld8(GR18_frame_pcs, GR18_frame_pcs);
  __ ld4(GR16_frame_count, GR16_frame_count);

  // %%%%%%% may have to setup some oop maps for the call site if G/O
  //         registers are used in code

  // Now we must pop the frame we are deoptimizing

  __ pop_dummy_thin_frame();  // Not really a dummy, but junk now. (Assumes the pops SP correctly too QQQ)

  // We now have in LC and UNAT the values that we there on entry to the deoptimized frame
  // We will propagate them to all the frames we create only the original caller of
  // the deoptimized frame will care (and if that caller is an i2c we will pop it and
  // used its LC and UNAT instead).
  // Seems like things like r4-r7 are ok because Java never touches them so they should
  // already be saved/restored by non-Java code and their values here should be correct
  // already.

  __ cmp4(PR6, PR0, 0, GR17_adapter_size, Assembler::equal);
  __ br(PR6, skip_i2c, Assembler::dptk);
  __ pop_dummy_thin_frame();  // Pop the adapter frame (Assumes the pops SP correctly too QQQ)

  __ bind(skip_i2c);

  // We are now in the frame that will become an interpreter frame
  // the RP is either the call_stub or the frame manager
  
  // loop through the UnrollBlock info and create interpreter frames
  // Everything must be done in global registers because the windows
  // will be changing constantly

  #ifdef ASSERT
  // make sure that there is at least one entry in the array
  __ cmp4(PR6, PR0, 0, GR16_frame_count, Assembler::equal);
  __ breaki(PR6, 1);
  #endif

  // QQQ what about adjusting the caller's stack and getting the sender_sp correct!!!

  // We could use LC but then we'd have to shuffle it to save the preserved value.
  // Doesn't seem to be worth the bother

  const Register GR19_pc  = GR19_SCRATCH;

  __ bind(loop);

  __ ld8(GR8, GR15_frame_sizes, sizeof(intptr_t));     // Get size of next memory frame to allocate
  __ ld8(GR19_pc, GR18_frame_pcs, sizeof(address));    // Get return address we want to store in the new frame
  __ sub(GR16_frame_count, GR16_frame_count, 1);
  __ push_dummy_full_frame(GR19_pc);            // Allocate register stack side of the frame 

  #ifdef ASSERT
  // trash local registers to show a clear pattern in backtraces
  // __ mov(GR_L0, 0xDEAD00); GR_Lsave_SP
  // __ mov(GR_L1, 0xDEAD01); GR_Lsave_BSP
  // __ mov(GR_L2, 0xDEAD02); GR_Lsave_PFS
  // __ mov(GR_L3, 0xDEAD03); GR_Lsave_RP
  // __ mov(GR_L4, 0xDEAD04); GR_Lsave_LC
  // __ mov(GR_L5, 0xDEAD05); GR_Lsave_GP
  // __ mov(GR_L6, 0xDEAD06); GR_Lsave_UNAT
  // __ mov(GR_L7, 0xDEAD07); GR_Lsave_PR
  __ mov(GR_L8, 0xDEAD08);  //GR_Lstate
  __ mov(GR_L9, 0xDEAD09);  //GR_Lscratch0
  __ mov(GR_L10, 0xDEAD10); //GR_Lmethod_addr
  __ mov(GR_L11, 0xDEAD11);
  __ mov(GR_L12, 0xDEAD12);
  __ mov(GR_L13, 0xDEAD13);
  __ mov(GR_L14, 0xDEAD14);
  __ mov(GR_L15, 0xDEAD15);
  #endif

  __ sub(SP, SP, GR8);                                 // Allocate memory stack side of the frame

  __ cmp4(PR0, PR6, 0, GR16_frame_count, Assembler::equal);
  __ br(PR6, loop, Assembler::dptk);

  // push a dummy "unpack_frame" taking care of float/int return values and
  // call Deoptimization::unpack_frames to have the unpacker layout
  // information in the interpreter frames just created and then return
  // to the interpreter entry point

  // Flush the windows to the stack. For Itanium2 we will have to also turn off eager filling
  // so that the windows don't get filled while we're in the middle of changing them!

  __ ld8(GR19_pc, GR18_frame_pcs, sizeof(address));    // Get return address we want to store in the new frame

  __ push_dummy_full_frame(GR19_pc);                    // Will return to uncommon trap entry point in frame manager
  __ sub(SP, SP, framesize*wordSize);                  // Allocate the proper frame size for this blob
						       // so stack walking will work properly.

  // We must be sure that the current windows flush so flushrs inline
  // will not be sufficient. The fact that the current window refills
  // is not important since we just want to be certain that the
  // stack is walkable and it will be.
  __ call_VM_leaf((address)StubRoutines::ia64::flush_register_stack());

  __ set_last_Java_frame(SP);
  __ mov(GR2, Deoptimization::Unpack_uncommon_trap);
  __ call_VM_leaf(CAST_FROM_FN_PTR(address, Deoptimization::unpack_frames), GR4_thread, GR2);
  __ reset_last_Java_frame();

  // We should be in the newly constructed top frame of the interpreter (frame_manager)

  __ pop_full_frame();
  __ ret();

  __ flush();
  _uncommon_trap_blob = UncommonTrapBlob::create(buffer, oop_maps, framesize);

}

//------------------------------generate_illegal_instruction_handler_blob------
//
// Generate a special Compile2Runtime blob that saves all registers, 
// and setup oopmap.
//
// It is important that we do not touch the %i registers before restore and
// %o registers (other than %o7) after restore here (these may also be modified by gc, 
// and we just propagate them up to the responsible Java frame)
//
// This blob is jumped to (via a breakpoint and the signal handler) from a safepoint
// in a thread code buffer.  On entry to this blob, O7 contains the address in the
// original nmethod at which we should resume normal execution.  Thus, this blob looks
// like a subroutine which must preserve lots of registers and return normally.
// Note that O7 is never register-allocated, so it is guaranteed to be free here.
//

void OptoRuntime::generate_illegal_instruction_handler_blob() {
#if 0
  assert (StubRoutines::forward_exception_entry() != NULL, "must be generated before");  

  // allocate space for the code
  ResourceMark rm;
  // setup code generation tools  
  CodeBuffer*    buffer = new CodeBuffer(1536, 1536, 0, 0, false);
  MacroAssembler masm(buffer);
  
  address start = __ pc();

  // decide which registers we are dumping to stack
  RegisterSaver rs(&masm, all_volatile_registers);
  rs.allocate_offsets(frame::memory_parameter_word_sp_offset);

  // setup a frame for saving the state
  __ save_frame(rs.num_saved_reg_words());
  int framesize = __ total_frame_size_in_bytes(rs.num_saved_reg_words())/wordSize;

  rs.save_registers();

  // setup last_Java_sp
  __ set_last_Java_frame(SP, G0);

  // call into the runtime to handle illegal instructions exception
  // Do not use call_VM_leaf, because we need to make a GC map at this call site.
  __ mov(Gthread, O0);
  __ save_thread(Lthread_cache);
  __ call(CAST_FROM_FN_PTR(address, SafepointSynchronize::handle_illegal_instruction_exception));
  __ delayed()->nop();
  address call_pc = __ pc();
  __ restore_thread(Lthread_cache);

  // clear last_Java_sp
  __ reset_last_Java_frame();
 
  Register Otemp = O7;

  // check for pending exceptions
  { Label L;
    Address exception_addr(Gthread, 0, in_bytes(Thread::pending_exception_offset()));
    __ ld_ptr(exception_addr, Otemp);
    // Even if !_LP64, it is OK to look at all 64 bits of register contents,
    // because the ld_ptr instruction (lduw in this case) zero-extends to 64 bits.
    __ bpr(Assembler::rc_z, false, Assembler::pt, Otemp, L);
    __ delayed()->nop();

    // Restore all registers, including flags. 
    // This could be merged with the other call to restore_registers() and hoisted 
    // above the check for pending exceptions, if the above tst instruction 
    // didn't clobber the condition codes.
    rs.restore_registers();

    // Tail-call forward_exception_entry, with the issuing PC in O7,
    // so it looks like the original nmethod called forward_exception_entry.
    __ set((intptr_t)StubRoutines::forward_exception_entry(), Otemp);
    __ jmpl(Otemp, G0, G0);
    __ delayed()->restore();

    __ bind(L);
  }

  Label negativeL;
  __ tst(O0);
  __ brx(Assembler::negative, false, Assembler::pt, negativeL);
  __ delayed()->nop();

  // Restore all registers, including flags
  rs.restore_registers();

  // Return to the original nmethod, as identified by the O7 value
  // (now I7 until the next restore) set up by the trap handler.
  // (probably, that logic should be moved out of the trap handler!)
  __ ret();
  __ delayed()->restore();

  // deoptimization happened at a compiled safepoint at a call; jimp to destination
  // instead of skipping the call in the blocked method; we do not have debug information
  // to reexecute the call
  __ bind(negativeL);
  rs.restore_registers();
  __ restore();
  __ mov(O7, Gscratch); // destination of the call
  DeoptimizationBlob* deopt_blob = OptoRuntime::deoptimization_blob();
  assert(deopt_blob != NULL, "deoptimization blob must have been created");
  __ load_address(Address(O7, deopt_blob->unpack() -  frame::pc_return_offset));
  // __ breakpoint_trap();
  __ jmpl(Gscratch, frame::pc_return_offset, G0);
  __ delayed()->nop();

  // Set an oopmap for the call site.
  // We need this not only for callee-saved registers, but also for
  // volatile registers that the compiler might be keeping live across a safepoint.
  OopMapSet *oop_maps = new OopMapSet();
  OopMap* map =  new OopMap( framesize, 0 );  
  rs.set_callee_saved(map, framesize);
  oop_maps->add_gc_map( call_pc - start, true, map);

  // -------------
  // make sure all code is generated
  __ flush();  

  // Set exception blob
  _illegal_instruction_handler_blob = SafepointBlob::create(buffer, oop_maps, framesize);
#else
//  warning("Unimplemented: OptoRuntime::generate_illegal_instruction_handler_blob");
#endif
}


//------------------------------exception_address_is_unpack_entry--------------
bool DeoptimizationBlob::exception_address_is_unpack_entry(address pc) const {
  address unpack_pc = OptoRuntime::deoptimization_blob()->unpack();
  return (pc == unpack_pc );
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
  CodeBuffer*    cb = new CodeBuffer(128, 64, 0, 0, 0, false);
  MacroAssembler masm(cb);    
  
  OopMapSet *oop_maps = new OopMapSet();
  OopMap* map =  new OopMap(frame_size, 0 );
  oop_maps->add_gc_map(0, true, map);
  
  __ breakm(2);

  // make sure all code is generated
  __ flush();

  warning("Kludge: OptoRuntime::generate_osr_blob");

  return OSRAdapter::new_osr_adapter(cb, oop_maps, frame_size);
}

#ifdef ASSERT
// Add this routine to print an oopMap since gdb refuses to cooperate.

extern "C" void prtOopMapSet( OopMapSet* p) { p->print(); }
#endif

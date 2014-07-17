#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)c1_Runtime1_i486.cpp	1.145 03/01/23 10:53:33 JVM"
#endif
// 
// Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

#include "incls/_precompiled.incl"
#include "incls/_c1_Runtime1_i486.cpp.incl"


// Implementation of StubAssembler

int StubAssembler::call_RT(Register oop_result1, Register oop_result2, address entry, int args_size) {
  // setup registers
  const Register thread = edi; // is callee-saved register (Visual C++ calling conventions)
  assert(!(oop_result1->is_valid() || oop_result2->is_valid()) || oop_result1 != oop_result2, "registers must be different");
  assert(oop_result1 != thread && oop_result2 != thread, "registers must be different");
  assert(args_size >= 0, "illegal args_size");
  // push java thread (becomes first argument of C function)
  get_thread(thread);
  pushl(thread);
  // set last Java frame before call
  movl(Address(thread, JavaThread::last_Java_fp_offset()), ebp);
  movl(Address(thread, JavaThread::last_Java_sp_offset()), esp);
  // do the call
  call(entry, relocInfo::runtime_call_type);
  int call_offset = offset();
  // verify callee-saved register
#ifdef ASSERT
  guarantee(thread != eax, "change this code");
  pushl(eax);
  { Label L;
    get_thread(eax);
    cmpl(thread, eax);
    jcc(Assembler::equal, L);
    stop("StubAssembler::call_RT: edi not callee saved?");
    bind(L);
  }
  popl(eax);
#endif
  // reset last Java frame after call
  movl(Address(thread, JavaThread::last_Java_sp_offset()), (int)NULL);
  movl(Address(thread, JavaThread::last_Java_fp_offset()), (int)NULL);
  // discard thread and arguments
  addl(esp, (1 + args_size)*BytesPerWord);
  // check for pending exceptions
  { Label L;
    cmpl(Address(thread, Thread::pending_exception_offset()), (int)NULL);
    jcc(Assembler::equal, L);
    // exception pending => remove activation and forward to exception handler
    movl(eax, Address(thread, Thread::pending_exception_offset()));
    // make sure that the vm_results are cleared
    if (oop_result1->is_valid()) {
      movl(Address(thread, JavaThread::vm_result_offset()), (int)NULL);
    }
    if (oop_result2->is_valid()) {
      movl(Address(thread, JavaThread::vm_result_2_offset()), (int)NULL);
    }
    leave();
    jmp(StubRoutines::forward_exception_entry(), relocInfo::runtime_call_type);
    bind(L);
  }
  // get oop results if there are any and reset the values in the thread
  if (oop_result1->is_valid()) {
    movl(oop_result1, Address(thread, JavaThread::vm_result_offset()));
    movl(Address(thread, JavaThread::vm_result_offset()), (int)NULL);
    verify_oop(oop_result1);
  }
  if (oop_result2->is_valid()) {
    movl(oop_result2, Address(thread, JavaThread::vm_result_2_offset()));
    movl(Address(thread, JavaThread::vm_result_2_offset()), (int)NULL);
    verify_oop(oop_result2);
  }
  return call_offset;
}


int StubAssembler::call_RT(Register oop_result1, Register oop_result2, address entry, Register arg1) {
  pushl(arg1);
  return call_RT(oop_result1, oop_result2, entry, 1);
}


int StubAssembler::call_RT(Register oop_result1, Register oop_result2, address entry, Register arg1, Register arg2) {
  pushl(arg2);
  pushl(arg1);
  return call_RT(oop_result1, oop_result2, entry, 2);
}


int StubAssembler::call_RT(Register oop_result1, Register oop_result2, address entry, Register arg1, Register arg2, Register arg3) {
  pushl(arg3);
  pushl(arg2);
  pushl(arg1);
  return call_RT(oop_result1, oop_result2, entry, 3);
}


// Implementation of StubFrame

class StubFrame: public StackObj {
 private:
  StubAssembler* _sasm;

 public:
  StubFrame(StubAssembler* sasm, const char* name, bool must_gc_arguments);
  StubFrame(StubAssembler* sasm, const char* name, bool must_gc_arguments, Register arg1);
  StubFrame(StubAssembler* sasm, const char* name, bool must_gc_arguments, Register arg1, Register arg2);
  StubFrame(StubAssembler* sasm, const char* name, bool must_gc_arguments, Register arg1, Register arg2, Register arg3);

  ~StubFrame();
};


#define __ _sasm->

StubFrame::StubFrame(StubAssembler* sasm, const char* name, bool must_gc_arguments) {
  _sasm = sasm;
  __ set_info(name, must_gc_arguments);
  __ enter();
}


StubFrame::StubFrame(StubAssembler* sasm, const char* name, bool must_gc_arguments, Register arg1) {
  _sasm = sasm;
  __ set_info(name, must_gc_arguments);
  __ enter();
  // tos + 0: link
  //     + 1: return address
  //     + 2: arg1
  __ movl(arg1, Address(ebp, 2*BytesPerWord));
}


StubFrame::StubFrame(StubAssembler* sasm, const char* name, bool must_gc_arguments, Register arg1, Register arg2) {
  _sasm = sasm;
  __ set_info(name, must_gc_arguments);
  assert(arg1 != arg2, "registers must be different");
  __ enter();
  // tos + 0: link
  //     + 1: return address
  //     + 2: arg2
  //     + 3: arg1
  __ movl(arg2, Address(ebp, 2*BytesPerWord));
  __ movl(arg1, Address(ebp, 3*BytesPerWord));
}


StubFrame::StubFrame(StubAssembler* sasm, const char* name, bool must_gc_arguments, Register arg1, Register arg2, Register arg3) {
  _sasm = sasm;
  __ set_info(name, must_gc_arguments);
  assert(arg1 != arg2 && arg1 != arg3 && arg2 != arg3, "registers must be different");
  __ enter();
  // tos + 0: link
  //     + 1: return address
  //     + 2: arg3
  //     + 3: arg2
  //     + 4: arg1
  __ movl(arg3, Address(ebp, 2*BytesPerWord));
  __ movl(arg2, Address(ebp, 3*BytesPerWord));
  __ movl(arg1, Address(ebp, 4*BytesPerWord));
}


StubFrame::~StubFrame() {
  __ leave();
  __ ret(0);
}

#undef __


// Implementation of Runtime1

#define __ sasm->

static OopMap* save_caller_save_registers(StubAssembler* sasm, int num_rt_args);
static void restore_caller_save_registers(StubAssembler* sasm);

const int fpu_stack_as_doubles_size_in_words = 16;
const int fpu_stack_as_doubles_size = 64;

enum reg_save_layout {
  fpu_stack_as_doubles_off = 0,
  fpu_state_off = fpu_stack_as_doubles_off + fpu_stack_as_doubles_size_in_words,
  flags_off = fpu_state_off + FPUStateSizeInWords,
  edi_off,
  esi_off,
  ebp_off,
  esp_off,
  ebx_off,
  edx_off,
  ecx_off,
  eax_off,      
  saved_ebp_off,
  return_off  
};

const int reg_save_size = eax_off + 1;


static OopMap* save_caller_save_registers(StubAssembler* sasm, int num_rt_args) {
  // save all register state - int, flags, fpu
  __ pushad();
  __ pushfd();
  __ push_FPU_state();
  // save all fp data registers in double-precision format for use in possible deoptimization;
  // must first restore FPUStatusWord that was initialized by push_FPU_state (fnsave instruction)
  __ fldenv(Address(esp));
  __ subl(esp, fpu_stack_as_doubles_size);
  __ fstp_d(Address(esp    ));
  __ fstp_d(Address(esp,  8));
  __ fstp_d(Address(esp, 16));
  __ fstp_d(Address(esp, 24));
  __ fstp_d(Address(esp, 32));
  __ fstp_d(Address(esp, 40));
  __ fstp_d(Address(esp, 48));
  __ fstp_d(Address(esp, 56));

  // record saved value locations in an OopMap
  // locations are offsets from sp after runtime call; num_rt_args is number of arguments in call, including thread
  OopMap* map = new OopMap(reg_save_size+num_rt_args, 0);
  map->set_callee_saved(OptoReg::Name(SharedInfo::stack0+eax_off+num_rt_args), reg_save_size, 0, OptoReg::Name(eax->encoding()));
  map->set_callee_saved(OptoReg::Name(SharedInfo::stack0+ecx_off+num_rt_args), reg_save_size, 0, OptoReg::Name(ecx->encoding()));
  map->set_callee_saved(OptoReg::Name(SharedInfo::stack0+edx_off+num_rt_args), reg_save_size, 0, OptoReg::Name(edx->encoding()));
  map->set_callee_saved(OptoReg::Name(SharedInfo::stack0+ebx_off+num_rt_args), reg_save_size, 0, OptoReg::Name(ebx->encoding()));
  map->set_callee_saved(OptoReg::Name(SharedInfo::stack0+esi_off+num_rt_args), reg_save_size, 0, OptoReg::Name(esi->encoding()));
  map->set_callee_saved(OptoReg::Name(SharedInfo::stack0+edi_off+num_rt_args), reg_save_size, 0, OptoReg::Name(edi->encoding()));
  int fpu_off = fpu_stack_as_doubles_off;
  for (int n = 0; n < FrameMap::nof_fpu_regs; n++) {
    OptoReg::Name fpu_name_0 = FrameMap::fpu_stack_regname(n);
    OptoReg::Name fpu_name_1 = OptoReg::add(fpu_name_0, 1);
    map->set_callee_saved(OptoReg::Name(SharedInfo::stack0+fpu_off+  num_rt_args), reg_save_size, 0, fpu_name_0);
    map->set_callee_saved(OptoReg::Name(SharedInfo::stack0+fpu_off+1+num_rt_args), reg_save_size, 0, fpu_name_1);
    fpu_off += 2;
  }
  return map;
}


static void restore_caller_save_registers(StubAssembler* sasm) {
  // pop fpu regs that were saved in double-precision format
  __ addl(esp, fpu_stack_as_doubles_size);
  // restore FPU and IU state
  __ pop_FPU_state();
  __ popfd();
  __ popad();
}


void Runtime1::initialize_pd() {
  // nothing to do
}


OopMapSet* Runtime1::generate_exception_throw(StubAssembler* sasm, address target, Register arg1) {
  // preserve registers for recording debug info in case deoptimization happens during the throw
  // (only needed for local variables cached in registers)
  int num_rt_args = arg1 == noreg ? 1 : 2;  // thread and arg1, if present
  OopMap* map = save_caller_save_registers(sasm, num_rt_args);
  int call_offset;
  if (arg1 == noreg) {
    call_offset = __ call_RT(noreg, noreg, target);
  } else {
    call_offset = __ call_RT(noreg, noreg, target, arg1);
  }
  __ should_not_reach_here();
  OopMapSet* oop_maps = new OopMapSet();
  oop_maps->add_gc_map(call_offset, true, map);
  return oop_maps;
}


OopMapSet* Runtime1::generate_patching(StubAssembler* sasm, address target) {
  const int num_rt_args = 1;  // thread
  OopMap* map = save_caller_save_registers(sasm, num_rt_args);
  int call_offset = __ call_RT(noreg, noreg, target);
  OopMapSet* oop_maps = new OopMapSet();
  oop_maps->add_gc_map(call_offset, true, map);

  // correct the address so that it is reexecuted
  __ movl (eax, Address(esp, return_off*BytesPerWord));

  // if the return address is the normal deoptimization handler entry, change it to the
  // reexecute entry so the bytecode associated with the patched instruction will be executed
  Label reexecuteEntry, cont;
  DeoptimizationBlob* deopt_blob = (DeoptimizationBlob*) Runtime1::blob_for(Runtime1::deoptimization_handler_id);
  assert(deopt_blob != NULL, "deoptimization blob must have been created");
  __ cmpl(eax, (int)deopt_blob->unpack());
  __ jcc(Assembler::notEqual, cont);
  // replace standard entry with reexecution entry
  __ movl(Address(esp, return_off*BytesPerWord), (int)deopt_blob->unpack_with_reexecution());

  __ bind(cont);
  restore_caller_save_registers(sasm);
  __ leave();
  __ ret(0);
  return oop_maps;
}

static void deopt_placeholder(void) { }

OopMapSet* Runtime1::generate_deoptimization_handler(StubAssembler* sasm, int* exception_offset, int* frame_size, int* reexecute_offset) {
  enum layout { arg0_off,       // used for thread
                arg1_off,       // used for sp
                arg2_off,       // used for has_exception
                eax_off,        // slot for java return value
                edx_off,        // slot for java return value
                fp0_off,        // slot for java return value
                fp0_off_2,      // slot for java return value
                ret_type_off,   // slot for return type
                exception_off,  // slot for has_exception
                ebp_off,        // slot for returned frame pointer
                return_off,     // slot for return address
                framesize };

  // This code enters when returning to a de-optimized frame.
  // The return address has been popped from the stack, and return values
  // are in registers.  At this point we need to de-opt.  We save 
  // the argument return registers.  We call the first C routine,
  // fetch_unroll_info().  This routine captures the callee-save registers and
  // return values, and returns a structure which describes the current frame
  // size and the sizes of all replacement frames.  The current frame is 
  // compiled code and may contain many inlined functions, each with their own
  // JVM state.  We pop the current frame, then push all the new frames.  Then
  // we call the C routine unpack_frames() to populate these frames.  Finally
  // unpack_frames() returns us the new target address.
  address start = __ pc();
  Label cont;

  // Add empty oopmap at begining. This can both be at_call and not_at_call,
  // Normally it would be at call. However, it could be at not_at_call, if the
  // thread happended to be blocked at safepoint_node in the frame that got
  // deoptimized.
  OopMapSet* oop_maps = new OopMapSet();

  // First oopmap needs to record EBP so's link_offset() can find it
  OopMap* map = new OopMap(framesize, 0);
  map->set_callee_saved(SharedInfo::stack2reg(ebp_off), framesize, 0, OptoReg::Name(ebp->encoding()));
  oop_maps->add_gc_map(0, true, map);
  oop_maps->add_gc_map(0, false, new OopMap(framesize, 0));

  // Prolog for non exception case
  __ subl(esp, framesize * wordSize);
  __ movl(Address(esp, exception_off * wordSize), Deoptimization::Unpack_deopt);
  __ jmp(cont);
  
  *exception_offset = __ pc() - start;

  // Prolog for exception case
  __ subl(esp, framesize * wordSize);
  __ movl(Address(esp,exception_off*wordSize), Deoptimization::Unpack_exception); 
  __ jmp(cont);

  *reexecute_offset = __ pc() - start;

  // Prolog for non-exception case that requires reexecution (i.e. uncommon trap)
  __ subl(esp, framesize * wordSize);
  __ movl(Address(esp, exception_off * wordSize), Deoptimization::Unpack_reexecute);

  __ bind(cont);

  // Save return-value registers

  __ movl(Address(esp,   eax_off * wordSize), eax);
  __ movl(Address(esp,   edx_off * wordSize), edx);
  __ fstp_d(Address(esp, fp0_off * wordSize)); // Pop floating point stack and store into local 
                                               // (if tos is empty, NaN is saved)

  // Compiled code may leave the floating point stack dirty, empty it.
  __ empty_FPU_stack();

  // Call fetch_unroll_info().  Need thread and this frame, but NOT official VM entry - cannot block on
  // this call, no GC can happen.  Call should capture return values.
  __ get_thread(ecx);
  __ movl(Address(esp, arg0_off * wordSize), ecx);

  // Make a walkable stack so fetch_unroll_info can see the current sender of
  // the deoptee. The return pc is not important but ebp is
  __ movl(Address(esp,   ebp_off * wordSize), ebp);  // save current ebp
  __ leal(ebp, Address(esp,   ebp_off * wordSize)); // and link it
  // Use an address that looks decent in the debugger
  __ movl(Address(esp,return_off * wordSize), CAST_FROM_FN_PTR(int, deopt_placeholder));

  __ movl(Address(ecx, JavaThread::last_Java_sp_offset()), esp);
  __ movl(Address(ecx, JavaThread::last_Java_fp_offset()), ebp);
  __ call(CAST_FROM_FN_PTR(address, Deoptimization::fetch_unroll_info), relocInfo::runtime_call_type);
  __ get_thread(ecx);
  __ movl(Address(ecx, JavaThread::last_Java_sp_offset()), (int)NULL);
  __ movl(Address(ecx, JavaThread::last_Java_fp_offset()), (int)NULL);
  __ movl(ebp, Address(esp,   ebp_off * wordSize));       // restore original ebp

  // Set an oopmap for the call site   
  oop_maps->add_gc_map(__ pc()-start, true, new OopMap(framesize, 0));

  // Load UnrollBlock into EDI
  __ movl(edi, eax);


  // Store frame locals in registers or memory

  // Move the unpack kind to a safe place in the UnrollBlock because
  // we are very short of registers

  Address unpack_kind(edi, Deoptimization::UnrollBlock::unpack_kind_offset_in_bytes());
  __ movl(eax, Address(esp, exception_off * wordSize));
  __ movl(unpack_kind, eax);                      // save the unpack_kind value

  // Move the possible live values

  __ movl(eax, Address(esp, eax_off * wordSize));
  __ movl(edx, Address(esp, edx_off * wordSize));
  __ fld_d(Address(esp, fp0_off * wordSize));    // Push on float stack

  //
  // Frame picture (youngest to oldest)
  // 1: self-frame (no frame link)
  // 2: deopting frame  (ebp points to this frame's base)
  // 3: possible-osr-adapter-frame or alignment frame
  // 4: caller of deopting frame
  // 

  // First compute the final sp after all 1-3 are removed

  __ movl(ecx, esp);                             // copy current sp
  __ addl(ecx, framesize*wordSize);              // sp of #2
  __ addl(ecx, Address(edi,Deoptimization::UnrollBlock::size_of_deoptimized_frame_offset_in_bytes()));
  __ addl(ecx,Address(edi,Deoptimization::UnrollBlock::adapter_size_offset_in_bytes()));

  // Extract the ebp of #4 by walking the links.

  Label no_pop_adapter;
  __ cmpl(Address(edi, Deoptimization::UnrollBlock::adapter_size_offset_in_bytes()), (int)NULL);
  __ jcc(Assembler::zero, no_pop_adapter);
  __ movl(ebp, Address(ebp));			// follow one frame link
  __ bind(no_pop_adapter);

  // sp should now be pointing at the saved fp of the caller of the deopting frame (#4)
  // in the remnant of frame 2 or 3.

  __ movl(ebp, Address(ebp));			// get the original stacked frame link (#4's)

  // Now pop the stack

  __ movl(esp, ecx);                            // set sp to #4 frame.sp()

  // Load array of frame pcs into ECX
  __ movl(ecx,Address(edi,Deoptimization::UnrollBlock::frame_pcs_offset_in_bytes()));

  // Now adjust the caller's stack to make up for the extra locals
  // but record the original sp so that we can save it in the skeletal interpreter
  // frame and the stack walking of interpreter_sender will get the unextended sp
  // value and not the "real" sp value.

  Address sp_temp(edi, Deoptimization::UnrollBlock::sender_sp_temp_offset_in_bytes());
  __ movl(sp_temp, esp);
  __ subl(esp, Address(edi, Deoptimization::UnrollBlock::caller_adjustment_offset_in_bytes()));

  // Load array of frame sizes into ESI
  __ movl(esi, Address(edi, Deoptimization::UnrollBlock::frame_sizes_offset_in_bytes()));

  Address counter(edi, Deoptimization::UnrollBlock::number_of_frames_offset_in_bytes());

  // Push interpreter frames in a loop
  Label loop;

  __ bind(loop);
  __ movl(ebx, Address(esi));           // Load frame size
  __ subl(ebx, 2*wordSize);             // we'll push pc and ebp by hand
  __ pushl(Address(ecx));               // save return address
  __ enter();                           // save old & set new ebp
  __ subl(esp, ebx);                    // Prolog!
  __ movl(ebx, sp_temp);                // sender's sp
  __ movl(Address(ebp, frame::interpreter_frame_sender_sp_offset * wordSize), ebx); // Make it walkable
  __ movl(sp_temp, esp);                // pass to next frame
  __ addl(esi, 4);                      // Bump array pointer (sizes)
  __ addl(ecx, 4);                      // Bump array pointer (pcs)
  __ decl(counter);                     // decrement counter
  __ jcc(Assembler::notZero, loop);
  __ pushl(Address(ecx));               // save final return address

  // Re-push self-frame
  __ enter();                           // save old & set new ebp
  __ subl(esp, (framesize-2) * wordSize);   // Prolog!

  // Restore frame locals after moving the frame
  __ movl(Address(esp, eax_off * wordSize), eax);
  __ movl(Address(esp, edx_off * wordSize), edx);
  __ fstp_d(Address(esp, fp0_off * wordSize));   // Pop float stack and store in local
  __ movl(eax, unpack_kind);                     // get the unpack_kind value
  __ movl(Address(esp, exception_off * wordSize), eax);

  // Store whether the top interpreter frame expects a float or double.
  __ movl(eax, Address(edi, Deoptimization::UnrollBlock::return_type_offset_in_bytes()));
  __ movl(Address(esp, ret_type_off * wordSize), eax);

  // set last_Java_sp, last_Java_fp
  __ get_thread(ecx);
  __ movl(Address(ecx, JavaThread::last_Java_sp_offset()), esp);
  __ movl(Address(ecx, JavaThread::last_Java_fp_offset()), ebp);

  // Call unpack_frames().  Need thread and this frame, but NOT official VM entry - cannot block on
  // this call, no GC can happen.  Call should capture callee-save registers as well as return values.
  __ movl(Address(esp, arg0_off * wordSize), ecx);
  __ movl(eax, Address(esp, exception_off * wordSize)); 
  __ movl(Address(esp, arg1_off * wordSize), eax);		// exec mode
  __ call(CAST_FROM_FN_PTR(address, Deoptimization::unpack_frames), relocInfo::runtime_call_type);

  // Set an oopmap for the call site
  oop_maps->add_gc_map(__ pc()-start, true, new OopMap(framesize, 0));

  __ get_thread(ecx);
  // reset last Java frame
  // we must set sp to zero to clear frame
  __ movl(Address(ecx, JavaThread::last_Java_sp_offset()), (int)NULL);
  // must clear fp, so that compiled frames are not confused; it is possible
  // that we need it only for debugging
  __ movl(Address(ecx, JavaThread::last_Java_fp_offset()), (int)NULL);

  // Collect return values
  __ movl(eax, Address(esp, eax_off * wordSize));
  __ movl(edx, Address(esp, edx_off * wordSize));

  // Clear floating point stack before returning to interpreter
  __ empty_FPU_stack();

  // Push a float or double return value if necessary.
  Label no_double_value, yes_double_value;
  __ cmpl(Address(esp, ret_type_off*wordSize), T_DOUBLE);
  __ jcc (Assembler::zero, yes_double_value);
  __ cmpl(Address(esp, ret_type_off*wordSize), T_FLOAT);
  __ jcc (Assembler::notZero, no_double_value);
  __ bind(yes_double_value);
  __ fld_d(Address(esp,fp0_off*wordSize)); // Push on float stack
  __ bind(no_double_value);

  // Pop self-frame.
  __ leave();

  // Jump to interpreter
  __ ret(0);
  
  *frame_size            = framesize;
  return oop_maps;
}


OopMapSet* Runtime1::generate_code_for(StubID id, StubAssembler* sasm, int* unpack_exception_offset, int* frame_size, int* unpack_reexecute_offset) {

  // for better readability
  const bool must_gc_arguments = true;
  const bool dont_gc_arguments = false;

  // stub code & info for the different stubs
  OopMapSet* oop_maps = NULL;
  switch (id) {
    case new_instance_id:
      { StubFrame f(sasm, "new_instance", dont_gc_arguments);
        // eax: klassOop
        __ call_RT(eax, noreg, CAST_FROM_FN_PTR(address, new_instance), eax);
        // eax: new instance
      }
      break;

    case new_type_array_id:
      { StubFrame f(sasm, "new_type_array", dont_gc_arguments, eax, ebx);
        // eax: elt_type
        // ebx: length
        __ call_RT(eax, noreg, CAST_FROM_FN_PTR(address, new_type_array), eax, ebx);
        // eax: new type array
      }
      break;

    case new_object_array_id:
      { StubFrame f(sasm, "new_object_array", dont_gc_arguments, eax, ebx);
        // eax: klass
        // ebx: length
        __ call_RT(eax, noreg, CAST_FROM_FN_PTR(address, new_object_array), eax, ebx);
        // eax: new object array
      }
      break;

    case new_multi_array_id:
      { StubFrame f(sasm, "new_multi_array", dont_gc_arguments);
        // eax: klass
        // ebx: rank
        // ecx: address of 1st dimension
        __ call_RT(eax, noreg, CAST_FROM_FN_PTR(address, new_multi_array), eax, ebx, ecx);
        // eax: new multi array
      }
      break;

    case resolve_invokestatic_id:
      { StubFrame f(sasm, "resolve_invokestatic", must_gc_arguments);
        __ xorl(ecx, ecx); // recv must be NULL
        __ call_RT(ebx, noreg, CAST_FROM_FN_PTR(address, resolve_invoke), ecx);
        // eax: interpreter or verified entry point
        // ebx: callee method (always)
        __ leave();
        __ jmp(eax);
      }
      break;

    case resolve_invoke_id:
      { StubFrame f(sasm, "resolve_invoke", must_gc_arguments);
        // ecx: receiver
        __ call_RT(ebx, ecx, CAST_FROM_FN_PTR(address, resolve_invoke), ecx);
        // eax: interpreter or verified entry point
        // ebx: callee method (always)
        // ecx: receiver
        __ leave();
        __ jmp(eax);
      }
      break;

    case handle_ic_miss_id:
      { StubFrame f(sasm, "handle_ic_miss", must_gc_arguments);
        // eax: ic klass
        // ecx: receiver
        __ call_RT(ebx, ecx, CAST_FROM_FN_PTR(address, handle_ic_miss), ecx);
        // eax: interpreter or verified entry point
        // ebx: callee method (always)
        // ecx: receiver
        __ leave();
        __ jmp(eax);
      }
      break;

    case handle_wrong_static_method_id:
      { StubFrame f(sasm, "handle_wrong_static_method", must_gc_arguments);
        __ xorl(ecx, ecx); // recv must be NULL
        __ call_RT(ebx, ecx, CAST_FROM_FN_PTR(address, handle_wrong_method), ecx);
        // eax: interpreter or verified entry point
        // ebx: callee method (always)
        __ leave();
        __ jmp(eax);
      }
      break;

    case handle_wrong_method_id:
      { StubFrame f(sasm, "handle_wrong_method", must_gc_arguments);
        // ecx: receiver
        __ call_RT(ebx, ecx, CAST_FROM_FN_PTR(address, handle_wrong_method), ecx);
        // eax: interpreter or verified entry point
        // ebx: callee method (always)
        // ecx: receiver
        __ leave();
        __ jmp(eax);
      }
      break;

    case entering_non_entrant_id:
      { StubFrame f(sasm, "entering_non_entrant", must_gc_arguments);
        __ stop ("we entered a non entrant method: internal error");
      }
      break;

    case range_check_failed_id:
      { StubFrame f(sasm, "range_check_failed", dont_gc_arguments);
        oop_maps = generate_exception_throw(sasm, CAST_FROM_FN_PTR(address, throw_range_check_exception), eax);
      }
      break;

    case throw_index_exception_id:
      { StubFrame f(sasm, "index_range_check_failed", dont_gc_arguments);
        oop_maps = generate_exception_throw(sasm, CAST_FROM_FN_PTR(address, throw_index_exception), eax);
      }
      break;

    case throw_div0_exception_id:
      { StubFrame f(sasm, "throw_div0_exception", dont_gc_arguments);
        oop_maps = generate_exception_throw(sasm, CAST_FROM_FN_PTR(address, throw_div0_exception));
      }
      break;

    case throw_abstract_method_error_id:
      // exception PC is on TOS
      { StubFrame f(sasm, "throw_abstract_method_exception", dont_gc_arguments); // arguments will be discarded
        __ call_RT(noreg, noreg, CAST_FROM_FN_PTR(address, throw_abstract_method_error));
        __ should_not_reach_here();
      }
      break;

    case handle_exception_id:
      {
        StubFrame f(sasm, "handle_exception", dont_gc_arguments);
        // clear the stack in case any FPU results are left behind
        __ empty_FPU_stack();
        // tos: pc in the exception handler of the nmethod
        // eax: exception
        // edx: throwing pc
        // compute exception handler
        __ movl(ebx, Address(esp, 1*BytesPerWord));   // get real return address (to exception handler)
        __ movl(Address(esp, 1*BytesPerWord), edx);   // use throwing pc as return address (has bci & oop map)
        __ pushl(ebx);                                // save real return address
        // note: this call assumes any arguments to be on the stack if the
        //       exception happened at a call site - otherwise GC will not
        //       work correctly
        __ call_RT(ecx, noreg, CAST_FROM_FN_PTR(address, exception_handler_for_pc), eax, edx);
        __ popl(ebx);                                 // restore real return address
        // eax: handler address or NULL if no handler exists
        // ebx: real return address
        // ecx: exception
        __ verify_oop(ecx);
        // move continuation address into ebx
        // => use handler address if any, use ebx otherwise        
        
        __ movl(edi, ebx); // save original return address (could be deoptimization handler)
        __ testl(eax, eax);
        if (VM_Version::supports_cmov()) {
          __ cmovl(Assembler::notZero, ebx, eax);
        } else {
          Label L;
          __ jcc(Assembler::zero, L);
          __ movl(ebx, eax);
          __ bind(L);
        }
        __ movl(eax, ecx);   // restore exception in eax
        // eax: exception
        // ebx: continuation (exception handler or unwind code)
        __ leave();          // manually remove stub-frame since we do a jmp at end.
        __ popl(edx);        // remove return address

        Label contLbl;
        DeoptimizationBlob* deopt_blob = (DeoptimizationBlob*) Runtime1::blob_for(Runtime1::deoptimization_handler_id);
        assert(deopt_blob != NULL, "deoptimization blob must have been created");
        __ cmpl(edx, (int)deopt_blob->unpack());
        __ jcc(Assembler::notEqual, contLbl);
        __ movl(ebx, (int)deopt_blob->unpack_with_exception());
        __ bind(contLbl);
        __ jmp(ebx);

      }
      break;

    case unwind_exception_id:
      { __ set_info("unwind_exception", dont_gc_arguments);
        // note: no stubframe since we are about to leave the current
        //       activation and we are calling a leaf VM function only.
        // eax: exception
        // leave activation
        __ leave();
        __ movl(edx, Address(esp));
        if (ForceStackAlignment) {
          // cleanup dummy frame used by ForceStackAlignment
          Label L;
          __ cmpl(edx, (int)Runtime1::entry_for(Runtime1::alignment_frame_return_id));
          __ jcc(Assembler::notEqual, L);
          __ popl(edx);
          __ leave();
          __ movl(edx, Address(esp));
          __ bind(L);
        }
        __ pushl(eax);       // save exception (no GC can happen due to leaf call)
        __ call_VM_leaf(CAST_FROM_FN_PTR(address, SharedRuntime::exception_handler_for_return_address), edx);
        // tos + 0: exception
        //     + 1: return address
        // eax    : exception handler
        __ movl(ebx, eax);   // set exception handler
        __ popl(eax);        // restore exception
        __ popl(edx);        // get throwing pc (= return address)
        // continue at exception handler (return address removed)
        // note: do *not* remove arguments when unwinding the
        //       activation since the caller assumes having
        //       all arguments on the stack when entering the
        //       runtime to determine the exception handler
        //       (GC happens at call site with arguments!)
        // eax: exception
        // ebx: exception handler
        // edx: throwing pc
        __ verify_oop(eax);
        __ jmp(ebx);
      }
      break;

    case throw_array_store_exception_id:
      { StubFrame f(sasm, "throw_array_store_exception", dont_gc_arguments);
        // tos + 0: link
        //     + 1: return address
        oop_maps = generate_exception_throw(sasm, CAST_FROM_FN_PTR(address, throw_array_store_exception));
      }
      break;

    case throw_class_cast_exception_id:
      { StubFrame f(sasm, "throw_class_cast_exception", dont_gc_arguments);
        oop_maps = generate_exception_throw(sasm, CAST_FROM_FN_PTR(address, throw_class_cast_exception), eax);
      }
      break;

    case slow_subtype_check_id:
      {
        enum layout {
          eax_off,
          ecx_off,
          esi_off,
          edi_off,
          saved_ebp_off,
          return_off,
          sub_off,
          super_off,
          framesize
        };
        
        __ set_info("slow_subtype_check", dont_gc_arguments);
        __ pushl(edi);
        __ pushl(esi);
        __ pushl(ecx);
        __ pushl(eax);
        __ movl(esi, Address(esp, (super_off - 1) * BytesPerWord)); // super
        __ movl(eax, Address(esp, (sub_off   - 1) * BytesPerWord)); // sub

        __ movl(edi,Address(esi,sizeof(oopDesc) + Klass::secondary_supers_offset_in_bytes()));
        __ movl(ecx,Address(edi,arrayOopDesc::length_offset_in_bytes()));
        __ addl(edi,arrayOopDesc::base_offset_in_bytes(T_OBJECT));

        Label miss;
        __ repne_scan();
        __ jcc(Assembler::notEqual, miss);
        __ movl(Address(esi,sizeof(oopDesc) + Klass::secondary_super_cache_offset_in_bytes()), eax);
        __ movl(Address(esp, (super_off   - 1) * BytesPerWord), 1); // result
        __ popl(eax);
        __ popl(ecx);
        __ popl(esi);
        __ popl(edi);
        __ ret(0);

        __ bind(miss);
        __ movl(Address(esp, (super_off   - 1) * BytesPerWord), 0); // result
        __ popl(eax);
        __ popl(ecx);
        __ popl(esi);
        __ popl(edi);
        __ ret(0);
      }
      break;

    case monitorenter_id:
      { StubFrame f(sasm, "monitorenter", dont_gc_arguments, eax, ebx);
        // eax: object
        // ebx: lock address
        __ call_RT(noreg, noreg, CAST_FROM_FN_PTR(address, monitorenter), eax, ebx);
      }
      break;

    case monitorexit_id:
      { StubFrame f(sasm, "monitorexit", dont_gc_arguments, eax);
        // eax: lock address
        // note: really a leaf routine but must setup last java sp
        //       => use call_RT for now (speed can be improved by
        //       doing last java sp setup manually)
        __ call_RT(noreg, noreg, CAST_FROM_FN_PTR(address, monitorexit), eax);
      }
      break;

    case monitorenter_with_jvmpi_id:
      { // This is used for slow-case synchronization at method entry when JVMPI method entry events are enabled.
        // If the compiled activation has been deoptimized after the monitorenter, then jvmpi notification is done
        // here since execution will resume in the interpreter at the first bytecode.
        // eax: object
        // ebx: lock address
        StubFrame f(sasm, "monitorenter", dont_gc_arguments, eax, ebx);
        // Preserve the method's receiver in case its needed for jvmpi method entry
        __ get_thread(edx);
        __ movl(Address(edx, JavaThread::vm_result_offset()), eax);
        __ call_RT(eax, noreg, CAST_FROM_FN_PTR(address, monitorenter), eax, ebx);

        // If the compiled frame was deoptimized, the return address of this frame
        // was changed to be the deoptimization handler's normal entry point
        Label no_deopt;
        DeoptimizationBlob* deopt_blob = (DeoptimizationBlob*) Runtime1::blob_for(Runtime1::deoptimization_handler_id);
        assert(deopt_blob != NULL, "deoptimization blob must have been created");
        __ movl(ebx, Address(esp, 1*BytesPerWord));
        __ cmpl(ebx, (int)deopt_blob->unpack());
        __ jcc(Assembler::notEqual, no_deopt);
        __ call_RT(noreg, noreg, CAST_FROM_FN_PTR(address, jvmpi_method_entry_after_deopt), eax);
        __ bind(no_deopt);
      }
      break;

    case implicit_div0_exception_id: // fall through
    case implicit_null_exception_id:
      { // tos + 0: exception filter return address
        //     + 1: tos at exception point
        // all registers are as at the exception point (except eip & flags)
        //
        // how it works: 1) we check if there is a valid return address on the stack
        // (that is the case if the exception happened in an inline cache check);
        // if not, we compute a fake return address and use it instead (i.e., we
        // replace the filter's return address with the new address and don't pop
        // it afterwards) - 2) once we have the return address we throw the exception
        // via a runtime call; i.e., from the Java execution point-of-view it looks
        // like we called the VM to explicitly throw the exception
        //
        // note 1: for now the filter never returns but handles the exception always
        //         (in a 2-compiler system we would need to return if the exception
        //         happened outside the code generated by C1)
        //
        // note 2: div0 exceptions could be handled slightly simpler as they never
        //         occur at an invoke and thus always need a fake return address;
        //         i.e., no need to call return_address_for_exception except that
        //         we need to do the set_is_handling_implicit_exception(true) anyway
        //
        // step 1: determine fake return address and push it if necessary
        { Label L;
          __ pushad();                           // save registers
          __ call_VM_leaf(CAST_FROM_FN_PTR(address, return_address_for_exception));
          __ movl(Address(esp, RegisterImpl::number_of_registers * BytesPerWord), eax); // potential return address
          __ testl(eax, eax);                    // check for fake return address
          __ popad();                            // restore registers
          __ jcc(Assembler::notZero, L);         // we need a fake return address => don't pop it
          __ addl(esp, 1*BytesPerWord);          // we have a real return address => pop fake address
          __ bind(L);
        }

        // step 2: throw the exception via standard runtime call
        // tos + 0: real/fake return address used as exception pc
        //     + 1: tos at exception point
        // all registers are as at the exception point (except eip & flags)
        const char * name;
        address fn;
        if (id == implicit_div0_exception_id) {
          name = "implicit_div0_exception";
          fn = CAST_FROM_FN_PTR(address, implicit_throw_div0_exception);
        } else {
          name = "implicit_null_exception";
          fn = CAST_FROM_FN_PTR(address, implicit_throw_null_exception);
        }

        __ set_info(name, dont_gc_arguments);
        __ enter();
        const int num_rt_args = 1; // thread
        OopMap* map = save_caller_save_registers(sasm, num_rt_args);
        int call_offset = __ call_RT(noreg, noreg, fn);
        __ should_not_reach_here();
        oop_maps = new OopMapSet();
        oop_maps->add_gc_map(call_offset, true, map);
      }
      break;

    case interpreter_entries_id:
      { __ set_info("interpreter_entries", dont_gc_arguments);
        __ stop("do not call interpreter entries entry directly");
        // generate entries for all method kinds
        for (int i = 0; i < AbstractInterpreter::number_of_method_entries; i++) {
          AbstractInterpreter::MethodKind kind = (AbstractInterpreter::MethodKind)i;

          // entry point to methods that are final
          int virtual_final_call_offset = __ offset();
          // get ic method
          // tos + 0: return address
          //       1: last argument
          // eax    : compiledICHolderOop
          // ecx    : receiver
          Label continueL;
          __ movl(ebx, Address(eax, compiledICHolderOopDesc::holder_method_offset()));
          __ verify_oop(eax);
          __ verify_oop(ebx);
          __ verify_oop(ecx);
          __ jmp (continueL);

          // entry point for virtual calls
          int virtual_call_offset = __ offset();
          // get ic method & klass
          // tos + 0: return address
          //       1: last argument
          // eax    : compiledICHolderOop
          // ecx    : receiver
          __ movl(edx, Address(eax, compiledICHolderOopDesc::holder_klass_offset ()));
          __ movl(ebx, Address(eax, compiledICHolderOopDesc::holder_method_offset()));
          __ verify_oop(eax);
          __ verify_oop(ebx);
          __ verify_oop(ecx);
          __ verify_oop(edx);
#ifdef ASSERT
          // check that receiver is truly in ecx
          if (VerifyReceiver) {
            Label L;
            __ load_unsigned_word(eax, Address(ebx, methodOopDesc::size_of_parameters_offset()));
            __ cmpl(ecx, Address(esp, eax, Address::times_4));
            __ jcc(Assembler::equal, L);
            __ stop("interpreter IC check: receiver must be in ecx");
            __ bind(L);
          }
#endif // ASSERT
          // do ic check
          // ebx: ic method
          // ecx: receiver
          // edx: ic klass
          __ cmpl(edx, Address(ecx, oopDesc::klass_offset_in_bytes()));
          __ jcc(Assembler::notEqual, entry_for(handle_ic_miss_id));
          // fall through

          __ bind(continueL);
          // entry point for optimized virtual calls
          int optimized_call_offset = __ offset();
          // ebx: methodOop
          // ecx: receiver
          __ cmpl(Address(ebx, methodOopDesc::compiled_code_offset()), NULL);
          __ jcc(Assembler::notEqual, entry_for(handle_wrong_method_id), relocInfo::runtime_call_type);
          __ jmp(AbstractInterpreter::entry_for_kind(kind), relocInfo::runtime_call_type);

          // entry point for static calls
          int static_call_offset = __ offset();
          // ebx: methodOop
          __ cmpl(Address(ebx, methodOopDesc::compiled_code_offset()), NULL);
          __ jcc(Assembler::notEqual, entry_for(handle_wrong_static_method_id), relocInfo::runtime_call_type);
          __ jmp(AbstractInterpreter::entry_for_kind(kind), relocInfo::runtime_call_type);

          // setup entries
          _ientries[i] = iEntries(static_call_offset, optimized_call_offset, virtual_call_offset, virtual_final_call_offset);
        }
      }
      break;

    case stack_overflow_exception_id:
      // All registers are available.
      // First we remove the topmost frame, making the exception state as it
      // looks when it happens at a call.
      __ leave();
      // PC on TOS: it looks as if a call has been made to this routine
      { StubFrame f(sasm, "stack_overflow_exception", dont_gc_arguments);
        __ call_RT(noreg, noreg, CAST_FROM_FN_PTR(address, throw_stack_overflow));
        __ should_not_reach_here();
      }
      break;

    case init_check_patching_id:
      { StubFrame f(sasm, "init_check_patching", dont_gc_arguments);
        // we should set up register map
        oop_maps = generate_patching(sasm, CAST_FROM_FN_PTR(address, init_check_patching));
      }
      break;
      
    case load_klass_patching_id:
      { StubFrame f(sasm, "load_klass_patching", dont_gc_arguments);
        // we should set up register map
        oop_maps = generate_patching(sasm, CAST_FROM_FN_PTR(address, move_klass_patching));
      }
      break;

    case osr_frame_return_id:
      { __ set_info("osr_frame_return", dont_gc_arguments);
        // continuation point after returning from an osr'ed method -
        // the current frame is the interpreter frame in the state when
        // it was osr'ed, with esp & ebp setup correctly and the osr'ed
        // method's result in eax and/or edx or FPU ST0 (=> do not touch
        // these registers!)
        __ movl(ebx, Address(ebp, frame::interpreter_frame_sender_sp_offset * BytesPerWord)); // get sender sp
        __ leave();                              // remove frame anchor
        __ popl(ecx);                            // get return address
        __ movl(esp, ebx);                       // set sp to sender sp
        __ jmp(ecx);                             // return
      }
      break;

    case osr_unwind_exception_id:
      { __ set_info("osr_unwind_exception", dont_gc_arguments);
        // exception handler entry point for an osr'ed method -
        // the current frame is the interpreter frame in the state when
        // it was osr'ed, with esp & ebp setup correctly and the exception
        // in eax => ignore this frame & further unwind the stack and pro-
        // pagate the excepion back
        //
        // note: do not remove the arguments when removing the activation
        //       since the callee may be compiled code which expects the
        //       arguments on the stack when handling the exception at a
        //       call site! (i.e., use the new calling conventions!)
        //
        // remove activation
        __ movl(ebx, Address(ebp, frame::interpreter_frame_sender_sp_offset * BytesPerWord)); // get caller sp
        __ leave();                              // remove frame anchor
        __ popl(edx);                            // get return address
        __ movl(esp, ebx);                       // set sp to caller sp (w/ arguments untouched!)
        // inbetween activations - previous activation type unknown yet
        // compute continuation point - the continuation point expects
        // the following registers set up:
        //
        // eax: exception
        // edx: return address/pc that threw exception
        // esp: expression stack of caller
        // ebp: ebp of caller
        __ pushl(eax);                           // save exception
        __ pushl(edx);                           // save return address
        __ call_VM_leaf(CAST_FROM_FN_PTR(address, SharedRuntime::exception_handler_for_return_address), edx);
        __ movl(ebx, eax);                       // save exception handler
        __ popl(edx);                            // restore return address
        __ popl(eax);                            // restore exception
        // continue at exception handler (return address removed)
        // note: do *not* remove arguments when unwinding the
        //       activation since the caller assumes having
        //       all arguments on the stack when entering the
        //       runtime to determine the exception handler
        //       (GC happens at call site with arguments!)
        // eax: exception
        // ebx: exception handler
        // edx: throwing pc
        __ verify_oop(eax);
        __ jmp(ebx);
      }
      break;

    case illegal_instruction_handler_id:
      { __ set_info("illegal_instruction_handler", dont_gc_arguments);
        const Register java_thread = edi; // callee-saved for VC++
        Label noException, doReturnType, doCallType;

        // make return address slot and build frame
        __ pushl(ebx);
        __ enter();
       
        // save all register state (int, flags, fpu)
        const int num_rt_args = 1;  // thread
        OopMap* map = save_caller_save_registers(sasm, num_rt_args);

        // The following is basically a call_VM. However, we need the precise address of the call in order to
        // generate an oopmap. Hence, we do all the work ourselves.

        // push thread argument and setup last_Java_sp
        __ get_thread(java_thread);
        __ pushl(java_thread);
        __ movl(Address(java_thread, JavaThread::last_Java_sp_offset()), esp);
        __ movl(Address(java_thread, JavaThread::last_Java_fp_offset()), ebp);

        // do the call
        __ call(CAST_FROM_FN_PTR(address, SafepointSynchronize::handle_illegal_instruction_exception), relocInfo::runtime_call_type);
        oop_maps = new OopMapSet();
        oop_maps->add_gc_map(__ offset(), true, map);

        // result of SafepointSynchronize::handle_illegal_instruction_exception:
        //   eax < 0 => deoptimized at *_call_type safepoint; finish call and return to deoptimization stub
        //   eax = 0 => safepoint not at method return; restore registers and continue execution
        //   eax > 0 => safepoint at method return; eax is number of arguments that must be popped

        // remove thread argument
        __ popl(ecx);
        // clear last_Java_sp again
        __ get_thread(java_thread);
        __ movl(Address(java_thread, JavaThread::last_Java_fp_offset()), 0);
        __ movl(Address(java_thread, JavaThread::last_Java_sp_offset()), 0);

        // handle pending exception
        __ cmpl(Address(java_thread, Thread::pending_exception_offset()), 0);
        __ jcc(Assembler::equal, noException);
        restore_caller_save_registers(sasm);
        __ leave();
        __ get_thread(java_thread);
        // get exception oop, exception pc is on TOS 
        __ movl (eax, Address(java_thread, JavaThread::pending_exception_offset())); 
        assert(StubRoutines::forward_exception_entry() != NULL, "must be generated before");  
        __ jmp(StubRoutines::forward_exception_entry(), relocInfo::runtime_call_type);
    
        // no pending exception; check safepoint type
        __ bind(noException);
        __ testl(eax, eax);
        __ jcc(Assembler::negative, doCallType);
        __ jcc(Assembler::notZero, doReturnType);

        // continue normal execution (restore registers and return)
        restore_caller_save_registers(sasm);
        __ leave();
        __ ret(0);

        __ bind(doCallType);
        // deoptimized at a call safepoint but cannot execute the corresponding invoke in the interpreter
        // because its arguments have been consumed; execute the call, whose destination is valid because
        // the receiver was already consumed, and make the call return to the deoptimization stub
        restore_caller_save_registers(sasm);
        __ leave();
        // jump to call destination; receiver was restored above
        __ popl(esi);  // use of esi here implies esi may not be used for passing method arguments!!
        DeoptimizationBlob* deopt_blob = (DeoptimizationBlob*) Runtime1::blob_for(Runtime1::deoptimization_handler_id);
        assert(deopt_blob != NULL, "deoptimization blob must have been created");
        __ pushl((int)deopt_blob->unpack());
        __ jmp(esi);

        __ bind (doReturnType);
        // safepoint was at method return; any method result is in saved registers
        // eax is the size of arguments in bytes that have to be removed; store it as ecx in saved register
        // area before restoring saved registers to prevent overwriting of method result (eax, edx)
        __ movl(Address(esp, ecx_off * BytesPerWord), eax);
        restore_caller_save_registers(sasm);
        __ leave();        // remove frame
        __ popl(ebx);      // get return address
        __ addl(esp, ecx); // remove arguments
        __ jmp(ebx);       // continue in caller

      }
      break;

    case deoptimization_handler_id:
      { __ set_info("deoptimizion_handler", dont_gc_arguments);
        oop_maps = generate_deoptimization_handler(sasm, unpack_exception_offset, frame_size, unpack_reexecute_offset);
      }
      break;

    case check_safepoint_and_suspend_for_native_trans_id:
      {
        // ebx: thread
        // edx: preserve result
        // eax: preserve result (don't worry about oop, it is still a handle)
        __ pushl (eax);
        __ pushl (edx);
        __ pushl (ebx);
        __ call(CAST_FROM_FN_PTR(address, JavaThread::check_safepoint_and_suspend_for_native_trans), relocInfo::runtime_call_type);
        __ popl  (ebx);
        __ popl  (edx);
        __ popl  (eax);
        __ ret   (0);
      }
      break;
    case jvmdi_exception_throw_id:
      { // eax: exception oop
        StubFrame f(sasm, "jvmdi_exception_throw", dont_gc_arguments);
        // Preserve all registers across this potentially blocking call
        const int num_rt_args = 2;  // thread, exception oop
        OopMap* map = save_caller_save_registers(sasm, num_rt_args);
        int call_offset = __ call_RT(noreg, noreg, CAST_FROM_FN_PTR(address, Runtime1::post_jvmdi_exception_throw), eax);
        oop_maps = new OopMapSet();
        oop_maps->add_gc_map(call_offset, true, map);
        restore_caller_save_registers(sasm);
      }
      break;
    case jvmpi_method_entry_id:
      { // eax: methodOop; ecx: receiver or null
        StubFrame f(sasm, "jvmpi_method_entry", dont_gc_arguments);
        __ call_RT(noreg, noreg, CAST_FROM_FN_PTR(address, SharedRuntime::jvmpi_method_entry), eax, ecx);
      }
      break;
    case jvmpi_method_exit_id:
      { // eax: methodOop; ecx: method's oop result or null
        // This stub is frameless so that its compiled call site doesn't need debug info and
        // can't be deoptimized, since the compiled frame will be viewed as a stub frame.
        __ set_info("jvmpi_method_exit", dont_gc_arguments);
        // preserve potential oop result in current thread
        __ get_thread(ebx);
        __ movl(Address(ebx, JavaThread::vm_result_offset()), ecx);
        __ call_RT(eax, noreg, CAST_FROM_FN_PTR(address, SharedRuntime::jvmpi_method_exit), eax);
        // eax is now the method's oop result, possibly updated by GC, or null
        __ ret(0);
      }
      break;

    case alignment_frame_return_id:
      {
        __ leave();
        __ ret(0);
      }
      break;

    case f2i_is32bit_stub_id:
    case f2i_not32bit_stub_id:
      __ enter();
      __ movl (Address(esp, -(int)os::vm_page_size()), eax);
      __ pushl(esi);
      __ pushl(edi);
      __ pushl(ecx);
      __ pushl(ebx);
      __ pushl(edx);
      __ pushl(edx); // make room for parameter
      __ fstp_s(Address(esp));
      __ call(CAST_FROM_FN_PTR(address, SharedRuntime::f2i), relocInfo::runtime_call_type);
      __ popl (edx); // discard parameter
      __ popl (edx);
      __ popl (ebx);
      __ popl (ecx);
      __ popl (edi);
      __ popl (esi);
      if (id == f2i_is32bit_stub_id) {
        __ fldcw(Address((int)StubRoutines::addr_fpu_cntrl_wrd_24(), relocInfo::none));
      } else {
        __ fldcw(Address((int)StubRoutines::addr_fpu_cntrl_wrd_std(), relocInfo::none));
      }
      
      // result is in eax
      __ leave();
      __ ret(0);
      break;

    case f2l_stub_id:
    case d2l_stub_id:
      {
        // eax and edx are destroyed, but should be free since the result is returned there
        // preserve esi,ecx
        __ pushl(esi);
        __ pushl(ecx);
        
        // check for NaN
        Label return0, do_return, return_min_jlong, do_convert;
        
        Address value_high_word(esp, 8);
        Address value_low_word(esp, 4);
        Address result_high_word(esp, 16);
        Address result_low_word(esp, 12);
        
        __ subl(esp, 20);
        __ fst_d(value_low_word);
        __ movl(eax, value_high_word);
        __ andl(eax, 0x7ff00000);
        __ cmpl(eax, 0x7ff00000);
        __ jcc(Assembler::notEqual, do_convert);
        __ movl(eax, value_high_word);
        __ andl(eax, 0xfffff);
        __ orl(eax, value_low_word);
        __ jcc(Assembler::notZero, return0);
        
        __ bind(do_convert);
        __ fnstcw(Address(esp));
        __ movzxw(eax, Address(esp));
        __ orl(eax, 0xc00);
        __ movw(Address(esp, 2), eax);
        __ fldcw(Address(esp, 2));
        __ fwait();
        __ fistp_d(result_low_word);
        __ fldcw(Address(esp));
        __ fwait();
        __ movl(eax, result_low_word);
        __ movl(edx, result_high_word);
        __ movl(ecx, eax);
        __ xorl(ecx, 0x0);
        __ movl(esi, 0x80000000);
        __ xorl(esi, edx);
        __ orl(ecx, esi);
        __ jcc(Assembler::notEqual, do_return);
        __ fldz();
        __ fcomp_d(value_low_word);
        __ fnstsw_ax();
        __ sahf();
        __ jcc(Assembler::above, return_min_jlong);
        // return max_jlong
        __ movl(edx, 0x7fffffff);
        __ movl(eax, 0xffffffff);
        __ jmp(do_return);
        
        __ bind(return_min_jlong);
        __ movl(edx, 0x80000000);
        __ xorl(eax, eax);
        __ jmp(do_return);
        
        __ bind(return0);
        __ fpop();
        __ xorl(edx,edx);
        __ xorl(eax,eax);
        
        __ bind(do_return);
        __ addl(esp, 20);
        __ popl(ecx);
        __ popl(esi);
        __ ret(0);
      }
      break;
      
    case ldiv_stub_id:
    case lrem_stub_id:
      {
        Label div_by_zero;
        if (id == lrem_stub_id) {
          __ set_info("lrem", dont_gc_arguments);
        } else {
          __ set_info("ldiv", dont_gc_arguments);
        }
        __ enter();
        
        // preserve unused registers
        __ pushl(esi);
        __ pushl(edi);
        
        // check for 0
        __ movl(esi, ebx);
        __ orl(esi, ecx);
        __ jcc(Assembler::zero, div_by_zero);
        
        // pass arguments to call
        __ pushl(edx);
        __ pushl(eax);
        __ pushl(ecx);
        __ pushl(ebx);
        if (id == lrem_stub_id) {
          __ call(CAST_FROM_FN_PTR(address, SharedRuntime::lrem), relocInfo::runtime_call_type);
        } else {
          __ call(CAST_FROM_FN_PTR(address, SharedRuntime::ldiv), relocInfo::runtime_call_type);
        }
        // get rid of arguments
        __ addl(esp,16);
        // restore registers
        __ popl(edi);
        __ popl(esi);
        __ leave();
        __ ret(0);
        
        __ bind(div_by_zero);
        // correct stack
        __ popl(edi);
        __ popl(esi);
        generate_exception_throw(sasm, CAST_FROM_FN_PTR(address, throw_div0_exception));
      }
      break;

    default:
      { StubFrame f(sasm, "unimplemented entry", dont_gc_arguments);
        __ movl(eax, (int)id);
        __ call_RT(noreg, noreg, CAST_FROM_FN_PTR(address, unimplemented_entry), eax);
        __ should_not_reach_here();
      }
      break;
  }
  return oop_maps;
}

#undef __


JRT_ENTRY(void, Runtime1::prepare_interpreter_call(JavaThread* thread, methodOop method))
  // not used for i486
  Unimplemented();
JRT_END


void GC_Support::preserve_callee_argument_oops(const frame fr, symbolHandle signature, bool is_static, OopClosure* f) {
  // not used for i486
  Unimplemented();
}


jint* GC_Support::get_argument_addr_at(const frame fr, ArgumentLocation location) {
  // not used for i486
  Unimplemented();
  return NULL;
}


jint* GC_Support::get_stack_argument_addr_at(const frame fr, int offset_from_sp_in_words) {
  // not used for i486
  Unimplemented();
  return NULL;
}


jint* GC_Support::get_register_argument_addr_at(const frame fr, Register r) {
  // not used for i486
  Unimplemented();
  return NULL;
}


bool DeoptimizationBlob::exception_address_is_unpack_entry(address pc) const {
  DeoptimizationBlob* deopt_blob = (DeoptimizationBlob*) Runtime1::blob_for(Runtime1::deoptimization_handler_id);
  return (pc == deopt_blob->unpack());
}

int vframeArray::extend_caller_frame(int callee_locals, int callee_parameters) {
  // for i486, extend SP of caller to accomodate interpreted callee's locals
  return callee_locals - callee_parameters;
}

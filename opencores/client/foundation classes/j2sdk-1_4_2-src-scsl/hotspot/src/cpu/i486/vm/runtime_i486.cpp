#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)runtime_i486.cpp	1.78 03/03/28 19:12:41 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_runtime_i486.cpp.incl"

DeoptimizationBlob *OptoRuntime::_deopt_blob;
UncommonTrapBlob   *OptoRuntime::_uncommon_trap_blob;
ExceptionBlob      *OptoRuntime::_exception_blob;
SafepointBlob      *OptoRuntime::_illegal_instruction_handler_blob;

#define __ masm->

static void deopt_placeholder(void) { }

//------------------------------generate_deopt_blob----------------------------
// Ought to generate an ideal graph & compile, but here's some Intel ASM
// instead.  Sigh, Cliff.
void OptoRuntime::generate_deopt_blob() {
  // allocate space for the code
  ResourceMark rm;
  // setup code generation tools
  CodeBuffer*   buffer = new CodeBuffer(512, 512, 0, 0, 0, false);
  MacroAssembler* masm = new MacroAssembler(buffer);

  // Capture info about frame layout
  enum layout { arg0_off,       // used for thread
                arg1_off,       // used for sp
                arg2_off,       // used for has_exception
                eax_off,        // slot for java return value
                edx_off,        // slot for java return value
                fp0_off,        // slot for java return value
                fp0_off_2,      // slot for java return value
                xmm0_off,       // slot for java return value
                xmm0_off_2,     // slot for java return value
                ret_type_off,   // slot for return type
                exception_off,  // slot for has_exception
		ebp_off,        // slot for frame pointer
                return_off,     // slot for return address
	        framesize };

  OopMapSet *oop_maps = new OopMapSet();

  // -------------
  // This code enters when returning to a de-optimized frame.  The return
  // address has been popped from the stack, and return values are in
  // registers.  At this point we need to de-opt.  We save the argument return
  // registers.  We call the first C routine, fetch_unroll_info().  This
  // routine captures the return values and returns a structure which
  // describes the current frame size and the sizes of all replacement frames.
  // The current frame is compiled code and may contain many inlined
  // functions, each with their own JVM state.  We pop the current frame, then
  // push all the new frames.  Then we call the C routine unpack_frames() to
  // populate these frames.  Finally unpack_frames() returns us the new target
  // address.  Notice that callee-save registers are BLOWN here; they have
  // already been captured in the vframeArray at the time the return PC was
  // patched.
  address start = __ pc();
  Label cont;

  // Add empty oopmap at begining. This can both be at_call and not_at_call,
  // Normally it would be at call. However, it could be at not_at_call, if the
  // thread happended to be blocked at safepoint_node in the frame that got
  // deoptimized.
  // This ought to go aways now that ebp is not return to us in this manner
  OopMap* map = new OopMap( framesize, 0 );
  // First oopmap needs to record EBP so's link_offset() can find it
  map->set_callee_saved( SharedInfo::stack2reg(ebp_off), framesize,0, OptoReg::Name(EBP_num) );
  oop_maps->add_gc_map( 0, true, map );
  oop_maps->add_gc_map( 0, false, new OopMap( framesize, 0) );

  __ subl(esp,framesize*wordSize); // Prolog for non exception case!
  __ movl(Address(esp,exception_off*wordSize), Deoptimization::Unpack_deopt);
  __ jmp(cont);
  
  int exception_offset = __ pc() - start;

  __ subl(esp,framesize*wordSize);	// Prolog for exception case
  __ movl(Address(esp,exception_off*wordSize), Deoptimization::Unpack_exception); 

  __ bind(cont);

  // Save return-value registers
  __ movl(Address(esp,eax_off*wordSize),eax);
  __ movl(Address(esp,edx_off*wordSize),edx);
  __ fstp_d(Address(esp,fp0_off  *wordSize)); // Pop floating point stack and store into local
                                       // If tos is empty, NaN is saved
  if( UseSSE==2 ) __ movsd(Address(esp,xmm0_off*wordSize), xmm0);
  if( UseSSE==1 ) __ movss(Address(esp,xmm0_off*wordSize), xmm0);

  // Compiled code leaves the floating point stack dirty, empty it.
  __ empty_FPU_stack();

  // Make stack walkable so fetch_unroll_info can find the current sender of the deoptee
  // Use the initial pc and that way it looks like the original deoptimzed frame
  __ leal(ebp, Address((int)start, relocInfo::internal_word_type));
  __ movl(Address(esp, return_off * wordSize), ebp); 
  __ leal(ebp, Address(esp,   ebp_off * wordSize));  // Make ebp chain circular?
  __ movl(Address(esp,   ebp_off * wordSize), ebp);  // save current ebp
  // Use an address that looks decent in the debugger


  // Call C code.  Need thread and this frame, but NOT official VM entry
  // crud.  We cannot block on this call, no GC can happen.  
  __ get_thread(ecx);
  __ movl(Address(esp, arg0_off*wordSize),ecx);
  // fetch_unroll_info needs to call last_java_frame()
  __ movl(Address(ecx, JavaThread::last_Java_sp_offset()), esp);
  // __ movl(Address(ecx, JavaThread::last_Java_fp_offset()), ebp);
  __ call( CAST_FROM_FN_PTR(address, Deoptimization::fetch_unroll_info), relocInfo::runtime_call_type );
  __ get_thread(ecx);
  __ movl(Address(ecx, JavaThread::last_Java_sp_offset()), 0);
  __ movl(Address(ecx, JavaThread::last_Java_fp_offset()), 0);

  // Load UnrollBlock into EDI
  __ movl(edi, eax);

  // Move the unpack kind to a safe place in the UnrollBlock because
  // we are very short of registers

  Address unpack_kind(edi, Deoptimization::UnrollBlock::unpack_kind_offset_in_bytes());
  __ movl(eax, Address(esp,exception_off*wordSize));
  __ movl(unpack_kind, eax);                      // save the unpack_kind value

  // Store (possible live values) frame locals in registers
  __ movl(eax, Address(esp,eax_off*wordSize));
  __ movl(edx, Address(esp,edx_off*wordSize));
  __ fld_d(Address(esp,fp0_off*wordSize));    // Push on float stack
  if( UseSSE==2 ) __ movsd(xmm0,Address(esp,xmm0_off*wordSize));
  if( UseSSE==1 ) __ movss(xmm0,Address(esp,xmm0_off*wordSize));



  // Pop all the frames we must move/replace. 
  // 
  // Frame picture (youngest to oldest)
  // 1: self-frame (no frame link)
  // 2: deopting frame  (no frame link)
  // 3: possible-i2c-adapter-frame 
  // 4: caller of deopting frame (could be compiled/interpreted. If interpreted we will create an
  //    and c2i here)

  // Pop self-frame.  We have no frame, and must rely only on EAX and ESP.
  __ addl(esp,(framesize-1)*wordSize);     // Epilog!

  // Pop deoptimized frame
  __ addl(esp,Address(edi,Deoptimization::UnrollBlock::size_of_deoptimized_frame_offset_in_bytes()));

  // Pop I2C adapter frame, if any
  __ addl(esp,Address(edi,Deoptimization::UnrollBlock::adapter_size_offset_in_bytes()));

  // sp should be pointing at the return address to the caller (4)

  // Load array of frame pcs into ECX
  __ movl(ecx,Address(edi,Deoptimization::UnrollBlock::frame_pcs_offset_in_bytes()));

  __ popl(esi); // trash the old pc

  // Load array of frame sizes into ESI
  __ movl(esi,Address(edi,Deoptimization::UnrollBlock::frame_sizes_offset_in_bytes()));

  Address counter(edi, Deoptimization::UnrollBlock::counter_temp_offset_in_bytes());

  __ movl(ebx, Address(edi, Deoptimization::UnrollBlock::number_of_frames_offset_in_bytes()));
  __ movl(counter, ebx);

  // Pick up the initial fp we should save
  __ movl(ebp, Address(edi, Deoptimization::UnrollBlock::initial_fp_offset_in_bytes()));


  Label adapter_done;

  // Do we need an adapter frame?

  __ cmpl(Address(edi, Deoptimization::UnrollBlock::new_adapter_offset_in_bytes()), 0);
  __ jcc (Assembler::zero, adapter_done);

  // Push a c2i adapter frame
  __ pushl(Address(ecx));                             // push new pc
  __ movl(ebx, Address(esi));                         // get framesize (includes return address)
  __ subl(ebx, wordSize);                             // account for return address
  __ decl(counter);                                   // count the frame
  __ addl(esi, 4);                                    // Bump array pointer (sizes)
  __ addl(ecx, 4);                                    // Bump array pointer (pcs)
  __ subl(esp, ebx);                                  // allocate adapter frame
  __ bind(adapter_done);

  // Now adjust the caller's stack to make up for the extra locals
  // but record the original sp so that we can save it in the skeletal interpreter
  // frame and the stack walking of interpreter_sender will get the unextended sp
  // value and not the "real" sp value.

  Address sp_temp(edi, Deoptimization::UnrollBlock::sender_sp_temp_offset_in_bytes());
  __ movl(sp_temp, esp);
  __ subl(esp, Address(edi, Deoptimization::UnrollBlock::caller_adjustment_offset_in_bytes()));

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
  __ movl(Address(esp,eax_off*wordSize),eax);
  __ movl(Address(esp,edx_off*wordSize),edx);
  __ fstp_d(Address(esp,fp0_off*wordSize));   // Pop float stack and store in local
  __ movl(eax, unpack_kind);                     // get the unpack_kind value
  if( UseSSE==2 ) __ movsd(Address(esp,xmm0_off*wordSize), xmm0);
  if( UseSSE==1 ) __ movss(Address(esp,xmm0_off*wordSize), xmm0);
  __ movl(Address(esp, exception_off * wordSize), eax);

  // Store whether the top interpreter frame expects a float or double.
  __ movl(eax, Address(edi,Deoptimization::UnrollBlock::return_type_offset_in_bytes()));
  __ movl(Address(esp, ret_type_off*wordSize),eax);

  // set last_Java_sp, last_Java_fp
  __ get_thread(ecx);
  __ movl(Address(ecx, JavaThread::last_Java_sp_offset()), esp);
  __ movl(Address(ecx, JavaThread::last_Java_fp_offset()), ebp);

  // Call C code.  Need thread but NOT official VM entry
  // crud.  We cannot block on this call, no GC can happen.  Call should
  // restore return values to their stack-slots with the new SP.
  __ movl(Address(esp,arg0_off*wordSize),ecx);
  __ movl(eax, Address(esp,exception_off*wordSize)); 
  __ movl(Address(esp, arg1_off*wordSize),eax);
  __ call( CAST_FROM_FN_PTR(address, Deoptimization::unpack_frames), relocInfo::runtime_call_type );

  // Set an oopmap for the call site
  oop_maps->add_gc_map( __ pc()-start, true, new OopMap( framesize, 0 ));

  __ get_thread(ecx);
  // reset last Java frame
  // we must set sp to zero to clear frame
  __ movl(Address(ecx, JavaThread::last_Java_sp_offset()), 0);
  // must clear fp, so that compiled frames are not confused; it is possible
  // that we need it only for debugging
  __ movl(Address(ecx, JavaThread::last_Java_fp_offset()), 0);

  // Collect return values
  __ movl(eax,Address(esp,eax_off*wordSize));
  __ movl(edx,Address(esp,edx_off*wordSize));

  // Clear floating point stack before returning to interpreter
  __ empty_FPU_stack();

  // Check if we should push the float or double return value.
  Label no_double_value, yes_double_value;
  __ cmpl(Address(esp, ret_type_off*wordSize), T_DOUBLE);
  __ jcc (Assembler::zero, yes_double_value);
  __ cmpl(Address(esp, ret_type_off*wordSize), T_FLOAT);
  __ jcc (Assembler::notZero, no_double_value);

  if( UseSSE>=1 ) __ fld_s(Address(esp,xmm0_off*wordSize)); 
  else            __ fld_d(Address(esp,fp0_off *wordSize)); // Push on float stack
  __ jmp(no_double_value);

  __ bind(yes_double_value);
  if( UseSSE==2 ) __ fld_d(Address(esp,xmm0_off*wordSize)); 
  else            __ fld_d(Address(esp,fp0_off *wordSize)); // Push on float stack
  __ bind(no_double_value);

  // Pop self-frame.
  __ leave();                              // Epilog!

  // Jump to interpreter
  __ ret(0);
  
  // -------------
  // make sure all code is generated
  masm->flush();

  _deopt_blob = DeoptimizationBlob::create( buffer, oop_maps, 0, exception_offset, 0, framesize);
}


//------------------------------generate_uncommon_trap_blob--------------------
// Ought to generate an ideal graph & compile, but here's some Intel ASM
// instead.  Sigh, Cliff.
void OptoRuntime::generate_uncommon_trap_blob() {
  // allocate space for the code
  ResourceMark rm;
  // setup code generation tools
  CodeBuffer*   buffer = new CodeBuffer(512, 512, 0, 0, 0, false);
  MacroAssembler* masm = new MacroAssembler(buffer);

  enum frame_layout {
    arg0_off,      // thread                     sp + 0 // Arg location for 
    arg1_off,      // unloaded_class_index       sp + 1 // calling C
    xmm6_off,      // callee saved register      sp + 2
    xmm6_off_2,    // callee saved register      sp + 3
    xmm7_off,      // callee saved register      sp + 4
    xmm7_off_2,    // callee saved register      sp + 5
    edi_off,       // callee saved register      sp + 6
    esi_off,       // callee saved register      sp + 7
    ebp_off,       // callee saved register      sp + 8
    return_off,    // slot for return address    sp + 9
    framesize
  };
  
  address start = __ pc();
  // Push self-frame.
  __ subl(esp,return_off*wordSize);     // Epilog!

  // Save callee saved registers.  None for UseSSE=0, 
  // floats-only for UseSSE=1, and doubles for UseSSE=2.
  if( OptoRuntimeCalleeSavedFloats ) {
    if( UseSSE == 1 ) {
      __ movss(Address(esp,xmm6_off*wordSize),xmm6);
      __ movss(Address(esp,xmm7_off*wordSize),xmm7);
    } else if( UseSSE == 2 ) {
      __ movsd(Address(esp,xmm6_off*wordSize),xmm6);
      __ movsd(Address(esp,xmm7_off*wordSize),xmm7);
    }
  }
  __ movl(Address(esp,ebp_off  *wordSize),ebp);
  __ movl(Address(esp,edi_off  *wordSize),edi);
  __ movl(Address(esp,esi_off  *wordSize),esi);

  // Clear the floating point exception stack
  __ empty_FPU_stack();

  // set last_Java_sp
  __ get_thread(edx);
  __ movl(Address(edx, JavaThread::last_Java_sp_offset()), esp);

  // Call C code.  Need thread but NOT official VM entry
  // crud.  We cannot block on this call, no GC can happen.  Call should
  // capture callee-saved registers as well as return values.
  __ movl(Address(esp, arg0_off*wordSize),edx);
  // argument already in ECX 
  __ movl(Address(esp, arg1_off*wordSize),ecx);
  __ call(CAST_FROM_FN_PTR(address, Deoptimization::uncommon_trap), relocInfo::runtime_call_type);

  // Set an oopmap for the call site
  OopMapSet *oop_maps = new OopMapSet();
  OopMap* map =  new OopMap( framesize, 0 );
  // oop_map->set_callee(
  if( OptoRuntimeCalleeSavedFloats ) {
    map->set_callee_saved( SharedInfo::stack2reg(xmm6_off  ), framesize,0, OptoReg::Name(XMM6a_num) );
    map->set_callee_saved( SharedInfo::stack2reg(xmm6_off+1), framesize,0, OptoReg::Name(XMM6b_num) );
    map->set_callee_saved( SharedInfo::stack2reg(xmm7_off  ), framesize,0, OptoReg::Name(XMM7a_num) );
    map->set_callee_saved( SharedInfo::stack2reg(xmm7_off+1), framesize,0, OptoReg::Name(XMM7b_num) );
  }
  map->set_callee_saved( SharedInfo::stack2reg(ebp_off   ), framesize,0, OptoReg::Name(EBP_num  ) );
  map->set_callee_saved( SharedInfo::stack2reg(edi_off   ), framesize,0, OptoReg::Name(EDI_num  ) );
  map->set_callee_saved( SharedInfo::stack2reg(esi_off   ), framesize,0, OptoReg::Name(ESI_num  ) );
  oop_maps->add_gc_map( __ pc()-start, true, map);

  __ get_thread(ecx);

  // reset last Java frame
  // we must set sp to zero to clear frame
  __ movl(Address(ecx, JavaThread::last_Java_sp_offset()), 0);
  // must clear fp, so that compiled frames are not confused; it is possible
  // that we need it only for debugging
  __ movl(Address(ecx, JavaThread::last_Java_fp_offset()), 0);

  // Load UnrollBlock into EDI
  __ movl(edi, eax);

  // Pop all the frames we must move/replace. 
  // 
  // Frame picture (youngest to oldest)
  // 1: self-frame (no frame link)
  // 2: deopting frame  (no frame link)
  // 3: possible-i2c-adapter-frame 
  // 4: caller of deopting frame (could be compiled/interpreted. If interpreted we will create an
  //    and c2i here)

  // Pop self-frame.  We have no frame, and must rely only on EAX and ESP.
  __ addl(esp,(framesize-1)*wordSize);     // Epilog!

  // Pop deoptimized frame
  __ addl(esp,Address(edi,Deoptimization::UnrollBlock::size_of_deoptimized_frame_offset_in_bytes()));

  // Pop I2C adapter frame, if any
  __ addl(esp,Address(edi,Deoptimization::UnrollBlock::adapter_size_offset_in_bytes()));

  // sp should be pointing at the return address to the caller (4)

  // Load array of frame pcs into ECX
  __ movl(ecx,Address(edi,Deoptimization::UnrollBlock::frame_pcs_offset_in_bytes()));

  __ popl(esi); // trash the pc

  // Load array of frame sizes into ESI
  __ movl(esi,Address(edi,Deoptimization::UnrollBlock::frame_sizes_offset_in_bytes()));

  Address counter(edi, Deoptimization::UnrollBlock::counter_temp_offset_in_bytes());

  __ movl(ebx, Address(edi, Deoptimization::UnrollBlock::number_of_frames_offset_in_bytes()));
  __ movl(counter, ebx);

  // Pick up the initial fp we should save
  __ movl(ebp, Address(edi, Deoptimization::UnrollBlock::initial_fp_offset_in_bytes()));

  Label adapter_done;

  // Do we need an adapter frame?

  __ cmpl(Address(edi, Deoptimization::UnrollBlock::new_adapter_offset_in_bytes()), 0);
  __ jcc (Assembler::zero, adapter_done);

  // Push a c2i adapter frame
  __ pushl(Address(ecx));                             // push new pc
  __ movl(ebx, Address(esi));                         // get framesize (includes return address)
  __ subl(ebx, wordSize);                             // account for return address
  __ decl(counter);                                   // count the frame
  __ subl(esp, ebx);                                  // allocate adapter frame
  __ addl(esi, 4);                                    // Bump array pointer (sizes)
  __ addl(ecx, 4);                                    // Bump array pointer (pcs)
  __ bind(adapter_done);

  // Now adjust the caller's stack to make up for the extra locals
  // but record the original sp so that we can save it in the skeletal interpreter
  // frame and the stack walking of interpreter_sender will get the unextended sp
  // value and not the "real" sp value.

  Address sp_temp(edi, Deoptimization::UnrollBlock::sender_sp_temp_offset_in_bytes());
  __ movl(sp_temp, esp);
  __ subl(esp, Address(edi, Deoptimization::UnrollBlock::caller_adjustment_offset_in_bytes()));

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


  // set last_Java_sp, last_Java_fp
  __ get_thread(edi);
  __ movl(Address(edi, JavaThread::last_Java_sp_offset()), esp);
  __ movl(Address(edi, JavaThread::last_Java_fp_offset()), ebp);

  // Call C code.  Need thread but NOT official VM entry
  // crud.  We cannot block on this call, no GC can happen.  Call should
  // restore return values to their stack-slots with the new SP.
  __ movl(Address(esp,arg0_off*wordSize),edi);
  __ movl(Address(esp,arg1_off*wordSize), Deoptimization::Unpack_uncommon_trap); 
  __ call(CAST_FROM_FN_PTR(address, Deoptimization::unpack_frames), relocInfo::runtime_call_type);
  // Set an oopmap for the call site
  oop_maps->add_gc_map( __ pc()-start, true, new OopMap( framesize, 0 ) );

  __ get_thread(edi);
  // reset last Java frame
  // we must set sp to zero to clear frame
  __ movl(Address(edi, JavaThread::last_Java_sp_offset()), 0);
  // must clear fp, so that compiled frames are not confused; it is possible
  // that we need it only for debugging
  __ movl(Address(edi, JavaThread::last_Java_fp_offset()), 0);

  // Pop self-frame.
  __ leave();     // Epilog!

  // Jump to interpreter
  __ ret(0);

  // -------------
  // make sure all code is generated
  masm->flush();

  _uncommon_trap_blob = UncommonTrapBlob::create(buffer, oop_maps, framesize);
}


//------------------------------fill_in_exception_blob-------------------------
// Ought to generate an ideal graph & compile, but here's some Intel ASM
// instead.  Sigh, Cliff.
int handle_exception_deopt_offset = 0;
int handle_exception_deopt_exception_offset = 0;
int handle_exception_deopt_offset_2 = 0;
int handle_exception_deopt_exception_offset_2 = 0;
int handle_exception_call_pc_offset = 0;

void OptoRuntime::fill_in_exception_blob() {  
  enum layout { 
    thread_off,                 // last_java_sp                
    ebp_off,                
    edi_off,         
    esi_off,
    xmm6_off,                   // callee saved register
    xmm6_off_2,                 // callee saved register
    xmm7_off,                   // callee saved register
    xmm7_off_2,                 // callee saved register
    return_off,                 // slot for return address
    framesize 
  };

   // Patch blob
  assert(handle_exception_stub() != NULL, "exception stub must have been generated");
  assert(handle_exception_call_pc_offset != 0, "");
  assert(handle_exception_deopt_exception_offset != 0, "");
  assert(handle_exception_deopt_offset != 0, "");
  assert(handle_exception_deopt_exception_offset_2 != 0, "");
  assert(handle_exception_deopt_offset_2 != 0, "");

  {
  NativeMovConstReg* move = nativeMovConstReg_at(exception_blob()->instructions_begin() + handle_exception_deopt_offset);
  move->set_data((int)OptoRuntime::deoptimization_blob()->unpack());

  NativeMovConstReg* move2 = nativeMovConstReg_at(exception_blob()->instructions_begin() + handle_exception_deopt_exception_offset);
  move2->set_data((int)OptoRuntime::deoptimization_blob()->unpack_with_exception());
  }
  {
  NativeMovConstReg* move = nativeMovConstReg_at(exception_blob()->instructions_begin() + handle_exception_deopt_offset_2);
  move->set_data((int)OptoRuntime::deoptimization_blob()->unpack());

  NativeMovConstReg* move2 = nativeMovConstReg_at(exception_blob()->instructions_begin() + handle_exception_deopt_exception_offset_2);
  move2->set_data((int)OptoRuntime::deoptimization_blob()->unpack_with_exception());
  }
  // Set an oopmap for the call site.  This oopmap will only be used if we
  // are unwinding the stack.  Hence, all locations will be dead.
  // Callee-saved registers will be the same as the frame above (i.e.,
  // handle_exception_stub), since they were restored when we got the
  // exception.
  ResourceMark rm;
  OopMapSet *oop_maps = new OopMapSet();
  OopMap* map =  new OopMap( framesize, 0 );        
  map->set_callee_saved( SharedInfo::stack2reg(ebp_off   ), framesize, 0, OptoReg::Name(EBP_num  ) );
  map->set_callee_saved( SharedInfo::stack2reg(edi_off   ), framesize, 0, OptoReg::Name(EDI_num  ) );
  map->set_callee_saved( SharedInfo::stack2reg(esi_off   ), framesize, 0, OptoReg::Name(ESI_num  ) );
  if( OptoRuntimeCalleeSavedFloats ) {
    map->set_callee_saved( SharedInfo::stack2reg(xmm6_off  ), framesize, 0, OptoReg::Name(XMM6a_num) );
    map->set_callee_saved( SharedInfo::stack2reg(xmm6_off+1), framesize, 0, OptoReg::Name(XMM6b_num) );
    map->set_callee_saved( SharedInfo::stack2reg(xmm7_off  ), framesize, 0, OptoReg::Name(XMM7a_num) );
    map->set_callee_saved( SharedInfo::stack2reg(xmm7_off+1), framesize, 0, OptoReg::Name(XMM7b_num) );
  }
  oop_maps->add_gc_map( handle_exception_call_pc_offset, true, map);
  exception_blob()->set_oop_maps(oop_maps);
}


//------------------------------setup_exception_blob---------------------------
// creates exception blob at the end
// Using exception blob, this code is jumped from a compiled method.
// (see emit_exception_handler in i486.ad file)
// It is also the default exception destination for c2i and i2c adapters
// 
// Given an exception pc at a call, we may unwind the stack and jump to
// caller's exception handler. We may also continue into the handler
// This code is entered with a jmp.
// 
// Arguments:
//   eax: exception oop
//   edx: exception pc
//
// Results:
//   eax: exception oop
//   edx: exception pc in caller or ???
//   destination: exception handler of caller
// 
// Note: the exception pc MUST be at a call (precise debug information)
//       Only register eax, edx, ecx are not callee saved.
//

void OptoRuntime::setup_exception_blob() {
  assert(!OptoRuntime::is_callee_saved_register(EDX_num), "");
  assert(!OptoRuntime::is_callee_saved_register(EAX_num), "");
  assert(!OptoRuntime::is_callee_saved_register(ECX_num), "");

  // Capture info about frame layout  
  enum layout { 
    thread_off,                 // last_java_sp                
    ebp_off,                
    edi_off,         
    esi_off,
    xmm6_off,                   // callee saved register
    xmm6_off_2,                 // callee saved register
    xmm7_off,                   // callee saved register
    xmm7_off_2,                 // callee saved register
    return_off,                 // slot for return address
    framesize 
  };

  // allocate space for the code
  ResourceMark rm;
  // setup code generation tools  
  CodeBuffer*   buffer = new CodeBuffer(512, 512, 0, 0, 0, false);
  MacroAssembler* masm = new MacroAssembler(buffer);


  Label exception_handler_found;

  address start = __ pc();  

  __ pushl(edx);
  __ subl(esp, return_off * wordSize);   // Prolog!

  // Save callee saved registers.  None for UseSSE=0, 
  // floats-only for UseSSE=1, and doubles for UseSSE=2.
  if( OptoRuntimeCalleeSavedFloats ) {
    if( UseSSE == 1 ) {
      __ movss(Address(esp,xmm6_off*wordSize),xmm6);
      __ movss(Address(esp,xmm7_off*wordSize),xmm7);
    } else if( UseSSE == 2 ) {
      __ movsd(Address(esp,xmm6_off*wordSize),xmm6);
      __ movsd(Address(esp,xmm7_off*wordSize),xmm7);
    }
  }
  __ movl(Address(esp,ebp_off  *wordSize),ebp);
  __ movl(Address(esp,edi_off  *wordSize),edi);
  __ movl(Address(esp,esi_off  *wordSize),esi);
          
  // Store exception in Thread object. We cannot pass any arguments to the
  // handle_exception call, since we do not want to make any assumption
  // about the size of the frame where the exception happened in.
  __ get_thread(ecx);
  __ movl(Address(ecx, JavaThread::exception_oop_offset()), eax);
  __ movl(Address(ecx, JavaThread::exception_pc_offset()),  edx);

  // This call does all the hard work.  It checks if an exception handler
  // exists in the method.  
  // If so, it returns the handler address.
  // If not, it prepares for stack-unwinding, restoring the callee-save 
  // registers of the frame being removed.
  //  
  __ movl(Address(esp, thread_off * wordSize), ecx); // Thread is first argument
  __ movl(Address(ecx, JavaThread::last_Java_sp_offset()), esp);
  __ call(CAST_FROM_FN_PTR(address, OptoRuntime::handle_exception_C), relocInfo::runtime_call_type);
  handle_exception_call_pc_offset = __ pc() - start;
  __ get_thread(ecx);
  __ movl(Address(ecx, JavaThread::last_Java_sp_offset()), 0);


  // Restore callee-saved registers
  if( OptoRuntimeCalleeSavedFloats ) {
    if( UseSSE == 1 ) {
      __ movss(xmm6,Address(esp,xmm6_off*wordSize));
      __ movss(xmm7,Address(esp,xmm7_off*wordSize));
    } else if( UseSSE == 2 ) {
      __ movsd(xmm6,Address(esp,xmm6_off*wordSize));
      __ movsd(xmm7,Address(esp,xmm7_off*wordSize));
    }
  }
  __ movl(ebp, Address(esp, ebp_off * wordSize));
  __ movl(edi, Address(esp, edi_off * wordSize));
  __ movl(esi, Address(esp, esi_off * wordSize));
  __ addl(esp, return_off * wordSize);   // Epilog!
  __ popl(edx); // Exception pc


  // eax: 0 if no exception handler for given <exception oop/exception pc>
  // found
  __ testl(eax, eax);
  __ jcc(Assembler::notZero, exception_handler_found);    

  // No exception handler found. We unwind stack one level.
  // All information is stored in thread.
  // thread->exception_oop
  // thread->exception_handler_pc
  // thread->exception_pc;
  // thread->exception_stack_size

  // The registers ebp, edi and esi contain the addresses of their values
  // load the values into the callee saved registers (only if register is non-zero)

  Label skipEBP, skipEDI, skipESI;
  __ testl(ebp, ebp);
  __ jcc  (Assembler::zero, skipEBP);
  __ movl (ebp, Address(ebp));
  __ bind (skipEBP);

  __ testl(edi, edi);
  __ jcc  (Assembler::zero, skipEDI);
  __ movl (edi, Address(edi));
  __ bind (skipEDI);

  __ testl(esi, esi);
  __ jcc  (Assembler::zero, skipESI);
  __ movl (esi, Address(esi));
  __ bind (skipESI);

  // We have to check if the caller frame has been deoptimized;
  // If the caller frame has been deoptimized then we must continue
  // at DeoptimizationBlob::unpack_with_exception address; 
  // If the caller frame is not deoptimized, then we continue at
  // JavaThread::exception_handler_pc_offset_in_bytes.
  // The caller frame state is defined by its return address.

  __ get_thread (ecx);    
  __ movl(edx, Address(ecx, JavaThread::exception_pc_offset()));
  __ movl(eax, Address(ecx, JavaThread::exception_stack_size_offset()));
  //   edx: exception-pc
  //   eax: stack-size
              
  Label endL, deoptL;

  handle_exception_deopt_offset = __ pc() - start;
  __ movl (ecx, (int)-1);  // will be filled in later with fill_in_exception_blob

  __ subl (eax, wordSize); // we need to check the return address, do not remove it from the stack
  __ addl (esp, eax);      // remove frame (excluding return address)

  __ popl (eax);           // get return address to check if the caller frame is deoptimized
  __ cmpl (eax, ecx);
  __ jcc  (Assembler::equal, deoptL);

  __ get_thread(ecx);
  __ movl(eax,Address(ecx,JavaThread::exception_handler_pc_offset()));
  __ jmp (endL);

  __ bind (deoptL);
  handle_exception_deopt_exception_offset = __ pc() - start;
  __ movl (eax, (int)-1);  // will be filled in later with fill_in_exception_blob

  __ bind (endL);
  __ pushl(eax);          // handler-pc of caller is where we continue or the deoptimization blob address
  
  __ get_thread(ecx);
  __ movl(eax, Address(ecx, JavaThread::exception_oop_offset()));   // exception oop 
#ifdef ASSERT
  __ movl(Address(ecx, JavaThread::exception_oop_offset()), 0);
  __ movl(Address(ecx, JavaThread::exception_handler_pc_offset()), 0);
  __ movl(Address(ecx, JavaThread::exception_pc_offset()), 0);
#endif

  // eax: exception oop
  // edx: exception pc
  // return to handler
 
  __ ret(0);

  // Exception handler found.       
  __ bind (exception_handler_found);  
  
  Label deoptL_2, endL_2;

  // check if top frame has been deoptimized; if yes, exception handler must be
  // deoptimized blob
  // address to check is in edx
  handle_exception_deopt_offset_2 = __ pc() - start;
  __ movl (ecx, (int)-1);  // will be filled in later with fill_in_exception_blob
  __ cmpl (edx, ecx);
  __ jcc  (Assembler::equal, deoptL_2);
  __ movl (ecx,eax);             // Move handler address to ECX
  __ jmp  (endL_2);
  __ bind (deoptL_2);
  handle_exception_deopt_exception_offset_2 = __ pc() - start;
  __ movl (ecx, (int)-1);  // will be filled in later with fill_in_exception_blob
  __ bind (endL_2);

  // ecx contains handler address

  __ get_thread(edx);           // TLS
  __ movl(eax, Address(edx, JavaThread::exception_oop_offset()));   // exception oop    
#ifdef ASSERT
  __ movl(Address(edx, JavaThread::exception_handler_pc_offset()), 0);
  __ movl(Address(edx, JavaThread::exception_pc_offset()), 0); 
  __ movl(Address(edx, JavaThread::exception_oop_offset()), 0);
#endif

  // eax: exception oop
  // ecx: exception handler
  // edx: not used 
  __ jmp (ecx);

  // -------------
  // make sure all code is generated
  masm->flush();  

  // Set exception blob
  OopMapSet *oop_maps = NULL; // will be set later; currently the register stuff is not yet initialized!
  _exception_blob = ExceptionBlob::create(buffer, oop_maps, framesize);  
}


//------------------------------generate_illegal_instruction_handler_blob------
//
// Generate a special Compile2Runtime blob that saves all registers, 
// and setup oopmap.
//
void OptoRuntime::generate_illegal_instruction_handler_blob() {
  assert (StubRoutines::forward_exception_entry() != NULL, "must be generated before");  
  enum { FPU_regs_used_by_C2 = 7 /*for the FPU stack*/+8/*eight more for XMM registers*/ };
  // Capture info about frame layout
  enum layout { thread_off      = 0,      // last_java_sp
                fpu_state_off,
                fpu_state_end = fpu_state_off+FPUStateSizeInWords-1,
                fpr1_off, fpr1H_off, // server compiler can pass debug info in 
                fpr2_off, fpr2H_off, // any of the first 7 FPU registers
                fpr3_off, fpr3H_off,
                fpr4_off, fpr4H_off,
                fpr5_off, fpr5H_off,
                fpr6_off, fpr6H_off,
                fpr7_off, fpr7H_off, // FPU_regs_used_by_C2 = 7
                xmm0_off, xmm0H_off, 
                xmm1_off, xmm1H_off, 
                xmm2_off, xmm2H_off, 
                xmm3_off, xmm3H_off, 
                xmm4_off, xmm4H_off, 
                xmm5_off, xmm5H_off, 
                xmm6_off, xmm6H_off, 
                xmm7_off, xmm7H_off, // FPU_regs_used_by_C2 += 8
                flags_off,
                edi_off,         
                esi_off,
                ebp_off,
                esp_off,                
                ebx_off,
                edx_off,
                ecx_off,
                eax_off,            
                return_off,      // slot for return address
	        framesize };

  assert(FPUStateSizeInWords == 27, "update stack layout");

  // allocate space for the code
  Label done, noException;
  ResourceMark rm;
  // setup code generation tools  
  CodeBuffer*   buffer = new CodeBuffer(512, 512, 0, 0, 0, false);
  MacroAssembler* masm = new MacroAssembler(buffer);
  
  const Register java_thread = edi; // callee-saved for VC++
  address start   = __ pc();  
  address call_pc = NULL;  

  // save registers, fpu state, and flags  
  __ pushl(ebx);                // Make room for return address (or push it again)
  __ pushad();        
  __ pushfd();        
  __ subl(esp,FPU_regs_used_by_C2*sizeof(jdouble)); // Push FPU registers space
  __ push_FPU_state();          // Save FPU state & init
  __ frstor(Address(esp));      // Restore state (but keep stack copy)
  
  // The following is basically a call_VM. However, we need the precise
  // address of the call in order to generate an oopmap. Hence, we do all the
  // work outselvs.

  // Push thread argument and setup last_Java_sp
  __ get_thread(java_thread);
  __ pushl(java_thread);
  __ movl(Address(java_thread, JavaThread::last_Java_sp_offset()), esp);

  // Save the FPU registers in de-opt-able form after the last push has
  // finished up the stack frame.
  __ fstp_d(Address(esp, fpr1_off*wordSize)); // FPR1
  __ fstp_d(Address(esp, fpr2_off*wordSize)); // FPR2
  __ fstp_d(Address(esp, fpr3_off*wordSize)); // FPR3
  __ fstp_d(Address(esp, fpr4_off*wordSize)); // FPR4
  __ fstp_d(Address(esp, fpr5_off*wordSize)); // FPR5
  __ fstp_d(Address(esp, fpr6_off*wordSize)); // FPR6
  __ fstp_d(Address(esp, fpr7_off*wordSize)); // FPR7
  __ finit();                   // Reset FPU state for following C code

  if( UseSSE == 1 ) {           // Save the XMM state
    __ movss(Address(esp,xmm0_off*wordSize),xmm0);
    __ movss(Address(esp,xmm1_off*wordSize),xmm1);
    __ movss(Address(esp,xmm2_off*wordSize),xmm2);
    __ movss(Address(esp,xmm3_off*wordSize),xmm3);
    __ movss(Address(esp,xmm4_off*wordSize),xmm4);
    __ movss(Address(esp,xmm5_off*wordSize),xmm5);
    __ movss(Address(esp,xmm6_off*wordSize),xmm6);
    __ movss(Address(esp,xmm7_off*wordSize),xmm7);
  } else if( UseSSE == 2 ) {
    __ movsd(Address(esp,xmm0_off*wordSize),xmm0);
    __ movsd(Address(esp,xmm1_off*wordSize),xmm1);
    __ movsd(Address(esp,xmm2_off*wordSize),xmm2);
    __ movsd(Address(esp,xmm3_off*wordSize),xmm3);
    __ movsd(Address(esp,xmm4_off*wordSize),xmm4);
    __ movsd(Address(esp,xmm5_off*wordSize),xmm5);
    __ movsd(Address(esp,xmm6_off*wordSize),xmm6);
    __ movsd(Address(esp,xmm7_off*wordSize),xmm7);
  }

  // do the call
  __ call(CAST_FROM_FN_PTR(address, SafepointSynchronize::handle_illegal_instruction_exception), relocInfo::runtime_call_type);
  call_pc = __ pc();
  __ movl(ecx, eax); // move result into ecx, as eax may be loaded with the result of return

  // Clear last_Java_sp again
  __ get_thread(java_thread);
  __ movl(Address(java_thread, JavaThread::last_Java_sp_offset()), 0);

  // Recover XMM & FPU state
  if( UseSSE == 1 ) {
    __ movss(xmm0,Address(esp,xmm0_off*wordSize));
    __ movss(xmm1,Address(esp,xmm1_off*wordSize));
    __ movss(xmm2,Address(esp,xmm2_off*wordSize));
    __ movss(xmm3,Address(esp,xmm3_off*wordSize));
    __ movss(xmm4,Address(esp,xmm4_off*wordSize));
    __ movss(xmm5,Address(esp,xmm5_off*wordSize));
    __ movss(xmm6,Address(esp,xmm6_off*wordSize));
    __ movss(xmm7,Address(esp,xmm7_off*wordSize));
  } else if( UseSSE == 2 ) {
    __ movsd(xmm0,Address(esp,xmm0_off*wordSize));
    __ movsd(xmm1,Address(esp,xmm1_off*wordSize));
    __ movsd(xmm2,Address(esp,xmm2_off*wordSize));
    __ movsd(xmm3,Address(esp,xmm3_off*wordSize));
    __ movsd(xmm4,Address(esp,xmm4_off*wordSize));
    __ movsd(xmm5,Address(esp,xmm5_off*wordSize));
    __ movsd(xmm6,Address(esp,xmm6_off*wordSize));
    __ movsd(xmm7,Address(esp,xmm7_off*wordSize));
  }
  __ popl(eax); // Remove thread argument
  __ pop_FPU_state();
  __ addl(esp,FPU_regs_used_by_C2*sizeof(jdouble)); // Pop FPU registers

  __ cmpl(Address(java_thread, Thread::pending_exception_offset()), NULL);
  __ jcc(Assembler::equal, noException);

  __ popfd();
  __ popad();
  __ jmp(StubRoutines::forward_exception_entry(), relocInfo::runtime_call_type);

  // No exception case
  Label negativeL;
  __ bind(noException);
  __ testl (ecx, ecx);
  __ jcc(Assembler::negative, negativeL);

  // Normal exit, register restoring and exit  
  __ bind(done);
  __ popfd();
  __ popad();

  __ ret(0);
  
  // we have deoptimized at a blocked call, we may not reexecute the
  // instruction as we would skip the call in interpreter; therefore
  // execute the destination of the call; the destination is valid
  // because the receiver was already consumed
  __ bind(negativeL);
  __ popfd();
  __ popad();
  // now jump to destination (receiver is restored with popad)
  // esi may not be used for passing arguments!!
  __ popl(ebx);
  DeoptimizationBlob* deopt_blob = OptoRuntime::deoptimization_blob();
  assert(deopt_blob != NULL, "deoptimization blob must have been created");
  __ pushl((int)deopt_blob->unpack());
  __ jmp(ebx);

  // Set an oopmap for the call site.  This oopmap will map all
  // oop-registers and debug-info registers as callee-saved.  This
  // will allow deoptimization at this safepoint to find all possible
  // debug-info recordings, as well as let GC find all oops.

  OopMapSet *oop_maps = new OopMapSet();
  OopMap* map =  new OopMap( framesize, 0 );  
  map->set_callee_saved(SharedInfo::stack2reg(  eax_off), framesize, 0, OptoReg::Name(  EAX_num));
  map->set_callee_saved(SharedInfo::stack2reg(  ecx_off), framesize, 0, OptoReg::Name(  ECX_num));
  map->set_callee_saved(SharedInfo::stack2reg(  edx_off), framesize, 0, OptoReg::Name(  EDX_num));
  map->set_callee_saved(SharedInfo::stack2reg(  ebx_off), framesize, 0, OptoReg::Name(  EBX_num));  
  map->set_callee_saved(SharedInfo::stack2reg(  ebp_off), framesize, 0, OptoReg::Name(  EBP_num));
  map->set_callee_saved(SharedInfo::stack2reg(  esi_off), framesize, 0, OptoReg::Name(  ESI_num));
  map->set_callee_saved(SharedInfo::stack2reg(  edi_off), framesize, 0, OptoReg::Name(  EDI_num));    
  map->set_callee_saved(SharedInfo::stack2reg( fpr1_off), framesize, 0, OptoReg::Name(FPR1L_num));    
  map->set_callee_saved(SharedInfo::stack2reg( fpr2_off), framesize, 0, OptoReg::Name(FPR2L_num)); 
  map->set_callee_saved(SharedInfo::stack2reg( fpr3_off), framesize, 0, OptoReg::Name(FPR3L_num));    
  map->set_callee_saved(SharedInfo::stack2reg( fpr4_off), framesize, 0, OptoReg::Name(FPR4L_num));    
  map->set_callee_saved(SharedInfo::stack2reg( fpr5_off), framesize, 0, OptoReg::Name(FPR5L_num));    
  map->set_callee_saved(SharedInfo::stack2reg( fpr6_off), framesize, 0, OptoReg::Name(FPR6L_num));    
  map->set_callee_saved(SharedInfo::stack2reg( fpr7_off), framesize, 0, OptoReg::Name(FPR7L_num));    
  map->set_callee_saved(SharedInfo::stack2reg(fpr1H_off), framesize, 0, OptoReg::Name(FPR1H_num));    
  map->set_callee_saved(SharedInfo::stack2reg(fpr2H_off), framesize, 0, OptoReg::Name(FPR2H_num)); 
  map->set_callee_saved(SharedInfo::stack2reg(fpr3H_off), framesize, 0, OptoReg::Name(FPR3H_num));    
  map->set_callee_saved(SharedInfo::stack2reg(fpr4H_off), framesize, 0, OptoReg::Name(FPR4H_num));    
  map->set_callee_saved(SharedInfo::stack2reg(fpr5H_off), framesize, 0, OptoReg::Name(FPR5H_num));    
  map->set_callee_saved(SharedInfo::stack2reg(fpr6H_off), framesize, 0, OptoReg::Name(FPR6H_num));    
  map->set_callee_saved(SharedInfo::stack2reg(fpr7H_off), framesize, 0, OptoReg::Name(FPR7H_num));    
  map->set_callee_saved(SharedInfo::stack2reg( xmm0_off), framesize, 0, OptoReg::Name(XMM0a_num));    
  map->set_callee_saved(SharedInfo::stack2reg( xmm1_off), framesize, 0, OptoReg::Name(XMM1a_num));    
  map->set_callee_saved(SharedInfo::stack2reg( xmm2_off), framesize, 0, OptoReg::Name(XMM2a_num));    
  map->set_callee_saved(SharedInfo::stack2reg( xmm3_off), framesize, 0, OptoReg::Name(XMM3a_num));    
  map->set_callee_saved(SharedInfo::stack2reg( xmm4_off), framesize, 0, OptoReg::Name(XMM4a_num));    
  map->set_callee_saved(SharedInfo::stack2reg( xmm5_off), framesize, 0, OptoReg::Name(XMM5a_num));    
  map->set_callee_saved(SharedInfo::stack2reg( xmm6_off), framesize, 0, OptoReg::Name(XMM6a_num));    
  map->set_callee_saved(SharedInfo::stack2reg( xmm7_off), framesize, 0, OptoReg::Name(XMM7a_num));    
  map->set_callee_saved(SharedInfo::stack2reg(xmm0H_off), framesize, 0, OptoReg::Name(XMM0b_num));    
  map->set_callee_saved(SharedInfo::stack2reg(xmm1H_off), framesize, 0, OptoReg::Name(XMM1b_num));    
  map->set_callee_saved(SharedInfo::stack2reg(xmm2H_off), framesize, 0, OptoReg::Name(XMM2b_num));    
  map->set_callee_saved(SharedInfo::stack2reg(xmm3H_off), framesize, 0, OptoReg::Name(XMM3b_num));    
  map->set_callee_saved(SharedInfo::stack2reg(xmm4H_off), framesize, 0, OptoReg::Name(XMM4b_num));    
  map->set_callee_saved(SharedInfo::stack2reg(xmm5H_off), framesize, 0, OptoReg::Name(XMM5b_num));    
  map->set_callee_saved(SharedInfo::stack2reg(xmm6H_off), framesize, 0, OptoReg::Name(XMM6b_num));    
  map->set_callee_saved(SharedInfo::stack2reg(xmm7H_off), framesize, 0, OptoReg::Name(XMM7b_num));    
  oop_maps->add_gc_map( call_pc - start, true, map);

  // -------------
  // make sure all code is generated
  masm->flush();  

  // Fill-out other meta info
  _illegal_instruction_handler_blob = SafepointBlob::create(buffer, oop_maps, framesize);    
}


//------------------------------exception_address_is_unpack_entry--------------
//%note runtime_26
bool DeoptimizationBlob::exception_address_is_unpack_entry(address pc) const {
  return (pc == OptoRuntime::deoptimization_blob()->unpack());
}


//------------------------------pd_unwind_stack--------------------------------
void OptoRuntime::pd_unwind_stack(JavaThread *thread, frame fr, RegisterMap* reg_map) {
  // Update callee-saved registers across frame
  jint* reg_locs[REG_COUNT];
  // Copy values of all callee-saved registers out of frame
  for(int i = 0; i < REG_COUNT; i++) {
    jint* loc = (jint*)reg_map->location(VMReg::Name(i));    
    reg_locs[i] = loc;
  }
  
  // Find oopmap for stub, and update it stack with callee-saved
  // reg. info for next frame.
  frame stub_frame = thread->last_frame();
  CodeBlob* blob = CodeCache::find_blob(stub_frame.pc());
  assert(blob->is_exception_stub(), "sanity check");
  OopMap* map = blob->oop_maps()->singular_oop_map();
  // Update callee-saved registers in stub
  OopMapValue omv;
  for(OopMapStream oms(map,OopMapValue::callee_saved_value); !oms.is_done(); oms.next()) {
    omv = oms.current();
    assert(omv.is_stack_loc(), "sanity check");
    jint* location = (jint*)stub_frame.oopmapreg_to_location(omv.reg(),NULL);
    VMReg::Name reg = omv.content_reg();
    assert(reg >= 0 && reg < REG_COUNT, "reg out of range");
    jint** location2 = (jint**)location;
    *location2 = reg_locs[reg];
  }
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
  
  // Create oopmap for osr adapter. All it contains is where to find the
  // link offset (ebp) on windows.
  int link_offset = ((frame_size - frame::sender_sp_offset) + frame::link_offset);

  OopMapSet *oop_maps = new OopMapSet();
  OopMap* map =  new OopMap(frame_size, 0 );
  map->set_callee_saved(OptoReg::Name(SharedInfo::stack0 + link_offset), frame_size, 0, OptoReg::Name(EBP_num));
  oop_maps->add_gc_map(0, true, map);

  // Empty all except FPR0 in case of float/double returns
  if( !returning_fp ) masm->ffree(0);
  for (int i = 1; i<8; i++ ) masm->ffree(i);

  if (UseC2CallingConventions) {
    masm->movl(ecx, Address(ebp, frame::interpreter_frame_sender_sp_offset * wordSize)); // get sender sp
    masm->leave();                                // remove frame anchor
    masm->popl(esi);                              // get return address
    masm->movl(esp, ecx);                         // set sp to sender sp
  } else {
    masm->movl(ecx, Address(ebp, frame::interpreter_frame_locals_offset * wordSize)); // get sender sp
    masm->leave();                                // remove frame anchor
    masm->popl(esi);                              // get return address
    masm->leal(esp, Address(ecx, wordSize));      // set sp to sender sp
  }
  masm->jmp(esi);  
  masm->flush();

  return OSRAdapter::new_osr_adapter(cb, oop_maps, frame_size);
}


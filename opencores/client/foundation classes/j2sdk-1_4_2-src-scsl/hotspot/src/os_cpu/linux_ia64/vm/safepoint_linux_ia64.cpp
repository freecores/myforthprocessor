#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)safepoint_linux_ia64.cpp	1.12 03/01/27 08:28:19 JVM"
#endif

# include "incls/_precompiled.incl"
# include "incls/_safepoint_linux_ia64.cpp.incl"

/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

bool SafepointSynchronize::can_be_at_safepoint_before_suspend(JavaThread *thread, JavaThreadState state) {
  switch(state) {
   // blocked threads and native threads should have already saved state
   case _thread_in_native:
     return !thread->has_last_Java_frame() || thread->frame_anchor()->walkable();
   case _thread_blocked:
   case _thread_in_native_blocked:
     assert(!thread->has_last_Java_frame() || thread->frame_anchor()->walkable(), "blocked and not walkable");
     return true;

   default:
    ShouldNotReachHere();
  }
}


bool SafepointSynchronize::can_be_at_safepoint_after_suspend(JavaThread *thread, JavaThreadState state) {
  switch(state) {
   case _thread_in_vm_trans:
     // threads in vm code have not yet saved state
     return false;

   default:
    ShouldNotReachHere();
  }
}

void SafepointSynchronize::save_thread_state_at_safepoint(JavaThread *thread) {
  // We flush windows in the trampoline or by hand when we block
  // nothing to do here.
}

// We always need code generated to check for a deopt suspend because top frame
// has volatile state
bool SafepointSynchronize::needs_deopt_suspend(void) {
  return false;
}

bool SafepointSynchronize::needs_deopt_suspend(JavaThread *thread) {
  return false;
}

#ifndef CORE

void SafepointSynchronize::do_deopt_suspend(JavaThread *thread) {
  ShouldNotReachHere();
}

#endif // !CORE

// The patching of the return instruction is machine-specific
void SafepointSynchronize::patch_return_instruction_md(address cb_pc) {
#if 0
  // The relocation info. is actually after the ret
  // (i.e., where we are going to patch with an illegal instruction.
  cb_pc -= NativeReturn::instruction_size;
  
  assert(((NativeInstruction*)cb_pc)->is_return(), "must be a return instruction");
  
  // In C1  code is part of ret eventually
  NOT_COMPILER1(assert(((NativeInstruction*)(cb_pc+1))->is_nop(), "must be a nop instruction");)
  NOT_COMPILER1(assert(((NativeInstruction*)(cb_pc+2))->is_nop(), "must be a nop instruction");)
  assert(NativePopReg::instruction_size + NativeIllegalInstruction::instruction_size == 3, "wrong code format");
  NativePopReg::insert(cb_pc, ebx); // Pop return address (it is getting pushed by the SafepointBlob again)
  NativeIllegalInstruction::insert(cb_pc + NativePopReg::instruction_size);        
#endif
}

#ifndef CORE

int CompiledCodeSafepointHandler::pd_thread_code_buffer_size(nmethod* nm) {
  return nm->instructions_size();
}

void CompiledCodeSafepointHandler::pd_patch_runtime_calls_with_trampolines(ThreadCodeBuffer* cb,
                                                                           int offset_of_first_trampoline) {
  // Nothing to do (yet) -- Itanium currently uses long calls in all cases
}

#endif // !CORE

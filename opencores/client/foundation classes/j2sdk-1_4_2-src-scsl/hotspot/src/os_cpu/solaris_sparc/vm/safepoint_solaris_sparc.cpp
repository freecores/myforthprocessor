#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)safepoint_solaris_sparc.cpp	1.7 03/01/23 11:10:32 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_safepoint_solaris_sparc.cpp.incl"

bool SafepointSynchronize::can_be_at_safepoint_before_suspend(JavaThread *thread, JavaThreadState state) {
  switch(state) {
   case _thread_in_native:
     // threads in native code have not yet saved state
     return false;

   // New thread may have resources during creation. (ie. malloc lock)
   case _thread_new:
     return false;

   // blocked threads have saved state
   // new threads have no state to save
   case _thread_blocked:
   case _thread_in_native_blocked:
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

// The patching of the return instruction is machine-specific
void SafepointSynchronize::patch_return_instruction_md(address cb_pc) {
  // In the case of SPARC the relocation info. is on the ret instruction
  // which is where we are going to patch
  assert(((NativeInstruction*)cb_pc)->is_return(), "must be a return instruction");
  assert(((NativeInstruction*)(cb_pc+NativeInstruction::nop_instruction_size))->is_nop(), "delay slot must be a nop instruction");
  NativeIllegalInstruction::insert(cb_pc);
}

void SafepointSynchronize::save_thread_state_at_safepoint(JavaThread *thread) {
  if (thread->is_vm_suspended() && thread->has_last_Java_frame()) {
    // cause pd state to be cached
    // the state saver recognizes that since we are suspended that our
    // register windows are flushed
    (void)thread->last_frame();
  }
}


// We always need code generated to check for a deopt suspend because top frame
// has volatile state
bool SafepointSynchronize::needs_deopt_suspend(void) {
#if 0 /* not until Tiger */
  return true;
#else
  return false;
#endif /* Tiger */
}

bool SafepointSynchronize::needs_deopt_suspend(JavaThread *thread) {
#if 0 /* not until Tiger */
  JavaThreadState state = thread->thread_state();
  // Seems like _thread_in_native_blocked should be able to return false but
  // better safe than sorry.
  return state == _thread_in_native || state == _thread_in_native_trans || state == _thread_in_native_blocked;
#else
  return false;
#endif /* Tiger */
}

#ifndef CORE

void SafepointSynchronize::do_deopt_suspend(JavaThread *thread) {
#if 0 /* not until Tiger */
  // Determine if the deopt modified the return address in the
  // java/native transition frame. If it did then we must flush
  // the windows again. Update the memory copy with the correct
  // return address.
  // In any case we must clear the deopt_suspend flag for the thread.
  MutexLockerEx ml(thread->SR_lock(), Mutex::_no_safepoint_check_flag);
  assert(thread->has_deopt_suspension(), "Expected deopt suspended");
  thread->clear_deopt_suspended();
  RegisterMap map(thread, false);
  frame fr = thread->last_frame().sender(&map);
  vframeArray* head = thread->vframe_array_head();
  assert(head != NULL, "Must have a deopt pending");
  // This is the frame before we patched it. It has original pc
  frame deoptee = head->original();
  if (deoptee.id() == fr.id() ) {
    if (TraceDeoptimization) {
      tty->print_cr("Deoptimization: possible stale return address case");
      // Force a flush now  and rewalk to see if we actually caught the window
      StubRoutines::sparc::flush_callers_register_windows_func()();
      RegisterMap map(thread, false);
      frame now = thread->last_frame().sender(&map);
      if (now.pc() != fr.pc()) {
	tty->print_cr("Deoptimization: fixed actual stale return address");
      }
    }
    // We have a winner!
    // Make the callee of the deoptimized frame return to the runtime routine that will
    // use the vframeArray to transform the compiled frame into interpreted frames.
    // 
    // Note:  fr_patch... here will cause a window flush before it updates the pc
    // so the transition hole is closed.
    COMPILER1_ONLY(fr.patch_for_deoptimization(thread, callee_fr, Runtime1::entry_for(Runtime1::deoptimization_handler_id));)
    COMPILER2_ONLY(fr.patch_pc(thread, OptoRuntime::deoptimize_repack_stack());)
  }
#else
  ShouldNotReachHere();
#endif /* Tiger */
}


static const int TRAMPOLINE_SIZE = 8 * jintSize;

int CompiledCodeSafepointHandler::pd_thread_code_buffer_size(nmethod* nm) {
  int scale_factor = 1;
#ifdef _LP64
  // Worst case is that every other instruction is a runtime call.
  // Could alternatively iterate through all relocation information to
  // compute precise size, but not worth it.
  scale_factor += (TRAMPOLINE_SIZE / 2);
#endif

  return scale_factor * nm->instructions_size();
}

void CompiledCodeSafepointHandler::pd_patch_runtime_calls_with_trampolines(ThreadCodeBuffer* cb,
                                                                           int offset_of_first_trampoline) {
#ifdef _LP64
#define __ masm.

  nmethod* nm = cb->method();

  // Round offset up to 8-byte boundary
  int cur_trampoline_offset = (int) align_size_up(offset_of_first_trampoline, wordSize);

  RelocIterator iter(nm);
  while(iter.next()) {
    address reloc_pc = iter.addr();
    if (reloc_pc >= nm->code_end())
    break;
    
    address cb_pc = cb->code_begin() + (reloc_pc - nm->instructions_begin());
    assert(cb->contains(cb_pc), "pc must be in temp. buffer");            

    switch(iter.type()) {	
      case relocInfo::runtime_call_type:
        {
          if (ShowSafepointMsgs && Verbose) { 
            tty->print_cr("trampolining runtime call at " INTPTR_FORMAT " (org. pc: " INTPTR_FORMAT ")", cb_pc, reloc_pc); 
          }
          Register tmp_reg = O7;
          // Replace offset of call instruction with that of trampoline, which
          // will fabricate the real 64-bit destination address and jump to it.
          // Contents of trampoline (32-bit words):
          //   dest address hi
          //   dest address lo
          //   save
          //   sethi %tmp_reg, [dest address offset wrt %i7]
          //   add   %tmp_reg, [dest address offset wrt %i7]
          //   ld    [%i7 + %tmp_reg], [%tmp_reg]
          //   jmpl  %tmp_reg, 0, %g0
          //   restore
          NativeCall* real_runtime_call = nativeCall_at(reloc_pc);
          address real_destination_pc = real_runtime_call->destination();
          NativeCall* patched_runtime_call = nativeCall_at(cb_pc);
          CodeBuffer trampoline_cb(cb->code_begin() + cur_trampoline_offset, TRAMPOLINE_SIZE);
          MacroAssembler masm(&trampoline_cb);
          intptr_t dest_addr_offset_in_bytes = (__ pc()) - cb_pc;
          assert(dest_addr_offset_in_bytes < cb->size(), "must be within ThreadCodeBuffer");
          assert(((uintptr_t) __ pc()) % wordSize == 0, "must be aligned");
          __ emit_data(((uintptr_t) real_destination_pc) >> 32);
          __ emit_data((uintptr_t) real_destination_pc);
          patched_runtime_call->set_destination(trampoline_cb.code_end());
          __ save_frame(0);
          __ sethi(dest_addr_offset_in_bytes & ~0x3ff, tmp_reg);
          __ add(tmp_reg, dest_addr_offset_in_bytes & 0x3ff, tmp_reg);
          __ ldx(I7, tmp_reg, tmp_reg);
          __ jmpl(tmp_reg, 0, G0);
          __ delayed()->restore();
          assert(trampoline_cb.code_end() - (cb->code_begin() + cur_trampoline_offset) == TRAMPOLINE_SIZE,
                 "miscalculation of trampoline size");
          assert(trampoline_cb.code_end() - cb->code_begin() < cb->size(),
                 "trampolines must fit within ThreadCodeBuffer");
          cur_trampoline_offset += TRAMPOLINE_SIZE;
          break;
        }
        
      default:
        break;
    }
  }
#undef __
#endif /* _LP64 */
}

#endif // !CORE

#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)c1_CodePatterns_i486.cpp	1.25 03/01/23 10:52:37 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_c1_CodePatterns_i486.cpp.incl"


void C1_CodePatterns::inline_cache_check(Register receiver, Register iCache) {
  verify_oop(receiver);
  // explicit NULL check not needed since load from [klass_offset] causes a trap  
  // check against inline cache
  assert(!MacroAssembler::needs_explicit_null_check(oopDesc::klass_offset_in_bytes()), "must add explicit null check");
  cmpl(iCache, Address(receiver, oopDesc::klass_offset_in_bytes())); 
  // if icache check fails, then jump to runtime routine
  // Note: RECEIVER must still contain the receiver!
  jcc(Assembler::notEqual, Runtime1::entry_for(Runtime1::handle_ic_miss_id), relocInfo::runtime_call_type); 
}


void C1_CodePatterns::method_exit(bool restore_frame) {
  if (restore_frame) {
    leave();
  }
  address ret_addr = pc();
  ret(0);
  // for compatibility reasons with compiler 2, we must emit ret_start + 1
  // as pc address; safepoint handling is substracting one from that relocInfo (!!!)
  // we do not need oop maps, and pc/bci mapping here
  code()->relocate(ret_addr + 1, relocInfo::return_type);
  // add two nops so that return can be patched for compiler safepoint
  nop ();
  nop ();
}


void C1_CodePatterns::build_frame(int frame_size_in_bytes) {
  enter();

  // Make sure there is enough stack space for this method's activation.
  generate_stack_overflow_check(frame_size_in_bytes);
  decrement(esp, frame_size_in_bytes); // does not emit code for frame_size == 0
}


void C1_CodePatterns::exception_handler(bool has_Java_exception_handler, int frame_size_in_bytes) {
  if (has_Java_exception_handler) {
#ifdef ASSERT
    // make sure the frame size hasn't changed during execution of this method.
    // this code can go away once we're sure we've removed all the pushes and
    // pops in our generated code.
    movl(esi, esp);
    addl(esi, frame_size_in_bytes);
    cmpl(ebp, esi);
    Label ok;
    jcc(Assembler::equal, ok);
    stop("stacks don't match");
    should_not_reach_here();
    bind(ok);
#endif // ASSERT
    // exception handler exist
    // => check if handler for particular exception exists,
    //    continue at exception handler if so, return otherwise
    // eax: exception
    // edx: throwing pc 
    call(Runtime1::entry_for(Runtime1::handle_exception_id), relocInfo::runtime_call_type);
    // exception handler for particular exception doesn't exist
    // => unwind activation and forward exception to caller
    // eax: exception
  } else {
    // There is no handler in this method, we continue in caller
    // reset the implicit flag.
    // Note: comment why do we need to reset the implicit  flag
    get_thread(esi);
    movl(Address(esi,JavaThread::is_handling_implicit_exception_offset()), 0);
  }
}


// Tries fast Object.hashCode access; 
void C1_CodePatterns::fast_ObjectHashCode(Register receiver, Register result) {
  Label slowCase;
  assert(result != receiver, "do not destroy receiver");
  Register hdr = result; // will also be the result
  movl(hdr, Address(receiver, oopDesc::mark_offset_in_bytes()));

  // check if locked
  testl (hdr, markOopDesc::unlocked_value);
  jcc (Assembler::zero, slowCase);

  // get hash
  andl (hdr, markOopDesc::hash_mask_in_place);
  // test if hashCode exists
  jcc  (Assembler::zero, slowCase);
  shrl (result, markOopDesc::hash_shift);
  method_exit(false);
  bind (slowCase);
}


void C1_CodePatterns::unverified_entry(Register receiver, Register ic_klass) {
  if (C1Breakpoint) int3();
  inline_cache_check(receiver, ic_klass);
}


void C1_CodePatterns::verified_entry() {
  if (C1Breakpoint)int3();
  // build frame
  verify_FPU(0, "method_entry");
}



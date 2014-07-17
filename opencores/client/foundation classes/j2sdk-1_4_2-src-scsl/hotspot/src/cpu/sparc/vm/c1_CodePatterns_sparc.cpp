#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)c1_CodePatterns_sparc.cpp	1.27 03/01/23 10:59:30 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_c1_CodePatterns_sparc.cpp.incl"

void C1_CodePatterns::inline_cache_check(Register receiver, Register iCache) {
  Label L;
  const Register temp_reg = G3_scratch;
  // Note: needs more testing of out-of-line vs. inline slow case
  Address ic_miss(temp_reg, Runtime1::entry_for(Runtime1::handle_ic_miss_id));
  verify_oop(receiver);
  ld_ptr(receiver, oopDesc::klass_offset_in_bytes(), temp_reg);
  cmp(temp_reg, iCache);
  brx(Assembler::equal, true, Assembler::pt, L);
  delayed()->nop();
  jump_to(ic_miss, 0);
  delayed()->nop();
  align(CodeEntryAlignment);
  bind(L);
}


void C1_CodePatterns::method_exit(bool restore_frame) {
  // this code must be structured this way so that the return
  // instruction can be a safepoint.
  if (restore_frame) {
    restore();
  }
  relocate(relocInfo::return_type);
  retl();
  delayed()->nop();
}


void C1_CodePatterns::explicit_null_check(Register base) {
  Unimplemented();
}


void C1_CodePatterns::build_frame(int frame_size_in_bytes) {

  generate_stack_overflow_check(frame_size_in_bytes);
  // Create the frame.
  save_frame_c1(frame_size_in_bytes);
}


void C1_CodePatterns::exception_handler(bool has_Java_exception_handler, int frame_size_in_bytes) {
  verify_oop(Oexception);
  if (has_Java_exception_handler) {
    // O0: exception
    // O1: issuing pc
    call(Runtime1::entry_for(Runtime1::handle_exception_id), relocInfo::runtime_call_type);
    delayed()->nop();
    // exception handler for particular exception doesn't exist
    // => unwind activation and forward exception to caller
    // O0: exception
  } else {
    // There is no handler in this method, we continue in caller
    // reset the implicit flag
    stw(G0, G2_thread, in_bytes(JavaThread::is_handling_implicit_exception_offset()));
  }
  // unlock the receiver/klass if necessary
  // O0: exception
  verify_oop(Oexception);
}


void C1_CodePatterns::fast_ObjectHashCode(Register receiver, Register result) {
  Label SlowerPath;
  Register header               = Gtemp;
  Register hash                 = Gtemp;  // overwrite header value with hash value
  Register mask                 = G1;  // to get hash field from header

  // Get the receiver's register.  Use outgoing_reg_location() because no SAVE has been issued.
  if (receiver == noreg) return;   // Don't bother with this optimization when the receiver is passed in memory.

  // Read the header and build a mask to get its hash field.  Give up if the object is not unlocked.
  // We depend on hash_mask being at most 32 bits and avoid the use of
  // hash_mask_in_place because it could be larger than 32 bits in a 64-bit
  // vm: see markOop.hpp.
  ld_ptr(receiver, oopDesc::mark_offset_in_bytes(), header);
  sethi(markOopDesc::hash_mask, mask);
  btst(markOopDesc::unlocked_value, header);
  br(Assembler::zero, false, Assembler::pn, SlowerPath);
  delayed()->or3(mask, low10(markOopDesc::hash_mask), mask);

  // Check for a valid (non-zero) hash code and get its value.
#ifdef _LP64
  srlx(header, markOopDesc::hash_shift, hash);
#else
  srl(header, markOopDesc::hash_shift, hash);
#endif
  andcc(hash, mask, hash);
  br(Assembler::equal, false, Assembler::pn, SlowerPath);
  delayed()->nop();

  // Safepoint leaf return.
  mov(hash, result);
  method_exit(false);
  bind(SlowerPath);
}


void C1_CodePatterns::unverified_entry(Register receiver, Register ic_klass) {
  if (C1Breakpoint) breakpoint_trap();
  inline_cache_check(receiver, ic_klass);
}


void C1_CodePatterns::verified_entry() {
  if (C1Breakpoint) breakpoint_trap();
  // build frame
  verify_FPU(0, "method_entry");
}




#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)c1_MacroAssembler_sparc.cpp	1.38 03/01/23 11:00:06 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_c1_MacroAssembler_sparc.cpp.incl"

void C1_MacroAssembler::lock_object(Register Rmark, Register Roop, Register Rbox, Register Rscratch, Label& slow_case) {
  assert_different_registers(Rmark, Roop, Rbox, Rscratch);

  Label done;

  Address mark_addr(Roop, 0, oopDesc::mark_offset_in_bytes());

  // The following move must be the first instruction of emitted since debug
  // information may be generated for it.
  // Load object header
  ld_ptr(mark_addr, Rmark);

  verify_oop(Roop);

#ifdef ASSERT
  {
    Label ok;

    ld_ptr(mark_addr, Rmark);
    cmp(Rbox, Rmark);
    brx(Assembler::notEqual, false, Assembler::pt, ok);
    delayed()->nop();
    stop("bad recursive lock");
    should_not_reach_here();
    bind(ok);
  }
#endif

  // Save Rbox in Rscratch to be used for the cas operation
  mov(Rbox, Rscratch);
 
  // and mark it unlocked
  or3(Rmark, markOopDesc::unlocked_value, Rmark);

  // save unlocked object header into the displaced header location on the stack
  st_ptr(Rmark, Rbox, BasicLock::displaced_header_offset_in_bytes());
  // save object being locked into the BasicObjectLock
  st_ptr(Roop, Rbox, BasicObjectLock::obj_offset_in_bytes());

  // compare object markOop with Rmark and if equal exchange Rscratch with object markOop
  assert(mark_addr.disp() == 0, "cas must take a zero displacement");
  casx_under_lock(mark_addr.base(), Rmark, Rscratch, (address)StubRoutines::sparc::atomic_memory_operation_lock_addr());
  // if compare/exchange succeeded we found an unlocked object and we now have locked it
  // hence we are done
  cmp(Rmark, Rscratch);
#ifdef _LP64
  sub(Rscratch, STACK_BIAS, Rscratch);
#endif
  brx(Assembler::equal, false, Assembler::pt, done);
  delayed()->sub(Rscratch, SP, Rscratch);  //pull next instruction into delay slot
  // we did not find an unlocked object so see if this is a recursive case
  // sub(Rscratch, SP, Rscratch);
  assert(os::vm_page_size() > 0xfff, "page size too small - change the constant");
  andcc(Rscratch, 0xfffff003, Rscratch);
  br(Assembler::notZero, false, Assembler::pn, slow_case);
  delayed()->st_ptr(Rscratch, Rbox, BasicLock::displaced_header_offset_in_bytes());
  bind(done);
}


void C1_MacroAssembler::unlock_object(Register Rmark, Register Roop, Register Rbox, Label& slow_case) {
  assert_different_registers(Rmark, Roop, Rbox);

  Label done;
                         
  // Test first it it is a fast recursive unlock
  ld_ptr(Rbox, BasicLock::displaced_header_offset_in_bytes(), Rmark);
  br_null(Rmark, false, Assembler::pt, done);
  delayed()->nop();
  // load object
  ld_ptr(Rbox, BasicObjectLock::obj_offset_in_bytes(), Roop);
  verify_oop(Roop);
                         
  // Check if it is still a light weight lock, this is is true if we see
  // the stack address of the basicLock in the markOop of the object
  Address mark_addr(Roop, 0, oopDesc::mark_offset_in_bytes());
  assert(mark_addr.disp() == 0, "cas must take a zero displacement");
  casx_under_lock(mark_addr.base(), Rbox, Rmark, (address)StubRoutines::sparc::atomic_memory_operation_lock_addr());
  cmp(Rbox, Rmark);

  brx(Assembler::notEqual, false, Assembler::pn, slow_case);
  delayed()->nop();
  // Done
  bind(done);       
}


void C1_MacroAssembler::try_allocate(
  Register obj,                        // result: pointer to object after successful allocation
  Register var_size_in_bytes,          // object size in bytes if unknown at compile time; invalid otherwise
  int      con_size_in_bytes,          // object size in bytes if   known at compile time
  Register t1,                         // temp register
  Register t2,                         // temp register
  Register t3,                         // temp register
  int      alignment_mask,             // alignment mask, must be power of 2 minus 1
  Label&   slow_case                   // continuation point if fast allocation fails
){
  // make sure arguments make sense
  assert_different_registers(obj, var_size_in_bytes, t1, t2, t3);
  assert(0 <= con_size_in_bytes && is_simm13(con_size_in_bytes), "illegal object size");
  assert((con_size_in_bytes & alignment_mask) == 0, "object size is not multiple of alignment");

  if (UseTLAB) {
    const Register top  = obj;
    const Register end  = t1;
    const Register free = t2;

    ld_ptr(G2_thread, in_bytes(JavaThread::tlab_top_offset()), top);
    ld_ptr(G2_thread, in_bytes(JavaThread::tlab_end_offset()), end);

    // calculate amount of free space
    sub(end, top, free);
    
    if (var_size_in_bytes->is_valid()) {
      // size is unknown at compile time
      cmp(free, var_size_in_bytes);
      br(Assembler::lessUnsigned, false, Assembler::pn, slow_case);
      delayed()->add(top, var_size_in_bytes, end);
    } else {
      cmp(free, con_size_in_bytes);
      br(Assembler::lessUnsigned, false, Assembler::pn, slow_case); 
      delayed()->add(top, con_size_in_bytes, end);
    }
    st_ptr(end, G2_thread, in_bytes(JavaThread::tlab_top_offset()));
  } else {
    // get eden boundaries
    // note: we need both top & top_addr!
    const Register top_addr = t1;
    const Register lim      = t2;

    CollectedHeap* ch = Universe::heap();
    assert(ch->supports_inline_contig_alloc(), "Assuming inline allocation");
    set((intx)ch->top_addr(), top_addr);
    intx delta = (intx)ch->end_addr() - (intx)ch->top_addr();
    ld_ptr(top_addr, delta, lim);
    ld_ptr(top_addr, 0, obj);

    // try to allocate
    { Label retry;
      bind(retry);
  #ifdef ASSERT
      // make sure eden top is properly aligned
      { Label L;
        btst(alignment_mask, obj);
        br(Assembler::zero, false, Assembler::pt, L);
        delayed()->nop();
        stop("eden top is not properly aligned");
        bind(L);
      }
  #endif // ASSERT
      const Register end  = t3;
      const Register free = end;
      sub(lim, obj, free);                                   // compute amount of free space
      if (var_size_in_bytes->is_valid()) {
        // size is unknown at compile time
        cmp(free, var_size_in_bytes);
        br(Assembler::lessUnsigned, false, Assembler::pn, slow_case); // if there is not enough space go the slow case
        delayed()->add(obj, var_size_in_bytes, end);
      } else {
        // size is known at compile time
        cmp(free, con_size_in_bytes);
        br(Assembler::lessUnsigned, false, Assembler::pn, slow_case); // if there is not enough space go the slow case
        delayed()->add(obj, con_size_in_bytes, end);
      }
      // Compare obj with the value at top_addr; if still equal, swap the value of
      // end with the value at top_addr. If not equal, read the value at top_addr
      // into end.
      casx_under_lock(top_addr, obj, end, (address)StubRoutines::sparc::atomic_memory_operation_lock_addr());
      // if someone beat us on the allocation, try again, otherwise continue
      cmp(obj, end);
      brx(Assembler::notEqual, false, Assembler::pn, retry);
      delayed()->mov(end, obj);                              // nop if successfull since obj == end
    }
  }
}


void C1_MacroAssembler::initialize_header(Register obj, Register klass, Register len, Register tmp) {
  assert_different_registers(obj, klass, len, tmp);
                     set((intx)markOopDesc::prototype(), tmp);
                     st_ptr(tmp  , obj, oopDesc::mark_offset_in_bytes       ());
                     st_ptr(klass, obj, oopDesc::klass_offset_in_bytes      ());
  if (len->is_valid()) st(len  , obj, arrayOopDesc::length_offset_in_bytes());
}


void C1_MacroAssembler::initialize_body(Register base, Register index) {
  assert_different_registers(base, index);
  Label loop;
  bind(loop);
  subcc(index, wordSize, index);
  brx(Assembler::greaterEqual, true, Assembler::pt, loop);
  delayed()->st_ptr(G0, base, index);
}


void C1_MacroAssembler::allocate_object(
  Register obj,                        // result: pointer to object after successful allocation
  Register t1,                         // temp register
  Register t2,                         // temp register
  Register t3,                         // temp register
  int      hdr_size,                   // object header size in words
  int      obj_size,                   // object size in words 
  Register klass,                      // object klass
  Label&   slow_case                   // continuation point if fast allocation fails
) {
  assert_different_registers(obj, t1, t2, t3, klass);

  // determine alignment mask
  assert(!(BytesPerWord & 1), "must be a multiple of 2 for masking code to work");
  const int am = (ALIGN_ALL_OBJECTS ? 2 : 1) * BytesPerWord - 1;
  
  // allocate space & initialize header
  if (!is_simm13(obj_size * wordSize)) {
    // would need to use extra register to load
    // object size => go the slow case for now
    br(Assembler::always, false, Assembler::pt, slow_case);
    delayed()->nop();
    return;
  }
  try_allocate(obj, noreg,obj_size * wordSize, t1, t2, t3, am, slow_case);
  initialize_header(obj, klass, noreg, t1);

  // initialize body
  const int threshold = 5;                                 // approximate break even point for code size
  if (VM_Version::v9_instructions_work() && obj_size < threshold * 2) {
    // on v9 we can do double word stores to fill twice as much space.
#ifndef _LP64
    assert(hdr_size % 2 == 0, "double word aligned");
    assert(obj_size % 2 == 0, "double word aligned");
#endif
    for (int i = hdr_size; i < obj_size; i+=(longSize/BytesPerWord)) stx(G0, obj, i*BytesPerWord);
  } else if (obj_size <= threshold) {
    // use explicit NULL stores
    for (int i = hdr_size; i < obj_size; i++) st_ptr(G0, obj, i*BytesPerWord);
  } else if (obj_size > hdr_size) {
    // use a loop
    const Register base  = t1;
    const Register index = t2;
    add(obj, hdr_size * wordSize, base);               // compute address of first element
    set((obj_size - hdr_size) * wordSize, index);      // compute index = number of words to clear
    initialize_body(base, index);
  }
}


void C1_MacroAssembler::allocate_array(
  Register obj,                        // result: pointer to array after successful allocation
  Register len,                        // array length
  Register t1,                         // temp register
  Register t2,                         // temp register
  Register t3,                         // temp register
  Register t4,                         // temp register
  int      hdr_size,                   // object header size in words
  int      elt_size,                   // element size in bytes 
  Register klass,                      // object klass
  Label&   slow_case                   // continuation point if fast allocation fails
) {
  assert_different_registers(obj, len, t1, t2, t3, t4, klass);

  // determine alignment mask
  assert(!(BytesPerWord & 1), "must be a multiple of 2 for masking code to work");
  const int am = (ALIGN_ALL_OBJECTS ? 2 : 1) * BytesPerWord - 1;

  // check for negative or excessive length
  // note: the maximum length allowed is chosen so that arrays of any
  //       element size with this length are always smaller or equal
  //       to the largest integer (i.e., array size computation will
  //       not overflow)
  const int max_length = 0x01000000; // sparc friendly value, requires sethi only
  set(max_length, t1);
  cmp(len, t1);
  br(Assembler::greaterUnsigned, false, Assembler::pn, slow_case);
   
  // compute array size
  // note: if 0 <= len <= max_length, len*elt_size + header + alignment is
  //       smaller or equal to the largest integer; also, since top is always
  //       aligned, we can do the alignment here instead of at the end address
  //       computation
  const Register arr_size = t1;
  switch (elt_size) {
    case  1: delayed()->mov(len,    arr_size); break;
    case  2: delayed()->sll(len, 1, arr_size); break;
    case  4: delayed()->sll(len, 2, arr_size); break;
    case  8: delayed()->sll(len, 3, arr_size); break;
    default: ShouldNotReachHere();
  }
  add(arr_size, hdr_size * wordSize + am, arr_size);   // add space for header & alignment
  and3(arr_size, ~am, arr_size);                           // align array size
  // in case of elt_size 4 or 8, no and3(...) would be necessary
  // (depending on the ALIGN_ALL_OBJECTS value) since the arr_size
  // is automatically aligned and the hdr_size could be aligned at
  // compile time; this could save one instruction => don't do this
  // optimization for now for simplicity
 
  // allocate space & initialize header
  try_allocate(obj, arr_size, 0, t2, t3, t4, am, slow_case);
  initialize_header(obj, klass, len, t2);

  // initialize body
  const Register base  = t2;
  const Register index = t3;
  add(obj, hdr_size * wordSize, base);               // compute address of first element
  sub(arr_size, hdr_size * wordSize, index);         // compute index = number of words to clear
  initialize_body(base, index);
}

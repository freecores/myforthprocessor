#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)c1_MacroAssembler_i486.cpp	1.32 03/01/23 10:53:17 JVM"
#endif
// 
// Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

#include "incls/_precompiled.incl"
#include "incls/_c1_MacroAssembler_i486.cpp.incl"

void C1_MacroAssembler::lock_object(Register hdr, Register obj, Register disp_hdr, Label& slow_case) {
  const int aligned_mask = 3;
  const int hdr_offset = oopDesc::mark_offset_in_bytes();
  assert(hdr == eax, "hdr must be eax for the cmpxchg instruction");
  assert(hdr != obj && hdr != disp_hdr && obj != disp_hdr, "registers must be different");
  assert(BytesPerWord == 4, "adjust aligned_mask and code");
  Label done;
  // The following move must be the first instruction of emitted since debug
  // information may be generated for it.
  // Load object header
  movl(hdr, Address(obj, hdr_offset));

  verify_oop(obj);

#ifdef ASSERT
  {
    Label ok;

    movl(hdr, Address(obj));
    cmpl(disp_hdr, hdr);
    jcc(Assembler::notEqual, ok);
    stop("bad recursive lock");
    bind(ok);
  }
#endif

  // save object being locked into the BasicObjectLock
  movl(Address(disp_hdr, BasicObjectLock::obj_offset_in_bytes()), obj);
  // and mark it as unlocked
  orl(hdr, markOopDesc::unlocked_value);
  // save unlocked object header into the displaced header location on the stack
  movl(Address(disp_hdr), hdr);
  // test if object header is still the same (i.e. unlocked), and if so, store the
  // displaced header address in the object header - if it is not the same, get the
  // object header instead
  if (os::is_MP()) MacroAssembler::lock(); // must be immediately before cmpxchg!
  cmpxchg(disp_hdr, Address(obj, hdr_offset));
  // if the object header was the same, we're done
  jcc(Assembler::equal, done);
  // if the object header was not the same, it is now in the hdr register
  // => test if it is a stack pointer into the same stack (recursive locking), i.e.:
  //
  // 1) (hdr & aligned_mask) == 0
  // 2) esp <= hdr
  // 3) hdr <= esp + page_size
  //
  // these 3 tests can be done by evaluating the following expression:
  //
  // (hdr - esp) & (aligned_mask - page_size)
  //
  // assuming both the stack pointer and page_size have their least
  // significant 2 bits cleared and page_size is a power of 2
  subl(hdr, esp);
  andl(hdr, aligned_mask - os::vm_page_size());
  // for recursive locking, the result is zero => save it in the displaced header
  // location (NULL in the displaced hdr location indicates recursive locking)
  movl(Address(disp_hdr), hdr);
  // otherwise we don't care about the result and handle locking via runtime call
  jcc(Assembler::notZero, slow_case);
  // done
  bind(done);
}


void C1_MacroAssembler::unlock_object(Register hdr, Register obj, Register disp_hdr, Label& slow_case) {
  const int aligned_mask = 3;
  const int hdr_offset = oopDesc::mark_offset_in_bytes();
  assert(disp_hdr == eax, "disp_hdr must be eax for the cmpxchg instruction");
  assert(hdr != obj && hdr != disp_hdr && obj != disp_hdr, "registers must be different");
  assert(BytesPerWord == 4, "adjust aligned_mask and code");
  Label done;
  // load displaced header
  movl(hdr, Address(disp_hdr));
  // if the loaded hdr is NULL we had recursive locking
  testl(hdr, hdr);
  // if we had recursive locking, we are done
  jcc(Assembler::zero, done);
  // load object
  movl(obj, Address(disp_hdr, BasicObjectLock::obj_offset_in_bytes()));
  verify_oop(obj);
  // test if object header is pointing to the displaced header, and if so, restore
  // the displaced header in the object - if the object header is not pointing to
  // the displaced header, get the object header instead
  if (os::is_MP()) MacroAssembler::lock(); // must be immediately before cmpxchg!
  cmpxchg(hdr, Address(obj, hdr_offset));
  // if the object header was not pointing to the displaced header,
  // we do unlocking via runtime call
  jcc(Assembler::notEqual, slow_case);
  // done
  bind(done);
}


void C1_MacroAssembler::allocate_object(Register obj, Register t1, Register t2, int header_size, int object_size, Register klass, Label& slow_case) {
  assert(obj == eax, "obj must be in eax for cmpxchg");
  assert(obj != t1 && obj != t2 && t1 != t2, "registers must be different");
  assert(header_size >= 0 && object_size >= header_size, "illegal sizes");

  // allocate space
  if (UseTLAB) {
    const Register end    = t1;
    const Register thread = t2;
    
    get_thread(thread);
    movl(obj, Address(thread, JavaThread::tlab_top_offset()));
    leal(end, Address(obj, object_size * BytesPerWord));
    cmpl(end, Address(thread, JavaThread::tlab_end_offset()));
    jcc(Assembler::above, slow_case);
    movl(Address(thread, JavaThread::tlab_top_offset()), end);
  }
  else {
    const Register end = t1;
    Label retry;
    bind(retry);
    movl(obj, Address((int)Universe::heap()->top_addr(), relocInfo::none));
    leal(end, Address(obj, object_size * BytesPerWord));
    cmpl(end, Address((int)Universe::heap()->end_addr(), relocInfo::none));
    jcc(Assembler::above, slow_case);
    // Compare obj with the top addr, and if still equal, store the new top addr in
    // end at the address of the top addr pointer. Sets ZF if was equal, and clears
    // it otherwise. Use lock prefix for atomicity on MPs.
    if (os::is_MP()) lock();
    cmpxchg(end, Address((int)Universe::heap()->top_addr(), relocInfo::none));
    // if someone beat us on the allocation, try again, otherwise continue 
    jcc(Assembler::notEqual, retry);
  }

  // initialize header
  movl(Address(obj, oopDesc::mark_offset_in_bytes ()), (int)markOopDesc::prototype());
  movl(Address(obj, oopDesc::klass_offset_in_bytes()), klass);

  // clear rest of allocated space
  const Register t1_zero = t1;
  const Register index = t2;
  const int threshold = 6;   // approximate break even point for code size (see comments below)
  if (object_size <= threshold) {
    // use explicit null stores
    // code size = 2 + 3*n bytes (n = number of fields to clear)
    xorl(t1_zero, t1_zero); // use t1_zero reg to clear memory (shorter code)
    for (int i = header_size; i < object_size; i++) 
      movl(Address(obj, i*BytesPerWord), t1_zero);
  } else if (object_size > header_size) {
    // use loop to null out the fields
    // code size = 16 bytes for even n (n = number of fields to clear)
    // initialize last object field first if odd number of fields
    xorl(t1_zero, t1_zero); // use t1_zero reg to clear memory (shorter code)
    movl(index, (object_size - header_size) >> 1);
    // initialize last object field if object size is odd
    if (((object_size - header_size) & 1) != 0) 
      movl(Address(obj, (object_size - 1)*BytesPerWord), t1_zero);
    // initialize remaining object fields: edx is a multiple of 2
    { Label loop;
      bind(loop);
      movl(Address(obj, index, Address::times_8, 
	(header_size - 1)*BytesPerWord), t1_zero);
      movl(Address(obj, index, Address::times_8, 
	(header_size - 2)*BytesPerWord), t1_zero);
      decl(index);
      jcc(Assembler::notZero, loop);
    }
  }
}


void C1_MacroAssembler::allocate_array(Register obj, Register len, Register t, Register t2, int header_size, Address::ScaleFactor f, Register klass, Label& slow_case) {

  assert(obj == eax, "obj must be in eax for cmpxchg");
  assert(obj != len && obj != t && len != t, "registers must be different");

  // determine alignment mask
  assert(BytesPerWord == 4, "must be a multiple of 2 for masking code to work");
  const int am = (ALIGN_ALL_OBJECTS ? 2 : 1) * BytesPerWord - 1;

  // check for negative or excessive length
  const int max_length = 0x00FFFFFF;
  cmpl(len, max_length);
  jcc(Assembler::above, slow_case);

  // allocate space
  const Register end = t;
  if (UseTLAB) {
    const Register thread = t2;
    get_thread(thread);
    movl(obj, Address(thread, JavaThread::tlab_top_offset()));
    leal(end, Address(obj, len, f));
    // align object end
    addl(end, header_size * BytesPerWord + am);
    andl(end, ~am);
    cmpl(end, Address(thread, JavaThread::tlab_end_offset()));
    jcc(Assembler::above, slow_case);
    movl(Address(thread, JavaThread::tlab_top_offset()), end);
  }
  else {
    Label retry;
    bind(retry);
    movl(obj, Address((int)Universe::heap()->top_addr(), relocInfo::none));
    leal(end, Address(obj, len, f));
    // align object end
    addl(end, header_size * BytesPerWord + am);
    andl(end, ~am);
    // if end < obj then we wrapped around => object too long => slow case
    // note: if 0 <= len <= max_length, len*f + header + alignment is always
    //       smaller than then the largest integer, thus we can wrap around
    //       at most once
    cmpl(end, obj);
    jcc(Assembler::below, slow_case);
    cmpl(end, Address((int)Universe::heap()->end_addr(), relocInfo::none));
    jcc(Assembler::above, slow_case);
    // Compare obj with the top addr, and if still equal, store the new top addr in
    // end at the address of the top addr pointer. Sets ZF if was equal, and clears
    // it otherwise. Use lock prefix for atomicity on MPs.
    if (os::is_MP()) lock();
    cmpxchg(end, Address((int)Universe::heap()->top_addr(), relocInfo::none));
    // if someone beat us on the allocation, try again, otherwise continue 
    jcc(Assembler::notEqual, retry);
  }

  // initialize header
  movl(Address(obj, oopDesc::mark_offset_in_bytes ()), (int)markOopDesc::prototype());
  movl(Address(obj, oopDesc::klass_offset_in_bytes()), klass);
  movl(Address(obj, arrayOopDesc::length_offset_in_bytes()), len);

  // clear rest of allocated space
  Label done;
  const Register len_zero = len;
  const Register index = end;
  // determine index of last word relative to (object start + header)
  subl(index, obj);
  subl(index, header_size * BytesPerWord);
  jcc(Assembler::zero, done);
  // initialize topmost word, divide index by 2, check if odd and test if zero
  // note: for the remaining code to work, index must be a multiple of BytesPerWord
#ifdef ASSERT
  { Label L;
    testl(index, BytesPerWord - 1);
    jcc(Assembler::zero, L);
    stop("index is not a multiple of BytesPerWord");
    bind(L);
  }
#endif
  xorl(len_zero, len_zero);  // use _zero reg to clear memory (shorter code)
  shrl(index, 3);            // divide by 8 and set carry flag if bit 2 was set
  // index could have been not a multiple of 8 (i.e., bit 2 was set)
  { Label even;
    // note: if index was a multiple of 8, than it cannot
    //       be 0 now otherwise it must have been 0 before
    //       => if it is even, we don't need to check for 0 again
    jcc(Assembler::carryClear, even);
    // clear topmost word (no jump needed if conditional assignment would work here)
    movl(Address(obj, index, Address::times_8, (header_size - 0)*BytesPerWord),
      len_zero);
    // index could be 0 now, need to check again
    jcc(Assembler::zero, done);
    bind(even);
  }
  // initialize remaining object fields: edx is a multiple of 2 now
  { Label loop;
    bind(loop);
    movl(Address(obj, index, Address::times_8, (header_size - 1)*BytesPerWord),
      len_zero);
    movl(Address(obj, index, Address::times_8, (header_size - 2)*BytesPerWord),
      len_zero);
    decl(index);
    jcc(Assembler::notZero, loop);
  }

  // done
  bind(done);
}




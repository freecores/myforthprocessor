#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)assembler_solaris_i486.cpp	1.10 03/01/23 11:09:45 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_assembler_solaris_i486.cpp.incl"


void Assembler::int3() {
  pushl(eax);
  pushl(edx);
  pushl(ecx);
  call(CAST_FROM_FN_PTR(address, os::breakpoint), relocInfo::runtime_call_type);
  popl(ecx);
  popl(edx);
  popl(eax);
}


void MacroAssembler::get_thread(Register thread) {  
  // slow call to of thr_getspecific
  // int thr_getspecific(thread_key_t key, void **value);  
  pushl(0);								// allocate space for return value
  if (thread != eax) pushl(eax);					// save eax if caller still wants it
  pushl(ecx);							        // save caller save
  pushl(edx);							        // save caller save
  if (thread != eax) {
    leal(thread, Address(esp, 3 * sizeof(int)));	                // address of return value
  } else {
    leal(thread, Address(esp, 2 * sizeof(int)));	                // address of return value
  }
  pushl(thread);							// and pass the address
  pushl(ThreadLocalStorage::thread_index());				// the key
  call(CAST_FROM_FN_PTR(address, thr_getspecific), relocInfo::runtime_call_type);
  increment(esp, 2 * wordSize);
  popl(edx);
  popl(ecx);
  if (thread != eax) popl(eax);
  popl(thread);
}

void MacroAssembler::get_thread_id(Register thread) {  

  // Use the TLS as the id.

  get_thread(thread);
}

bool MacroAssembler::needs_explicit_null_check(int offset) {
  // Identical to Sparc/Solaris code
  bool offset_in_first_page =   0 <= offset  &&  offset < os::vm_page_size();
  return !offset_in_first_page;
}




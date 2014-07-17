#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)assembler_linux_i486.cpp	1.8 03/01/23 11:08:46 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_assembler_linux_i486.cpp.incl"

// define _USE_PTHREAD_SPECIFIC in Makefile if you want to use 
// pthread_getspecific and pthread_setspecific for thread local 
// storage, even when we're running  with fixed stack pthread 
// library. DO NOT just define it in this file.

void Assembler::int3() {
  call(CAST_FROM_FN_PTR(address, os::breakpoint), relocInfo::runtime_call_type);
}

void MacroAssembler::get_thread(Register thread) {
   // call pthread_getspecific
   // void * pthread_getspecific(pthread_key_t key);
   if (thread != eax) pushl(eax);
   pushl(ecx);
   pushl(edx);

   pushl(ThreadLocalStorage::thread_index());
   call(CAST_FROM_FN_PTR(address, pthread_getspecific), relocInfo::runtime_call_type);
   increment(esp, wordSize);

   popl(edx);
   popl(ecx);
   if (thread != eax) {
       movl(thread, eax);
       popl(eax);
   }
}

void MacroAssembler::get_thread_id(Register thread) {  
  // Use the TLS as the id.
  get_thread(thread);
}

// NOTE: since the linux kernel resides at the low end of
// user address space, no null pointer check is needed.
bool MacroAssembler::needs_explicit_null_check(int offset) {
  return (offset < 0 || offset >= 0x100000);
}

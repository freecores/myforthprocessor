#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)assembler_win32_ia64.cpp	1.5 03/01/23 11:11:16 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_assembler_win32_ia64.cpp.incl"


#if 0
void Assembler::int3() {
  emit_byte(0xCC);
}
#endif


void MacroAssembler::get_thread_id(Register thread) {
#ifdef _M_IA64
  assert(0, "Fix get_thread_id");
#else
  prefix(FS_segment); movl(thread, Address(ThreadLocalStorage::array_offset(), relocInfo::none));
#endif
}

void MacroAssembler::get_thread(Register thread) {
#ifdef _M_IA64
  assert(0, "Fix get_thread");
#else
  get_thread_id(thread);
  movl(thread, Address(thread, ThreadLocalStorage::base_offset() + ThreadLocalStorage::thread_offset()));
#endif
}


/*
 * We don't have implicit null check support yet so we have
 * to do explicit checks for nulls. 
 */
bool MacroAssembler::needs_explicit_null_check(intptr_t offset) {
#if 0
  return offset < 0 || (intptr_t)os::vm_page_size() <= offset;
#else
  return true;
#endif
}


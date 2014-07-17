#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)assembler_win32_i486.cpp	1.7 03/01/23 11:10:45 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_assembler_win32_i486.cpp.incl"


void Assembler::int3() {
  emit_byte(0xCC);
}


void MacroAssembler::get_thread_id(Register thread) {
  prefix(FS_segment); movl(thread, Address(ThreadLocalStorage::array_offset(), relocInfo::none));
}

void MacroAssembler::get_thread(Register thread) {
  get_thread_id(thread);
  if (os::win32::is_nt() && ThreadLocalStorage::thread_index() >= 64 ) {
    movl(thread, Address(thread, 0x0f94)) ; //next entry
  }
  movl(thread, Address(thread, ThreadLocalStorage::base_offset() + ThreadLocalStorage::thread_offset()));
}


bool MacroAssembler::needs_explicit_null_check(int offset) {
  return offset < 0 || (int)os::vm_page_size() <= offset;
}







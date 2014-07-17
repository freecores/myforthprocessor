#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)os_win32_i486.cpp	1.12 03/01/23 11:10:58 JVM"
#endif
// 
// Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

// do not include  precompiled  header file
# include "incls/_os_win32_i486.cpp.incl"


char* os::reserve_memory(size_t bytes) {
  char* res = (char*) VirtualAlloc(NULL, bytes, MEM_RESERVE, PAGE_EXECUTE_READWRITE);
  if (!res) return NULL;

  // Ensure MP-correctness when we patch instructions containing addresses.
  //
  // The above doesn't seem to be the real reason for this check.  Rather,
  // Universe::is_non_oop needs this guarantee in order for it to work properly.
  // See bug 4726610.
  const int high_half_mask = -1 << 24;
  guarantee(((long)res & high_half_mask) != ((long)os::non_memory_address_word() & high_half_mask), "high half of address must not be all-ones");

  return res;
}

char* os::non_memory_address_word() {
  // Must never look like an address returned by reserve_memory,
  // even in its subfields (as defined by the CPU immediate fields,
  // if the CPU splits constants across multiple instructions).
  // On Intel Win32, virtual addresses never have the sign bit set.
  return (char*) -1;
}

void os::initialize_thread() {
// Nothing to do.
}

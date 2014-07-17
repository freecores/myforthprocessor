#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)threadLS_win32_i486.cpp	1.10 03/01/23 11:11:06 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Provides an entry point we can link against and
// a buffer we can emit code into. The buffer is
// filled by ThreadLocalStorage::generate_code_for_get_thread
// and called from ThreadLocalStorage::thread()

#include "incls/_precompiled.incl"
#include "incls/_threadLS_win32_i486.cpp.incl"

int ThreadLocalStorageOffset = 0;

// This code is win32 specific
int ThreadLocalStorage::array_offset() {
  // see definition of struct _NT_TIB in WINNT.H
  return 0x18;
}

int ThreadLocalStorage::base_offset() {
  return os::win32::is_nt()
       ? (_thread_index < 64 ? 0xe10 : -256) // found by looking at TLS code on NT and 2K
       :  0x88;                              // found by looking at TLS code on Windows95
}

// This code is win32/x86 specific
void ThreadLocalStorage::generate_code_for_get_thread() {
  ::ThreadLocalStorageOffset = base_offset() + thread_offset();

  // We have to generate code for get_thread since the
  // tls index is resolved at load time.
  // The reason we cannot use the nice assembler class is
  // it uses resource object which in turn uses get_thread
  // for allocation.
  address buffer = (address) &get_thread;

  if (os::win32::is_nt()) {
    if( _thread_index < 64 ) {
      // mov eax, dword ptr fs:[ThreadLocalStorageOffset]
      // 64 A1 ThreadLocalStorageOffset
      *buffer++ = 0x64;
      *buffer++ = 0xA1;
      *((int*) buffer) = ThreadLocalStorageOffset;
      buffer += sizeof(int);
    } else {
      // mov eax, dword ptr fs:[0x18]
      // 64 A1 18 00 00 00
      *buffer++ = 0x64;
      *buffer++ = 0xA1;
      *buffer++ = 0x18;
      *buffer++ = 0x00;
      *buffer++ = 0x00;
      *buffer++ = 0x00;

      // mov eax, [eax + 0f94 ]
      // 8B 80 94 0f 00 00
      *buffer++ = 0x8b; 
      *buffer++ = 0x80;
      *buffer++ = 0x94;
      *buffer++ = 0x0f;
      *buffer++ = 0x00;
      *buffer++ = 0x00;

      // mov eax, dword ptr [eax+ThreadLocalStorageOffset];
      // 8B 80 ThreadLocalStorageOffset
      *buffer++ = 0x8b; 
      *buffer++ = 0x80;
      *((int*) buffer) = ThreadLocalStorageOffset;
      buffer += sizeof(int);
    }
  } else {
    // mov eax, dword ptr fs:[18H];
    // 64 A1 18 00 00 00
    *buffer++ = 0x64;
    *buffer++ = 0xA1;
    *buffer++ = 0x18;
    *buffer++ = 0x00;
    *buffer++ = 0x00;
    *buffer++ = 0x00;

    // mov eax, dword ptr [eax+ThreadLocalStorageOffset];
    // 8B 80 ThreadLocalStorageOffset
    *buffer++ = 0x8B;
    *buffer++ = 0x80;
    *((int*) buffer) = ThreadLocalStorageOffset;
    buffer += sizeof(int);
  }

  // ret
  // C3
  *buffer++ = 0xC3;

  const int get_thread_size = 20; // constant copied from getThread_i486.cpp
  guarantee((buffer - (address) &get_thread) <= get_thread_size, "check boundary");
}

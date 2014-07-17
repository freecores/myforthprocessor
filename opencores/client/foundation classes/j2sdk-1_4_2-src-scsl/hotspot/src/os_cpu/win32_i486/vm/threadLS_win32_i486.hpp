#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)threadLS_win32_i486.hpp	1.11 03/01/23 11:11:08 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Processor dependent parts of ThreadLocalStorage
public:
  enum Constants {
    _pd_cache_size         =   8*2, // projected max # of processors * 2
    _pd_min_page_size      =   4*K, // smallest possible thread-stack page size
    _pd_typical_stack_size = 512*K, // typical "small" thread stack size
  };

  static address pd_sp_address() { return os::current_stack_pointer(); }

  // Support for assembly stub generation (Windows)
  // ex. fetching the thread:
  //     mov  eax, dword ptr fs:[array_offset()]
  //     mov  eax, dword ptr [eax + base_offset() + thread_offset()]
  static int array_offset();
  static int base_offset();
  static inline int thread_offset() {return thread_index() * sizeof(int);}

  // Java Thread
  inline static Thread* thread() {
    extern int ThreadLocalStorageOffset;
    Thread* current;
    int thread_index = ThreadLocalStorage::thread_index();

    __asm {
      cmp thread_index, 0x40 ;
      // switch to slow path
      jge L ;
      // load thread information block (TIB) pointer from special segment
      mov eax, dword ptr FS:[18H];

      // load the offset of Thread::current()
      mov ecx, dword ptr [ThreadLocalStorageOffset];

      // load the Thread::current() from TIB
      mov eax, dword ptr [eax+ecx];

      // make compiler happy. the #prama does not work inside class declaration
      mov current, eax;
    }

    return current;

L:  return ThreadLocalStorage::get_thread_slow();

  }



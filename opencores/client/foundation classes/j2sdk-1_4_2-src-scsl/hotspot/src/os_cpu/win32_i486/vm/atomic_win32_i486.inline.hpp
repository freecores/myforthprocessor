#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)atomic_win32_i486.inline.hpp	1.6 03/03/04 17:51:18 JVM"
#endif
// 
// Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

// The following alternative implementations are needed because
// Windows 95 doesn't support (some of) the corresponding Windows NT
// calls. Furthermore, these versions allow inlining in the caller.
// (More precisely: The documentation for InterlockedExchange says
// it is supported for Windows 95. However, when single-stepping
// through the assembly code we cannot step into the routine and
// when looking at the routine address we see only garbage code.
// Better safe then sorry!). Was bug 7/31/98 (gri).
//
// Performance note: On uniprocessors, the 'lock' prefixes are not
// necessary (and expensive). We should generate separate cases if
// this becomes a performance problem.

#pragma warning(disable: 4035) // Disables warnings reporting missing return statement

inline jint atomic::add(jint add_value, jint* dest) {
  __asm {
    mov edx, dest;
    mov eax, add_value;
    mov ecx, eax;
    lock xadd dword ptr [edx], eax;
    add eax, ecx;
  }
}

inline void atomic::increment(jint* dest) {
  // alternative for InterlockedIncrement
  __asm {
    mov edx, dest;
    lock inc dword ptr [edx];
  }
}

inline void atomic::decrement(jint* dest) {
  // alternative for InterlockedDecrement
  __asm {
    mov edx, dest;
    lock dec dword ptr [edx];
  }
}

inline jint atomic::exchange(jint exchange_value, jint* dest) {
  // alternative for InterlockedExchange
  __asm {
    mov eax, exchange_value;
    mov ecx, dest;
    xchg eax, dword ptr [ecx];
  }
}

inline void atomic::store(jint store_value, volatile jint* dest) {
  *dest = store_value;
}

inline jint atomic::compare_and_exchange(jint exchange_value, jint* dest, jint compare_value) {
  // alternative for InterlockedCompareExchange
  __asm {
    mov edx, dest
    mov ecx, exchange_value
    mov eax, compare_value
    lock cmpxchg dword ptr [edx], ecx   
  }
}

inline intptr_t atomic::add_ptr(intptr_t add_value, intptr_t* dest) {
  __asm {
    mov edx, dest;
    mov eax, add_value;
    mov ecx, eax;
    lock xadd dword ptr [edx], eax;
    add eax, ecx;
  }
}

inline void atomic::increment_ptr(intptr_t* dest) {
  // alternative for InterlockedIncrement
  __asm {
    mov edx, dest;
    lock inc dword ptr [edx];
  }
}

inline void atomic::decrement_ptr(intptr_t* dest) {
  // alternative for InterlockedDecrement
  __asm {
    mov edx, dest;
    lock dec dword ptr [edx];
  }
}

inline intptr_t atomic::exchange_ptr(intptr_t exchange_value, intptr_t* dest) {
  // alternative for InterlockedExchange
  __asm {
    mov eax, exchange_value;
    mov ecx, dest;
    xchg eax, dword ptr [ecx];
  }
}

inline void atomic::store_ptr(intptr_t store_value, volatile intptr_t* dest) {
  *dest = store_value;
}

// If compare_value == [dest], then [dest] = exchange_value, return exchange_value
// else return compare_value
inline intptr_t atomic::compare_and_exchange_ptr(intptr_t exchange_value, intptr_t* dest, intptr_t compare_value) {
  // alternative for InterlockedCompareExchange
  __asm {
    mov edx, dest
    mov ecx, exchange_value
    mov eax, compare_value
    lock cmpxchg dword ptr [edx], ecx   
  }
}

inline jlong atomic::compare_and_exchange_long(jlong exchange_value, jlong* dest, jlong compare_value) {
   return (*os::win32::atomic_compare_and_exchange_long_func)(exchange_value, dest, compare_value);
}

inline void atomic::membar() {
  __asm {
    lock add dword ptr [esp], 0;
  }
}

inline void atomic::prefetch_read(void *loc, intx interval) {
  // no-op
}
inline void atomic::prefetch_write(void *loc, intx interval) {
  // no-op
}
#pragma warning(default: 4035) // Enables warnings reporting missing return statement

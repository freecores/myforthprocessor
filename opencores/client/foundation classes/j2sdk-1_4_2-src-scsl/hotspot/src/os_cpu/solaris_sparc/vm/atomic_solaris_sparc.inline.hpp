#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)atomic_solaris_sparc.inline.hpp	1.9 03/01/23 11:10:18 JVM"
#endif
// 
// Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 


extern "C" uintptr_t _raw_thread_id();  // in .il file
inline uintptr_t atomic::raw_thread_id() {
  return _raw_thread_id(); 
}

#if defined(COMPILER2) || defined(_LP64)

// This is the interface to the atomic instructions in the .il
// file.  It's very messy because we need to support v8 and these
// instructions are illegal there.  When sparc v8 is dropped, we
// can drop out lots of this code.  Also compiler2 does not support
// v8 so the conditional code omits the instruction set check.

extern "C" jint     _atomic_exchange_v9(jint exchange_value, jint *dest);
extern "C" jint     _atomic_cas_v9(jint exchange_value,
                                   jint* dest,
                                   jint compare_value);
extern "C" intptr_t _atomic_casx_v9(intptr_t exchange_value,
                                    intptr_t* dest,
                                    intptr_t compare_value);
extern "C" jlong    _atomic_cas_long_v9(jlong exchange_value,
                                        jlong* dest,
                                        jlong compare_value);
extern "C" jint _atomic_add_v9(jint inc, jint *loc);

extern "C" intptr_t _atomic_add_ptr_v9(intptr_t add_value, intptr_t* dest);

extern "C" intptr_t _atomic_exchange_ptr_v9(intptr_t exchange_value, intptr_t* dest);


inline jint atomic::exchange(jint exchange_value, jint* dest) {
  // 64 bit uses the CASX instruction
  return _atomic_exchange_v9(exchange_value, dest);
}


inline jint atomic::compare_and_exchange(jint exchange_value,
                                  jint* dest,
                                  jint compare_value) {
  return _atomic_cas_v9(exchange_value, dest, compare_value);
}


inline intptr_t atomic::compare_and_exchange_ptr(intptr_t exchange_value,
                                          intptr_t* dest, 
                                          intptr_t compare_value) {
#ifdef _LP64
  return _atomic_casx_v9(exchange_value, dest, compare_value);
#else  // COMPILER2
  return _atomic_cas_v9(exchange_value, dest, compare_value);
#endif // _LP64
}


inline jint atomic::add(jint inc, jint* loc) {
  return _atomic_add_v9(inc, loc);
}

inline intptr_t atomic::add_ptr(intptr_t add_value, intptr_t* dest) {
#ifdef _LP64
  return _atomic_add_ptr_v9(add_value, dest);
#else // COMPILER2
  return _atomic_add_v9(add_value, dest);
#endif // _LP64
}


inline intptr_t atomic::exchange_ptr(intptr_t exchange_value, intptr_t* dest) {
#ifdef _LP64
  return _atomic_exchange_ptr_v9(exchange_value, dest);
#else   // COMPILER2
  return _atomic_exchange_v9(exchange_value, dest);
#endif // _LP64
}

inline jlong atomic::compare_and_exchange_long(jlong exchange_value,
                                        jlong* dest,
                                        jlong compare_value) {
#ifdef _LP64
  // Return 64 bit value in %o0
  return _atomic_casx_v9((intptr_t)exchange_value, (intptr_t *)dest,
                         (intptr_t)compare_value);
#else // COMPILER2
  assert (VM_Version::v9_instructions_work(), "only supported on v9");
  // Return 64 bit value in %o0,%o1 by hand
  return _atomic_cas_long_v9(exchange_value, dest, compare_value);
#endif
}


extern "C" void _atomic_membar_v9();

inline void atomic::membar(void) {
  _atomic_membar_v9();
}

extern "C" void _atomic_prefetch_read(void *loc, intx interval);
inline void atomic::prefetch_read(void *loc, intx interval) {
  // Code outside the loop that this is in should check the
  // Prefetch{Scan,Copy}Interval or PrefetchFieldInterval and not call this.
  _atomic_prefetch_read(loc, interval);
}

extern "C" void _atomic_prefetch_write(void *loc, intx interval);
inline void atomic::prefetch_write(void *loc, intx interval) {
  _atomic_prefetch_write(loc, interval);
}

#else // _LP64 || COMPILER2

// Implementation of class atomic

inline jint atomic::add(jint add_value, jint* dest) {
  return (*os::atomic_increment_func)(add_value, dest);
}

inline jint atomic::exchange(jint exchange_value, jint* dest) {
  return (*os::atomic_exchange_func)(exchange_value, dest);
}

inline jint atomic::compare_and_exchange(jint exchange_value, jint* dest, jint compare_value) {
  return (*os::atomic_compare_and_exchange_func)(exchange_value, dest, compare_value);
}

inline jlong atomic::compare_and_exchange_long(jlong exchange_value, jlong* dest, jlong compare_value) {
  return (*os::atomic_compare_and_exchange_long_func)(exchange_value, dest, compare_value);
}

inline intptr_t atomic::add_ptr(intptr_t add_value, intptr_t* dest) {
  return (intptr_t)(*os::atomic_increment_func)((jint)add_value, (jint*)dest);
}

inline intptr_t atomic::exchange_ptr(intptr_t exchange_value, intptr_t* dest) {
  return (intptr_t)(*os::atomic_exchange_func)((jint)exchange_value, (jint*)dest);
}

inline intptr_t atomic::compare_and_exchange_ptr(intptr_t exchange_value, intptr_t* dest, intptr_t compare_value) {
  return (*os::atomic_compare_and_exchange_func)((jint)exchange_value, (jint*)dest, (jint)compare_value);
}

inline void atomic::membar() {
  (*os::atomic_membar_func)();
}

inline void atomic::prefetch_read(void *loc, intx interval) {
  // no-op
}

inline void atomic::prefetch_write(void *loc, intx interval) {
  // no-op
}
#endif // _LP64 || COMPILER2
